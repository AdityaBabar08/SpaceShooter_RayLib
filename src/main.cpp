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

struct Background
{
	Texture2D bgTexture;
	Vector2 bgPos;
	Rectangle bgRect;
	float bgScale;
};

struct Bullet
{
	Vector2 bulletPos;
	Vector2 bulletDir;
};

struct GameData
{
	Player player;
	Background background;

	std::vector<Meteor> meteors;
	std::vector<Bullet> bullets;
};


int main(void)
{

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	//SetConfigFlags(FLAG_FULLSCREEN_MODE);
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "raylib [core] example - basic window");


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

	GameData gameData{};

#pragma region Player Initialization
	gameData.player.playerTexture = LoadTexture(RESOURCES_PATH "playerShip3_red.png");
	gameData.player.playerPos = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f };
	gameData.player.playerHeight = gameData.player.playerTexture.height * 0.5f;
	gameData.player.playerWidth = gameData.player.playerTexture.width * 0.5f;
	gameData.player.playerSpeed = 500;
	gameData.player.playerSource = {
		0,
		0,
		(float)gameData.player.playerTexture.width,
		(float)gameData.player.playerTexture.height
	};
	gameData.player.playerOrigin = { gameData.player.playerWidth / 2.f, gameData.player.playerHeight / 2.f };
#pragma endregion

#pragma region Background Initialization
	gameData.background.bgTexture = LoadTexture(RESOURCES_PATH "Backgrounds/darkPurple.png");
	gameData.background.bgPos = { 0,0 };
	gameData.background.bgScale = 2;
#pragma endregion

	meteorTexture[TINY] = LoadTexture(RESOURCES_PATH "Meteors/meteorBrown_small.png");
	meteorTexture[MID] = LoadTexture(RESOURCES_PATH "Meteors/meteorBrown_med.png");
	meteorTexture[LARGE] = LoadTexture(RESOURCES_PATH "Meteors/meteorBrown_big.png");


	for (int i = 0; i < MAX_METEORS; i++)
	{
		Meteor m;
		m.type = GetRandomValue(TINY, LARGE);
		m.isActive = true;
		m.meteorPos = { (float)GetRandomValue(0, WINDOW_WIDTH), (float)GetRandomValue(0, WINDOW_HEIGHT) };
		// Ensure not too close to player
		while (Vector2Distance(m.meteorPos, gameData.player.playerPos) < 100.0f) {
			m.meteorPos = { (float)GetRandomValue(0, WINDOW_WIDTH), (float)GetRandomValue(0, WINDOW_HEIGHT) };
		}

		m.meteorRot = (float)GetRandomValue(0, 360);

		m.meteorSpeed = { (float)GetRandomValue(100,200), (float)GetRandomValue(100,200) };

		gameData.meteors.push_back(m);
	}



	Texture2D bulletTexture = LoadTexture(RESOURCES_PATH "laserBlue02.png");

	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(RAYWHITE);

		float deltaTime = GetFrameTime();

#pragma region imgui
		rlImGuiBegin();

		ImGui::PushStyleColor(ImGuiCol_WindowBg, {});
		ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, {});
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		ImGui::PopStyleColor(2);
#pragma endregion

		if (IsWindowResized())
		{
			gameData.player.playerPos = {
				GetScreenWidth() / 2.0f - gameData.player.playerWidth / 2.f,
				GetScreenHeight() / 2.0f - gameData.player.playerHeight / 2.f
			};
		}

#pragma region Handle Mouse
		Vector2 mouseScreenPos = GetMousePosition();
		Vector2 mouseDir = Vector2Subtract(mouseScreenPos, gameData.player.playerPos);

		if (mouseDir.x == 0 && mouseDir.y == 0) {
			mouseDir = { 1, 0 };
		}
		else
		{
			mouseDir = Vector2Normalize(mouseDir);
		}
#pragma endregion

#pragma region Background Rendering
		// Calculate background tile size (using scaled texture size)
		float backgroundSize = gameData.background.bgTexture.height * gameData.background.bgScale;

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
				DrawTextureEx(gameData.background.bgTexture, tilePos, 0, gameData.background.bgScale, RED);
			}
		}
#pragma endregion


		
#pragma region Draw meteors
		for (const auto& m : gameData.meteors)
		{
			if (m.isActive)
			{
				Rectangle source = { 0, 0, (float)meteorTexture[m.type].width, (float)meteorTexture[m.type].height };
				Rectangle dest = { m.meteorPos.x, m.meteorPos.y, (float)meteorTexture[m.type].width, (float)meteorTexture[m.type].height };
				Vector2 origin = { meteorTexture[m.type].width / 2.f, meteorTexture[m.type].height / 2.f };
				DrawTexturePro(meteorTexture[m.type], source, dest, origin, 0, WHITE);
			}
		}
#pragma endregion



#pragma region Collison

		auto bulletIt = gameData.bullets.begin();
		while (bulletIt != gameData.bullets.end()) {
			bool bulletHit = false;
			auto meteorIt = gameData.meteors.begin();
			while (meteorIt != gameData.meteors.end()) {
				// Calculate meteor's bounding rectangle with position as center
				float meteorWidth = (float)meteorTexture[meteorIt->type].width;
				float meteorHeight = (float)meteorTexture[meteorIt->type].height;
				Rectangle meteorRect = {
					meteorIt->meteorPos.x - meteorWidth / 2.0f,
					meteorIt->meteorPos.y - meteorHeight / 2.0f,
					meteorWidth,
					meteorHeight
				};

				if (CheckCollisionPointRec(bulletIt->bulletPos, meteorRect)) {
					// Remove meteor and bullet
					meteorIt = gameData.meteors.erase(meteorIt);
					bulletIt = gameData.bullets.erase(bulletIt);
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
				gameData.player.playerPos.x + gameData.player.playerWidth / 2.f,
				gameData.player.playerPos.y + gameData.player.playerHeight / 2.f
			};
			b.bulletDir = mouseDir;

			gameData.bullets.push_back(b);
		}

		// Update bullet positions
		for (Bullet& b : gameData.bullets)
		{
			float bulletSpeed = 800.0f;
			b.bulletPos.x += b.bulletDir.x * bulletSpeed * deltaTime;
			b.bulletPos.y += b.bulletDir.y * bulletSpeed * deltaTime;
		}

		// Draw bullets with proper rotation
		for (const Bullet& b : gameData.bullets)
		{
			float bulletRotation = std::atan2(b.bulletDir.y, b.bulletDir.x);
			bulletRotation = bulletRotation * (180.0f / 3.1415926535f) + 90.f;

			DrawTexturePro(
				bulletTexture,
				{
					0,0,
					(float)bulletTexture.width,
					(float)bulletTexture.height
				},
				{
					b.bulletPos.x,
					b.bulletPos.y,
					(float)bulletTexture.width * 0.5f,
					(float)bulletTexture.height * 0.5f
				},
				{
					(float)bulletTexture.width * 0.5f / 2.f,
					(float)bulletTexture.height * 0.5f / 2.f
				},
				bulletRotation,
				WHITE
			);
		}

		// Remove bullets that are off-screen
		auto it = gameData.bullets.begin();
		while (it != gameData.bullets.end())
		{
			if (it->bulletPos.x < -bulletTexture.width ||
				it->bulletPos.x > GetScreenWidth() + bulletTexture.width ||
				it->bulletPos.y < -bulletTexture.height ||
				it->bulletPos.y > GetScreenHeight() + bulletTexture.height)
			{
				it = gameData.bullets.erase(it);
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
			movement.x *= deltaTime * gameData.player.playerSpeed;
			movement.y *= deltaTime * gameData.player.playerSpeed;

			gameData.player.playerPos.x += movement.x;
			gameData.player.playerPos.y += movement.y;
		}

		// Apply screen wrapping for player
		int screenWidth = GetScreenWidth();
		int screenHeight = GetScreenHeight();

		if (gameData.player.playerPos.x + gameData.player.playerWidth < 0)
			gameData.player.playerPos.x = screenWidth;
		else if (gameData.player.playerPos.x > screenWidth)
			gameData.player.playerPos.x = -gameData.player.playerWidth;

		if (gameData.player.playerPos.y + gameData.player.playerHeight < 0)
			gameData.player.playerPos.y = screenHeight;
		else if (gameData.player.playerPos.y > screenHeight)
			gameData.player.playerPos.y = -gameData.player.playerHeight;

		float playerRotation = std::atan2(mouseDir.y, mouseDir.x);
		playerRotation = playerRotation * (180.0f / 3.1415926535f);

		gameData.player.playerDes = {
			gameData.player.playerPos.x + gameData.player.playerWidth / 2.f,
			gameData.player.playerPos.y + gameData.player.playerHeight / 2.f,
			(float)gameData.player.playerWidth,
			(float)gameData.player.playerHeight
		};

		DrawTexturePro(
			gameData.player.playerTexture,
			gameData.player.playerSource,
			gameData.player.playerDes,
			gameData.player.playerOrigin,
			playerRotation + 90.f,
			WHITE
		);
#pragma endregion

		
#pragma region Update meteors
		for (auto& m : gameData.meteors) 
		{
			if (m.isActive)
			{
				m.meteorPos.x += m.meteorSpeed.x * deltaTime * cosf(m.meteorRot * DEG2RAD);
				m.meteorPos.y += m.meteorSpeed.y * deltaTime * sinf(m.meteorRot * DEG2RAD);
				//m.rotation += m.rotationSpeed * deltaTime;

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
			}

		}
#pragma endregion





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

	UnloadTexture(gameData.player.playerTexture);
	UnloadTexture(bulletTexture);

	UnloadTexture(meteorTexture[TINY]);
	UnloadTexture(meteorTexture[MID]);
	UnloadTexture(meteorTexture[LARGE]);

	UnloadTexture(gameData.background.bgTexture);
	CloseWindow();

	return 0;
}