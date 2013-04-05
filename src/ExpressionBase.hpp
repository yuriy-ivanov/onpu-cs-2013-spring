#ifndef EXPRESSIONBASE_HPP_
#define EXPRESSIONBASE_HPP_

#include <map>
#include <string>
#include <cassert>
#include <boost/shared_ptr.hpp>

struct Exp;  // fwd
typedef boost::shared_ptr<const Exp> PCExp;
typedef std::map<std::string, PCExp> Heap;

// Базовый класс для всех выражений языка
struct Exp {
   // Однашаговая редукция выражения
   virtual PCExp eval(Heap& c) const = 0;

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

#endif /* EXPRESSIONBASE_HPP_ */
