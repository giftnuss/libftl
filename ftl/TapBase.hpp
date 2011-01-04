/*
 * TapBase.hpp -- base class for user tests
 *
 * Copyright (c) 2011, Frank Mertens
 *
 * See ../COPYING for the license.
 */
#ifndef FTL_TAPBASE_HPP
#define FTL_TAPBASE_HPP

#include "List.hpp"
#include "TapSuite.hpp"

namespace ftl
{			

template<typename T>
	class TapBase : public TapSuite
{
		 
	template<typename F>
	  class Func : public Instance
	{
		public:
			Func(F f) : do_(f) {}
		 
			template <typename O>
			void
			perform(O obj)
			{
				 (obj->*do_)();			
			}
 
		private:
			F do_;
	};
	 
	typedef Func< void (T::*)() > Function;
	typedef List< Function* > Functions;

  public:
	  ~TapBase()
    {
			typename Functions::Index i = fp_.first();
	    Function *f;
			 
      while (fp_.def(i)) {
		     f = fp_.at(i);
		     delete f;
				 ++i;
	    }
		}
	 
	  void add(int cnt, void (T::*f)())
	  {
      count_.push(cnt);
			Function *t = new Function(f);
      fp_.push(t);
    }

    int testCount()
    {
      int sum = 0;
	    List<int>::Index i = count_.first();
	    while (count_.def(i)) {
		     sum += count_.at(i);
		     ++i;
	    }
      return sum;
    }

    void testRun()
    {
      T* self = new T();
			typename Functions::Index i = fp_.first();
	    Function *f;
			 
      while (fp_.def(i)) {
		     f = fp_.at(i);
		     f->perform(self);
				 ++i;
	    }
      delete self;
		}

  private:
    List<int> count_;
    Functions fp_;
};

} // end namespace ftl

#endif

