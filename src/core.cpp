#include "core.hpp"

#include "GLFW/glfw3.h"
#include "box2d/b2_circle_shape.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_math.h"
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

GameCamera::GameCamera(int screen_width, int screen_height) {
    this->camera2d = {
        .offset = {0.5f * screen_width, 0.5f * screen_height},
        .target = {0.0, 0.0},
        .rotation = 0.0,
        .zoom = this->zoom};
}

void GameCamera::begin_mode_2d() {
    BeginMode2D(this->camera2d);
}

void GameCamera::end_mode_2d() {
    EndMode2D();
}

// -----------------------------------------------------------------------
// dude
Dude::Dude(
    DudeType type,
    b2Body *body,
    float move_speed,
    float rotate_speed,
    int n_view_rays,
    float view_angle,
    float view_distance
)
    : type(type)
    , body(body)
    , move_speed(move_speed)
    , rotate_speed(rotate_speed)
    , n_view_rays(n_view_rays)
    , view_angle(view_angle)
    , view_distance(view_distance) {}

void Dude::update(Game &game) {
    b2Vec2 position = this->body->GetPosition();
    float angle = this->body->GetAngle();

    switch (this->type) {
        case DudeType::PLAYER:
            // move
            b2Vec2 linear_velocity = {0.0, 0.0};

            if (IsKeyDown(KEY_W)) linear_velocity.y -= 1.0;
            if (IsKeyDown(KEY_S)) linear_velocity.y += 1.0;
            if (IsKeyDown(KEY_A)) linear_velocity.x -= 1.0;
            if (IsKeyDown(KEY_D)) linear_velocity.x += 1.0;

            if (linear_velocity.Normalize() > EPSILON)
                linear_velocity *= this->move_speed;
            else linear_velocity.SetZero();
            this->body->SetLinearVelocity(linear_velocity);

            // rotate
            Vector2 screen_mouse_position = GetMousePosition();
            Vector2 world_mouse_position = GetScreenToWorld2D(
                screen_mouse_position, game.camera.camera2d
            );
            float target_angle = Vector2Angle(
                {1.0, 0.0},
                {world_mouse_position.x - position.x, world_mouse_position.y - position.y}
            );
            this->body->SetTransform(position, target_angle);

            break;
    }
}

void Dude::draw_debug() {
    b2Vec2 position = this->body->GetPosition();

    b2Fixture *fixture = this->body->GetFixtureList();
    b2Shape *shape = fixture->GetShape();
    b2CircleShape *circle = static_cast<b2CircleShape *>(shape);
    float radius = circle->m_radius;

    DrawCircleV({position.x, position.y}, radius, RED);

    for (auto point : this->get_view_ray_end_points()) {
        DrawLineV({position.x, position.y}, {point.x, point.y}, GREEN);
    }
}

std::vector<b2Vec2> Dude::get_view_ray_end_points() {
    std::vector<b2Vec2> rays;

    b2Vec2 origin = this->body->GetPosition();
    float angle = this->body->GetAngle();
    Vector2 first_ray = Vector2Rotate({this->view_distance, 0.0}, angle);

    if (this->n_view_rays == 1) {
        rays.push_back({first_ray.x + origin.x, first_ray.y + origin.y});
    } else {
        float step = this->view_angle / (this->n_view_rays - 1);
        for (int i = 0; i < this->n_view_rays; ++i) {
            float angle = -0.5 * this->view_angle + i * step;
            Vector2 ray = Vector2Rotate(first_ray, angle);
            rays.push_back({ray.x + origin.x, ray.y + origin.y});
        }
    }

    return rays;
}

// -----------------------------------------------------------------------
// game
Game::Game(int screen_width, int screen_height)
    : b2_world({0.0f, 0.0f}) {

    // init raylib window
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetTargetFPS(60);
    InitWindow(screen_width, screen_height, "crossover_2");

    // init imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    GLFWwindow *window = (GLFWwindow *)GetWindowHandle();
    glfwGetWindowUserPointer(window);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 420 core");
    ImGui::StyleColorsDark();

    this->camera = GameCamera(screen_width, screen_height);
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

    while (this->dt >= this->timestep) {
        this->dt -= this->timestep;
        this->time += this->timestep;
        this->step();
    }
}

void Game::step() {
    this->b2_world.Step(this->timestep, 8, 3);

    // update dudes
    for (Dude &dude : this->dudes) {
        dude.update(*this);
    }
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

    // draw dudes
    for (Dude &dude : this->dudes) {
        dude.draw_debug();
    }

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
    this->spawn_dude({0.0, 0.0});

    while (!WindowShouldClose()) {
        this->update();
        this->draw();
    }
}

void Game::spawn_dude(b2Vec2 position) {
    // body
    b2BodyDef body_def;
    body_def.type = b2_dynamicBody;
    body_def.position.Set(position.x, position.y);
    b2Body *body = this->b2_world.CreateBody(&body_def);

    // mass data
    b2MassData mass_data;
    mass_data.center.SetZero();
    mass_data.mass = 80.0;
    mass_data.I = 0.0;
    body->SetMassData(&mass_data);

    // shape
    b2CircleShape shape;
    shape.m_p.Set(0.0, 0.0);
    shape.m_radius = 1.0;

    b2FixtureDef fixture_def;
    fixture_def.shape = &shape;
    body->CreateFixture(&fixture_def);

    // dude
    Dude dude(DudeType::PLAYER, body, 5.0, 4.0, 16, DEG2RAD * 70.0, 5.0);
    this->dudes.push_back(dude);
}
