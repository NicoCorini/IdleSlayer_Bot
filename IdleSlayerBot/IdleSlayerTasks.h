#pragma once

#ifndef IDLESLAYERTASKS_H
#define IDLESLAYERTASKS_H

#include "windows.h"
#include "InteractionUtils.h"
#include "GlobalVariables.h"
#include "chrono"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <thread>

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
void claimQuests();
void ascend();

#endif // IDLESLAYERTASKS_H