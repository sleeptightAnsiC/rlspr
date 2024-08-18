
#ifndef DRAW_H
#define DRAW_H

struct GameState;


//     DRAW
//
// Simply: takes the GameState and draws it to the screen context.
// DRAW translation unit does not modify the GameState itself,
//     it only references it in read-only manner, BUT without any validation.
// GameState is expected to be properly set before being passed to DRAW


void draw_board(const struct GameState *gs);
void draw_cells(const struct GameState *gs);
void draw_borders(const struct GameState *gs);


#endif //DRAW_H

