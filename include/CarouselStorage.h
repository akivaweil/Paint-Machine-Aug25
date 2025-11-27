#ifndef CAROUSEL_STORAGE_H
#define CAROUSEL_STORAGE_H

#include <Arduino.h>

//* ************************************************************************
//* ************************ CAROUSEL STORAGE SYSTEM ************************
//* ************************************************************************

class CarouselStorage {
private:
    // Storage grid: 6 columns × 6 rows = 36 positions
    // Each position can be either occupied (true) or empty (false)
    bool storageGrid[6][6];
    
    // Track total occupied and empty positions
    uint8_t occupiedCount;
    uint8_t emptyCount;
    
    // Position validation
    bool isValidPosition(uint8_t column, uint8_t row) const;

public:
    // Constructor
    CarouselStorage();
    
    // Core functionality
    bool isPositionOccupied(uint8_t column, uint8_t row) const;
    bool setPositionOccupied(uint8_t column, uint8_t row);
    bool setPositionEmpty(uint8_t column, uint8_t row);
    bool togglePosition(uint8_t column, uint8_t row);
    
    // Status queries
    uint8_t getOccupiedCount() const;
    uint8_t getEmptyCount() const;
    bool isFull() const;
    bool isEmpty() const;
    float getOccupancyPercentage() const;
    
    // Grid operations
    void clearAll();
    void fillAll();
    void resetToDefault();
    
    // Position finding
    bool findNextEmptyPosition(uint8_t& column, uint8_t& row) const;
    bool findNextOccupiedPosition(uint8_t& column, uint8_t& row) const;
    
    // Grid information
    uint8_t getColumnCount() const;
    uint8_t getRowCount() const;
    uint8_t getTotalPositions() const;
    
    // Debug and status
    void printGridStatus() const;
    String getGridAsString() const;
};

#endif // CAROUSEL_STORAGE_H 