#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "./util.h"
#include "raylib.h"

enum CellState {
	CELL_STATE_UNTOUCHED = 0,
	CELL_STATE_REVEALED,
	CELL_STATE_FLAGGED,
	CELL_STATE_QUESTIONED,
};

struct CellData {
	uint8_t nearby : 4;
	// WARN: this one should be 'enum CellState'
	uint8_t state: 2;
	bool planted: 1;
	bool _pad: 1;
};
// kurde balans, git majonez
UTIL_STATIC_ASSERT(sizeof(struct CellData) == 1);

// FIXME:
// Everything uses ambigious 'int'
// because Raylib uses it everywhere
// but 'uint32_t' should be used instead!

struct CellArr {
	struct CellData *_data;
	int w;
	int h;
};
UTIL_STATIC_ASSERT(sizeof(struct CellArr) == 16);

static struct CellArr
cell_new(int w, int h)
{
	struct CellData *data = calloc((size_t)(w * h), sizeof(struct CellData));
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

static struct CellData *
cell_get(struct CellArr *arr, int x, int y)
{
	int idx = cell_xy_to_idx(arr, x, y);
	return &(arr->_data[idx]);
}

static void
cell_foreach_around(struct CellArr *arr, int x, int y, void(func)(struct CellData *))
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
_nearby_increment(struct CellData *cd)
{
	assert(cd != NULL);
	++(cd->nearby);
}

static void
cell_plant(struct CellArr *arr, int x, int y)
{
	struct CellData *cd = cell_get(arr, x, y);
	cd->planted = true;
	cell_foreach_around(arr, x, y, _nearby_increment);
}

#define NUM 10

#define X_NUMBERS_COLORS_MAP \
        X(1, BLUE)           \
        X(2, GREEN)          \
        X(3, RED)            \
        X(4, DARKBLUE)       \
        X(5, MAROON)         \
        X(6, SKYBLUE)        \
        X(7, PURPLE)         \
        X(8, DARKPURPLE)     \

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

	// reveal 2x2 cells in top-left corner
	cell_get(&cells, 0, 0)->state = CELL_STATE_REVEALED;
	cell_get(&cells, 1, 0)->state = CELL_STATE_REVEALED;
	cell_get(&cells, 0, 1)->state = CELL_STATE_REVEALED;
	cell_get(&cells, 1, 1)->state = CELL_STATE_REVEALED;

	// reveal 2 empty cells
	cell_get(&cells, 2, 2)->state = CELL_STATE_REVEALED;
	cell_get(&cells, 2, 3)->state = CELL_STATE_REVEALED;

	// flag and question 2 cells on the middle
	cell_get(&cells, 5, 5)->state = CELL_STATE_FLAGGED;
	cell_get(&cells, 4, 4)->state = CELL_STATE_QUESTIONED;

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
			// TODO: zooming should follow the cursor position
			// TODO: add ability to scroll/pad the area (by cholding scroll-wheel / mouse3)
			float mouseWheelMove = GetMouseWheelMove();
			scale += (int)mouseWheelMove;
		}

		// draw cell contents
		for (int x = 0; x < cells.w; ++x) for (int y = 0; y < cells.h; ++y) {
			const int char_pos_x = scale * (x + border) + (int)(0.3f * (float)(scale));
			const int char_pos_y = scale * (y + border) + (int)(0.1f * (float)(scale));
			const struct CellData *cd = cell_get(&cells, x, y);
			switch (cd->state)
			{
			case CELL_STATE_REVEALED: {
				const int rect_x = (border + x) * scale;
				const int rect_y = (border + y) * scale;
				DrawRectangle(rect_x, rect_y, scale, scale, DARKGRAY);
				if (cd->planted) {
					const int scale_half = (int)((float)(scale) * 0.5f);
					const int center_x = scale * (x + border) + scale_half;
					const int center_y = scale * (y + border) + scale_half;
					const float radius = (float)(scale_half) * 0.8f;
					DrawCircle(center_x, center_y, radius, DARKPURPLE);
				}
				else switch (cd->nearby)
				{
				case 0:
					break;
#				define X(NUM, COLOR) case NUM: DrawText(#NUM, char_pos_x, char_pos_y, scale, COLOR); break;
				X_NUMBERS_COLORS_MAP
#				undef X
				default:
					UTIL_UNREACHABLE();
				}
				break;
			}
			case CELL_STATE_QUESTIONED:
				DrawText("?", char_pos_x, char_pos_y, scale, ORANGE);
				break;
			case CELL_STATE_FLAGGED:
				// TODO: draw actual flag glyph
				DrawText("F", char_pos_x, char_pos_y, scale, ORANGE);
				break;
			case CELL_STATE_UNTOUCHED:
				break;
			default:
				UTIL_UNREACHABLE();
			}
		}

		// draw cell borders
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
			const int pos_x = (mouse_x / scale) * scale;
			const int pos_y = (mouse_y / scale) * scale;
			DrawRectangleLines(pos_x, pos_y, scale, scale, BLACK);
			SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
		} else {
			SetMouseCursor(MOUSE_CURSOR_DEFAULT);
		}

		DrawFPS(0, 0);
		EndDrawing();
	}
	CloseWindow();

	cell_destroy(&cells);
	return EXIT_SUCCESS;
}
