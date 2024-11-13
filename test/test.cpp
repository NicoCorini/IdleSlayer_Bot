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


HWND gameHWND = NULL;

// Prototipi
void sendShootInput();
void sendJumpInput();
void sendBoostInput();
void jumpBoostThread();
void chestHunt();
void bonusStageSlider();
void bonusStage();
void buyEquipment();
void buyUpgrade();
void collectMinion();
std::string getCurrentTimestamp();
HWND findGameWindow();
void moveMouseToCenter(HWND hwnd);
bool isColorClose(COLORREF pixelColor, COLORREF targetColor, int tolerance);
bool isRectangleColor(HWND hwnd, int x1, int y1, int x2, int y2, COLORREF color, int tolerance = 0);
void mouseClick(HWND hwnd, int button, int x, int y, int clicks = 1, int speed = 20);
void mouseMove(HWND hwnd, int x, int y);
void mouseClickDrag(HWND hwnd, int x1, int y1, int x2, int y2);
void mouseWheelScroll(int scrollAmount);



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

void moveMouseToCenter(HWND hwnd) {
    RECT rect;
    if (GetWindowRect(hwnd, &rect)) {
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
bool isColorClose(COLORREF pixelColor, COLORREF targetColor, int tolerance) {
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

bool isRectangleColor(HWND hwnd, int x1, int y1, int x2, int y2, COLORREF color, int tolerance ) {


    // Estrai i componenti del colore BGR
    int b = GetBValue(color);
    int g = GetGValue(color);
    int r = GetRValue(color);


    RECT rect;
    if (GetWindowRect(hwnd, &rect)) {

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
                if (!isColorClose(pixelColor, color, tolerance)) {
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

void mouseClick(HWND hwnd, int button, int x, int y, int clicks, int speed) {

    RECT rect;
    if (GetWindowRect(hwnd, &rect)) {

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

void mouseMove(HWND hwnd, int x, int y) {

    RECT rect;
    if (GetWindowRect(hwnd, &rect)) {

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

void mouseClickDrag(HWND hwnd, int x1, int y1, int x2, int y2) {

    RECT rect;
    if (GetWindowRect(hwnd, &rect)) {

        // Coordinate relative a finestra
        int rx1 = rect.left + x1;
        int ry1 = rect.top + y1;
        int rx2 = rect.left + x2;
        int ry2 = rect.top + y2;


        // Posiziona il mouse usando coordinate assolute per rx1 e ry1
        SetCursorPos(rx1, ry1);
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        Sleep(1000);

        // Muovi il mouse alla seconda posizione rx2, ry2
        SetCursorPos(rx2, ry2);
        Sleep(1000);
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

    }
    else {
        std::cerr << "Impossibile ottenere le dimensioni della finestra." << std::endl;
    }


}

// Funzione per simulare lo scroll della rotellina del mouse
void mouseWheelScroll(int scrollAmount) {

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

void sendJumpInput() {
    // Stampa il messaggio di log con timestamp
    //std::cout << "[" << getCurrentTimestamp() << "] Salto con durata: " << msLunghezzaSalto << " ms" << std::endl;

    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    SendInput(1, &input, sizeof(INPUT));

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


void chestHunt() {

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

            if (isRectangleColor(gameHWND, x + 3, y - 1, x + 3, y - 1, 0x4ebff)) {

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
                mouseClick(gameHWND, 0, saverX + 33, saverY - 23);
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
            mouseClick(gameHWND, 0, x + 33, y - 23);
            std::cout << "[" << getCurrentTimestamp() << "] Apro chest: " << 1 + count << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(4000));


            // Controllo se Chest Hunt è finita
            //
            if (isRectangleColor(gameHWND, 719, 687, 719, 687, 0xb4)) {
                std::cout << "[" << getCurrentTimestamp() << "] Chest Hunt finita!" << std::endl;
                isFinished = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));

                // Clicko tasto per uscire
                //
                mouseClick(gameHWND, 0, 719, 687);

                break;
            }

            // if 2x wait some more
            // TODO: VERIFICARE UNA VOLTA CHE HO POWER UP
            //
            //if (isRectangleColor(gameHWND, 500, 210, 500, 210, 0x00FF00)) {
            //    std::cout << "[" << getCurrentTimestamp() << "] Trovato 2x!" << std::endl;
            //	std::this_thread::sleep_for(std::chrono::milliseconds(2500));
            //}

            // If mimic wait some more
            // TODO: NOT WORKING
            //
            //if (isRectangleColor(gameHWND, 450, 212, 450, 212, 0xf60250, 9)) {
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
void bonusStageSlider() {
    // Top left
    if (isRectangleColor(gameHWND, 443, 560, 443, 560, 0x007E00)) {
        mouseMove(gameHWND, 840, 560);
        mouseClickDrag(gameHWND, 840, 560, 450, 560);
        return;
    }

    // Bottom left
    if (isRectangleColor(gameHWND, 443, 620, 443, 620, 0x007E00)) {
        mouseMove(gameHWND, 840, 620);
        mouseClickDrag(gameHWND, 840, 620, 450, 620);
        return;
    }

    // Top right
    if (isRectangleColor(gameHWND, 850, 560, 850, 560, 0x007E00)) {
        mouseMove(gameHWND, 450, 560);
        mouseClickDrag(gameHWND, 450, 560, 840, 560);
        return;
    }

    // Bottom right
    if (isRectangleColor(gameHWND, 850, 620, 850, 620, 0x007E00)) {
        mouseMove(gameHWND, 450, 620);
        mouseClickDrag(gameHWND, 450, 620, 840, 620);
        return;
    }
}

void bonusStage() {

    // Stampa il messaggio di log con timestamp
    std::cout << "[" << getCurrentTimestamp() << "] Bonus Stage!" << std::endl;

    // Ciclare fino a trovare il pixel bianco
    while (true) {
        bonusStageSlider();
        Sleep(500);
        if (isRectangleColor(gameHWND, 775, 448, 775, 448, 0xFFFFFF)) {
            break; // Esci dal ciclo se trovi il pixel
        }
    }

    Sleep(3900);

    // Controlla se il pixel di stato bonus è presente
    if (isRectangleColor(gameHWND, 454, 91, 454, 91, 0xE1E0E2)) {

        std::cout << "[" << getCurrentTimestamp() << "] Ignoro Bonus Stage" << std::endl;

        // Ciclare fino a quando la fase bonus fallisce
        while (true) {
            Sleep(200);
            if (isRectangleColor(gameHWND, 775, 600, 775, 600, 0xB40000)) {
                mouseClick(0, 0, 721, 577); // Clicca in caso di fallimento
                break; // Esci dal ciclo
            }
        }
    }

}

void buyEquipment() {

    // Stampa il messaggio di log con timestamp
    std::cout << "[" << getCurrentTimestamp() << "] Compro Equipment" << std::endl;

    // Se NEGOZIO APERTO
    if (isRectangleColor(gameHWND, 1203, 687, 1203, 687, 0x1010a6, 5)) {

        // Stampa il messaggio di log con timestamp
        std::cout << "[" << getCurrentTimestamp() << "] Negozio Aperto" << std::endl;

        // Clicca sulla scheda delle armi
        mouseClick(gameHWND, 0, 850, 690);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Stampa il messaggio di log con timestamp
        std::cout << "[" << getCurrentTimestamp() << "] Seleziono prima tab" << std::endl;

        // Clicca su "Acquisto massimo"
        mouseClick(gameHWND, 0, 1205, 631, 4);

        // Stampa il messaggio di log con timestamp
        std::cout << "[" << getCurrentTimestamp() << "] Setto acquisto massimo" << std::endl;

        // Controlla se non c'è la barra di scorrimento (scrollbar)
        if (isRectangleColor(gameHWND, 1257, 340, 1257, 340, 0x11AA23)) {
            // Compra la spada se la barra di scorrimento non è visibile
            mouseClick(gameHWND, 0, 1200, 200);
            std::cout << "[" << getCurrentTimestamp() << "] Non ho trovato barra di scorrimento, compro spada" << std::endl;
        }
        else {
            // Clicca in fondo alla barra di scorrimento
            mouseClick(gameHWND, 0, 1254, 604);
            std::cout << "[" << getCurrentTimestamp() << "] Trovata barra di scorrimento, scorro in fondo" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Compra l'ultimo oggetto
            mouseClick(gameHWND, 0, 1200, 560);
            std::cout << "[" << getCurrentTimestamp() << "] Compro ultimo oggetto" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Clicca in cima alla barra di scorrimento
            mouseClick(gameHWND, 0, 1254, 170, 3);
            std::cout << "[" << getCurrentTimestamp() << "] Torno in alto" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Setto acquisto 50
        mouseClick(gameHWND, 0, 1100, 636, 5);
        std::cout << "[" << getCurrentTimestamp() << "] Setto acquisto a 50" << std::endl;

        // Metto mouse in posizione hover al primo elemento
        //
        mouseMove(gameHWND, 1190, 179);

        // Cerca caselle verdi (indicanti un acquisto possibile)
        while (true) {
            if (!isRectangleColor(gameHWND, 1160, 170, 1160, 170, 0x22a310, 9)) {

                //std::cout << "[" << getCurrentTimestamp() << "] No caselle verdi, scorro" << std::endl;

                // Se non ci sono, scorro verso il basso

                mouseWheelScroll(-1);
                std::this_thread::sleep_for(std::chrono::milliseconds(150));

                // Verifica se la barra di scorrimento non è grigia (fine dello scorrimento)
                if (!isRectangleColor(gameHWND, 1253, 605, 1253, 605, 0xd6d6d6)) {
                    std::cout << "[" << getCurrentTimestamp() << "] Fine lista oggetti, valuto ultimi oggetti della lista" << std::endl;


                    // Click sugli ultimi oggetti della pagina
                    //
                    mouseClick(gameHWND, 0, 1190, 185, 5);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    mouseClick(gameHWND, 0, 1190, 269, 5);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    mouseClick(gameHWND, 0, 1190, 365, 5);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    mouseClick(gameHWND, 0, 1190, 465, 5);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    mouseClick(gameHWND, 0, 1190, 562, 5);

                    break; // Esce dal ciclo se non c'è più la barra di scorrimento
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            else {
                // Se trova una casella verde, clicca su di essa per comprare
                mouseClick(gameHWND, 0, 1160, 170, 5);
                std::cout << "[" << getCurrentTimestamp() << "] Casella verde, compro oggetto" << std::endl;
            }
        }

        buyUpgrade();

    }
    else {
        std::cerr << "[" << getCurrentTimestamp() << "] Negozio non aperto! " << std::endl;
    }

}

void buyUpgrade() {

    // Stampa il messaggio di log con timestamp
    std::cout << "[" << getCurrentTimestamp() << "] Compro Upgrade" << std::endl;

    // Premo su tab Upgrade
    //
    mouseClick(gameHWND, 0, 927, 683);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Stampa il messaggio di log con timestamp
    std::cout << "[" << getCurrentTimestamp() << "] Seleziono seconda tab" << std::endl;

    //Scrollo in alto
    //
    mouseMove(gameHWND, 1254, 173);
    while (isRectangleColor(gameHWND, 1254, 167, 1254, 167, 0xD6D6D6)) {
        mouseWheelScroll(20);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    bool somethingBought = false;
    int y = 170;

    while (true) {

        // Controllo se Random Box Magnet è il prossimo upgrade (per skipparlo)
        //
        //
        if ((isRectangleColor(gameHWND, 846, 207, 846, 207, 0x37e7ff) && isRectangleColor(gameHWND, 880, 207, 880, 207, 0x1bb4f4)) ||
            (isRectangleColor(gameHWND, 845, 305, 845, 305, 0x37e7ff) && isRectangleColor(gameHWND, 880, 305, 880, 305, 0xff78e4))) {

            std::cout << "[" << getCurrentTimestamp() << "] Trovato Random Box Magnet Upgrade, lo salto" << std::endl;

            y += 96;
        }

        // Controllo se casella verde disponibile
        //
        if (!isRectangleColor(gameHWND, 1180, y, 1190, y, 0x22a310, 9)) {

            std::cout << "[" << getCurrentTimestamp() << "] Finiti gli upgrade, esco" << std::endl;

            break;
        }
        else {
            somethingBought = true;

            // Clicco su casella verde
            //
            mouseClick(gameHWND, 0, 1190, y);
            std::cout << "[" << getCurrentTimestamp() << "] Comprato un upgrade" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

    }

    // Se ho comprato qualcosa ricontrollo l'equipaggiamento, altrimenti esco
    //
    if (somethingBought) {

        std::cout << "[" << getCurrentTimestamp() << "] Siccome ho comprato ricontrollo l'equipment" << std::endl;

        buyEquipment();
    }
    else {

        std::cout << "[" << getCurrentTimestamp() << "] Fine compro upgrade" << std::endl;
        mouseClick(gameHWND, 0, 1222, 677);
    }


}

void collectMinion() {

    // Stampa il messaggio di log con timestamp
    std::cout << "[" << getCurrentTimestamp() << "] Colleziono minion" << std::endl;

    // Clicca sul bottone ascensione
    mouseClick(gameHWND, 0, 95, 90);
    std::this_thread::sleep_for(std::chrono::milliseconds(400));

    // Clicca Ascension Tab
    //
    mouseClick(gameHWND, 0, 93, 680);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // Clicca sulla tab minion
    mouseClick(gameHWND, 0, 332, 680);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // Mouse in mezzo alla tab minion
    //
    mouseMove(gameHWND, 311, 421);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    mouseWheelScroll(10);
    std::this_thread::sleep_for(std::chrono::milliseconds(150));



    // Finchè la scrollbar non è in fondo, scorro e vedo se il minion attuale ha tasto verde
    //
    while (!isRectangleColor(gameHWND, 612, 638, 612, 638, 0xffffff)) {

        mouseMove(gameHWND, 498, 190);
        std::this_thread::sleep_for(std::chrono::milliseconds(150));

        // Controllo primo punto possibile casella verde (quello se ce bonus giornaliero)
        //
        if (isRectangleColor(gameHWND, 498, 180, 498, 180, 0x22a310, 5)) {
            mouseClick(0, 498, 180, 2, 200);
            std::cout << "[" << getCurrentTimestamp() << "] Click primo minion" << std::endl;
        }
        else {

            mouseMove(gameHWND, 498, 190);
            std::this_thread::sleep_for(std::chrono::milliseconds(150));

            // Controllo secondo punto possibile (non ce bonus giornaliero)
            //
            if (isRectangleColor(gameHWND, 498, 180, 498, 180, 0x22a310, 5)) {
                mouseClick(0, 498, 180, 2, 200);
                std::cout << "[" << getCurrentTimestamp() << "] Click primo minion" << std::endl;
            }


        }

        mouseWheelScroll(-1);

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }


    std::cout << "[" << getCurrentTimestamp() << "] Fine scrollbar, claimo ultimi minions" << std::endl;

    mouseClick(gameHWND, 0, 500, 244, 2, 200);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    mouseClick(gameHWND, 0, 499, 397, 2, 200);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    mouseClick(gameHWND, 0, 498, 547, 2, 200);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));


    // Controlla se il bonus giornaliero è disponibile
    if (isRectangleColor(gameHWND, 306, 186, 306, 186, 0xffffff)) {

        // Clicca su "Claim All"
        //mouseClick(0, 320, 280);
        //std::this_thread::sleep_for(std::chrono::milliseconds(200));
        // Clicca su "Send All"
        //mouseClick(0, 320, 280);
        //std::this_thread::sleep_for(std::chrono::milliseconds(200));


        // Richiedi il bonus giornaliero
        mouseClick(gameHWND, 0, 306, 186);
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        std::cout << "[" << getCurrentTimestamp() << "] Collezionati minion con Daily Bonus, ricontrollo se posso claimare" << std::endl;

        collectMinion();
    }
    else {
        // Clicca "Exit"
        mouseClick(gameHWND, 0, 570, 694);
    }



}

void claimQuests()
{
    // Stampa il messaggio di log con timestamp
    std::cout << "[" << getCurrentTimestamp() << "] Claimo quests" << std::endl;

    // Se NEGOZIO APERTO
    if (isRectangleColor(gameHWND, 1203, 687, 1203, 687, 0x1010a6, 5)) {

        // Stampa il messaggio di log con timestamp
        std::cout << "[" << getCurrentTimestamp() << "] Negozio Aperto" << std::endl;

        // Clicca sulla scheda delle quest
        mouseClick(gameHWND, 0, 1004, 690);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Stampa il messaggio di log con timestamp
        std::cout << "[" << getCurrentTimestamp() << "] Seleziono terza tab" << std::endl;

        // Clicca in cima alla barra di scorrimento
        mouseClick(gameHWND, 0, 1252, 273, 3);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Controlla se non c'è la barra di scorrimento (scrollbar)
        if (isRectangleColor(gameHWND, 1257, 340, 1257, 340, 0x11AA23)) {

            std::cout << "[" << getCurrentTimestamp() << "] Non ho trovato barra di scorrimento, ERRORE, non dovrebbe esserci il caso" << std::endl;
        }


        // COORDINATE UTILI CLAIM QUEST
        /*
        Coordinate relative: (1193, 278), Colore: 0x15ccd9
        Coordinate relative: (1192, 335), Colore: 0x1010a6
        Coordinate relative: (1192, 449), Colore: 0x1010a6
        Coordinate relative: (1190, 558), Colore: 0x1010a6
        */

        // Metto mouse in posizione hover al primo elemento
        //
        mouseMove(gameHWND, 1193, 278);

        // Cerca caselle verdi (indicanti un acquisto possibile)
        while (true) {
            if (!isRectangleColor(gameHWND, 1193, 278, 1193, 278, 0x22a310, 9)) {

                //std::cout << "[" << getCurrentTimestamp() << "] No caselle verdi, scorro" << std::endl;

                // Se non ci sono, scorro verso il basso

                mouseWheelScroll(-1);
                std::this_thread::sleep_for(std::chrono::milliseconds(150));

                // Verifica se la barra di scorrimento non è grigia (fine dello scorrimento)
                if (!isRectangleColor(gameHWND, 1253, 643, 1253, 643, 0xd6d6d6)) {
                    std::cout << "[" << getCurrentTimestamp() << "] Fine lista oggetti, claimo le ultime quest" << std::endl;

                    // Scrollo bene fino in fondo
                    mouseWheelScroll(-3);
                    std::this_thread::sleep_for(std::chrono::milliseconds(450));

                    // Click sugli ultimi oggetti della pagina
                    //
                    mouseClick(gameHWND, 0, 1192, 335);
                    std::this_thread::sleep_for(std::chrono::milliseconds(400));
                    mouseClick(gameHWND, 0, 1192, 449);
                    std::this_thread::sleep_for(std::chrono::milliseconds(400));
                    mouseClick(gameHWND, 0, 1192, 558);

                    break; // Esce dal ciclo se non c'è più la barra di scorrimento
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            else {
                // Se trova una casella verde, clicca su di essa per comprare
               // mouseClick(gameHWND, 0, 1193, 278, 5);
                std::cout << "[" << getCurrentTimestamp() << "] Casella verde, compro oggetto" << std::endl;
            }
        }

    }
    else {
        std::cerr << "[" << getCurrentTimestamp() << "] Negozio non aperto! " << std::endl;
    }
}


// Funzione per stampare posizione del mouse e colore del pixel
void PrintMousePositionAndColor() {
    POINT p;
    if (GetCursorPos(&p)) {
        RECT rect;
        if (GetWindowRect(gameHWND, &rect)) {
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




int main() {

    gameHWND = findGameWindow(); // Trova la finestra del gioco, cambia il nome se necessario
    if (gameHWND == NULL) {
        std::cerr << "Impossibile trovare la finestra del gioco." << std::endl;
        return 1;
    }

    SetForegroundWindow(gameHWND);

    coordinate(); // Esegui la funzione per stampare le coordinate e il colore

   

    return 0;

}
