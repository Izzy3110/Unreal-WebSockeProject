param(
    [string]$EngineRoot = "C:\Users\sasch\UnrealEngine\UnrealEngine-5.6.1-release"
)
Write-Host "🔧 EngineRoot: $EngineRoot"
exit 0

# --- Anfang Auto-Setup Snippet ---
$propsFile = Join-Path $EngineRoot "Engine\Source\Programs\Shared\UnrealEngine.csproj.props"
if (-not (Test-Path $propsFile)) {
    Write-Host "🔧 Engine scheint nicht vorbereitet. Starte Setup + GenerateProjectFiles + Build UBT..."
    Push-Location $EngineRoot
    try {
        if (Test-Path ".\Setup.bat") {
            & ".\Setup.bat"
            if ($LASTEXITCODE -ne 0) { throw "Setup.bat fehlgeschlagen" }
        } else {
            throw "Setup.bat nicht gefunden im Engine-Root"
        }

        if (Test-Path ".\GenerateProjectFiles.bat") {
            & ".\GenerateProjectFiles.bat"
            if ($LASTEXITCODE -ne 0) { throw "GenerateProjectFiles.bat fehlgeschlagen" }
        } else {
            throw "GenerateProjectFiles.bat nicht gefunden im Engine-Root"
        }

        # Build UnrealBuildTool
        & ".\Engine\Build\BatchFiles\Build.bat" UnrealBuildTool Win64 Development
        if ($LASTEXITCODE -ne 0) { throw "Build UnrealBuildTool fehlgeschlagen" }

    } finally {
        Pop-Location
    }
    Write-Host "✅ Engine vorbereitet."
} else {
    Write-Host "Engine bereits vorbereitet (Props gefunden)."
}
# --- Ende Auto-Setup Snippet ---
