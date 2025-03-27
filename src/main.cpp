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
	Vector2 playerPos{};
	Vector2 playerDir{};
	int playerWidth;
	int playerHeight;
};

struct GameData
{
	Player player
	{ 
		{
			WINDOW_WIDTH / 2.0f,
			WINDOW_HEIGHT / 2.0f
		},
		{},
		100,
		100
	};
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



	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(RAYWHITE);


#pragma region imgui
		rlImGuiBegin();

		ImGui::PushStyleColor(ImGuiCol_WindowBg, {});
		ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, {});
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		ImGui::PopStyleColor(2);
#pragma endregion

		

#pragma region Player Movement
		if (IsKeyDown(KEY_A))
		{
			gameData.player.playerPos.x -= 1;
		}
		if (IsKeyDown(KEY_D))
		{
			gameData.player.playerPos.x += 1;
		}
		if (IsKeyDown(KEY_W))
		{
			gameData.player.playerPos.y -= 1;
		}
		if (IsKeyDown(KEY_S))
		{
			gameData.player.playerPos.y += 1;
		}

		
#pragma endregion


		//DrawCircle(gameData.player.playerPos.x, gameData.player.playerPos.y, 50.f, RED);
		DrawRectangle(gameData.player.playerPos.x, gameData.player.playerPos.y, gameData.player.playerWidth, gameData.player.playerHeight, GREEN);

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



	CloseWindow();

	return 0;
}