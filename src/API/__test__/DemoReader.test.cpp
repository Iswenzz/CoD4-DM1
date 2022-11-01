#include "Demo/__test__/Demo.test.hpp"

TEST_F(DemoFixture, DemoReader)
{
    while (Reader->Next())
    {
        auto archive = Reader->GetCurrentFrame();
        auto snapshot = Reader->GetCurrentSnapshot();

        if (!snapshot.valid)
            continue;

        for (const entityState_t& entity : Reader->Entities)
        {
            if (entity.number != 208)
                continue;

            std::cout
                << entity.lerp.pos.trTime << " "
                << entity.lerp.pos.trDuration << " "
                << entity.lerp.pos.trBase[0] << " "
                << entity.lerp.pos.trBase[1] << " "
                << entity.lerp.pos.trBase[2] << " "
                << std::endl;
        }

        float x = snapshot.ps.velocity[0];
        float y = snapshot.ps.velocity[1];

        //std::cout << snapshot.serverTime << " " << std::sqrt((x * x) + (y * y)) << std::endl;
        //std::cout << Reader->GetFPS() << std::endl;
    }
    EXPECT_TRUE(Reader->GetCurrentFrame().commandTime);
}

TEST_F(DemoFixture, DemoReaderPlayerName)
{
    Reader->Parse();

    EXPECT_TRUE(Reader->GetPlayerName().netname.size());
}

TEST_F(DemoFixture, DemoReaderParseConfigString)
{
    Reader->Parse();

    EXPECT_TRUE(Reader->ParseConfigString("mapname").size());
}

TEST_F(DemoFixture, DemoReaderReflect)
{
    Reader->Parse();

    EXPECT_EQ(Reader->ReflectDemoValue("Snapshot.ping"), "999");
}

TEST_F(DemoFixture, DemoReaderTime)
{
    Reader->Parse();

    float time = Reader->GetTime();
    float ms = Reader->GetTimeMilliseconds();
    float server = Reader->GetServerTime();

    EXPECT_TRUE(time);
    EXPECT_TRUE(ms);
    EXPECT_TRUE(server);
}
