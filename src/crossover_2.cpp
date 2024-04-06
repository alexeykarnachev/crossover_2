
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "raylib.h"
#include "raymath.h"
#include <array>
#include <iostream>
#include <stdexcept>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define TARGET_FPS 60

#define WORLD_TIMESTEP (1.0 / 60.0)
#define MAX_N_DUDES 16
#define MAX_N_BULLETS 256
#define DEFAULT_BULLET_TTL 5.0
#define DEFAULT_DUDE_RADIUS 1.0
#define DEFAULT_DUDE_MAX_HEALTH 100.0
#define DEFAULT_DUDE_MOVE_SPEED 3.0

class Dude {
  public:
    float radius = DEFAULT_DUDE_RADIUS;
    float max_health = DEFAULT_DUDE_MAX_HEALTH;
    float move_speed = DEFAULT_DUDE_MOVE_SPEED;

    Vector2 position;
    float health = 0.0;

    Dude() = default;

    Dude(Vector2 position) {
        this->position = position;
        this->health = this->max_health;
    };

    bool check_if_can_be_deleted() {
        return this->health <= 0.0;
    }
};

class Bullet {
  public:
    Vector2 position;
    Dude *owner;
    float ttl = DEFAULT_BULLET_TTL;

    Bullet() = default;

    Bullet(Vector2 position, Dude *owner) {
        this->position = position;
        this->owner = owner;
    };

    bool check_if_can_be_deleted() {
        return ttl <= 0.0;
    }
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

template <typename T> int find_free_index(T arr, uint32_t arr_length) {
    int idx = -1;
    for (int i = 0; i < arr_length; ++i) {
        if (arr[i].check_if_can_be_deleted()) {
            idx = i;
            break;
        }
    }

    return idx;
}

class World {
  public:
    float timestep = WORLD_TIMESTEP;

    std::array<Dude, MAX_N_DUDES> dudes;
    std::array<Bullet, MAX_N_BULLETS> bullets;

    GameCamera camera;

    World(){};
    ~World(){};

    void update() {}

    void spawn_dude(Vector2 position) {
        int idx = find_free_index(this->dudes, MAX_N_DUDES);
        if (idx == -1) {
            throw std::runtime_error("ERROR: Can't spawn more dudes");
        }

        this->dudes[idx] = Dude(position);
    }

    void spawn_bullet(Vector2 position, Dude *owner) {
        int idx = find_free_index(this->bullets, MAX_N_BULLETS);
        if (idx == -1) {
            fprintf(stderr, "WARNING: Can't spawn more bullets");
        }

        this->bullets[idx] = Bullet(position, owner);
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
        ClearBackground(RAYWHITE);

        BeginMode2D(world.camera.camera2d);

        for (Dude &dude : world.dudes) {
            if (dude.check_if_can_be_deleted()) continue;

            DrawCircleV(dude.position, dude.radius, RED);
        }

        EndMode2D();

        DrawFPS(0, 0);
        EndDrawing();
    }
};

void start_game() {
    Renderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT);

    World world;
    world.camera = GameCamera(SCREEN_WIDTH, SCREEN_HEIGHT);

    world.spawn_dude({0.0, 0.0});
    world.spawn_dude({10.0, 10.0});

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
