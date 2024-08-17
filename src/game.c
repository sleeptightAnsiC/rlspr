
#include <stdlib.h>
#include "raylib.h"
#include "./game.h"
#include "./cell.h"


struct GameState
game_init(void)
{
	const struct GameOptions opts = {
		.width = 9,
		.height = 9,
		.bombs = 10,
		.border = 1,
		.safe_first_try = true,
	};
	const struct CellArr arr = { .data = NULL };
	struct GameState out = {
		.opts = opts,
		.arr = arr,
		.scale = 50,
		.stage = GAME_STAGE_INITIALIZED,
		.hovered_cell = NULL,
		.remaining_bombs = opts.bombs,
		.hovered_x = -1,
		.hovered_y = -1,
		.hovered_pushed = false,
	};
	game_replant(&out);
	return out;
}

void
game_rehover(struct GameState *gs)
{
	if (gs->stage == GAME_STAGE_WON || gs->stage == GAME_STAGE_LOST)
		return;

	if (gs->hovered_cell != NULL) {
		gs->hovered_cell->hovered = false;
		gs->hovered_pushed = false;
	}
	gs->hovered_x = -1;
	gs->hovered_y = -1;

	const int mouse_x = GetMouseX();
	const int mouse_y = GetMouseY();
	const struct GameOptions o = gs->opts;

	const int offset_y = GAME_OFFSET_Y(gs);
	if ((mouse_x < (o.border + o.width) * gs->scale) && (mouse_x >= o.border * gs->scale))
		gs->hovered_x = (mouse_x - o.border * gs->scale) / gs->scale;
	if ((mouse_y < (o.border + o.height) * gs->scale + offset_y) && (mouse_y >= o.border * gs->scale + offset_y))
		gs->hovered_y = (mouse_y - o.border * gs->scale - offset_y) / gs->scale;

	if (gs->hovered_x != -1 && gs->hovered_y != -1) {
		CELL_GET(&gs->arr, gs->hovered_x, gs->hovered_y, gs->hovered_cell);
		gs->hovered_cell->hovered = true;
	} else {
		gs->hovered_cell = NULL;
	}
}

void
game_replant(struct GameState *gs)
{
	struct CellArr *arr = &gs->arr;
	cell_setup(arr, gs->opts.width, gs->opts.height);
	// WARN: notice that 'i' increments only when bomb has been planted
	for (int i = 0; i < gs->opts.bombs;) {
		const int x = GetRandomValue(0, gs->opts.width - 1);
		const int y = GetRandomValue(0, gs->opts.height - 1);
		struct CellData *cd;
		CELL_GET(arr, x, y, cd);
		if (cd->_bomb)
			continue;
		cell_plant(arr, x, y);
		++i;
	}
}

// FIXME: part of this function probably should be moved to cell.c (cell_toggle)
void
game_hovered_toggle(struct GameState *gs)
{
	if ( true
		&& gs->stage != GAME_STAGE_WON
		&& gs->stage != GAME_STAGE_LOST
		&& gs->hovered_cell != NULL
	) {
		struct CellData *cd;
		CELL_GET(&gs->arr, gs->hovered_x, gs->hovered_y, cd);
		switch (cd->state)
		{
		case CELL_STATE_UNTOUCHED:
			cd->state = CELL_STATE_FLAGGED;
			--(gs->remaining_bombs);
			--(gs->arr.untouched_count);
			if (gs->remaining_bombs == 0 && gs->arr.untouched_count == 0) {
				gs->stage = GAME_STAGE_WON;
				gs->time_ended = GetTime();
			}
			break;
		case CELL_STATE_FLAGGED:
			cd->state = CELL_STATE_QUESTIONED;
			++(gs->remaining_bombs);
			break;
		case CELL_STATE_QUESTIONED:
			cd->state = CELL_STATE_UNTOUCHED;
			++(gs->arr.untouched_count);
			break;
		case CELL_STATE_REVEALED:
			// do nothing if it's already revealed
			break;
		default:
			UTIL_UNREACHABLE();
		}
	}
}

void
game_hovered_reveal(struct GameState *gs)
{
	if ( true
		&& gs->stage != GAME_STAGE_WON
		&& gs->stage != GAME_STAGE_LOST
		&& gs->hovered_cell != NULL
		&& (gs->hovered_cell->state == CELL_STATE_REVEALED || gs->hovered_cell->state == CELL_STATE_UNTOUCHED)
	) {
		while (gs->opts.safe_first_try && gs->stage == GAME_STAGE_INITIALIZED && gs->hovered_cell->_nearby != 0)
			game_replant(gs);
		if (gs->stage == GAME_STAGE_INITIALIZED) {
			gs->stage = GAME_STAGE_STARTED;
			gs->time_started = GetTime();
		}
		const bool blown = cell_reveal(&gs->arr, gs->hovered_x, gs->hovered_y);
		if (blown) {
			gs->stage = GAME_STAGE_LOST;
			gs->time_ended = GetTime();
		} else if (gs->remaining_bombs == 0 && gs->arr.untouched_count == 0) {
			gs->stage = GAME_STAGE_WON;
			gs->time_ended = GetTime();
		}
	}
}

void
game_hovered_push(struct GameState *gs)
{
	if ( true
		&& gs->stage != GAME_STAGE_WON
		&& gs->stage != GAME_STAGE_LOST
		&& gs->hovered_cell != NULL
		&& gs->hovered_cell->state == CELL_STATE_UNTOUCHED
	) {
		gs->hovered_pushed = true;
	}
}

void
game_restart(struct GameState *gs)
{
	gs->stage = GAME_STAGE_INITIALIZED;
	game_replant(gs);
}

void
game_deinit(struct GameState *gs)
{
	free(gs->arr.data);
	gs->arr.data = NULL;
}

