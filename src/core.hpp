#pragma once

#include "raylib.h"
#include <vector>

class Game;
class Dude;
class GameCamera;

// -----------------------------------------------------------------------
// game camera
class GameCamera {
  private:
    float zoom = 1.0;
    Camera2D camera2d;

  public:
    GameCamera(Vector2 screen_size);
    void begin_mode_2d();
    void end_mode_2d();
};

// -----------------------------------------------------------------------
// dude
class Dude {
private:
    Vector2 position;
    float radius;

public:
    Dude(Vector2 position, float radius);
    void update(Game& game);
};

// -----------------------------------------------------------------------
// game
class Game {
  private:
    GameCamera camera;
    std::vector<Dude> dudes;

    float time;
    float dt;

    void update();
    void draw();

    void draw_world();
    void draw_imgui();

  public:
    Game(Vector2 screen_size);
    ~Game();
    void run();
};

