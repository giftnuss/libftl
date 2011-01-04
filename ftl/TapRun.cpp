
#include "TapSuite.hpp"
#include "TapRun.hpp"

namespace ftl 
{

TapRun::TapRun(TapSuite *s)
   : suite(s)
{
   suite->init();
}

TapRun::~TapRun()
{
   delete suite;
}

void
TapRun::plan(int c)
{
	 print("1..%%\n",c);
}
	 
void 
TapRun::plan()
{
   plan(suite->testCount());      
}

int 
TapRun::run()
{
   suite->testRun();
   return 0;
}

}
