#include "game_state.h"

// Internal state
static int currentFloor = 1;  // Start at Floor 1

// Floor 2 challenge states
static bool tetrisSolved = false;
static bool circuitSolved = false;
static bool projectionSolved = false;
bool puzzleSolved = false;
bool rsaSolved = false;

void resetGameProgress() {
    currentFloor = 1;
    tetrisSolved = false;
    circuitSolved = false;
    projectionSolved = false;
}

int getCurrentFloor() {
    return currentFloor;
}

void setCurrentFloor(int floor) {
    currentFloor = floor;
}

bool isFloorUnlocked(int floor) {
    return floor <= currentFloor;
}

void advanceToNextFloor() {
    if (currentFloor == 2) {
        // Require all floor 2 challenges before moving to floor 3
        if (tetrisSolved && circuitSolved && projectionSolved) {
            currentFloor = 3;
        }
    } else if (currentFloor < 3) {
        currentFloor++;
    }
}

// Challenge completion setters
void setTetrisSolved(bool solved) {
    tetrisSolved = solved;
}

void setCircuitSolved(bool solved) {
    circuitSolved = solved;
}

void setProjectionSolved(bool solved) {
    projectionSolved = solved;
}

// Challenge completion getters
bool isTetrisSolved() {
    return tetrisSolved;
}

bool isCircuitSolved() {
    return circuitSolved;
}

bool isProjectionSolved() {
    return projectionSolved;
}