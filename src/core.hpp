#pragma once

#include "box2d/b2_collision.h"
#include "box2d/b2_math.h"
#include <tuple>
#include <vector>
#include "box2d/b2_body.h"
#include "box2d/b2_world.h"
#include "raylib.h"

class Game;
class Dude;
class Eyes;
class GameCamera;

// -----------------------------------------------------------------------
// game camera
class GameCamera {
  private:
    float zoom = 50.0;
    Camera2D camera2d;

  public:
    GameCamera();
    GameCamera(int screen_width, int screen_height);
    void begin_mode_2d();
    void end_mode_2d();
};

// -----------------------------------------------------------------------
// dude
class Dude {
private:
    b2Body* body;

    float speed;

    // eyes
    int n_view_rays;
    float view_angle;
    float view_distance;

public:
    Dude(b2Body* body, float speed, int n_view_rays, float view_angle, float view_distance);
    void update(Game& game);

    b2Vec2 get_body_position();
    float get_body_radius();
    float get_body_angle();

    std::vector<b2Vec2> get_view_ray_end_points();
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
    Game(int screen_width, int screen_height);
    ~Game();
    void run();

    void spawn_dude(b2Vec2 position);
};

