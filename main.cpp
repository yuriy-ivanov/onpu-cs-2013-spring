#include <vector>
#include <string>
#include <cassert>
#include <utility>
#include <iostream>
#include <stdexcept>
#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/mpl/set.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>

// ====================== simple.ml ==================
// exception Hell

// type exp =
//     Int of int
//   | Var of string
//   | Plus of exp * exp

// let rec eval1 ctx e = match e with
//     Int _ -> raise Hell
//   | Var name -> List.assoc name ctx
//   | Plus (Int a, Int b) -> Int (a + b)
//   | Plus (Int a, b) -> Plus (Int a, eval1 ctx b)
//   | Plus (a, b) -> Plus (eval1 ctx a, b)

// let rec eval ctx e =
//   let is_simplistic = function Int _ -> true | _ -> false in
//   let e' = eval1 ctx e in
//   if is_simplistic e' then e' else eval ctx e'

// let variables  = [("a", Plus (Int 1, Int 2)); ("b", Int 1)]
// let expression = Plus (Plus (Var "a", Int 10), Var "b")
// ====================== /simple.ml ==================
// ====================== running interpreter: ========
// ocaml
// #use "simple.ml";;
// eval variables expression;;

namespace oo = boost;
using namespace std;

struct Exp;
struct Context {
   Context(const string& n, const Exp* v) : name(n), value(v) { }
   string name;
   oo::shared_ptr<const Exp> value;
};

// Базовый класс для всех выражений языка
struct Exp {
   // Однашаговая редукция выражения
   virtual oo::shared_ptr<const Exp> eval(vector<Context>& c) const = 0;

   // Печать текущего выражения
   virtual void print() const = 0;

   // default: return false, specialized: true
   virtual bool isSimplistic() const { return false; }
   virtual bool isSkip() const { return false; }

   // getters: уродливые, но лучше чем ничего, not to be called directly:
   virtual int	getInt()  const { assert(false); }
   virtual bool getBool() const { assert(false); }

   virtual ~Exp() { }
};

//======================================================================
class Bool : public Exp {
   bool b_;
public:
   Bool(bool b) : b_(b) { }

   bool isSimplistic() const { return true; }
   bool getBool() const { return b_; }

   oo::shared_ptr<const Exp> eval(vector<Context>& c) const
   {
      assert(false); // Not to be evaluated explicitly
      return oo::shared_ptr<const Exp>();
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

   oo::shared_ptr<const Exp> eval(vector<Context>& c) const
   {
      assert(false); // Not to be evaluated explicitly
      return oo::shared_ptr<const Exp>();
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
   Var(string name) : name_ (name) { }

   oo::shared_ptr<const Exp> eval(vector<Context>& c) const
   {
      vector<Context>::iterator it =
	 find_if(c.begin(), c.end(), oo::bind(&Context::name, _1) == name_);

      if (it == c.end())
	 throw logic_error("Unbound variable name!");

      return it->value;
   }

   void print() const
   {
      cout <<  "(Var " << name_ << ")";
   }

   ~Var() { }
};

//======================================================================
template<typename Operation, typename RetType, typename ValueAccessor,
						const char* OpString>
class Binop : public Exp {
   // Available types for RetType in `class Binop'
   typedef oo::mpl::set<
      pair<plus<int>,Int>,   pair<multiplies<int>,Int>, pair<minus<int>,Int>,
      pair<divides<int>,Int>, pair<less<int>,Bool>,     pair<equal_to<int>,Bool>
      > BinopConstraints;
   BOOST_STATIC_ASSERT((oo::mpl::has_key<BinopConstraints,
			pair<Operation, RetType> >::value));

   oo::shared_ptr<const Exp> e1_;
   oo::shared_ptr<const Exp> e2_;
   ValueAccessor va;
   Operation op;

   // Кривизна, но что делать??? Следующий компиллер на С++ будет красивее...
   Binop(oo::shared_ptr<const Exp> e1,
	 oo::shared_ptr<const Exp> e2) : e1_(e1), e2_(e2) { }
public:
   Binop(const Exp* e1, const Exp* e2) : e1_(e1), e2_(e2) { }

   oo::shared_ptr<const Exp> eval(vector<Context>& c) const
   {
      if (e1_->isSimplistic() && e2_->isSimplistic())
	 return oo::shared_ptr<const Exp>(new RetType(op(va(e1_.get()),
							 va(e2_.get()))));

      if (e1_->isSimplistic())
	 return oo::shared_ptr<const Exp>
	    (new Binop (oo::shared_ptr<const Exp>(new Int(va(e1_.get()))),
			e2_->eval(c)));

      return oo::shared_ptr<const Exp>(new Binop(e1_->eval(c), e2_));
   }

   void print() const
   {
      cout << "(" << OpString << " "; e1_->print(); cout << ",";
				      e2_->print(); cout << ")";
   }

   ~Binop() { }
};

//======================================================================
class If : public Exp {
   oo::shared_ptr<const Exp> cond_;
   oo::shared_ptr<const Exp> true_;
   oo::shared_ptr<const Exp> false_;
public:
   If(const Exp* c, const Exp* t, const Exp* f) :
      cond_(c), true_(t), false_(f) { }

   oo::shared_ptr<const Exp> eval(vector<Context>& c) const
   {
      oo::shared_ptr<const Exp> cond = cond_;
      oo::shared_ptr<const Exp> _true = true_;
      oo::shared_ptr<const Exp> _false = false_;

      for (; !cond->isSimplistic(); cond = cond->eval(c));
      for (; !_true->isSimplistic(); _true = _true->eval(c));
      for (; !_false->isSimplistic(); _false = _false->eval(c));

      // @todo: _true and _false may be evaluated lazily
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

   oo::shared_ptr<const Exp> eval(vector<Context>& c) const
   {
      assert(false); // Not to be evaluated explicitly
      return oo::shared_ptr<const Exp>();
   }

   void print() const { cout <<  "(Skip)"; }

   ~Skip() { }
};

//======================================================================
class Seq : public Exp {
   oo::shared_ptr<const Exp> e1_;
   oo::shared_ptr<const Exp> e2_;
   Seq(oo::shared_ptr<const Exp> e1,
       oo::shared_ptr<const Exp> e2) : e1_(e1), e2_(e2) { }
public:
   Seq(const Exp* e1, const Exp* e2) : e1_(e1), e2_(e2) { }

   oo::shared_ptr<const Exp> eval(vector<Context>& c) const
   {
      // @todo: think about to remove isSkip() from the code, is this just the
      // same as isSimplistic()??? isSimplistic() is XXX, isn't it???
      if (e1_->isSkip() || e1_->isSimplistic())
	 return e2_;

      oo::shared_ptr<const Exp> e1 = e1_;
      for (; !e1->isSimplistic(); e1 = e1->eval(c));

      return oo::shared_ptr<const Exp>(new Seq(e1, e2_));
   }

   void print() const
   {
      cout <<  "(Seq "; e1_->print(); cout << ","; e2_->print(); cout <<")";
   }

   ~Seq() { }
};

//======================================================================
class Assign : public Exp {
   string name_;
   oo::shared_ptr<const Exp> e_;
   Assign(const string& name, oo::shared_ptr<const Exp> e)
      : name_(name), e_(e) { }
public:
   Assign(const string& name, const Exp* e) : name_(name), e_(e) { }

   oo::shared_ptr<const Exp> eval(vector<Context>& c) const
   {
      if (e_->isSimplistic()) {
	 vector<Context>::iterator it =
	    find_if(c.begin(), c.end(), oo::bind(&Context::name, _1) == name_);

	 if (it == c.end())
	    throw logic_error("Unbound variable name!"); // @todo: push back var

	 it->value = e_;
	 return oo::shared_ptr<const Exp>(new Skip());
      }

      oo::shared_ptr<const Exp> e = e_;
      for (; !e->isSimplistic(); e = e->eval(c));
      return oo::shared_ptr<const Exp>(new Assign(name_, e));
   }

   void print() const
   {
      cout <<  "(Assign "; cout << name_ << ","; e_->print(); cout <<")";
   }

   ~Assign() { }
};

//======================================================================
struct IntValueAccessor { // @todo: bind it???
   int operator()(const Exp* e) const { return e->getInt(); }
};

extern const char __extern_Plus[] = "Plus";
extern const char __extern_Minus[] = "Minus";
extern const char __extern_Times[] = "Times";
extern const char __extern_Divide[] = "Divide";
extern const char __extern_Less[] = "Less";
extern const char __extern_Equal[] = "Equal";

typedef Binop<plus<int>,       Int,  IntValueAccessor, __extern_Plus>   Plus;
typedef Binop<minus<int>,      Int,  IntValueAccessor, __extern_Minus>  Minus;
typedef Binop<multiplies<int>, Int,  IntValueAccessor, __extern_Times>  Times;
typedef Binop<divides<int>,    Int,  IntValueAccessor, __extern_Divide> Divide;
typedef Binop<less<int>,       Bool, IntValueAccessor, __extern_Less>   Less;
typedef Binop<equal_to<int>,   Bool, IntValueAccessor, __extern_Equal>  Equal;

//======================================================================
int main()
{
   try {
      vector<Context> c;
      oo::shared_ptr<const Exp>
	 e(new Seq(new Less(new Var("c"),
			    new Times(new Minus(new Var("a"),
						new Int(10)),
				      new Plus(new Var("b"),
					       new Int(1)))),
		   new Seq(new Assign("d", new Int(10)),
			   new Var("d"))));

      c.push_back(Context("a", new Int(12)));
      c.push_back(Context("b", new Int(31)));
      c.push_back(Context("c", new If(new Less(new Var("a"),
						       new Var("b")),
					      new Int(444),
				      new Int(4))));
      c.push_back(Context("d", new Int(0)));

      e->print(); cout << "\n";

      for (int i = 0; i < c.size(); ++i) {
	 cout << "Var " << c[i].name << " = ";
	 c[i].value->print(); cout << "\n";
      }

      for (; !e->isSimplistic(); e = e->eval(c)) {
	 cout << ":::::"; e->print(); cout << "\n";
      }
      e->print(); cout << "\n";

   } catch (exception& e) {
      cout << e.what()	<< '\n';
      return 1;
   } catch (...) {
      cout << "Unexpected" << '\n';
      return 2;
   }

   return 0;
}

//assert(e1->getInt() == (e1.get() ->* &Exp::getInt)());
