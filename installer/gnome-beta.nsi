; Self-extracting NSIS container for the GNOME Libadwaita Installer
Unicode true

!define APP_NAME "GNOME Web"
!ifndef VERSION
  !define VERSION "51.2.0"
!endif

Name "${APP_NAME}"
OutFile "..\dist\GNOME-Beta-Setup-${VERSION}.exe"
RequestExecutionLevel user
SilentInstall silent

Section "MainSection" SecMain
  InitPluginsDir
  SetOutPath "$PLUGINSDIR"
  File /r "..\dist\app\*.*"
  ExecWait '"$PLUGINSDIR\gnome-web-installer.exe" --source "$PLUGINSDIR"'
SectionEnd
