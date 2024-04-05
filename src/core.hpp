#pragma once

#include "box2d/b2_collision.h"
#include <vector>
#include "box2d/b2_body.h"
#include "box2d/b2_world.h"
#include "raylib.h"

class Game;
class Dude;
class GameCamera;

// -----------------------------------------------------------------------
// game camera
class GameCamera {
  private:
    float zoom = 50.0;
    Camera2D camera2d;

  public:
    GameCamera();
    GameCamera(Vector2 screen_size);
    void begin_mode_2d();
    void end_mode_2d();
};

// -----------------------------------------------------------------------
// dude
class Dude {
private:
    b2Body* body;
    float speed;

public:
    Dude(b2Body* body, float speed);
    void update(Game& game);

    Vector2 get_body_position();
    float get_body_radius();
};

// -----------------------------------------------------------------------
// game
class Game {
  private:
    b2World b2_world;

    GameCamera camera;
    std::vector<Dude> dudes;

    float dt = 0.0;
    float time = 0.0;
    float timestep = 1.0 / 60.0;

    void update();
    void step();
    void draw();

    void draw_world();
    void draw_imgui();

  public:
    Game(Vector2 screen_size);
    ~Game();
    void run();

    void spawn_dude(float x, float y, float speed);
};

