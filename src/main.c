#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "./raylib.h"
#include "./util.h"
#include "./cell.h"
#include "./game.h"


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
	struct GameState gs = game_init();

	{
		const int win_w = (gs.opts.width + (gs.opts.border * 2)) * gs.scale;
		const int win_h = (gs.opts.height + (gs.opts.border * 2)) * gs.scale;
		SetConfigFlags(FLAG_VSYNC_HINT + FLAG_WINDOW_RESIZABLE + FLAG_WINDOW_UNDECORATED + FLAG_MSAA_4X_HINT);
		InitWindow(win_w, win_h, "rlspr");
		UTIL_ASSERT(IsWindowReady());
	}

	while (!WindowShouldClose()) {

		BeginDrawing();
		ClearBackground(GRAY);

		if (gs.started)
			game_plant(&gs);

		if (!gs.finished)
			game_hover(&gs);

		// TODO:
		//    this huge block should be the very first thing abstracted away
		//    there should be a separated translation unit for it (maybe called "draw")
		for (int x = 0; x < gs.arr.w; ++x) for (int y = 0; y < gs.arr.h; ++y) {
			const int char_x = gs.scale * (x + gs.opts.border) + (int)(0.3f * (float)(gs.scale));
			const int char_y = gs.scale * (y + gs.opts.border) + (int)(0.1f * (float)(gs.scale));
			const int rect_x = (gs.opts.border + x) * gs.scale;
			const int rect_y = (gs.opts.border + y) * gs.scale;
			const struct CellData *cd = cell_at(&gs.arr, x, y);
			if (
				true
				&& cd == gs.hovered_cell
				&& cd->state == CELL_STATE_UNTOUCHED
				&& IsMouseButtonDown(MOUSE_BUTTON_LEFT)
				&& !gs.finished
			) {
				DrawRectangle(rect_x, rect_y, gs.scale, gs.scale, DARKGRAY);
				continue;
			}
			switch (cd->state)
			{
			case CELL_STATE_REVEALED: {
				// HACK: unique RED background for the bomb causing a game loss
				const Color color = (gs.finished && cd->_bomb && cd->hovered) ? (RED) : (DARKGRAY);
				DrawRectangle(rect_x, rect_y, gs.scale, gs.scale, color);
				if (cd->_bomb)
					goto label_draw_bomb;
				switch (cd->_nearby)
				{
#				define X(NEARBY, COLOR)                                  \
				case NEARBY:                                             \
				        DrawText(#NEARBY, char_x, char_y, gs.scale, COLOR); \
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
				const Color color = (gs.finished && !cd->_bomb) ? (RED) : (ORANGE);
				const char *glyph = (cd->state == CELL_STATE_FLAGGED) ? "F" : "?";
				DrawText(glyph, char_x, char_y, gs.scale, color);
				continue;
			} case CELL_STATE_UNTOUCHED:
				if (gs.finished && cd-> _bomb)
					goto label_draw_bomb;
				continue;
			default:
				UTIL_UNREACHABLE();
			} //end switch (cd->state)
		// FIXME: HAHAHA, turn this label into the function already...
		label_draw_bomb: {
			const int scale_half = (int)((float)(gs.scale) * 0.5f);
			const int bomb_x = gs.scale * (x + gs.opts.border) + scale_half;
			const int bomb_y = gs.scale * (y + gs.opts.border) + scale_half;
			const float bomb_radius = (float)(scale_half) * 0.8f;
			DrawCircle(bomb_x, bomb_y, bomb_radius, DARKPURPLE);
			continue;
		} //end label_draw_bomb
		}

		for (int i = 0; i <= gs.opts.height + 1; ++i) {
			const int pos_x_start = gs.opts.border * gs.scale;
			const int pos_x_end = gs.opts.width * gs.scale + pos_x_start;
			const int pos_y = i * gs.scale;
			DrawLine(pos_x_start, pos_y, pos_x_end, pos_y, DARKGRAY);
		}
		for (int i = 0; i <= gs.opts.width + 1; ++i) {
			const int pos_x = i * gs.scale;
			const int pos_y_start = gs.opts.border * gs.scale;
			const int pos_y_end = gs.opts.height * gs.scale + pos_y_start;
			DrawLine(pos_x, pos_y_start, pos_x, pos_y_end, DARKGRAY);
		}

		if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
			game_hovered_reveal(&gs);

		if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))
			game_hovered_toggle(&gs);

		if (IsKeyPressed(KEY_R))
			gs.started = true;

		EndDrawing();
	}

	CloseWindow();
	cell_destroy(&gs.arr);
	return EXIT_SUCCESS;
}
