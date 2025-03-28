#define RAYMATH_IMPLEMENTATION
#include "raylib.h"
#include "raymath.h"
#include <iostream>

#pragma region imgui
#include "imgui.h"
#include "rlImGui.h"
#include "imguiThemes.h"
#pragma endregion


const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 450;

struct Player
{
	Vector2 playerPos;
	Vector2 playerDir;
	int playerWidth;
	int playerHeight;
	float playerSpeed;
	Texture2D playerTexture;
};

struct Background
{
	Texture2D bgTexture;
	Vector2 bgPos;
	Rectangle bgRect;
	float bgScale;
};

struct GameData
{
	Player player;
	Background background;
	
};


int main(void)
{

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
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

	GameData gameData;
#pragma region Player Initialization
	gameData.player;
	gameData.player.playerPos = { WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f };
	gameData.player.playerHeight = 100;
	gameData.player.playerWidth = 100;
	gameData.player.playerSpeed = 500;

#pragma endregion

#pragma region Camera
	Camera2D camera;
	camera.offset = { WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f };
	camera.target = { gameData.player.playerPos.x + 50, gameData.player.playerPos.y + 50 };
	camera.rotation = 0.f;
	camera.zoom = 1.0f;
#pragma endregion


#pragma region Background Initialization
	gameData.background.bgTexture = LoadTexture(RESOURCES_PATH "Backgrounds/darkPurple.png");
	gameData.background.bgPos = { 0,0 };
	gameData.background.bgScale = 2;
#pragma endregion


	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(RAYWHITE);
		BeginMode2D(camera);

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
			gameData.player.playerPos = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
		}



#pragma region Camera Following

		camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
		camera.target = { gameData.player.playerPos.x + 50, gameData.player.playerPos.y + 50 };

#pragma endregion


#pragma region Inf Background

		// Calculate background tile size (using scaled texture size)
		float backgroundSize = gameData.background.bgTexture.height * gameData.background.bgScale;


		// Calculate tile indices based on camera target
		int tileX = static_cast<int>(std::floor(camera.target.x / backgroundSize));
		int tileY = static_cast<int>(std::floor(camera.target.y / backgroundSize));

		// Determine how many tiles to render based on screen size
		int tilesX = static_cast<int>(std::ceil(GetScreenWidth() / backgroundSize)) + 2;
		int tilesY = static_cast<int>(std::ceil(GetScreenHeight() / backgroundSize)) + 2;

		// Render background tiles
		for (int y = -tilesY / 2; y <= tilesY / 2; y++) 
		{
			for (int x = -tilesX / 2; x <= tilesX / 2; x++) 
			{
				Vector2 tilePos = 
				{
					(tileX + x) * backgroundSize,
					(tileY + y) * backgroundSize
				};
				DrawTextureEx(gameData.background.bgTexture, tilePos, 0, gameData.background.bgScale, RED);
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
		DrawRectangle(gameData.player.playerPos.x, gameData.player.playerPos.y, gameData.player.playerWidth, gameData.player.playerHeight, GREEN);
#pragma endregion

#pragma region imgui
		rlImGuiEnd();

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
#pragma endregion
		EndMode2D();
		EndDrawing();
	}


#pragma region imgui
	rlImGuiShutdown();
#pragma endregion


	UnloadTexture(gameData.background.bgTexture);
	CloseWindow();

	return 0;
}