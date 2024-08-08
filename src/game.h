#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include "./cell.h"

#define X_GAME_BINDS                                                      \
        X(IsMouseButtonDown, MOUSE_BUTTON_LEFT, game_hovered_push)        \
        X(IsMouseButtonReleased, MOUSE_BUTTON_LEFT, game_hovered_reveal)  \
        X(IsMouseButtonPressed, MOUSE_BUTTON_RIGHT, game_hovered_toggle)  \
        X(IsKeyPressed, KEY_ENTER, game_hovered_reveal)                   \
        X(IsKeyPressed, KEY_SPACE, game_hovered_toggle)                   \
        X(IsKeyPressed, KEY_R, game_restart)                              \
        X(IsKeyPressed, KEY_F2, game_restart)                             \


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
	bool hovered_pushed;
};


struct GameState game_init(void);
void game_plant(struct GameState *gs);
void game_restart(struct GameState *gs);
void game_hover(struct GameState *gs);
void game_hovered_toggle(struct GameState *gs);
void game_hovered_reveal(struct GameState *gs);
void game_hovered_push(struct GameState *gs);


#endif

