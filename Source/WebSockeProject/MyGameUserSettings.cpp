// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameUserSettings.h"
UMyGameUserSettings::UMyGameUserSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ServerHost = TEXT("192.168.137.51");
	WSSecure = false;

	DisplayIndex = 0;
	DisplayResolution = TEXT("1920x1080");
	
}

UMyGameUserSettings* UMyGameUserSettings::GetBetterGameUserSettings()
{
	return Cast<UMyGameUserSettings>(GetGameUserSettings());
}
