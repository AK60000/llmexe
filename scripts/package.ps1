param (
    [string]$exePath = "build\bin\llmexe.exe",
    [string]$modelPath = "models\Qwen3-0.6B-Q4_K_M.gguf",
    [string]$outputPath = "llmexe_standalone.exe"
)

Write-Host "Packaging standalone executable..."
Write-Host "EXE: $exePath"
Write-Host "Model: $modelPath"

if (-Not (Test-Path $exePath)) {
    Write-Error "Executable not found at $exePath"
    exit 1
}

if (-Not (Test-Path $modelPath)) {
    Write-Error "Model not found at $modelPath"
    exit 1
}

# Get file sizes
$modelSize = (Get-Item $modelPath).Length

# We need to construct the footer: 8 bytes (little endian size) + 8 bytes ("LLMEXE00")
$footer = New-Object byte[] 16
[System.BitConverter]::GetBytes([uint64]$modelSize).CopyTo($footer, 0)
[System.Text.Encoding]::ASCII.GetBytes("LLMEXE00").CopyTo($footer, 8)

Write-Host "Model size: $modelSize bytes"

# Use CMD copy to efficiently append binary files
# Copy /B exe + model output
$tempModel = "temp_model_payload.bin"
Copy-Item $modelPath $tempModel

# Append footer to the temp model file
$fs = [System.IO.File]::OpenWrite($tempModel)
$fs.Seek(0, [System.IO.SeekOrigin]::End) | Out-Null
$fs.Write($footer, 0, $footer.Length)
$fs.Close()

Write-Host "Appending files..."
cmd.exe /c "copy /b `"$exePath`" + `"$tempModel`" `"$outputPath`" > nul"

Remove-Item $tempModel

Write-Host "Packaging complete! Saved as $outputPath"
Write-Host "You can now run: .\$outputPath --prompt `"Hello! `""
