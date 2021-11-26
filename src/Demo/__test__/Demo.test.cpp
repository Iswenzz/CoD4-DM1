#include "Demo.test.hpp"

TEST_F(DemoFixture, DemoParsing)
{
    std::unique_ptr<Demo> demo = std::make_unique<Demo>(DEMO_PATH_19);
    demo->Parse();
}
