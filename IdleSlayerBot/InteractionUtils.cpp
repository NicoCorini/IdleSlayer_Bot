#include "InteractionUtils.h" 
#include "Windows.h"
#include "chrono"
#include <string>
#include <sstream> 
#include <iostream>
#include <iomanip>
#include <thread>


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

bool isRectangleColor(HWND hwnd, int x1, int y1, int x2, int y2, COLORREF color, int tolerance) {


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