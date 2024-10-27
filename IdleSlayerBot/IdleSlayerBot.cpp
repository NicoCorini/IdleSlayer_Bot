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

int main() {

    // Imposta la posizione e le dimensioni della finestra del terminale
    setConsolePositionAndSize(0, 0, 800, 250);

    gameHWND = findGameWindow();

    if (!gameHWND) {
        std::cerr << "[" << getCurrentTimestamp() << "] Finestra 'Idle Slayer' non trovata!" << std::endl;
        return -1;
    }

    // Imposta il focus sulla finestra del gioco
    SetForegroundWindow(gameHWND);

    // Mouse al centro
    moveMouseToCenter(gameHWND);

   
    std::thread bot(jumpBoostThread);

    std::srand(std::time(nullptr));  // Inizializza il random seed
    botRunning = true;


    std::cout << "[" << getCurrentTimestamp() << "] Premi 'q' per uscire dal bot" << std::endl;

    auto lastBuyEquipmentTime = std::chrono::steady_clock::now();
    auto lastCollectMinionTime = std::chrono::steady_clock::now();

    while (botRunning) {
    
        // Controlla se l'utente ha premuto 'q' per terminare il bot
        //
        if (GetAsyncKeyState('Q') & 0x8000) {
            botRunning = false;
            std::cout << "[" << getCurrentTimestamp() << "] Bot terminato!" << std::endl;
        }

        // Gestione stato pausa
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

        // Vero bot loop
        //
        if (!paused) {

            auto currentTime = std::chrono::steady_clock::now();
            
            // Chest Hunt
            //
            if (isRectangleColor(gameHWND, 318, 515, 318, 515, 0x3568a7, 10) &&
                isRectangleColor(gameHWND, 410, 420, 410, 420, 0x3568a7, 10) &&
                isRectangleColor(gameHWND, 649, 688, 649, 688, 0xffffff, 2)) {

				jumpState = false;
                chestHunt();
                jumpState = true;
            
            }


            // Bonus Stage
            //
			if (isRectangleColor(gameHWND, 660, 254, 660, 254, 0xFFE737) &&
                isRectangleColor(gameHWND, 638, 236, 638, 236, 0xFFBB31) &&
                isRectangleColor(gameHWND, 775, 448, 775, 448, 0xFFFFFF)) {

                jumpState = false;
                bonusStage();
                jumpState = true;

			}

            // Silver Box Collect
            // todo: rivedere con anche bonus da tempo offline
            //
            if (isRectangleColor(gameHWND, 647, 52, 647, 52, 0xffffff)) {
                std::cout << "[" << getCurrentTimestamp() << "] Colleziono silver box" << std::endl;
                mouseClick(gameHWND, 0, 647, 52);
            }


            if (currentTime - lastBuyEquipmentTime >= std::chrono::seconds(45) ||
                isRectangleColor(gameHWND, 1138, 642, 1138, 642, 0x44cdcd, 10)) {

                // Buy Equipment Se si illumina di giallo
                //
                //
                jumpState = false;
                buyEquipment();
                jumpState = true;

                lastBuyEquipmentTime = std::chrono::steady_clock::now();
            }

            if (currentTime - lastCollectMinionTime >= std::chrono::minutes(30) ||
                isRectangleColor(gameHWND, 81, 84, 81, 84, 0x628d66, 10)) {
            
                // Claim Minion se si illumina di giallo
                //
                //
                jumpState = false;
                collectMinion();
                jumpState = true;

                lastCollectMinionTime = std::chrono::steady_clock::now();
            }

        }

        
        
        // Pausa per ridurre l'utilizzo della CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }

    // Disattiva il salto
    jumpState = false;

    // Ferma il bot
    botRunning = false;
    bot.join();


    return 0;
}

