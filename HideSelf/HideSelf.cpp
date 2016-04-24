// HideSelf.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <stdlib.h>
#define ALPHABET_LEN 256

void MakeDelta1(INT *Delta1, PBYTE pSign, DWORD dwSignLen) {
	INT iI;
	for (iI = 0; iI<ALPHABET_LEN; iI++) {
		Delta1[iI] = dwSignLen;
	}
	for (iI = 0; iI < dwSignLen - 1; iI++) {
		Delta1[pSign[iI]] = dwSignLen - 1 - iI;
	}
}

BOOL IsPrefix(BYTE *Word, DWORD dwWordLen, DWORD dwPos) {
	INT iI;
	DWORD dwSuffixlen = dwWordLen - dwPos;
	for (iI = 0; iI < dwSuffixlen; iI++) {
		if (Word[iI] != Word[dwPos + iI]) {
			return FALSE;
		}
	}
	return TRUE;
}

DWORD SuffixLength(BYTE *Word, DWORD dwWordLen, DWORD dwPos) {
	INT iI;
	for (iI = 0; (Word[dwPos - iI] == Word[dwWordLen - 1 - iI]) && (iI<dwPos); iI++);

	return iI;
}

VOID MakeDelta2(INT *Delta2, BYTE *pSign, DWORD dwSignLen) {
	INT iI, iSuffixLen;
	INT LastPrefixIndex = dwSignLen - 1;

	for (iI = dwSignLen - 1; iI >= 0; iI--) {
		if (IsPrefix(pSign, dwSignLen, iI + 1)) {
			LastPrefixIndex = iI + 1;
		}
		Delta2[iI] = LastPrefixIndex + (dwSignLen - 1 - iI);
	}

	for (iI = 0; iI<dwSignLen - 1; iI++) {
		iSuffixLen = SuffixLength(pSign, dwSignLen, iI);
		if (pSign[iI - iSuffixLen] != pSign[dwSignLen - 1 - iSuffixLen]) {
			Delta2[dwSignLen - 1 - iSuffixLen] = dwSignLen - 1 - iI + iSuffixLen;
		}
	}
}

LPVOID BooyerSearch(LPBYTE lpSrc, DWORD dwStringLen, LPBYTE lpSign, DWORD dwSignLen) {

	INT iI;
	INT Delta1[ALPHABET_LEN];
	INT *Delta2 = (INT*)malloc(dwSignLen * sizeof(INT));

	MakeDelta1(Delta1, lpSign, dwSignLen);
	MakeDelta2(Delta2, lpSign, dwSignLen);

	iI = dwSignLen - 1;

	while (iI < dwStringLen) {
		INT dwJ = dwSignLen - 1;
		while (dwJ >= 0 && (lpSrc[iI] == lpSign[dwJ])) {
			--iI;
			--dwJ;
		}
		if (dwJ < 0) {
			free(Delta2);
			return (lpSrc + iI + 1);
		}
		else
			iI += max(Delta1[lpSrc[iI]], Delta2[dwJ]);
	}
	free(Delta2);
	return NULL;
}


//void preQsBc(char *x, int m, int qsBc[]) {
//	int i;
//
//	for (i = 0; i < ASIZE; ++i)
//		qsBc[i] = m + 1;
//	for (i = 0; i < m; ++i)
//		qsBc[x[i]] = m - i;
//}

#define ASIZE 255
char* QS(char *x, int m, char *y, int n) {
	int i, j, qsBc[ASIZE];

	/* Preprocessing */

	for (i = 0; i < ASIZE; ++i)
		qsBc[i] = m + 1;
	for (i = 0; i < m; ++i)
		qsBc[x[i]] = m - i;
	//preQsBc(x, m, qsBc);

	/* Searching */
	j = 0;
	while (j <= n - m) {
		if (memcmp(x, y + j, m) == 0)
			//LOG(TEXT("match pos:%d"), j);
			return y + j;
		j += qsBc[y[j + m]];               /* shift */
	}
	return NULL;
}

LPVOID SearchReadWriteMemory(LPVOID lpBaseAddress, LPVOID lpPtn, DWORD dwPtnSize)
{
	LPVOID lpMatchAddr = NULL;
	MEMORY_BASIC_INFORMATION MemoryInformation;
	LPVOID BaseAddress = lpBaseAddress;

	while (0 != VirtualQuery((LPVOID)BaseAddress, &MemoryInformation, sizeof(MEMORY_BASIC_INFORMATION))) 
	{
		if (MemoryInformation.State == MEM_COMMIT && MemoryInformation.Protect == PAGE_READWRITE)
		{
			lpMatchAddr = BooyerSearch((LPBYTE)BaseAddress, MemoryInformation.RegionSize - ((ULONG)BaseAddress - (ULONG)MemoryInformation.BaseAddress + 1), (LPBYTE)lpPtn, dwPtnSize);
			if (lpMatchAddr) {
				return lpMatchAddr;
			}
		}
		BaseAddress = (LPVOID)((ULONG)MemoryInformation.BaseAddress + MemoryInformation.RegionSize);
	}
	return NULL;
}

LPVOID SearchExecutableMemory(LPVOID lpAddress, LPVOID lpPattern, ULONG ulPatternSize)
{
	LPVOID lpMatchAddr = NULL;
	MEMORY_BASIC_INFORMATION MemoryInformation;
	LPVOID BaseAddress = lpAddress;

	while (TRUE) {
		if (0 != VirtualQuery((LPVOID)BaseAddress, &MemoryInformation, sizeof(MEMORY_BASIC_INFORMATION))) 
		{
			if (MemoryInformation.State == MEM_COMMIT && MemoryInformation.Protect & 0xf0) 
			{
				lpMatchAddr = BooyerSearch((LPBYTE)BaseAddress, MemoryInformation.RegionSize - ((ULONG)BaseAddress - (ULONG)MemoryInformation.BaseAddress + 1), (LPBYTE)lpPattern, ulPatternSize);
				//lpMatchAddr = QS((char*)BaseAddress, MemoryInformation.RegionSize - ((ULONG)BaseAddress - (ULONG)MemoryInformation.BaseAddress), (char*)lpPattern, ulPatternSize);
				if (lpMatchAddr) {
					return lpMatchAddr;
				}
			}
			BaseAddress =(LPVOID) ((ULONG)MemoryInformation.BaseAddress + MemoryInformation.RegionSize);
		}else{
			DWORD dwErr = GetLastError();
			if (dwErr == ERROR_INVALID_PARAMETER)
			{
				return NULL;
			}
		}
	}
	return NULL;
}