#!/bin/bash

# Script: 00_polardesigner_all_formats_reaper_test.sh
# Purpose: Automate plugin testing in REAPER on macOS by rendering a project and comparing output with a reference file.
# Specification: for PolarDesigner, AU and VST3 versions of the plugin
# Requirements: REAPER installed, project file with saved render settings, reference audio file, sox for comparison.
# Features:
#   - Normal mode: Render project, compare with reference, log results.
#   - --create-gold-reference: Render project, move output to reference path, skip comparison.

# Configuration variables
REAPER_PATH="/Applications/REAPER.app/Contents/MacOS/REAPER"  					# Path to REAPER executable
PROJECT_PATH="Reaper/00_polardesigner_all_formats.rpp"        					# Path to REAPER project file
OUTPUT_PATH="Results/00_polardesigner_all_formats_reaper.wav"  					# Path where REAPER will save rendered output
REFERENCE_PATH="Reference/00_polardesigner_all_formats_reaper_reference.wav"   	# Path to reference audio file
LOG_FILE="Reference/00_polardesigner_all_formats_reaper_results.txt"           	# Log file for REAPER output
TIMEOUT=300                                                 					# Maximum time (seconds) to wait for render
SOX_DIFF_OUTPUT="Results/00_polardesigner_all_formats_reaper_test_diff_output.wav"     # Output file for audio difference
SOX_ERROR_LOG="Results/sox_error_log.txt"                                      	# Log file for sox errors
TEST_HISTORY_LOG="Results/test_history.log"                                    	# Log file for test results

# Check for --create-gold-reference argument
CREATE_GOLD_REFERENCE=0
if [[ "$1" == "--create-gold-reference" ]]; then
    CREATE_GOLD_REFERENCE=1
fi

# Step 1: Validate environment
echo "Validating environment..."

# Check if REAPER executable exists
if [[ ! -x "$REAPER_PATH" ]]; then
    echo "Error: REAPER executable not found at $REAPER_PATH"
    exit 1
fi

# Check if project file exists
if [[ ! -f "$PROJECT_PATH" ]]; then
    echo "Error: Project file not found at $PROJECT_PATH"
    exit 1
fi

# If not creating gold reference, check if reference file exists
if [[ $CREATE_GOLD_REFERENCE -eq 0 && ! -f "$REFERENCE_PATH" ]]; then
    echo "Error: Reference file not found at $REFERENCE_PATH"
    exit 1
fi

# If creating gold reference, check if reference file already exists (prevent overwrite)
if [[ $CREATE_GOLD_REFERENCE -eq 1 && -f "$REFERENCE_PATH" ]]; then
    echo "Error: Reference file $REFERENCE_PATH already exists. Remove or rename it first."
    exit 1
fi

# Check if sox is installed (only needed for comparison)
if [[ $CREATE_GOLD_REFERENCE -eq 0 ]]; then
    if ! command -v sox &> /dev/null; then
        echo "Error: sox not installed. Install it using 'brew install sox'."
        exit 1
    fi

    # Check if soxi is installed (for debugging file info)
    if ! command -v soxi &> /dev/null; then
        echo "Error: soxi not installed. Install it using 'brew install sox'."
        exit 1
    fi
fi

# Step 2: Ensure output directory exists
# For gold reference, create Reference/ directory; otherwise, create Results/
if [[ $CREATE_GOLD_REFERENCE -eq 1 ]]; then
    OUTPUT_DIR=$(dirname "$REFERENCE_PATH")
else
    OUTPUT_DIR=$(dirname "$OUTPUT_PATH")
fi
if [[ ! -d "$OUTPUT_DIR" ]]; then
    echo "Creating output directory: $OUTPUT_DIR"
    mkdir -p "$OUTPUT_DIR"
fi

# Always create Results/ directory since REAPER renders to OUTPUT_PATH
RESULTS_DIR=$(dirname "$OUTPUT_PATH")
if [[ ! -d "$RESULTS_DIR" ]]; then
    echo "Creating results directory: $RESULTS_DIR"
    mkdir -p "$RESULTS_DIR"
fi

# Step 3: Run REAPER to render the project
if [[ $CREATE_GOLD_REFERENCE -eq 1 ]]; then
    echo "Rendering project to create gold reference: $OUTPUT_PATH"
else
    echo "Rendering project: $PROJECT_PATH"
fi
"$REAPER_PATH" -renderproject "$PROJECT_PATH" > "$LOG_FILE" 2>&1 &

# Get REAPER process ID
REAPER_PID=$!

# Wait for rendering to complete or timeout
echo "Waiting for render to complete (timeout: ${TIMEOUT}s)..."
wait_timeout=$TIMEOUT
while [[ $wait_timeout -gt 0 ]]; do
    if ! ps -p $REAPER_PID > /dev/null; then
        echo "Rendering completed."
        break
    fi
    sleep 1
    ((wait_timeout--))
done

# Check if REAPER is still running (timeout reached)
if ps -p $REAPER_PID > /dev/null; then
    echo "Error: Rendering timed out after ${TIMEOUT}s. Killing REAPER process."
    kill -9 $REAPER_PID
    exit 1
fi

# Step 4: Verify rendered output exists
if [[ ! -f "$OUTPUT_PATH" ]]; then
    echo "Error: Rendered output not found at $OUTPUT_PATH. Check $LOG_FILE for details."
    exit 1
fi

# If creating gold reference, move the rendered file to REFERENCE_PATH
if [[ $CREATE_GOLD_REFERENCE -eq 1 ]]; then
    echo "Moving rendered output to gold reference: $REFERENCE_PATH"
    if ! mv "$OUTPUT_PATH" "$REFERENCE_PATH"; then
        echo "Error: Failed to move $OUTPUT_PATH to $REFERENCE_PATH."
        exit 1
    fi
    echo "Successfully created gold reference file: $REFERENCE_PATH"
    echo "$(date): Created gold reference file ($REFERENCE_PATH)" >> "$TEST_HISTORY_LOG"
    exit 0
fi

# Step 5: Compare rendered output with reference
echo "Comparing rendered output with reference..."

# Debug: Print file info for reference and output files
echo "Reference file info:"
soxi "$REFERENCE_PATH"
echo "Rendered output file info:"
soxi "$OUTPUT_PATH"

# Step 5a: Mix the files with inverted phase to create difference file
if ! sox -V2 -m "$REFERENCE_PATH" -v -1 "$OUTPUT_PATH" "$SOX_DIFF_OUTPUT" 2> "$SOX_ERROR_LOG"; then
    echo "Error: sox failed to compute difference. Details:"
    cat "$SOX_ERROR_LOG"
    echo "Check input files, formats, or sox installation."
    exit 1
fi

# Verify difference file was created
if [[ ! -f "$SOX_DIFF_OUTPUT" ]]; then
    echo "Error: Difference file $SOX_DIFF_OUTPUT was not created."
    exit 1
fi

# Step 5b: Analyze difference file with sox stat
sox "$SOX_DIFF_OUTPUT" -n stat 2> "$SOX_ERROR_LOG"

# Extract RMS amplitude from stat output
RMS=$(grep "RMS     amplitude" "$SOX_ERROR_LOG" | awk '{print $3}')
if [[ -z "$RMS" ]]; then
    echo "Error: Failed to extract RMS amplitude from stat output."
    cat "$SOX_ERROR_LOG"
    exit 1
fi

# Debug: Print difference file size
DIFF_SIZE=$(stat -f %z "$SOX_DIFF_OUTPUT" 2>/dev/null)
if [[ $? -ne 0 || -z "$DIFF_SIZE" ]]; then
    echo "Error: Failed to get size of difference file $SOX_DIFF_OUTPUT."
    exit 1
fi
echo "Difference file size: $DIFF_SIZE bytes"

# Check if difference file is silent (RMS amplitude < 0.0001)
if (( $(echo "$RMS < 0.0001" | bc -l) )); then
    echo "Success: Rendered output matches reference (RMS: $RMS, size: $DIFF_SIZE bytes)."
    echo "$(date): Success (RMS: $RMS, size: $DIFF_SIZE bytes)" >> "$TEST_HISTORY_LOG"
else
    echo "Warning: Rendered output differs from reference (RMS: $RMS, size: $DIFF_SIZE bytes). Check $SOX_DIFF_OUTPUT for differences."
    echo "$(date): Warning (RMS: $RMS, size: $DIFF_SIZE bytes)" >> "$TEST_HISTORY_LOG"
    exit 1
fi

# Step 6: Clean up
echo "Cleaning up..."
rm -f "$SOX_DIFF_OUTPUT" "$SOX_ERROR_LOG"

echo "Test completed successfully."
exit 0
