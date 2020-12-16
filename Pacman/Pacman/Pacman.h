#pragma once

// If Windows and not in Debug, this will run without a console window
// You can use this to output information when debugging using cout or cerr
#ifdef WIN32 
	#ifndef _DEBUG
		#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	#endif
#endif

// Just need to include main header file
#include "S2D/S2D.h"

// Reduces the amount of typing by including all classes in S2D namespace
using namespace S2D;

//Definitions of data
#define MUNCHIECOUNT 50
#define GHOSTCOUNT 3

// Data to represent Pacman
struct Player
{
	bool dead;
	Vector2* position;
	Rect* sourceRect;
	Texture2D* texture;

	int score;
	//Adding direction
	int direction;
	//Sprint variable
	float speedMultiplier;
	//Animation variables
	int frame;
	int currentFrameTime;
};

struct MovingEnemy
{
	Vector2* position;
	Texture2D* texture;
	Rect* sourceRect;
	int direction;
	float speed;

	//Ghost animation variables
	int frame;
	int currentFrameTime;
	int frameTime;
};


struct Enemy
{
	// Data to represent Munchie/Cherry
	Vector2* position;
	Rect* rect;
	Texture2D* texture;

	//Munchie/Cherry animation variables
	int frame;
	int currentFrameTime;
	int frameTime;
};

struct Screen
{
	// Data for Menus
	Texture2D* Background;
	Rect* Rectangle;
	Vector2* StringPosition;
	bool paused;
	bool pKeyDown;
	bool started;
	bool mKeyDown;
};

// Declares the Pacman class which inherits from the Game class.
// This allows us to overload the Game class methods to help us
// load content, draw and update our game.
class Pacman : public Game
{
private:
	Player* _pacman;
	Enemy* _munchies[MUNCHIECOUNT];
	MovingEnemy* _ghosts[GHOSTCOUNT];
	Screen* _menu;
	Enemy* _cherry;
	SoundEffect* _pop;
	SoundEffect* _dead;

	//Input methods
	void Input(int elapsedTime, Input::KeyboardState* state, Input::MouseState* mouseState);

	//Check methods
	void CheckPaused(Input::KeyboardState* state, Input::Keys pauseKey);
	void CheckStarted(Input::KeyboardState* state, Input::Keys startKey);
	void CheckViewportCollision();
	void CheckGhostCollisions();
	void CheckMunchieCollisions();

	//Update methods
	void UpdatePacman(int elapsedTime);
	void UpdateMunchies(Enemy*, int elapsedTime);
	void UpdateCherries(Enemy*, int elapsedTime);
	void UpdateGhost(MovingEnemy*, int elapsedTime);

	//New constants
	const float cSpeed;
	const int cFrameTime;

	// Position for String
	Vector2* _stringPosition;

public:
	/// <summary> Constructs the Pacman class. </summary>
	Pacman(int argc, char* argv[]);

	/// <summary> Destroys any data associated with Pacman class. </summary>
	virtual ~Pacman();

	/// <summary> All content should be loaded in this method. </summary>
	void virtual LoadContent();

	/// <summary> Called every frame - update game logic here. </summary>
	void virtual Update(int elapsedTime);

	/// <summary> Called every frame - draw game here. </summary>
	void virtual Draw(int elapsedTime);
};