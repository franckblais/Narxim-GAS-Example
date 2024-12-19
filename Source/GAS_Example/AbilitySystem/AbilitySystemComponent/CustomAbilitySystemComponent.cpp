// Copyright 2021 Joseph "Narxim" Thigpen.


#include "CustomAbilitySystemComponent.h"

#include "GAS_Example/AbilitySystem/AttributeSets/LevelAttributeSet.h"


UCustomAbilitySystemComponent::UCustomAbilitySystemComponent()
{
	// Sets the Ability System Component to use "Mixed" replication mode. This will replicate minimal Gameplay Effects to Simulated Proxies and full info to everyone else.
	ReplicationMode = EGameplayEffectReplicationMode::Mixed;

	// Explicitly set the Ability System Component to replicate.
	SetIsReplicatedByDefault(true);
}

const UAttributeSet* UCustomAbilitySystemComponent::GetOrCreateAttributeSet(const TSubclassOf<UAttributeSet>& InAttributeSet)
{
	return GetOrCreateAttributeSubobject(InAttributeSet);
}

void UCustomAbilitySystemComponent::LevelUpEffectsAndAbilities(const float NewLevel)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	const int32 Level = FMath::Floor(NewLevel < 0.f ? 0 : NewLevel);
	FGameplayEffectQuery Query{};

	// Note: Floats could be used, but need to create a new method.
	// Note 2: This method performance could be improved (Loop of Loops)
	SetActiveGameplayEffectLevelUsingQuery(Query, Level);

	for (FGameplayAbilitySpec& ActivatableAbility : GetActivatableAbilities())
	{
		if (!ActivatableAbility.Level == Level)
		{
			continue;
		}
		ActivatableAbility.Level = Level;
		MarkAbilitySpecDirty(ActivatableAbility, false);
	} 
}

void UCustomAbilitySystemComponent::InitializeAbilitySystemData(const FAbilitySystemInitializationData& InitializationData, AActor* InOwningActor, AActor* InAvatarActor)
{
	if (AbilitySystemDataInitialized)
	{
		return;
	}
	
	AbilitySystemDataInitialized = true;

	// Set the Owning Actor and Avatar Actor. (Used throughout the Gameplay Ability System to get references etc.)
	InitAbilityActorInfo(InOwningActor, InAvatarActor);

	// Apply the Gameplay Tag container as loose Gameplay Tags. (These are not replicated by default and should be applied on both server and client respectively.)
	if (!InitializationData.GameplayTags.IsEmpty())
	{
		AddLooseGameplayTags(InitializationData.GameplayTags);
	}
	
	// Check to see if we have authority. (Attribute Sets / Attribute Base Values / Gameplay Abilities / Gameplay Effects should only be added -or- set on authority and will be replicated to the client automatically.)
	if (!GetOwnerActor()->HasAuthority())
	{
		return;
	}
	
	// Grant Attribute Sets if the array isn't empty.
	if (!InitializationData.AttributeSets.IsEmpty())
	{
		for (const TSubclassOf<UAttributeSet> AttributeSetClass : InitializationData.AttributeSets)
		{
			GetOrCreateAttributeSet(AttributeSetClass);
		}
	}

	// Set base attribute values if the map isn't empty.
	if (!InitializationData.AttributeBaseValues.IsEmpty())
	{
		for (const TTuple<FGameplayAttribute, float>& AttributeBaseValue : InitializationData.AttributeBaseValues)
		{
			if (HasAttributeSetForAttribute(AttributeBaseValue.Key))
			{
				SetNumericAttributeBase(AttributeBaseValue.Key, AttributeBaseValue.Value);
			}
		}
	}

	// Grant Gameplay Abilities if the array isn't empty.
	if (!InitializationData.GameplayAbilities.IsEmpty())
	{
		for (const TSubclassOf<UGameplayAbility> GameplayAbility : InitializationData.GameplayAbilities)
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(GameplayAbility, 1, INDEX_NONE, this);
			
			GiveAbility(AbilitySpec);
		}
	}

	// Apply Gameplay Effects if the array isn't empty.
	if (!InitializationData.GameplayEffects.IsEmpty())
	{
		for (const TSubclassOf<UGameplayEffect>& GameplayEffect : InitializationData.GameplayEffects)
		{
			if (!IsValid(GameplayEffect))
			{
				continue;
			}
			
			FGameplayEffectContextHandle EffectContextHandle = MakeEffectContext();
			EffectContextHandle.AddSourceObject(this);

			if (FGameplayEffectSpecHandle GameplayEffectSpecHandle = MakeOutgoingSpec(GameplayEffect, 1, EffectContextHandle); GameplayEffectSpecHandle.IsValid())
			{
				ApplyGameplayEffectSpecToTarget(*GameplayEffectSpecHandle.Data.Get(), this);
			}
		}
	}

	LevelUpEffectsAndAbilities(GetNumericAttribute(ULevelAttributeSet::GetCurrentLevelAttribute()));
}
