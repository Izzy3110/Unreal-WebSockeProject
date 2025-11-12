#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "IWebSocket.h"
#include "RenderStreamerComponent.generated.h"

UENUM(BlueprintType)
enum class ERenderStreamerFormat : uint8
{
    PNG  UMETA(DisplayName="PNG (lossless)"),
    JPEG UMETA(DisplayName="JPEG (compressed)")
};

UCLASS(ClassGroup=(Streaming), meta=(BlueprintSpawnableComponent))
class RENDERWEBSOCKETSTREAMER_API URenderStreamerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URenderStreamerComponent();

    /** Base WS URL (no stream path), e.g. ws://127.0.0.1:8765 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Render Streaming")
    FString WebSocketBase = TEXT("ws://127.0.0.1:8765");

    /** Stream identifier (used as /ingest/{StreamId}) – e.g. cam1..cam9 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Render Streaming")
    FString StreamId = TEXT("cam1");

    /** Target width/height */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Render Streaming", meta=(ClampMin="16", ClampMax="4096"))
    int32 TargetWidth = 640;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Render Streaming", meta=(ClampMin="16", ClampMax="4096"))
    int32 TargetHeight = 480;

    /** Capture source */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Render Streaming")
    TEnumAsByte<ESceneCaptureSource> CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

    /** Frames per second to send */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Render Streaming", meta=(ClampMin="1", ClampMax="120"))
    int32 SendFps = 15;

    /** Encoding format + JPEG quality */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Render Streaming")
    ERenderStreamerFormat Format = ERenderStreamerFormat::JPEG;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Render Streaming", meta=(ClampMin="1", ClampMax="100"))
    int32 JPEGQuality = 70;

    /** Auto-start when playing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Render Streaming")
    bool bAutoStart = true;

    /** Optional external SceneCapture actor to use */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Render Streaming")
    ASceneCapture2D* ExternalCaptureActor = nullptr;

    /** Start/Stop */
    UFUNCTION(BlueprintCallable, Category="Render Streaming")
    void StartStreaming();

    UFUNCTION(BlueprintCallable, Category="Render Streaming")
    void StopStreaming();

    UFUNCTION(BlueprintCallable, Category="Render Streaming")
    bool IsStreaming() const { return bIsStreaming; }

    /** Editor convenience buttons */
    UFUNCTION(CallInEditor, Category="Render Streaming")
    void EditorStartStreaming();

    UFUNCTION(CallInEditor, Category="Render Streaming")
    void EditorStopStreaming();

    // ActorComponent
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    bool EnsureCaptureSetup();
    bool EncodeFrame(TArray<uint8>& OutBytes) const;         // (kept for reference/tests)
    bool EncodeFrame_Async(TArray<uint8>& OutBytes) const;   // preferred
    void TrySendFrame(float DeltaTime);
    void ConnectWebSocket();
    void Cleanup();

    FString MakeIngestUrl() const;
    FTextureRenderTargetResource* GetRTResource() const;

private:
    TWeakObjectPtr<ASceneCapture2D> OwnedCaptureActor;

    UPROPERTY(Transient)
    USceneCaptureComponent2D* CaptureComp = nullptr;

    UPROPERTY(Transient)
    UTextureRenderTarget2D* RenderTarget = nullptr;

    TSharedPtr<IWebSocket> Socket;

    bool bIsStreaming = false;
    float TimeSinceLastSend = 0.f;

    /** Per-instance frame counter */
    int32 FrameCounter = 0;

    /** True after we have captured at least once */
    bool bHasCapturedOnce = false;
};
