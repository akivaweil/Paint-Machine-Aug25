#ifndef CAROUSEL_CONFIG_H
#define CAROUSEL_CONFIG_H

//* ************************************************************************
//* ************************ CAROUSEL STORAGE CONFIG ************************
//* ************************************************************************

// Carousel storage grid configuration
#define CAROUSEL_COLUMNS 6          // Number of columns in carousel
#define CAROUSEL_ROWS 6             // Number of rows in carousel
#define CAROUSEL_TOTAL_POSITIONS 36 // Total positions (6x6)

//* ************************************************************************
//* ************************ ROW POSITIONS (X COORDINATES) ******************
//* ************************************************************************

// X positions for each row (in inches from home position)
// Row 0 is the top row, Row 5 is the bottom row
#define CAROUSEL_ROW_0_X 1.0        // Top row X position
#define CAROUSEL_ROW_1_X 1.0        // Second row X position
#define CAROUSEL_ROW_2_X 1.0        // Third row X position
#define CAROUSEL_ROW_3_X 1.0        // Fourth row X position
#define CAROUSEL_ROW_4_X 1.0        // Fifth row X position
#define CAROUSEL_ROW_5_X 1.0        // Bottom row X position

//* ************************************************************************
//* ************************ ROW HEIGHTS (Y COORDINATES) *******************
//* ************************************************************************

// Y heights for each row (in inches from home position)
// Row 0 is the top row, Row 5 is the bottom row
#define CAROUSEL_ROW_0_Y 8.5        // Top row Y height
#define CAROUSEL_ROW_1_Y 7.0        // Second row Y height
#define CAROUSEL_ROW_2_Y 5.5        // Third row Y height
#define CAROUSEL_ROW_3_Y 4.0        // Fourth row Y height
#define CAROUSEL_ROW_4_Y 2.5        // Fifth row Y height
#define CAROUSEL_ROW_5_Y 1.0        // Bottom row Y height

//* ************************************************************************
//* ************************ COLUMN POSITIONS (Z COORDINATES) **************
//* ************************************************************************

// Z positions for each column (in inches from home position)
// Column 0 is the leftmost column, Column 5 is the rightmost column
#define CAROUSEL_COL_0_Z 1.0        // Leftmost column Z position
#define CAROUSEL_COL_1_Z 3.0        // Second column Z position
#define CAROUSEL_COL_2_Z 5.0        // Third column Z position
#define CAROUSEL_COL_3_Z 7.0        // Fourth column Z position
#define CAROUSEL_COL_4_Z 9.0        // Fifth column Z position
#define CAROUSEL_COL_5_Z 11.0       // Rightmost column Z position

//* ************************************************************************
//* ************************ POSITION ARRAYS *******************************
//* ************************************************************************

// Array of X positions for each row (for easy access)
static const float CAROUSEL_ROW_X_POSITIONS[6] = {
    CAROUSEL_ROW_0_X,
    CAROUSEL_ROW_1_X,
    CAROUSEL_ROW_2_X,
    CAROUSEL_ROW_3_X,
    CAROUSEL_ROW_4_X,
    CAROUSEL_ROW_5_X
};

// Array of Y heights for each row (for easy access)
static const float CAROUSEL_ROW_Y_HEIGHTS[6] = {
    CAROUSEL_ROW_0_Y,
    CAROUSEL_ROW_1_Y,
    CAROUSEL_ROW_2_Y,
    CAROUSEL_ROW_3_Y,
    CAROUSEL_ROW_4_Y,
    CAROUSEL_ROW_5_Y
};

// Array of Z positions for each column (for easy access)
static const float CAROUSEL_COL_Z_POSITIONS[6] = {
    CAROUSEL_COL_0_Z,
    CAROUSEL_COL_1_Z,
    CAROUSEL_COL_2_Z,
    CAROUSEL_COL_3_Z,
    CAROUSEL_COL_4_Z,
    CAROUSEL_COL_5_Z
};

//* ************************************************************************
//* ************************ HELPER FUNCTIONS ******************************
//* ************************************************************************

// Get X position for a specific row
inline float getCarouselRowX(uint8_t row) {
    if (row < 6) {
        return CAROUSEL_ROW_X_POSITIONS[row];
    }
    return 0.0; // Default to 0 if invalid row
}

// Get Y height for a specific row
inline float getCarouselRowY(uint8_t row) {
    if (row < 6) {
        return CAROUSEL_ROW_Y_HEIGHTS[row];
    }
    return 0.0; // Default to 0 if invalid row
}

// Get Z position for a specific column
inline float getCarouselColZ(uint8_t column) {
    if (column < 6) {
        return CAROUSEL_COL_Z_POSITIONS[column];
    }
    return 0.0; // Default to 0 if invalid column
}

// Get complete position for a specific grid position
inline void getCarouselPosition(uint8_t column, uint8_t row, float& x, float& y, float& z) {
    x = getCarouselColZ(column);  // X axis moves in Z direction
    y = getCarouselRowY(row);     // Y axis moves up/down
    z = getCarouselRowX(row);     // Z axis moves in X direction
}

#endif // CAROUSEL_CONFIG_H 