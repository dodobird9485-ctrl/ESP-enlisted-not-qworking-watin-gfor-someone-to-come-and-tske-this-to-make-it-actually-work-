$paths = @(
    "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe",
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe"
)

$msbuild = $null
foreach ($path in $paths) {
    if (Test-Path $path) {
        $msbuild = $path
        break
    }
}

if ($null -eq $msbuild) {
    Write-Host "MSBuild not found"
    exit 1
}

Write-Host "Found MSBuild at: $msbuild"
& "$msbuild" "d:\Research\EnlistedESPOverlay.vcxproj" /p:Configuration=Release /p:Platform=x64

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build succeeded"
} else {
    Write-Host "Build failed with code $LASTEXITCODE"
}
exit $LASTEXITCODE
