#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "./util.h"
#include "raylib.h"

struct CellState {
	uint8_t nearby : 4;
	bool planted : 1;
	bool flagged : 1;
	bool revealed : 1;
	bool questioned : 1;
};
// kurde balans, git majonez
UTIL_STATIC_ASSERT(sizeof(struct CellState) == 1);

// FIXME:
// Everything uses ambigious 'int'
// because Raylib uses it everywhere
// but 'uint32_t' should be used instead!

struct CellArr {
	struct CellState *_data;
	int w;
	int h;
};
UTIL_STATIC_ASSERT(sizeof(struct CellArr) == 16);

static struct CellArr
cell_new(int w, int h)
{
	struct CellState *data = calloc((size_t)(w * h), sizeof(struct CellState));
	struct CellArr arr = {
		._data = data,
		.w = w,
		.h = h,
	};
	return arr;
}

static void
cell_destroy(struct CellArr *arr)
{
	free(arr->_data);
}

static int
cell_xy_to_idx(struct CellArr *arr, int x, int y)
{
	assert(arr != NULL);
	assert(x >= 0);
	assert(y >= 0);
	assert(x < arr->w);
	assert(x < arr->h);
	return (y * arr->w) + x;
}

static struct CellState *
cell_get(struct CellArr *arr, int x, int y)
{
	int idx = cell_xy_to_idx(arr, x, y);
	return &(arr->_data[idx]);
}

static void
cell_foreach_around(struct CellArr *arr, int x, int y, void(func)(struct CellState *))
{
	const bool xg0 = x > 0;
	const bool xlw = x < arr->w - 1;
	const bool yg0 = y > 0;
	const bool ylh = y < arr->h - 1;
	// -1 -1
	// -1 0
	// -1 1
	if (xg0 && yg0)
		func(cell_get(arr, x-1, y-1));
	if (xg0)
		func(cell_get(arr, x-1, y));
	if (xg0 && ylh)
		func(cell_get(arr, x-1, y+1));
	// 1 -1
	// 1 0
	// 1 1
	if (xlw && yg0)
		func(cell_get(arr, x+1, y-1));
	if (xlw)
		func(cell_get(arr, x+1, y));
	if (xlw && ylh)
		func(cell_get(arr, x+1, y+1));
	// 0 -1
	// 0 1
	if (yg0)
		func(cell_get(arr, x, y-1));
	if (ylh)
		func(cell_get(arr, x, y+1));
}

static void
_nearby_increment(struct CellState *cs)
{
	assert(cs != NULL);
	++(cs->nearby);
}

static void
cell_plant(struct CellArr *arr, int x, int y)
{
	struct CellState *cs = cell_get(arr, x, y);
	cs->planted = true;
	cell_foreach_around(arr, x, y, _nearby_increment);
}

#define NUM 10

#define X_NUMBERS_COLORS_MAP() \
        X(1, BLUE)             \
        X(2, GREEN)            \
        X(3, RED)              \
        X(4, DARKBLUE)         \
        X(5, MAROON)           \
        X(6, SKYBLUE)          \
        X(7, PURPLE)           \
        X(6, DARKPURPLE)       \

int
main(void)
{
	struct CellArr cells = cell_new(NUM, NUM);

	// FIXME: this should be semi-random
	cell_plant(&cells, 2, 5);
	cell_plant(&cells, 2, 6);
	cell_plant(&cells, 3, 5);
	cell_plant(&cells, 2, 9);
	cell_plant(&cells, 9, 5);
	cell_plant(&cells, 9, 5);
	cell_plant(&cells, 0, 5);
	cell_plant(&cells, 0, 0);

	int border = 1;
	int scale = 50;
	int win_w = (NUM + (border * 2)) * scale;
	int win_h = win_w;

	SetConfigFlags(FLAG_VSYNC_HINT + FLAG_WINDOW_RESIZABLE + FLAG_WINDOW_UNDECORATED + FLAG_MSAA_4X_HINT);
	InitWindow(win_w, win_h, "rlspr");
	assert(IsWindowReady());
	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(GRAY);

		{
			float mouseWheelMove = GetMouseWheelMove();
			scale += (int)mouseWheelMove;
		}

		// draw blank cells
		for (int i = 0; i <= NUM + 1; ++i) {
			const int stride = scale * i;
			const int startPos = border * scale;
			const int endPos = NUM * scale + startPos;
			DrawLine(startPos, stride, endPos, stride, DARKGRAY);
			DrawLine(stride, startPos, stride, endPos, DARKGRAY);
		}

		const int mouse_x = GetMouseX();
		const int mouse_y = GetMouseY();

		// highlight currently hovered cell
		if (
			true
			&& mouse_x <= (border + NUM) * scale
			&& mouse_x >= border * scale
			&& mouse_y <= (border + NUM) * scale
			&& mouse_y >= border * scale
		) {
			const int pos_x = (int)(mouse_x / scale) * scale;
			const int pos_y = (int)(mouse_y / scale) * scale;
			DrawRectangleLines(pos_x, pos_y, scale, scale, BLACK);
			SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
		} else {
			SetMouseCursor(MOUSE_CURSOR_DEFAULT);
		}

		for (int x = 0; x < cells.w; ++x) for (int y = 0; y < cells.h; ++y) {
			const struct CellState *curr = cell_get(&cells, x, y);
			if (curr->planted) {
				const int scale_half = (int)(scale * 0.5);
				const int center_x = scale * (x + border) + scale_half;
				const int center_y = scale * (y + border) + scale_half;
				const float radius = (float)(scale_half * 0.8);
				DrawCircleLines(center_x, center_y, radius, DARKPURPLE);
			}
// 			else switch (curr->nearby)
// 			{
// 			default: UTIL_UNREACHABLE();
// #			define X(NUM, COLOR) DrawText(#NUM, )
// 			}
		}

		DrawFPS(0, 0);
		EndDrawing();
	}
	CloseWindow();

	cell_destroy(&cells);
	return EXIT_SUCCESS;
}
