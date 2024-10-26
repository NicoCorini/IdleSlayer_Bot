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


HWND idleSlayerHwnd = NULL;

struct Message {
    std::string type;
    int value;
};

std::queue<Message> messageQueue;
std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> botRunning(true);
std::atomic<bool> paused(false);
std::atomic<bool> jumpState(true);

// Prototipi delle funzioni

void setConsolePositionAndSize(int x, int y, int width, int height);
std::string getCurrentTimestamp();
HWND findGameWindow();
void moveMouseToCenter();
bool IsColorClose(COLORREF pixelColor, COLORREF targetColor, int tolerance);
bool IsRectangleColor(int x1, int y1, int x2, int y2, COLORREF color, int tolerance);
void sendShootInput();
void sendJumpInput(int msLunghezzaSalto);
void sendBoostInput();
void mouseClick(int button, int x, int y, int clicks, int speed);
void sendMessage(const std::string& type, int value);
void readMessages();
void jumpBoostThread();
void MouseMove(int x, int y);
void MouseClickDrag(int x1, int y1, int x2, int y2);
void MouseWheelScroll(int scrollAmount);
void ChestHunt();
void BonusStageSlider();
void BonusStage();
void BuyEquipment();
void BuyUpgrade();




void setConsolePositionAndSize(int x, int y, int width, int height) {
    HWND consoleWindow = GetConsoleWindow(); // Ottieni l'handle della finestra del terminale
    MoveWindow(consoleWindow, x, y, width, height, TRUE); // Muovi la finestra
}

// Funzione per ottenere il timestamp corrente
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm; // Struttura per memorizzare la data e l'ora

    // Usa localtime_s per ottenere il tempo locale
    localtime_s(&now_tm, &now_c);

    // Calcola i millisecondi
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%H:%M:%S") << "." << std::setw(3) << std::setfill('0') << milliseconds.count(); // Aggiungi i millisecondi
    return oss.str();
}

HWND findGameWindow() {
    return FindWindow(NULL, L"Idle Slayer");
}

void moveMouseToCenter() {
    RECT rect;
    if (GetWindowRect(idleSlayerHwnd, &rect)) {
        // Calcola le coordinate del centro della finestra
        int centerX = (rect.left + rect.right) / 2;
        int centerY = (rect.top + rect.bottom) / 2;

        // Sposta il cursore del mouse
        SetCursorPos(centerX, centerY);
    }
    else {
        std::cerr << "Impossibile ottenere le dimensioni della finestra." << std::endl;
    }
}

// Funzione per verificare se il colore di un pixel è entro una certa tolleranza
bool IsColorClose(COLORREF pixelColor, COLORREF targetColor, int tolerance) {
    int bPixel = GetBValue(pixelColor);
    int gPixel = GetGValue(pixelColor);
    int rPixel = GetRValue(pixelColor);

    int bTarget = GetBValue(targetColor);
    int gTarget = GetGValue(targetColor);
    int rTarget = GetRValue(targetColor);

    // Verifica se le componenti BGR sono entro il range di tolleranza
    return (abs(bPixel - bTarget) <= tolerance &&
        abs(gPixel - gTarget) <= tolerance &&
        abs(rPixel - rTarget) <= tolerance);
}

bool IsRectangleColor(int x1, int y1, int x2, int y2, COLORREF color, int tolerance = 0) {
    

    // Estrai i componenti del colore BGR
    int b = GetBValue(color);
    int g = GetGValue(color);
    int r = GetRValue(color);


    RECT rect;
    if (GetWindowRect(idleSlayerHwnd, &rect)) {

        // Calcola le coordinate relative alla finestra
        int rx1 = rect.left + x1;
        int ry1 = rect.top + y1;

		int rx2 = rect.left + x2;
		int ry2 = rect.top + y2;

        // Assicurati che le coordinate siano valide
        if (rx1 < rect.left || ry1 < rect.top || rx2 > rect.right || ry2 > rect.bottom) {
            std::cerr << "Le coordinate sono fuori dai limiti della finestra." << std::endl;
            return false;
        }

		HDC hdcScreen = GetDC(NULL); // Ottiene il contesto del dispositivo per lo schermo

        if (!hdcScreen) {
            std::cerr << "Impossibile ottenere il contesto del dispositivo." << std::endl;
            return false;
        }

		for (int y = ry1; y <= ry2; y++) {
			for (int x = rx1; x <= rx2; x++) {
				COLORREF pixelColor = GetPixel(hdcScreen, x, y);
                // Se il colore del pixel non è entro la tolleranza, esci
                if (!IsColorClose(pixelColor, color, tolerance)) {
                    ReleaseDC(NULL, hdcScreen); // Rilascia il contesto del dispositivo
                    return false;
                }
			}
		}

		ReleaseDC(NULL, hdcScreen); // Rilascia il contesto del dispositivo
		return true; // Se tutti i pixel corrispondono, restituisci TRUE
	}
	else {
		std::cerr << "Impossibile ottenere le dimensioni della finestra." << std::endl;
		return false;
    }

}

void sendShootInput() {

    // Stampa il messaggio di log con timestamp
    //std::cout << "[" << getCurrentTimestamp() << "] SPARO!" << std::endl;

    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    SendInput(1, &input, sizeof(INPUT));

    // Rilascia il tasto
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}

void sendJumpInput(int msLunghezzaSalto) {

    // Stampa il messaggio di log con timestamp
    std::cout << "[" << getCurrentTimestamp() << "] Salto con durata: " << msLunghezzaSalto << " ms" << std::endl;

    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));

    // Tieni premuto per la durata del salto inviando MOUSEEVENTF_LEFTDOWN ripetutamente
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < std::chrono::milliseconds(msLunghezzaSalto)) {
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &input, sizeof(INPUT));
        std::this_thread::sleep_for(std::chrono::milliseconds(5)); // intervallo di invio dell'input
    }

    // Rilascia il tasto
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));

}

void sendBoostInput() {

    //std::cout << "[" << getCurrentTimestamp() << "] Boost attivato!" << std::endl;

    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;

    SendInput(1, &input, sizeof(INPUT));

    input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    SendInput(1, &input, sizeof(INPUT));

}

void mouseClick(int button, int x, int y, int clicks = 1, int speed = 15) {

    RECT rect;
    if (GetWindowRect(idleSlayerHwnd, &rect)) {

        // Calcola le coordinate del centro della finestra
        int relativeX = rect.left + x;
        int relativeY = rect.top + y;

        // Sposta il cursore del mouse
        SetCursorPos(relativeX, relativeY);

        INPUT input = { 0 };
        input.type = INPUT_MOUSE;

        // Determina quale bottone utilizzare
        if (button == 0) { // 0 per il pulsante sinistro
            input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        }
        else if (button == 1) { // 1 per il pulsante destro
            input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        }

        for (int i = 0; i < clicks; ++i) {
            // Simula il clic
            SendInput(1, &input, sizeof(INPUT));

            // Rilascia il tasto
            if (button == 0) {
                input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
            }
            else if (button == 1) {
                input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
            }
            SendInput(1, &input, sizeof(INPUT));

            // Aspetta il tempo specificato tra i clic
            if (speed > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(speed));
            }

            // Ripristina il flag per il pulsante
            input.mi.dwFlags = (button == 0) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
        }

    }
    else {
        std::cerr << "Impossibile ottenere le dimensioni della finestra." << std::endl;
    }



}

void sendMessage(const std::string& type, int value) {
    std::lock_guard<std::mutex> lock(mtx);
    messageQueue.push({ type, value });
    cv.notify_one(); // Risveglia il thread del bot se è in attesa

	// Stampa il messaggio di log con timestamp
	std::cout << "[" << getCurrentTimestamp() << "] Messaggio inviato: " << type << " - " << value << std::endl;
}

void readMessages() {
    while (botRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::unique_lock<std::mutex> lock(mtx);
        if (!messageQueue.empty()) {
            // Elimina i messaggi dalla coda e aggiorna le variabili
            while (!messageQueue.empty()) {
                Message msg = messageQueue.front();
                messageQueue.pop();

                if (msg.type == "JumpState") {
                    jumpState = msg.value == 1; // Considera 1 come true
                    std::cout << "JumpState cambiato a: " << jumpState << std::endl;
                }
            }
        }
    }
}

void jumpBoostThread() {
    while (botRunning) {
        if (!paused) {
            if (jumpState) {
                // Simula il comando di salto e sparo
                //std::cout << "[" << getCurrentTimestamp() << "] Boost, Salto e Sparo" << std::endl;

                // Mouse al centro
                moveMouseToCenter();

                // Invia il comando di boost, salto e sparo
                sendBoostInput();
                sendJumpInput(rand() % 100 + 1950);

                // Per la durata del salto premi il mouse un numero umano di volte (spam)
                for (int i = 0; i < 3; i += 1) {
                    sendShootInput();
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }

                // Aspetta 5 secondi per prossimo salto
                //
				std::this_thread::sleep_for(std::chrono::milliseconds(5000));

            }
            else {
                // Se non è in stato di salto, aspetta prima di controllare di nuovo
                std::this_thread::sleep_for(std::chrono::milliseconds(700));
            }
        } 
    }
}

void MouseMove(int x, int y) {

    RECT rect;
    if (GetWindowRect(idleSlayerHwnd, &rect)) {

		// Calcola le coordinate relative alla finestra
		int rx = rect.left + x;
		int ry = rect.top + y;

		// Sposta il cursore del mouse
		SetCursorPos(rx, ry);
    }
    else {
        std::cerr << "Impossibile ottenere le dimensioni della finestra." << std::endl;
    }

}

void MouseClickDrag(int x1, int y1, int x2, int y2) {
    
    RECT rect;
    if (GetWindowRect(idleSlayerHwnd, &rect)) {

        // Coordinate relative a finestra
		int rx1 = rect.left + x1;
		int ry1 = rect.top + y1;
		int rx2 = rect.left + x2;
		int ry2 = rect.top + y2;


        // Simula un click del mouse e un movimento da start a end
        mouse_event(MOUSEEVENTF_LEFTDOWN, rx1, ry1, 0, 0);
        Sleep(1000); // Aggiungi una breve pausa
        mouse_event(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, rx2, ry2, 0, 0);
		Sleep(1000); // Aggiungi una breve pausa
        mouse_event(MOUSEEVENTF_LEFTUP, rx2, ry2, 0, 0);

    }
    else {
        std::cerr << "Impossibile ottenere le dimensioni della finestra." << std::endl;
    }

    
}

// Funzione per simulare lo scroll della rotellina del mouse
void MouseWheelScroll(int scrollAmount) {

	scrollAmount *= 120; // Converti il valore in "clicks" di rotellina del mouse

    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_WHEEL;
    input.mi.mouseData = scrollAmount;

    UINT result = SendInput(1, &input, sizeof(INPUT));
    if (result == 0) {
        std::cerr << "SendInput failed with error: " << GetLastError() << std::endl;
    }
}

void ChestHunt() {

    // Stampa il messaggio di log con timestamp
    std::cout << "[" << getCurrentTimestamp() << "] Chest Hunt!" << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(4000));


    int saverX = 0;
    int saverY = 0;

    bool saverFound = false;
    
	int x = 185;
	int y = 325;

    // Cerco il saver
    //
    std::cout << "[" << getCurrentTimestamp() << "] Cerco Saver" << std::endl;

    for (int i = 1; i <= 3 && !saverFound; i++) {
        for (int j = 1; j <= 10 && !saverFound; j++) {

            std::cout << "[" << getCurrentTimestamp() << "] Controllo chest: " << i << ", " << j << std::endl;

            if (IsRectangleColor(x + 3, y - 1, x + 3, y - 1, 0x4ebff)) {

                saverX = x;
                saverY = y;

                std::cout << "[" << getCurrentTimestamp() << "] Trovato Saver in posizione: " << i << ", " << j << std::endl;
				saverFound = true;
                break;
            }
            
            x += 95;
            

        }
        y += 95;
        x = 185;
    }

	if (saverX == 0 && saverY == 0) {
		std::cout << "[" << getCurrentTimestamp() << "] Saver non trovato!" << std::endl;
	}

    // Actual Chest Hunt
    //
    x = 185;
    y = 325;
    int count = 0;
    bool isFinished = false;

    for (int i = 1; i <= 3 && !isFinished; i++) {
        for (int j = 1; j <= 10 && !isFinished; j++) {
            
            // Utilizzo Saver
            //
            if (count == 1 && saverX > 0) {
				mouseClick(0, saverX + 33, saverY - 23);
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            }

            // Salto Saver
            //
			if (x == saverX && y == saverY) {
				
                //Prossima riga se saver è l'ultima chest
                //
                if (j == 10) {
                    break;
                }
                else {
					x += 95;
					continue;
                }

			}

            // Apro Chest
            //
			mouseClick(0, x + 33, y - 23);
            std::cout << "[" << getCurrentTimestamp() << "] Apro chest: " << 1 + count << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(4000));


            // Controllo se Chest Hunt è finita
            //
			if (IsRectangleColor(719, 687, 719, 687, 0xb4)) {
				std::cout << "[" << getCurrentTimestamp() << "] Chest Hunt finita!" << std::endl;
				isFinished = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));

                // Clicko tasto per uscire
                //
                mouseClick(0, 719, 687);

                break;
			}

            // if 2x wait some more
			// TODO: VERIFICARE UNA VOLTA CHE HO POWER UP
            //
            //if (IsRectangleColor(500, 210, 500, 210, 0x00FF00)) {
            //    std::cout << "[" << getCurrentTimestamp() << "] Trovato 2x!" << std::endl;
			//	std::this_thread::sleep_for(std::chrono::milliseconds(2500));
            //}

            // If mimic wait some more
            // TODO: NOT WORKING
            //
			//if (IsRectangleColor(450, 212, 450, 212, 0xf60250, 9)) {
            //    std::cout << "[" << getCurrentTimestamp() << "] Trovato Mimic!" << std::endl;
			//	std::this_thread::sleep_for(std::chrono::milliseconds(2500));
			//}
            

			x += 95;
			count += 1;
        }

		y += 95;
		x = 185;
    }



	

    return;
}

// Funzione Slider
void BonusStageSlider() {
    // Top left
    if (IsRectangleColor(443, 560, 443, 560, 0x007E00)) {
        MouseMove(840, 560);
        MouseClickDrag(840, 560, 450, 560);
        return;
    }

    // Bottom left
    if (IsRectangleColor(443, 620, 443, 620, 0x007E00)) {
        MouseMove(840, 620);
        MouseClickDrag(840, 620, 450, 620);
        return;
    }

    // Top right
    if (IsRectangleColor(850, 560, 850, 560, 0x007E00)) {
        MouseMove(450, 560);
        MouseClickDrag(450, 560, 840, 560);
        return;
    }

    // Bottom right
    if (IsRectangleColor(850, 620, 850, 620, 0x007E00)) {
        MouseMove(450, 620);
        MouseClickDrag(450, 620, 840, 620);
        return;
    }
}

void BonusStage() {

    // Stampa il messaggio di log con timestamp
    std::cout << "[" << getCurrentTimestamp() << "] Bonus Stage!" << std::endl;

    // Ciclare fino a trovare il pixel bianco
    while (true) {
        BonusStageSlider();
        Sleep(500);
        if (IsRectangleColor(775, 448, 775, 448, 0xFFFFFF)) {
            break; // Esci dal ciclo se trovi il pixel
        }
    }

    Sleep(3900);

    // Controlla se il pixel di stato bonus è presente
    if (IsRectangleColor(454, 91, 454, 91, 0xE1E0E2)) {     

		std::cout << "[" << getCurrentTimestamp() << "] Ignoro Bonus Stage" << std::endl;

        // Ciclare fino a quando la fase bonus fallisce
        while (true) {
            Sleep(200);
            if (IsRectangleColor(775, 600, 775, 600, 0xB40000)) {
                mouseClick(0, 0, 721, 577); // Clicca in caso di fallimento
				break; // Esci dal ciclo
            }
        }
    }

}

// Funzione per ottenere le coordinate dei pixel con un colore specifico
std::vector<POINT> GetColorCoordinates(int x1, int y1, int x2, int y2, COLORREF color) {

    std::vector<POINT> result;

    // Ottieni il DC della finestra
    HDC hdc = GetDC(idleSlayerHwnd);
    if (!hdc) {
        return result; // Se non riusciamo a ottenere il DC, ritorna un vettore vuoto
    }

    // Scorri attraverso i pixel nel rettangolo
    for (int y = y1; y <= y2; ++y) {
        for (int x = x1; x <= x2; ++x) {
            // Ottieni il colore del pixel
            COLORREF pixelColor = GetPixel(hdc, x, y);

            // Controlla se il colore corrisponde
            if (pixelColor == color) {
                result.push_back({ x, y });
            }
        }
    }

    // Rilascia il DC
    ReleaseDC(idleSlayerHwnd, hdc);
    return result; // Restituisci le coordinate trovate
}

void BuyEquipment() {

    // Stampa il messaggio di log con timestamp
    std::cout << "[" << getCurrentTimestamp() << "] Compro Equipment" << std::endl;

    // Chiude la finestra del negozio se è aperta
    mouseClick(0, 1244, 712);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // Apre la finestra del negozio
    mouseClick(0, 1163, 655);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // Se NEGOZIO APERTO
    if (IsRectangleColor(807, 142, 807, 142, 0xFFFFFF)) {

        // Stampa il messaggio di log con timestamp
        std::cout << "[" << getCurrentTimestamp() << "] Negozio Aperto" << std::endl;

        // Clicco sulla scheda quest giusto per togliere il flash giallo se una è completa
        // TODO: Implementare quest claiming
        //
        mouseClick(0, 1010, 684);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Clicca sulla scheda dell'armatura
        mouseClick(0, 850, 690);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Stampa il messaggio di log con timestamp
        std::cout << "[" << getCurrentTimestamp() << "] Seleziono prima tab" << std::endl;

        // Clicca su "Acquisto massimo"
        mouseClick(0, 1205, 631, 4);

        // Stampa il messaggio di log con timestamp
        std::cout << "[" << getCurrentTimestamp() << "] Setto acquisto massimo" << std::endl;

        // Controlla se non c'è la barra di scorrimento (scrollbar)
        if (IsRectangleColor(1257, 340, 1257, 340, 0x11AA23)) {
            // Compra la spada se la barra di scorrimento non è visibile
            mouseClick(0, 1200, 200);
            std::cout << "[" << getCurrentTimestamp() << "] Non ho trovato barra di scorrimento, compro spada" << std::endl;
        }
        else {
            // Clicca in fondo alla barra di scorrimento
            mouseClick(0, 1254, 604);
            std::cout << "[" << getCurrentTimestamp() << "] Trovata barra di scorrimento, scorro in fondo" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Compra l'ultimo oggetto
            mouseClick(0, 1200, 560);
            std::cout << "[" << getCurrentTimestamp() << "] Compro ultimo oggetto" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Clicca in cima alla barra di scorrimento
            mouseClick(0, 1254, 170, 3);
            std::cout << "[" << getCurrentTimestamp() << "] Torno in alto" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Setto acquisto 50
        mouseClick(0, 1100, 636, 5);
        std::cout << "[" << getCurrentTimestamp() << "] Setto acquisto a 50" << std::endl;

        // Metto mouse in posizione hover al primo elemento
        //
		MouseMove(1190, 179);

        // Cerca caselle verdi (indicanti un acquisto possibile)
        while (true) {
            if (!IsRectangleColor(1160, 170, 1160, 170, 0x22a310, 9)) {

                //std::cout << "[" << getCurrentTimestamp() << "] No caselle verdi, scorro" << std::endl;

                // Se non ci sono, scorro verso il basso

                MouseWheelScroll(-1);
                std::this_thread::sleep_for(std::chrono::milliseconds(150));

                // Verifica se la barra di scorrimento non è grigia (fine dello scorrimento)
                if (!IsRectangleColor(1253, 605, 1253, 605, 0xd6d6d6)) {
                    std::cout << "[" << getCurrentTimestamp() << "] Fine lista oggetti, valuto ultimi oggetti della lista" << std::endl;
                    
                    
                    // Click sugli ultimi oggetti della pagina
                    //
					mouseClick(0, 1190, 185, 5);
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					mouseClick(0, 1190, 269, 5);
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					mouseClick(0, 1190, 365, 5);
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					mouseClick(0, 1190, 465, 5);
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					mouseClick(0, 1190, 562, 5);

                    break; // Esce dal ciclo se non c'è più la barra di scorrimento
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            else {
                // Se trova una casella verde, clicca su di essa per comprare
                mouseClick(0, 1160, 170, 5);
                std::cout << "[" << getCurrentTimestamp() << "] Casella verde, compro oggetto" << std::endl;
            }
        }

		BuyUpgrade();

	}
	else {
        std::cerr << "[" << getCurrentTimestamp() << "] Negozio non aperto! " << std::endl;
	}

}

void BuyUpgrade() {

    // Stampa il messaggio di log con timestamp
    std::cout << "[" << getCurrentTimestamp() << "] Compro Upgrade" << std::endl;

	// Premo su tab Upgrade
    //
    mouseClick(0, 927, 683);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

    //Scrollo in alto
    //
    MouseMove(1254, 173);
    while (IsRectangleColor(1254, 167, 1254, 167, 0xD6D6D6)) {
        MouseWheelScroll(20);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

	bool somethingBought = false;
    int y = 170;

    while (true) {

        // Controllo se Random Box Magnet è il prossimo upgrade (per skipparlo)
        //
        //
        if (IsRectangleColor(882, y, 909, y + 72, 0xF4B41B) || IsRectangleColor(882, y, 909, y + 72, 0xE478FF)) {
            
            std::cout << "[" << getCurrentTimestamp() << "] Trovato Random Box Magnet Upgrade, lo salto" << std::endl;
            
            y += 96;
        }

        // Controllo se casella verde disponibile
        //
        if (!IsRectangleColor(1180, y, 1190, y, 0x22a310, 9)) {

            std::cout << "[" << getCurrentTimestamp() << "] Finiti gli upgrade, esco" << std::endl;

            break;
        }
        else {
            somethingBought = true;

			// Clicco su casella verde
            //
            mouseClick(0, 1190, y);
            std::cout << "[" << getCurrentTimestamp() << "] Comprato un upgrade" << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

    }

    // Se ho comprato qualcosa ricontrollo l'equipaggiamento, altrimenti esco
    //
    if (somethingBought) {

         std::cout << "[" << getCurrentTimestamp() << "] Siccome ho comprato ricontrollo l'equipment" << std::endl;

		BuyEquipment();
    }
    else {

        std::cout << "[" << getCurrentTimestamp() << "] Fine compro upgrade" << std::endl;
        mouseClick(0, 1222, 677);
    }


}

void CollectMinion() {

    // Stampa il messaggio di log con timestamp
    std::cout << "[" << getCurrentTimestamp() << "] Colleziono minion" << std::endl;

    // Clicca sul bottone ascensione
    mouseClick(0, 95, 90);
    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    // Clicca Ascension Tab
    //
    mouseClick(0, 93, 680);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Clicca sulla tab minion
    mouseClick(0, 332, 680);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    MouseMove(498, 180);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    MouseWheelScroll(5);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));



    // Finchè la scrollbar non è in fondo, scorro e vedo se il minion attuale ha tasto verde
    //
    while (!IsRectangleColor(612, 638, 612, 638, 0xffffff)) {

        if (IsRectangleColor(498, 180, 498, 180, 0x22a310)) {
            mouseClick(0, 498, 180, 2, 200);
            std::cout << "[" << getCurrentTimestamp() << "] Click primo minion" << std::endl;
        }

        MouseWheelScroll(-1);

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }


    std::cout << "Fine scrollbar, claimo ultimi minions" << std::endl;

    mouseClick(0, 500, 244, 2, 200);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    mouseClick(0, 499, 397, 2, 200);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    mouseClick(0, 498, 547, 2, 200);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));


    // Controlla se il bonus giornaliero è disponibile
    if (IsRectangleColor(306, 186, 306, 186, 0xffffff)) {

        // Clicca su "Claim All"
        //mouseClick(0, 320, 280);
        //std::this_thread::sleep_for(std::chrono::milliseconds(200));
        // Clicca su "Send All"
        //mouseClick(0, 320, 280);
        //std::this_thread::sleep_for(std::chrono::milliseconds(200));


        // Richiedi il bonus giornaliero
        mouseClick(0, 306, 186);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        std::cout << "[" << getCurrentTimestamp() << "] Collezionati minion con Daily Bonus, ricontrollo se posso claimare" << std::endl;

        CollectMinion();
    }
    else {
        // Clicca "Exit"
        mouseClick(0, 570, 694);
    }



}


int main() {

    // Imposta la posizione e le dimensioni della finestra del terminale
    setConsolePositionAndSize(0, 0, 800, 250);

    idleSlayerHwnd = findGameWindow();

    if (!idleSlayerHwnd) {
        std::cerr << "[" << getCurrentTimestamp() << "] Finestra 'Idle Slayer' non trovata!" << std::endl;
        return -1;
    }

    // Imposta il focus sulla finestra del gioco
    SetForegroundWindow(idleSlayerHwnd);

    // Mouse al centro
    moveMouseToCenter();

    //ChestHunt();
    //BuyEquipment();

    //CollectMinion();

    //return 0;

    
    std::thread reader(readMessages);
    std::thread bot(jumpBoostThread);

    std::srand(std::time(nullptr));  // Inizializza il random seed
    botRunning = true;

    // Simula invio di messaggi dal thread principale
    //sendMessage("JumpState", 1); // 1 per attivo

    std::cout << "[" << getCurrentTimestamp() << "] Premi 'q' per uscire dal bot" << std::endl;

    auto lastCheckTime = std::chrono::steady_clock::now();

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
        if (paused && (GetAsyncKeyState('P') & 0x8000) && GetForegroundWindow() == idleSlayerHwnd) {
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
            if (IsRectangleColor(318, 515, 318, 515, 0x3568a7, 10) &&
                IsRectangleColor(410, 420, 410, 420, 0x3568a7, 10) &&
                IsRectangleColor(649, 688, 649, 688, 0xffffff, 2)) {

				jumpState = false;
                ChestHunt();
                jumpState = true;
            
            }


            // Bonus Stage
            //
			if (IsRectangleColor(660, 254, 660, 254, 0xFFE737) &&
                IsRectangleColor(638, 236, 638, 236, 0xFFBB31) &&
                IsRectangleColor(775, 448, 775, 448, 0xFFFFFF)) {

                jumpState = false;
                BonusStage();
                jumpState = true;

			}

            // Silver Box Collect
            //
            if (IsRectangleColor(647, 52, 647, 52, 0xffffff)) {
                std::cout << "[" << getCurrentTimestamp() << "] Colleziono silver box" << std::endl;
                mouseClick(0, 647, 52);
            }


            if (currentTime - lastCheckTime >= std::chrono::seconds(10)) {

                //std::cout << "[" << getCurrentTimestamp() << "] Valuto equipment / upgrade / minions / quests" << std::endl;

                // Buy Equipment Se si illumina di giallo
                //
                //
                if (IsRectangleColor(1138, 642, 1138, 642, 0x44cdcd, 10)) {
                    jumpState = false;
                    BuyEquipment();
                    jumpState = true;

                    lastCheckTime = std::chrono::steady_clock::now();
               }

                

            }

        }

        
        
        // Pausa per ridurre l'utilizzo della CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }

    // Disattiva il salto
    jumpState = false;

    // Ferma il bot
    botRunning = false;
    reader.join();
    bot.join();


    return 0;
}

