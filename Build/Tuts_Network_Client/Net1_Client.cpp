#include "Net1_Client.h"
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <nclgl\NCLDebug.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\CommonUtils.h>

const Vector3 status_color3 = Vector3(1.0f, 0.6f, 0.6f);
const Vector4 status_color = Vector4(status_color3.x, status_color3.y, status_color3.z, 1.0f);

Net1_Client::Net1_Client(const std::string& friendly_name)
	: Scene(friendly_name)
	, serverConnection(NULL)
{
}

void Net1_Client::OnInitializeScene()
{
	generator = new MazeGenerator();
	generator->Generate(10, 0);

	//--------------------------------------------------------------------------------------------//
	//Initialize Client Network
	//--------------------------------------------------------------------------------------------//
	if (network.Initialize(0))
	{
		NCLDebug::Log("Network: Initialized!");
		//Attempt to connect to the server on localhost:1234
		serverConnection = network.ConnectPeer(127, 0, 0, 1, 1234);
		NCLDebug::Log("Network: Attempting to connect to server.");
	}
	//--------------------------------------------------------------------------------------------//
	// Meshes & Textures
	//--------------------------------------------------------------------------------------------//

	wallmesh = new OBJMesh(MESHDIR"cube.obj");

	GLuint whitetex;
	glGenTextures(1, &whitetex);
	glBindTexture(GL_TEXTURE_2D, whitetex);
	unsigned int pixel = 0xFFFFFFFF;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &pixel);
	glBindTexture(GL_TEXTURE_2D, 0);

	wallmesh->SetTexture(whitetex);

	//--------------------------------------------------------------------------------------------//
	// Ground
	//--------------------------------------------------------------------------------------------//
	//GameObject* ground = CommonUtils::BuildCuboidObject(
	//	"Ground",
	//	Vector3(0.0f, -1.0f, 0.0f),
	//	Vector3(20.0f, 1.0f, 20.0f),
	//	true,
	//	0.0f,
	//	false,
	//	false,
	//	Vector4(0.2f, 0.5f, 1.0f, 1.0f));

	//this->AddGameObject(ground);

}

void Net1_Client::OnCleanupScene()
{
	Scene::OnCleanupScene();
	
	//Send one final packet telling the server we are disconnecting
	// - We are not waiting to resend this, so if it fails to arrive
	//   the server will have to wait until we time out naturally
	enet_peer_disconnect_now(serverConnection, 0);

	//Release network and all associated data/peer connections
	network.Release();
	serverConnection = NULL;
	SAFE_DELETE(wallmesh);
	if(maze)	maze =  nullptr;

	SAFE_DELETE(generator);
	
}

void Net1_Client::OnUpdateScene(float dt)
{
	Scene::OnUpdateScene(dt);

	//Update Network
	auto callback = std::bind(
		&Net1_Client::ProcessNetworkEvent,	// Function to call
		this,								// Associated class instance
		std::placeholders::_1);				// Where to place the first parameter
	network.ServiceNetwork(dt, callback);

	// Draw the path that was generated from start to end node
	if (maze && drawPath)
		maze->DrawFinalPath(path, 2.5f / (float)size,size);

	// Draw A Grid of squares that shows the mesh
	if (maze && drawMesh)
		maze->DrawNavMesh();


		

	//Add Debug Information to screen
	uint8_t ip1 = serverConnection->address.host & 0xFF;
	uint8_t ip2 = (serverConnection->address.host >> 8) & 0xFF;
	uint8_t ip3 = (serverConnection->address.host >> 16) & 0xFF;
	uint8_t ip4 = (serverConnection->address.host >> 24) & 0xFF;

	//NCLDebug::DrawTextWs(box->Physics()->GetPosition() + Vector3(0.f, 0.6f, 0.f), STATUS_TEXT_SIZE, TEXTALIGN_CENTRE, Vector4(0.f, 0.f, 0.f, 1.f),
	//	"Peer: %u.%u.%u.%u:%u", ip1, ip2, ip3, ip4, serverConnection->address.port);

	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   Grid Size : %2d ([-]/[+] to change)", size);
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "   Density : %2.0f percent ([,]/[.] to change)", density * 100.f);
	
	NCLDebug::AddStatusEntry(status_color, "Network Traffic");
	NCLDebug::AddStatusEntry(status_color, "    Incoming: %5.2fKbps", network.m_IncomingKb);
	NCLDebug::AddStatusEntry(status_color, "    Outgoing: %5.2fKbps", network.m_OutgoingKb);
}

void Net1_Client::ProcessNetworkEvent(const ENetEvent& evnt)
{
	switch (evnt.type)
	{
		//New connection request or an existing peer accepted our connection request
	case ENET_EVENT_TYPE_CONNECT:
	{
		if (evnt.peer == serverConnection)
		{
			NCLDebug::Log(status_color3, "Network: Successfully connected to server!");

			//Send a 'hello' packet
			char* text_data = "4Hellooo!";
			ENetPacket* packet = enet_packet_create(text_data, strlen(text_data) + 1, 0);
			enet_peer_send(serverConnection, 0, packet);
		}
	}
	break;


	//Server has sent us a new packet
	case ENET_EVENT_TYPE_RECEIVE:
	{
		//--------------------------------------------------------------------------------------------//
		// Different commands are defined by the first char in the packet. 
		// They correspond to the enum Packet_Type
		//--------------------------------------------------------------------------------------------//
		int i = evnt.packet->data[0] - '0';
		switch (i) {

		case MAZE_WALLS:
		{
			MazeStruct m = Recieve_maze(evnt);
			ApplyMaze(m);
			break;
		}
		case PATH:
		{
			path.clear();
			vector<int> idcs = Recieve_path(evnt);
			for (int i = 0; i < idcs.size(); ++i) {
				path.push_back(&generator->allNodes[idcs[i]]);
			}
			break;
		}
		case AVATAR_POS_UPDATE:
		{
			string packet;
			for (int i = 2; i < evnt.packet->dataLength; ++i) {
				packet.push_back(evnt.packet->data[i]);
			}

			// TODO: Make a method to do this is
			RenderNode * cube, *root = maze->Render();

			//vector<int> s = split_string_toInt(packet, ' ');

			vector<PosStruct> ps = Recieve_positions(evnt);
			for (int i = 0; i < ps.size(); ++i) {
				if (avatars.size() == i) {
					avatars.push_back(ps[i].idx);
					avatar_positions.push_back(ps[i].pos);
				}
				else if (avatars.size() > i) {
					avatars[i] = ps[i].idx;
					avatar_positions[i] = ps[i].pos;
				}

				if (root->GetChildWithName("Avatar_" + to_string(i))) {
					root->RemoveChild(root->GetChildWithName("Avatar_" + to_string(i)));
				}

				if (avatars[i] != OUT_OF_RANGE) {


					cube = new RenderNode(CommonMeshes::Cube(), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
					cube->SetName("Avatar_" + to_string(i));
					cube->SetTransform(
						Matrix4::Translation(Pos_To_Maze(avatar_positions[i])) *
						Matrix4::Scale(Maze_Scale()));
					root->AddChild(cube);
				}
			}
		}
		break;
		case HAZARD_POS_UPDATE:
		{
			RenderNode * cube, *root = maze->Render();

			vector<PosStruct> ps = Recieve_positions(evnt);
			for (int i = 0; i < ps.size(); ++i) {
				if (root->GetChildWithName("Hazard_" + to_string(i))) {
					root->RemoveChild(root->GetChildWithName("Hazard_" + to_string(i)));
				}

				cube = new RenderNode(CommonMeshes::Cube(), Vector4(0.5f, 0.0f, 0.0f, 1.0f));
				cube->SetName("Hazard_" + to_string(i));
				cube->SetTransform(
					Matrix4::Translation(Pos_To_Maze(ps[i].pos)) *
					Matrix4::Scale(Maze_Scale()));
				root->AddChild(cube);
			}
		}
		break;

		default:
		{
			NCLERROR("Recieved Invalid Network Packet!");
			break;
		}

		}
		break;
		//Server has disconnected
	}
	case ENET_EVENT_TYPE_DISCONNECT:
		{
			NCLDebug::Log(status_color3, "Network: Server has disconnected!");
		}
		break;
	}
}

//--------------------------------------------------------------------------------------------//
// Takes the maze info recieved and applies it to the clients version of the maze
//--------------------------------------------------------------------------------------------//
void Net1_Client::ApplyMaze(MazeStruct m) {

	this->RemoveGameObject(FindGameObject("maze"));
	delete maze;
	maze = nullptr;
	
	generator->Generate(m.size, 0);
	
	// Sets the start and end nodes to -1 which means they will not be put into the maze
	generator->SetStartNode(OUT_OF_RANGE);
	generator->SetEndNode(OUT_OF_RANGE);

	size	= m.size;
	density = m.density;

	if (m.walls.size() > 0) {
		for (int i = 0; i < m.walls.size() - 1; ++i) {
			generator->allEdges[m.walls[i]]._iswall = true;
		}
	}

	maze = new MazeRenderer(generator, wallmesh);
	maze->UpdateRenderer();
	maze->SetMeshRenderNodes(this);
	maze->Render()->SetTransform(
		Matrix4::Translation(Vector3(-m.size/2.0f,0,-m.size/2.0f)) * 
		Matrix4::Scale(Vector3(m.size, 5.0f / float(m.size), m.size)));


	this->AddGameObject(maze);

}

//--------------------------------------------------------------------------------------------//
// Message Sending
//--------------------------------------------------------------------------------------------//
void Net1_Client::SendStartPosition() {

	Vector3 pos = generator->GetStartNode()->_pos;//generator->startnodes[ID]->_pos;
	int idx = generator->GetStartNode()->_idx;//generator->startnodes[ID]->_idx;

	string s = to_string(START_POS) + ":" + to_string(idx) + " " + to_string(pos.x) + " " + to_string(pos.y) + " " + to_string(pos.z);
	const char * char_pos = s.c_str();

	ENetPacket* position_update = enet_packet_create(char_pos,sizeof(char) * s.length(), 0);
	enet_peer_send(serverConnection, 0, position_update);

}

void Net1_Client::SendEndPosition() {

	Vector3 pos = generator->GetGoalNode()->_pos;//generator->endnodes[ID]->_pos;
	int idx = generator->GetGoalNode()->_idx;//generator->endnodes[ID]->_idx;

	string s = to_string(END_POS) + ":" + to_string(idx) + " " + to_string(pos.x) + " " + to_string(pos.y) + " " + to_string(pos.z);
	const char * char_pos = s.c_str();

	ENetPacket* position_update = enet_packet_create(char_pos, sizeof(char) * s.length(), 0);
	enet_peer_send(serverConnection, 0, position_update);

}

void Net1_Client::RequestNewMaze() {
	string s = to_string(NEW_MAZE) + ":" + to_string(size) + " " + to_string(density);

	const char * char_pos = s.c_str();

	ENetPacket* new_maze = enet_packet_create(char_pos, sizeof(char) * s.length(), 0);
	enet_peer_send(serverConnection, 0, new_maze);
}

void Net1_Client::SendAvatarLocation() {
	string s = to_string(AVATAR_POS_UPDATE) + ":" + to_string(avatars[ID]);

	ENetPacket* av_location = enet_packet_create(s.c_str(), sizeof(char) * s.length(), 0);
	enet_peer_send(serverConnection, 0, av_location);
}

//--------------------------------------------------------------------------------------------//
// Keyboard input handling
//--------------------------------------------------------------------------------------------//
void Net1_Client::HandleKeyboardInputs()
{
	GraphNode * start = generator->GetStartNode();
	GraphNode * end = generator->GetGoalNode();
	
	if (Window::GetKeyboard()->KeyHeld(KEYBOARD_CAPITAL))
	{
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_LEFT))
		{
			if (end->_idx % size != 0) {
				generator->SetEndNode(end->_idx - 1);
				maze->UpdateRenderer();
				SendEndPosition();
			}
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_RIGHT))
		{
			if (end->_idx + 1 % generator->size != 0) {
				generator->SetEndNode(end->_idx + 1);
				maze->UpdateRenderer();
				SendEndPosition();
			}
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_DOWN))
		{

			if (end->_idx < (size*size) - size) {
				generator->SetEndNode(end->_idx + size);
				maze->UpdateRenderer();
				SendEndPosition();
			}
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_UP))
		{
			if (end->_idx > size) {
				generator->SetEndNode(end->_idx - size);
				maze->UpdateRenderer();
				SendEndPosition();
			}
		}
	}
	else {
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_LEFT))
		{
			if (start->_idx % size != 0) {
				generator->SetStartNode(start->_idx - 1);
				maze->UpdateRenderer();
				SendStartPosition();
			}
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_RIGHT))
		{
			if (start->_idx + 1 % generator->size != 0) {
				generator->SetStartNode(start->_idx + 1);
				maze->UpdateRenderer();
				SendStartPosition();
			}
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_DOWN))
		{

			if (start->_idx < (size*size) - size) {
				generator->SetStartNode(start->_idx + size);
				maze->UpdateRenderer();
				SendStartPosition();
			}
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_UP))
		{
			if (start->_idx > size) {
				generator->SetStartNode(start->_idx - size);
				maze->UpdateRenderer();
				SendStartPosition();
			}
		}
	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_RETURN))
	{
		if (generator->GetStartNode())
		{
			avatars[ID] = start->_idx;
			SendAvatarLocation();
		}

	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_N))
	{
		RequestNewMaze();
	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_PLUS))
	{
		size = min(size + 1, 40);
		RequestNewMaze();
	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_MINUS))
	{
		size = max(size - 1, 5);
		RequestNewMaze();
	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_PERIOD))
	{
		density = min(density + 0.1f, 1.0f);
		RequestNewMaze();
	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_COMMA))
	{
		density = max(density - 0.1f, 0.0);
		RequestNewMaze();
	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_P)) {
		drawPath = !drawPath;
	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_K)) {
		drawMesh = !drawMesh;
	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_Q)) {
		drawTiles = !drawTiles;
		if (maze && drawTiles) { maze->Render()->GetChildWithName("MazeNodes")->Wake(); }
		else if (maze && !drawTiles) { maze->Render()->GetChildWithName("MazeNodes")->Sleep(); }
	}
}


Vector3 Net1_Client::Pos_To_Maze(Vector3 pos) {
	int flat_maze_size = size * 3 - 1;
	const float scalar = 1.f / (float)flat_maze_size;

	Vector3 cellpos = Vector3(
		pos.x * 3,
		0.0f,
		pos.y * 3
	) * scalar;

	Vector3 cellsize = Vector3(
		scalar * 2,
		1.0f,
		scalar * 2
	);

	return (cellpos + cellsize * 0.5f);
}

Vector3 Net1_Client::Maze_Scale() {
	int flat_maze_size = size * 3 - 1;
	const float scalar = 1.f / (float)flat_maze_size;

	Vector3 cellsize = Vector3(
		scalar * 1.8,
		1.2f,
		scalar * 1.8
	);

	return cellsize * 0.5f;
}


//--------------------------------------------------------------------------------------------//
// Point&Click interface callback
//--------------------------------------------------------------------------------------------//
void Net1_Client::ClickableLocationCallback(int idx) {
	if (Window::GetMouse()->ButtonDown(MOUSE_LEFT)) {
		generator->SetEndNode(idx);
		if (avatars[ID] != OUT_OF_RANGE) {
			generator->SetStartNode(avatars[ID]);
		}
		if (generator->GetStartNode())
			SendStartPosition();
		SendEndPosition();
		maze->UpdateRenderer();
	}
	else if (Window::GetMouse()->ButtonDown(MOUSE_RIGHT)) {
		generator->SetStartNode(idx);
		SendAvatarLocation();
		SendStartPosition();
		maze->UpdateRenderer();
	}
}