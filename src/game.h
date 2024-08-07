#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include "./cell.h"

struct GameOptions {
	int width;
	int height;
	int bombs;
	int border;
	bool safe_first_try;
};

struct GameState {
	const struct GameOptions opts;
	struct CellArr arr;
	struct CellData *hovered_cell;
	int hovered_x;
	int hovered_y;
	int scale;
	bool finished;
	bool started;
};

struct GameState game_init(void);
void game_plant(struct GameState *gs);
void game_hover(struct GameState *gs);
void game_hovered_toggle(struct GameState *gs);
void game_hovered_reveal(struct GameState *gs);


#endif

