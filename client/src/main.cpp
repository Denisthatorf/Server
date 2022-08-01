#include "game.hpp"

int main()
{
    Game game;
	game.Connect("127.0.0.1", 60000);
    game.Start();
    return 0;
}