# Pong

It's just Pong. In a terminal. Yes it's a TUI. 

## Installation (macOS)
```bash
brew install ncurses sdl2 sdl2_mixer
```

## Building and Running
```bash
g++ main.cpp -o game -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib -lSDL2 -lSDL2_mixer -lncurses -std=c++11 && ./game

```

## Controls
- ← → : Move paddle left/right
- P : Pause/Unpause
- Q : Quit
