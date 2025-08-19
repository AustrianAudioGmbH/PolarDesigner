# Test Plan for Austrian Audio PolarDesigner Plugin in REAPER

## Objective

To validate the functionality, performance, and compatibility of the Austrian Audio PolarDesigner plugin within REAPER, ensuring it operates as expected for audio engineers manipulating polar patterns in post-production.

## Scope

- **Plugin**: Austrian Audio PolarDesigner (VST3, AU, or AAX format, as available).
- **DAW**: REAPER (version 7.35 or latest stable version as of April 24, 2025).

### Features to Test

- Polar pattern selection (omni, cardioid, hypercardioid, figure-eight, etc.).
- Frequency band adjustments (up to five bands).
- Dual-output mode processing for OC818 microphone emulation.
- Automation of polar pattern and frequency band parameters.
- Plugin stability and performance (CPU usage, latency, crashes).
- Preset management (save, load, and recall).
- Compatibility with REAPER’s routing and rendering capabilities.

## Test Environment

- **OS**: Windows 11 or macOS (latest stable versions).
- **Audio Interface**: Any ASIO-compatible interface for Windows or Core Audio for macOS.
- **Sample Rate**: 48 kHz (standard for testing).
- **Bit Depth**: 24-bit.
- **Buffer Size**: 256 samples (adjust as needed for performance).

## Test Scenarios

### Polar Pattern Manipulation

**Objective**: Verify that PolarDesigner can switch between polar patterns (omni, cardioid, hypercardioid, figure-eight) across different frequency bands.

**Test Steps**:
1. Load PolarDesigner on a track with a dual-output .wav file simulating OC818 microphone recordings.
2. Select each polar pattern and listen for changes in the audio’s spatial characteristics.
3. Adjust polar patterns in one frequency band at a time (e.g., 100 Hz–1 kHz cardioid, 1 kHz–5 kHz omni).
4. Record output to compare differences.

**Expected Result**: Smooth transitions between polar patterns, audible differences in directionality, and no artifacts.

### Frequency Band Adjustments

**Objective**: Test the ability to adjust up to five frequency bands independently.

**Test Steps**:
1. Configure PolarDesigner to use five frequency bands (e.g., 50 Hz–200 Hz, 200 Hz–1 kHz, 1 kHz–5 kHz, 5 kHz–10 kHz, 10 kHz–20 kHz).
2. Apply different polar patterns to each band (e.g., figure-eight for low, cardioid for mid, omni for high).
3. Adjust crossover points between bands and verify smooth transitions.
4. Export rendered audio for analysis.

**Expected Result**: Precise control over frequency bands, no phase issues, and consistent audio quality.

### Dual-Output Mode Processing

**Objective**: Ensure PolarDesigner correctly processes dual-output signals to emulate OC818’s capabilities.

**Test Steps**:
1. Use two mono .wav files (simulating front and rear diaphragms of OC818).
2. Route them to a stereo track with PolarDesigner.
3. Test polar pattern recombination (e.g., blending front and rear signals to create hypercardioid).
4. Monitor for signal integrity and balance.

**Expected Result**: Accurate recombination of dual-output signals, matching expected polar pattern characteristics.

### Automation

**Objective**: Validate automation of polar pattern and frequency band parameters in REAPER.

**Test Steps**:
1. Create automation envelopes for polar pattern changes (e.g., omni to cardioid over 10 seconds).
2. Automate frequency band crossover points and gain.
3. Playback and render the project to verify automation accuracy.

**Expected Result**: Automation executes smoothly, with parameters updating in real-time and rendering correctly.

### Preset Management

**Objective**: Test saving, loading, and recalling PolarDesigner presets.

**Test Steps**:
1. Create a custom preset with specific polar patterns and frequency bands.
2. Save the preset in PolarDesigner.
3. Close and reopen the project, then load the preset.
4. Compare settings and audio output.

**Expected Result**: Presets save and load correctly, retaining all settings.

### Stability and Performance

**Objective**: Assess plugin stability under various conditions.

**Test Steps**:
1. Load PolarDesigner on multiple tracks (e.g., 10 tracks) with complex settings.
2. Monitor CPU usage and latency in REAPER’s Performance Meter.
3. Perform rapid parameter changes and check for crashes or glitches.
4. Render the project and verify output integrity.

**Expected Result**: Stable operation, low CPU usage, and no crashes or audio dropouts.

### Rendering and Export

**Objective**: Ensure PolarDesigner’s effects are accurately rendered in exported audio.

**Test Steps**:
1. Render the project with PolarDesigner applied to a .wav file.
2. Compare rendered audio with real-time playback.
3. Export stems with PolarDesigner effects baked in.

**Expected Result**: Rendered audio matches real-time playback, with no loss of quality.

## Test Data

### Dummy .wav Files

- **Front_Diaphragm.wav**: Mono, 1-minute recording of a vocal or instrument (e.g., sine wave or white noise for testing).
- **Rear_Diaphragm.wav**: Mono, 1-minute recording, slightly different in phase or amplitude to simulate OC818’s rear diaphragm.
- **Stereo_Source.wav**: Stereo file for testing non-dual-output scenarios.

### Creation

1. Use REAPER’s “Insert > Media File” to generate a sine wave (440 Hz) or white noise for 60 seconds.
2. Export as Front_Diaphragm.wav.
3. Duplicate and apply a slight phase shift (e.g., 180 degrees) or amplitude reduction (-3 dB) using REAPER’s FX (e.g., ReaEQ or JS:PhaseRotator), then export as Rear_Diaphragm.wav.
4. Combine both into a stereo file for Stereo_Source.wav using REAPER’s rendering tools.

## Checklist for Setup

- **Install REAPER**:
  - Download and install the latest version of REAPER from [www.reaper.fm](https://www.reaper.fm).
  - Ensure REAPER is associated with .RPP files during installation.
- **Install PolarDesigner**:
  - Download PolarDesigner (VST3, AU, or AAX) from [austrian.audio](https://austrian.audio).
  - Install the plugin and verify it appears in REAPER’s FX browser (VST3 folder).
- **Create Dummy .wav Files**:
  - Open REAPER and create a new project.
  - Go to Insert > Media File and generate a 60-second sine wave (440 Hz) or white noise.
  - Export as Front_Diaphragm.wav (File > Render, select WAV, 48 kHz, 24-bit).
  - Duplicate the track, apply a phase shift (e.g., JS:PhaseRotator) or amplitude reduction (-3 dB), and export as Rear_Diaphragm.wav.
  - Combine both into a stereo file and export as Stereo_Source.wav.
  - Configure the Reaper Project to place all .wav files in the same directory as the .RPP file.
- **Create the Reaper Project (.RPP) File**:
  - Create a new .RPP (Reaper Project).
  - Save as PolarDesigner_Test.rpp in the same directory as the .wav files.
  - Open in REAPER to verify tracks and files load correctly.
- **Configure Tracks**:
  - Open PolarDesigner_Test.rpp in REAPER.
  - Ensure the “Dual Output Test” track has two mono items (Front_Diaphragm.wav, Rear_Diaphragm.wav) routed to a stereo track with PolarDesigner.
  - Verify the “Stereo Source Test” track has Stereo_Source.wav with PolarDesigner applied.
  - Check the “Automation Test” track for an automation envelope on the polar pattern parameter.
- **Run Test Scenarios**:
  - Follow the test steps outlined in the “Test Scenarios” section.
  - Use REAPER’s monitoring (solo tracks, adjust levels) to evaluate audio output.
  - Record or render outputs for comparison (File > Render).
- **Monitor Performance**:
  - Open REAPER’s Performance Meter (View > Performance Meter) to monitor CPU usage.
  - Check for latency or glitches during playback and rendering.
- **Document Results**:
  - Note any discrepancies, crashes, or unexpected behaviors.
  - Save rendered audio files and screenshots of PolarDesigner settings for reference.

## Assumptions and Limitations

- The test assumes access to PolarDesigner and an OC818 microphone or simulated dual-output .wav files. If no OC818 is available, dummy files simulate the dual-output mode.
- Automation parameter names (e.g., “PolarPattern”) are placeholders; actual names must be confirmed in REAPER’s automation lane.
- The Reaper Project file is basic and may need adjustments for specific PolarDesigner parameters or additional test cases.
- macOS users running M1/M2 systems should use Rosetta for PolarDesigner unless a native version is available.

## Deliverables

- **PolarDesigner_Test.rpp**: REAPER project file for testing.
- **Front_Diaphragm.wav, Rear_Diaphragm.wav, Stereo_Source.wav**: Dummy audio files.
- **Test results document**: Summary of test outcomes, including audio renders and screenshots.
- **Setup checklist**: As provided above.

## Additional Notes

- **Backup and Version Control**: Save the .RPP file frequently to avoid data loss. REAPER automatically creates .RPP-bak files for recovery.
- **Plugin Updates**: Ensure PolarDesigner is the latest version, as Austrian Audio may release updates with new features.
- **Community Resources**: Check the REAPER subreddit or Cockos forums for additional tips on plugin integration.

This test plan provides a structured approach to validate the PolarDesigner plugin in REAPER, with a practical .RPP file and setup instructions. 
