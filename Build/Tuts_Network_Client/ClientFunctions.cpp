#include "ClientFunctions.h"
#include "Net1_Client.h"

void PrintStatusEntries()
{
	const Vector4 status_color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	//Print Current Scene Name
	NCLDebug::AddStatusEntry(status_color, "[%d/%d]: %s ([T]/[Y] to cycle or [R] to reload)",
		SceneManager::Instance()->GetCurrentSceneIndex() + 1,
		SceneManager::Instance()->SceneCount(),
		SceneManager::Instance()->GetCurrentScene()->GetSceneName().c_str()
	);
}

void HandleKeyboardInputs(Net1_Client * c)
{
	uint sceneIdx = SceneManager::Instance()->GetCurrentSceneIndex();
	uint sceneMax = SceneManager::Instance()->SceneCount();

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_Y))
		SceneManager::Instance()->JumpToScene((sceneIdx + 1) % sceneMax);

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_T))
		SceneManager::Instance()->JumpToScene((sceneIdx == 0 ? sceneMax : sceneIdx) - 1);

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_R)) {
		SceneManager::Instance()->JumpToScene(sceneIdx);
	}

	c->HandleKeyboardInputs();
}

int ClientLoop(Net1_Client * c)
{
	Window::GetWindow().GetTimer()->GetTimedMS();

	//Create main game-loop
	while (Window::GetWindow().UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
		//Start Timing
		float dt = Window::GetWindow().GetTimer()->GetTimedMS() * 0.001f;	//How many milliseconds since last update?

																			//Print Status Entries
		PrintStatusEntries();

		//Handle Keyboard Inputs
		HandleKeyboardInputs(c);

		//Update Scene
		SceneManager::Instance()->GetCurrentScene()->FireOnSceneUpdate(dt);

		//Update Physics
		//PhysicsEngine::Instance()->Update(dt);
		//PhysicsEngine::Instance()->DebugRender();

		//Render Scene

		GraphicsPipeline::Instance()->UpdateScene(dt);
		GraphicsPipeline::Instance()->RenderScene();				 //Finish Timing
	}

	//Cleanup
	return 0;
}

MazeStruct Recieve_maze(const ENetEvent& evnt) {

	MazeStruct m;

	string packet;

	for (int i = 2; i < evnt.packet->dataLength; ++i) {
		packet.push_back(evnt.packet->data[i]);
	}

	if (packet.find_first_of(':') != string::npos) {
		m.size = stoi(packet.substr(0, packet.find_first_of(':')));

		packet = packet.substr(packet.find_first_of(':')+1);

		m.density = stof(packet.substr(0, packet.find_first_of(':')));

		packet = packet.substr(packet.find_first_of(':') + 1);

		vector<int> split_elements = split_string_toInt(packet, ' ');

		m.walls = split_elements;
	}

	return m;
}

PosStruct Recieve_pos(const ENetEvent& evnt) {

	PosStruct p;
	Vector3 pos;

	string packet;
	for (int i = 2; i < evnt.packet->dataLength; ++i) {
		packet.push_back(evnt.packet->data[i]);
	}

	p.idx = stoi(packet.substr(0, packet.find_first_of(' ')));

	string indcs = packet.substr(packet.find_first_of(' ') + 1);
	vector<float> split_elements = split_string_toFloat(packet, ' ');

	pos.x = split_elements[0];
	pos.y = split_elements[1];
	pos.z = split_elements[2];

	p.pos = pos;
	
	return p;

}

vector<PosStruct> Recieve_positions(const ENetEvent& evnt) {

	string packet;
	for (int i = 2; i < evnt.packet->dataLength; ++i) {
		packet.push_back(evnt.packet->data[i]);
	}

	vector<float> split_elements = split_string_toFloat(packet, ' ');

	vector<PosStruct> positions;
	for (int i = 0; i < split_elements.size(); i += 4)
	{
		PosStruct p;
		p.idx = split_elements[i];
		p.pos.x = split_elements[i+1];
		p.pos.y = split_elements[i+2];
		p.pos.z = split_elements[i+3];

		positions.push_back(p);
	}
	return positions;

}

vector<int> Recieve_path(const ENetEvent & evnt)
{
	string packet;
	for (int i = 2; i < evnt.packet->dataLength; ++i) {
		packet.push_back(evnt.packet->data[i]);
	}

	vector<int> split_elements = split_string_toInt(packet, ' ');

	return split_elements;
}

vector<string> split_string(string s, char d) {
	vector<string> chars;

	size_t delim_idx = s.find_first_of(d);
	if (delim_idx) {
		chars.push_back(s.substr(0, delim_idx));
		vector<string> subs = split_string(s.substr(delim_idx), d);
		for (vector<string>::iterator it = subs.begin(); it != subs.end(); ++it) {
			chars.push_back((*it));
		}
	}

	return chars;
}

vector<int> split_string_toInt(string s, char d) {
	vector<int> chars;

	size_t delim_idx = s.find_first_of(d);
	if (delim_idx != string::npos && s != "" && s != " ") {
		chars.push_back(stoi(s.substr(0, delim_idx)));
		vector<int> subs = split_string_toInt(s.substr(delim_idx+1), d);
		for (vector<int>::iterator it = subs.begin(); it != subs.end(); ++it) {
			chars.push_back((*it));
		}
	}

	return chars;
}

vector<float> split_string_toFloat(string s, char d) {
	vector<float> chars;

	size_t delim_idx = s.find_first_of(d);
	if (delim_idx != string::npos && s != "" && s != " ") {
		chars.push_back(stof(s.substr(0, delim_idx)));
		vector<float> subs = split_string_toFloat(s.substr(delim_idx + 1), d);
		for (vector<float>::iterator it = subs.begin(); it != subs.end(); ++it) {
			chars.push_back((*it));
		}
	}
	else if (s != "") {
		chars.push_back(stof(s));
	}
	return chars;
}

bool isNumber(char c) {
	string nums = "0123456789";
	if (nums.find_first_of(c)) {
		return true;
	}
	else {
		return false;
	}
}

