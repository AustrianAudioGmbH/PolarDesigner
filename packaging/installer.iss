#define Version Trim(FileRead(FileOpen("..\VERSION")))
#define ProductName 'PolarDesigner3'
#define Publisher 'AustrianAudio'
#define Year GetDateTimeString("yyyy","","")
#define PD_BUILD_DIR GetEnv('PD_SHARED_BUILD_DIR')
#define PD_BUILD_ARCHIVE GetEnv('PD_BUILD_ARCHIVE')
#define PD_BUILD_TYPE GetEnv('PD_BUILD_TYPE')

[Setup]
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
AppName={#ProductName}
OutputBaseFilename="{#ProductName}_{#GetEnv('PD_BUILD_MARK')}_Installer"
AppCopyright=Copyright (C) {#Year} {#Publisher}
AppPublisher={#Publisher}
AppVersion={#Version}
DefaultDirName="{commoncf64}\VST3\{#ProductName}.vst3"
DisableDirPage=yes
OutputDir={#PD_BUILD_ARCHIVE}

; MAKE SURE YOU READ THE FOLLOWING!
LicenseFile="EULA"
UninstallFilesDir="{commonappdata}\{#ProductName}\uninstall"

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#ProductName}Data"

; MSVC adds a .ilk when building the plugin. Let's not include that.
[Files]
Source: "{#PD_BUILD_DIR}\PolarDesigner_artefacts\{#PD_BUILD_TYPE}\VST3\{#ProductName}.vst3\*"; DestDir: "{commoncf64}\VST3\{#ProductName}.vst3\"; Excludes: *.ilk; Flags: ignoreversion recursesubdirs;

[Run]
Filename: "{cmd}"; \
    WorkingDir: "{commoncf64}\VST3"; \
    Parameters: "/C mklink /D ""{commoncf64}\VST3\{#ProductName}Data"" ""{commonappdata}\{#ProductName}"""; \
    Flags: runascurrentuser;
