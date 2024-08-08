#ifndef CELL_H
#define CELL_H

#include <stdbool.h>
#include <stdint.h>
#include "./util.h"


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
	// FIXME: this should have a different name
	bool hovered : 1;
};
UTIL_STATIC_ASSERT(sizeof(struct CellData) == 1);

struct CellArr {
	struct CellData *data;
	int width;
	int height;
};


#define CELL_GET(ARR_PTR_OUT, X, Y, CELL_PTR_OUT)            \
do {                                                         \
        UTIL_ASSERT((ARR_PTR_OUT) != 0);                     \
        UTIL_ASSERT(X >= 0);                                 \
        UTIL_ASSERT(Y >= 0);                                 \
        UTIL_ASSERT(X < (ARR_PTR_OUT)->width);               \
        UTIL_ASSERT(Y < (ARR_PTR_OUT)->height);              \
        const int _OUT_idx = (Y * (ARR_PTR_OUT)->width) + X; \
        CELL_PTR_OUT =  (ARR_PTR_OUT)->data + _OUT_idx;      \
} while(0)                                                   \


void cell_setup(struct CellArr *arr_out, int w, int h);
void cell_plant(struct CellArr *arr, int x, int y);
void cell_toggle(struct CellArr *arr, int x, int y);
bool cell_reveal(struct CellArr *arr, int x, int y);


#endif
