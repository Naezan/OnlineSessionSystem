// Fill out your copyright notice in the Description page of Project Settings.


#include "OnlineGameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"

UOnlineGameInstanceSubsystem::UOnlineGameInstanceSubsystem()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> MainMenuClassRef(TEXT("/Script/UMGEditor.WidgetBlueprint'/OnlineSessionSystem/UI/MainMenu/WBP_MainMenu.WBP_MainMenu_C'"));
	if (MainMenuClassRef.Class)
	{
		MainMenuWidgetClass = MainMenuClassRef.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> HostMenuClassRef(TEXT("/Script/UMGEditor.WidgetBlueprint'/OnlineSessionSystem/UI/MainMenu/WBP_HostMenu.WBP_HostMenu_C'"));
	if (HostMenuClassRef.Class)
	{
		HostMenuWidgetClass = HostMenuClassRef.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> LoadingScreenClassRef(TEXT("/Script/UMGEditor.WidgetBlueprint'/OnlineSessionSystem/UI/Game/WBP_LoadingScreen.WBP_LoadingScreen_C'"));
	if (LoadingScreenClassRef.Class)
	{
		LoadingScreenWidgetClass = LoadingScreenClassRef.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> ServerMenuClassRef(TEXT("/Script/UMGEditor.WidgetBlueprint'/OnlineSessionSystem/UI/MainMenu/WBP_ServerMenu.WBP_ServerMenu_C'"));
	if (ServerMenuClassRef.Class)
	{
		ServerMenuWidgetClass = ServerMenuClassRef.Class;
	}

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

void UOnlineGameInstanceSubsystem::ShowMainMenu()
{
	if (!MainMenuWidgetClass)
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (APlayerController* PC = GI->GetLocalPlayerByIndex(0)->PlayerController)
		{
			MainMenuWidget = CreateWidget(PC, MainMenuWidgetClass);
			MainMenuWidget->AddToViewport();
			PC->bShowMouseCursor = true;
		}
	}
}

void UOnlineGameInstanceSubsystem::ShowHostMenu()
{
	if (!HostMenuWidgetClass)
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (APlayerController* PC = GI->GetLocalPlayerByIndex(0)->PlayerController)
		{
			HostMenuWidget = CreateWidget(PC, HostMenuWidgetClass);
			HostMenuWidget->AddToViewport();
		}
	}
}

void UOnlineGameInstanceSubsystem::ShowServerMenu()
{
	if (!ServerMenuWidgetClass)
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (APlayerController* PC = GI->GetLocalPlayerByIndex(0)->PlayerController)
		{
			UUserWidget* ServerWidget = CreateWidget(PC, ServerMenuWidgetClass);
			ServerWidget->AddToViewport();
		}
	}
}

void UOnlineGameInstanceSubsystem::ShowLoadingScreen()
{
	if (!LoadingScreenWidgetClass)
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (APlayerController* PC = GI->GetLocalPlayerByIndex(0)->PlayerController)
		{
			LoadingScreenWidget = CreateWidget(PC, LoadingScreenWidgetClass);
			LoadingScreenWidget->AddToViewport();
		}
	}
}

void UOnlineGameInstanceSubsystem::LaunchLobby(int32 InNumOfPlayers, bool bIsLan, FText InServerName)
{
	NumOfPlayers = InNumOfPlayers;
	ServerName = InServerName;

	ShowLoadingScreen();

	FOnlineSessionSettings Settings;
	Settings.NumPublicConnections = NumOfPlayers;
	Settings.bShouldAdvertise = true;
	Settings.bAllowJoinInProgress = true;
	Settings.bIsLANMatch = bIsLan;
	Settings.bUsesPresence = true;
	Settings.bAllowJoinViaPresence = true;

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, Settings);
}

void UOnlineGameInstanceSubsystem::JoinServer(const FBlueprintSessionResult& SearchResult)
{
	JoinSession(SearchResult.OnlineResult);
}

void UOnlineGameInstanceSubsystem::JoinSession(const FOnlineSessionSearchResult& SearchResult)
{
	ShowLoadingScreen();

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SearchResult);
}

void UOnlineGameInstanceSubsystem::DestroySession()
{
	SessionInterface->DestroySession(NAME_GameSession);
}

void UOnlineGameInstanceSubsystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UOnlineGameInstanceSubsystem, NumOfPlayers);
	DOREPLIFETIME(UOnlineGameInstanceSubsystem, ServerName);
}

void UOnlineGameInstanceSubsystem::OnCreateSessionComplete(FName SessionName, bool bSucceeded)
{
	if (bSucceeded)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FString PathToLobby = FString::Printf(TEXT("%s?listen"), TEXT("/OnlineSessionSystem/Game/Level/Lobby"));
			World->ServerTravel(PathToLobby);
		}
	}
}

void UOnlineGameInstanceSubsystem::OnFindSessionComplete(bool bSucceeded)
{
	//@SrTODO currently not doing anything.
}

void UOnlineGameInstanceSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	//@SrTODO currently not doing anything.
}

void UOnlineGameInstanceSubsystem::OnDestroySessionComplete(FName SessionName, bool bSucceeded)
{
	//@SrTODO currently not doing anything.
}
