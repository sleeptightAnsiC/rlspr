#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include "./cell.h"

//     GAME
//
// Manages and manipulates the GameState.
// GAME does not perform any visualization/drawing on its own,
// and can be run headless. However, it dependes on input system from raylib
// and requires window context to be initialized accordingly.
// GameState and GameOptions also contain few members which the only pusrpose if for visualization,
// but the GAME itself does not do anything with them (other than managing their states).
//
// TODO: perhaps input and any other raylib functionality should be separated from GAME


// TODO: This has been left here in header because it will be reused
#define X_GAME_BINDS                                                       \
        X(IsMouseButtonDown, MOUSE_BUTTON_LEFT, game_hovered_push)         \
        X(IsMouseButtonReleased, MOUSE_BUTTON_LEFT, game_hovered_action_1) \
        X(IsMouseButtonPressed, MOUSE_BUTTON_RIGHT, game_hovered_action_2) \
        X(IsKeyPressed, KEY_ENTER, game_hovered_action_1)                  \
        X(IsKeyPressed, KEY_SPACE, game_hovered_action_2)                  \
        X(IsKeyPressed, KEY_R, game_restart)                               \
        X(IsKeyPressed, KEY_F2, game_restart)                              \

// TODO: this name is too ambigious...
#define GAME_OFFSET_Y(GS_PTR)                           \
        ((GS_PTR)->scale * ((GS_PTR)->opts.border + 1)) \


enum GameStage {
	GAME_STAGE_INITIALIZED,
	GAME_STAGE_STARTED,
	GAME_STAGE_LOST,
	GAME_STAGE_WON,
};

struct GameOptions {
	int width;
	int height;
	int bombs;
	int border;
	bool safe_first_try;
};

struct GameState {
	struct GameOptions opts;
	struct CellArr arr;
	struct CellData *hovered_cell;
	double time_started;
	double time_ended;
	enum GameStage stage;
	int scale;
	int remaining_bombs;
	int hovered_x;
	int hovered_y;
	bool hovered_icon;
	bool hovered_pushed;
};


struct GameState game_init(void);
void game_setup(struct GameState *gs);
void game_free(struct GameState *gs);
void game_replant(struct GameState *gs);
void game_restart(struct GameState *gs);
void game_rehover(struct GameState *gs);
void game_handle_binds(struct GameState *gs);
void game_hovered_action_1(struct GameState *gs);
void game_hovered_action_2(struct GameState *gs);
void game_hovered_push(struct GameState *gs);


#endif

