#include "./Game.hpp"

int main(int argc, char* argv[]) {
    bool enableImGui = false;
    if (argc > 1 && std::string(argv[1]) == "--enable-imgui") {
        enableImGui = true;
    }

    Game game(enableImGui);
    game.run();

    return 0;
}