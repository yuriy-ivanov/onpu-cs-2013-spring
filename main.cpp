#include <vector>
#include <string>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

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
   Context(const string& n, int v) : name(n), value(v) { }
   string name;
   int value;
};

// Базовый класс для всех выражений языка
struct Exp {
   // Однашаговая редукция выражения
   virtual oo::shared_ptr<const Exp> eval(vector<Context>& c) const = 0;

   // Печать текущего выражения
   virtual void print() const = 0;

   // default: return false, specialized: true
   virtual bool isSimplistic() const { return false; }

   // getters: уродливые, но лучше чем ничего, not to be called directly:
   virtual int	getInt()  const { assert(false); }
   virtual bool getBool() const { assert(false); }

   virtual ~Exp() { }
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

      return oo::shared_ptr<const Exp>(new Int(it->value));
   }

   void print() const
   {
      cout <<  "(Var " << name_ << ")";
   }

   ~Var() { }
};

//======================================================================
//assert(e1->getInt() == (e1.get() ->* &Exp::getInt)());
template<typename Operation, typename ValueAccessor, const char* OpString>
class Binop : public Exp {
   oo::shared_ptr<const Exp> e1_;
   oo::shared_ptr<const Exp> e2_;
   ValueAccessor va;
   Operation op;

   // Кривизна, но что делать??? Следующий компиллер на С++ будет красивее...
   Binop(oo::shared_ptr<const Exp> e1, oo::shared_ptr<const Exp> e2)
      : e1_(e1), e2_(e2) { }
public:
   Binop(const Exp* e1, const Exp* e2) : e1_(e1), e2_(e2) { }

   oo::shared_ptr<const Exp> eval(vector<Context>& c) const
   {
      if (e1_->isSimplistic() && e2_->isSimplistic())
	 return oo::shared_ptr<const Exp>(new Int(op(va(e1_.get()),
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
struct IntValueAccessor { // bind it???
   int operator()(const Exp* e) const { return e->getInt(); }
};

extern const char __extern_Plus[] = "Plus";
extern const char __extern_Minus[] = "Minus";
extern const char __extern_Times[] = "Times";
extern const char __extern_Divide[] = "Divide";

typedef Binop<plus<int>, IntValueAccessor, __extern_Plus> Plus;
typedef Binop<minus<int>, IntValueAccessor, __extern_Minus> Minus;
typedef Binop<multiplies<int>, IntValueAccessor, __extern_Times> Times;
typedef Binop<divides<int>, IntValueAccessor, __extern_Divide> Divide;

//======================================================================
int main()
{
   try {
      vector<Context> c;
      oo::shared_ptr<const Exp> e0(new Var("a"));
      oo::shared_ptr<const Exp> e1(new Divide(new Times(new Minus(new Var("a"),
								  new Int(10)),
							new Plus(new Var("b"),
								 new Int(1))),
					      new Int(4)));

      c.push_back(Context("a", 12));
      c.push_back(Context("b", 31));

      e0->print();		cout << "\n";
      e1->print();		cout << "\n";
      e0->eval(c)->print();	cout << "\n";

      for (; !e1->isSimplistic(); e1 = e1->eval(c));
      e1->print();		cout << "\n";
   } catch (exception& e) {
      cout << e.what()	<< '\n';
      return 1;
   } catch (...) {
      cout << "Unexpected" << '\n';
      return 2;
   }

   return 0;
}
