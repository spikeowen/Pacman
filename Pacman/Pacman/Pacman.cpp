#include "Pacman.h"
#include <time.h>
#include <sstream>
#include <iostream>

Pacman::Pacman(int argc, char* argv[]) : Game(argc, argv) , cSpeed(0.1f), cFrameTime(225)
{
	_pacman = new Player();
	_menu = new Screen();
	_cherry = new Enemy();
	srand(time(NULL));
	
	for (int i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchies[i] = new Enemy();
		//Munchie frame count
		_munchies[i]->currentFrameTime = 0;
		_munchies[i]->frame = rand() % 1;
		_munchies[i]->frameTime = rand() % 500 + 50;
	}
	_pop = new SoundEffect();
	_dead = new SoundEffect();

	//Pacman frame count
	_pacman->currentFrameTime = 0;
	_pacman->frame = 0;
	_pacman->speedMultiplier = 1.0f;
	//Pacman facing direction
	_pacman->direction = 0;
	_pacman->dead = false;

	_cherry->currentFrameTime = 0;
	_cherry->frame = rand() % 1;
	_cherry->frameTime = rand() % 500 + 50;

	//initialise ghost character
	for (int i = 0; i < GHOSTCOUNT; i++)
	{
		_ghosts[i] = new MovingEnemy();
		_ghosts[i]->speed = 0.075f;
		_ghosts[i]->currentFrameTime = 0;
		_ghosts[i]->frame = rand() % 1;
		_ghosts[i]->frameTime = rand() % 500 + 50;
	}
	_ghosts[0]->direction = 0;
	_ghosts[1]->direction = 2;
	_ghosts[2]->direction = 4;


	//Pausing check variables
	_menu->paused = false;
	_menu->pKeyDown = false;

	//Start menu check variables
	_menu->started = false;
	_menu->mKeyDown = false;

	//Initialise important Game aspects
	Audio::Initialise();
	Graphics::Initialise(argc, argv, this, 1024, 768, false, 25, 25, "Pacman", 60);
	Input::Initialise();

	// Start the Game Loop - This calls Update and Draw in game loop
	Graphics::StartGameLoop();
}

Pacman::~Pacman()
{
	delete _pacman->texture;
	delete _pacman->sourceRect;

	delete _munchies[0]->texture;
	for (int nCount = 0; nCount < MUNCHIECOUNT; nCount++)
	{
		delete _munchies[nCount]->texture;
		delete _munchies[nCount]->position;
		delete _munchies[nCount]->rect;
		delete _munchies[nCount];
	}

	delete[] _munchies;

	delete _cherry->texture;
	delete _cherry->position;
	delete _cherry->rect;
	delete _cherry;

	delete _ghosts[0]->texture;
	for (int nCount = 0; nCount < GHOSTCOUNT; nCount++)
	{
		delete _ghosts[nCount]->texture;
		delete _ghosts[nCount]->position;
		delete _ghosts[nCount]->sourceRect;
		delete _ghosts[nCount];
	}

	delete _pop;
	delete _dead;
}

void Pacman::LoadContent()
{
	// Set Menu Paramters
	_menu->Background = new Texture2D();
	_menu->Background->Load("Textures/Transparency.png", false);
	_menu->Rectangle = new Rect(0.0f, 0.0f, Graphics::GetViewportWidth(), Graphics::GetViewportHeight());
	_menu->StringPosition = new Vector2(Graphics::GetViewportWidth() / 2.0f, Graphics::GetViewportHeight() / 2.0f);

	// Load Pacman
	_pacman->texture = new Texture2D();
	_pacman->texture->Load("Textures/Pacman.tga", false);
	_pacman->position = new Vector2(350.0f, 350.0f);
	_pacman->sourceRect = new Rect(0.0f, 0.0f, 32, 32);

	// Load Munchie
	Texture2D* munchieTex = new Texture2D();
	munchieTex->Load("Textures/Munchies.png", false);
	for (int i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchies[i]->texture = munchieTex;
		_munchies[i]->position = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()));
		_munchies[i]->rect = new Rect(12.0f, 12.0f, 12, 12);
	}

	// Load Cherry
	Texture2D* cherryTex = new Texture2D();
	cherryTex->Load("Textures/Cherries.png", true);
	_cherry->texture = cherryTex;
	_cherry->rect = new Rect(0.0f, 0.0f, 32, 32);
	_cherry->position = new Vector2((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()));

	//Load Ghost
	_ghosts[0]->texture = new Texture2D();
	_ghosts[0]->texture->Load("Textures/GhostRed.png", false);
	_ghosts[1]->texture = new Texture2D();
	_ghosts[1]->texture->Load("Textures/GhostBlue.png", false);
	_ghosts[2]->texture = new Texture2D();
	_ghosts[2]->texture->Load("Textures/GhostPink.png", false);
	for (int i = 0; i < GHOSTCOUNT; i++)
	{
		_ghosts[i]->position = new Vector2((rand() % (Graphics::GetViewportWidth() - 20)), (rand() % (Graphics::GetViewportHeight() - 20)));
		_ghosts[i]->sourceRect = new Rect(0.0f, 0.0f, 20, 20);
	}

	//Load sounds
	_pop->Load("Sounds/pop.wav");
	_dead->Load("Sounds/game-over.wav");

	// Set string position
	_stringPosition = new Vector2(10.0f, 25.0f);
}

void Pacman::Update(int elapsedTime)
{
	// Gets the current state of the keyboard
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();
	//Gets the current state of the mouse
	Input::MouseState* mouseState = Input::Mouse::GetState();

	CheckStarted(keyboardState, Input::Keys::M);
	if (!_menu->started)
	{
		//check for start
		if (keyboardState->IsKeyDown(Input::Keys::M))
		{
			_menu->started = true;
		}

	}

	CheckPaused(keyboardState, Input::Keys::P);

	if (!_menu->paused && _menu->started && !_pacman->dead)
	{
		Input(elapsedTime, keyboardState, mouseState);
		UpdatePacman(elapsedTime);
		CheckViewportCollision();
		for (int i = 0; i < MUNCHIECOUNT; i++)
		{
			UpdateMunchies(_munchies[i],elapsedTime);
		}
		UpdateCherries(_cherry, elapsedTime);
		for (int i = 0; i < GHOSTCOUNT; i++)
		{
			UpdateGhost(_ghosts[i], elapsedTime);
		}
		CheckGhostCollisions();
		CheckMunchieCollisions();
	}
}

void Pacman::Draw(int elapsedTime)
{
	// Allows us to easily create a string
	std::stringstream stream;
	stream << "Pacman X: " << _pacman->position->X << " Y: " << _pacman->position->Y << " Score: " << _pacman->score;

	SpriteBatch::BeginDraw(); // Starts Drawing
	if (!_pacman->dead) 
	{
		SpriteBatch::Draw(_pacman->texture, _pacman->position, _pacman->sourceRect); // Draws Pacman
	}
	//Draws munchies
	for (int i = 0; i < MUNCHIECOUNT; i++)
	{
		SpriteBatch::Draw(_munchies[i]->texture, _munchies[i]->position, _munchies[i]->rect);
	}

	//draw ghosts
	for (int i = 0; i < GHOSTCOUNT; i++)
	{
		SpriteBatch::Draw(_ghosts[i]->texture, _ghosts[i]->position, _ghosts[i]->sourceRect);
	}

	//Draws Cherry
	SpriteBatch::Draw(_cherry->texture, _cherry->position, _cherry->rect);

	if (_menu->paused && _menu->started)
	{
		std::stringstream menuStream;
		menuStream << "PAUSED!";
		SpriteBatch::Draw(_menu->Background, _menu->Rectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menu->StringPosition, Color::Red);
	}

	if (!_menu->started)
	{
		std::stringstream menuStream;
		menuStream << "MAIN MENU\nPRESS M TO START";
		SpriteBatch::Draw(_menu->Background, _menu->Rectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menu->StringPosition, Color::Yellow);
	}

	if (_pacman->dead)
	{
		std::stringstream menuStream;
		menuStream << "GAME OVER!";
		SpriteBatch::Draw(_menu->Background, _menu->Rectangle, nullptr);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menu->StringPosition, Color::Red);
	}

	// Draws String
	SpriteBatch::DrawString(stream.str().c_str(), _stringPosition, Color::Green);
	SpriteBatch::EndDraw(); // Ends Drawing
}

void Pacman::Input(int elapsedTime, Input::KeyboardState* state, Input::MouseState* mouseState)
{
	float pacmanSpeed = cSpeed * elapsedTime * _pacman->speedMultiplier;
	// Checks if a key is pressed to move pacman in ONE direction
	if (state->IsKeyDown(Input::Keys::D))//Moves RIGHT across x axis
	{
		_pacman->position->X += pacmanSpeed;
		_pacman->direction = 0;
	}
	else if (state->IsKeyDown(Input::Keys::W))
	{
		_pacman->position->Y -= pacmanSpeed; //Moves Pacman UP across Y axis
		_pacman->direction = 3;
	}
	else if (state->IsKeyDown(Input::Keys::A))
	{
		_pacman->position->X -= pacmanSpeed; //Moves Pacman LEFT across X axis
		_pacman->direction = 2;
	}
	else if (state->IsKeyDown(Input::Keys::S))
	{
		_pacman->position->Y += pacmanSpeed; //Moves Pacman DOWN across Y axis
		_pacman->direction = 1;
	}

	// Handle Mouse Input – Reposition Cherry 
	if (mouseState->LeftButton == Input::ButtonState::PRESSED)
	{
		_cherry->position->X = mouseState->X;
		_cherry->position->Y = mouseState->Y;
	}

	// Speed Multiplier
	if (state->IsKeyDown(Input::Keys::LEFTSHIFT))
	{
		// Apply multiplier
		_pacman->speedMultiplier = 2.0f;
	}
	else
	{
		// Reset multiplier
		_pacman->speedMultiplier = 1.0f;
	}

	if (state->IsKeyDown(Input::Keys::R))
	{
		_cherry->position->X = rand() % Graphics::GetViewportWidth();
		_cherry->position->Y = rand() % Graphics::GetViewportHeight();
	}
}

void Pacman::CheckPaused(Input::KeyboardState* state, Input::Keys pauseKey)
{
	//Pause button
	if (state->IsKeyDown(Input::Keys::P) && !_menu->pKeyDown && _menu->started)
	{
		_menu->pKeyDown = true;
		_menu->paused = !_menu->paused;
	}

	if (state->IsKeyUp(Input::Keys::P))
	{
		_menu->pKeyDown = false;
	}
}

void Pacman::CheckStarted(Input::KeyboardState* state, Input::Keys startKey)
{
	//Start button
	if (state->IsKeyDown(Input::Keys::M) && !_menu->mKeyDown)
	{
		_menu->mKeyDown = true;
		_menu->started = !_menu->started;
	}

	if (state->IsKeyUp(Input::Keys::M))
	{
		_menu->mKeyDown = false;
	}
}

void Pacman::CheckViewportCollision()
{
	//Wrapping feature
	if (_pacman->position->X + _pacman->sourceRect->Width > Graphics::GetViewportWidth())
	{
		_pacman->position->X = 0 - _pacman->sourceRect->Width;
	}
	if (_pacman->position->X + _pacman->sourceRect->Width < 0)
	{
		_pacman->position->X = Graphics::GetViewportWidth() - _pacman->sourceRect->Width;
	}
	if (_pacman->position->Y + _pacman->sourceRect->Height > Graphics::GetViewportHeight())
	{
		_pacman->position->Y = 0 - _pacman->sourceRect->Height;
	}
	if (_pacman->position->Y + _pacman->sourceRect->Height < 0)
	{
		_pacman->position->Y = Graphics::GetViewportHeight() - _pacman->sourceRect->Height;
	}
}

void Pacman::UpdatePacman(int elapsedTime)
{
	//Pacman animation updater
	_pacman->currentFrameTime += elapsedTime;
	if (_pacman->currentFrameTime > cFrameTime)
	{
		_pacman->frame++;
		if (_pacman->frame >= 2)
			_pacman->frame = 0;

		_pacman->currentFrameTime = 0;
	}
	_pacman->sourceRect->X = _pacman->sourceRect->Width * _pacman->frame;

	//Updating his sprite to match direction of movement
	_pacman->sourceRect->Y = _pacman->sourceRect->Height * _pacman->direction;
}

void Pacman::UpdateMunchies(Enemy*, int elapsedTime)
{
	//Munchie animation updater
	for (int i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchies[i]->currentFrameTime += elapsedTime;
		if (_munchies[i]->currentFrameTime > _munchies[i]->frameTime)
		{
			_munchies[i]->frame++;
			if (_munchies[i]->frame >= 2)
				_munchies[i]->frame = 0;

			_munchies[i]->currentFrameTime = 0;
		}
		_munchies[i]->rect->X = _munchies[i]->rect->Width * _munchies[i]->frame;
	}
}

void Pacman::UpdateCherries(Enemy*, int elapsedTime)
{
	//Munchie animation updater
	_cherry->currentFrameTime += elapsedTime;
	if (_cherry->currentFrameTime > _cherry->frameTime)
	{
		_cherry->frame++;
		if (_cherry->frame >= 2)
			_cherry->frame = 0;

		_cherry->currentFrameTime = 0;
	}
	_cherry->rect->X = _cherry->rect->Width * _cherry->frame;
}

void Pacman::UpdateGhost(MovingEnemy* ghost, int elapsedTime)
{
	//Ghost Movements
	if (ghost->direction == 0) //Moves Right 
	{
		ghost->position->X += ghost->speed * elapsedTime;
	}
	else if (ghost->direction == 1) //Moves Left 
	{
		ghost->position->X -= ghost->speed * elapsedTime;
	}
	else if (ghost->direction == 2)//Moves Up
	{
		ghost->position->Y -= ghost->speed * elapsedTime;
	}
	else if (ghost->direction == 3)//Moves Down
	{
		ghost->position->Y += ghost->speed * elapsedTime;
	}
	else if (ghost->direction == 4)//Moves Up and Right Diagonally
	{
		ghost->position->Y -= ghost->speed * elapsedTime;
		ghost->position->X += ghost->speed * elapsedTime;
	}
	else if (ghost->direction == 5)//Moves Up and Left Diagonally
	{
		ghost->position->Y -= ghost->speed * elapsedTime;
		ghost->position->X -= ghost->speed * elapsedTime;
	}
	else if (ghost->direction == 6)//Moves Down and Right Diagonally
	{
		ghost->position->Y += ghost->speed * elapsedTime;
		ghost->position->X += ghost->speed * elapsedTime;
	}
	else if (ghost->direction == 7)//Moves Down and Left Diagonally
	{
		ghost->position->Y += ghost->speed * elapsedTime;
		ghost->position->X -= ghost->speed * elapsedTime;
	}

	//Red and Blue Ghost Moves
	if (ghost->direction < 4)
	{
		if (ghost->position->X + ghost->sourceRect->Width >= Graphics::GetViewportWidth()) //Hits Right edge 
		{
			ghost->position->X = Graphics::GetViewportWidth() - ghost->sourceRect->Width;
			ghost->direction = 1; //Change direction to left
			ghost->speed = ghost->speed + 0.01f;
		}
		else if (ghost->position->X <= 0) //Hits left edge 
		{
			ghost->position->X = 0 + ghost->sourceRect->Width;
			ghost->direction = 0; //Change direction to right
			ghost->speed = ghost->speed + 0.01f;
		}
		if (ghost->position->Y + ghost->sourceRect->Height >= Graphics::GetViewportHeight()) //Hits Bottom edge 
		{
			ghost->position->Y = Graphics::GetViewportHeight() - ghost->sourceRect->Height;
			ghost->direction = 2; //Change direction to up
			ghost->speed = ghost->speed + 0.005f;
		}
		else if (ghost->position->Y <= 0) //Hits top edge 
		{
			ghost->position->Y = 0 + ghost->sourceRect->Height;
			ghost->direction = 3; //Change direction to down
			ghost->speed = ghost->speed + 0.005f;
		}
	}
	//Pink Ghost Moves
	else if (ghost->direction >= 4)
	{
		if (ghost->position->X + ghost->sourceRect->Width >= Graphics::GetViewportWidth()) //Hits Right edge 
		{
			ghost->position->X = Graphics::GetViewportWidth() - ghost->sourceRect->Width;
			ghost->direction++; //Change direction to left
			ghost->speed = ghost->speed + 0.01f;
		}
		else if (ghost->position->X <= 0) //Hits left edge 
		{
			ghost->position->X = 0 + ghost->sourceRect->Width;
			ghost->direction--; //Change direction to right
			ghost->speed = ghost->speed + 0.01f;
		}
		if (ghost->position->Y + ghost->sourceRect->Height >= Graphics::GetViewportHeight()) //Hits Bottom edge 
		{
			ghost->position->Y = Graphics::GetViewportHeight() - ghost->sourceRect->Height;
			ghost->direction = ghost->direction - 2; //Change diagonal direction to up
			ghost->speed = ghost->speed + 0.01f;
		}
		else if (ghost->position->Y <= 0) //Hits top edge 
		{
			ghost->position->Y = 0 + ghost->sourceRect->Height;
			ghost->direction = ghost->direction + 2; //Change diagonal direction to down
			ghost->speed = ghost->speed + 0.01f;
		}
	}

	//Ghost Speed Limiter
	if (ghost->speed >= 1.0f)
		ghost->speed = 1.0f;

	//Ghost animation updater
	for (int i = 0; i < GHOSTCOUNT; i++)
	{
		ghost->currentFrameTime += elapsedTime;
		if (ghost->currentFrameTime > ghost->frameTime)
		{
			ghost->frame++;
			if (ghost->frame >= 2)
				ghost->frame = 0;

			ghost->currentFrameTime = 0;
		}
		ghost->sourceRect->X = ghost->sourceRect->Width * ghost->frame;

		//Updating the ghost sprite to match direction of movement
		if (ghost->direction == 1 || ghost->direction == 3 || ghost->direction == 5)
		{
			ghost->sourceRect->Y = ghost->sourceRect->Height * 1;
		}
		else if (ghost->direction == 6)
		{
			ghost->sourceRect->Y = ghost->sourceRect->Height * 2;
		}
		else if (ghost->direction == 7)
		{
			ghost->sourceRect->Y = ghost->sourceRect->Height * 3;
		}
		else
			ghost->sourceRect->Y = ghost->sourceRect->Height * 0;
	}
}

void Pacman::CheckGhostCollisions()
{
	// Local Variables
	int top1 = _pacman->position->Y;
	int top2 = 0;
	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;

	for (int i = 0; i < GHOSTCOUNT; i++)
	{
		// Populate variables with Ghost data
		bottom2 = _ghosts[i]->position->Y + _ghosts[i]->sourceRect->Height;
		left2 = _ghosts[i]->position->X;
		right2 = _ghosts[i]->position->X + _ghosts[i]->sourceRect->Width;
		top2 = _ghosts[i]->position->Y;

		if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2))
		{
			Audio::Play(_dead);
			//_pacman->dead = true;
			i = GHOSTCOUNT;
		}
	}
}

void Pacman::CheckMunchieCollisions()
{
	// Local Variables
	int top1 = _pacman->position->Y;
	int top2 = 0;
	int bottom1 = _pacman->position->Y + _pacman->sourceRect->Height;
	int bottom2 = 0;
	int left1 = _pacman->position->X;
	int left2 = 0;
	int right1 = _pacman->position->X + _pacman->sourceRect->Width;
	int right2 = 0;

	for (int i = 0; i < MUNCHIECOUNT; i++)
	{
		// Populate variables with Munchie data
		bottom2 = _munchies[i]->position->Y + _munchies[i]->rect->Height;
		left2 = _munchies[i]->position->X;
		right2 = _munchies[i]->position->X + _munchies[i]->rect->Width;
		top2 = _munchies[i]->position->Y;

		if ((bottom1 > top2) && (top1 < bottom2) && (right1 > left2) && (left1 < right2))
		{
			Audio::Play(_pop);
			_pacman->score++;
			_munchies[i]->position->Y = -100;
			_munchies[i]->position->X = -100;
			for (int i = 0; i < GHOSTCOUNT; i++)
			{
				_ghosts[i]->speed = _ghosts[i]->speed + 0.025f;
			}
		}
	}
}