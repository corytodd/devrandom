#include "random.h"
#include "pcg32.h"

VOID
NTAPI
DevRandomFillBufferRand(
    PVOID Buffer,
    SIZE_T Length)
{
    LARGE_INTEGER tickCount;
    UINT32 i, randomValue;
    PUINT32 ptr;

    KeQueryTickCount(&tickCount);
    pcg32_seed(tickCount.LowPart);


    ptr = Buffer;
    for (i = 0; i < Length / sizeof(UINT32); i++)
    {
        ptr[i] = pcg32();
    }

    Length &= (sizeof(UINT32) - 1);
    if (Length > 0)
    {
        randomValue = pcg32();
        RtlCopyMemory(&ptr[i], &randomValue, Length);
    }
}
