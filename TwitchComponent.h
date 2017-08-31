// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "Networking.h"
#include "Json.h"
#include "JsonUtilities.h"
#include <string>
#include <vector>
//#include "SceneController.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "TwitchComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class JAM_API UTwitchComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTwitchComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	void OnResponseReceivedUserID(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnResponseReceivedUsersOnline(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void SetUserDelay(float sec);
	bool IsCommand(FString name, FString msg);
	void ReceivedChatMessage(FString UserName, FString message);
	void EraseLastNickUse();
	void ErasePreviousMessages();
	void ParseMessage(FString msg);
	bool UserDelay(FString name);
	bool SendString(FString msg);
	FString returnedStringJsonValue;
	FString login;
	FString oath;
	FString userID;
	FHttpModule* Http;
	FSocket* ListenerSocket;
	FSocket* ConnectionSocket;
	float timer = 0;
	float delay;
	FString key;
	FString joinedChannel;
	float userCommandDelay;
	TArray<std::pair<FString, FString>> log;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SocketListener(float deltaTime);
	void Send(FString msg);
	void JoinChannel(FString channel);
	void SendLogin(FString name, FString okey);
	void SetMostPopularWindow(float sec);
	void UpdateViewrsList();
	FString GetMaxMessage();
	std::pair<FString, FString> GetLastIncome();
	FString GetUserLogoByName(FString name);
	TArray<FString> viewersNames;
	bool connect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		bool debug;
};
