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
	// WARN: this one should be 'enum CellState' but it would break the padding
	uint8_t state: 2;
	bool planted: 1;
	bool hovered: 1;
};
// kurde balans, git majonez
UTIL_STATIC_ASSERT(sizeof(struct CellData) == 1);

// FIXME:
// Everything uses ambigious signed 'int'
// because Raylib uses it everywhere
// but all those ints should be 'uint32_t' instead!
// For now it's just easier to keep as it is...

struct CellArr {
	struct CellData *_data;
	int w;
	int h;
};
UTIL_STATIC_ASSERT(sizeof(struct CellArr) == 16);

static struct CellData *
cell_get(struct CellArr *arr, int x, int y)
{
	UTIL_ASSERT(arr != NULL);
	UTIL_ASSERT(x >= 0);
	UTIL_ASSERT(y >= 0);
	UTIL_ASSERT(x < arr->w);
	UTIL_ASSERT(x < arr->h);
	int idx = (y * arr->w) + x;
	return &(arr->_data[idx]);
}

// TODO: this returns true in case of revealing the bomb
//    but I'm not sure if placing said logic here is a good idea
static bool
cell_reveal(struct CellArr *arr, int x, int y)
{
	struct CellData *ca = cell_get(arr, x, y);
	if (ca->state == CELL_STATE_UNTOUCHED) {
		ca->state = CELL_STATE_REVEALED;
		if (ca->planted)
			return true;
		if (ca->nearby == 0) {
			if (x > 0)
				cell_reveal(arr, x-1, y);
			if (x < arr->w - 1)
				cell_reveal(arr, x+1, y);
			if (y > 0)
				cell_reveal(arr, x, y-1);
			if (y < arr->h - 1)
				cell_reveal(arr, x, y+1);
		}
	}
	return false;
}

static void
cell_foreach_around(struct CellArr *arr, int x, int y, void(func)(struct CellData *))
{
	const bool xg = x >= 0 + 1;
	const bool xl = x < arr->w - 1;
	const bool yg = y >= 0 + 1;
	const bool yl = y < arr->h - 1;
	if (xg && yg)
		func(cell_get(arr, x-1, y-1));
	if (xg)
		func(cell_get(arr, x-1, y));
	if (xg && yl)
		func(cell_get(arr, x-1, y+1));
	if (xl && yg)
		func(cell_get(arr, x+1, y-1));
	if (xl)
		func(cell_get(arr, x+1, y));
	if (xl && yl)
		func(cell_get(arr, x+1, y+1));
	if (yg)
		func(cell_get(arr, x, y-1));
	if (yl)
		func(cell_get(arr, x, y+1));
}

static void
cell_nearby_increment(struct CellData *cd)
{
	UTIL_ASSERT(cd != NULL);
	++(cd->nearby);
}

#define X_NUMBERS_COLORS_MAP \
        X(1, BLUE)           \
        X(2, GREEN)          \
        X(3, RED)            \
        X(4, DARKBLUE)       \
        X(5, MAROON)         \
        X(6, SKYBLUE)        \
        X(7, PURPLE)         \
        X(8, DARKPURPLE)     \

static const int width = 9;
static const int height = 9;
static const int bombs = 10;

int
main(void)
{
	struct CellArr arr;

	{  // initialize cells array
		struct CellData *data = calloc((size_t)(width * height), sizeof(struct CellData));
		arr._data = data;
		arr.w = width;
		arr.h = height;
	}

	int border = 1;
	int scale = 50;
	int win_w = (width + (border * 2)) * scale;
	int win_h = win_w;
	bool bombed = false;

	SetConfigFlags(FLAG_VSYNC_HINT + FLAG_WINDOW_RESIZABLE + FLAG_WINDOW_UNDECORATED + FLAG_MSAA_4X_HINT);
	InitWindow(win_w, win_h, "rlspr");
	UTIL_ASSERT(IsWindowReady());

	// plant bombs into random cells
	// WARN: This must be after after InitWindow (see link below) !!!
	// https://www.reddit.com/r/raylib/comments/r58340/comment/hmnkc48/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button
	for (int i = 0; i < bombs;) {
		const int x = GetRandomValue(0, width - 1);
		const int y = GetRandomValue(0, height - 1);
		struct CellData *cd = cell_get(&arr, x, y);
		if (cd->planted)
			continue;
		cd->planted = true;
		cell_foreach_around(&arr, x, y, cell_nearby_increment);
		++i;
	}

	while (!WindowShouldClose())
	{
		// frame init
		BeginDrawing();
		ClearBackground(GRAY);

		int hovered_x = -1;
		int hovered_y = -1;
		struct CellData *hovered_cell = NULL;

		// set hovered_x/y, check if any cell is being hovered
		const int mouse_x = GetMouseX();
		const int mouse_y = GetMouseY();
		if (mouse_x < (border + width) * scale && mouse_x >= border * scale)
			hovered_x = (mouse_x - border * scale) / scale;
		if (mouse_y < (border + height) * scale && mouse_y >= border * scale)
			hovered_y = (mouse_y - border * scale) / scale;
		if (hovered_x != -1 && hovered_y != -1) {
			hovered_cell = cell_get(&arr, hovered_x, hovered_y);
			hovered_cell->hovered = true;
		}

		// draw cell contents
		for (int x = 0; x < arr.w; ++x) for (int y = 0; y < arr.h; ++y) {
			const int char_pos_x = scale * (x + border) + (int)(0.3f * (float)(scale));
			const int char_pos_y = scale * (y + border) + (int)(0.1f * (float)(scale));
			const struct CellData *cd = cell_get(&arr, x, y);
			if (
				true
				&& cd == hovered_cell
				&& cd->state == CELL_STATE_UNTOUCHED
				&& IsMouseButtonDown(MOUSE_BUTTON_LEFT)
			) {
				// FIXME: code repetition
				const int rect_x = (border + x) * scale;
				const int rect_y = (border + y) * scale;
				DrawRectangle(rect_x, rect_y, scale, scale, DARKGRAY);
			} else switch (cd->state) {
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
				} else switch (cd->nearby) {
				case 0:
					break;
#				define X(NEARBY, COLOR)                                          \
				case NEARBY:                                                     \
				        DrawText(#NEARBY, char_pos_x, char_pos_y, scale, COLOR); \
				        break                                                    \
				        ;                                                        
				X_NUMBERS_COLORS_MAP
#				undef X
				default:
					UTIL_UNREACHABLE();
				} //end else switch (cd->nearby)
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
			} //end else switch (cd->state)
		}

		// draw cell borders
		for (int i = 0; i <= width + 1; ++i) {
			const int stride = scale * i;
			const int start_pos_x = border * scale;
			const int end_pos_x = width * scale + start_pos_x;
			DrawLine(start_pos_x, stride, end_pos_x, stride, DARKGRAY);
			DrawLine(stride, start_pos_x, stride, end_pos_x, DARKGRAY);
		}

		// highlight border around currently hovered cell
		if (hovered_cell != NULL) {
			const int pos_x = (hovered_x + border) * scale;
			const int pos_y = (hovered_y + border) * scale;
			DrawRectangleLines(pos_x, pos_y, scale, scale, BLACK);
		}

		// adjust cursor visuals
		if (hovered_cell != NULL) {
			const bool not_revealed = hovered_cell->state  != CELL_STATE_REVEALED;
			const int cursor = not_revealed ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT;
			SetMouseCursor(cursor);
		} else {
			SetMouseCursor(MOUSE_CURSOR_DEFAULT);
		}

		// react to LMB click
		if (
			true
			&& hovered_cell != NULL
			&& hovered_cell->state == CELL_STATE_UNTOUCHED
			&& IsMouseButtonReleased(MOUSE_BUTTON_LEFT)
		) {
			// FIXME: add logic related to winning, loosing and restarting the game
			bombed = cell_reveal(&arr, hovered_x, hovered_y);
		}
		(void)bombed;

		// react to RMB click
		if (hovered_cell != NULL && IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
			const enum CellState hcs = hovered_cell->state;
			switch (hcs)
			{
			case CELL_STATE_REVEALED:
				break;
			case CELL_STATE_UNTOUCHED:
				hovered_cell->state = CELL_STATE_FLAGGED;
				break;
			case CELL_STATE_FLAGGED:
				hovered_cell->state = CELL_STATE_QUESTIONED;
				break;
			case CELL_STATE_QUESTIONED:
				hovered_cell->state = CELL_STATE_UNTOUCHED;
				break;
			default:
				UTIL_UNREACHABLE();
			}
		}

		// zoom-in/out based on mouse-scroll movement (visible in next frame)
		{
			// TODO: zooming should follow the cursor position
			// TODO: add ability to scroll/pad the area (by cholding scroll-wheel / mouse3)
			float mouseWheelMove = GetMouseWheelMove();
			scale += (int)mouseWheelMove;
		}

		// clean up at the end of the frame
		if (hovered_cell != NULL)
			hovered_cell->hovered = false;
		DrawFPS(0, 0);
		EndDrawing();
	}
	CloseWindow();

	{  // deinitialize cells array
		free(arr._data);
		arr._data = NULL;
	}

	return EXIT_SUCCESS;
}
