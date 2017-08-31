// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Networking.h"
#include "Json.h"
#include "JsonUtilities.h"
#include <string>
#include <vector>
//#include "SceneController.h"
#include "Runtime/Online/HTTP/Public/Http.h"

typedef bool(*commandClassFunction)(FString name, FString msg);

/**
 * 
 */
class JAM_API TwitchClass
{
public:
	TwitchClass(commandClassFunction);
	TwitchClass();
	//TwitchClass(USceneController* obj);
	~TwitchClass();
	void SocketListener(float deltaTime);
	void Send(FString msg);
	void JoinChannel(FString channel);
	void SendLogin(FString name, FString okey);
	void SetMostPopularWindow(float sec);
	FString GetMaxMessage();
	std::pair<FString, FString> GetLastIncome();

private:
	void OnResponseReceivedUserID(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
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
	//USceneController* controller;
	TArray<std::pair<FString, FString>> log;

	commandClassFunction commandFunction;
	FString GetUserLogoByName(FString name);
};
