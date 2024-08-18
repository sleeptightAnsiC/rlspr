#include <stdlib.h>
#include "raylib.h"
#include "./game.h"
#include "./draw.h"


int
main(void)
{
	struct GameState gs = game_init();

	{
		SetConfigFlags(FLAG_VSYNC_HINT + FLAG_WINDOW_RESIZABLE);
		const int w = (gs.opts.width + (gs.opts.border * 2)) * gs.scale;
		const int h = (gs.opts.height + (gs.opts.border * 2)) * gs.scale + GAME_OFFSET_Y(&gs);
		InitWindow(w, h, "rlspr");
	}

	while (!WindowShouldClose())
	{
		game_rehover(&gs);
		game_handle_binds(&gs);
		BeginDrawing();
			ClearBackground(GRAY);
			draw_board(&gs);
			draw_cells(&gs);
			draw_borders(&gs);
		EndDrawing();
	}

	CloseWindow();
	game_deinit(&gs);
	return EXIT_SUCCESS;
}
