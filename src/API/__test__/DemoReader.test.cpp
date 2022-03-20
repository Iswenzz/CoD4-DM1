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

        std::cout << frame.commandTime << " " << std::sqrt((x * x) + (y * y)) << std::endl;
    }
}
