#pragma once
#include <ncltech\PhysicsEngine.h>
#include <ncltech\SceneManager.h>
#include <nclgl\NCLDebug.h>
#include <nclgl\PerfTimer.h>
#include <ncltech\NetworkBase.h>

class Net1_Client;

struct MazeStruct {
	int size;
	float density;
	vector<int> walls;
};

struct PosStruct {
	int idx;
	Vector3 pos;
};

void PrintStatusEntries();
void HandleKeyboardInputs(Net1_Client * c);
int ClientLoop(Net1_Client * c);

MazeStruct Recieve_maze(const ENetEvent& evnt);
PosStruct Recieve_pos(const ENetEvent& evnt);
vector<PosStruct> Recieve_positions(const ENetEvent& evnt);
vector<int> Recieve_path(const ENetEvent& evnt);

vector<string> split_string(string s, char d);
vector<int> split_string_toInt(string s, char d);
vector<float> split_string_toFloat(string s, char d);
bool isNumber(char c);