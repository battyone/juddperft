#include "movegen.h"
#include "fen.h"
#include "search.h"
#include "winboard.h"
#include "Diagnostics.h"
#include "hashtable.h"

#include <stdio.h>
#include <intrin.h>

#include <atomic>
#include "Juddperft.h"

// Globals:
Engine TheEngine;
#ifdef _USE_HASH
HashTable <std::atomic<PerftTableEntry>> PerftTable("Perft table");
HashTable <std::atomic<LeafEntry>> LeafTable("Leaf Node Table");
#endif
//

int main(int argc, char *argv[], char *envp[])
{

#ifdef _USE_HASH
	unsigned __int64 nBytesToAllocate = 6'000'000'000i64; // 6 GiBytes
#ifdef _WIN64
	MEMORYSTATUSEX statex;
	GlobalMemoryStatusEx(&statex);
	nBytesToAllocate = statex.ullAvailPhys; // Take all avail physical memory !
	printf_s("Available Physical RAM: %I64d\n\n", nBytesToAllocate);
#endif

	while (!SetMemory(nBytesToAllocate)) {
		nBytesToAllocate >>= 1;	// Progressively halve until acceptable size found
		if (nBytesToAllocate < MINIMAL_HASHTABLE_SIZE)
			return EXIT_FAILURE;	// not going to end well ...
	}
#endif
	SetProcessPriority();

	// RunTestSuite();

	/*printf("sizeof(PerftTableEntry) == %zd\n", sizeof(PerftTableEntry));
	printf("sizeof(std::atomic<PerftTableEntry>) == %zd\n", sizeof(std::atomic<PerftTableEntry>));
	printf("sizeof(ChessMove) == %zd\n", sizeof(ChessMove));
	printf("sizeof(std::atomic<LeafEntry>) == %zd\n", sizeof(std::atomic<LeafEntry>));*/

	//ChessPosition P;
	//DumpChessPosition(P);
	//P.SetupStartPosition();
	//FindPerftBug(&P, 8);

	WinBoard(&TheEngine);
	return EXIT_SUCCESS;
}

bool SetMemory(unsigned __int64 nTotalBytes) {
#ifdef _USE_HASH
	// constraint: Leaf Table should have 3 times as many Entries as PerftTable (ie 3:1 ratio)
	
	unsigned __int64 BytesForPerftTable = (nTotalBytes * sizeof(std::atomic<PerftTableEntry>)) /
		(sizeof(std::atomic<PerftTableEntry>) + 3 * sizeof(std::atomic<LeafEntry>));

	unsigned __int64 BytesForLeafTable = (nTotalBytes * 3 * sizeof(std::atomic<LeafEntry>)) /
		(sizeof(std::atomic<PerftTableEntry>) + 3 * sizeof(std::atomic<LeafEntry>));

	return	(PerftTable.SetSize(BytesForPerftTable) && LeafTable.SetSize(BytesForLeafTable));
#else
	return false;
#endif
}

void SetProcessPriority()
{
	DWORD dwError;

	if (!SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS)){
		dwError = GetLastError();
		printf_s("Failed to set Process priority (%d)\n", dwError);
	}
}