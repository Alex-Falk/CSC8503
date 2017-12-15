#pragma once
#include <ncltech\GameObject.h>
#include <ncltech\Scene.h>
#include "SearchAlgorithm.h"

#define OUT_OF_RANGE -1

//Maze Generator which is mainly taken from the tutorial but extended to handle things such as multiple start and goal nodes
//The multiple start and end nodes are used on the server side, and the single start and end node are used on the client side

class MazeGenerator
{
public:
	MazeGenerator(); //Maze_density goes from 1 (single path to exit) to 0 (no walls at all)
	virtual ~MazeGenerator();

	void Generate(int size, float maze_density);

	//All points on the maze grid are connected in some shape or form
	// so any two nodes picked randomly /will/ have a path between them
	GraphNode* GetStartNode() const { return start; }
	GraphNode* GetGoalNode()  const { return end; }
	GraphNode* GetNode(int i) const { return &(allNodes[i]); }
	
	void SetStartNode(int i)		{
		if (i != OUT_OF_RANGE)
			start = &(allNodes[i]);
		else
			start = nullptr;
	}


	void SetEndNode(int i)			{
		if (i != OUT_OF_RANGE)
			end = &(allNodes[i]);
		else
			end = nullptr;
	}

	void Push_StartNode(int i)		{ startnodes.push_back(&(allNodes[i])); }
	void Push_EndNode(int i)		{ endnodes.push_back(&(allNodes[i])); }

	void SetStartNode(int idx, int i)	{ 
		if (startnodes.size() <= idx) 
		{ 
			if (i != OUT_OF_RANGE)
				startnodes.push_back(&(allNodes[i]));
			else
				startnodes.push_back(nullptr);
		}
		else { 
			if (i != OUT_OF_RANGE)
				startnodes[idx] = &(allNodes[i]);
			else
				startnodes[idx] = nullptr;
		}
	}
	void SetEndNode(int idx, int i)	{
		if (endnodes.size() <= idx) { endnodes.push_back(&(allNodes[i])); }
		else { endnodes[idx] = &(allNodes[i]); }
	}

	

	uint GetSize() const { return size; }


	//Used as a hack for the MazeRenderer to generate the walls more effeciently
	GraphNode* GetAllNodesArr() { return allNodes; }

	string AllWalls();

protected:
	void GetRandomStartEndNodes();

	void Initiate_Arrays();

	void Generate_Prims();
	void Generate_Sparse(float density);



public:
	uint size;
	float density = 0.0f;
	GraphNode *start, *end;
	vector<GraphNode*> startnodes;
	vector<GraphNode*> endnodes;

	GraphNode* allNodes;
	GraphEdge* allEdges;
};