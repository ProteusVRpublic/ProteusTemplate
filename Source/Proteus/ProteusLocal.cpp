// Fill out your copyright notice in the Description page of Project Settings.

#include "Proteus.h"
#include "ProteusLocal.h"
#include "OvrAvatarManager.h"
#include "Object.h"

ovrAvatarTransform gAvatarLeftHandTrans[] =
{
	{ { 0.0000f,0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0400f,-0.0250f,0.1091f },{ -0.6825f, 0.1749f, 0.7085f, 0.0411f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0735f,-0.0140f,-0.0240f },{ -0.3899f, 0.7092f, -0.1519f, 0.5674f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.1361f,0.0000f,-0.0000f },{ -0.2270f, 0.0000f, -0.0000f, 0.9739f },{ 1.0000f,1.0000f,1.0000f } },

	{ { 0.0871f,-0.0351f,0.0068f },{ -0.3804f, 0.6000f, -0.5778f, -0.4017f },{ 1.0000f,1.0000f,1.0000f } }, //Index
	{ { -0.0416f,-0.0000f,-0.0000f },{ -0.0000f, 0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0222f,0.0000f,0.0000f },{ -0.0000f, 0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0291f,0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { 0.0914f,-0.0095f,0.0108f },{ 0.4631f, -0.4423f, 0.5945f, 0.4863f },{ 1.0000f,1.0000f,1.0000f } }, //Middle
	{ { -0.0460f,-0.0000f,-0.0000f },{ 0.0000f, -0.0000f, -0.8362f, 0.5484f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0296f,0.0000f,0.0000f },{ -0.0000f, -0.0000f, -0.7300f, 0.6834f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0265f,0.0000f,-0.0000f },{ -0.0000f, 0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { 0.0313f,0.0191f,-0.0115f },{ 0.4713f, 0.0618f, 0.0753f, 0.8766f },{ 1.0000f,1.0000f,1.0000f } }, //Pinky
	{ { 0.0536f,0.0024f,0.0015f },{ 0.1300f, 0.0348f, 0.6327f, 0.7626f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0334f,0.0000f,-0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0174f,-0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0194f,0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { 0.0895f,0.0127f,0.0019f },{ 0.4589f, -0.3678f, 0.6193f, 0.5203f },{ 1.0000f,1.0000f,1.0000f } }, //Ring
	{ { -0.0386f,0.0000f,-0.0000f },{ -0.0000f, -0.0000f, -0.8446f, 0.5354f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0258f,-0.0000f,0.0000f },{ 0.0000f, -0.0000f, -0.7372f, 0.6757f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0242f,-0.0000f,0.0000f },{ -0.0000f, -0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { 0.0309f,-0.0415f,-0.0206f },{ 0.1999f, 0.9526f, 0.0626f, -0.2205f },{ 1.0000f,1.0000f,1.0000f } }, //Thumb
	{ { -0.0326f,0.0000f,-0.0000f },{ -0.0087f, 0.0964f, -0.2674f, 0.9587f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0264f,0.0000f,-0.0000f },{ -0.0000f, 0.0000f, -0.5985f, 0.8011f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0341f,0.0000f,0.0000f },{ -0.0000f, -0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
};

ovrAvatarTransform gAvatarRightHandTrans[] =
{
	{ { 0.0000f,0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0400f,-0.0250f,0.1091f },{ 0.0411f, -0.7085f, 0.1749f, 0.6825f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0735f,0.0140f,0.0240f },{ -0.5702f, -0.0164f, 0.8065f, -0.1554f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.1361f,0.0000f,-0.0000f },{ -0.2270f, -0.0000f, 0.0000f, 0.9739f },{ 1.0000f,1.0000f,1.0000f } },

	{ { -0.0871f,0.0351f,-0.0068f },{ -0.3804f, 0.6000f, -0.5778f, -0.4017f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0416f,-0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0222f,-0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0291f,0.0000f,-0.0000f },{ -0.0000f, -0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { -0.0914f,0.0095f,-0.0108f },{ 0.4631f, -0.4423f, 0.5945f, 0.4863f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0460f,0.0000f,0.0000f },{ 0.0000f, -0.0000f, -0.8362f, 0.5484f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0296f,-0.0000f,0.0000f },{ 0.0000f, -0.0000f, -0.7300f, 0.6834f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0265f,0.0000f,-0.0000f },{ 0.0000f, -0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { -0.0313f,-0.0191f,0.0115f },{ 0.4713f, 0.0618f, 0.0753f, 0.8766f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0536f,-0.0024f,-0.0015f },{ 0.1300f, 0.0348f, 0.6327f, 0.7626f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0334f,0.0000f,-0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0174f,-0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0194f,0.0000f,-0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { -0.0895f,-0.0127f,-0.0019f },{ 0.4589f, -0.3678f, 0.6193f, 0.5203f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0386f,0.0000f,0.0000f },{ -0.0000f, -0.0000f, -0.8446f, 0.5354f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0258f,0.0000f,-0.0000f },{ -0.0000f, 0.0000f, -0.7372f, 0.6757f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0242f,-0.0000f,-0.0000f },{ 0.0000f, 0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { -0.0309f,0.0415f,0.0206f },{ 0.1999f, 0.9526f, 0.0626f, -0.2205f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0326f,0.0000f,0.0000f },{ -0.0087f, 0.0964f, -0.2674f, 0.9587f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0264f,-0.0000f,-0.0000f },{ 0.0000f, -0.0000f, -0.5985f, 0.8011f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0341f,0.0000f,-0.0000f },{ 0.0000f, -0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
};

static const uint32_t HAND_JOINTS = 25;

// Constructor
AProteusLocal::AProteusLocal()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ProteusLocalRoot"));

	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Disabled;

	/** Pawn base eye height above collision center. */
	BaseEyeHeight = 0.f;
	
	AvatarComponent = CreateDefaultSubobject<UOvrAvatar>(TEXT("ProteusLocalAvatarComponent"));

	//SET FIRST, THIRD OR SELF_OCCLUDING FLAG
	AvatarComponent->SetVisibilityType(ovrAvatarVisibilityFlag_SelfOccluding);

	//Owner No See?

	//TODO EXPOSE TO BP
	//AvatarComponent->SetPlayerHeightOffset(BaseEyeHeight / 0.f);
}

// Called when the game starts or when spawned
void AProteusLocal::BeginPlay()
{
	Super::BeginPlay();

	AvatarHands[ovrHand_Left] = nullptr;
	AvatarHands[ovrHand_Right] = nullptr;

	AProteusLocal::AdditionalSetup();
}

// Called every frame
void AProteusLocal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AvatarHands[ovrHand_Left] = nullptr;
	AvatarHands[ovrHand_Right] = nullptr;

	UpdatePacketRecording(DeltaTime);
}

//Proteus Additional Setup
void AProteusLocal::AdditionalSetup()
{
	Online::GetVoiceInterface()->StartNetworkedVoice(0);
	UE_LOG(LogTemp, Warning, TEXT(" StartNetworkingVoice has fired!"));
}

//AVATAR LOGIN
int64 AProteusLocal::ConvertAvatarID(int32 rough1, int32 rough2, int32 rough3)
{
	UE_LOG(LogTemp, Warning, TEXT("ConvertAvatar has fired!"));
	UE_LOG(LogTemp, Warning, TEXT("Part1(rough) is %d, Part2(rough) is %d, Part3(rough) is %d"), rough1, rough2, rough3);

	int64 final1 = (int64)rough1;
	int64 final2 = (int64)rough2;
	int64 final3 = (int64)rough3;

	finalAvatar = final1 * 10000000000 + final2 * 100000 + final3;

	UE_LOG(LogTemp, Warning, TEXT("Part1 (final) is %d, Part2 (final) is %d, Part3 (final) is %d"), final1, final2, final3);
	return finalAvatar;
}

void AProteusLocal::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	// TODO SW: Check bWasSuccessful and get proper login flow/user IDs working.
	UE_LOG(LogTemp, Warning, TEXT("OnLoginComplete has fired!"));
	if (AvatarComponent)
	{
		AvatarComponent->RequestAvatar(finalAvatar);
	}
}

void AProteusLocal::FetchAvatarID(int32 PartA, int32 PartB, int32 PartC)
{
	int32 rough1 = PartA;
	int32 rough2 = PartB;
	int32 rough3 = PartC;

	UE_LOG(LogTemp, Warning, TEXT("FetchAvatar has fired!"));
	UE_LOG(LogTemp, Warning, TEXT("PartA is %d, PartB is %d, PartC is %d"), PartA, PartB, PartC);

	AProteusLocal::ConvertAvatarID(rough1, rough2, rough3);

	IOnlineIdentityPtr IdentityInterface = Online::GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		OnLoginCompleteDelegateHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(0, FOnLoginCompleteDelegate::CreateUObject(this, &AProteusLocal::OnLoginComplete));
		IdentityInterface->AutoLogin(0);
	}
	return;
}

/*void AProteusLocal::DisconnectVOIP()
{
Online::GetVoiceInterface()->UnregisterRemoteTalker(<FUniqueNetIdOculus>);
}
*/
bool AProteusLocal::RegisterRemoteTalker(FString OculusID)
{
	auto uniqueNetId = Online::GetIdentityInterface()->CreateUniquePlayerId(OculusID);
	UE_LOG(LogTemp, Warning, TEXT("Remote Talker is Registered"));
	return Online::GetVoiceInterface()->RegisterRemoteTalker(*uniqueNetId);
}

bool AProteusLocal::UnregisterRemoteTalker(FString OculusID)
{
	auto uniqueNetId = Online::GetIdentityInterface()->CreateUniquePlayerId(OculusID);
	return Online::GetVoiceInterface()->UnregisterRemoteTalker(*uniqueNetId);
}

//PACKETS
static float gDegreeOffset = 60.f;

void AProteusLocal::UpdatePacketRecording(float DeltaTime)
{
	if (!AvatarComponent)
		return;

	if (!PacketSettings.Initialized)
	{
		AvatarComponent->StartPacketRecording();
		PacketSettings.AccumulatedTime = 0.f;
		PacketSettings.RecordingFrames = true;
		PacketSettings.Initialized = true;
	}

	if (PacketSettings.RecordingFrames)
	{
		PacketSettings.AccumulatedTime += DeltaTime;

		if (PacketSettings.AccumulatedTime >= PacketSettings.UpdateRate)
		{
			PacketSettings.AccumulatedTime = 0.f;
			FOvrAvatarManager::Get().QueueAvatarPacket(AvatarComponent->EndPacketRecording());
			AvatarComponent->StartPacketRecording();
		}
	}
}

// Called to bind functionality to input
void AProteusLocal::SetupPlayerInputComponent(UInputComponent* Input)
{
	Super::SetupPlayerInputComponent(Input);

#define INPUT_ENTRY(entry, hand, flag) \
    Input->BindAction(#entry, IE_Pressed, this, &AProteusLocal::##entry##_Pressed); \
    Input->BindAction(#entry, IE_Released, this, &AProteusLocal::##entry##_Released); 
	INPUT_COMMAND_TUPLE
#undef INPUT_ENTRY

#define AXIS_ENTRY(entry, hand, flag) \
    Input->BindAxis(#entry, this, &AProteusLocal::##entry##_Value);
		AXIS_INPUT_TUPLE
#undef AXIS_ENTRY

#define CUSTOM_ENTRY(entry, hand, field, invert) \
        Input->BindAxis(#entry, this, &AProteusLocal::##entry##_Value);
		CUSTOM_AXIS_TUPLE
#undef CUSTOM_ENTRY

	Input->BindAxis("AvatarLeftThumbstickX", this, &AProteusLocal::LeftThumbstickX_Value);
	Input->BindAxis("AvatarLeftThumbstickY", this, &AProteusLocal::LeftThumbstickY_Value);
	Input->BindAxis("AvatarRightThumbstickX", this, &AProteusLocal::RightThumbstickX_Value);
	Input->BindAxis("AvatarRightThumbstickY", this, &AProteusLocal::RightThumbstickY_Value);

	//Input->BindAction("AvatarToggleRecordPackets", IE_Pressed, this, &ALocalAvatar::ToggleRecordPackets);
	//Input->BindAction("AvatarSpawnRemoteAvatar", IE_Pressed, this, &ALocalAvatar::SpawnNewRemoteAvatar);
	//Input->BindAction("AvatarDestroyRemoteAvatar", IE_Pressed, this, &ALocalAvatar::DestroyRemoteAvatar);
	//Input->BindAction("AvatarCycleLeftHandAttach", IE_Pressed, this, &ALocalAvatar::CycleLeftHandAttach);
	//Input->BindAction("AvatarCycleRightHandAttach", IE_Pressed, this, &ALocalAvatar::CycleRightHandAttach);
	//Input->BindAction("AvatarCycleRightHandPose", IE_Pressed, this, &ALocalAvatar::CycleRightHandPose);
	//Input->BindAction("AvatarCycleLeftHandPose", IE_Pressed, this, &ALocalAvatar::CycleLeftHandPose);
}
//INPUT VALUES
void AProteusLocal::LeftThumbstickX_Value(float value)
{
	StickPosition[ovrHand_Left].X = value;
}
void AProteusLocal::LeftThumbstickY_Value(float value)
{
	StickPosition[ovrHand_Left].Y = -value;
}
void AProteusLocal::RightThumbstickX_Value(float value)
{
	StickPosition[ovrHand_Right].X = value;
}
void AProteusLocal::RightThumbstickY_Value(float value)
{
	StickPosition[ovrHand_Right].Y = -value;
}

//DRIVE HAND & POSES
void AProteusLocal::DriveHand(ovrHandType hand, float DeltaTime)
{
	const float HandSpeed = 50.f; // m/s hand speed
	const float Threshold = 0.5f*0.5f;

	if (AvatarHands[hand].IsValid() && StickPosition[hand].SizeSquared() > Threshold)
	{
		AvatarHands[hand]->MoveComponent(FVector(StickPosition[hand].Y, StickPosition[hand].X, 0.f) * HandSpeed * DeltaTime, AvatarHands[hand]->GetComponentRotation(), false);
	}
}

#define CUSTOM_ENTRY(entry, hand, field, invert) \
        void AProteusLocal::##entry##_Value(float value)  {  AvatarComponent->##entry##_Value(value); }
CUSTOM_AXIS_TUPLE
#undef CUSTOM_ENTRY

#define INPUT_ENTRY(entry, hand, flag) \
    void AProteusLocal::##entry##_Pressed()  { AvatarComponent->##entry##_Pressed();}\
    void AProteusLocal::##entry##_Released() {  AvatarComponent->##entry##_Released(); }
INPUT_COMMAND_TUPLE
#undef INPUT_ENTRY

#define AXIS_ENTRY(entry, hand, flag) \
    void AProteusLocal::##entry##_Value( float value) { AvatarComponent->##entry##_Value(value); }
AXIS_INPUT_TUPLE
#undef AXIS_ENTRY