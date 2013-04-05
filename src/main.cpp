#include <stdexcept>
#include "Expressions.hpp"

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

using std::cout;
using std::exception;

//======================================================================
// @todo: undef them???
#define _Bool	new Bool
#define _Int	new Int
#define _Var	new Var
#define _Plus	new Plus
#define _Minus	new Minus
#define _Times	new Times
#define _Divide new Divide
#define _Less	new Less
#define _Equal	new Equal
#define _While	new While
#define _If		new If
#define _Skip	new Skip
#define _Seq	new Seq
#define _While	new While
#define _Assign new Assign

//======================================================================
int main()
{
   try {
      Heap c;
      PCExp
	 // 1
	 // e(_Seq(_While(_Less(_Var("d"), _Int(5)),
	 // 	       _Assign("d", _Int(11))),
	 // 	_Var("d")));
	 // 2
	 e(
	   _Seq(
			_Less(
				  _Var("c"), _Times(
						            _Minus(_Var("a"),
						            	   _Int(10)
						            	   ),
						            _Plus(
						            	  _Var("b"),
						            	  _Int(1))
						            	  )
				  ),
		    _Seq(
		    	 _Assign(
		    			 "d",
		    			 _Int(10)
		    			 ),
		         _Var("d")
		         )
		   )
	  );

      c["a"] = PCExp(_Int(12));
      c["b"] = PCExp(_Int(31));
      c["c"] = PCExp(_If(_Less(_Var("a"), _Var("b")),
    		  	  	 _Int(444),
    		  	  	 _Int(4)));
      c["d"] = PCExp(_Int(0));

      e->print();
      cout << "\n";

      for (Heap::const_iterator i = c.begin(); i != c.end(); ++i) {
    	  cout << "Var "
    		   << i->first
    		   << " = ";
    	  	   i->second->print();
    	  cout << "\n";
      }

      for (; !e->isSimplistic(); e = e->eval(c)) {
    	  cout << "===> ";
    	  e->print();
    	  cout << "\n";
      }
      e->print();
      cout << "\n";

   } catch (exception& e) {
      cout << e.what()	<< '\n';
      return 1;
   } catch (...) {
      cout << "Unexpected" << '\n';
      return 2;
   }

   return 0;
}

