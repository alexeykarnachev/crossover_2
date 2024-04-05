#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "raylib.h"
#include <Eigen/Dense>
#include <iostream>
#include <GLFW/glfw3.h>


#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

int main() {
    std::cout << "Running...\n";

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "crossover_2");
    SetTargetFPS(60);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    GLFWwindow *window = (GLFWwindow *)GetWindowHandle();
    glfwGetWindowUserPointer(window);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
    bool show_another_window = true;

    Camera2D camera = {.offset = {SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0}, .target = {0.0, 0.0}, .rotation = 0.0, .zoom = 1.0};

    while (!WindowShouldClose()) {
        BeginDrawing();


        BeginMode2D(camera);
        ClearBackground(RAYWHITE);
        DrawCircle(0.0, 0.0, 100.0, RED);
        EndMode2D();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        EndDrawing();
    }
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
