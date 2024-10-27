#pragma once
// GlobalVariables.h
#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H

#include <Windows.h>
#include <atomic>

extern HWND gameHWND;
extern std::atomic<bool> botRunning;
extern std::atomic<bool> paused;
extern std::atomic<bool> jumpState;

#endif // GLOBALVARIABLES_H