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
	struct CellData *data;
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
	return &(arr->data[idx]);
}

// TODO: this returns true in case of revealing the bomb
//    but I'm not sure if placing said logic here is a good idea
static bool
cell_reveal_recur(struct CellArr *arr, int x, int y)
{
	struct CellData *ca = cell_get(arr, x, y);
	if (ca->state == CELL_STATE_UNTOUCHED) {
		// FIXME: do I actually want to reveal cell when it's planted with bomb??
		ca->state = CELL_STATE_REVEALED;
		if (ca->planted)
			return true;
		if (ca->nearby == 0) {
			if (x > 0)
				cell_reveal_recur(arr, x-1, y);
			if (x < arr->w - 1)
				cell_reveal_recur(arr, x+1, y);
			if (y > 0)
				cell_reveal_recur(arr, x, y-1);
			if (y < arr->h - 1)
				cell_reveal_recur(arr, x, y+1);
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

// NOTE: this is passed as a function pointer in only once place
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

// game settings
// TODO: these should be possible to change at runtime someday
//       (that's why names are lower-cased, despite
//       that values are clearly known at compile-time)
static const int width = 9;
static const int height = 9;
static const int bombs = 10;
static const int border = 1;

int
main(void)
{
	// game/draw state persistent between loop cycles
	struct CellArr arr = { .data = NULL };
	int scale = 50;
	bool lost = false;
	bool restarted = false;

	{  // initialize window
		const int win_w = (width + (border * 2)) * scale;
		const int win_h = (height + (border * 2)) * scale;
		SetConfigFlags(FLAG_VSYNC_HINT + FLAG_WINDOW_RESIZABLE + FLAG_WINDOW_UNDECORATED + FLAG_MSAA_4X_HINT);
		InitWindow(win_w, win_h, "rlspr");
		UTIL_ASSERT(IsWindowReady());
	}

	// main game/draw loop
	while (!WindowShouldClose()) {

		int hovered_x = -1;
		int hovered_y = -1;
		struct CellData *hovered_cell = NULL;

		// initialize cells array (only if needed)
		if (arr.data == NULL || restarted) {

			// TODO:
			// array does not need to be reallocated if it's width and height hasn't changed.
			// In this case, it should be only memset to 0 to avoid costly operation
			// BUT width and height are currently static and reallocation is just easier,
			// so noone cares for now... Also, this will work all the time :)

			if (restarted) {
				free(arr.data);
				lost = false;
				restarted = false;
			}

			arr.data = calloc((size_t)(width * height), sizeof(struct CellData));
			arr.w = width;
			arr.h = height;

			// plant bombs into random cells
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
		}

		// initialize frame
		BeginDrawing();
		ClearBackground(GRAY);

		if (!lost) {
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
		}

		// draw cell contents
		for (int x = 0; x < arr.w; ++x) for (int y = 0; y < arr.h; ++y) {
			const int char_x = scale * (x + border) + (int)(0.3f * (float)(scale));
			const int char_y = scale * (y + border) + (int)(0.1f * (float)(scale));
			const int rect_x = (border + x) * scale;
			const int rect_y = (border + y) * scale;
			const int scale_half = (int)((float)(scale) * 0.5f);
			const struct CellData *cd = cell_get(&arr, x, y);
			if (
				true
				&& cd == hovered_cell
				&& cd->state == CELL_STATE_UNTOUCHED
				&& IsMouseButtonDown(MOUSE_BUTTON_LEFT)
				&& !lost
			) {
				DrawRectangle(rect_x, rect_y, scale, scale, DARKGRAY);
				continue;
			}
			switch (cd->state)
			{
			case CELL_STATE_REVEALED: {
				const Color bg = (lost && cd->planted && cd->hovered) ? (RED) : (DARKGRAY);
				DrawRectangle(rect_x, rect_y, scale, scale, bg);
				if (cd->planted)
					goto label_draw_bomb;
				switch (cd->nearby)
				{
#				define X(NEARBY, COLOR)                                  \
				case NEARBY:                                             \
				        DrawText(#NEARBY, char_x, char_y, scale, COLOR); \
				        continue                                         \
				        ;                                                
				X_NUMBERS_COLORS_MAP
#				undef X
				case 0:
					continue;
				default:
					UTIL_UNREACHABLE();
				} //end switch (cd->nearby)
				continue;
			}
			case CELL_STATE_QUESTIONED:
				DrawText("?", char_x, char_y, scale, ORANGE);
				continue;
			case CELL_STATE_FLAGGED:
				DrawText("F", char_x, char_y, scale, ORANGE);
				continue;
			case CELL_STATE_UNTOUCHED:
				if (lost && cd-> planted)
					goto label_draw_bomb;
				continue;
			default:
				UTIL_UNREACHABLE();
			} //ends switch (cd->state)
		label_draw_bomb: {
			const int bomb_x = scale * (x + border) + scale_half;
			const int bomb_y = scale * (y + border) + scale_half;
			const float bomb_radius = (float)(scale_half) * 0.8f;
			DrawCircle(bomb_x, bomb_y, bomb_radius, DARKPURPLE);
			continue;
		} // label_draw_bomb
		}

		// draw cell borders
		// FIXME: this gonna break when using width and height that aren't equal
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
		// FIXME: this wasn't a part of OG game so better remove this
		if (hovered_cell != NULL && !lost) {
			const bool not_revealed = hovered_cell->state  != CELL_STATE_REVEALED;
			const int cursor = not_revealed ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT;
			SetMouseCursor(cursor);
		} else {
			SetMouseCursor(MOUSE_CURSOR_DEFAULT);
		}

		// react to LMB click
		if (hovered_cell != NULL && IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && !lost)
			lost = cell_reveal_recur(&arr, hovered_x, hovered_y);

		// react to RMB click
		if (hovered_cell != NULL && IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) && !lost) {
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

		{  // zoom-in/out based on mouse-scroll movement (visible in next frame)
			// TODO: zooming should follow the cursor position
			// TODO: add ability to scroll/pad the area (by cholding scroll-wheel / mouse3)
			const float mouseWheelMove = GetMouseWheelMove();
			scale += (int)mouseWheelMove;
		}

		// clean up at the end of the frame
		DrawFPS(0, 0);
		EndDrawing();
		// HACK: do not remove hover state from cell that lost the game
		// because this was a bomb and we gonna use it for RED highlight
		if (hovered_cell != NULL && !lost)
			hovered_cell->hovered = false;

		// trigger game restart
		if (IsKeyPressed(KEY_R))
			restarted = true;
	}

	// cleanup at the end of the game
	CloseWindow();
	// TODO: this may cause double-free
	free(arr.data);
	arr.data = NULL;

	return EXIT_SUCCESS;
}
