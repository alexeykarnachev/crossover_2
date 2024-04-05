#include "core.hpp"

#include "GLFW/glfw3.h"
#include "box2d/b2_circle_shape.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "raylib.h"
#include "raymath.h"
#include <iostream>

// -----------------------------------------------------------------------
// game camera
GameCamera::GameCamera(){};

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
Dude::Dude(b2Body *body)
    : body(body){};

void Dude::update(Game &game) {}

// -----------------------------------------------------------------------
// game
Game::Game(Vector2 screen_size)
    : b2_world({0.0f, 0.0f}) {
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

    this->camera = GameCamera(screen_size);
}

Game::~Game() {
    // close imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    CloseWindow();
}

void Game::update() {
    this->dt += GetFrameTime();

    int i = 0;
    while (this->dt >= this->timestep) {
        this->dt -= this->timestep;
        this->time += this->timestep;

        this->b2_world.Step(this->timestep, 8, 3);
        i += 1;
    }
    std::cout << i << "\n";
}

void Game::draw() {
    BeginDrawing();
    ClearBackground(BLACK);

    this->draw_world();
    this->draw_imgui();

    DrawFPS(0, 0);

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
    this->spawn_dude(0.0, 0.0);

    while (!WindowShouldClose()) {
        this->update();
        this->draw();
    }
}

void Game::spawn_dude(float x, float y) {
    b2BodyDef body_def;
    body_def.type = b2_staticBody;
    body_def.position.Set(x, y);
    b2Body *body = this->b2_world.CreateBody(&body_def);

    b2CircleShape shape;
    shape.m_p.Set(0.0, 0.0);
    shape.m_radius = 1.0;

    b2FixtureDef myFixtureDef;
    myFixtureDef.shape = &shape;
    body->CreateFixture(&myFixtureDef);

    Dude dude(body);
    this->dudes.push_back(dude);
}
