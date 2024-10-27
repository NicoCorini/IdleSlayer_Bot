#pragma once

#ifndef INTERACTIONUTILS_H
#define INTERACTIONUTILS_H

#include "InteractionUtils.h" 
#include "Windows.h"
#include "chrono"
#include <string>
#include <sstream> 
#include <iostream>
#include <iomanip>
#include <thread>

// Prototipi delle funzioni (dichiarazioni)
std::string getCurrentTimestamp();
HWND findGameWindow();
void moveMouseToCenter(HWND hwnd);
bool isColorClose(COLORREF pixelColor, COLORREF targetColor, int tolerance);
bool isRectangleColor(HWND hwnd, int x1, int y1, int x2, int y2, COLORREF color, int tolerance = 0);
void mouseClick(HWND hwnd, int button, int x, int y, int clicks = 1, int speed = 20);
void mouseMove(HWND hwnd, int x, int y);
void mouseClickDrag(HWND hwnd, int x1, int y1, int x2, int y2);
void mouseWheelScroll(int scrollAmount);

#endif // INTERACTIONUTILS_H
