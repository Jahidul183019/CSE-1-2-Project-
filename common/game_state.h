#pragma once

void resetGameProgress();
int getCurrentFloor();
void setCurrentFloor(int floor);
void advanceToNextFloor();
bool isFloorUnlocked(int floor);

// Challenge status setters
void setTetrisSolved(bool solved);
void setCircuitSolved(bool solved);
void setProjectionSolved(bool solved);

// Challenge status getters
bool isTetrisSolved();
bool isCircuitSolved();
bool isProjectionSolved();

extern bool rsaSolved;