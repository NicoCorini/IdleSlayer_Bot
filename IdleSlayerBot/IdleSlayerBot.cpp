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

	// get current time chrono
	auto lastBuyEquipment = std::chrono::steady_clock::now();
	auto lastAscend = std::chrono::steady_clock::now();
	auto lastRage = std::chrono::steady_clock::now();
	

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
			// Ascension every 90 minutes
			//
			if (std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - lastAscend).count() >= 90)
			{
				jumpState = false;
				ascend();
				jumpState = true;
				lastAscend = std::chrono::steady_clock::now();
			}

			// Chest Hunt
			//
			if (isRectangleColor(gameHWND, 318, 515, 318, 515, 0x3568a7, 10) &&
				isRectangleColor(gameHWND, 410, 420, 410, 420, 0x3568a7, 10) &&
				(isRectangleColor(gameHWND, 661, 674, 661, 674, 0xffffff, 2) || isRectangleColor(gameHWND, 661, 674, 661, 674, 0xf2eada, 2))
				)
			{

				jumpState = false;
				chestHunt();
				jumpState = true;

			}

			// Bonus Stage
			// 
			if (isRectangleColor(gameHWND, 648, 232, 648, 232, 0x31bbff, 0) &&
				isRectangleColor(gameHWND, 634, 308, 634, 308, 0xFFFFFF, 0))
			{

				jumpState = false;
				bonusStage();
				jumpState = true;

			}

			// Rage
			//
			if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - lastRage).count() >= 630)
			{
				jumpState = false;
				rage();
				jumpState = true;
				lastRage = std::chrono::steady_clock::now();
			}

			// Silver Box Collect
			// TODO: rivedere con anche bonus da tempo offline
			//
			if (isRectangleColor(gameHWND, 647, 52, 647, 52, 0xffffff)) {
				std::cout << "[" << getCurrentTimestamp() << "] Colleziono silver box" << std::endl;
				mouseClick(gameHWND, 0, 647, 52);
			}

			// Negozio si illumina di giallo, oppure ogni 10 minuti
			//
			if (isRectangleColor(gameHWND, 1138, 642, 1138, 642, 0x44cdcd, 10) || std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - lastBuyEquipment).count() >= 5)
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

				lastBuyEquipment = std::chrono::steady_clock::now();

				jumpState = true;

				// Chiude la finestra del negozio
				mouseClick(gameHWND, 0, 1228, 688);
				std::this_thread::sleep_for(std::chrono::milliseconds(150));


			}
		}

	}

	std::cout << "[" << getCurrentTimestamp() << "] Bot terminato!" << std::endl;
	std::cin.get();


	return 0;

}

