// Fill out your copyright notice in the Description page of Project Settings.

#include "TriggerChangeHP.h"
#include "TPS2Character.h"
#include "Math/UnrealMathUtility.h"
#include "Containers/UnrealString.h"


ATriggerChangeHP::ATriggerChangeHP() 
{
	OnActorBeginOverlap.AddDynamic(this, &ATriggerChangeHP::OnOverlapBegin);
}

void ATriggerChangeHP::BeginPlay()
{
	Super::BeginPlay();
}

void ATriggerChangeHP::OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor) 
{

	if (OtherActor && (OtherActor != this)) 
	{
		
		ATPS2Character* Character = Cast<ATPS2Character>(OtherActor);

		if (Character == nullptr)
			return;

		if (OverlappedActor->ActorHasTag(FName(TEXT("TriggerChange"))))
		{
			Character->Health = Character->Health + ChangeValue;
			Character->Health = FMath::Clamp(Character->Health, 0, 100);
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, Color, FString::FromInt(Character->Health));
		}
		else if (OverlappedActor->ActorHasTag(FName(TEXT("TriggerDeath")))) 
		{
			Character->Health = 0;
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, Color, FString::FromInt(Character->Health));
		}
		
	}

}
