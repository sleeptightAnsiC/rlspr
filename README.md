# TODOs
- add board (with emoji face, bomb count and time) on the top edge of the window
- those files has been taken from mtpl project, clean them up later
- fix issues with building Raylib while using: make CFLAGS=''
- I don't like the fact that everything uses ambigious 'int'. This is because raylib's API uses it everywhere but I should probably take care of it
- (in far future) once again, try to build raylib through raw C code, without using Makefile
- (in far future) replace current draw pass with GLSL shader


# NOTEs
- https://minesweeper-pro.com/introduction/
- https://en.wikipedia.org/wiki/Minesweeper_(video_game)
- https://en.wikipedia.org/wiki/Microsoft_Minesweeper


# rlspr
Simple (but faithful) clone of classic WindowsNT's Minesweeper game.\
Created with Raylib, written in C99 - builds and runs everywhere, on everything!


# context (why rlspr has been made)
I was looking for a reason to try Raylib for a long long time and recently,
while I was scattering through WINE's directories on Linux,
I've spotted a similar clone there - `C:\windows\system32\winemine.exe` -
this trully inspired me to give it a try.

In polish Windows translations this game is called "Saper" (with one "p"),
that's why the project's name is `rlspr` as it stands for "Raylib's Saper".
