#include <raylib.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>




#define NUM 10


int
main(void)
{
	int border = 1;
	int scale = 50;
	int width = (NUM + 2) * scale;
	int height = width;


	SetConfigFlags(FLAG_VSYNC_HINT + FLAG_WINDOW_RESIZABLE + FLAG_WINDOW_UNDECORATED + FLAG_MSAA_4X_HINT);
	InitWindow(width, height, "rlspr");
	assert(IsWindowReady());

	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(GRAY);

		// draw cells
		for (int i = 0; i <= NUM + 1; ++i) {
			const int stride = scale * i;
			const int startPos = border * scale;
			const int endPos = NUM * scale + startPos;
			DrawLine(startPos, stride, endPos, stride, DARKGRAY);
			DrawLine(stride, startPos, stride, endPos, DARKGRAY);
		}

		const int mouseX = GetMouseX();
		const int mouseY = GetMouseY();

		// highlight currently hovered cell
		if (
			true
			&& mouseX <= (border + NUM) * scale
			&& mouseX >= border * scale
			&& mouseY <= (border + NUM) * scale
			&& mouseY >= border * scale
		) {
			const int posX = (int)(mouseX / scale) * scale;
			const int posY = (int)(mouseY / scale) * scale;
			DrawRectangleLines(posX, posY, scale, scale, BLACK);
			SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
		} else {
			SetMouseCursor(MOUSE_CURSOR_DEFAULT);
		}

		{
			float mouseWheelMove = GetMouseWheelMove();
			scale += (int)mouseWheelMove;
		}

		// const int x = GetRenderWidth()/2;
		// const int y = GetRenderHeight()/2;

		EndDrawing();
	}
	CloseWindow();

	return 0;
}
