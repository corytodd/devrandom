#pragma once

#include <ntddk.h>

//
// Fills Buffer with Length random bytes... 2
//
VOID
NTAPI
DevRandomFillBufferRand(
    PVOID Buffer,
    SIZE_T Length);

