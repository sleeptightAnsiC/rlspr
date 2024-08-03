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
	// WARN: this one should be 'enum CellState' but it would break the padding
	uint8_t state: 2;
	bool _bomb: 1;
	bool hovered: 1;
};
// kurde balans, git majonez
UTIL_STATIC_ASSERT(sizeof(struct CellData) == 1);

struct CellArr {
	struct CellData *data;
	int w;
	int h;
};
UTIL_STATIC_ASSERT(sizeof(struct CellArr) == 16);


struct CellData * cell_get(struct CellArr *arr, int x, int y);
void cell_foreach_around(struct CellArr *arr, int x, int y, void(func)(struct CellArr *arr, int x, int y));
void cell_reveal_recur(struct CellArr *arr, int x, int y);
void cell_bomb_set(struct CellArr *arr, int x, int y, bool bomb);
bool cell_bomb_get(struct CellArr *arr, int x, int y);


#endif
