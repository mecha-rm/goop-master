/*
 * Name: Roderick "R.J." Montague
 * Student Number: 100701758
 * Date: 12/03/2019
 * Description: viewpoint viewer and terrain renderer for assignment 2 and assignment 3 respectively.
	* Assignment 3's terrain is viewed through assignment 2's viewports.
 */  
#include "Game.h"
#include "Logging.h" 

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // checks for memory leaks once the program ends.
	long long allocPoint = 0;
	if (allocPoint)
	{
		_CrtSetBreakAlloc(allocPoint); // sets where you want to stop our program by assigning the allocation block index stopping point.
	}

	Logger::Init();

	Game* game = new Game();
	game->Run();
	delete game;

	Logger::Uninitialize();
	 
	return 0;
}