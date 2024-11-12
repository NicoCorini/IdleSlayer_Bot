#include <Windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip> 
#include <sstream> 
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <string>
#include <vector>
#include <cstdint> // per uint32_t
#include "interactionUtils.h"
#include "GlobalVariables.h"
#include "IdleSlayerTasks.h"

HWND gameHWND = NULL;


std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> botRunning(true);
std::atomic<bool> paused(false);
std::atomic<bool> jumpState(true);


void setConsolePositionAndSize(int x, int y, int width, int height);

void setConsolePositionAndSize(int x, int y, int width, int height) {
	HWND consoleWindow = GetConsoleWindow(); // Ottieni l'handle della finestra del terminale
	MoveWindow(consoleWindow, x, y, width, height, TRUE); // Muovi la finestra
}

void botLoop() {


	// Chest Hunt
	//
	if (isRectangleColor(gameHWND, 318, 515, 318, 515, 0x3568a7, 10) &&
		isRectangleColor(gameHWND, 410, 420, 410, 420, 0x3568a7, 10) &&
		isRectangleColor(gameHWND, 649, 688, 649, 688, 0xffffff, 2))
	{

		jumpState = false;
		chestHunt();
		jumpState = true;

	}

	// Bonus Stage TODO: testare
	// 
	if (isRectangleColor(gameHWND, 648, 232, 648, 232, 0x31bbff, 0) &&
		isRectangleColor(gameHWND, 634, 308, 634, 308, 0xFFFFFF, 0))
	{

		jumpState = false;
		bonusStage();
		jumpState = true;

	}

	// Silver Box Collect
	// TODO: rivedere con anche bonus da tempo offline
	//
	if (isRectangleColor(gameHWND, 647, 52, 647, 52, 0xffffff)) {
		std::cout << "[" << getCurrentTimestamp() << "] Colleziono silver box" << std::endl;
		mouseClick(gameHWND, 0, 647, 52);
	}

	// Negozio si illumina di giallo
	//
	if (isRectangleColor(gameHWND, 1138, 642, 1138, 642, 0x44cdcd, 10))
	{
		// Apre la finestra del negozio
		mouseClick(gameHWND, 0, 1163, 655);
		std::this_thread::sleep_for(std::chrono::milliseconds(150));


		// Premo sulla 4 tab (per questione colore)
		mouseClick(gameHWND, 0, 1081, 690);
		std::this_thread::sleep_for(std::chrono::milliseconds(150));

		// Controllo per un secondo se le quest sono da claimare
		//
		auto start = std::chrono::steady_clock::now();
		auto duration = std::chrono::seconds(3);
		// 
		while (std::chrono::steady_clock::now() - start < duration) {

			// Controllo se Quest da claimare
			if (isRectangleColor(gameHWND, 998, 660, 998, 660, 0x1299c3, 5))
			{
				jumpState = false;

				// Claim Quest
				//
				claimQuests();

				jumpState = true;
			}

			// Sleep per ridurre il carico della CPU
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		
		jumpState = false;

		// Buy Equipment
		//
		buyEquipment();

		jumpState = true;

		// Chiude la finestra del negozio
		mouseClick(gameHWND, 0, 1228, 688);
		std::this_thread::sleep_for(std::chrono::milliseconds(150));


	}


	

	//COLLECT MINION DEPRECATED
	/*
	if (currentTime - lastCollectMinionTime >= std::chrono::minutes(30) ||
		isRectangleColor(gameHWND, 81, 84, 81, 84, 0x628d66, 10))
	{

		// Claim Minion se si illumina di giallo
		//
		jumpState = false;
		collectMinion();
		jumpState = true;

		lastCollectMinionTime = std::chrono::steady_clock::now();
	}
	*/

}

int main()
{
	// Imposta la posizione e le dimensioni della finestra del terminale
	setConsolePositionAndSize(0, 0, 800, 250);

	gameHWND = findGameWindow();
	if (!gameHWND) {
		std::cerr << "[" << getCurrentTimestamp() << "] Finestra 'Idle Slayer' non trovata!" << std::endl;
		std::cin.get();
		return -1;
	}

	// Imposta il focus sulla finestra del gioco
	SetForegroundWindow(gameHWND);

	// Mouse al centro
	moveMouseToCenter(gameHWND);

	// Avvia il thread del bot che salta e spara
	//
	std::thread bot(jumpBoostThread);

	std::srand(std::time(nullptr));  // Inizializza il random seed


	std::cout << "[" << getCurrentTimestamp() << "] Premi 'q' per uscire dal bot" << std::endl;



	while (true)
	{
		// Q press for Exit
		//
		if (GetAsyncKeyState('Q') & 0x8000) {
			std::cout << "[" << getCurrentTimestamp() << "] Bot in chiusura..." << std::endl;
			jumpState = false;
			bot.join();
			break;
		}

		// Pause state
		//
		if (!paused && (GetAsyncKeyState('P') & 0x8000)) {
			paused = !paused;
			std::cout << "[" << getCurrentTimestamp() << "] Pausa " << (paused ? "true" : "false") << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		//
		if (paused && (GetAsyncKeyState('P') & 0x8000) && GetForegroundWindow() == gameHWND) {
			paused = !paused;
			std::cout << "[" << getCurrentTimestamp() << "] Pausa " << (paused ? "true" : "false") << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		if (!paused)
		{
			botLoop();
		}

	}

	std::cout << "[" << getCurrentTimestamp() << "] Bot terminato!" << std::endl;
	std::cin.get();


	return 0;

}

