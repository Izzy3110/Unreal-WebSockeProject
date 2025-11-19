// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "MyGameUserSettings.generated.h"

/**
 * 
 */
UCLASS()
class WEBSOCKEPROJECT_API UMyGameUserSettings : public UGameUserSettings
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static UMyGameUserSettings* GetBetterGameUserSettings();

	UPROPERTY(Config, BlueprintReadWrite, Category="WS|Connection")
	FString ServerHost;

	UPROPERTY(Config, BlueprintReadWrite, Category="WS|Connection")
	bool WSSecure;
	
	UPROPERTY(Config, BlueprintReadWrite, Category="Video|Display")
	int32 DisplayIndex;

	UPROPERTY(Config, BlueprintReadWrite, Category="Video|Display")
	FString DisplayResolution;

	UPROPERTY(Config, BlueprintReadWrite, Category="Video|Display")
	int32 WindowMode;
	
	
	
};
