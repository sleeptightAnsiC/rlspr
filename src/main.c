#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "raylib.h"
#include "./util.h"
#include "./cell.h"

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
//       (that's why names are lower-case, despite
//       that values are clearly known at compile-time)
static const int width = 30;
static const int height = 16;
static const int bombs = 99;
static const int border = 1;
static const bool interactive_cursor = true;
static const bool interactive_cell = true;
static const bool safe_first_try = true;

int
main(void)
{
	// game/draw state persistent between loop cycles
	struct CellArr arr = { .data = NULL };
	// TODO: would be nice to set it based on screen DPI
	int scale = 50;
	bool finished = false;
	bool started = true;

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

		// initialize cells array
		if (started) {

			const size_t nmemb = (size_t)(width * height);
			const size_t size = sizeof(struct CellData);
			if (arr.data == NULL) {
				arr.data = calloc(nmemb, size);
			} else if ((int)(nmemb) < arr.w * arr.h) {
				free(arr.data);
				arr.data = calloc(nmemb, size);
			} else {
				memset(arr.data, 0, nmemb);
			}

			arr.w = width;
			arr.h = height;
			finished = false;

			// add bombs into random cells
			for (int i = 0; i < bombs;) {
				const int x = GetRandomValue(0, width - 1);
				const int y = GetRandomValue(0, height - 1);
				if (cell_bomb_get(&arr, x, y))
					continue;
				cell_bomb_set(&arr, x, y, true);
				++i;
			}
		}

		// initialize frame
		BeginDrawing();
		ClearBackground(GRAY);

		if (!finished) {
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
		// TODO:
		//    this huge block should be the very first thing abstracted away
		//    there should be a separated translation unit for it (maybe called "draw")
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
				&& !finished
			) {
				DrawRectangle(rect_x, rect_y, scale, scale, DARKGRAY);
				continue;
			}
			switch (cd->state)
			{
			case CELL_STATE_REVEALED: {
				// HACK: unique RED background for the bomb causing a game loss
				const Color color = (finished && cd->_bomb && cd->hovered) ? (RED) : (DARKGRAY);
				DrawRectangle(rect_x, rect_y, scale, scale, color);
				if (cd->_bomb)
					goto label_draw_bomb;
				switch (cd->_nearby)
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
			} case CELL_STATE_QUESTIONED:
			case CELL_STATE_FLAGGED: {
				const Color color = (finished && !cd->_bomb) ? (RED) : (ORANGE);
				const char *glyph = (cd->state == CELL_STATE_FLAGGED) ? "F" : "?";
				DrawText(glyph, char_x, char_y, scale, color);
				continue;
			} case CELL_STATE_UNTOUCHED:
				if (finished && cd-> _bomb)
					goto label_draw_bomb;
				continue;
			default:
				UTIL_UNREACHABLE();
			} //ends switch (cd->state)
		// FIXME: HAHAHA, turn this label into the function already...
		label_draw_bomb: {
			const int bomb_x = scale * (x + border) + scale_half;
			const int bomb_y = scale * (y + border) + scale_half;
			const float bomb_radius = (float)(scale_half) * 0.8f;
			DrawCircle(bomb_x, bomb_y, bomb_radius, DARKPURPLE);
			continue;
		} // label_draw_bomb
		}

		// draw cell borders
		for (int i = 0; i <= height + 1; ++i) {
			const int pos_x_start = border * scale;
			const int pos_x_end = width * scale + pos_x_start;
			const int pos_y = i * scale;
			DrawLine(pos_x_start, pos_y, pos_x_end, pos_y, DARKGRAY);
		}
		for (int i = 0; i <= width + 1; ++i) {
			const int pos_x = i * scale;
			const int pos_y_start = border * scale;
			const int pos_y_end = height * scale + pos_y_start;
			DrawLine(pos_x, pos_y_start, pos_x, pos_y_end, DARKGRAY);
		}

		// highlight border around currently hovered cell
		if (interactive_cell && hovered_cell != NULL) {
			const int pos_x = (hovered_x + border) * scale;
			const int pos_y = (hovered_y + border) * scale;
			DrawRectangleLines(pos_x, pos_y, scale, scale, BLACK);
		}

		// adjust cursor visuals
		if (interactive_cursor) {
			if (hovered_cell != NULL && !finished) {
				const bool revealed = hovered_cell->state == CELL_STATE_REVEALED;
				const int cursor = revealed ? MOUSE_CURSOR_DEFAULT : MOUSE_CURSOR_POINTING_HAND;
				SetMouseCursor(cursor);
			} else {
				SetMouseCursor(MOUSE_CURSOR_DEFAULT);
			}
		}

		// react to LMB click
		if (hovered_cell != NULL && IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && !finished) {
			if (safe_first_try && started && hovered_cell->_bomb) {

			} if (hovered_cell->_bomb) {
				finished = true;
				hovered_cell->state = CELL_STATE_REVEALED;
			} else {
				cell_reveal_recur(&arr, hovered_x, hovered_y);
			}
			started = false;
		}

		// react to RMB click
		if (hovered_cell != NULL && IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) && !finished) {
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

		// HACK: do not remove hover state from cell that lost the game
		// because this was a bomb that caused a loss,
		// and we gonna use this information to draw unique RED background
		if (hovered_cell != NULL && !finished)
			hovered_cell->hovered = false;

		// trigger game restart
		if (IsKeyPressed(KEY_R))
			started = true;

		// clean up at the end of the frame
		DrawFPS(0, 0);
		EndDrawing();
	}

	// cleanup at the end of the game
	CloseWindow();
	free(arr.data);
	arr.data = NULL;

	return EXIT_SUCCESS;
}
