#include "Server.h"

#define ID evnt.peer->incomingPeerID
#define CLIENT_N server->m_pNetwork->connectedPeers
#define STRING_PACKET enet_packet_create(s.c_str(), sizeof(char) * s.length(), 0)
#define PATH_LIST std::list<const GraphNode*>


int Server::ServerLoop() {
	while (true)
	{
		float dt = timer.GetTimedMS() * 0.001f;
		accum_time += dt;
		rotation += 0.5f * PI * dt;

		//Handle All Incoming Packets and Send any enqued packets
		server->ServiceNetwork(dt, [&](const ENetEvent& evnt)
		{
			switch (evnt.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
			{
				generator->SetStartNode(ID, OUT_OF_RANGE);
				generator->SetEndNode(ID, OUT_OF_RANGE);
				printf("- New Client Connected\n");
				NewUser(ID);
				SendNumberClients();
				SendWalls(ID);
				SendStartPositions(ID);
				SendEndPositions(ID);
				if (avatars.size() ==ID) { avatars.push_back(OUT_OF_RANGE); }
				else if (avatars.size() > ID) { avatars[ID] = OUT_OF_RANGE; }
				SendAvatarPositions(ID);

				if (paths.size() == ID) { paths.push_back(PATH_LIST()); }
				else if (paths.size() > ID) { paths[ID] = PATH_LIST(); }


			}
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				switch (evnt.packet->data[0] - '0') {
				case START_POS:
				{
					PosStruct p = Recieve_pos(evnt);
					generator->SetStartNode(ID, p.idx);
					UpdateAStarPreset(ID);
					SendPath(ID);

					SendStartPositions();
					break;
				}
					
				case END_POS:
				{
					PosStruct p = Recieve_pos(evnt);
					generator->SetEndNode(ID, p.idx);

					if (avatars[ID] != OUT_OF_RANGE) {
						generator->SetStartNode(ID, avatars[ID]);
						SendStartPositions(ID);
					}
					UpdateAStarPreset(ID);
					SendPath(ID);

					SendEndPositions();
					break;
				}
					
				case TEXT:
				{
					printf("\t Client %d says: %s\n", evnt.peer->incomingPeerID, evnt.packet->data);
					enet_packet_destroy(evnt.packet);
					break;
				}

				case NEW_MAZE:
				{
					string packet;
					for (int i = 2; i < evnt.packet->dataLength; ++i) {
						packet.push_back(evnt.packet->data[i]);
					}
					vector<float> params = split_string_toFloat(packet, ' ');

			/*		delete generator;
					generator = nullptr;
					generator = new MazeGenerator();*/
					generator->Generate((int)params[0], params[1]);

					for (int i = 0; i < server->m_pNetwork->connectedPeers; ++i) {
						generator->SetStartNode(i, OUT_OF_RANGE);
						generator->SetEndNode(i, OUT_OF_RANGE);
						avatars[i] = OUT_OF_RANGE;

					}

					SendStartPositions();
					SendEndPositions();
					SendWalls();


					for (int i = 0; i < server->m_pNetwork->connectedPeers; ++i) {
						UpdateAStarPreset(i);
						SendPath(i);
					}				
					break;
				}

				case AVATAR_POS_UPDATE:
				{
					string packet;
					for (int i = 2; i < evnt.packet->dataLength; ++i) {
						packet.push_back(evnt.packet->data[i]);
					}
					avatars[ID] = stoi(packet);
					SendAvatarPositions();
				}
				}

				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf("- Client %d has disconnected.\n", ID);
				//generator->startnodes.erase(generator->startnodes.begin() + evnt.peer->incomingPeerID);
				//generator->endnodes.erase(generator->startnodes.end() + evnt.peer->incomingPeerID);
				SendNumberClients();
				break;
			}
		});

		if (accum_time >= UPDATE_TIMESTEP)
		{
			accum_time = 0.0f;
			for (int i = 0; i < CLIENT_N; ++i) {
				if (avatars[i] != OUT_OF_RANGE) {
					FollowPath(i);
				}
			}
		}


		Sleep(0);
	}
	system("pause");
	server->Release();
}

void Server::SendWalls(int i)
{
	string * walls = new string;
	*walls = to_string(MAZE_WALLS) + ":" + generator->AllWalls();

	const char * char_walls = walls->c_str();

	ENetPacket* wall = enet_packet_create(char_walls, sizeof(char) * walls->length(), 0);

	if (i != OUT_OF_RANGE)
		enet_peer_send(&server->m_pNetwork->peers[i], 0, wall);
	else 
		enet_host_broadcast(server->m_pNetwork, 0, wall);
}

void Server::NewUser(int i)
{
	string userInfo = to_string(NEW_USER) + ":" + to_string(i);

	ENetPacket* packet = enet_packet_create(userInfo.c_str(), sizeof(char) * userInfo.length(), 0);
	enet_host_broadcast(server->m_pNetwork, 0, packet);
}

void Server::SendPath(int i) {
	
	std::list<const GraphNode*> path = search_as->GetFinalPath();

	string str = to_string(PATH) + ":";
	std::list<const GraphNode*>::iterator it = path.begin();
	for (int i = 1; i < path.size(); ++i) {
		str = str + to_string((*it)->_idx) + " ";
		++it;
	}

	ENetPacket* path_packet = enet_packet_create(str.c_str(), sizeof(char)*str.size(), 0);
	//enet_host_broadcast(server->m_pNetwork, 0, path_packet);
	enet_peer_send(&server->m_pNetwork->peers[i], 0, path_packet);
}

void Server::SendStartPositions(int i) {

	string s = to_string(START_POS) + ":";

	for (vector<GraphNode *>::iterator it = generator->startnodes.begin(); it != generator->startnodes.end(); ++it) {
		int idx;
		Vector3 pos;
		if (*(it))
		{
			idx = (*it)->_idx;
			pos = (*it)->_pos;
		}
		else
		{
			idx = OUT_OF_RANGE;
			pos = Vector3(OUT_OF_RANGE, OUT_OF_RANGE, OUT_OF_RANGE);
		}

			s += to_string(idx) + " " + to_string(pos.x) + " " + to_string(pos.y) + " " + to_string(pos.z) + " ";
	}

	ENetPacket* position_update = STRING_PACKET;

	if (i == OUT_OF_RANGE)
		enet_host_broadcast(server->m_pNetwork, 0, position_update);
	else
		enet_peer_send(&server->m_pNetwork->peers[i], 0, position_update);

}

void Server::SendEndPositions(int i) {

	string s = to_string(END_POS) + ":";

	for (vector<GraphNode *>::iterator it = generator->endnodes.begin(); it != generator->endnodes.end(); ++it) {
		int idx;
		Vector3 pos;
		if (*(it))
		{
			idx = (*it)->_idx;
			pos = (*it)->_pos;
		}
		else
		{
			idx = OUT_OF_RANGE;
			pos = Vector3(OUT_OF_RANGE, OUT_OF_RANGE, OUT_OF_RANGE);
		}

		s += to_string(idx) + " " + to_string(pos.x) + " " + to_string(pos.y) + " " + to_string(pos.z) + " ";
	}

	ENetPacket* position_update = enet_packet_create(s.c_str(), sizeof(char) * s.length(), 0);
	
	if (i == OUT_OF_RANGE)
		enet_host_broadcast(server->m_pNetwork, 0, position_update);
	else
		enet_peer_send(&server->m_pNetwork->peers[i], 0, position_update);

}

void Server::UpdateAStarPreset(int i)
{
	//Example presets taken from:
	// http://movingai.com/astar-var.html
	float weightingG, weightingH;
	switch (astar_preset_idx)
	{
	default:
	case 0:
		//Only distance from the start node matters - fans out from start node
		weightingG = 1.0f;
		weightingH = 0.0f;
		astar_preset_text = "Dijkstra - None heuristic search";
		break;
	case 1:
		//Only distance to the end node matters
		weightingG = 0.0f;
		weightingH = 1.0f;
		astar_preset_text = "Pure Hueristic search";
		break;
	case 2:
		//Equal weighting
		weightingG = 1.0f;
		weightingH = 1.0f;
		astar_preset_text = "Traditional A-Star";
		break;
	case 3:
		//Greedily goes towards the goal node where possible, but still cares about distance travelled a little bit
		weightingG = 1.0f;
		weightingH = 2.0f;
		astar_preset_text = "Weighted Greedy A-Star";
		break;
	}
	search_as->SetWeightings(weightingG, weightingH);

	GraphNode* start	= generator->startnodes[i];
	GraphNode* end		= generator->endnodes[i];

	if (start && end) {
		search_as->FindBestPath(start, end);
	}
	paths[i] = search_as->GetFinalPath();
	
}

void Server::SendNumberClients() {

	string s = to_string(NUM_CLIENT) + ":" + to_string(server->m_pNetwork->connectedPeers);

	ENetPacket* num_clients = STRING_PACKET;
	enet_host_broadcast(server->m_pNetwork, 0, num_clients);
}

void Server::SendAvatarPositions(int i) {
	string s = to_string(AVATAR_POS_UPDATE) + ":";

	for (int j = 0; j < CLIENT_N; ++j) {
		s += to_string(avatars[j]) + " ";
	}

	ENetPacket* avatar_positions = STRING_PACKET;

	if (i == OUT_OF_RANGE)
		enet_host_broadcast(server->m_pNetwork, 0, avatar_positions);
	else
		enet_peer_send(&server->m_pNetwork->peers[i], 0, avatar_positions);
}

void Server::FollowPath(int i) {

	for (std::list<const GraphNode*>::iterator it = paths[i].begin(); it != paths[i].end();) {
		if ((*it)->_idx == avatars[i]) {
			++it;
			if (it != paths[i].end()) {
				avatars[i] = (*it)->_idx;
				break;
			}
		}
		else {
			++it;
		}
	}
	SendAvatarPositions();
}


void Win32_PrintAllAdapterIPAddresses()
{
	//Initially allocate 5KB of memory to store all adapter info
	ULONG outBufLen = 5000;


	IP_ADAPTER_INFO* pAdapters = NULL;
	DWORD status = ERROR_BUFFER_OVERFLOW;

	//Keep attempting to fit all adapter info inside our buffer, allocating more memory if needed
	// Note: Will exit after 5 failed attempts, or not enough memory. Lets pray it never comes to this!
	for (int i = 0; i < 5 && (status == ERROR_BUFFER_OVERFLOW); i++)
	{
		pAdapters = (IP_ADAPTER_INFO *)malloc(outBufLen);
		if (pAdapters != NULL) {

			//Get Network Adapter Info
			status = GetAdaptersInfo(pAdapters, &outBufLen);

			// Increase memory pool if needed
			if (status == ERROR_BUFFER_OVERFLOW) {
				free(pAdapters);
				pAdapters = NULL;
			}
			else {
				break;
			}
		}
	}


	if (pAdapters != NULL)
	{
		//Iterate through all Network Adapters, and print all IPv4 addresses associated with them to the console
		// - Adapters here are stored as a linked list termenated with a NULL next-pointer
		IP_ADAPTER_INFO* cAdapter = &pAdapters[0];
		while (cAdapter != NULL)
		{
			IP_ADDR_STRING* cIpAddress = &cAdapter->IpAddressList;
			while (cIpAddress != NULL)
			{
				printf("\t - Listening for connections on %s:%u\n", cIpAddress->IpAddress.String, SERVER_PORT);
				cIpAddress = cIpAddress->Next;
			}
			cAdapter = cAdapter->Next;
		}

		free(pAdapters);
	}

}

