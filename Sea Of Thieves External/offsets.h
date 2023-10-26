#pragma once

namespace offsets
{
	namespace UWorld
	{
		constexpr auto PersistentLevel = 0x30; // Size: 8, Type: struct ULevel*
		
		constexpr auto GameState = 0x58; // Size: 8, Type: struct AGameStateBase*
	
		constexpr auto Levels = 0x150; // Size: 16, Type: struct TArray<struct ULevel*>
	
		constexpr auto OwningGameInstance = 0x1c0; // Size: 8, Type: struct UGameInstance*
		
	
	}

	namespace AActor
	{
	
	

		constexpr auto Owner = 0x88; // Size: 8, Type: struct AActor*
	
		constexpr auto Children = 0x158; // Size: 16, Type: struct TArray<struct AActor*>
		constexpr auto RootComponent = 0x168; // Size: 8, Type: struct USceneComponent*
	
		
	}

	namespace USceneComponent
	{

		constexpr auto RelativeLocation = 0x108; // Size: 12, Type: struct FVector
		constexpr auto RelativeRotation = 0x114; // Size: 12, Type: struct FRotator
		constexpr auto RelativeScale3D = 0x120; // Size: 12, Type: struct FVector
		constexpr auto ComponentVelocity = 0x23c; // Size: 12, Type: struct FVector

	}





}
	