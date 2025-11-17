<#
.SYNOPSIS
Counts added, deleted, and total lines in a Git repository over a date range.

.PARAMETER Since
Start date (inclusive) in YYYY-MM-DD format.

.PARAMETER Until
End date (inclusive) in YYYY-MM-DD format.

.EXAMPLE
.\CountGitChanges.ps1 -Since "2025-11-01" -Until "2025-11-17"
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$Since,

    [Parameter(Mandatory=$true)]
    [string]$Until
)

# Ensure we are inside a Git repo
if (-not (git rev-parse --is-inside-work-tree 2>$null)) {
    Write-Error "This script must be run inside a Git repository."
    exit 1
}

# Get the line stats from git log
$stats = git log --since="$Since" --until="$Until" --pretty=tformat: --numstat

# Initialize counters
$added = 0
$deleted = 0

# Process each line
foreach ($line in $stats) {
    if ($line -match '^\d+\s+\d+\s+') {
        $parts = $line -split "\s+"
        $added += [int]$parts[0]
        $deleted += [int]$parts[1]
    }
}

# Output results
Write-Host "Date range: $Since → $Until"
Write-Host "Lines Added:    $added"
Write-Host "Lines Deleted:  $deleted"
Write-Host "Total Changed:  $($added + $deleted)"
