#ifndef CELL_H
#define CELL_H

#include <stdbool.h>
#include <stdint.h>
#include "./util.h"

//     CELL
//
// Utilities for managing the array of cells.
// The whole purpose of this translations unit
// was to abstract away the code repetitions that occurred in GAME
// CellArr is just a continuous memory block (two-dimensional array)
// that holds the CellData, and GAME is manipulating said data.
// Cells do not do anything on their own and they're dependent on each other.


#define CELL_GET(ARR_PTR_OUT, X, Y, CELL_PTR_OUT)                \
        do {                                                     \
                UTIL_ASSERT((ARR_PTR_OUT) != 0);                 \
                UTIL_ASSERT(X >= 0);                             \
                UTIL_ASSERT(Y >= 0);                             \
                UTIL_ASSERT(X < (ARR_PTR_OUT)->width);           \
                UTIL_ASSERT(Y < (ARR_PTR_OUT)->height);          \
                const int _idx = (Y * (ARR_PTR_OUT)->width) + X; \
                CELL_PTR_OUT =  (ARR_PTR_OUT)->data + _idx;      \
        } while(0)                                               \


enum CellState {
	CELL_STATE_UNTOUCHED = 0,
	CELL_STATE_REVEALED,
	CELL_STATE_FLAGGED,
	CELL_STATE_QUESTIONED,
};

struct CellData {
	uint8_t _nearby : 4;
	uint8_t state : 2;
	bool _bomb : 1;
	// TODO: this should have a different name
	// since it's also set to 'true' when a cell causes the game loss
	bool hovered : 1;
};
UTIL_STATIC_ASSERT(sizeof(struct CellData) == 1);

struct CellArr {
	struct CellData *data;
	int width;
	int height;
	int untouched_count;
};


void cell_setup(struct CellArr *arr_out, int w, int h);
bool cell_action_1(struct CellArr *arr, int x, int y);
void cell_action_2(struct CellArr *arr, int x, int y);
void cell_plant(struct CellArr *arr, int x, int y);


#endif
