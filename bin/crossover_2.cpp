#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "raylib.h"
#include "raymath.h"
#include <Eigen/Dense>
#include <GLFW/glfw3.h>
#include <iostream>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

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

GameCamera::GameCamera(Vector2 screen_size) {
    Vector2 offset = Vector2Scale(screen_size, 0.5);
    this->camera2d = {
        .offset = offset, .target = {0.0, 0.0}, .rotation = 0.0, .zoom = this->zoom};
}

void GameCamera::begin_mode_2d() {
    BeginMode2D(this->camera2d);
}

void GameCamera::end_mode_2d() {
    EndMode2D();
}

// -----------------------------------------------------------------------
// game
class Game {
  private:
    GameCamera camera;

    float time;
    float dt;

    void update();
    void draw();

    void draw_world();
    void draw_imgui();

  public:
    Game(Vector2 screen_size);
    void run();
};

Game::Game(Vector2 screen_size)
    : camera(screen_size) {
    // init raylib window
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetTargetFPS(60);
    InitWindow(screen_size.x, screen_size.y, "Game");

    // init imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    GLFWwindow *window = (GLFWwindow *)GetWindowHandle();
    glfwGetWindowUserPointer(window);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 420 core");
    ImGui::StyleColorsDark();
}

void Game::update() {
    this->dt = GetFrameTime();
    this->time += this->dt;
}

void Game::draw() {
    BeginDrawing();
    ClearBackground(BLACK);

    this->draw_world();
    this->draw_imgui();

    EndDrawing();
}

void Game::draw_world() {
    this->camera.begin_mode_2d();
    DrawCircle(0.0, 0.0, 100.0, RED);
    this->camera.end_mode_2d();
}

void Game::draw_imgui() {
    // start imgui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("Another Window", NULL);
        ImGui::Text("Hello from another window!");
        ImGui::End();
    }

    // draw imgui frame
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Game::run() {
    while (!WindowShouldClose()) {
        this->update();
        this->draw();
    }
}

// -----------------------------------------------------------------------
// main
int main() {
    Game game({SCREEN_WIDTH, SCREEN_HEIGHT});
    game.run();
}

// // eigen test
// // Define matrices
// Eigen::MatrixXd A = Eigen::MatrixXd::Random(1000, 1000); // Random 1000x1000 matrix
// Eigen::MatrixXd B = Eigen::MatrixXd::Random(1000, 1000); // Random 1000x1000 matrix
// Eigen::MatrixXd C;

// for (int i = 0; i < 10; ++i) {
//     C = A * B;
// }

// std::cout << "Result of matrix multiplication:\n" << C << std::endl;
