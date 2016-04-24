#pragma once
#include <windows.h>

LPVOID SearchExecutableMemory(LPVOID lpMemStart, LPVOID lpFunctionHeader, ULONG ulHeaderSize);
LPVOID SearchReadWriteMemory(LPVOID lpBaseAddress, LPVOID lpPtn, DWORD dwPtnSize);