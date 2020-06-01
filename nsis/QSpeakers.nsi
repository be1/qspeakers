;NSIS QSpeakers installer
;Based on example scripts Written by Joost Verburg
;Written by Benoît Rouits

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

  Unicode true

;--------------------------------
;Init checks

Function .onInit
  Push $R0
  ReadRegStr $R0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\QSpeakers" "UninstallString"
  StrCmp $R0 "" QSpeakers_NOT_PRESENT
  MessageBox MB_OK $(DESC_SecInit)
  Pop $R0
  Abort
  QSpeakers_NOT_PRESENT:
  Pop $R0
FunctionEnd

;General

  ;Name and file
  Name "QSpeakers"
  OutFile "qspeakers_installer.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\QSpeakers"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\QSpeakers" "Install_Dir"

  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin

;--------------------------------
;Interface Settings

  !define MUI_ICON "qspeakers_install.ico"
  !define MUI_UNICON "qspeakers_uninstall.ico"
  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\win.bmp"
  !define MUI_ABORTWARNING

;--------------------------------
;Pages

;  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "../COPYING"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
;  !insertmacro MUI_PAGE_FINISH
  
;  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
;  !insertmacro MUI_UNPAGE_FINISH
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "French"

;--------------------------------
;Installer Sections

Section "Install" SecInstall

  SetOutPath "$INSTDIR"
  
  ; Add program, db and translations
  File /r ../release/qspeakers.exe
  File ../qspeakers.png
  File ../qspeakers_db.xml
  File /r ../locale
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\QSpeakers "Install_Dir" "$INSTDIR"

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\QSpeakers" "DisplayName" "QSpeakers"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\QSpeakers" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\QSpeakers" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\QSpeakers" "NoRepair" 1

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"

SectionEnd

Section "Start Menu Shortcuts" SecShortcut

  CreateDirectory "$SMPROGRAMS\QSpeakers"
  CreateShortcut "$SMPROGRAMS\QSpeakers\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortcut "$SMPROGRAMS\QSpeakers\QSpeakers.lnk" "$INSTDIR\qspeakers.exe" "" "$INSTDIR\qspeakers.exe" 0

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecInit ${LANG_ENGLISH} "QSpeakers is already installed. \
		  Please uninstall it before to run this installer."
  LangString DESC_SecInstall ${LANG_ENGLISH} "Install QSpeakers software."
  LangString DESC_SecShortcut ${LANG_ENGLISH} "Install launcher in menu."

  LangString DESC_SecInit ${LANG_FRENCH} "QSpeakers est déjà installé. \
		  Veuillez le désinstaller avant d'utiliser cet installateur."
  LangString DESC_SecInstall ${LANG_FRENCH} "Installe le logiciel QSpeakers."
  LangString DESC_SecShortcut ${LANG_FRENCH} "Installe le lanceur dans le menu."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecInstall} $(DESC_SecInstall)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecShortcut} $(DESC_SecShortcut)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ; Remove db, translations and program
  Delete "$INSTDIR\qspeakers_db.xml"
  Delete "$INSTDIR\locale\*.*"
  Delete "$INSTDIR\qspeakers.png"
  Delete "$INSTDIR\qspeakers.exe"

  ; Remove translation diretory
  RMDir	"$INSTDIR\locale"

  ; remove uninstaller
  Delete "$INSTDIR\uninstall.exe"

  ; Remove used directory
  RMDir "$INSTDIR"

  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\QSpeakers"
  DeleteRegKey HKLM SOFTWARE\QSpeakers

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\QSpeakers\*.*"
  ; Remove directory used
  RMDir "$SMPROGRAMS\QSpeakers"

SectionEnd
