#pragma once
#include <windows.h>
#include <MinHook/MinHook.h>
#include "SDK.hpp"
#include "config.hpp"

inline void* oGetEntityScale = nullptr;
inline float hkGetEntityScale(Entity* entity)
{
	CameraConfig* camera = GetCameraConfig();
	camera->fov = config::fov;
	//camera->weaponFov = config::weaponFov;
	return entity->scale = config::scale;
}

typedef uint64_t(*__fastcall tHandleGroundNavContext)(GroundNavContext* groundNavContext);
inline tHandleGroundNavContext oHandleGroundNavContext = nullptr;
inline uint64_t hkHandleGroundNavContext(GroundNavContext* groundNavContext)
{
	groundNavContext->pawn->timescale = config::speed;
	return oHandleGroundNavContext(groundNavContext);
}

namespace hooks
{
	inline bool initialize()
	{
		void* terminateprocess = GetProcAddress(GetModuleHandleA("kernel32.dll"), "TerminateProcess");
		DWORD oldProtect = 0;
		VirtualProtect((void*)terminateprocess, 1, PAGE_EXECUTE_READWRITE, &oldProtect);
		*(uint8_t*)terminateprocess = 0xC3; // ret
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

	inline bool finalize()
	{
		MH_DisableHook(MH_ALL_HOOKS);
		return MH_Uninitialize() == MH_STATUS::MH_OK;
	}
}