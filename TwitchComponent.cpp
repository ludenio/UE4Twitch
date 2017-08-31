// Fill out your copyright notice in the Description page of Project Settings.

#include "Jam.h"
#include "TwitchComponent.h"


// Sets default values for this component's properties
UTwitchComponent::UTwitchComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	Http = &FHttpModule::Get();
	connect = false;
	viewersNames.Empty();
	// ...
}

struct MaxMessageStruct {
	float startTime;
	int counter;
	FString message;

	MaxMessageStruct(float t, int c, FString msg) {
		this->startTime = t;
		this->counter = c;
		this->message = msg;
	};
};

struct LastNickUseStruct {
	float exitTime;
	FString nick;

	LastNickUseStruct(float t, FString ni) {
		this->exitTime = t;
		this->nick = ni;
	};
};

TArray<MaxMessageStruct> messageList;
TArray<LastNickUseStruct> lastNick;

//DECLARE_DELEGATE_OneParam(RequestDelegate, FHttpRequestPtr , FHttpResponsePtr , bool );
//DECLARE_DELEGATE_<Num>Params(NameDelegate, FHttpRequestPtr , FHttpResponsePtr , bool )

FString UTwitchComponent::GetUserLogoByName(FString name) {
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "3");
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UTwitchComponent::OnResponseReceivedUserID);
	Request->SetURL("https://api.twitch.tv/kraken/users?login=" + name);
	Request->SetVerb("GET");
	Request->SetHeader("Client-ID", "56egycd86qrurldm0q4qp4ssc73cun");
	Request->SetHeader("Accept", "application/vnd.twitchtv.v5+json");
	Request->ProcessRequest();
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "*" + returnedStringJsonValue);
	return returnedStringJsonValue;
}

void UTwitchComponent::OnResponseReceivedUserID(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *(FString(Response->GetContentAsString())));
		TSharedPtr<FJsonObject> firstObject = (JsonObject->GetArrayField("users"))[0]->AsObject();
		FString ans = *(FString(firstObject->GetStringField("logo")));
		//firstObject->
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, ans);
		returnedStringJsonValue = ans;
	}
}

void UTwitchComponent::UpdateViewrsList() {
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "3");
	Request->OnProcessRequestComplete().BindUObject(this, &UTwitchComponent::OnResponseReceivedUsersOnline);
	Request->SetURL("https://tmi.twitch.tv/group/user/" + joinedChannel + "/chatters");
	Request->SetVerb("GET");
	Request->SetHeader("Client-ID", "56egycd86qrurldm0q4qp4ssc73cun");
	Request->SetHeader("Accept", "application/vnd.twitchtv.v5+json");
	Request->ProcessRequest();
}

void UTwitchComponent::OnResponseReceivedUsersOnline(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) {
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		viewersNames.Empty();
		//UE_LOG(LogTemp, Warning, TEXT("%s"), *(FString(Response->GetContentAsString())));
		//UE_LOG(LogTemp, Warning, TEXT("%s"), *(FString(JsonObject->GetObjectField("chatters")->Get)));
		TSharedPtr<FJsonObject> firstObject = JsonObject->GetObjectField("chatters");
		TArray < TSharedPtr < FJsonValue > > viewers = firstObject->GetArrayField("viewers");
		for (int i = 0; i < viewers.Num(); i++) {
			viewersNames.Add(FString(viewers[i]->AsString()));
		}
		 viewers = firstObject->GetArrayField("moderators");
		for (int i = 0; i < viewers.Num(); i++) {
			viewersNames.Add(FString(viewers[i]->AsString()));
		}
		viewers = firstObject->GetArrayField("staff");
		for (int i = 0; i < viewers.Num(); i++) {
			viewersNames.Add(FString(viewers[i]->AsString()));
		}
		viewers = firstObject->GetArrayField("admins");
		for (int i = 0; i < viewers.Num(); i++) {
			viewersNames.Add(FString(viewers[i]->AsString()));
		}
		viewers = firstObject->GetArrayField("global_mods");
		for (int i = 0; i < viewers.Num(); i++) {
			viewersNames.Add(FString(viewers[i]->AsString()));
		}
	//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::FromInt(viewersNames.Num()));
		//UE_LOG(LogTemp, Warning, TEXT("%s"), *(FString(viewers[0]->AsString())));
		//UE_LOG(LogTemp, Warning, TEXT("%s"), *(FString(firstObject->GetArrayField("users")[0]->AsObject));
		//FString ans = *(FString(firstObject->GetStringField("logo")));
		//firstObject->
		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, ans);
		//returnedStringJsonValue = ans;
	}
}

void UTwitchComponent::SocketListener(float deltaTime)
{
	timer += deltaTime;
	TArray<uint8> ReceivedData;
	uint32 Size;
	bool Received = false;
	while (ListenerSocket->HasPendingData(Size))
	{
		Received = true;
		ReceivedData.SetNumUninitialized(FMath::Min(Size, 65507u));

		int32 Read = 0;
		ListenerSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);
	}
	if (Received)
	{
		const std::string cstr(reinterpret_cast<const char*>(ReceivedData.GetData()), ReceivedData.Num());
		FString fs(cstr.c_str());

		ParseMessage(fs);
	}
}

void UTwitchComponent::ParseMessage(FString msg)
{
	TArray<FString> lines;
	msg.ParseIntoArrayLines(lines);
	for (FString fs : lines)
	{
		TArray<FString> parts;
		fs.ParseIntoArray(parts, TEXT(":"));
		TArray<FString> meta;
		parts[0].ParseIntoArrayWS(meta);
		if (parts.Num() >= 2)
		{
			if (meta[0] == TEXT("PING"))
			{
				SendString(TEXT("PONG :tmi.twitch.tv"));
			}
			else if (meta.Num() == 3 && meta[1] == TEXT("PRIVMSG"))
			{
				FString message = parts[1];
				if (parts.Num() > 2)
				{
					for (int i = 2; i < parts.Num(); i++)
					{
						message += TEXT(":") + parts[i];
					}
				}
				FString username;
				FString tmp;
				meta[0].Split(TEXT("!"), &username, &tmp);
				ReceivedChatMessage(username, message);
				continue;
			}
		}
	}
}


void UTwitchComponent::ErasePreviousMessages() {
	for (int i = 0; i < messageList.Num(); i++) {
		if (messageList[i].startTime + delay < timer) {
			messageList.RemoveAt(i);
			for (int j = i; j < messageList.Num(); j++) {
				if (messageList[i].message == messageList[j].message) {
					messageList[j].counter--;
				}
			}
			i--;
		}
	}
}

void UTwitchComponent::EraseLastNickUse() {
	for (int i = 0; i < lastNick.Num(); i++) {
		if (lastNick[i].exitTime < timer) {
			lastNick.RemoveAt(i);
			i--;
		}
	}
}

FString UTwitchComponent::GetMaxMessage() {
	int cou = 0;
	FString ans = "";
	for (int i = 0; i < messageList.Num(); i++) {
		if (messageList[i].counter > cou) {
			cou = messageList[i].counter;
			ans = messageList[i].message;
		}
	}
	return ans;// +FString::FromInt(cou);
}

void UTwitchComponent::SetUserDelay(float sec) {
	delay = sec;
}

void UTwitchComponent::SetMostPopularWindow(float sec) {
	userCommandDelay = sec;
}

bool UTwitchComponent::UserDelay(FString name) {
	for (int i = 0; i < lastNick.Num(); i++) {
		if (lastNick[i].nick == name) {
			return false;
		}
	}
	return true;
}

bool UTwitchComponent::IsCommand(FString name, FString msg) {
	//return (*commandFunction)(name, msg);
	//return controller->IsCommand(name, msg);
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, name + " " + msg);
	return true;
}

void UTwitchComponent::ReceivedChatMessage(FString UserName, FString message)
{
	//UE_LOG(LogTemp, Warning, TEXT("%s: %s"), *UserName, *message);
	FString s = *UserName + FString(" ") + *message;
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, s);
	s = "";
	ErasePreviousMessages();
	EraseLastNickUse();
	int cou = 0;
	for (int i = 0; i < messageList.Num(); i++) {
		if (messageList[i].message == message) {
			messageList[i].counter++;
			cou++;
		}
	}
	cou++;
	messageList.Add(MaxMessageStruct(timer, cou, message));
	log.Add(std::make_pair(UserName, message));
	if (this->UserDelay(UserName)) {
		if (true) {//this->IsCommand(message, UserName)) {
			lastNick.Add(LastNickUseStruct(timer + this->userCommandDelay, UserName));
		}
	}
}

std::pair<FString, FString> UTwitchComponent::GetLastIncome() {
	if (log.Num() > 0) {
		std::pair<FString, FString> pair = std::make_pair(log[0].first, log[0].second);
		log.RemoveAt(0);
		return pair;
	}
	return std::make_pair("", "");
}

void UTwitchComponent::SendLogin(FString name, FString okey)
{
	key = okey;
	auto ResolveInfo = ISocketSubsystem::Get()->GetHostByName("irc.twitch.tv");
	while (!ResolveInfo->IsComplete());
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Start");
	if (ResolveInfo->GetErrorCode() != 0)
	{
		//	UE_LOG(LogTemp, Warning, TEXT("Couldn't resolve hostname."));
		return;
	}
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "IniSocket");
	const FInternetAddr* Addr = &ResolveInfo->GetResolvedAddress();
	uint32 OutIP = 0;
	Addr->GetIp(OutIP);
	int32 port = 6667;

	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(OutIP);
	addr->SetPort(port);
	ListenerSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);

	bool connected = ListenerSocket->Connect(*addr);
	if (!connected)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Failed to connect."));
		if (ListenerSocket)
			ListenerSocket->Close();
		return;
	}
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "Connect");

	SendString(TEXT("PASS oauth:" + okey));
	SendString(TEXT("NICK " + name));

}

void UTwitchComponent::JoinChannel(FString channel) {
	SendString("JOIN #" + channel);
	joinedChannel = channel;
	connect = true;
}

void UTwitchComponent::Send(FString msg) {
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, joinedChannel + " : " + msg);
	SendString("PRIVMSG #" + joinedChannel + " : " + msg);
}

bool UTwitchComponent::SendString(FString msg)
{
	FString serialized = msg + TEXT("\r\n");
	TCHAR *serializedChar = serialized.GetCharArray().GetData();
	int32 size = FCString::Strlen(serializedChar);
	int32 sent = 0;
	if (debug) {
			return false;
	}
	return ListenerSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
}


// Called when the game starts
void UTwitchComponent::BeginPlay()
{
	Super::BeginPlay();
	userCommandDelay = 0;
	timer = 0;
	FIPv4Endpoint Endpoint(FIPv4Address(127, 0, 0, 1), 6667);
	FSocket* ListenerSocket = FTcpSocketBuilder(TEXT("TwitchListener")).AsReusable().BoundToEndpoint(Endpoint).Listening(8);

	//Set Buffer Size
	int32 NewSize = 0;
	ListenerSocket->SetReceiveBufferSize(2 * 1024 * 1024, NewSize);
	// ...
	
}


// Called every frame
void UTwitchComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (connect) {
		SocketListener(DeltaTime);
	}
	// ...
}

