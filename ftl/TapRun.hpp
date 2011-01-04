/*
 * AvlTree.hpp -- binary tree balancing logic
 *
 * Copyright (c) 2011, Frank Mertens
 *
 * See ../COPYING for the license.
 */
#ifndef FTL_TAPRUN_HPP
#define FTL_TAPRUN_HPP

#include "TapSuite.hpp"

namespace ftl{

class TapRun
{
public:
  TapRun(TapSuite*);
  ~TapRun();
	void plan(int);
  void plan();
  int run();
private:
  TapSuite* suite;
};

}

#endif

