#define RAYMATH_IMPLEMENTATION
#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <vector>


#pragma region imgui
#include "imgui.h"
#include "rlImGui.h"
#include "imguiThemes.h"
#pragma endregion


const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const int MAX_METEORS = 20;


#pragma region Game Data 
struct Player
{
	Vector2 playerPos;
	Vector2 playerDir;
	int playerWidth;
	int playerHeight;
	float playerSpeed;
	Texture2D playerTexture;
	Rectangle playerSource;
	Rectangle playerDes;
	Vector2 playerOrigin;
	float playerRotation;
};

struct Meteor
{
	bool isActive;
	Vector2 meteorPos;
	Vector2 meteorDir;
	int meteorWidth;
	int meteorHeight;
	Vector2 meteorSpeed;
	float meteorRot;
	int type;
};

enum MeteorType
{
	TINY = 0,
	MID,
	LARGE
};

Texture2D meteorTexture[3];


enum AudioType
{
	LASER_SHOT
};

Sound sounds[1];


struct Background
{
	Texture2D bgTexture;
	Vector2 bgPos;
	Rectangle bgRect;
	float bgScale;
};

enum BulletType
{
	B_TINY = 0,
	B_MID,
	B_LARGE
};

Texture2D bulletTexture[3];

struct Bullet
{
	Vector2 bulletPos;
	Vector2 bulletDir;
};

enum GameState
{
	GAME_TITLE,
	GAME_ACTIVE,
	GAME_OVER
};

struct GameData
{
	Player player;
	Background background;

	std::vector<Meteor> meteors;
	std::vector<Bullet> bullets;

	GameState state;  // Add this to track game state
	float restartTimer; // Timer for restart delay

	int score;
	int lives;
	float invincibilityTimer;
	bool isInvincible;
};
#pragma endregion


#pragma region Function Decleration

void InitGame(GameData*);
void UpdateGame(GameData*, float);
void RenderGame(GameData*);
void EndGame(GameData*);

#pragma endregion


int main(void)
{

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	//SetConfigFlags(FLAG_FULLSCREEN_MODE);
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "raylib [core] example - basic window");
	InitAudioDevice();

#pragma region imgui
	rlImGuiSetup(true);

	//you can use whatever imgui theme you like!
	//ImGui::StyleColorsDark();
	//imguiThemes::yellow();
	//imguiThemes::gray();
	imguiThemes::green();
	//imguiThemes::red();
	//imguiThemes::embraceTheDarkness();


	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.FontGlobalScale = 2;

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		//style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 0.5f;
		//style.Colors[ImGuiCol_DockingEmptyBg].w = 0.f;
	}

#pragma endregion

	GameData defaultData{};

	InitGame(&defaultData);

	while (!WindowShouldClose())
	{

		float deltaTime = GetFrameTime();

		UpdateGame(&defaultData, deltaTime);

		if (IsWindowResized())
		{
			defaultData.player.playerPos = {
				GetScreenWidth() / 2.0f - defaultData.player.playerWidth / 2.f,
				GetScreenHeight() / 2.0f - defaultData.player.playerHeight / 2.f
			};
		}

		BeginDrawing();
		ClearBackground(RAYWHITE);


#pragma region imgui
		rlImGuiBegin();

		ImGui::PushStyleColor(ImGuiCol_WindowBg, {});
		ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, {});
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		ImGui::PopStyleColor(2);
#pragma endregion

		RenderGame(&defaultData);

#pragma region imgui
		rlImGuiEnd();

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
#pragma endregion

		EndDrawing();
	}

#pragma region imgui
	rlImGuiShutdown();
#pragma endregion

	EndGame(&defaultData);


	CloseWindow();

	return 0;
}


void InitGame(GameData* defaultData)
{


	sounds[LASER_SHOT] = LoadSound(RESOURCES_PATH "Sounds/sfx_laser1.ogg");

	// Reset game state
	defaultData->state = GAME_TITLE;
	defaultData->restartTimer = 0.0f;

	// Clear any existing meteors and bullets
	defaultData->meteors.clear();
	defaultData->bullets.clear();

#pragma region Player Initialization
	defaultData->player.playerTexture = LoadTexture(RESOURCES_PATH "playerShip3_red.png");
	defaultData->player.playerPos = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f };
	defaultData->player.playerHeight = defaultData->player.playerTexture.height * 0.5f;
	defaultData->player.playerWidth = defaultData->player.playerTexture.width * 0.5f;
	defaultData->player.playerSpeed = 500;
	defaultData->player.playerSource = {
		0,
		0,
		(float)defaultData->player.playerTexture.width,
		(float)defaultData->player.playerTexture.height
	};
	defaultData->player.playerOrigin = { defaultData->player.playerWidth / 2.f, defaultData->player.playerHeight / 2.f };
	defaultData->player.playerRotation = 0.0f;
#pragma endregion

#pragma region Background Initialization

	// Keep the background initialization as is
	if (defaultData->background.bgTexture.id == 0) {
		defaultData->background.bgTexture = LoadTexture(RESOURCES_PATH "Backgrounds/darkPurple.png");
	}
	//defaultData->background.bgTexture = LoadTexture(RESOURCES_PATH "Backgrounds/darkPurple.png");
	defaultData->background.bgPos = { 0,0 };
	defaultData->background.bgScale = 2;
#pragma endregion

#pragma region Meteor Initialization

	// Only load textures if they're not already loaded
	if (meteorTexture[TINY].id == 0) {
		meteorTexture[TINY] = LoadTexture(RESOURCES_PATH "Meteors/meteorBrown_small.png");
		meteorTexture[MID] = LoadTexture(RESOURCES_PATH "Meteors/meteorBrown_med.png");
		meteorTexture[LARGE] = LoadTexture(RESOURCES_PATH "Meteors/meteorBrown_big.png");
	}
	/*meteorTexture[TINY] = LoadTexture(RESOURCES_PATH "Meteors/meteorBrown_small.png");
	meteorTexture[MID] = LoadTexture(RESOURCES_PATH "Meteors/meteorBrown_med.png");
	meteorTexture[LARGE] = LoadTexture(RESOURCES_PATH "Meteors/meteorBrown_big.png");*/


	if (defaultData->state != GAME_TITLE)
	{
		for (int i = 0; i < MAX_METEORS; i++)
		{
			Meteor m;
			m.type = GetRandomValue(TINY, LARGE);
			m.isActive = true;
			m.meteorPos = { (float)GetRandomValue(0, WINDOW_WIDTH), (float)GetRandomValue(0, WINDOW_HEIGHT) };
			// Ensure not too close to player
			while (Vector2Distance(m.meteorPos, defaultData->player.playerPos) < 100.0f) {
				m.meteorPos = { (float)GetRandomValue(0, WINDOW_WIDTH), (float)GetRandomValue(0, WINDOW_HEIGHT) };
			}

			m.meteorRot = (float)GetRandomValue(0, 360);
			m.meteorSpeed = { (float)GetRandomValue(100,200), (float)GetRandomValue(100,200) };
			defaultData->meteors.push_back(m);
		}
	}
#pragma endregion



	// Only load bullet textures if they're not already loaded
	if (bulletTexture[B_TINY].id == 0) {
		bulletTexture[B_TINY] = LoadTexture(RESOURCES_PATH "laserBlue02.png");
	}
	/*bulletTexture[B_TINY] = LoadTexture(RESOURCES_PATH "laserBlue02.png");*/

	defaultData->lives = 3;
	defaultData->isInvincible = false;
	defaultData->invincibilityTimer = 0.0f;
	defaultData->score = 0;
}

void UpdateGame(GameData* defaultData, float deltaTime)
{
	// Title screen logic
	if (defaultData->state == GAME_TITLE)
	{
		// If space is pressed, transition to active game
		if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			defaultData->state = GAME_ACTIVE;

			// Generate meteors when transitioning to active game
			for (int i = 0; i < MAX_METEORS; i++)
			{
				Meteor m;
				m.type = GetRandomValue(TINY, LARGE);
				m.isActive = true;
				m.meteorPos = { (float)GetRandomValue(0, WINDOW_WIDTH), (float)GetRandomValue(0, WINDOW_HEIGHT) };
				// Ensure not too close to player
				while (Vector2Distance(m.meteorPos, defaultData->player.playerPos) < 100.0f) {
					m.meteorPos = { (float)GetRandomValue(0, WINDOW_WIDTH), (float)GetRandomValue(0, WINDOW_HEIGHT) };
				}

				m.meteorRot = (float)GetRandomValue(0, 360);
				m.meteorSpeed = { (float)GetRandomValue(100,200), (float)GetRandomValue(100,200) };
				defaultData->meteors.push_back(m);
			}

			// Reset player position
			defaultData->player.playerPos = {
				GetScreenWidth() / 2.0f - defaultData->player.playerWidth / 2.f,
				GetScreenHeight() / 2.0f - defaultData->player.playerHeight / 2.f
			};
		}

		// Background animation for title screen
		// You could add some floating meteors or other animations here
	}
	else if (defaultData->state == GAME_ACTIVE)
	{
		// Keep your existing GAME_ACTIVE logic
#pragma region Handle Mouse
		Vector2 mouseScreenPos = GetMousePosition();
		Vector2 mouseDir = Vector2Subtract(mouseScreenPos, defaultData->player.playerPos);

		if (mouseDir.x == 0 && mouseDir.y == 0) {
			mouseDir = { 1, 0 };
		}
		else
		{
			mouseDir = Vector2Normalize(mouseDir);
		}
#pragma endregion

#pragma region Collison
		auto bulletIt = defaultData->bullets.begin();
		while (bulletIt != defaultData->bullets.end()) {
			bool bulletHit = false;
			auto meteorIt = defaultData->meteors.begin();
			while (meteorIt != defaultData->meteors.end()) {
				// Calculate meteor's bounding rectangle with position as center
				float meteorWidth = (float)meteorTexture[meteorIt->type].width;
				float meteorHeight = (float)meteorTexture[meteorIt->type].height;
				Rectangle meteorRect = {
					meteorIt->meteorPos.x - meteorWidth / 2.0f,
					meteorIt->meteorPos.y - meteorHeight / 2.0f,
					meteorWidth,
					meteorHeight
				};

				float meteorRadius = meteorWidth / 2.0f * 0.8f;
				if (CheckCollisionPointCircle(bulletIt->bulletPos, meteorIt->meteorPos, meteorRadius)) {
					// Remove meteor and bullet
					switch (meteorIt->type)
					{
					case TINY: defaultData->score += 50;
						break;
					case MID: defaultData->score += 25;
						break;
					case LARGE: defaultData->score += 10;
						break;
					}
					meteorIt = defaultData->meteors.erase(meteorIt);
					bulletIt = defaultData->bullets.erase(bulletIt);
					bulletHit = true;
					break; // Bullet can only hit one meteor
				}
				else {
					++meteorIt;
				}
			}
			if (!bulletHit) {
				++bulletIt;
			}
		}
#pragma endregion

#pragma region Bullets
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			Bullet b{};

			b.bulletPos = {
				defaultData->player.playerPos.x + defaultData->player.playerWidth / 2.f,
				defaultData->player.playerPos.y + defaultData->player.playerHeight / 2.f
			};
			b.bulletDir = mouseDir;

			defaultData->bullets.push_back(b);

			PlaySound(sounds[LASER_SHOT]);
		}

		// Update bullet positions
		for (Bullet& b : defaultData->bullets)
		{
			float bulletSpeed = 800.0f;
			b.bulletPos.x += b.bulletDir.x * bulletSpeed * deltaTime;
			b.bulletPos.y += b.bulletDir.y * bulletSpeed * deltaTime;
		}

		// Remove bullets that are off-screen
		auto it = defaultData->bullets.begin();
		while (it != defaultData->bullets.end())
		{
			if (it->bulletPos.x < -bulletTexture[B_TINY].width ||
				it->bulletPos.x > GetScreenWidth() + bulletTexture[B_TINY].width ||
				it->bulletPos.y < -bulletTexture[B_TINY].height ||
				it->bulletPos.y > GetScreenHeight() + bulletTexture[B_TINY].height)
			{
				it = defaultData->bullets.erase(it);
			}
			else
			{
				++it;
			}
		}
#pragma endregion

#pragma region Player Movement
		Vector2 movement = {};

		if (IsKeyDown(KEY_A))
		{
			movement.x -= 1;
		}
		if (IsKeyDown(KEY_D))
		{
			movement.x += 1;
		}
		if (IsKeyDown(KEY_W))
		{
			movement.y -= 1;
		}
		if (IsKeyDown(KEY_S))
		{
			movement.y += 1;
		}

		if (movement.x != 0 || movement.y != 0)
		{
			movement = Vector2Normalize(movement);
			movement.x *= deltaTime * defaultData->player.playerSpeed;
			movement.y *= deltaTime * defaultData->player.playerSpeed;

			defaultData->player.playerPos.x += movement.x;
			defaultData->player.playerPos.y += movement.y;
		}

		// Apply screen wrapping for player
		int screenWidth = GetScreenWidth();
		int screenHeight = GetScreenHeight();

		if (defaultData->player.playerPos.x + defaultData->player.playerWidth < 0)
			defaultData->player.playerPos.x = screenWidth;
		else if (defaultData->player.playerPos.x > screenWidth)
			defaultData->player.playerPos.x = -defaultData->player.playerWidth;

		if (defaultData->player.playerPos.y + defaultData->player.playerHeight < 0)
			defaultData->player.playerPos.y = screenHeight;
		else if (defaultData->player.playerPos.y > screenHeight)
			defaultData->player.playerPos.y = -defaultData->player.playerHeight;

		float playerRotation = std::atan2(mouseDir.y, mouseDir.x);
		defaultData->player.playerRotation = playerRotation * (180.0f / 3.1415926535f);

		defaultData->player.playerDes = {
			defaultData->player.playerPos.x + defaultData->player.playerWidth / 2.f,
			defaultData->player.playerPos.y + defaultData->player.playerHeight / 2.f,
			(float)defaultData->player.playerWidth,
			(float)defaultData->player.playerHeight
		};
#pragma endregion

#pragma region Invincibility
		if (defaultData->isInvincible) {
			defaultData->invincibilityTimer -= deltaTime;
			if (defaultData->invincibilityTimer <= 0.0f) {
				defaultData->isInvincible = false;
			}
		}
#pragma endregion


#pragma region Update meteors
		for (auto& m : defaultData->meteors)
		{
			if (m.isActive)
			{
				m.meteorPos.x += m.meteorSpeed.x * deltaTime * cosf(m.meteorRot * DEG2RAD);
				m.meteorPos.y += m.meteorSpeed.y * deltaTime * sinf(m.meteorRot * DEG2RAD);

				// Screen wrapping for meteors
				float meteorWidth = (float)meteorTexture[m.type].width;
				float meteorHeight = (float)meteorTexture[m.type].height;
				float mleft = m.meteorPos.x - meteorWidth / 2.f;
				float mright = m.meteorPos.x + meteorWidth / 2.f;
				float mtop = m.meteorPos.y - meteorHeight / 2.f;
				float mbottom = m.meteorPos.y + meteorHeight / 2.f;
				if (mright < 0)
					m.meteorPos.x = screenWidth + meteorWidth / 2.f;
				else if (mleft > screenWidth)
					m.meteorPos.x = -meteorWidth / 2.f;
				if (mbottom < 0)
					m.meteorPos.y = screenHeight + meteorHeight / 2.f;
				else if (mtop > screenHeight)
					m.meteorPos.y = -meteorHeight / 2.f;

				// Collision with player
				Rectangle meteorRect = {
					m.meteorPos.x - meteorWidth / 2.0f,
					m.meteorPos.y - meteorHeight / 2.0f,
					meteorWidth,
					meteorHeight
				};



				//if (CheckCollisionRecs(meteorRect, { defaultData->player.playerPos.x + 12, defaultData->player.playerPos.y, (float)defaultData->player.playerWidth - 25, (float)defaultData->player.playerHeight }))
				//{
				//	// Set game state to game over
				//	defaultData->state = GAME_OVER;
				//	defaultData->restartTimer = 3.0f; // 3 second delay before restart
				//	break;
				//}

				// Collision with player - using circle for meteor and rectangle for player
				float meteorRadius = meteorWidth / 2.0f * 0.8f; // Using 80% of half-width for a tighter circle

				if (CheckCollisionCircleRec(
					m.meteorPos,                                // Meteor center position
					meteorRadius,                               // Meteor radius (slightly reduced for better gameplay)
					{                                           // Player hitbox rectangle
						defaultData->player.playerPos.x + 12,   // X offset to narrow the hitbox
						defaultData->player.playerPos.y,        // Y position
						(float)defaultData->player.playerWidth - 25, // Narrower width
						(float)defaultData->player.playerHeight      // Full height
					})) {

					if (!defaultData->isInvincible) {
						defaultData->lives--;

						if (defaultData->lives > 0) {
							// Player still has lives, make invincible and respawn
							defaultData->isInvincible = true;
							defaultData->invincibilityTimer = 3.0f; // 3 seconds of invincibility

							// Reset player position to center of screen
							defaultData->player.playerPos = {
								GetScreenWidth() / 2.0f - defaultData->player.playerWidth / 2.f,
								GetScreenHeight() / 2.0f - defaultData->player.playerHeight / 2.f
							};
						}
						else {
							// No lives left, game over
							defaultData->state = GAME_OVER;
							defaultData->restartTimer = 3.0f;
						}
						break;
					}

				}
			}
		}
#pragma endregion 
	}
	else if (defaultData->state == GAME_OVER)
	{
		// Game over state logic
		defaultData->restartTimer -= deltaTime;

		// Reset game after timer expires or when space is pressed
		if (defaultData->restartTimer <= 0.0f || IsKeyPressed(KEY_SPACE))
		{
			InitGame(defaultData); // Reset the game
		}
	}
}


// Finally, modify the RenderGame function to show the title screen
void RenderGame(GameData* defaultData)
{
#pragma region Background Rendering
	// Calculate background tile size (using scaled texture size)
	float backgroundSize = defaultData->background.bgTexture.height * defaultData->background.bgScale;

	// Determine how many tiles to render based on screen size
	int tilesX = static_cast<int>(std::ceil(GetScreenWidth() / backgroundSize)) + 1;
	int tilesY = static_cast<int>(std::ceil(GetScreenHeight() / backgroundSize)) + 1;

	// Render background tiles
	for (int y = 0; y < tilesY; y++)
	{
		for (int x = 0; x < tilesX; x++)
		{
			Vector2 tilePos =
			{
				x * backgroundSize,
				y * backgroundSize
			};
			DrawTextureEx(defaultData->background.bgTexture, tilePos, 0, defaultData->background.bgScale, RED);
		}
	}
#pragma endregion

#pragma region Title Screen
	// Render different content based on game state
	if (defaultData->state == GAME_TITLE)
	{
		// Draw animated title screen
		const char* titleText = "SPACE SHOOTER";
		int titleFontSize = 80;
		int titleTextWidth = MeasureText(titleText, titleFontSize);

		// Calculate position for centered text
		int screenWidth = GetScreenWidth();
		int screenHeight = GetScreenHeight();

		// Create a pulsing effect for the title
		float pulse = 0.8f + 0.2f * sinf(GetTime() * 3.0f);
		Color titleColor = { 255, 255, 255, 255 };

		// Draw the title with a shadow effect
		DrawText(titleText, screenWidth / 2 - titleTextWidth / 2 + 4, screenHeight / 4 + 4, titleFontSize, { 0, 0, 0, 128 });
		DrawText(titleText, screenWidth / 2 - titleTextWidth / 2, screenHeight / 4, titleFontSize, titleColor);

		// Draw subtitle
		const char* subtitleText = "Survive the meteor storm!";
		int subtitleFontSize = 30;
		int subtitleTextWidth = MeasureText(subtitleText, subtitleFontSize);
		DrawText(subtitleText, screenWidth / 2 - subtitleTextWidth / 2, screenHeight / 4 + titleFontSize + 20, subtitleFontSize, { 180, 180, 220, 255 });

		// Draw player ship in the center screen
		Rectangle playerSource = {
			0, 0,
			(float)defaultData->player.playerTexture.width,
			(float)defaultData->player.playerTexture.height
		};

		Rectangle playerDest = {
			screenWidth / 2,
			screenHeight / 2 + 40,
			(float)defaultData->player.playerWidth * 2.0f * pulse, // Animated size
			(float)defaultData->player.playerHeight * 2.0f * pulse // Animated size
		};

		Vector2 playerOrigin = {
			defaultData->player.playerWidth,
			defaultData->player.playerHeight
		};

		DrawTexturePro(
			defaultData->player.playerTexture,
			playerSource,
			playerDest,
			playerOrigin,
			GetTime() * 30.0f, // Slowly rotating
			WHITE
		);

		// Instructions
		const char* startText = "Press SPACE or CLICK to start";
		int startFontSize = 35;
		int startTextWidth = MeasureText(startText, startFontSize);

		// Blinking effect for start text
		if ((int)(GetTime() * 2) % 2 == 0) {
			DrawText(startText, screenWidth / 2 - startTextWidth / 2, screenHeight * 3 / 4, startFontSize, YELLOW);
		}

		// Draw controls info
		const char* controlsText = "Controls: WASD to move, MOUSE to aim, LEFT CLICK to shoot";
		int controlsFontSize = 20;
		int controlsTextWidth = MeasureText(controlsText, controlsFontSize);
		DrawText(controlsText, screenWidth / 2 - controlsTextWidth / 2, screenHeight - 50, controlsFontSize, LIGHTGRAY);
	}
#pragma endregion

	else
	{
		// In active game or game over state, render game elements
#pragma region Draw meteors
		for (const auto& m : defaultData->meteors)
		{
			if (m.isActive)
			{
				Rectangle source = { 0, 0, (float)meteorTexture[m.type].width, (float)meteorTexture[m.type].height };
				Rectangle dest = { m.meteorPos.x, m.meteorPos.y, (float)meteorTexture[m.type].width, (float)meteorTexture[m.type].height };
				Vector2 origin = { meteorTexture[m.type].width / 2.f, meteorTexture[m.type].height / 2.f };
				DrawTexturePro(meteorTexture[m.type], source, dest, origin, 0, WHITE);
				//DrawRectangleLines(m.meteorPos.x - meteorTexture[m.type].width / 2.f, m.meteorPos.y - meteorTexture[m.type].height / 2.f, (float)meteorTexture[m.type].width, (float)meteorTexture[m.type].height, WHITE);
				//DrawCircleLines( m.meteorPos.x, m.meteorPos.y, (float)meteorTexture[m.type].width / 2.f, WHITE);

				float meteorRadius = meteorTexture[m.type].width / 2.0f * 0.8f;
				DrawCircleLines(m.meteorPos.x, m.meteorPos.y, meteorRadius, RED);
			}
		}
#pragma endregion

#pragma region Draw Bullets
		// Draw bullets with proper rotation
		for (const Bullet& b : defaultData->bullets)
		{
			float bulletRotation = std::atan2(b.bulletDir.y, b.bulletDir.x);
			bulletRotation = bulletRotation * (180.0f / 3.1415926535f) + 90.f;

			DrawTexturePro(
				bulletTexture[B_TINY],
				{
					0,0,
					(float)bulletTexture[B_TINY].width,
					(float)bulletTexture[B_TINY].height
				},
				{
					b.bulletPos.x,
					b.bulletPos.y,
					(float)bulletTexture[B_TINY].width * 0.5f,
					(float)bulletTexture[B_TINY].height * 0.5f
				},
				{
					(float)bulletTexture[B_TINY].width * 0.5f / 2.f,
					(float)bulletTexture[B_TINY].height * 0.5f / 2.f
				},
				bulletRotation,
				WHITE
			);
		}
#pragma endregion

#pragma region Draw Player

		Color playerTint = WHITE;
		if (defaultData->isInvincible && (int)(GetTime() * 10) % 2 == 0) { // Flashing effect
			playerTint = BLANK; // Or GRAY, or ColorAlpha(WHITE, 0.5f)
		}


		DrawTexturePro(
			defaultData->player.playerTexture,
			defaultData->player.playerSource,
			defaultData->player.playerDes,
			defaultData->player.playerOrigin,
			defaultData->player.playerRotation + 90.f,
			playerTint
		);

		DrawRectangleLines(defaultData->player.playerPos.x + 12,
			defaultData->player.playerPos.y,
			(float)defaultData->player.playerWidth - 25,
			(float)defaultData->player.playerHeight, GREEN
		);
#pragma endregion

		// Draw game over screen if in game over state
		if (defaultData->state == GAME_OVER)
		{
			DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), ColorAlpha(BLACK, 0.7f));

			const char* gameOverText = "GAME OVER";
			int fontSize = 60;
			int textWidth = MeasureText(gameOverText, fontSize);
			DrawText(gameOverText, GetScreenWidth() / 2 - textWidth / 2, GetScreenHeight() / 2 - 60, fontSize, RED);

			const char* restartText = "Press SPACE to restart";
			int restartFontSize = 30;
			int restartTextWidth = MeasureText(restartText, restartFontSize);
			DrawText(restartText, GetScreenWidth() / 2 - restartTextWidth / 2, GetScreenHeight() / 2 + 20, restartFontSize, WHITE);
			
			char timerText[50];
			sprintf(timerText, "Restarting in %.1f", defaultData->restartTimer);
			int timerTextWidth = MeasureText(timerText, restartFontSize);
			DrawText(timerText, GetScreenWidth() / 2 - timerTextWidth / 2, GetScreenHeight() / 2 + 60, restartFontSize, WHITE);
		}

		char playerScore[50];
		sprintf(playerScore, "SCORE: %d", defaultData->score);
		DrawText(playerScore, 80, 80, 25, RAYWHITE);

		char playerLives[20];
		sprintf(playerLives, "LIVES: %d", defaultData->lives);
		DrawText(playerLives, 80, 50, 25, RAYWHITE);


	}
}

void EndGame(GameData* defaultData)
{
	UnloadSound(sounds[LASER_SHOT]);

	UnloadTexture(defaultData->player.playerTexture);
	UnloadTexture(bulletTexture[B_TINY]);

	UnloadTexture(meteorTexture[TINY]);
	UnloadTexture(meteorTexture[MID]);
	UnloadTexture(meteorTexture[LARGE]);

	UnloadTexture(defaultData->background.bgTexture);
}