# Microsoft Developer Studio Project File - Name="FileManager" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=FileManager - Win32 Release
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "FileManager.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "FileManager.mak" CFG="FileManager - Win32 Release"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "FileManager - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "FileManager - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FileManager - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "include" /I "." /I "../fltk37" /I "../fltk37/zlib" /I "../fltk37/png" /I "../fltk37/jpeg" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x807 /d "NDEBUG"
# ADD RSC /l 0x807 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ../fltk37/lib/fltk.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib ole32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd" /out:"Release/FileMerge.exe" /pdbtype:sept /libpath:"..\..\lib"
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "FileManager - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Win32_Debug"
# PROP BASE Intermediate_Dir "Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Win32_Debug"
# PROP Intermediate_Dir "Win32_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../include" /I "../../fltk37" /I "../../fltk37/zlib" /I "../../fltk37/png" /I "../../fltk37/jpeg" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x807 /d "_DEBUG"
# ADD RSC /l 0x807 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ../../fltk37/lib/fltkd.lib ../w32libs/sqlite3.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd" /pdbtype:sept /libpath:"..\..\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "FileManager - Win32 Release"
# Name "FileManager - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\BastelUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\src\CToolBox.cpp
# End Source File
# Begin Source File

SOURCE=..\src\FileArchivDB.cpp
# End Source File
# Begin Source File

SOURCE=..\src\FileFolder.cpp
# End Source File
# Begin Source File

SOURCE=..\src\FileMerge.cpp
# End Source File
# Begin Source File

SOURCE=..\src\FileTable.cpp
# End Source File
# Begin Source File

SOURCE=..\src\FltkDialogBox.cpp
# End Source File
# Begin Source File

SOURCE=..\src\FltkWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ProgressStatus.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Resource.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TreeView.cpp
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\basteltype.h
# End Source File
# Begin Source File

SOURCE=..\include\BastelUtils.h
# End Source File
# Begin Source File

SOURCE=..\include\CToolBox.h
# End Source File
# Begin Source File

SOURCE=..\include\FileArchivDB.h
# End Source File
# Begin Source File

SOURCE=..\include\FileFolder.h
# End Source File
# Begin Source File

SOURCE=..\include\FileMerge.h
# End Source File
# Begin Source File

SOURCE=..\include\FileTable.h
# End Source File
# Begin Source File

SOURCE=..\include\FltkDialogBox.h
# End Source File
# Begin Source File

SOURCE=..\include\FltkWindow.h
# End Source File
# Begin Source File

SOURCE=..\include\ProgressStatus.h
# End Source File
# Begin Source File

SOURCE=..\include\Resource.h
# End Source File
# Begin Source File

SOURCE=..\include\TreeView.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
