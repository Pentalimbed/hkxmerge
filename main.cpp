#include "merger.h"

static void errorReport(const char* str, void* errorOutputObject)
{
    LOG(str);
}

void initHavok()
{
    hkMemoryRouter* memoryRouter = hkMemoryInitUtil::initDefault(
        hkMallocAllocator::m_defaultMallocAllocator, hkMemorySystem::FrameInfo(2 << 20));
    hkBaseSystem::init(memoryRouter, errorReport);
}

void exitHavok()
{
    hkBaseSystem::quit();
    hkMemoryInitUtil::quit();
}

int main(int argc, char* argv[])
{
    LOG("PROGRAM BEGINS!");

    initHavok();
    try
    {
        process(argc, argv);
    }
    catch (std::exception e)
    {
        LOG("Failed!");
    }

    exitHavok();

    LOG("PROGRAM ENDS!");
    return 0;
}