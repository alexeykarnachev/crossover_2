#include "box2d/box2d.h"
#include "core.hpp"
#include <iostream>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

// -----------------------------------------------------------------------
// main
int main() {
    Game game(SCREEN_WIDTH, SCREEN_HEIGHT);
    game.run();
}
