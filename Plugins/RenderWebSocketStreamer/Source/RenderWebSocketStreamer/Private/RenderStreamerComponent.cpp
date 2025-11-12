#include "RenderStreamerComponent.h"
#include "Engine/World.h"
#include "Engine/TextureRenderTarget2D.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "WebSocketsModule.h"
#include "Logging/LogMacros.h"
#include "TimerManager.h"
#include "RenderUtils.h"
#include "Async/Async.h"
#include "RHICommandList.h"

DEFINE_LOG_CATEGORY_STATIC(LogRenderStreamer, Log, All);

URenderStreamerComponent::URenderStreamerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void URenderStreamerComponent::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogRenderStreamer, Warning, TEXT("BeginPlay: No world"));
        return;
    }

    if ((World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE) && bAutoStart)
    {
        // Stagger startup so multiple cams don’t connect at the exact same moment
        const float Offset = (StreamId.IsEmpty() ? 0.f : (StreamId[0] % 10) * 0.10f);
        FTimerHandle TmpHandle;
        World->GetTimerManager().SetTimer(TmpHandle, [this]() { StartStreaming(); }, Offset, false);
    }
}

void URenderStreamerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (bIsStreaming) { StopStreaming(); }
    Super::EndPlay(EndPlayReason);
}

FString URenderStreamerComponent::MakeIngestUrl() const
{
    FString Base = WebSocketBase;
    Base.TrimStartAndEndInline();
    while (Base.EndsWith(TEXT("/"))) Base.LeftChopInline(1);
    const FString Sid = StreamId.IsEmpty() ? TEXT("default") : StreamId;
    return FString::Printf(TEXT("%s/ingest/%s"), *Base, *Sid);
}

void URenderStreamerComponent::StartStreaming()
{
    UWorld* World = GetWorld();
    if (!World || (World->WorldType != EWorldType::Game && World->WorldType != EWorldType::PIE))
    {
        UE_LOG(LogRenderStreamer, Warning, TEXT("StartStreaming ignored (not in Game/PIE)"));
        return;
    }

    if (bIsStreaming) return;

    if (!EnsureCaptureSetup())
    {
        UE_LOG(LogRenderStreamer, Error, TEXT("Failed to setup capture"));
        return;
    }

    FrameCounter = 0;
    TimeSinceLastSend = 0.f;
    bHasCapturedOnce = false;

    ConnectWebSocket();
    bIsStreaming = true;

    UE_LOG(LogRenderStreamer, Log, TEXT("Render streaming started (%s) → %s"),
        *StreamId, *MakeIngestUrl());
}

void URenderStreamerComponent::StopStreaming()
{
    if (!bIsStreaming) return;
    bIsStreaming = false;
    Cleanup();
    UE_LOG(LogRenderStreamer, Log, TEXT("Render streaming stopped (%s)"), *StreamId);
}

bool URenderStreamerComponent::EnsureCaptureSetup()
{
    UWorld* World = GetWorld();
    AActor* Owner = GetOwner();

    if (!World || !Owner)
    {
        UE_LOG(LogRenderStreamer, Error, TEXT("EnsureCaptureSetup: No world or owner"));
        return false;
    }

    if (!RenderTarget)
    {
        RenderTarget = NewObject<UTextureRenderTarget2D>(this);
        RenderTarget->ClearColor = FLinearColor::Black;
        RenderTarget->InitCustomFormat(TargetWidth, TargetHeight, PF_B8G8R8A8, false);
        RenderTarget->UpdateResourceImmediate(true);
    }

    if (ExternalCaptureActor && IsValid(ExternalCaptureActor))
    {
        CaptureComp = ExternalCaptureActor->GetCaptureComponent2D();
    }
    else
    {
        if (!OwnedCaptureActor.IsValid())
        {
            FActorSpawnParameters Params;
            Params.Owner = Owner;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            ASceneCapture2D* NewCap = World->SpawnActor<ASceneCapture2D>(
                Owner->GetActorLocation(), Owner->GetActorRotation(), Params);
            if (!NewCap)
            {
                UE_LOG(LogRenderStreamer, Error, TEXT("Failed to spawn ASceneCapture2D"));
                return false;
            }
            OwnedCaptureActor = NewCap;
            UE_LOG(LogRenderStreamer, Log, TEXT("Spawned capture (%s) at %s"),
                *StreamId, *Owner->GetActorLocation().ToString());
        }
        CaptureComp = OwnedCaptureActor->GetCaptureComponent2D();
    }

    if (!CaptureComp)
    {
        UE_LOG(LogRenderStreamer, Error, TEXT("EnsureCaptureSetup: No SceneCaptureComponent2D"));
        return false;
    }

    // Trim show flags for speed (optional)
    CaptureComp->ShowFlags.SetMotionBlur(false);
    CaptureComp->ShowFlags.SetAntiAliasing(true);
    CaptureComp->ShowFlags.SetPostProcessing(true);

    CaptureComp->CaptureSource = CaptureSource;
    CaptureComp->TextureTarget = RenderTarget;
    CaptureComp->bCaptureEveryFrame = false;            // manual cadence
    CaptureComp->bAlwaysPersistRenderingState = true;   // keep RT valid

    return true;
}

FTextureRenderTargetResource* URenderStreamerComponent::GetRTResource() const
{
    return RenderTarget ? RenderTarget->GameThread_GetRenderTargetResource() : nullptr;
}

// (kept for reference; not used when async is available)
bool URenderStreamerComponent::EncodeFrame(TArray<uint8>& OutBytes) const
{
    if (!RenderTarget) return false;
    FTextureRenderTargetResource* RTRes = GetRTResource();
    if (!RTRes) return false;

    TArray<FColor> Pixels;
    FReadSurfaceDataFlags Flags(RCM_UNorm);
    Flags.SetLinearToGamma(true);
    if (!RTRes->ReadPixels(Pixels, Flags))
    {
        UE_LOG(LogRenderStreamer, Warning, TEXT("[%s] ReadPixels failed"), *StreamId);
        return false;
    }

    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
    const EImageFormat ImgFmt = (Format == ERenderStreamerFormat::PNG) ? EImageFormat::PNG : EImageFormat::JPEG;
    TSharedPtr<IImageWrapper> Wrapper = ImageWrapperModule.CreateImageWrapper(ImgFmt);

    const int32 W = RenderTarget->SizeX;
    const int32 H = RenderTarget->SizeY;

    if (!Wrapper->SetRaw(Pixels.GetData(), Pixels.Num() * sizeof(FColor), W, H, ERGBFormat::BGRA, 8))
        return false;

    const TArray64<uint8> Compressed = (Format == ERenderStreamerFormat::PNG)
        ? Wrapper->GetCompressed(100)
        : Wrapper->GetCompressed(JPEGQuality);

    OutBytes = TArray<uint8>(Compressed);
    return OutBytes.Num() > 0;
}

bool URenderStreamerComponent::EncodeFrame_Async(TArray<uint8>& OutBytes) const
{
    if (!RenderTarget) return false;

    FTextureRenderTargetResource* RTRes = RenderTarget->GameThread_GetRenderTargetResource();
    if (!RTRes) return false;

    // Be explicit: for UTextureRenderTarget2D, resource is FTextureRenderTarget2DResource
    FTextureRenderTarget2DResource* RT2D = static_cast<FTextureRenderTarget2DResource*>(RTRes);
    if (!RT2D) return false;

    const int32 W = RenderTarget->SizeX;
    const int32 H = RenderTarget->SizeY;

    // CPU buffer for readback
    TArray<FColor> Pixels;
    Pixels.SetNumUninitialized(W * H);

    FEvent* ReadDoneEvent = FPlatformProcess::GetSynchEventFromPool(false);

    ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)(
        [RT2D, W, H, &Pixels, ReadDoneEvent](FRHICommandListImmediate& RHICmdList)
        {
            FRHITexture* Texture = RT2D->GetTextureRHI();
            if (Texture)
            {
                // Transition just to be safe for copy/read
                RHICmdList.Transition(FRHITransitionInfo(Texture, ERHIAccess::Unknown, ERHIAccess::CopySrc));

                FReadSurfaceDataFlags Flags(RCM_UNorm);
                Flags.SetLinearToGamma(true);

                RHICmdList.ReadSurfaceData(
                    Texture,
                    FIntRect(0, 0, W, H),
                    Pixels,
                    Flags
                );
            }
            ReadDoneEvent->Trigger();
        });

    // Wait only for this readback (no full flush)
    ReadDoneEvent->Wait();
    FPlatformProcess::ReturnSynchEventToPool(ReadDoneEvent);

    if (Pixels.Num() == 0)
    {
        UE_LOG(LogRenderStreamer, Warning, TEXT("[%s] Async read produced no pixels"), *StreamId);
        return false;
    }

    // Encode on game thread
    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
    const EImageFormat ImgFmt = (Format == ERenderStreamerFormat::PNG) ? EImageFormat::PNG : EImageFormat::JPEG;
    TSharedPtr<IImageWrapper> Wrapper = ImageWrapperModule.CreateImageWrapper(ImgFmt);

    if (!Wrapper->SetRaw(Pixels.GetData(), Pixels.Num() * sizeof(FColor), W, H, ERGBFormat::BGRA, 8))
        return false;

    const TArray64<uint8> Compressed = (Format == ERenderStreamerFormat::PNG)
        ? Wrapper->GetCompressed(100)
        : Wrapper->GetCompressed(JPEGQuality);

    OutBytes = TArray<uint8>(Compressed);
    return OutBytes.Num() > 0;
}

void URenderStreamerComponent::ConnectWebSocket()
{
    if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
        FModuleManager::LoadModuleChecked<FWebSocketsModule>("WebSockets");

    const FString Url = MakeIngestUrl();
    UE_LOG(LogRenderStreamer, Log, TEXT("Connecting WebSocket (%s) → %s"), *StreamId, *Url);

    Socket = FWebSocketsModule::Get().CreateWebSocket(Url);

    Socket->OnConnected().AddLambda([this]()
    {
        UE_LOG(LogRenderStreamer, Log, TEXT("[%s] WebSocket connected"), *StreamId);
    });

    Socket->OnConnectionError().AddLambda([this](const FString& Err)
    {
        UE_LOG(LogRenderStreamer, Error, TEXT("[%s] WebSocket error: %s"), *StreamId, *Err);
    });

    Socket->OnClosed().AddLambda([this](int32 StatusCode, const FString& Reason, bool bWasClean)
    {
        UE_LOG(LogRenderStreamer, Warning, TEXT("[%s] WebSocket closed (%d) %s clean=%d"),
            *StreamId, StatusCode, *Reason, bWasClean);
    });

    Socket->Connect();
}

void URenderStreamerComponent::Cleanup()
{
    if (Socket.IsValid())
    {
        Socket->Close();
        Socket.Reset();
    }
}

void URenderStreamerComponent::TrySendFrame(float DeltaTime)
{
    if (!bIsStreaming || !Socket.IsValid() || !Socket->IsConnected())
        return;

    TimeSinceLastSend += DeltaTime;
    const float Interval = 1.0f / FMath::Max(1, SendFps);
    if (TimeSinceLastSend < Interval) return;
    TimeSinceLastSend = 0.f;

    // Only send if we have previously captured at least once
    if (!bHasCapturedOnce) return;

    TArray<uint8> Encoded;

    // Preferred async readback
    if (!EncodeFrame_Async(Encoded) || Encoded.Num() == 0)
    {
        UE_LOG(LogRenderStreamer, VeryVerbose, TEXT("[%s] No frame data (skipped)"), *StreamId);
        return;
    }

    Socket->Send(Encoded.GetData(), Encoded.Num(), true);
    UE_LOG(LogRenderStreamer, Verbose, TEXT("[%s][%p] Sent %d bytes (async)"),
        *StreamId, this, Encoded.Num());
}

void URenderStreamerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsStreaming || !CaptureComp || !RenderTarget) return;

    ++FrameCounter;
    const int32 CaptureInterval = FMath::Max(1, 60 / SendFps);

    // ✅ Pipeline one frame behind:
    // 1) Capture now (this frame's scene)
    if (FrameCounter % CaptureInterval == 0)
    {
        CaptureComp->CaptureScene();
        bHasCapturedOnce = true; // after first capture, reads can produce valid pixels
        UE_LOG(LogRenderStreamer, VeryVerbose, TEXT("[%s] Captured scene"), *StreamId);
    }

    // 2) Try to send (readback previous captured contents)
    TrySendFrame(DeltaTime);
}

void URenderStreamerComponent::EditorStartStreaming()
{
#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (!World || (World->WorldType != EWorldType::PIE && World->WorldType != EWorldType::EditorPreview))
    {
        UE_LOG(LogRenderStreamer, Warning, TEXT("EditorStartStreaming only in PIE/EditorPreview"));
        return;
    }
    StartStreaming();
#endif
}

void URenderStreamerComponent::EditorStopStreaming()
{
#if WITH_EDITOR
    StopStreaming();
#endif
}
