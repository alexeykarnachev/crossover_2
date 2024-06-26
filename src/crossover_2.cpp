#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>

#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "raylib.h"
#include "raymath.h"

#include "geometry.hpp"
#include "list.hpp"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define TARGET_FPS 60

#define WORLD_TIMESTEP (1.0 / 60.0)
#define MAX_N_DUDES 16
#define MAX_N_BULLETS 256
#define MAX_N_OBSTACLES 256
#define DEFAULT_BULLET_TTL 2.0
#define DEFAULT_BULLET_SPEED 50.0
#define DEFAULT_BULLET_DAMAGE 1.0
#define DEFAULT_DUDE_RADIUS 1.0
#define DEFAULT_DUDE_MAX_HEALTH 5.0
#define DEFAULT_DUDE_MOVE_SPEED 10.0
#define DEFAULT_DUDE_FIRE_RATE 5.0
#define DEFAULT_DUDE_VIEW_DISTANCE 10.0
#define DEFAULT_DUDE_VIEW_ANGLE (DEG2RAD * 75.0)
#define DEFAULT_DUDE_N_VIEW_RAYS 32

class World;

enum class AIType {
    NONE,
    MANUAL,
    DUMMY,
};

enum class ViewRayTarget {
    NONE,
    DUDE,
    OBSTACLE,
};

class ViewRayInfo {
  public:
    Vector2 origin;
    Vector2 end_point;
    ViewRayTarget target = ViewRayTarget::NONE;
    float dist = FLT_MAX;

    ViewRayInfo() = default;

    void reset(Vector2 origin, Vector2 end_point) {
        this->origin = origin;
        this->end_point = end_point;
        this->dist = FLT_MAX;
        this->target = ViewRayTarget::NONE;
    }

    void hit(Vector2 hit_position, ViewRayTarget target) {
        float dist = Vector2Distance(hit_position, this->origin);
        if (this->target == ViewRayTarget::NONE || dist < this->dist) {
            this->end_point = hit_position;
            this->dist = dist;
            this->target = target;
        }
    }
};

class Dude {
  public:
    AIType ai_type;
    float body_radius = DEFAULT_DUDE_RADIUS;
    float max_health = DEFAULT_DUDE_MAX_HEALTH;
    float move_speed = DEFAULT_DUDE_MOVE_SPEED;
    float fire_rate = DEFAULT_DUDE_FIRE_RATE;

    float view_distance = DEFAULT_DUDE_VIEW_DISTANCE;
    float view_angle = DEFAULT_DUDE_VIEW_ANGLE;
    int n_view_rays = DEFAULT_DUDE_N_VIEW_RAYS;
    std::array<ViewRayInfo, MAX_N_RAYS_IN_RAYS_FAN> view_ray_infos;

    Vector2 position;
    float orientation = 0.0;
    float health = 0.0;
    float last_shot_time = -FLT_MAX;

    Dude() = default;

    Dude(Vector2 position, AIType ai_type) {
        this->ai_type = ai_type;
        this->position = position;
        this->health = this->max_health;
    };

    void update(World &world);
    void draw();
};

class Bullet {
  public:
    Vector2 prev_position;
    Vector2 curr_position;
    Vector2 velocity;
    Dude *owner = NULL;
    float ttl = 0.0;
    float damage = DEFAULT_BULLET_DAMAGE;

    Bullet() = default;

    Bullet(Vector2 position, Vector2 velocity, Dude *owner) {
        this->prev_position = position;
        this->curr_position = position;
        this->velocity = velocity;
        this->owner = owner;
        this->ttl = DEFAULT_BULLET_TTL;
    };

    void update(World &world);
    void draw();
};

class Obstacle {
  public:
    Rectangle rect;

    Obstacle(){};
    Obstacle(Rectangle rect) {
        this->rect = rect;
    }

    void draw();
};

class GameCamera {
  public:
    float zoom = 25.0;
    Camera2D camera2d;

    GameCamera() = default;
    GameCamera(int screen_width, int screen_height) {
        this->camera2d = {
            .offset = {0.5f * screen_width, 0.5f * screen_height},
            .target = {0.0, 0.0},
            .rotation = 0.0,
            .zoom = this->zoom};
    }
};

class World {
  public:
    float timestep = WORLD_TIMESTEP;
    float time = 0.0;

    List<Dude, MAX_N_DUDES> dudes;
    List<Bullet, MAX_N_BULLETS> bullets;
    List<Obstacle, MAX_N_OBSTACLES> obstacles;

    GameCamera camera;

    World(){};
    ~World(){};

    void update() {
        this->time += this->timestep;

        for (Dude &dude : this->dudes) {
            dude.update(*this);
        }

        for (Bullet &bullet : this->bullets) {
            bullet.update(*this);
        }
    }

    void spawn_dude(Dude dude) {
        if (!this->dudes.insert(dude)) {
            throw std::runtime_error("ERROR: Can't spawn more dudes");
        }
    }

    void spawn_bullet(Bullet bullet) {
        if (!this->bullets.insert(bullet)) {
            fprintf(stderr, "WARNING: Can't spawn more bullets");
        }
    }

    void spawn_obstacle(Obstacle obstacle) {
        if (!this->obstacles.insert(obstacle)) {
            throw std::runtime_error("ERROR: Can't spawn more obstacles");
        }
    }
};

class Renderer {
  public:
    Renderer(int screen_width, int screen_height) {
        SetConfigFlags(FLAG_MSAA_4X_HINT);
        SetTargetFPS(TARGET_FPS);
        InitWindow(screen_width, screen_height, "crossover_2");

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        GLFWwindow *window = (GLFWwindow *)GetWindowHandle();
        glfwGetWindowUserPointer(window);
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 420 core");
        ImGui::StyleColorsDark();
    }

    ~Renderer() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        CloseWindow();
    }

    void draw(World &world) {
        BeginDrawing();
        ClearBackground({10, 10, 10, 255});

        BeginMode2D(world.camera.camera2d);

        for (Dude &dude : world.dudes) {
            dude.draw();
        }

        for (Bullet &bullet : world.bullets) {
            bullet.draw();
        }

        for (Obstacle &obstacle : world.obstacles) {
            obstacle.draw();
        }

        EndMode2D();

        DrawFPS(0, 0);
        EndDrawing();
    }
};

void Dude::update(World &world) {
    if (this->health <= 0.0) {
        world.dudes.remove(*this);
        return;
    }

    // update controls
    switch (this->ai_type) {
        case AIType::MANUAL: {
            Vector2 dir = Vector2Zero();
            if (IsKeyDown(KEY_W)) dir.y -= 1.0;
            if (IsKeyDown(KEY_S)) dir.y += 1.0;
            if (IsKeyDown(KEY_A)) dir.x -= 1.0;
            if (IsKeyDown(KEY_D)) dir.x += 1.0;

            if (dir.x != 0.0 || dir.y != 0.0) {
                float dist = world.timestep * this->move_speed;
                Vector2 step = Vector2Scale(Vector2Normalize(dir), dist);
                this->position = Vector2Add(this->position, step);
            }

            Vector2 look_at = GetScreenToWorld2D(
                GetMousePosition(), world.camera.camera2d
            );
            this->orientation = get_vec_orientation(
                Vector2Subtract(look_at, this->position)
            );

            bool is_shot = IsMouseButtonDown(MOUSE_LEFT_BUTTON)
                           && (world.time - this->last_shot_time)
                                  >= 1.0 / this->fire_rate;
            if (is_shot) {
                Vector2 bullet_velocity = Vector2Scale(
                    get_orientation_vec(this->orientation), DEFAULT_BULLET_SPEED
                );
                world.spawn_bullet({this->position, bullet_velocity, this});
                this->last_shot_time = world.time;
            }

            break;
        }
        case AIType::DUMMY: {
            this->position.x -= world.timestep * this->move_speed * 0.1;
        }
        default: break;
    }

    // -------------------------------------------------------------------
    // update collisions
    for (Obstacle &obstacle : world.obstacles) {
        Vector2 mtv = get_circle_rect_mtv(
            this->position, this->body_radius, obstacle.rect
        );
        this->position = Vector2Add(this->position, mtv);
    }

    for (Dude &dude : world.dudes) {
        if (&dude == this) continue;
        Vector2 mtv = get_circle_circle_mtv(
            this->position, this->body_radius, dude.position, dude.body_radius
        );
        this->position = Vector2Add(this->position, mtv);
    }

    // -------------------------------------------------------------------
    // update view ray infos
    RaysFan view_rays_fan = get_rays_fan(
        this->position,
        this->n_view_rays,
        this->view_distance,
        this->view_angle,
        this->orientation
    );
    Vector2 hit_position;
    for (int i = 0; i < view_rays_fan.n; ++i) {
        Vector2 start = view_rays_fan.start;
        Vector2 end = view_rays_fan.end[i];

        ViewRayInfo &info = this->view_ray_infos[i];
        info.reset(this->position, end);

        for (Obstacle &obstacle : world.obstacles) {
            if (get_line_rect_intersection_nearest(
                    start, end, obstacle.rect, &hit_position
                )) {
                info.hit(hit_position, ViewRayTarget::OBSTACLE);
            }
        }

        for (Dude &dude : world.dudes) {
            if (&dude == this) continue;

            if (get_line_circle_intersection_nearest(
                    start, end, dude.position, dude.body_radius, &hit_position
                )) {
                info.hit(hit_position, ViewRayTarget::DUDE);
            }
        }
    }
}

void Bullet::update(World &world) {
    this->ttl -= world.timestep;
    if (this->ttl <= 0.0) {
        world.bullets.remove(*this);
        return;
    }

    Vector2 step = Vector2Scale(this->velocity, world.timestep);
    this->prev_position = this->curr_position;
    this->curr_position = Vector2Add(this->curr_position, step);

    // resolve collisions with obstacles
    for (Obstacle &obstacle : world.obstacles) {
        Vector2 intersection;
        bool is_hit = get_line_rect_intersection_nearest(
            this->prev_position, this->curr_position, obstacle.rect, &intersection
        );
        if (is_hit) {
            world.bullets.remove(*this);
        }
    }

    // resolve collisions with dudes
    for (Dude &dude : world.dudes) {
        if (&dude == this->owner) continue;

        Vector2 intersection;
        bool is_hit = get_line_circle_intersection_nearest(
            this->prev_position,
            this->curr_position,
            dude.position,
            dude.body_radius,
            &intersection
        );
        if (is_hit) {
            dude.health -= this->damage;
            world.bullets.remove(*this);
        }
    }
}

void Dude::draw() {
    DrawCircleV(this->position, this->body_radius, RAYWHITE);

    for (int i = 0; i < this->n_view_rays; ++i) {
        ViewRayInfo info = this->view_ray_infos[i];
        DrawLineV(this->position, info.end_point, GREEN);
        if (info.target == ViewRayTarget::OBSTACLE) {
            DrawCircleV(info.end_point, 0.2, BLUE);
        } else if (info.target == ViewRayTarget::DUDE) {
            DrawCircleV(info.end_point, 0.2, RED);
        }
    }
}

void Bullet::draw() {
    DrawLineV(this->prev_position, this->curr_position, YELLOW);
    DrawCircleV(this->curr_position, 0.1, ORANGE);
}

void Obstacle::draw() {
    DrawRectangleRec(this->rect, {50, 50, 50, 255});
}

void start_game() {
    Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT);

    World world;
    world.camera = GameCamera(SCREEN_WIDTH, SCREEN_HEIGHT);

    world.spawn_dude({{0.0, 0.0}, AIType::MANUAL});
    world.spawn_dude({{10.0, 10.0}, AIType::DUMMY});
    world.spawn_obstacle({{.x = -5.0, .y = 5.0, .width = 10.0, .height = 2.0}});
    world.spawn_obstacle({{.x = -15.0, .y = 0.0, .width = 3.0, .height = 10.0}});

    float accum_frame_time = 0.0;
    while (!WindowShouldClose()) {
        accum_frame_time += GetFrameTime();
        while (accum_frame_time >= world.timestep) {
            world.update();
            accum_frame_time -= world.timestep;
        }
        renderer.draw(world);
    }
}

int main() {
    start_game();
}
