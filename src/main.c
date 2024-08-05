#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "./raylib.h"
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
static const int width = 9;
static const int height = 9;
static const int bombs = 10;
static const int border = 1;
static const bool safe_first_try = true;


int
main(void)
{
	struct CellArr arr = { .data = NULL };
	// TODO: would be nice to set it based on screen DPI
	int scale = 50;
	bool finished = false;
	bool started = true;

	{
		const int win_w = (width + (border * 2)) * scale;
		const int win_h = (height + (border * 2)) * scale;
		SetConfigFlags(FLAG_VSYNC_HINT + FLAG_WINDOW_RESIZABLE + FLAG_WINDOW_UNDECORATED + FLAG_MSAA_4X_HINT);
		InitWindow(win_w, win_h, "rlspr");
		UTIL_ASSERT(IsWindowReady());
	}

	while (!WindowShouldClose()) {

		int hovered_x = -1;
		int hovered_y = -1;
		struct CellData *hovered_cell = NULL;

		if (started) {
			cell_setup(&arr, width, height);
			finished = false;
			// add bombs into random cells
			for (int i = 0; i < bombs;) {
				const int x = GetRandomValue(0, width - 1);
				const int y = GetRandomValue(0, height - 1);
				struct CellData *cd = cell_at(&arr, x, y);
				if (cd->_bomb)
					continue;
				cell_bomb_plant(&arr, x, y);
				++i;
			}
		}

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
				hovered_cell = cell_at(&arr, hovered_x, hovered_y);
				hovered_cell->hovered = true;
			}
		}

		// TODO:
		//    this huge block should be the very first thing abstracted away
		//    there should be a separated translation unit for it (maybe called "draw")
		for (int x = 0; x < arr.w; ++x) for (int y = 0; y < arr.h; ++y) {
			const int char_x = scale * (x + border) + (int)(0.3f * (float)(scale));
			const int char_y = scale * (y + border) + (int)(0.1f * (float)(scale));
			const int rect_x = (border + x) * scale;
			const int rect_y = (border + y) * scale;
			const int scale_half = (int)((float)(scale) * 0.5f);
			const struct CellData *cd = cell_at(&arr, x, y);
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
			} case CELL_STATE_QUESTIONED: {
				// FALLTHROUGH
			} case CELL_STATE_FLAGGED: {
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
			} //end switch (cd->state)
		// FIXME: HAHAHA, turn this label into the function already...
		label_draw_bomb: {
			const int bomb_x = scale * (x + border) + scale_half;
			const int bomb_y = scale * (y + border) + scale_half;
			const float bomb_radius = (float)(scale_half) * 0.8f;
			DrawCircle(bomb_x, bomb_y, bomb_radius, DARKPURPLE);
			continue;
		} //end label_draw_bomb
		}

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

		if (hovered_cell != NULL && IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && !finished) {

			while (safe_first_try && started && hovered_cell->_nearby != 0) {
				cell_setup(&arr, width, height);
				for (int i = 0; i < bombs;) {
					const int x = GetRandomValue(0, width - 1);
					const int y = GetRandomValue(0, height - 1);
					struct CellData *cd = cell_at(&arr, x, y);
					if (cd->_bomb)
						continue;
					cell_bomb_plant(&arr, x, y);
					++i;
				}
			}
			started = false;

			finished = cell_reveal(&arr, hovered_x, hovered_y);
		}

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

		// HACK: do not remove hover state from cell that lost the game
		// because this was a bomb that caused a loss,
		// and we gonna use this information to draw unique RED background
		if (hovered_cell != NULL && !finished)
			hovered_cell->hovered = false;

		if (IsKeyPressed(KEY_R))
			started = true;

		EndDrawing();
	}

	CloseWindow();
	cell_destroy(&arr);
	return EXIT_SUCCESS;
}
