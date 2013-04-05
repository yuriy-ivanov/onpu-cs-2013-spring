#ifndef EXPRESSIONS_HPP_
#define EXPRESSIONS_HPP_

#include <iostream>
#include <utility>
#include <boost/mpl/set.hpp>
#include <boost/static_assert.hpp>
#include "ExpressionBase.hpp"

using std::cout;
using std::string;
using std::pair;

//======================================================================
class Bool : public Exp {
   bool b_;
public:
   Bool(bool b) : b_(b) { }

   bool isSimplistic() const { return true; }
   bool getBool() const { return b_; }

   PCExp eval(Heap& c) const
   {
      assert(false); // Not to be evaluated explicitly
      return PCExp();
   }

   void print() const
   {
      cout <<  "(Bool " << b_ << ")";
   }

   ~Bool() { }
};

//======================================================================
class Int : public Exp {
   int i_;
public:
   Int(int i) : i_(i) { }

   bool isSimplistic() const { return true; }
   int getInt() const { return i_; }

   PCExp eval(Heap& c) const
   {
      assert(false); // Not to be evaluated explicitly
      return PCExp();
   }

   void print() const
   {
      cout <<  "(Int " << i_ << ")";
   }

   ~Int() { }
};

//======================================================================
class Var : public Exp {
   string name_;
public:
   Var(const string& name) : name_ (name) { }

   PCExp eval(Heap& c) const
   {
      return c.at(name_);
   }

   void print() const
   {
      cout <<  "(Var " << name_ << ")";
   }

   ~Var() { }
};

//======================================================================
class If : public Exp {
   PCExp cond_;
   PCExp true_;
   PCExp false_;
   friend class While; // ugly? why???

   If(PCExp c, PCExp t, PCExp f) : cond_(c), true_(t), false_(f) { }
public:
   If(const Exp* c, const Exp* t, const Exp* f) :
      cond_(c), true_(t), false_(f) { }

   PCExp eval(Heap& c) const
   {
      PCExp cond = cond_;
      PCExp _true = true_;
      PCExp _false = false_;

      for (; !cond->isSimplistic(); cond = cond->eval(c));

      // The following strange looking construct makes things lazy
      if (cond->getBool())
	 for (; !_true->isSimplistic(); _true = _true->eval(c));
      else
	 for (; !_false->isSimplistic(); _false = _false->eval(c));

      return cond->getBool() ? _true : _false;
   }

   void print() const
   {
      cout << "(If "; cond_->print(); cout << ","; true_->print();  cout << ",";
						   false_->print(); cout << ")";
   }

   ~If() { }
};

//======================================================================
class Skip : public Exp {
public:
   Skip() { }

   bool isSimplistic() const { return true; }
   bool isSkip()       const { return true; }

   PCExp eval(Heap& c) const
   {
      assert(false); // Not to be evaluated explicitly
      return PCExp();
   }

   void print() const { cout <<  "(Skip)"; }

   ~Skip() { }
};

//======================================================================
class Seq : public Exp {
   PCExp e1_;
   PCExp e2_;
   friend class While; // ugly? maybe...

   Seq(PCExp e1, PCExp e2) : e1_(e1), e2_(e2) { }
public:
   Seq(const Exp* e1, const Exp* e2) : e1_(e1), e2_(e2) { }

   PCExp eval(Heap& c) const
   {
      // @todo: think about to remove isSkip() from the code, is this just the
      // same as isSimplistic()??? isSimplistic() is XXX, isn't it???
      if (e1_->isSkip() || e1_->isSimplistic())
	 return e2_;

      PCExp e1 = e1_;
      for (; !e1->isSimplistic(); e1 = e1->eval(c));

      return PCExp(new Seq(e1, e2_));
   }

   void print() const
   {
      cout <<  "(Seq "; e1_->print(); cout << ","; e2_->print(); cout <<")";
   }

   ~Seq() { }
};

//======================================================================
class While : public Exp {
   PCExp e_; // condition
   PCExp b_; // body
   While(PCExp e, PCExp b) : e_(e), b_(b) { }
public:
   While(const Exp* e, const Exp* b) : e_(e), b_(b) { }

   PCExp eval(Heap& c) const
   {
      // let rec interp_s (s:exp) = function
      // While(e,s1) -> If(e, Seq(s1,s), Skip)
      PCExp skip(new Skip());
      PCExp seq (new Seq(b_, PCExp(new While(e_, b_))));

      return PCExp(new If(e_, seq, skip));
   }

   void print() const
   {
      cout <<  "(While "; e_->print(); cout << ","; b_->print(); cout <<")";
   }

   ~While() { }
};

//======================================================================
class Assign : public Exp {
   string name_;
   PCExp e_;
   Assign(const string& name, PCExp e)
      : name_(name), e_(e) { }
public:
   Assign(const string& name, const Exp* e) : name_(name), e_(e) { }

   PCExp eval(Heap& c) const
   {
      if (e_->isSimplistic()) {
	 c[name_] = e_;
	 return PCExp(new Skip());
      }

      PCExp e = e_;
      for (; !e->isSimplistic(); e = e->eval(c));
      return PCExp(new Assign(name_, e));
   }

   void print() const
   {
      cout <<  "(Assign "; cout << name_ << ","; e_->print(); cout <<")";
   }

   ~Assign() { }
};



//======================================================================
template<typename Operation,
		 typename RetType,
		 typename ValueAccessor,
		 const char* OpString>
class Binop : public Exp {
   // Available types for (Operation, RetType) in `class Binop'

   typedef boost::mpl::set<
		   pair<std::plus<int>,Int>,
		   pair<std::multiplies<int>,Int>,
		   pair<std::minus<int>,Int>,
		   pair<std::divides<int>,Int>,
		   pair<std::less<int>,Bool>,
		   pair<std::equal_to<int>,Bool>
      	   > BinopConstraints;
   BOOST_STATIC_ASSERT((boost::mpl::has_key<BinopConstraints,
		   	   	   	    pair<Operation, RetType> >::value));

   typedef typename ValueAccessor::Type VAType;
   ValueAccessor va;
   Operation op;
   PCExp e1_;
   PCExp e2_;

   // Кривизна, но что делать??? Следующий компиллер на С++ будет красивее...
   Binop(PCExp e1, PCExp e2) : e1_(e1), e2_(e2) { }
public:
   Binop(const Exp* e1, const Exp* e2) : e1_(e1), e2_(e2) { }

   PCExp eval(Heap& c) const
   {
      if (e1_->isSimplistic() && e2_->isSimplistic())
	 return PCExp(new RetType(op(va(e1_.get()), va(e2_.get()))));

      if (e1_->isSimplistic())
	 return PCExp(new Binop(PCExp(new VAType(va(e1_.get()))), e2_->eval(c)));

     return PCExp(new Binop(e1_->eval(c), e2_));
   }

   void print() const
   {
      cout << "(" << OpString << " "; e1_->print(); cout << ",";
				      e2_->print(); cout << ")";
   }

   ~Binop() { }
};

//======================================================================
struct IntValueAccessor { // @todo: bind it???
   typedef Int Type;
   int operator()(const Exp* e) const { return e->getInt(); }
};

extern const char __extern_Plus  [] = "Plus";
extern const char __extern_Minus [] = "Minus";
extern const char __extern_Times [] = "Times";
extern const char __extern_Divide[] = "Divide";
extern const char __extern_Less  [] = "Less";
extern const char __extern_Equal [] = "Equal";

typedef Binop<std::plus<int>,       Int,  IntValueAccessor, __extern_Plus>   Plus;
typedef Binop<std::minus<int>,      Int,  IntValueAccessor, __extern_Minus>  Minus;
typedef Binop<std::multiplies<int>, Int,  IntValueAccessor, __extern_Times>  Times;
typedef Binop<std::divides<int>,    Int,  IntValueAccessor, __extern_Divide> Divide;
typedef Binop<std::less<int>,       Bool, IntValueAccessor, __extern_Less>   Less;
typedef Binop<std::equal_to<int>,   Bool, IntValueAccessor, __extern_Equal>  Equal;

#endif /* EXPRESSIONS_HPP_ */
