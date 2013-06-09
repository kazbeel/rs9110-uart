#include <cppunit\CompilerOutputter.h>
#include <cppunit\extensions\TestFactoryRegistry.h>
#include <cppunit\ui\text\TestRunner.h>



int main (int argc, char *argv[])
{
    CPPUNIT_NS::Test               *suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();
    CPPUNIT_NS::TextUi::TestRunner  runner;

    runner.addTest(suite);
    runner.run();

    return 0;
}
