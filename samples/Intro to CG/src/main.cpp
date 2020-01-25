#include "MemoryTracking.h"
#include "Game.h"
#include "Logging.h"

#include <imgui.h>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <glad/glad.h>

int main() {
	
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	long memBreak = 0;
	if (memBreak) _CrtSetBreakAlloc(memBreak);

	Logger::Init();

	Game* game = new Game();
	game->Run();
	delete game;

	LOG_INFO("Total allocations over run: {}", MemoryTracking::TotalAllocs);
	LOG_INFO("Total bytes allocated over run: {}", MemoryTracking::TotalBytes);

	Logger::Uninitialize();

	_CrtDumpMemoryLeaks();

	return 0;
}

