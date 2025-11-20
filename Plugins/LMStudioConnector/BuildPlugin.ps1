param(
    [string]$EngineRoot = "C:\Users\sasch\UnrealEngine\UnrealEngine-5.6.1-release",
    [string]$ProjectFile = "C:\Users\sasch\UnrealProjects\WebSockeProject-git\WebSockeProject.uproject",
    [string]$PluginName = "LMStudioConnector",
    [string]$Configuration = "Development",
    [string]$Platform = "Win64",
    [string]$Target = "WebSockeProjectEditor"
)

# -------------------------
# Paths
# -------------------------
$BuildBat = Join-Path $EngineRoot "Engine\Build\BatchFiles\Build.bat"
$ProjectDir = Split-Path $ProjectFile
$PluginDir = Join-Path $ProjectDir "Plugins\$PluginName"

Write-Host "Engine Root: $EngineRoot"
Write-Host "Project:     $ProjectFile"
Write-Host "Platform:    $Platform"
Write-Host "Plugin:      $PluginDir"
Write-Host "Target:      $Target"
Write-Host ""

if (-not (Test-Path $BuildBat)) {
    Write-Host "❌ Build.bat wurde nicht gefunden!"
    exit 1
}

if (-not (Test-Path $PluginDir)) {
    Write-Host "❌ Plugin-Ordner wurde nicht im Projekt gefunden!"
    exit 1
}

# -------------------------
# Optional: Alte Binaries löschen
# -------------------------
Write-Host "🔨 Entferne alte Binaries & Intermediate..."
$bin = Join-Path $PluginDir "Binaries"
$int = Join-Path $PluginDir "Intermediate"

Remove-Item $bin -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item $int -Recurse -Force -ErrorAction SilentlyContinue

# -------------------------
# Build only this plugin
# UnrealBuildTool baut automatisch nur Module, die geändert wurden
# -------------------------
Write-Host "🚀 Baue Plugin '$PluginName'..."
Write-Host ""

# Der entscheidende Call:
& "$BuildBat" "$Target" "$Platform" $Configuration "-Project=""$ProjectFile""" -WaitMutex -NoHotReloadFromIDE

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "✅ Plugin erfolgreich gebaut!"
} else {
    Write-Host ""
    Write-Host "❌ Build fehlgeschlagen (Exitcode $LASTEXITCODE)"
}
