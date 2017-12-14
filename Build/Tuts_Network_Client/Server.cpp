#include "Server.h"
#include "State.h"
#include <ncltech\CuboidCollisionShape.h>
#include <ncltech\PhysicsEngine.h>

#define ID evnt.peer->incomingPeerID
#define CLIENT_N server->m_pNetwork->connectedPeers
#define STRING_PACKET enet_packet_create(s.c_str(), sizeof(char) * s.length(), 0)
#define PATH_LIST std::list<const GraphNode*>

Server::Server() {

	srand(time(NULL));
	server = new NetworkBase();
	printf("Server Initiated\n");

	Win32_PrintAllAdapterIPAddresses();

	timer.GetTimedMS();
	generator = new MazeGenerator();
	search_as = new SearchAStar();
	generator->Generate(10, 0.7f);

	for (int i = 0; i < HAZARD_NUM; ++i){
		hazards.push_back(new Hazard(generator));
		hazards[i]->SetStartNode(OUT_OF_RANGE);
		hazards[i]->avatar_idcs = avatars;
	}

	PhysicsEngine::Instance()->SetGravity(Vector3(0, 0, 0));
	PhysicsEngine::Instance()->SetDampingFactor(1.0f);
	PhysicsEngine::Instance()->ResetOcTree();
	PhysicsEngine::Instance()->SetLimits(Vector3(0, 0, 0), Vector3(100, 100, 10));
	
}


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
				
				printf("- New Client Connected\n");

				InitializeArrayElements(ID);// Initalise array elements for start & end nodes, avatar positions and paths
				SendWalls(ID);				// Send maze info to new client
				SendAvatarPositions(ID);	// Send the avatars position to the client
				SendHazardPosition(ID);

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
					break;
				}
					
				case END_POS:
				{
					PosStruct p = Recieve_pos(evnt);
					generator->SetEndNode(ID, p.idx);
					UpdateAStarPreset(ID);
					SendPath(ID);
					if (paths[ID].size() > 0 && avatar_obj[ID])
						StartFollowing(ID);

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

					generator->Generate((int)params[0], params[1]);

					for (int i = 0; i < server->m_pNetwork->connectedPeers; ++i) {
						InitializeArrayElements(i);
						SendPath(i);
					}
					ResetHazards();
					SendAvatarPositions();
					SendHazardPosition();
					SendWalls();
			
					break;
				}

				case AVATAR_POS_UPDATE:
				{
					string packet;
					for (int i = 2; i < evnt.packet->dataLength; ++i) {
						packet.push_back(evnt.packet->data[i]);
					}

					int flat_maze_size = generator->size * 3 - 1;
					const float scalar = 1.f / (float)flat_maze_size;

					Vector3 cellsize = Vector3(
						scalar * 2,
						1.2f,
						scalar * 2
					);

					avatars[ID] = stoi(packet);
					if (!avatar_obj[ID]) {
						PhysicsNode * pnode = new PhysicsNode();
						CollisionShape * pColshape = new CuboidCollisionShape(Vector3(0.5f,0.5f,0.5f));
						pnode->SetCollisionShape(pColshape);
						pnode->SetInverseMass(1.0f/10.0f);
						pnode->SetBoundingRadius(2.0f);
						pnode->SetOnCollisionCallback(
							std::bind(
								&Server::ColissionCallback,
								this,
								std::placeholders::_1,
								std::placeholders::_2,
								ID
								));
						avatar_obj[ID] = pnode;

						PhysicsEngine::Instance()->AddPhysicsObject(pnode);
						
					}

					avatar_obj[ID]->SetPosition(generator->GetNode(avatars[ID])->_pos);
					avatar_obj[ID]->SetLinearVelocity(Vector3(0, 0, 0));

					// TODO: Extend for actual physics;
					SendAvatarPositions();
					UpdateHazards();

					UpdateAStarPreset(ID);
					SendPath(ID);
				}
				break;
				}

				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				printf("- Client %d has disconnected.\n", ID);
				InitializeArrayElements(ID);
				//generator->startnodes.erase(generator->startnodes.begin() + evnt.peer->incomingPeerID);
				//generator->endnodes.erase(generator->startnodes.end() + evnt.peer->incomingPeerID);
				//SendNumberClients();
				break;
			}
		});

		//Update Physics
		PhysicsEngine::Instance()->Update(dt);
		for (int i = 0; i < CLIENT_N; ++i) {
			if (avatars[i] != OUT_OF_RANGE) {
				FollowPath(i);
			}
		}

		if (accum_time >= UPDATE_TIMESTEP)
		{
			accum_time = 0.0f;
			SendAvatarPositions();

			for (int i = 0; i < HAZARD_NUM; ++i) {
				hazards[i]->Update();
				SendHazardPosition();
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

	string str = to_string(PATH) + ":";

	if (paths[i].size() > 0) {
		
		std::list<const GraphNode*>::iterator it = paths[i].begin();
		for (int j = 1; j < paths[i].size(); ++j) {
			str = str + to_string((*it)->_idx) + " ";
			++it;
		}
	}

	ENetPacket* path_packet = enet_packet_create(str.c_str(), sizeof(char)*str.size(), 0);
	//enet_host_broadcast(server->m_pNetwork, 0, path_packet);
	enet_peer_send(&server->m_pNetwork->peers[i], 0, path_packet);

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

	GraphNode* start = generator->startnodes[i];
	GraphNode* end = generator->endnodes[i];

	if (start && end) {
		search_as->FindBestPath(start, end);
	}
	paths[i] = search_as->GetFinalPath();

}

void Server::SendAvatarPositions(int i) {
	string s = to_string(AVATAR_POS_UPDATE) + ":";

	for (int j = 0; j < CLIENT_N; ++j) {
		if (avatar_obj[j]) {
			Vector3 pos = avatar_obj[j]->GetPosition();
			s += to_string(avatars[j]) + " " + to_string(pos.x) + " " + to_string(pos.y) + " " + to_string(pos.z) + " ";
		}
		else {
			s += to_string(avatars[j]) + " -1 -1 -1 ";
		}
	}

	ENetPacket* avatar_positions = STRING_PACKET;

	if (i == OUT_OF_RANGE)
		enet_host_broadcast(server->m_pNetwork, 0, avatar_positions);
	else
		enet_peer_send(&server->m_pNetwork->peers[i], 0, avatar_positions);
}

void Server::SendHazardPosition(int i) {
	string s = to_string(HAZARD_POS_UPDATE) + ":";

	for (int j = 0; j < HAZARD_NUM; ++j) {
		Vector3 pos = hazards[j]->current_pos;
		s += to_string(hazards[j]->current_idx) + " " + to_string(pos.x) + " " + to_string(pos.y) + " " + to_string(pos.z) + " ";
	}

	ENetPacket* avatar_positions = STRING_PACKET;

	if (i == OUT_OF_RANGE)
		enet_host_broadcast(server->m_pNetwork, 0, avatar_positions);
	else
		enet_peer_send(&server->m_pNetwork->peers[i], 0, avatar_positions);
}

void Server::FollowPath(int i) {

	if (avatar_obj[i]) {
		for (std::list<const GraphNode*>::iterator it = paths[i].begin(); it != paths[i].end();) {
			if ((*it)->_idx == avatars[i]) {
				Vector3 posA = ((*it)->_pos);
				int idxA = (*it)->_idx;
				++it;
				if (it != paths[i].end()) {
					Vector3 posB = ((*it)->_pos);
					int idxB = (*it)->_idx;

					if (idxA + 1 == idxB && (posA - avatar_obj[i]->GetPosition()).LengthSQ() <= 0.002f) {
						avatar_obj[i]->SetLinearVelocity(Vector3(1, 0, 0));
					}
					else if (idxA - 1 == idxB && (posA - avatar_obj[i]->GetPosition()).LengthSQ() <= 0.002f) {
						avatar_obj[i]->SetLinearVelocity(Vector3(-1, 0, 0));
					}
					else if (idxA + 10 == idxB && (posA - avatar_obj[i]->GetPosition()).LengthSQ() <= 0.002f) {
						avatar_obj[i]->SetLinearVelocity(Vector3(0, 1, 0));
					}
					else if (idxA - 10 == idxB && (posA - avatar_obj[i]->GetPosition()).LengthSQ() <= 0.002f) {
						avatar_obj[i]->SetLinearVelocity(Vector3(0, -1, 0));
					}

					Vector3 vel = avatar_obj[i]->GetLinearVelocity();

					if ((posB - avatar_obj[i]->GetPosition()).LengthSQ() < (posB - posA).LengthSQ() / 4.0f) {
						avatars[i] = idxB;
					}
					else if ((posA - avatar_obj[i]->GetPosition()).LengthSQ() > (posB - posA).LengthSQ() && (posA - avatar_obj[i]->GetPosition()).Normalise() == (vel).Normalise()) {
						avatar_obj[i]->SetLinearVelocity(avatar_obj[i]->GetLinearVelocity() * -1);

						Vector3 avatarPos = avatar_obj[i]->GetPosition();
						int avatarIdx = avatars[i];
						GraphNode * current = generator->GetNode(avatarIdx);
						GraphNode * other = generator->GetNode(avatarIdx - 1);

						//if ((avatarPos - other->_pos).LengthSQ() > (current->_pos - other->_pos).LengthSQ() / 4.0f) {
						//	avatars[i] = other->_idx;
						//}

						//other = generator->GetNode(avatarIdx + 1);
						//if ((avatarPos - other->_pos).LengthSQ() > (current->_pos - other->_pos).LengthSQ() / 4.0f) {
						//	avatars[i] = other->_idx; 
						//}

						//other = generator->GetNode(avatarIdx + generator->size);
						//if ((avatarPos - other->_pos).LengthSQ() > (current->_pos - other->_pos).LengthSQ() / 4.0f) { 
						//	avatars[i] = other->_idx;
						//}

						//other = generator->GetNode(avatarIdx - generator->size);
						//if ((avatarPos - other->_pos).LengthSQ() > (current->_pos - other->_pos).LengthSQ() / 4.0f) { 
						//	avatars[i] = other->_idx;
						//}


					}
				}
				else if ((posA - avatar_obj[i]->GetPosition()).LengthSQ() < 0.002f) {
					avatar_obj[i]->SetLinearVelocity(Vector3(0, 0, 0));
					avatar_obj[i]->SetPosition(posA);
				}
			}
			else {
				++it;
			}

		}
		UpdateHazards();
	}

}

void Server::StartFollowing(int i) {
	auto it = paths[i].begin();
	Vector3 pos = (*it)->_pos;
	int idx = (*it)->_idx;

	if (pos.x < avatar_obj[i]->GetPosition().x) {
		avatar_obj[i]->SetLinearVelocity(Vector3(1, 0, 0));
	}
	else if (pos.x > avatar_obj[i]->GetPosition().x) {
		avatar_obj[i]->SetLinearVelocity(Vector3(-1, 0, 0));
	}
	else if (pos.y < avatar_obj[i]->GetPosition().y) {
		avatar_obj[i]->SetLinearVelocity(Vector3(0, 1, 0));
	}
	else if (pos.y > avatar_obj[i]->GetPosition().y) {
		avatar_obj[i]->SetLinearVelocity(Vector3(0, -1, 0));
	}
}

void Server::InitializeArrayElements(int id) {
	// Initialize the stored Start and End nodes or this client
	generator->SetStartNode	(id, OUT_OF_RANGE);
	generator->SetEndNode	(id, OUT_OF_RANGE);

	// Initialize avatar element for given client to OUT_OF_RANGE
	if		(avatars.size() == id)	{ avatars.push_back(OUT_OF_RANGE); }	
	else if (avatars.size() > id)	{ avatars[id] = OUT_OF_RANGE; }

	if (avatar_obj.size() == id)	 { avatar_obj.push_back(nullptr); }
	else if (avatar_obj.size() > id) { avatar_obj[id] = nullptr; }

	// Initialize path for client as an empty path
	PATH_LIST a(0);
	if		(paths.size() == id)	{ paths.push_back(a); }		
	else if (paths.size() > id)		{ paths[id] = a; }	 

	if (lerp_factor.size() == id)		{ lerp_factor.push_back(0.0f); }
	else if (lerp_factor.size() > id)	{ lerp_factor[id] = 0.0f; }
}

void Server::UpdateHazards() {
	for (int i = 0; i < HAZARD_NUM; ++i) {
		hazards[i]->UpdateAvatars(avatars);
	}
}

void Server::ResetHazards() {
	for (int i = 0; i < hazards.size(); ++i) {
		delete hazards[i];
		hazards[i] = new Hazard(generator);
		hazards[i]->SetStartNode(OUT_OF_RANGE);
		hazards[i]->avatar_idcs = avatars;
	}
}

Vector3 Server::InterpolatePositionLinear(Vector3 posA, Vector3 posB, float factor)
{
	//With factor between 0-1, this is defined as:
	// LerpA-B(factor) = (1 - factor) * A + factor * B

	return posA * (1.0f - factor) + posB * factor;
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

bool Server::ColissionCallback(PhysicsNode* self, PhysicsNode* other,int self_idx) {
	Vector3 pos1 = self->GetPosition();
	Vector3 pos2 = other->GetPosition();

	Vector3 vel1 = self->GetLinearVelocity();
	Vector3 vel2 = other->GetLinearVelocity();

	if ((pos2 - pos1).Normalise() == vel1.Normalise() || (pos1 - pos2).Normalise() == vel2.Normalise()) {
		self->SetLinearVelocity(vel1*-1);
		other->SetLinearVelocity(vel2*-1);
	}
		
	return false;
}
