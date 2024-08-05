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
	bool hovered : 1;
};
UTIL_STATIC_ASSERT(sizeof(struct CellData) == 1);

struct CellArr {
	struct CellData *data;
	int w;
	int h;
};

struct CellData *   cell_at             (struct CellArr *arr, int x, int y);
void                cell_reveal_recur   (struct CellArr *arr, int x, int y, void *b_bomb_out);
void                cell_bomb_plant     (struct CellArr *arr, int x, int y);
void                cell_setup          (struct CellArr *arr_out, int w, int h);
void                cell_destroy        (struct CellArr *arr_out);


#endif
