// Fill out your copyright notice in the Description page of Project Settings.


#include "OnlineGameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"

UOnlineGameInstanceSubsystem::UOnlineGameInstanceSubsystem() :
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionComplete)),
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete))
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

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	if (ULocalPlayer* LP = GI->GetLocalPlayerByIndex(0))
	{
		if (APlayerController* PC = LP->PlayerController)
		{
			MainMenuWidget = CreateWidget(PC, MainMenuWidgetClass);
			MainMenuWidget->AddToViewport();
			FInputModeUIOnly UIOnlyInputMode;
			UIOnlyInputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
			PC->SetInputMode(UIOnlyInputMode);
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
	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		DestroySession();
	}

	NumOfPlayers = InNumOfPlayers;
	ServerName = InServerName;

	ShowLoadingScreen();

	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	CreateSessiomSettings = MakeShareable(new FOnlineSessionSettings());
	CreateSessiomSettings->NumPublicConnections = NumOfPlayers;
	CreateSessiomSettings->bShouldAdvertise = true;
	CreateSessiomSettings->bAllowJoinInProgress = true;
	CreateSessiomSettings->bIsLANMatch = bIsLan;
	CreateSessiomSettings->bUsesPresence = true;
	CreateSessiomSettings->bAllowJoinViaPresence = true;
	CreateSessiomSettings->BuildUniqueId = bIsLan ? 0 : 1;
	CreateSessiomSettings->bUseLobbiesIfAvailable = true;
	CreateSessiomSettings->bIsDedicated = false;

	CreateSessiomSettings->Set(SETTING_MAPNAME, ServerName.ToString(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *CreateSessiomSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}
}

bool UOnlineGameInstanceSubsystem::FindSessions(int32 MaxSessionResults, bool bIsLan)
{
	if (!SessionInterface.IsValid())
	{
		return false;
	}

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	SessionSearch = MakeShareable(new FOnlineSessionSearch);
	SessionSearch->MaxSearchResults = MaxSessionResults;
	SessionSearch->bIsLanQuery = bIsLan;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	bIsFindSessionCompleted = false;
	SearchSessionResults.Empty();

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

		GEngine->AddOnScreenDebugMessage(
			-1,
			5.f,
			FColor::Red,
			FString(TEXT("Find to Find Sesseion!"))
		);

		return false;
	}

	return true;
}

void UOnlineGameInstanceSubsystem::JoinServer(const FBlueprintSessionResult& SearchResult)
{
	JoinSession(SearchResult.OnlineResult);
}

void UOnlineGameInstanceSubsystem::JoinSession(const FOnlineSessionSearchResult& SearchResult)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	ShowLoadingScreen();

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	GEngine->AddOnScreenDebugMessage(
		-1,
		5.f,
		FColor::Red,
		FString(TEXT("JoinSession Start!"))
	);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SearchResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}
}

void UOnlineGameInstanceSubsystem::StartGame(const FString& InGameMapPath, bool bIsAbsolute)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FString PathToGame = FString::Printf(TEXT("%s?listen"), *InGameMapPath);
		if (World->ServerTravel(PathToGame, bIsAbsolute))
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				5.f,
				FColor::Red,
				FString(TEXT("Success to start game!"))
			);
		}
		else
		{
			const ETravelType TravelType = (bIsAbsolute ? TRAVEL_Absolute : TRAVEL_Relative);
			FWorldContext& WorldContext = GEngine->GetWorldContextFromWorldChecked(World);

			FURL TestURL(&WorldContext.LastURL, *PathToGame, TravelType);
			if (TestURL.IsLocalInternal())
			{
				// make sure the file exists if we are opening a local file
				if (!GEngine->MakeSureMapNameIsValid(TestURL.Map))
				{
					UE_LOG(LogLevel, Warning, TEXT("WARNING: The map '%s' does not exist."), *TestURL.Map);
				}
			}

			GEngine->SetClientTravel(World, *PathToGame, TravelType);

			GEngine->AddOnScreenDebugMessage(
				-1,
				5.f,
				FColor::Red,
				FString(TEXT("Failed to start game!, Try To Client Travel"))
			);
		}
	}
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
	if (SessionInterface)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	if (bSucceeded)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FString PathToLobby = FString::Printf(TEXT("%s?listen"), TEXT("/OnlineSessionSystem/Game/Level/Lobby"));
			World->ServerTravel(PathToLobby, true);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.f,
			FColor::Red,
			FString(TEXT("Failed to create session!"))
		);
	}
}

void UOnlineGameInstanceSubsystem::OnFindSessionComplete(bool bSucceeded)
{
	bIsFindSessionCompleted = true;
	bIsFindSessionSucceed = bSucceeded;

	if (SessionInterface)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (bSucceeded)
	{
		if (SessionSearch.IsValid())
		{
			for (auto& Result : SessionSearch->SearchResults)
			{
				FBlueprintSessionResult BPResult;
				BPResult.OnlineResult = Result;
				SearchSessionResults.Add(BPResult);
			}
		}

		GEngine->AddOnScreenDebugMessage(
			-1,
			5.f,
			FColor::Red,
			FString::Printf(TEXT("%d Session Found Complete!"), SearchSessionResults.Num())
		);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.f,
			FColor::Red,
			FString(TEXT("Find Sesseion Fail!"))
		);
	}
}

void UOnlineGameInstanceSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.f,
			FColor::Red,
			FString(TEXT("JoinSession Fail!"))
		);
		return;
	}
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}

		GEngine->AddOnScreenDebugMessage(
			-1,
			5.f,
			FColor::Red,
			FString(TEXT("JoinSession Success!"))
		);
	}
}

void UOnlineGameInstanceSubsystem::OnDestroySessionComplete(FName SessionName, bool bSucceeded)
{
	//@SrTODO currently not doing anything.
}
