#include <windows.h>
#include <chrono>
#include <thread>
#include <string>
#include <iostream>

#include "config.hpp"
#include "SDK.hpp"
#include "hooks.hpp"
#include "commands.hpp"

void run()
{
	printf("Character Size Mod\n");
	printf("Mod page: https://r6mods.com/mods/character-size-mod\n");
	printf("Authors: benjaminstrike & R6Modder\n");
	printf("Game Version: White Noise (11580709)\n\n");
	printf("Notice:\nMost weapon scopes don't work with this, please only use default iron sights or regular acog.\n\n");
	printf("Usage:\n");
	printf("$ set-size small | normal | giant: changes player size\n");
	printf("$ exit: disables the mod\n");

	while (!config::exit) {
		printf("> ");
		std::string input;
		std::getline(std::cin, input);

		if (input.empty())
			continue;

		std::string command = input.substr(0, input.find(' '));
		std::string args = input.substr(input.find(' ') + 1);
		CommandHandler(command.c_str(), args.c_str());

		std::this_thread::sleep_for(std::chrono::milliseconds(0));
	}
}

DWORD WINAPI finalize(void* instance)
{
	config::reset();
	hooks::finalize();

	fclose(stdin);
	fclose(stdout);
	FreeConsole();
	FreeLibrary((HMODULE)instance);
	return TRUE;
}

DWORD WINAPI initialize(void* instance)
{
	AllocConsole();
	SetConsoleTitleA("Character Size Mod");
	freopen_s((FILE**)(stdin), "CONIN$", "r", stdin);
	freopen_s((FILE**)(stdout), "CONOUT$", "w", stdout);
	SetLayeredWindowAttributes(GetConsoleWindow(), 0, 220, 2);

	if (!hooks::initialize())
		return finalize(instance);

	run();

	return finalize(instance);
}

auto __stdcall DllMain(HMODULE ModuleInstance, uint32_t AttachReason)
{
	if (AttachReason != DLL_PROCESS_ATTACH)
		return false;

	DisableThreadLibraryCalls(ModuleInstance);
	std::thread{ &initialize, ModuleInstance }.detach();

	return true;
}