#include <stdlib.h>
#include <string.h>
#include "./cell.h"
#include "./util.h"


static void _foreach_around(struct CellArr *arr, int x, int y, void *data, void(func)(struct CellArr *arr, int x, int y, void *data));
static void _nearby_increment(struct CellArr *arr, int x, int y, void *_unused);


void
cell_reveal_recur(struct CellArr *arr, int x, int y, void *b_bomb_out)
{
	struct CellData *cd = cell_at(arr, x, y);
	if (cd->state == CELL_STATE_UNTOUCHED) {
		cd->state = CELL_STATE_REVEALED;
		const bool bomb = cd->_bomb;
		*(bool *)(b_bomb_out) = bomb;
		if (cd->_nearby == 0 && !bomb)
			_foreach_around(arr, x, y, b_bomb_out, cell_reveal_recur);
	}
}

void
cell_bomb_plant(struct CellArr *arr, int x, int y)
{
	struct CellData *cd = cell_at(arr, x, y);
	UTIL_ASSERT(!cd->_bomb);
	cd->_bomb = true;
	++(cd->_nearby);
	_foreach_around(arr, x, y, NULL, _nearby_increment);
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
cell_destroy (struct CellArr *arr_out)
{
	UTIL_ASSERT(arr_out != NULL);
	free(arr_out->data);
	arr_out->data = NULL;
}


static void
_foreach_around(struct CellArr *arr, int x, int y, void *data, void(func)(struct CellArr *arr, int x, int y, void *data))
{
	const bool xg = x >= 0 + 1;
	const bool xl = x < arr->w - 1;
	const bool yg = y >= 0 + 1;
	const bool yl = y < arr->h - 1;
	if (xg && yg)
		func(arr, x-1, y-1, data);
	if (xg)
		func(arr, x-1, y, data);
	if (xg && yl)
		func(arr, x-1, y+1, data);
	if (xl && yg)
		func(arr, x+1, y-1, data);
	if (xl)
		func(arr, x+1, y, data);
	if (xl && yl)
		func(arr, x+1, y+1, data);
	if (yg)
		func(arr, x, y-1, data);
	if (yl)
		func(arr, x, y+1, data);
}

static void
_nearby_increment(struct CellArr *arr, int x, int y, void *_unused)
{
	(void)_unused;
	++(cell_at(arr, x, y)->_nearby);
}

