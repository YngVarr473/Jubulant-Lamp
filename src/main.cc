#include "./core.hh"

int main(int argc, char* argv[]) {
    std::srand(std::time(0));
    Game game;
    game.run();
    return 0;
}
