
#include "./draw.h"
#include "./game.h"
#include "./cell.h"
#include "raylib.h"
#include <stdio.h>


// FIXME: rename this
#define X_NUMBERS_COLORS_MAP \
        X(1, BLUE)           \
        X(2, GREEN)          \
        X(3, RED)            \
        X(4, DARKBLUE)       \
        X(5, MAROON)         \
        X(6, SKYBLUE)        \
        X(7, PURPLE)         \
        X(8, DARKPURPLE)     \

#define _CHAR4_FROM_INT(INT_IN)                                                                                 \
        {                                                                                                       \
                ((INT_IN) >= 0) ? (char)('0' + ((INT_IN) / 100 % 10)) : '-',                                    \
                ((INT_IN) >= 0) ? (char)('0' + ((INT_IN) / 10 % 10)) : (char)('0' + ((INT_IN) / 10 % 10 * -1)), \
                ((INT_IN) >= 0) ? (char)('0' + ((INT_IN) % 10)) : (char)('0' + ((INT_IN) % 10 * -1)),           \
                '\0',                                                                                           \
        }                                                                                                       \


static void _draw_bomb(const struct GameState *gs, int x, int y);


void
draw_board(const struct GameState *gs)
{
	const int offset_y = GAME_OFFSET_Y(gs);
	const int margin = (int)(0.2f * (float)(gs->scale));
	const int font_size = gs->scale;

	{  // border
		const int x = (gs->scale * gs->opts.border) + margin;
		const int y = (gs->scale * gs->opts.border) + margin;
		const int w = (gs->arr.width * gs->scale) - (2 * margin);
		const int h = offset_y - (2 * margin);
		DrawRectangleLines(x, y, w, h, DARKGRAY);
	}

	{  // bombs counter
		int time;
		switch (gs->stage) {
		case GAME_STAGE_INITIALIZED:
			time = 0;
			break;
		case GAME_STAGE_STARTED:
			time = (int)(GetTime() - gs->time_started);
			break;
		case GAME_STAGE_WON:
		case GAME_STAGE_LOST:
			time = (int)(gs->time_ended - gs->time_started);
			break;
		default:
			UTIL_UNREACHABLE();
		}
		UTIL_ASSERT(time >= 0);
		const char text[4] = _CHAR4_FROM_INT(time);
		const int x = (gs->arr.width + gs->opts.border - 2) * gs->scale - margin;
		const int y = (gs->scale * gs->opts.border) + (margin * 3);
		DrawText(text, x, y, font_size, RED);
	}

	{  // icon background
		// FIXME: code repetition - the same code checks whenever icon is being hovered
		const int x = (gs->scale * gs->opts.border) + ((gs->arr.width + 1) / 2 * gs->scale) - (gs->scale) - margin;
		const int y = (gs->scale * gs->opts.border) + (margin * 2);
		const int w = (gs->scale) + (2 * margin);
		const int h = (gs->scale) + (margin);
		if (gs->hovered_icon && gs->hovered_pushed)
			DrawRectangle(x, y, w, h, DARKGRAY);
		else
			DrawRectangleLines(x, y, w, h, DARKGRAY);
	}

	{  // icon
		const char *icon;
		if (gs->hovered_icon && gs->hovered_pushed)
			icon = ":-)";
		else if (gs->stage == GAME_STAGE_WON)
			icon = "B)";
		else if (gs->stage == GAME_STAGE_LOST)
			icon = "X(";
		else if (gs->hovered_pushed)
			icon = ":-0";
		else
			icon = ":-)";
		const int x = (gs->scale * gs->opts.border) + ((gs->arr.width + 1) / 2 * gs->scale) - (gs->scale);
		const int y = (gs->scale * gs->opts.border) + (margin * 2) + margin;
		DrawText(icon, x, y, font_size, YELLOW);
	}

	{  // timer
		const char text[4] = _CHAR4_FROM_INT(gs->remaining_bombs);
		const int x = (gs->scale * gs->opts.border) + (gs->scale / 2);
		const int y = (gs->scale * gs->opts.border) + (margin * 3);
		DrawText(text, x, y, font_size, RED);
	}
}

void
draw_borders(const struct GameState *gs)
{
	const int offset_y = GAME_OFFSET_Y(gs);
	for (int i = 1; i <= gs->opts.height + 1; ++i) {
		const int pos_x_start = gs->opts.border * gs->scale;
		const int pos_x_end = gs->opts.width * gs->scale + pos_x_start;
		const int pos_y = i * gs->scale + offset_y;
		DrawLine(pos_x_start, pos_y, pos_x_end, pos_y, DARKGRAY);
	}
	for (int i = 0; i <= gs->opts.width + 1; ++i) {
		const int pos_x = i * gs->scale;
		const int pos_y_start = gs->opts.border * gs->scale + offset_y;
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

		const int offset_y = GAME_OFFSET_Y(gs);
		const int char_x = gs->scale * (x + gs->opts.border) + (int)(0.3f * (float)(gs->scale));
		const int char_y = gs->scale * (y + gs->opts.border) + (int)(0.1f * (float)(gs->scale)) + offset_y;
		const int rect_x = (gs->opts.border + x) * gs->scale;
		const int rect_y = (gs->opts.border + y) * gs->scale + offset_y;

		switch (cd->state)
		{
		case CELL_STATE_REVEALED: {

			// HACK: unique RED background for the bomb that caused a game loss
			const Color color = (gs->stage == GAME_STAGE_LOST && cd->_bomb && cd->hovered) ? (RED) : (DARKGRAY);
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
			const char *glyph = (cd->state == CELL_STATE_FLAGGED) ? "F" : "?";
			DrawText(glyph, char_x, char_y, gs->scale, BLACK);
			continue;
		} case CELL_STATE_UNTOUCHED: {
			if (gs->stage == GAME_STAGE_LOST && cd-> _bomb)
				_draw_bomb(gs, x, y);
			else if (cd == gs->hovered_cell && gs->hovered_pushed)
				DrawRectangle(rect_x, rect_y, gs->scale, gs->scale, DARKGRAY);
			continue;
		} default:
			UTIL_UNREACHABLE();
		} //end switch
	}
}

static void
_draw_bomb(const struct GameState *gs, int x, int y)
{
	const int offset_y = GAME_OFFSET_Y(gs);
	const int scale_half = (int)((float)(gs->scale) * 0.5f);
	const int bomb_x = gs->scale * (x + gs->opts.border) + scale_half;
	const int bomb_y = gs->scale * (y + gs->opts.border) + scale_half + offset_y;
	const float bomb_radius = (float)(scale_half) * 0.8f;
	DrawCircle(bomb_x, bomb_y, bomb_radius, DARKPURPLE);
}


