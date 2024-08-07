#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./cell.h"
#include "./util.h"


// https://www.reddit.com/r/cprogramming/comments/1ekzyzu/comment/lgsv11a/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
#define _FOREACH_AROUND(ARR, X, Y, IT_X, IT_Y)             \
        for (                                              \
                int IT_X = UTIL_MAX(0, (X - 1));           \
                IT_X <= UTIL_MIN(((ARR)->w - 1), (X + 1)); \
                ++(IT_X)                                   \
        ) for (                                            \
                int IT_Y = UTIL_MAX(0, (Y - 1));           \
                IT_Y <= UTIL_MIN(((ARR)->h - 1), (Y + 1)); \
                ++(IT_Y)                                   \
        ) if (X == IT_X && Y == IT_Y); else                \


static bool _reveal_recur(struct CellArr *arr, int x, int y);


bool
cell_reveal(struct CellArr *arr, int x, int y)
{
	struct CellData *cd = cell_at(arr, x, y);
	bool bomb_hit = false;
	if (cd->state == CELL_STATE_UNTOUCHED) {
		bomb_hit = _reveal_recur(arr, x, y);
	} else if (cd->state == CELL_STATE_REVEALED) {
		int flags = 0;
		_FOREACH_AROUND(arr, x, y, it_x, it_y) {
			const struct CellData *it_cd = cell_at(arr, it_x, it_y);
			if (it_cd->state == CELL_STATE_FLAGGED)
				++flags;
		}
		if (flags == cd->_nearby) {
			_FOREACH_AROUND(arr, x, y, it_x, it_y) {
				const bool it_bombed = _reveal_recur(arr, it_x, it_y);
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
	struct CellData *cd = cell_at(arr, x, y);
	UTIL_ASSERT(!cd->_bomb);
	cd->_bomb = true;
	++(cd->_nearby);
	_FOREACH_AROUND(arr, x, y, it_x, it_y) {
		struct CellData *it_cd = cell_at(arr, it_x, it_y);
		++(it_cd->_nearby);
	}
}

struct CellData *
cell_at(struct CellArr *arr, int x, int y)
{
	UTIL_ASSERT(arr != NULL);
	UTIL_ASSERT(x >= 0);
	UTIL_ASSERT(y >= 0);
	UTIL_ASSERT(x < arr->w);
	UTIL_ASSERT(y < arr->h);
	const int idx = (y * arr->w) + x;
	return arr->data + idx;
}

void
cell_setup(struct CellArr *arr_out, int w, int h)
{
	UTIL_ASSERT(arr_out != NULL);
	const size_t nmemb = (size_t)(w * h);
	const size_t size = sizeof(struct CellData);
	if (arr_out->data == NULL) {
		arr_out->data = calloc(nmemb, size);
	} else if ((int)(nmemb) < arr_out->w * arr_out->h) {
		free(arr_out->data);
		arr_out->data = calloc(nmemb, size);
	} else {
		memset(arr_out->data, 0, nmemb);
	}
	arr_out->w = w;
	arr_out->h = h;
}

void
cell_destroy(struct CellArr *arr_out)
{
	UTIL_ASSERT(arr_out != NULL);
	free(arr_out->data);
	arr_out->data = NULL;
}

static bool
_reveal_recur(struct CellArr *arr, int x, int y)
{
	struct CellData *cd = cell_at(arr, x, y);
	if (cd->state == CELL_STATE_UNTOUCHED) {
		cd->state = CELL_STATE_REVEALED;
		const bool bombed = cd->_bomb;
		if (bombed) {
			cd->hovered = true;
			return true;
		}
		if (cd->_nearby == 0 && !bombed) {
			_FOREACH_AROUND(arr, x, y, it_x, it_y) {
				const bool it_bombed = _reveal_recur(arr, it_x, it_y);
				if (it_bombed)
					return true;
			}
		}
	}
	return false;
}

