#include "Demo/__test__/Demo.test.hpp"
#include "API/DemoReader.hpp"

TEST_F(DemoFixture, DemoReader)
{
    std::unique_ptr<DemoReader> demoReader = std::make_unique<DemoReader>(DEMO_19);
    while (demoReader->Next())
    {
        auto frame = demoReader->GetCurrentFrame();
        float x = frame.velocity[0];
        float y = frame.velocity[1];

        //std::cout << frame.commandTime << " " << std::sqrt((x * x) + (y * y)) << std::endl;
    }
    EXPECT_GE(demoReader->Frames.size(), 1);
}

TEST_F(DemoFixture, DemoReaderDifference)
{
    std::unique_ptr<DemoReader> demoReader = std::make_unique<DemoReader>(DEMO_19);
    while (demoReader->Next())
    {
        auto entities = demoReader->GetLastUpdatedEntities();

        if (entities.size() > 0)
            EXPECT_TRUE(entities.back().number);
    }
}

TEST_F(DemoFixture, DemoReaderTime)
{
    std::unique_ptr<DemoReader> demoReader = std::make_unique<DemoReader>(DEMO_19);
    demoReader->Parse();

    float time = demoReader->GetTime();
    float ms = demoReader->GetTimeMilliseconds();
    float server = demoReader->GetServerTime();

    EXPECT_TRUE(time);
    EXPECT_TRUE(ms);
    EXPECT_TRUE(server);
}
