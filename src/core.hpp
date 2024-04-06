#pragma once

#include "box2d/b2_collision.h"
#include "box2d/b2_math.h"
#include <tuple>
#include <vector>
#include "box2d/b2_body.h"
#include "box2d/b2_world.h"
#include "raylib.h"

enum class DudeType;

class Game;
class Dude;
class GameCamera;

// -----------------------------------------------------------------------
// game camera
class GameCamera {
  public:
    float zoom = 50.0;
    Camera2D camera2d;

    GameCamera();
    GameCamera(int screen_width, int screen_height);
    void begin_mode_2d();
    void end_mode_2d();
};

// -----------------------------------------------------------------------
// dude controller
enum class DudeType {
    PLAYER
};

// -----------------------------------------------------------------------
// dude
class Dude {
public:
    DudeType type;
    b2Body* body;

    float move_speed;
    float rotate_speed;

    // eyes
    int n_view_rays;
    float view_angle;
    float view_distance;

    Dude(
        DudeType type,
        b2Body* body,
        float move_speed,
        float rotate_speed,
        int n_view_rays,
        float view_angle,
        float view_distance
    );
    void update(Game& game);

    void draw_debug();

    std::vector<b2Vec2> get_view_ray_end_points();
};

// -----------------------------------------------------------------------
// game
class Game {
  public:
    b2World b2_world;

    GameCamera camera;
    std::vector<Dude> dudes;

    float dt = 0.0;
    float time = 0.0;
    float timestep = 1.0 / 60.0;

    Game(int screen_width, int screen_height);
    ~Game();
    void run();

    void update();
    void step();
    void draw();

    void draw_world();
    void draw_imgui();

    void spawn_dude(b2Vec2 position);
};

