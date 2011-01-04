
#include <ftl/taptest>

namespace ftl
{
 
class TapTest : public TapBase<TapTest>
{
  void init();
  void oktest(){ ok(true,"true"); };
};

void
TapTest::init()
{
  add(1,&TapTest::oktest);
}
	 
}

using namespace ftl;

int main(int argc, char** argv)
{
  TapRun t(new TapTest);
  t.plan();
  t.run();
  return 0;
}

