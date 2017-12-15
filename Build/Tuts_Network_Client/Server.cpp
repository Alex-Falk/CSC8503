#include "Server.h"
#include "State.h"
#include <ncltech\CuboidCollisionShape.h>
#include <ncltech\PhysicsEngine.h>

//Current ID of the peer that has sent the server a packet
#define ID evnt.peer->incomingPeerID

// The number of clients connected to the server
#define CLIENT_N server->m_pNetwork->connectedPeers

// Creates a char array packet from a given string to be sent .
#define STRING_PACKET enet_packet_create(s.c_str(), sizeof(char) * s.length(), 0)

// Because Neatness/lazyness
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

	//No Gravity or damping. Set Limits of map to allow positions to occur.
	// Physics space here is different to render space and is done in X and Y coordinates mainly.
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
				SendAvatarPositions(ID);	// Send the avatars' positions to the client
				SendHazardPosition(ID);		// Send the hazards' positions to the client

			}
				break;
			case ENET_EVENT_TYPE_RECEIVE:

				switch (evnt.packet->data[0] - '0') {

				// Client has updated start position
				case START_POS:
				{
					PosStruct p = Recieve_pos(evnt);
					generator->SetStartNode(ID, p.idx);
					UpdateAStarPreset(ID);
					SendPath(ID);
					break;
				}
				
				// client has updated ned position;
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
				
				// client has sent over text
				case TEXT:
				{
					printf("\t Client %d says: %s\n", evnt.peer->incomingPeerID, evnt.packet->data);
					enet_packet_destroy(evnt.packet);
					break;
				}

				// client requested a new maze. All avatars and hazards values are reset/re-initialized to be out of range/outside of the maze
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

				// client spawned avatar at start node
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

					// If the avatar's physics node doenst exist create a new one and add it to the physics engine
					avatars[ID] = stoi(packet);
					if (!avatar_obj[ID]) {
						PhysicsNode * pnode = new PhysicsNode();
						pnode->SetName("Avatar");
						CollisionShape * pColshape = new CuboidCollisionShape(Vector3(0.33f,0.33f,0.33f));
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
					// Update the avatar's physicsnode's position and velocity
					avatar_obj[ID]->SetPosition(generator->GetNode(avatars[ID])->_pos);
					avatar_obj[ID]->SetLinearVelocity(Vector3(0, 0, 0));

					// inform all clients and Hazards of this avatar
					SendAvatarPositions();
					UpdateHazards();

					// Update the avatar's path and iform the client of it (for drawing)
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

// Send maze information that tells all clients the size of the maze and at what indeces there are walls
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

// Tells all clients that a new user connted to initialise their arrays
void Server::NewUser(int i)
{
	string userInfo = to_string(NEW_USER) + ":" + to_string(i);

	ENetPacket* packet = enet_packet_create(userInfo.c_str(), sizeof(char) * userInfo.length(), 0);
	enet_host_broadcast(server->m_pNetwork, 0, packet);
}

// Iforms client I of its avatar's path
void Server::SendPath(int i) {

	string str = to_string(PATH) + ":";

	if (paths[i].size() > 0) {
		auto new_path = paths[i];
		std::list<const GraphNode*>::iterator it = new_path.begin();
		if (it != new_path.end()) {
			for (int j = 1; j < new_path.size(); ++j) {
				str = str + to_string((*it)->_idx) + " ";
				++it;
			}
		}

	}

	ENetPacket* path_packet = enet_packet_create(str.c_str(), sizeof(char)*str.size(), 0);
	//enet_host_broadcast(server->m_pNetwork, 0, path_packet);
	enet_peer_send(&server->m_pNetwork->peers[i], 0, path_packet);

}

// Update paths using Astar
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
		paths[i] = search_as->GetFinalPath();

		// THIS BIT APPLIES STRING PULLING. HOWEVER IN ITS CURRENT STATE THE AI AND 
		// MOVEMENT HANDLING WILL BREAK DUE TO THIS. HOWEVER THE ACTUAL STRING PULLING ITSELF
		// STILL WORKS.
		/*if (paths[i].size() > 2) {
			paths[i] = StringPulling(i);
		}*/
	}
	else {
		paths[i] = PATH_LIST(0);
	}


}

// Send client i the position of all avatars. If i is input as OUT_OF_RANGE then it wil broadcast it instead
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

// Same as for avatar positions
void Server::SendHazardPosition(int i) {
	string s = to_string(HAZARD_POS_UPDATE) + ":";

	for (int j = 0; j < HAZARD_NUM; ++j) {
		Vector3 pos = hazards[j]->pnode->GetPosition();
		s += to_string(hazards[j]->current_idx) + " " + to_string(pos.x) + " " + to_string(pos.y) + " " + to_string(pos.z) + " ";
	}

	ENetPacket* avatar_positions = STRING_PACKET;

	if (i == OUT_OF_RANGE)
		enet_host_broadcast(server->m_pNetwork, 0, avatar_positions);
	else
		enet_peer_send(&server->m_pNetwork->peers[i], 0, avatar_positions);
}

// Make Client i's avatar follow its current path.
void Server::FollowPath(int i) {

	bool on_path = false;
	if (avatar_obj[i]) {
		for (std::list<const GraphNode*>::iterator it = paths[i].begin(); it != paths[i].end();) {
			if ((*it)->_idx == avatars[i]) {
				on_path = true;
				Vector3 posA = ((*it)->_pos);
				int idxA = (*it)->_idx;
				++it;
				// If there's another node in the path
				if (it != paths[i].end()) {
					Vector3 posB = ((*it)->_pos);
					int idxB = (*it)->_idx;

					//If the avatar gets close to a node of the path, update it's velocity to be in the direction of the next node
					if ((posA - avatar_obj[i]->GetPosition()).LengthSQ() <= 0.002f) {
						avatar_obj[i]->SetLinearVelocity((posB - posA).Normalise());
					}

					// Left this in in case the above doesn't work.
					
					//if (idxA + 1 == idxB && (posA - avatar_obj[i]->GetPosition()).LengthSQ() <= 0.002f) {
					//	avatar_obj[i]->SetLinearVelocity(Vector3(1, 0, 0));
					//}
					//else if (idxA - 1 == idxB && (posA - avatar_obj[i]->GetPosition()).LengthSQ() <= 0.002f) {
					//	avatar_obj[i]->SetLinearVelocity(Vector3(-1, 0, 0));
					//}
					//else if (idxA + 10 == idxB && (posA - avatar_obj[i]->GetPosition()).LengthSQ() <= 0.002f) {
					//	avatar_obj[i]->SetLinearVelocity(Vector3(0, 1, 0));
					//}
					//else if (idxA - 10 == idxB && (posA - avatar_obj[i]->GetPosition()).LengthSQ() <= 0.002f) {
					//	avatar_obj[i]->SetLinearVelocity(Vector3(0, -1, 0));
					//}

					Vector3 vel = avatar_obj[i]->GetLinearVelocity();

					// If the avatar has passed the halfway point bewteen the 'current' and the 'next' node of the path then set the 
					// 'current' node to the 'next' node. These node indeces are used for the Hazards Line of sight check and therefore
					// the string pulling will mess with the AI. In order to fix this the line of sight check could be updated to be position wise rather than index.
					if ((posB - avatar_obj[i]->GetPosition()).LengthSQ() < (posB - posA).LengthSQ() / 4.0f) {
						avatars[i] = idxB;
					}
				}

				// If there's not a next node and the avatar is close to the current node then set velocity to zero and recenter the avatar onto the node
				else if ((posA - avatar_obj[i]->GetPosition()).LengthSQ() < 0.002f) {
					avatar_obj[i]->SetLinearVelocity(Vector3(0, 0, 0));
					avatar_obj[i]->SetPosition(posA);
				}
			}
			else {
				++it;
			}

		}

		// Update the hazards with the new avatar informations
		UpdateHazards();
	}

}

// Start folling a path by going into the direction of the next node.
void Server::StartFollowing(int i) {
	auto it = paths[i].begin();
	Vector3 pos = (*it)->_pos;
	int idx = (*it)->_idx;

	Vector3 newvel = (avatar_obj[i]->GetPosition() - pos).Normalise();

	avatar_obj[i]->SetLinearVelocity(-newvel);
}

// Initialize all arrays holind client information to "OUT_OF_RANGE" or null pointers in order to either:
// - initialise arrays when client connects
// - remove all information when client disconnects
// - "Kill" a client's avatar

void Server::InitializeArrayElements(int id) {
	// Initialize the stored Start and End nodes or this client
	generator->SetStartNode	(id, OUT_OF_RANGE);
	generator->SetEndNode	(id, OUT_OF_RANGE);

	// Initialize avatar element for given client to OUT_OF_RANGE
	if		(avatars.size() == id)	{ avatars.push_back(OUT_OF_RANGE); }	
	else if (avatars.size() > id)	{ avatars[id] = OUT_OF_RANGE; }

	if (avatar_obj.size() == id)	 { 
		avatar_obj.push_back(nullptr); 
	}
	else if (avatar_obj.size() > id) { 
		if (avatar_obj[id]) {
			PhysicsEngine::Instance()->RemovePhysicsObject(avatar_obj[id]);
			delete avatar_obj[id];
			avatar_obj[id] = nullptr;
		}
		else {
			avatar_obj[id] = nullptr;
		}
		
	}

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

// Determine what happens when an Avatar collides with another object
bool Server::ColissionCallback(PhysicsNode* self, PhysicsNode* other,int self_idx) {
	// If it collides with another Avatar they will both currently just move to their respective nodes and stop moving or if they are both 
	// assigned to the same node then it will set its path to lead to the previous hit node and then stop.
	if (other->getName() == "Avatar") {
		Vector3 pos1 = self->GetPosition();
		Vector3 pos2 = other->GetPosition();

		Vector3 vel1 = self->GetLinearVelocity();
		Vector3 vel2 = other->GetLinearVelocity();

		self->SetLinearVelocity(Vector3(0, 0, 0));
		int other_idx;

		for (int i = 0; i < avatar_obj.size(); ++i) {
			if (other == avatar_obj[i]) {
				other_idx = i;
				break;
			}
		}

		if ((pos2 - pos1).Normalise() == vel1.Normalise() || vel1.LengthSQ() == 0) {
			if (avatars[self_idx] != avatars[other_idx]) {
				avatar_obj[self_idx]->SetPosition(avatar_obj[self_idx]->GetPosition() - (pos2 - pos1).Normalise() * 0.1f);
				generator->SetStartNode(avatars[self_idx], self_idx);
				generator->SetEndNode(avatars[self_idx], self_idx);
				UpdateAStarPreset(self_idx);
				SendPath(self_idx);
				//StartFollowing(self_idx);
			}

			else {
				auto start = paths[self_idx].begin();
				++start;
				for (auto it = start; it != paths[self_idx].end(); ++it) {
					int i = (*it)->_idx;
					if (i == avatars[self_idx]) {
						--it;
						i = (*it)->_idx;
						generator->SetStartNode(i, self_idx);
						generator->SetEndNode(i, self_idx);
						UpdateAStarPreset(self_idx);
						SendPath(self_idx);
						//StartFollowing(self_idx);
						return false;
					}
				}
			}
		}
	}

	else if (other->getName() == "Hazard") {
		InitializeArrayElements(self_idx);
		UpdateHazards();
	}

	return false;
}

// Simple stirng pulling that works for the maze using the same line of sight check as the FSM.
std::list<const GraphNode *> Server::StringPulling(int i) {
	std::list<const GraphNode *> new_path;

	auto it = paths[i].begin();
	new_path.push_back((*it));

	for (auto it = paths[i].begin(); it != paths[i].end(); ++it) {
		auto it2 = new_path.end();
		--it2;
		if (!(Check_los((*it2)->_idx, (*it)->_idx))) {
			--it;
			new_path.push_back((*it));
			++it;
		}
	}

	--it;
	new_path.push_back(*it);

	return new_path;

}

bool Server::Check_los(int start, int end) {

	int j_start = start;
	int j_end = end;
	int i_start = 0;
	int i_end = 0;
	int s = generator->size;
	int base_offset = s * (s - 1);

	while (j_start > s - 1) {
		j_start -= s;
		++i_start;
	}

	while (j_end > s - 1) {
		j_end -= s;
		++i_end;
	}

	// Same column
	if (j_start == j_end) {
		if (i_start > i_end) {
			for (int i = i_start; i > i_end; --i) {
				if (generator->allEdges[base_offset + (j_start * (s - 1)) + i]._iswall) { return false; };
			}
		}
		else if (i_start < i_end) {
			for (int i = i_start; i < i_end; ++i) {
				if (generator->allEdges[base_offset + (j_start * (s - 1)) + i]._iswall) { return false; };
			}
		}
		else { return true; }
	}
	// Same Row
	else if (i_start == i_end) {
		if (j_start > j_end) {
			for (int j = j_start; j > j_end; --j) {
				if (generator->allEdges[(i_start * (s - 1) + j)]._iswall) { return false; };
			}
		}
		else if (j_start < j_end) {
			for (int j = j_start; j < j_end; ++j) {
				if (generator->allEdges[(i_start * (s - 1) + j)]._iswall) { return false; };
			}
		}
		else { return true; }
	}
	else { return false; }

}