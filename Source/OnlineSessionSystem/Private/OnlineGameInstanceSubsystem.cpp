// Fill out your copyright notice in the Description page of Project Settings.


#include "OnlineGameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

UOnlineGameInstanceSubsystem::UOnlineGameInstanceSubsystem()
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UOnlineGameInstanceSubsystem::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UOnlineGameInstanceSubsystem::OnFindSessionComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UOnlineGameInstanceSubsystem::OnJoinSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UOnlineGameInstanceSubsystem::OnDestroySessionComplete);
		}
	}
}

void UOnlineGameInstanceSubsystem::CreateSessionServer()
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	//技记 积己窍扁
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bIsDedicated = false;
	SessionSettings.bIsLANMatch = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.NumPublicConnections = 5;

	SessionInterface->CreateSession(0, FName("Crete Session"), SessionSettings);
}

void UOnlineGameInstanceSubsystem::JoinSessionServer()
{
}

void UOnlineGameInstanceSubsystem::OnCreateSessionComplete(FName SessionName, bool bSucceeded)
{
	if (bSucceeded)
	{
		//GetWorld()->ServerTravel("/Game/ThirdPerson/Maps/ThirdPersonMap?listen");
	}
}

void UOnlineGameInstanceSubsystem::OnFindSessionComplete(bool bSucceeded)
{
	if (bSucceeded)
	{
		
	}
}

void UOnlineGameInstanceSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
}

void UOnlineGameInstanceSubsystem::OnDestroySessionComplete(FName SessionName, bool bSucceeded)
{
}
