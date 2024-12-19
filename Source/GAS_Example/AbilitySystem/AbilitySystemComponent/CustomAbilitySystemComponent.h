// Copyright 2021 Joseph "Narxim" Thigpen.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GAS_Example/AbilitySystem/Data/AbilitySystemData.h"
#include "CustomAbilitySystemComponent.generated.h"

UCLASS()
class GAS_EXAMPLE_API UCustomAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	UCustomAbilitySystemComponent();

	// Called to initialize an Ability System Component with the supplied data. (Can be found in "AbilitySystemData.h")
	// Call this on the Server and Client to properly init references / values.
	UFUNCTION(BlueprintCallable)
	void InitializeAbilitySystemData(const FAbilitySystemInitializationData& InitializationData, AActor* InOwningActor, AActor* InAvatarActor);
	
	// Wrapper for the "GetOrCreateAttributeSubobject" function. Returns the specified Attribute Set / creates one if it isn't found.
	const UAttributeSet* GetOrCreateAttributeSet(const TSubclassOf<UAttributeSet>& InAttributeSet);

	/**
	 * Update all abilities and effects to reach the newly defined level.
	 * @param NewLevel New Level as Float. Will be floored to only Complete level will be taken in account
	 */
	virtual void LevelUpEffectsAndAbilities(const float NewLevel);

protected:
	
	bool AbilitySystemDataInitialized = false;
};