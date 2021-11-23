// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "TriggerChangeHP.generated.h"

/**
 * 
 */
UCLASS()
class TPS2_API ATriggerChangeHP : public ATriggerBox
{
	GENERATED_BODY()
	
public:

	ATriggerChangeHP();

	UPROPERTY(EditAnywhere)
	int ChangeValue;

	UPROPERTY(EditAnywhere)
	FColor Color;

	UFUNCTION()
	void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
