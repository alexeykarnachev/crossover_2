#include "core.hpp"

#include "GLFW/glfw3.h"
#include "box2d/b2_body.h"
#include "box2d/b2_circle_shape.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_friction_joint.h"
#include "box2d/b2_math.h"
#include "box2d/b2_polygon_shape.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "raylib.h"
#include "raymath.h"
#include <cstdio>
#include <iostream>

// -----------------------------------------------------------------------
// utils
static void draw_body_debug(b2Body *body, Color color) {
    if (!body) {
        fprintf(stderr, "WARNING: Can't draw NULL body\n");
        return;
    } else if (!body->IsEnabled()) {
        fprintf(stderr, "WARNING: Can't draw disabled body\n");
        return;
    }

    b2Vec2 position = body->GetPosition();

    b2Fixture *fixture = body->GetFixtureList();
    b2Shape *shape = fixture->GetShape();
    if (shape->GetType() == b2Shape::e_circle) {
        b2CircleShape *circle = static_cast<b2CircleShape *>(shape);
        float radius = circle->m_radius;
        DrawCircleV({position.x, position.y}, radius, color);
    }
}

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
// bullet
Bullet::Bullet(){};
Bullet::Bullet(b2Body *body)
    : body(body){};

void Bullet::update(Game &game) {
    this->ttl -= game.timestep;
    if (this->ttl <= 0.0) this->body->SetEnabled(false);
}

void Bullet::draw_debug() {
    draw_body_debug(this->body, YELLOW);
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

    if (this->type == DudeType::PLAYER) {
        // move
        b2Vec2 linear_velocity = {0.0, 0.0};

        if (IsKeyDown(KEY_W)) linear_velocity.y -= 1.0;
        if (IsKeyDown(KEY_S)) linear_velocity.y += 1.0;
        if (IsKeyDown(KEY_A)) linear_velocity.x -= 1.0;
        if (IsKeyDown(KEY_D)) linear_velocity.x += 1.0;

        if (linear_velocity.Normalize() > EPSILON) {
            linear_velocity *= this->move_speed;
        } else {
            linear_velocity.SetZero();
        }
        this->body->SetLinearVelocity(linear_velocity);

        // rotate
        Vector2 screen_mouse_position = GetMousePosition();
        Vector2 world_mouse_position = GetScreenToWorld2D(
            screen_mouse_position, game.camera.camera2d
        );
        Vector2 dir = {
            world_mouse_position.x - position.x, world_mouse_position.y - position.y};
        float target_angle = Vector2Angle({1.0, 0.0}, dir);
        this->body->SetTransform(position, target_angle);

        // shoot
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            game.spawn_bullet(position, {dir.x, dir.y});
        }
    } else if (this->type == DudeType::SIMPLE_AI) {
    }
}

void Dude::draw_debug() {
    draw_body_debug(this->body, RED);

    b2Vec2 position = this->body->GetPosition();
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
    : b2_world({0.0f, 0.0f})
    , camera(GameCamera(screen_width, screen_height)) {

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

    // -------------------------------------------------------------------
    // create ground body (for friction joints)
    b2BodyDef ground_body_def;
    ground_body_def.position.Set(0.0f, 0.0f);
    this->ground_body = this->b2_world.CreateBody(&ground_body_def);
    this->ground_body->SetType(b2_staticBody);

    b2PolygonShape ground_shape;
    ground_shape.SetAsBox(0.0, 0.0);

    // Create fixture for the ground
    b2FixtureDef ground_fixture_def;
    ground_fixture_def.shape = &ground_shape;
    ground_fixture_def.isSensor = true;
    this->ground_body->CreateFixture(&ground_fixture_def);
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

    // destroy bullets
    int free_idx = -1;
    for (int i = 0; i < this->bullets.size(); ++i) {
        Bullet &bullet = this->bullets[i];
        if (!bullet.body->IsEnabled() && free_idx == -1) {
            free_idx = i;
        } else if (bullet.body->IsEnabled() && free_idx != -1) {
            if (!this->bullets[free_idx].body->IsEnabled()) {
                this->b2_world.DestroyBody(this->bullets[free_idx].body);
            }
            this->bullets[free_idx++] = bullet;
        }
    }
    if (free_idx != -1) {
        this->bullets.resize(free_idx);
    }
}

void Game::step() {
    this->b2_world.Step(this->timestep, 8, 3);

    // update dudes
    for (Dude &dude : this->dudes) {
        dude.update(*this);
    }

    // update bullets
    for (Bullet &bullet : this->bullets) {
        bullet.update(*this);
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

    // draw bullets
    for (Bullet &bullet : this->bullets) {
        bullet.draw_debug();
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
    this->spawn_dude(DudeType::PLAYER, {0.0, 0.0});
    this->spawn_dude(DudeType::SIMPLE_AI, {0.0, 6.0});

    while (!WindowShouldClose()) {
        this->update();
        this->draw();
    }
}

void Game::spawn_dude(DudeType type, b2Vec2 position) {
    // body
    b2BodyDef body_def;
    body_def.type = b2_dynamicBody;
    body_def.position.Set(position.x, position.y);
    body_def.fixedRotation = true;
    b2Body *body = this->b2_world.CreateBody(&body_def);

    // shape
    b2CircleShape shape;
    shape.m_p.Set(0.0, 0.0);
    shape.m_radius = 1.0;

    b2FixtureDef fixture_def;
    fixture_def.shape = &shape;
    fixture_def.density = 1.0;
    body->CreateFixture(&fixture_def);

    // friction joint
    b2FrictionJointDef joint_def;
    joint_def.bodyA = this->ground_body;
    joint_def.bodyB = body;
    joint_def.localAnchorA.SetZero();
    joint_def.localAnchorB.SetZero();
    joint_def.collideConnected = false;
    joint_def.maxForce = 100.0;
    this->b2_world.CreateJoint(&joint_def);

    // dude
    Dude dude(type, body, 5.0, 4.0, 16, DEG2RAD * 70.0, 5.0);
    this->dudes.push_back(dude);
}

void Game::spawn_bullet(b2Vec2 position, b2Vec2 direction) {
    // body
    b2BodyDef body_def;
    body_def.type = b2_kinematicBody;
    body_def.position.Set(position.x, position.y);
    b2Body *body = this->b2_world.CreateBody(&body_def);
    body->SetBullet(true);

    // set bullet velocity
    direction.Normalize();
    body->SetLinearVelocity({direction.x * BULLET_SPEED, direction.y * BULLET_SPEED});

    // shape
    b2CircleShape shape;
    shape.m_p.Set(0.0, 0.0);
    shape.m_radius = 0.1;

    b2FixtureDef fixture_def;
    fixture_def.shape = &shape;
    fixture_def.isSensor = true;
    body->CreateFixture(&fixture_def);

    Bullet bullet(body);
    this->bullets.push_back(bullet);
}
