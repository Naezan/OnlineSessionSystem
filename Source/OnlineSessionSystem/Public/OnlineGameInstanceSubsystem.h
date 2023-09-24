// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include <Interfaces/OnlineSessionInterface.h>
#include <FindSessionsCallbackProxy.h>
#include "OnlineGameInstanceSubsystem.generated.h"

class UUserWidget;

/**
 *
 */
UCLASS()
class ONLINESESSIONSYSTEM_API UOnlineGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UOnlineGameInstanceSubsystem();

	UFUNCTION(BlueprintCallable)
		void ShowMainMenu();

	UFUNCTION(BlueprintCallable)
		void ShowHostMenu();

	UFUNCTION(BlueprintCallable)
		void ShowServerMenu();

	UFUNCTION(BlueprintCallable)
		void ShowLoadingScreen();

	UFUNCTION(BlueprintCallable)
		void LaunchLobby(int32 InNumOfPlayers, bool bIsLan, FText InServerName);

	UFUNCTION(BlueprintCallable)
		bool FindSessions(int32 MaxSessionResults, bool bIsLan);

	UFUNCTION(BlueprintCallable)
		void JoinServer(const FBlueprintSessionResult& SearchResult);

	void JoinSession(const FOnlineSessionSearchResult& SearchResult);

	UFUNCTION(BlueprintCallable)
		void StartGame(const FString& InGameMapPath, bool bIsAbsolute);

	UFUNCTION(BlueprintCallable)
		void DestroySession();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnCreateSessionComplete(FName SessionName, bool bSucceeded);
	virtual void OnFindSessionComplete(bool bSucceeded);
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	virtual void OnDestroySessionComplete(FName SessionName, bool bSucceeded);

protected:
	UPROPERTY()
		TSubclassOf<UUserWidget> MainMenuWidgetClass;
	UPROPERTY()
		TSubclassOf<UUserWidget> HostMenuWidgetClass;
	UPROPERTY()
		TSubclassOf<UUserWidget> LoadingScreenWidgetClass;
	UPROPERTY()
		TSubclassOf<UUserWidget> ServerMenuWidgetClass;

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> CreateSessiomSettings;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	UPROPERTY(BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"))
		TArray<FBlueprintSessionResult> SearchSessionResults;

	UPROPERTY(BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"))
		uint8 bIsFindSessionCompleted : 1;
	UPROPERTY(BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"))
		uint8 bIsFindSessionSucceed : 1;

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	TObjectPtr<UUserWidget> MainMenuWidget;
	TObjectPtr<UUserWidget> HostMenuWidget;
	TObjectPtr<UUserWidget> LoadingScreenWidget;

	UPROPERTY(Replicated, BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"))
		int32 NumOfPlayers;

	UPROPERTY(Replicated, BlueprintReadWrite, Meta = (AllowPrivateAccess = "true"))
		FText ServerName;
};
