#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./cell.h"
#include "./util.h"


#define _CELL_FOREACH_AROUND(ARR, X, Y, IT_X, IT_Y)            \
        for (                                                   \
                int IT_X = UTIL_MAX(0, (X - 1));                \
                IT_X <= UTIL_MIN(((ARR)->width - 1), (X + 1));  \
                ++(IT_X)                                        \
        ) for (                                                 \
                int IT_Y = UTIL_MAX(0, (Y - 1));                \
                IT_Y <= UTIL_MIN(((ARR)->height - 1), (Y + 1)); \
                ++(IT_Y)                                        \
        ) if (X == IT_X && Y == IT_Y); else                     \


static bool _cell_reveal_recur(struct CellArr *arr, int x, int y);


bool
cell_action_1(struct CellArr *arr, int x, int y)
{
	struct CellData *cd;
	CELL_GET(arr, x, y, cd);
	bool bomb_hit = false;
	if (cd->state == CELL_STATE_UNTOUCHED) {
		bomb_hit = _cell_reveal_recur(arr, x, y);
	} else if (cd->state == CELL_STATE_REVEALED) {
		int flags = 0;
		_CELL_FOREACH_AROUND(arr, x, y, it_x, it_y) {
			const struct CellData *it_cd;
			CELL_GET(arr, it_x, it_y, it_cd);
			if (it_cd->state == CELL_STATE_FLAGGED)
				++flags;
		}
		if (flags == cd->_nearby) {
			_CELL_FOREACH_AROUND(arr, x, y, it_x, it_y) {
				const bool it_bombed = _cell_reveal_recur(arr, it_x, it_y);
				if (it_bombed)
					bomb_hit = true;
			}
		}
	}
	return bomb_hit;
}

void
cell_plant(struct CellArr *arr, int x, int y)
{
	struct CellData *cd;
	CELL_GET(arr, x, y, cd);
	UTIL_ASSERT(!cd->_bomb);
	cd->_bomb = true;
	++(cd->_nearby);
	_CELL_FOREACH_AROUND(arr, x, y, it_x, it_y) {
		struct CellData *it_cd;
		CELL_GET(arr, it_x, it_y, it_cd);
		++(it_cd->_nearby);
	}
}

void
cell_setup(struct CellArr *arr_out, int w, int h)
{
	UTIL_ASSERT(arr_out != NULL);
	const size_t nmemb = (size_t)(w * h);
	const size_t size = sizeof(struct CellData);
	if (arr_out->data == NULL) {
		arr_out->data = calloc(nmemb, size);
	} else if ((int)(nmemb) < arr_out->width * arr_out->height) {
		free(arr_out->data);
		arr_out->data = calloc(nmemb, size);
	} else {
		memset(arr_out->data, 0, nmemb);
	}
	arr_out->width = w;
	arr_out->height = h;
	arr_out->untouched_count = w * h;
}

static bool
_cell_reveal_recur(struct CellArr *arr, int x, int y)
{
	struct CellData *cd;
	CELL_GET(arr, x, y, cd);

	if (cd->state != CELL_STATE_UNTOUCHED)
		return false;

	cd->state = CELL_STATE_REVEALED;
	--(arr->untouched_count);

	if (cd->_bomb) {
		cd->hovered = true;
		return true;
	}

	if (cd->_nearby != 0)
		return false;

	_CELL_FOREACH_AROUND(arr, x, y, it_x, it_y) {
		const bool it_bombed = _cell_reveal_recur(arr, it_x, it_y);
		if (it_bombed)
			return true;
	}

	return false;
}

