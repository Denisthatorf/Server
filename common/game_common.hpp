#include <stdint.h> 

struct Vector2D
{
    Vector2D() : X(0), Y(0){}
    Vector2D(int x, int y) : X(x), Y(y) {}

    bool operator==(const Vector2D& vector) const
	{
		return X == vector.X && Y == vector.Y;
	}

    int X;
    int Y;
};

struct Player
{
    int id;
    int score;
    Vector2D position;

	bool operator==(const Player& other) const
	{
		return id == other.id && score == other.score && position == other.position;
	}

};

enum class GameMsg : uint32_t
{
	Server_GetStatus,
	Server_GetPing,

	Client_Accepted,
	Client_AssignedId,
	Client_RegisterWithServer,
	Client_UnregisterWithServer,

	Game_AddPlayer,
	Game_RemovePlayer,
	Game_UpdatePlayer,
	Game_UpdateItem,
};