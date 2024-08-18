# rlspr
Simple clone of classic Minesweeper game.\
Created with Raylib, written in C99. \
Try it out at: https://stacc.itch.io/rlspr

# BUILDING
Requires either POSIX or MinGW enviroment, with GNU/make and git:
```sh
$ git clone https://github.com/sleeptightAnsiC/rlspr --depth 1 --recursive --recurse-submodules --shallow-submodules
$ cd rlspr
$ make
```

# GOALs
- Trying out raylib and testing its portability, building it for multiple systems and WEB (initial goal)
- Imlementing most of the features from other Minesweeper clones and the original ones
- Keeping the code as small as possible (if some feature would require a lot of code changes, it's out of the scope)

# NON-GOALs
- Beeter looks (polished functionality is the goal, not polished visuals)
- Proper menus for changing options, kebindings, themes, colors, etc. (although, I may provide simple intermediate GUI for changing difficulty, see TODOs)

# NOTEs
- I implemented everything from the scratch, without any tutorials or guides. This my take on Minesweeper, I don't know how other clones work.
- Although, I did NOT peek at any Minesweeper's implementation details, I refered to this page for the design details: https://minesweeper-pro.com/introduction/

# TODOs
- provide UI for changing difficulty and and few core options (functionality is already there, just front-end is missing)
- provide automatic resolution scalling
- (in far future) replace current draw pass with GLSL shader
- (in far future) once again, try to build raylib through raw C code, without using Makefile

# FIXMEs
- `make CFLAGS=''` will cause raylib build to fail (this is more of an issue with raylib's Makefile)
- Although, I haven't prepered any mobile support, the WEB version can be started from mobile web browser, but it will NOT work properly. This should be either prevented or fixed by implementing gestures.
- If a flag/question mark gets placed before starting the game, it will disappear upon revealing the first cell (this is caused by "safe first try" feature and there are multiple ways to fix it)

# LICENSE
Copyright holder: https://github.com/sleeptightAnsiC \
Code under this repository is licensed under permisive MIT/X license, \
but this does NOT include its dependencies and git-submodules, such as raylib. \
This repository contains the proper copy of License formula.
