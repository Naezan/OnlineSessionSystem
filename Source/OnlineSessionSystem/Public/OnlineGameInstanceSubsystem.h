// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include <Interfaces/OnlineSessionInterface.h>
#include "OnlineGameInstanceSubsystem.generated.h"

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
		void CreateSessionServer();
	UFUNCTION(BlueprintCallable)
		void JoinSessionServer();


protected:
	virtual void OnCreateSessionComplete(FName SessionName, bool bSucceeded);
	virtual void OnFindSessionComplete(bool bSucceeded);
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	virtual void OnDestroySessionComplete(FName SessionName, bool bSucceeded);

protected:
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	IOnlineSessionPtr SessionInterface;
};
