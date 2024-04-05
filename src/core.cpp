#include "core.hpp"

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "raymath.h"

// -----------------------------------------------------------------------
// game camera
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
// dude
Dude::Dude(Vector2 position, float radius)
    : position(position)
    , radius(radius){};

void Dude::update(Game &game) {}

// -----------------------------------------------------------------------
// game
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

Game::~Game() {
    // close imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    CloseWindow();
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
