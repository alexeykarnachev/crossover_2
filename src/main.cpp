#include "core.hpp"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

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
