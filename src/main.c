#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "raylib.h"
#include "./util.h"
#include "./cell.h"
#include "./game.h"
#include "./draw.h"


int
main(void)
{
	struct GameState gs = game_init();

	{
		SetConfigFlags(FLAG_VSYNC_HINT + FLAG_WINDOW_RESIZABLE);
		const int win_w = (gs.opts.width + (gs.opts.border * 2)) * gs.scale;
		const int win_h = (gs.opts.height + (gs.opts.border * 2)) * gs.scale + GAME_OFFSET_Y(&gs);
		InitWindow(win_w, win_h, "rlspr");
		UTIL_ASSERT(IsWindowReady());
	}

	while (!WindowShouldClose())
	{
		game_rehover(&gs);

#		define X(FUNC, KEY, EVENT) \
		if (FUNC(KEY)) EVENT(&gs)  \
		;                           
		X_GAME_BINDS
#		undef X

		BeginDrawing();
		ClearBackground(GRAY);
		draw_cells(&gs);
		draw_borders(&gs);
		EndDrawing();
	}

	game_deinit(&gs);
	CloseWindow();
	return EXIT_SUCCESS;
}
