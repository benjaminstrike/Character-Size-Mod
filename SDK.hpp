#pragma once
#include <cstdint>

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

struct CameraConfig
{
	union
	{
		MEMBER_VARIABLE(float, fov, 0x38); // DefaultGameFOV
		MEMBER_VARIABLE(float, weaponFov, 0x3C); // FirstPersonMeshFOV
	};
};

struct Entity
{
	union
	{
		MEMBER_VARIABLE(float, scale, 0x11C); // ScaleVector Y
	};
};

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

inline static auto BaseAddress = (uint64_t)GetBase();

inline CameraConfig* GetCameraConfig()
{
	uint64_t R6Configs = *(uint64_t*)(BaseAddress + 0x4793c58);
	CameraConfig*** cameraConfig = (CameraConfig***)(R6Configs + 0x28);
	return **cameraConfig;
}