#include "Demo/__test__/Demo.test.hpp"
#include "API/DemoReader.hpp"

TEST_F(DemoFixture, DemoReader)
{
    std::unique_ptr<DemoReader> demoReader = std::make_unique<DemoReader>(DEMO_19);
    while (demoReader->Next())
    {
        //std::cout << demoReader->GetCurrentFrame().commandTime << std::endl;
    }
}
