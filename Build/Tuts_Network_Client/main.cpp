

#include <enet\enet.h>  //<-- MUST include this before "<nclgl\Window.h>"
#include <nclgl\Window.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\SceneManager.h>
#include <nclgl\NCLDebug.h>
#include <nclgl\PerfTimer.h>
#include "Server.h"
#include "ClientFunctions.h"
#include "Net1_Client.h"

enum Type {SERVER,CLIENT};
Type thisType = SERVER;
Net1_Client * c;

void Quit(bool error = false, const string &reason = "");

Server * s = nullptr;

void Initialize()
{
	//Initialise the Window
	if (!Window::Initialise("Client", 1280, 800, false))
		Quit(true, "Window failed to initialise!");

	//Initialise ENET for networking  //!!!!!!NEW!!!!!!!!
	if (enet_initialize() != 0)
	{
		Quit(true, "ENET failed to initialize!");
	}

	//Initialise the PhysicsEngine
	PhysicsEngine::Instance();

	//Initialize Renderer
	GraphicsPipeline::Instance();
	SceneManager::Instance();	//Loads CommonMeshes in here (So everything after this can use them globally e.g. our scenes)

								//Enqueue All Scenes
								// - Add any new scenes you want here =D
	c = new Net1_Client("Network #1 - Example Client");
	SceneManager::Instance()->EnqueueScene(c);
}

void Quit(bool error, const string &reason) {
	//Release Singletons
	SceneManager::Release();
	GraphicsPipeline::Release();
	PhysicsEngine::Release();
	enet_deinitialize();  //!!!!!!!!!!!!!!!!!NEW!!!!!!!!!!!!!!
	Window::Destroy();
	

						  //Show console reason before exit
	if (error) {
		std::cout << reason << std::endl;
		system("PAUSE");
		exit(-1);
	}
}


int main()
{
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}
	s = new Server();
	NetworkBase * server = s->getBase();
	//Initialize Server on Port 1234, with a possible 32 clients connected at any time
	// If  a server already exists on this port then this will not intialise. 
	// It then chooses to createa client instead
	if (!server->Initialize(SERVER_PORT, 32))
	{
		fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
		//onExit(EXIT_FAILURE);
		thisType = CLIENT;
		//Initialize our Window, Physics, Scenes etc
		Initialize();
	}

	// If server run server loop, if client run client loop
	switch (thisType) {
	case SERVER:
		return s->ServerLoop();
		break;

	case CLIENT:
		int i = ClientLoop(c);
		enet_deinitialize(); 
		return 0;
		break;
	}
}
