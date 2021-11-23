// Copyright Epic Games, Inc. All Rights Reserved.

#include "TPS2GameMode.h"
#include "TPS2Character.h"
#include "UObject/ConstructorHelpers.h"
//#include "TPS2Character.generated.h"

ATPS2GameMode::ATPS2GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ATPS2GameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
