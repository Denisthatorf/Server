#include <iostream>
#include <unordered_map>
#include <network.h>
#include <game_common.hpp>


class GameServer final : public net::server_interface<GameMsg> 
{
private:
	const int MAP_WIDTH = 40;
    const int MAP_HEIGHT = 20;
	

public:
	GameServer(uint16_t nPort) : net::server_interface<GameMsg>(nPort)
	{
		generate_food_coord();
	}

private:
	bool OnClientConnect(std::shared_ptr<net::connection<GameMsg>> client) override
	{
		// For now we will allow all 
		return true;
	}

	void OnClientValidated(std::shared_ptr<net::connection<GameMsg>> client) override
	{
		// Client passed validation check, so send them a message informing
		// them they can continue to communicate
		net::message<GameMsg> msg;
		msg.header.id = GameMsg::Client_Accepted;
		client->Send(msg);
	}

	void OnClientDisconnect(std::shared_ptr<net::connection<GameMsg>> client) override
	{
		if (client)
		{
			if (players.find(client->GetID()) == players.end())
			{
				// client never added to roster, so just let it disappear
			}
			else
			{
				auto& pd = players[client->GetID()];
				std::cout << "[UNGRACEFUL REMOVAL]:" + std::to_string(pd.id) + "\n";
				players.erase(client->GetID());
				m_vGarbageIDs.push_back(client->GetID());
			}
		}

	}

	void OnMessage(std::shared_ptr<net::connection<GameMsg>> client, net::message<GameMsg>& msg) override
	{
		if (!m_vGarbageIDs.empty())
		{
			for (auto pid : m_vGarbageIDs)
			{
				net::message<GameMsg> m;
				m.header.id = GameMsg::Game_RemovePlayer;
				m << pid;
				std::cout << "Removing " << pid << "\n";
				MessageAllClients(m);
			}
			m_vGarbageIDs.clear();
		}

		switch (msg.header.id)
		{
		case GameMsg::Client_RegisterWithServer:
		{
			Player player;
			player.id = client->GetID();
			player.position = Vector2D(2, 2);
			player.score = 0;
			players.insert_or_assign(player.id, player);

			net::message<GameMsg> reg_msg;
			reg_msg.header.id = GameMsg::Client_AssignedId;
			reg_msg << player.id;
			MessageClient(client, reg_msg);

			net::message<GameMsg> food_pos_msg;
			food_pos_msg.header.id = GameMsg::Game_UpdateItem;
			food_pos_msg << m_food_position;
			MessageClient(client, food_pos_msg);

			net::message<GameMsg> msgAddPlayer;
			msgAddPlayer.header.id = GameMsg::Game_AddPlayer;
			msgAddPlayer << player;
			MessageAllClients(msgAddPlayer);

			for (const auto& [key, value] : players) 
			{
				net::message<GameMsg> msgAddOtherPlayers;
				msgAddOtherPlayers.header.id = GameMsg::Game_AddPlayer;
				msgAddOtherPlayers << value;
				MessageClient(client, msgAddOtherPlayers);
			}


			break;
		}

		case GameMsg::Client_UnregisterWithServer:
		{
			break;
		}

		case GameMsg::Game_UpdatePlayer:
		{
			// Simply bounce update to everyone except incoming client
			MessageAllClients(msg, client);

			Player player;
			msg >> player;

			if (player.position == m_food_position){
				generate_food_coord();
				net::message<GameMsg> new_food_pos_msg;
				new_food_pos_msg.header.id = GameMsg::Game_UpdateItem;
				new_food_pos_msg << m_food_position;
				MessageAllClients(new_food_pos_msg);
			}

			players[player.id] = player;

			break;
		}

		default:
			break;

		}

	}

private:
    void generate_food_coord() {
        srand( time( 0 ) );

        m_food_position.X = 3 + rand() % (MAP_WIDTH - 3);
        m_food_position.Y = 3 + rand() % (MAP_HEIGHT - 3);
    }

	Vector2D m_food_position;
	std::unordered_map<int, Player> players;
	std::vector<int> m_vGarbageIDs;
};