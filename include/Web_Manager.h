#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include <Arduino.h>

// Initialize the web server
void initWebServer();

// Check if a move has been requested from the web interface
bool isWebMoveRequested();

// Clear the move request flag
void clearWebMoveRequest();

// Get the target X coordinate
float getWebTargetX();

// Get the target Y coordinate
float getWebTargetY();

// Getters for Pick and Place Sequence
float getWebPickX();
float getWebPickY();
float getWebPlaceX();
float getWebPlaceY();
float getWebForkDistance();

#endif
