param (
    [Parameter(Mandatory=$true)]
    [string]$ModelPath,
    [string]$ExePath = "build\bin\llmexe.exe",
    [string]$OutputPath = "standalone_llm.exe"
)

Write-Host "Packaging standalone executable..."
Write-Host "Base EXE: $exePath"
Write-Host "GGUF Model: $modelPath"

if (-Not (Test-Path $exePath)) {
    Write-Error "Executable not found at $exePath. Please build the project first."
    exit 1
}

if (-Not (Test-Path $modelPath)) {
    Write-Error "Model not found at $modelPath"
    exit 1
}

# Get file sizes
$modelSize = (Get-Item $modelPath).Length

# Construct the footer: 8 bytes (little endian size) + 8 bytes ("LLMEXE00")
$footer = New-Object byte[] 16
[System.BitConverter]::GetBytes([uint64]$modelSize).CopyTo($footer, 0)
[System.Text.Encoding]::ASCII.GetBytes("LLMEXE00").CopyTo($footer, 8)

Write-Host "Model size: $modelSize bytes"

# Use CMD copy to efficiently append binary files
$tempModel = "temp_model_payload.bin"
Copy-Item $modelPath $tempModel

# Append footer to the temp model file
$fs = [System.IO.File]::OpenWrite($tempModel)
$fs.Seek(0, [System.IO.SeekOrigin]::End) | Out-Null
$fs.Write($footer, 0, $footer.Length)
$fs.Close()

Write-Host "Appending model to executable..."
cmd.exe /c "copy /b `"$exePath`" + `"$tempModel`" `"$outputPath`" > nul"

Remove-Item $tempModel

Write-Host "Packaging complete! Saved as $outputPath"
Write-Host "You can now run: .\$outputPath --prompt `"Hello! `""
