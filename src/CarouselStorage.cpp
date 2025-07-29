#include "CarouselStorage.h"

//* ************************************************************************
//* ************************ CAROUSEL STORAGE SYSTEM ************************
//* ************************************************************************

CarouselStorage::CarouselStorage() {
    // Initialize all positions as empty
    clearAll();
}

//* ************************************************************************
//* ************************ PRIVATE METHODS *******************************
//* ************************************************************************

bool CarouselStorage::isValidPosition(uint8_t column, uint8_t row) const {
    // Check if column and row are within valid range (0-5 for 6x6 grid)
    return (column < 6 && row < 6);
}

//* ************************************************************************
//* ************************ CORE FUNCTIONALITY ****************************
//* ************************************************************************

bool CarouselStorage::isPositionOccupied(uint8_t column, uint8_t row) const {
    // Check if position is valid first
    if (!isValidPosition(column, row)) {
        return false; // Invalid position considered empty
    }
    
    return storageGrid[column][row];
}

bool CarouselStorage::setPositionOccupied(uint8_t column, uint8_t row) {
    // Check if position is valid
    if (!isValidPosition(column, row)) {
        return false;
    }
    
    // Only update if position is currently empty
    if (!storageGrid[column][row]) {
        storageGrid[column][row] = true;
        occupiedCount++;
        emptyCount--;
    }
    
    return true;
}

bool CarouselStorage::setPositionEmpty(uint8_t column, uint8_t row) {
    // Check if position is valid
    if (!isValidPosition(column, row)) {
        return false;
    }
    
    // Only update if position is currently occupied
    if (storageGrid[column][row]) {
        storageGrid[column][row] = false;
        occupiedCount--;
        emptyCount++;
    }
    
    return true;
}

bool CarouselStorage::togglePosition(uint8_t column, uint8_t row) {
    // Check if position is valid
    if (!isValidPosition(column, row)) {
        return false;
    }
    
    // Toggle the position
    if (storageGrid[column][row]) {
        // Currently occupied, make it empty
        storageGrid[column][row] = false;
        occupiedCount--;
        emptyCount++;
    } else {
        // Currently empty, make it occupied
        storageGrid[column][row] = true;
        occupiedCount++;
        emptyCount--;
    }
    
    return true;
}

//* ************************************************************************
//* ************************ STATUS QUERIES *******************************
//* ************************************************************************

uint8_t CarouselStorage::getOccupiedCount() const {
    return occupiedCount;
}

uint8_t CarouselStorage::getEmptyCount() const {
    return emptyCount;
}

bool CarouselStorage::isFull() const {
    return (occupiedCount == 36); // 6x6 = 36 total positions
}

bool CarouselStorage::isEmpty() const {
    return (occupiedCount == 0);
}

float CarouselStorage::getOccupancyPercentage() const {
    return (float(occupiedCount) / 36.0) * 100.0; // 36 total positions
}

//* ************************************************************************
//* ************************ GRID OPERATIONS ******************************
//* ************************************************************************

void CarouselStorage::clearAll() {
    // Set all positions to empty
    for (uint8_t col = 0; col < 6; col++) {
        for (uint8_t row = 0; row < 6; row++) {
            storageGrid[col][row] = false;
        }
    }
    
    // Update counters
    occupiedCount = 0;
    emptyCount = 36; // 6x6 = 36 total positions
}

void CarouselStorage::fillAll() {
    // Set all positions to occupied
    for (uint8_t col = 0; col < 6; col++) {
        for (uint8_t row = 0; row < 6; row++) {
            storageGrid[col][row] = true;
        }
    }
    
    // Update counters
    occupiedCount = 36; // 6x6 = 36 total positions
    emptyCount = 0;
}

void CarouselStorage::resetToDefault() {
    // Reset to empty state (same as clearAll)
    clearAll();
}

//* ************************************************************************
//* ************************ POSITION FINDING *****************************
//* ************************************************************************

bool CarouselStorage::findNextEmptyPosition(uint8_t& column, uint8_t& row) const {
    // Search through grid to find next empty position
    for (uint8_t col = 0; col < 6; col++) {
        for (uint8_t row_idx = 0; row_idx < 6; row_idx++) {
            if (!storageGrid[col][row_idx]) {
                column = col;
                row = row_idx;
                return true; // Found empty position
            }
        }
    }
    
    return false; // No empty positions found
}

bool CarouselStorage::findNextOccupiedPosition(uint8_t& column, uint8_t& row) const {
    // Search through grid to find next occupied position
    for (uint8_t col = 0; col < 6; col++) {
        for (uint8_t row_idx = 0; row_idx < 6; row_idx++) {
            if (storageGrid[col][row_idx]) {
                column = col;
                row = row_idx;
                return true; // Found occupied position
            }
        }
    }
    
    return false; // No occupied positions found
}

//* ************************************************************************
//* ************************ GRID INFORMATION *****************************
//* ************************************************************************

uint8_t CarouselStorage::getColumnCount() const {
    return 6;
}

uint8_t CarouselStorage::getRowCount() const {
    return 6;
}

uint8_t CarouselStorage::getTotalPositions() const {
    return 36; // 6x6 = 36 total positions
}

//* ************************************************************************
//* ************************ DEBUG AND STATUS *****************************
//* ************************************************************************

void CarouselStorage::printGridStatus() const {
    // Print grid layout with occupied (X) and empty (O) positions
    for (uint8_t row = 0; row < 6; row++) {
        for (uint8_t col = 0; col < 6; col++) {
            if (storageGrid[col][row]) {
                // Occupied position
                Serial.print("X ");
            } else {
                // Empty position
                Serial.print("O ");
            }
        }
        Serial.println(); // New line after each row
    }
    
    // Print summary
    Serial.print("Occupied: ");
    Serial.print(occupiedCount);
    Serial.print("/36 (");
    Serial.print(getOccupancyPercentage(), 1);
    Serial.println("%)");
}

String CarouselStorage::getGridAsString() const {
    String gridString = "";
    
    // Build string representation of grid
    for (uint8_t row = 0; row < 6; row++) {
        for (uint8_t col = 0; col < 6; col++) {
            if (storageGrid[col][row]) {
                gridString += "X"; // Occupied
            } else {
                gridString += "O"; // Empty
            }
            
            if (col < 5) {
                gridString += " "; // Space between columns
            }
        }
        
        if (row < 5) {
            gridString += "\n"; // New line between rows
        }
    }
    
    return gridString;
} 