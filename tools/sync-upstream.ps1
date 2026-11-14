# Sync thirdparty/duilib from upstream (duilib/duilib) through git subtree.
#
# Usage:  powershell -ExecutionPolicy Bypass -File tools\sync-upstream.ps1
#         powershell -ExecutionPolicy Bypass -File tools\sync-upstream.ps1 -Remote duilib -Branch master
#
# Always merge with --squash: the subtree baseline is recorded in the squash
# commit message, and a non-squash merge breaks the next pull.

param(
    [string]$Remote = 'upstream',
    [string]$Branch = 'master'
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
Push-Location -LiteralPath $repoRoot
try {
    $remotes = @(git remote)
    if ($LASTEXITCODE -ne 0) { throw 'git remote failed; is this a git repository?' }

    if ($remotes -notcontains $Remote) {
        Write-Host "[info] remote '$Remote' not found, adding https://github.com/duilib/duilib.git"
        git remote add $Remote 'https://github.com/duilib/duilib.git'
        if ($LASTEXITCODE -ne 0) { throw "git remote add failed (exit $LASTEXITCODE)" }
    }

    Write-Host "[info] git subtree pull --prefix=thirdparty/duilib $Remote $Branch --squash"
    git subtree pull --prefix=thirdparty/duilib $Remote $Branch --squash
    if ($LASTEXITCODE -ne 0) { throw "git subtree pull failed (exit $LASTEXITCODE)" }

    Write-Host '[ok] duilib synced from upstream; review the merge commit, then push'
}
finally {
    Pop-Location
}
