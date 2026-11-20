#ifndef PluginBundle
  #error Pass the path of the compiled VST3 bundle using /DPluginBundle.
#endif
#ifndef PluginArch
  #error Pass the target architecture using /DPluginArch.
#endif
#if PluginArch == "ARM64"
  #define WindowsArch "arm64"
#elif PluginArch == "x64"
  #define WindowsArch "x64os"
#else
  #error Unsupported target architecture.
#endif

[Setup]
AppId=audio.gett1.dance
AppName=gett1 Dance
AppVersion=0.4.1
AppPublisher=gett1
PrivilegesRequired=admin
ArchitecturesAllowed={#WindowsArch}
ArchitecturesInstallIn64BitMode={#WindowsArch}
DefaultDirName={commoncf64}\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
UsePreviousAppDir=no
UninstallFilesDir={commonappdata}\gett1\Dance
OutputBaseFilename=Gett1Dance-0.4.1-Windows-{#PluginArch}-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern

[Files]
Source: "{#PluginBundle}\*"; DestDir: "{app}\gett1 Dance.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
