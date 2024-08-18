
#include <stdlib.h>
#include "raylib.h"
#include "./game.h"
#include "./cell.h"


struct GameState
game_init(void)
{
	// WARN: CellArr has to be initialized because cell_setup expects it
	const struct CellArr arr = { .data = NULL };
	const struct GameState out = { .arr = arr, };
	return out;
}

void
game_setup(struct GameState *gs)
{
	const struct GameOptions opts = {
		.width = 9,
		.height = 9,
		.bombs = 10,
		.border = 1,
		.safe_first_try = true,
	};
	gs->opts = opts,
	gs->scale = 50,
	gs->stage = GAME_STAGE_INITIALIZED,
	gs->hovered_cell = NULL,
	gs->remaining_bombs = opts.bombs,
	gs->hovered_x = -1,
	gs->hovered_y = -1,
	gs->hovered_icon = false,
	gs->hovered_pushed = false,
	game_replant(gs);
}

void
game_rehover(struct GameState *gs)
{
	gs->hovered_pushed = false;
	gs->hovered_x = -1;
	gs->hovered_y = -1;
	gs->hovered_icon = false;
	// HACK: when the game is lost, CellData:hovered is used to highlight the bomb that caused the loss
	if (gs->hovered_cell != NULL && gs->stage != GAME_STAGE_LOST)
		gs->hovered_cell->hovered = false;
	gs->hovered_cell = NULL;

	const int mouse_x = GetMouseX();
	const int mouse_y = GetMouseY();

	{  // check if board icon is being hovered
		// FIXME: code repetition - the same code draws board's icon background
		const int margin = (int)(0.2f * (float)(gs->scale));
		const int x = (gs->scale * gs->opts.border) + ((gs->arr.width + 1) / 2 * gs->scale) - (gs->scale) - margin;
		const int y = (gs->scale * gs->opts.border) + (margin * 2);
		const int w = (gs->scale) + (2 * margin);
		const int h = (gs->scale) + (margin);
		gs->hovered_icon =
			true
			&& (mouse_x >= x)
			&& (mouse_x <= x + w)
			&& (mouse_y >= y)
			&& (mouse_y <= y + h)
			;
	}

	// WARN: early return
	if (gs->hovered_icon)
		return;

	const struct GameOptions o = gs->opts;
	const int offset_y = GAME_OFFSET_Y(gs);
	if ((mouse_x < (o.border + o.width) * gs->scale) && (mouse_x >= o.border * gs->scale))
		gs->hovered_x = (mouse_x - o.border * gs->scale) / gs->scale;
	if ((mouse_y < (o.border + o.height) * gs->scale + offset_y) && (mouse_y >= o.border * gs->scale + offset_y))
		gs->hovered_y = (mouse_y - o.border * gs->scale - offset_y) / gs->scale;

	if (gs->hovered_x != -1 && gs->hovered_y != -1) {
		CELL_GET(&gs->arr, gs->hovered_x, gs->hovered_y, gs->hovered_cell);
		gs->hovered_cell->hovered = true;
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

// TODO: part of this function probably should be moved to cell.c (cell_action_2)
void
game_hovered_action_2(struct GameState *gs)
{
	const bool actionable =
		true
		&& gs->stage != GAME_STAGE_WON
		&& gs->stage != GAME_STAGE_LOST
		&& gs->hovered_cell != NULL
		;

	if (!actionable)
		return;

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

void
game_hovered_action_1(struct GameState *gs)
{
	if (gs->hovered_icon) {
		game_restart(gs);
		return;
	}

	const bool actionable =
		true
		&& gs->stage != GAME_STAGE_WON
		&& gs->stage != GAME_STAGE_LOST
		&& gs->hovered_cell != NULL
		&& (gs->hovered_cell->state == CELL_STATE_REVEALED || gs->hovered_cell->state == CELL_STATE_UNTOUCHED)
		;

	if (!actionable)
		return;

	while (gs->opts.safe_first_try && gs->stage == GAME_STAGE_INITIALIZED && gs->hovered_cell->_nearby != 0)
		game_replant(gs);

	if (gs->stage == GAME_STAGE_INITIALIZED) {
		gs->stage = GAME_STAGE_STARTED;
		gs->time_started = GetTime();
	}

	const bool blown = cell_action_1(&gs->arr, gs->hovered_x, gs->hovered_y);
	if (blown) {
		gs->stage = GAME_STAGE_LOST;
		gs->time_ended = GetTime();
	} else if (gs->remaining_bombs == 0 && gs->arr.untouched_count == 0) {
		gs->stage = GAME_STAGE_WON;
		gs->time_ended = GetTime();
	}
}

void game_handle_binds(struct GameState *gs)
{
	if (false);
#	define X(FUNC, KEY, EVENT) else if (FUNC(KEY)) { EVENT(gs); }
	X_GAME_BINDS
#	undef X
}

void
game_hovered_push(struct GameState *gs)
{
	gs->hovered_pushed = true;
}

void
game_restart(struct GameState *gs)
{
	game_setup(gs);
	game_rehover(gs);
}

void
game_free(struct GameState *gs)
{
	free(gs->arr.data);
	gs->arr.data = NULL;
}

