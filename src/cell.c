#include <stdlib.h>
#include <string.h>
#include "./cell.h"
#include "util.h"


static void _foreach_around(struct CellArr *arr, int x, int y, void(func)(struct CellArr *arr, int x, int y));
static void _nearby_increment(struct CellArr *arr, int x, int y);


// TODO: this returns true in case of revealing the bomb
//    but I'm not sure if placing said logic here is a good idea
void
cell_reveal_recur(struct CellArr *arr, int x, int y)
{
	struct CellData *cd = cell_at(arr, x, y);
	if (cd->state == CELL_STATE_UNTOUCHED) {
		cd->state = CELL_STATE_REVEALED;
		if (cd->_nearby == 0)
			_foreach_around(arr, x, y, cell_reveal_recur);
	}
}

void
cell_bomb_plant(struct CellArr *arr, int x, int y)
{
	struct CellData *cd = cell_at(arr, x, y);
	UTIL_ASSERT(!cd->_bomb);
	cd->_bomb = true;
	++(cd->_nearby);
	_foreach_around(arr, x, y, _nearby_increment);
}

struct CellData *
cell_at(struct CellArr *arr, int x, int y)
{
	UTIL_ASSERT(arr != NULL);
	UTIL_ASSERT(x >= 0);
	UTIL_ASSERT(y >= 0);
	UTIL_ASSERT(x < arr->w);
	UTIL_ASSERT(y < arr->h);
	int idx = (y * arr->w) + x;
	return &(arr->data[idx]);
}

void
cell_initialize(struct CellArr *arr_out, int w, int h)
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
_foreach_around(struct CellArr *arr, int x, int y, void(func)(struct CellArr *arr, int x, int y))
{
	const bool xg = x >= 0 + 1;
	const bool xl = x < arr->w - 1;
	const bool yg = y >= 0 + 1;
	const bool yl = y < arr->h - 1;
	if (xg && yg)
		func(arr, x-1, y-1);
	if (xg)
		func(arr, x-1, y);
	if (xg && yl)
		func(arr, x-1, y+1);
	if (xl && yg)
		func(arr, x+1, y-1);
	if (xl)
		func(arr, x+1, y);
	if (xl && yl)
		func(arr, x+1, y+1);
	if (yg)
		func(arr, x, y-1);
	if (yl)
		func(arr, x, y+1);
}

static void
_nearby_increment(struct CellArr *arr, int x, int y)
{
	++(cell_at(arr, x, y)->_nearby);
}

