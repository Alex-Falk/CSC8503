#pragma once
#include <fstream>
#include "MazeGenerator.h"

std::string All_walls(GraphEdge * edges, int size) {
	string walls = to_string(size) + ":";

	uint base_offset = size * (size - 1);

	for (uint i = 0; i < base_offset * 2; ++i) {
		if (edges[i]._iswall) {
			walls += to_string(i) + " ";
		}
	}

	return walls;
}