#include "Server.h"

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
				printf("- New Client Connected\n");
				SendWalls();
				SendStartPosition();
				SendEndPosition();
				UpdateAStarPreset();
				SendPath();
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				switch (evnt.packet->data[0] - '0') {
				case START_POS:
				{
					char * c = new char[evnt.packet->dataLength];

					memcpy(c, evnt.packet->data, evnt.packet->dataLength);

					ENetPacket* pos_packet = enet_packet_create(c, evnt.packet->dataLength, 0);
					enet_host_broadcast(server->m_pNetwork, 0, pos_packet);

					PosStruct p = Recieve_startpos(evnt);
					generator->SetStartNode(p.idx);
					UpdateAStarPreset();
					SendPath();
				}
				break;
				case END_POS:
				{
					char * c = new char[evnt.packet->dataLength];

					memcpy(c, evnt.packet->data, evnt.packet->dataLength);

					ENetPacket* pos_packet = enet_packet_create(c, evnt.packet->dataLength, 0);
					enet_host_broadcast(server->m_pNetwork, 0, pos_packet);

					PosStruct p = Recieve_startpos(evnt);
					generator->SetEndNode(p.idx);
					UpdateAStarPreset();
					SendPath();
				}
				break;
				case TEXT:
					printf("\t Client %d says: %s\n", evnt.peer->incomingPeerID, evnt.packet->data);
					enet_packet_destroy(evnt.packet);
					break;
				case NEW_MAZE:
				{
					string packet;
					for (int i = 2; i < evnt.packet->dataLength; ++i) {
						packet.push_back(evnt.packet->data[i]);
					}

					vector<float> params = split_string_toFloat(packet, ' ');

					generator->Generate((int)params[0], params[1]);
					SendWalls();
					SendStartPosition();
					UpdateAStarPreset();
					SendPath();
					break;
				}

				}
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf("- Client %d has disconnected.\n", evnt.peer->incomingPeerID);
				break;
			}
		});

		//Broadcast update packet to all connected clients at a rate of UPDATE_TIMESTEP updates per second
		//if (accum_time >= UPDATE_TIMESTEP)
		//{
		//	//Packet data
		//	// - At the moment this is just a position update that rotates around the origin of the world
		//	//   though this can be any variable, structure or class you wish. Just remember that everything 
		//	//   you send takes up valuable network bandwidth so no sending every PhysicsObject struct each frame ;)
		//	accum_time = 0.0f;
		//	Vector3 pos = Vector3(
		//		cos(rotation) * 2.0f,
		//		1.5f,
		//		sin(rotation) * 2.0f);

		//	//Create the packet and broadcast it (unreliable transport) to all clients
		//	ENetPacket* position_update = enet_packet_create(&pos, sizeof(Vector3), 0);
		//	enet_host_broadcast(server->m_pNetwork, 0, position_update);
		//}
		Sleep(0);
	}
	system("pause");
	server->Release();
}

void Server::SendWalls()
{
	string * walls = new string;
	*walls = to_string(MAZE_WALLS) + ":" + generator->AllWalls();

	const char * char_walls = walls->c_str();

	ENetPacket* wall = enet_packet_create(char_walls, sizeof(char) * walls->length(), 0);
	enet_host_broadcast(server->m_pNetwork, 0, wall);
}

void Server::SendPath() {
	
	std::list<const GraphNode*> path = search_as->GetFinalPath();

	string str = to_string(PATH) + ":";
	std::list<const GraphNode*>::iterator it = path.begin();
	for (int i = 1; i < path.size(); ++i) {
		str = str + to_string((*it)->_idx) + " ";
		++it;
	}

	const char * idcs = str.c_str();

	ENetPacket* path_packet = enet_packet_create(idcs, sizeof(char)*str.size(), 0);
	enet_host_broadcast(server->m_pNetwork, 0, path_packet);

}

void Server::SendStartPosition() {

	Vector3 pos = generator->GetStartNode()->_pos;
	int idx = generator->GetStartNode()->_idx;

	string s = to_string(START_POS) + ":" + to_string(idx) + " " + to_string(pos.x) + " " + to_string(pos.y) + " " + to_string(pos.z);
	const char * char_pos = s.c_str();

	ENetPacket* position_update = enet_packet_create(char_pos, sizeof(char) * s.length(), 0);
	enet_host_broadcast(server->m_pNetwork, 0, position_update);

}


void Server::SendEndPosition() {

	Vector3 pos = generator->GetGoalNode()->_pos;
	int idx = generator->GetGoalNode()->_idx;

	string s = to_string(END_POS) + ":" + to_string(idx) + " " + to_string(pos.x) + " " + to_string(pos.y) + " " + to_string(pos.z);
	const char * char_pos = s.c_str();

	ENetPacket* position_update = enet_packet_create(char_pos, sizeof(char) * s.length(), 0);
	enet_host_broadcast(server->m_pNetwork, 0, position_update);

}




void Server::UpdateAStarPreset()
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

	GraphNode* start = generator->GetStartNode();
	GraphNode* end = generator->GetGoalNode();
	search_as->FindBestPath(start, end);
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

