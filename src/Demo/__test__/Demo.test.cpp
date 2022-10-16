#include "Demo.test.hpp"

TEST_F(DemoFixture, DemoParsing)
{
    DemoFile->Parse();

    EXPECT_TRUE(DemoFile->CurrentFrameTime);
}
