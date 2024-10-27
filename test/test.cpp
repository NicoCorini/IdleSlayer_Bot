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

HWND idleSlayerHwnd = NULL;

HWND findGameWindow() {
    return FindWindow(NULL, L"Idle Slayer");
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

void MoveMouseTo(int x, int y) {
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

// Funzione per ottenere le coordinate dei pixel con un colore specifico
std::vector<POINT> GetColorCoordinates(int x1, int y1, int x2, int y2, COLORREF color) {

    std::vector<POINT> result;
    
    // Scorri attraverso i pixel nel rettangolo
    for (int y = y1; y <= y2; ++y) {
        for (int x = x1; x <= x2; ++x) {
            
			if (IsRectangleColor(x1, y1, x2, y2, color, 9)) {
				result.push_back({ x, y });
			}
        }
    }

    return result; // Restituisci le coordinate trovate
 
}

// Funzione per stampare posizione del mouse e colore del pixel
void PrintMousePositionAndColor() {
    POINT p;
    if (GetCursorPos(&p)) {
        RECT rect;
        if (GetWindowRect(idleSlayerHwnd, &rect)) {
            // Calcola le coordinate relative alla finestra
            int rx = p.x - rect.left;
            int ry = p.y - rect.top;

            HDC hdcScreen = GetDC(NULL); // Ottiene il contesto del dispositivo per lo schermo
            if (hdcScreen) {
                COLORREF pixelColor = GetPixel(hdcScreen, p.x, p.y);
                ReleaseDC(NULL, hdcScreen); // Rilascia il contesto del dispositivo

                std::cout << "Coordinate relative: (" << rx << ", " << ry << "), Colore: 0x"
                    << std::hex << pixelColor << std::dec << std::endl;
            }
            else {
                std::cerr << "Impossibile ottenere il contesto del dispositivo." << std::endl;
            }
        }
        else {
            std::cerr << "Impossibile ottenere le dimensioni della finestra." << std::endl;
        }
    }
    else {
        std::cerr << "Impossibile ottenere la posizione del cursore." << std::endl;
    }
}

void mouseClick(int button, int x, int y, int clicks = 1, int speed = 25) {

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

void PrintPoints(const std::vector<POINT>& points) {
    if (points.empty()) {
        std::cout << "Nessun punto trovato con il colore specificato.\n";
        return;
    }

    std::cout << "Punti trovati:\n";
    for (const auto& point : points) {
        std::cout << "x: " << point.x << ", y: " << point.y << "\n";
    }
}

void coordinate() {
    std::cout << "Premi il tasto destro del mouse per stampare le coordinate e il colore.\n";

    // Ciclo infinito per controllare se il tasto destro del mouse viene premuto
    while (true) {
        if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) { // Controlla se il tasto destro è premuto
            PrintMousePositionAndColor();
            Sleep(500); // Pausa per evitare di stampare troppe volte se il tasto è premuto a lungo
        }

        Sleep(10); // Riduce il carico della CPU
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
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // Clicca sulla tab minion
    mouseClick(0, 332, 680);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // Mouse in mezzo alla tab minion
    //
    MouseMove(311, 421);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    MouseWheelScroll(10);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));



    // Finchè la scrollbar non è in fondo, scorro e vedo se il minion attuale ha tasto verde
    //
    while (!IsRectangleColor(612, 638, 612, 638, 0xffffff)) {

        MouseMove(498, 190);
        std::this_thread::sleep_for(std::chrono::milliseconds(150));

        // Controllo primo punto possibile casella verde (quello se ce bonus giornaliero)
        //
        if (IsRectangleColor(498, 180, 498, 180, 0x22a310, 5)) {
            mouseClick(0, 498, 180, 2, 200);
            std::cout << "[" << getCurrentTimestamp() << "] Click primo minion" << std::endl;
        }
        else {

            MouseMove(498, 190);
            std::this_thread::sleep_for(std::chrono::milliseconds(150));

            // Controllo secondo punto possibile (non ce bonus giornaliero)
            //
            if (IsRectangleColor(498, 180, 498, 180, 0x22a310, 5)) {
                mouseClick(0, 498, 180, 2, 200);
                std::cout << "[" << getCurrentTimestamp() << "] Click primo minion" << std::endl;
            }


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
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        std::cout << "[" << getCurrentTimestamp() << "] Collezionati minion con Daily Bonus, ricontrollo se posso claimare" << std::endl;

        CollectMinion();
    }
    else {
        // Clicca "Exit"
        mouseClick(0, 570, 694);
    }



}

void measureKeyPressDuration() {
    // Variabili per la misurazione del tempo
    auto start = std::chrono::steady_clock::now();
    bool keyPressed = false;

    std::cout << "Tieni premuto il tasto freccia in alto...\n";

    // Ciclo per controllare lo stato del tasto
    while (true) {
        // Controlla se il tasto freccia in alto è premuto
        if (GetAsyncKeyState(VK_UP) & 0x8000) {
            if (!keyPressed) {  // Se il tasto non era già stato registrato come premuto
                keyPressed = true;
                start = std::chrono::steady_clock::now(); // Inizia a misurare
            }
        }
        else {
            if (keyPressed) {  // Se il tasto era premuto e ora è stato rilasciato
                keyPressed = false;
                auto end = std::chrono::steady_clock::now(); // Ferma la misurazione
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                std::cout << "Hai tenuto premuto il tasto per " << duration << " millisecondi.\n";
                break; // Esci dal ciclo
            }
        }

        // Piccola pausa per evitare un utilizzo eccessivo della CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {
    idleSlayerHwnd = findGameWindow(); // Trova la finestra del gioco, cambia il nome se necessario
    if (idleSlayerHwnd == NULL) {
        std::cerr << "Impossibile trovare la finestra del gioco." << std::endl;
        return 1;
    }

    

	coordinate(); // Esegui la funzione per stampare le coordinate e il colore


    //std::vector<POINT> points = GetColorCoordinates(1157, 177, 1157, 601, 0x22a310);
    

    //PrintPoints(points);


    return 0;
}
