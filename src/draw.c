
#include "./draw.h"
#include "./game.h"
#include "./cell.h"
#include "./raylib.h"


#define X_NUMBERS_COLORS_MAP \
        X(1, BLUE)           \
        X(2, GREEN)          \
        X(3, RED)            \
        X(4, DARKBLUE)       \
        X(5, MAROON)         \
        X(6, SKYBLUE)        \
        X(7, PURPLE)         \
        X(8, DARKPURPLE)     \


static void _draw_bomb(const struct GameState *gs, int x, int y);


void
draw_borders(const struct GameState *gs)
{
	for (int i = 0; i <= gs->opts.height + 1; ++i) {
		const int pos_x_start = gs->opts.border * gs->scale;
		const int pos_x_end = gs->opts.width * gs->scale + pos_x_start;
		const int pos_y = i * gs->scale;
		DrawLine(pos_x_start, pos_y, pos_x_end, pos_y, DARKGRAY);
	}
	for (int i = 0; i <= gs->opts.width + 1; ++i) {
		const int pos_x = i * gs->scale;
		const int pos_y_start = gs->opts.border * gs->scale;
		const int pos_y_end = gs->opts.height * gs->scale + pos_y_start;
		DrawLine(pos_x, pos_y_start, pos_x, pos_y_end, DARKGRAY);
	}
}

void
draw_cells(const struct GameState *gs)
{
	for (int x = 0; x < gs->arr.width; ++x) for (int y = 0; y < gs->arr.height; ++y)
	{
		const struct CellData *cd;
		CELL_GET(&gs->arr, x, y, cd);

		const int char_x = gs->scale * (x + gs->opts.border) + (int)(0.3f * (float)(gs->scale));
		const int char_y = gs->scale * (y + gs->opts.border) + (int)(0.1f * (float)(gs->scale));
		const int rect_x = (gs->opts.border + x) * gs->scale;
		const int rect_y = (gs->opts.border + y) * gs->scale;

		if (
			true
			&& cd == gs->hovered_cell
			&& gs->hovered_pushed
		) {
			DrawRectangle(rect_x, rect_y, gs->scale, gs->scale, DARKGRAY);
			continue;
		}

		switch (cd->state)
		{
		case CELL_STATE_REVEALED: {

			// HACK: unique RED background for the bomb causing a game loss
			const Color color = (gs->finished && cd->_bomb && cd->hovered) ? (RED) : (DARKGRAY);
			DrawRectangle(rect_x, rect_y, gs->scale, gs->scale, color);

			if (cd->_bomb) {
				_draw_bomb(gs, x, y);
				continue;
			}

			switch (cd->_nearby)
			{
#				define X(NEARBY, COLOR)                              \
			case NEARBY:                                                 \
				DrawText(#NEARBY, char_x, char_y, gs->scale, COLOR); \
				continue;                                            \
				;                                                    
				X_NUMBERS_COLORS_MAP
#				undef X
			case 0:
				continue;
			default:
				UTIL_UNREACHABLE();
			}

			continue;

		} case CELL_STATE_QUESTIONED: {
		} case CELL_STATE_FLAGGED: {
			const Color color = (gs->finished && !cd->_bomb) ? (RED) : (ORANGE);
			const char *glyph = (cd->state == CELL_STATE_FLAGGED) ? "F" : "?";
			DrawText(glyph, char_x, char_y, gs->scale, color);
			continue;
		} case CELL_STATE_UNTOUCHED: {
			if (gs->finished && cd-> _bomb)
				_draw_bomb(gs, x, y);
			continue;
		} default:
			UTIL_UNREACHABLE();
		} //end switch
	}
}

static void
_draw_bomb(const struct GameState *gs, int x, int y)
{
	const int scale_half = (int)((float)(gs->scale) * 0.5f);
	const int bomb_x = gs->scale * (x + gs->opts.border) + scale_half;
	const int bomb_y = gs->scale * (y + gs->opts.border) + scale_half;
	const float bomb_radius = (float)(scale_half) * 0.8f;
	DrawCircle(bomb_x, bomb_y, bomb_radius, DARKPURPLE);
}


