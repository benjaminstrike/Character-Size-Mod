#include <windows.h>
#include <chrono>
#include <thread>
#include <string>
#include <iostream>

#include <xor.hpp>
#include <MinHook/MinHook.h>

#define STR_MERGE_IMPL(a, b) a##b
#define STR_MERGE(a, b) STR_MERGE_IMPL(a, b)
#define MAKE_PAD(size) STR_MERGE(_pad, __COUNTER__)[size]
#define MEMBER_VARIABLE(type, name, offset) \
    struct { \
        unsigned char MAKE_PAD(offset > 0 ? offset : 1); \
        type name; \
    }

#define MEMBER_VAR(Name, Type, Offset)                                                       \
    Type Name( ) const							                                                  \
	{																						      \
		return *reinterpret_cast<Type*>( reinterpret_cast<std::uintptr_t>( this ) + Offset );     \
	}																						      \
	Type & Name( )       																		  \
	{																						      \
		return *reinterpret_cast<Type*>( reinterpret_cast<std::uintptr_t>( this ) + Offset );     \
	}

#define GetBase() (*(char**)(__readgsqword(0x60) + 0x10))
static auto BaseAddress = (uint64_t)GetBase();

namespace config
{
	bool exit = false;

	float scale = 1.0f;
	float fov = 1.570796371; // dynamic, so should ideally be different
	float weaponFov = 0.8726646304;
	float speed = 1.f;

	void reset()
	{
		scale = 1.f;
		fov = 1.570796371;
		weaponFov = 0.8726646304;
		speed = 1.f;
	}
};

struct CameraConfig
{
	union
	{
		MEMBER_VARIABLE(float, fov, 0x38);
		MEMBER_VARIABLE(float, weaponFov, 0x3C);
	};
};

CameraConfig* GetCameraConfig()
{
	uint64_t R6Configs = *(uint64_t*)(BaseAddress + 0x4793c58);
	CameraConfig*** cameraConfig = (CameraConfig***)(R6Configs + 0x28);
	return **cameraConfig;
}

struct Entity
{
	union
	{
		MEMBER_VARIABLE(float, scale, 0x11C);
	};
};

void* oGetEntityScale = nullptr;
float hkGetEntityScale(Entity* entity)
{
	CameraConfig* camera = GetCameraConfig();
	camera->fov = config::fov;
	//camera->weaponFov = config::weaponFov;
	return entity->scale = config::scale;
}

struct Pawn
{
	union
	{
		MEMBER_VARIABLE(float, timescale, 0x58);
	};
};

struct GroundNavContext
{
	union
	{
		MEMBER_VARIABLE(Pawn*, pawn, 0x8);
	};
};

typedef uint64_t(*__fastcall tHandleGroundNavContext)(GroundNavContext* groundNavContext);
tHandleGroundNavContext oHandleGroundNavContext = nullptr;
uint64_t hkHandleGroundNavContext(GroundNavContext* groundNavContext)
{
	groundNavContext->pawn->timescale = config::speed;
	return oHandleGroundNavContext(groundNavContext);
}

namespace hooks
{
	bool initialize()
	{
		void* terminateprocess = GetProcAddress(GetModuleHandleA(xorstr("kernel32.dll")), xorstr("TerminateProcess"));
		DWORD oldProtect = 0;
		VirtualProtect((void*)terminateprocess, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
		*(uint8_t*)terminateprocess = 0xC3;
		VirtualProtect((void*)terminateprocess, 1, oldProtect, &oldProtect);

		if (MH_Initialize() != MH_STATUS::MH_OK)
			return false;

		if (MH_CreateHook((LPVOID)(BaseAddress + 0x5983f0), hkGetEntityScale, &oGetEntityScale) != MH_STATUS::MH_OK)
			return false;

		if (MH_CreateHook((LPVOID)(BaseAddress + 0x17433f0), hkHandleGroundNavContext, (void**)&oHandleGroundNavContext) != MH_STATUS::MH_OK)
			return false;

		if (MH_EnableHook(MH_ALL_HOOKS) != MH_STATUS::MH_OK)
			return false;

		return true;
	}

	bool finalize()
	{
		MH_DisableHook(MH_ALL_HOOKS);
		return MH_Uninitialize() == MH_STATUS::MH_OK;
	}
};

void ScaleCommand(const char* preset)
{
	if (strcmp(preset, xorstr("normal")) == 0) {
		config::reset();
		return;
	}

	if (strcmp(preset, xorstr("small")) == 0) {
		config::scale = 0.3f;
		config::speed = 0.6;
		config::fov = 1.2;
		return;
	}

	if (strcmp(preset, xorstr("giant")) == 0) {
		config::scale = 1.5f;
		config::speed = 1.1;
		config::fov = 1.65;
		return;
	}

	printf(xorstr("Invalid arguments, valid arguments: small | normal | giant\n"));
}

void CommandHandler(const char* command, const char* args)
{
	if (strcmp(command, xorstr("exit")) == 0) {
		config::exit = true;
	}
	else if (strcmp(command, xorstr("reset")) == 0) {
		config::reset();
	}
	else if (strcmp(command, xorstr("set-size")) == 0) {
		ScaleCommand(args);
	}
	else if (strcmp(command, xorstr("customscale")) == 0) {
		config::scale = atof(args);
	}
	else if (strcmp(command, xorstr("gun")) == 0) {
		try {
			struct tagvaluelist {
				uint64_t a = 0x7FF633F96838;
				uint64_t b = 0;
				uint64_t c = 0xC000000000000000;
				uint64_t d = 0x29B790314C0;
				uint64_t e = 0;
			};

			typedef uint64_t(*__fastcall tFuncType)(uint64_t a1, uint64_t a2, tagvaluelist* a3);
			tFuncType func = (tFuncType)(BaseAddress + 0x1497390);
			tagvaluelist t;
			printf("func: %p\n", func(0x29BF6D4DA90, 0x29B0380DD20, &t));
		}
		catch (...) {
			printf("except\n");
		}
	}
	else {
		printf(xorstr("Invalid command, valid commands: set-size | exit\n"));
	}
}

void run()
{
	printf(xorstr("Character Size Mod\n"));
	printf(xorstr("Mod page: https://r6mods.com/mods/view/character-size-mod\n"));
	printf(xorstr("Authors: benjaminstrike & R6Modder\n"));
	printf(xorstr("Game Version: White Noise (11580709)\n\n"));
	printf(xorstr("Notice:\nMost weapon scopes don't work with this, please only use default iron sights or regular acog.\n\n"));
	printf(xorstr("Usage:\n"));
	printf(xorstr("$ set-size small | normal | giant: changes player size\n"));
	printf(xorstr("$ exit: disables the mod\n"));

	while (!config::exit) {
		printf(xorstr("> "));
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
	SetConsoleTitleA(xorstr("Character Size Mod"));
	freopen_s((FILE**)(stdin), xorstr("CONIN$"), "r", stdin);
	freopen_s((FILE**)(stdout), xorstr("CONOUT$"), "w", stdout);
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

#pragma optimize("", off)
struct CommandDefinition {
	const char* command; // e.g. "set-size"
	const char* description; // e.g. "small | normal | giant: changes player size"
};

struct ModInfo {
	const char* modName;
	const char* modDescription;
	const char* modAuthor;
	const char* modVersion;
	CommandDefinition* commands;
	uint16_t commandCount;
};

/**
 * Define a ModInfo structure with mod information and commands.
 *
 * @param modName       The name of the mod.
 * @param modDescription The description of the mod.
 * @param modAuthor     The author of the mod.
 * @param modVersion    The version of the mod.
 * @param ...			An optional list of commands, in the format {"command", "description" }
 * 
 * Example: REGISTER_MOD("example-mod", "This is an example mod", "benjaminstrike", "1.0.0", ...)
 */
#define REGISTER_MOD(modName, modDescription, modAuthor, modVersion, ...) \
    static struct CommandDefinition commandArray[] = { __VA_ARGS__ }; \
    static struct ModInfo modinfo = { \
        modName, \
        modDescription, \
        modAuthor, \
        modVersion, \
        commandArray, \
        sizeof(commandArray) / sizeof(commandArray[0]) \
    };                                                                                     \
                                                                                         \
    extern "C" __declspec(dllexport) uintptr_t ModLoader_GetModInfo()                     \
    {                                                                                    \
        return reinterpret_cast<uintptr_t>(&modinfo);                                     \
    }                                                                                    \
                                                                                         \
    extern "C" __declspec(dllexport) int ModLoader_Initialize()                           \
    {                                                                                    \
        return 0;                                                                        \
    }


#pragma optimize("", on)

REGISTER_MOD("MyMod", "Description", "Author", "1.0",
	{ "set-size", "small | normal | giant: changes player size" },
	{ "another-command", "Description of another command" });