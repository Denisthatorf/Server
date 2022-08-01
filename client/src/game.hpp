#include <iostream>
#include <unordered_map>

#include <game_common.hpp>
#include <network.h>
#include "input.hpp"

class Game final : public net::client_interface<GameMsg>
{
public:

    const int MAP_WIDTH = 40;
    const int MAP_HEIGHT = 20;
    const char PLAYER_SYMBOL = '@';
    const char FOOD_SYMBOL = '$'; // '&'
    const char WALL_SYMBOL = '#';

    Game() {}
    virtual ~Game() 
    {
        input_off();

        printf("\n\n###### THANK YOU FOR GAME ######\n\n");
    }

    virtual void Start() {
        input_on();

        m_timeout.tv_sec = 0;
        m_timeout.tv_usec = 0;

        m_bWaitingForConnection = true;

        while (Update()) { }

        Stop();
    }

protected:
    virtual bool Update() 
    {
        system("clear");

        if (IsConnected())
		{
			while (!Incoming().empty())
			{
				auto msg = Incoming().pop_front().msg;

				switch (msg.header.id)
				{
				case(GameMsg::Client_Accepted):
				{
					std::cout << "Server accepted client - you're in!\n";
					net::message<GameMsg> msg;
					msg.header.id = GameMsg::Client_RegisterWithServer;
					Send(msg);
					break;
				}

				case(GameMsg::Client_AssignedId):
				{
					msg >> player_id;
					std::cout << "Assigned Client ID = " << player_id << "\n";
					break;
				}

				case(GameMsg::Game_AddPlayer):
				{
					Player new_player;
					msg >> new_player;
					m_players.insert_or_assign(new_player.id, new_player);
                    
					if (player_id == new_player.id)
						m_bWaitingForConnection = false;
					break;
				}

				case(GameMsg::Game_RemovePlayer):
				{
					int nRemovalID = 0;
					msg >> nRemovalID;
                    if(nRemovalID)
                        m_players.erase(nRemovalID);
					break;
				}

				case(GameMsg::Game_UpdatePlayer):
				{
					Player player;
					msg >> player;
					m_players.insert_or_assign(player.id, player);
					break;
				}

                case( GameMsg::Game_UpdateItem):
                {
                    msg >> m_food_position;
                    break;
                }

                default:
                    break;
				}
			}
		}

		if (m_bWaitingForConnection)
		{
			std::cout << "Waiting To Connect...\n";
			return true;
		}

        FD_ZERO(&m_rfds);
        FD_SET(STDIN_FILENO, &m_rfds);
        bool isPositionOfPlayerChanged = false;

        if (select(1, &m_rfds, NULL, NULL, &m_timeout) > 0 && FD_ISSET(STDIN_FILENO, &m_rfds)) {
            switch (getchar()) {

                case 100:
                case 68:
                    //RIGHT
                    this_player().position.X += 1;
                    isPositionOfPlayerChanged = true;
                    break;

                case 83:
                case 115: 
                    //DOWN 
                    this_player().position.Y -= 1;
                    isPositionOfPlayerChanged = true;
                    break;

                case 65:
                case 97:  
                    //LEFT
                    this_player().position.X -= 1;
                    isPositionOfPlayerChanged = true;
                    break;

                case 119:
                case 87: 
                    //UP
                    this_player().position.Y += 1;
                    isPositionOfPlayerChanged = true;
                    break;

                case 27: 
                    return false;
                    break;
            }
        }

        draw();
        if(is_player_ate_food())
           m_players[player_id].score += 1;

        if(is_out_of_border())
            return false;

        if(isPositionOfPlayerChanged) {
			net::message<GameMsg> msg;
			msg.header.id = GameMsg::Game_UpdatePlayer;
            msg << this_player();
			Send(msg);
        }

        usleep(100000);

        return true;
    }
    virtual void Stop() {
        input_off();
    }

protected:
    inline bool is_map_border(Vector2D vec) {
        return vec.Y == 0 || vec.X == 0 || vec.X == MAP_WIDTH || vec.Y == MAP_HEIGHT;
    }
    inline bool is_food(Vector2D vec) {
        return m_food_position.X == vec.X && m_food_position.Y == vec.Y;
    }
    inline bool is_player(Vector2D vec) {
        for (const auto& [key, value] : m_players) {
            if(value.position == vec)
                return true;
        }

        return false;
    }
    inline bool is_player_ate_food() {
        return this_player().position == m_food_position;
    }
    inline bool is_out_of_border() {
        return this_player().position.X < 0 || this_player().position.Y < 0 
            || this_player().position.X > MAP_WIDTH || this_player().position.Y > MAP_HEIGHT;
    }

    void draw() {
        // draws the map
        for (int i(0); i <= MAP_HEIGHT; i++) {
            for (int j(0); j <= MAP_WIDTH; j++)
            {
                if (is_map_border( {j, i} )) {
                    std::cout << WALL_SYMBOL << std::flush;

                } else if (is_player( {j, i} )) {
                    std::cout << PLAYER_SYMBOL << std::flush;

                } else if (is_food( {j, i} )) {
                    std::cout << FOOD_SYMBOL << std::flush;

                } else {
                    std::cout << ' ' << std::flush;
                }
            }

            std::cout << "\n";
        }

        print_score();
    }
    void print_score() {
        for (auto& player : m_players)
        {
            std::cout << "\n###### SCORE: " << this_player().score  << "######\n";
        }
    }

    inline Player& this_player() { return m_players[player_id]; }
    //inline Player this_player() const { return this->m_players.at(player_id); }
protected:
    Vector2D m_food_position;
    std::unordered_map<int, Player> m_players;
    int player_id;

    fd_set m_rfds;
    timeval m_timeout;
    bool m_bWaitingForConnection;
};