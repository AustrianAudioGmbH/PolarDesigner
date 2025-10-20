#!/bin/bash

# Check if filename argument is provided
if [ -z "$1" ]; then
    echo "Error: No filename provided"
    echo "Usage: $0 <package_filename>"
    exit 1
fi

filename="$1"

# Extract 7-digit hash before _Mac_Release
hash=$(echo "$filename" | grep -oE '[0-9a-f]{7}_Mac_Release' | sed 's/_Mac_Release//')
if [ -z "$hash" ]; then
    echo "Error: Could not extract hash from filename"
    exit 1
fi

echo "========================================================================================"
echo "Starting test of Release $hash at $(date)"

echo "Installing package $filename"

# Check if file exists before running installer
if [ ! -f "$filename" ]; then
    echo "Error: Package file $filename not found"
    exit 1
fi

# Run installer with sudo if needed, and check for success
if ! installer -pkg "$filename" -target CurrentUserHomeDirectory; then
    echo "Error: Package installation failed"
    exit 1
fi

echo "Testing $hash:"

# Use pushd with error handling
if ! pushd . >/dev/null; then
    echo "Error: pushd failed"
    exit 1
fi

# Create directory and check if successful
if ! mkdir -p "Results/$hash"; then
    echo "Error: Failed to create directory $hash"
    exit 1
fi

# Change to directory and check if successful
if ! cd "Results/$hash"; then
    echo "Error: Failed to change to directory $hash"
    exit 1
fi

# Check if tar file exists before extracting
if [ ! -f "../../PolarDesigner_v2_vs_v3_TEST.tar" ]; then
    echo "Error: Tar file ../PolarDesigner_v2_vs_v3_TEST.tar not found"
    exit 1
fi

# Extract tar file
if ! tar xvf ../../PolarDesigner_v2_vs_v3_TEST.tar; then
    echo "Error: Failed to extract tar file"
    exit 1
fi

# Init git repo
git init . && git add . && git commit -m "Initial commit prior to test"

# Check if REAPER executable exists
reaper_path="/Applications/REAPER/REAPER.app/Contents/MacOS/REAPER"
if [ ! -x "$reaper_path" ]; then
    echo "Error: REAPER executable not found at $reaper_path"
    exit 1
fi

# Check if project and script files exist
project_file="PolarDesigner_v2_vs_v3.RPP"
script_file="Scripts/PolarDesigner_V2_vs_V3_RenderABTracks.lua"
if [ ! -f "$project_file" ] || [ ! -f "$script_file" ]; then
    echo "Error: Required project or script file not found"
    exit 1
fi

# Run REAPER with project and script
if ! "$reaper_path" "$project_file" "$script_file"; then
    echo "Error: REAPER execution failed"
    exit 1
fi

# Report git repo status
git status

# Return to original directory
if ! popd >/dev/null; then
    echo "Error: popd failed"
    exit 1
fi

echo "Script completed successfully"

echo "Ending test of Release $hash at $(date)"
echo "-----------------------------------------------------------------------------------------"
