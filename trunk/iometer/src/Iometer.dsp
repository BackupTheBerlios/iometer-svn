# Microsoft Developer Studio Project File - Name="Iometer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Iometer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Iometer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Iometer.mak" CFG="Iometer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Iometer - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Iometer - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Iometer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_WINDOWS" /D "IOMTR_OSFAMILY_WINDOWS" /D "IOMTR_OS_WIN32" /D "IOMTR_CPU_I386" /D "LITTLE_ENDIAN_ARCH" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "_WINDOWS" /D "IOMTR_OSFAMILY_WINDOWS" /D "IOMTR_OS_WIN32" /D "IOMTR_CPU_I386" /D "LITTLE_ENDIAN_ARCH" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /mktyplib203 /win32
# ADD MTL /nologo /mktyplib203 /win32
# ADD BASE RSC /l 0x409 
# ADD RSC /l 0x409 /d "_GALILEO_"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 Ws2_32.lib Mswsock.lib Version.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"msvcrt.lib"
# SUBTRACT LINK32 /incremental:yes /nodefaultlib

!ELSEIF  "$(CFG)" == "Iometer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "IOMTR_OSFAMILY_WINDOWS" /D "IOMTR_OS_WIN32" /D "IOMTR_CPU_I386" /D "LITTLE_ENDIAN_ARCH" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "_DEBUG" /D "_GALILEO_" /D "WIN32" /D "_WINDOWS" /D "IOMTR_OSFAMILY_WINDOWS" /D "IOMTR_OS_WIN32" /D "IOMTR_CPU_I386" /D "LITTLE_ENDIAN_ARCH" /D "_MBCS" /D "_DETAILS" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_GALILEO_"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Ws2_32.lib Mswsock.lib Version.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /incremental:no /nodefaultlib

!ENDIF 

# Begin Target

# Name "Iometer - Win32 Release"
# Name "Iometer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AccessDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\AccessSpecList.cpp
# End Source File
# Begin Source File

SOURCE=.\BigMeter.cpp
# End Source File
# Begin Source File

SOURCE=.\GalileoApp.cpp
# End Source File
# Begin Source File

SOURCE=.\GalileoCmdLine.cpp
# End Source File
# Begin Source File

SOURCE=.\GalileoDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\GalileoGlobals.cpp
# End Source File
# Begin Source File

SOURCE=.\GalileoView.cpp
# End Source File
# Begin Source File

SOURCE=.\ICF_ifstream.cpp
# End Source File
# Begin Source File

SOURCE=.\ICFOpenDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ICFSaveDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\IOAccess.cpp
# End Source File
# Begin Source File

SOURCE=.\IOGlobals.cpp
# End Source File
# Begin Source File

SOURCE=.\IOPort.cpp
# End Source File
# Begin Source File

SOURCE=.\IOPortTCP.cpp
# End Source File
# Begin Source File

SOURCE=.\IOTime.cpp
# End Source File
# Begin Source File

SOURCE=.\LegalBox.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\Manager.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagerList.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagerMap.cpp
# End Source File
# Begin Source File

SOURCE=.\MeterCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\PageAccess.cpp
# End Source File
# Begin Source File

SOURCE=.\PageDisk.cpp
# End Source File
# Begin Source File

SOURCE=.\PageDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\PageNetwork.cpp
# End Source File
# Begin Source File

SOURCE=.\PageSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\TextDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\WaitingForManagers.cpp
# End Source File
# Begin Source File

SOURCE=.\Worker.cpp
# End Source File
# Begin Source File

SOURCE=.\WorkerView.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AccessDialog.h
# End Source File
# Begin Source File

SOURCE=.\AccessSpecList.h
# End Source File
# Begin Source File

SOURCE=.\BigMeter.h
# End Source File
# Begin Source File

SOURCE=.\GalileoApp.h
# End Source File
# Begin Source File

SOURCE=.\GalileoCmdLine.h
# End Source File
# Begin Source File

SOURCE=.\GalileoDefs.h
# End Source File
# Begin Source File

SOURCE=.\GalileoDoc.h
# End Source File
# Begin Source File

SOURCE=.\GalileoView.h
# End Source File
# Begin Source File

SOURCE=.\ICF_ifstream.h
# End Source File
# Begin Source File

SOURCE=.\ICFOpenDialog.h
# End Source File
# Begin Source File

SOURCE=.\ICFSaveDialog.h
# End Source File
# Begin Source File

SOURCE=.\IOAccess.h
# End Source File
# Begin Source File

SOURCE=.\IOCommon.h
# End Source File
# Begin Source File

SOURCE=.\IOPort.h
# End Source File
# Begin Source File

SOURCE=.\IOPortTCP.h
# End Source File
# Begin Source File

SOURCE=.\IOSTREAM
# End Source File
# Begin Source File

SOURCE=.\IOTest.h
# End Source File
# Begin Source File

SOURCE=.\IOTransfers.h
# End Source File
# Begin Source File

SOURCE=.\IOVersion.h
# End Source File
# Begin Source File

SOURCE=.\ISTREAM
# End Source File
# Begin Source File

SOURCE=.\LegalBox.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\Manager.h
# End Source File
# Begin Source File

SOURCE=.\ManagerList.h
# End Source File
# Begin Source File

SOURCE=.\ManagerMap.h
# End Source File
# Begin Source File

SOURCE=.\MeterCtrl.h
# End Source File
# Begin Source File

SOURCE=.\OSTREAM
# End Source File
# Begin Source File

SOURCE=.\PageAccess.h
# End Source File
# Begin Source File

SOURCE=.\PageDisk.h
# End Source File
# Begin Source File

SOURCE=.\PageDisplay.h
# End Source File
# Begin Source File

SOURCE=.\PageNetwork.h
# End Source File
# Begin Source File

SOURCE=.\PageSetup.h
# End Source File
# Begin Source File

SOURCE=.\PageTarget.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TextDisplay.h
# End Source File
# Begin Source File

SOURCE=.\vipl.h
# End Source File
# Begin Source File

SOURCE=.\WaitingForManagers.h
# End Source File
# Begin Source File

SOURCE=.\Worker.h
# End Source File
# Begin Source File

SOURCE=.\WorkerView.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\access.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Galileo.ico
# End Source File
# Begin Source File

SOURCE=.\Galileo.rc
# End Source File
# Begin Source File

SOURCE=.\res\GalileoDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Iometer.rc2
# End Source File
# Begin Source File

SOURCE=.\res\LED.bmp
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\res\ticons.bmp
# End Source File
# Begin Source File

SOURCE=.\res\wicons.bmp
# End Source File
# End Group
# End Target
# End Project
