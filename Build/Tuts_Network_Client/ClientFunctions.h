#pragma once
#include <ncltech\PhysicsEngine.h>
#include <ncltech\SceneManager.h>
#include <nclgl\NCLDebug.h>
#include <nclgl\PerfTimer.h>
#include <ncltech\NetworkBase.h>

struct MazeStruct {
	int size;
	vector<int> walls;
};

struct PosStruct {
	int idx;
	Vector3 pos;
};

void PrintStatusEntries();
void HandleKeyboardInputs();
int ClientLoop();

MazeStruct Recieve_maze(const ENetEvent& evnt);
PosStruct Recieve_startpos(const ENetEvent& evnt);
vector<int> Recieve_path(const ENetEvent& evnt);

vector<string> split_string(string s, char d);
vector<int> split_string_toInt(string s, char d);
vector<float> split_string_toFloat(string s, char d);
bool isNumber(char c);