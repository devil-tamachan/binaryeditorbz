# Microsoft Developer Studio Generated NMAKE File, Format Version 4.10
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=BZ - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to BZ - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "BZ - Win32 Release" && "$(CFG)" != "BZ - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Bz.mak" CFG="BZ - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "BZ - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "BZ - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "BZ - Win32 Debug"
RSC=rc.exe
MTL=mktyplib.exe
CPP=cl.exe

!IF  "$(CFG)" == "BZ - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\Bz.exe"

CLEAN : 
	-@erase "$(INTDIR)\BZ.obj"
	-@erase "$(INTDIR)\Bz.pch"
	-@erase "$(INTDIR)\BZ.res"
	-@erase "$(INTDIR)\BZBmpVw.obj"
	-@erase "$(INTDIR)\BZDoc.obj"
	-@erase "$(INTDIR)\BZFormVw.obj"
	-@erase "$(INTDIR)\BZView.obj"
	-@erase "$(INTDIR)\ComboBar.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\Splitter.obj"
	-@erase "$(INTDIR)\StatBar.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\TextView.obj"
	-@erase "$(OUTDIR)\Bz.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MD /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/Bz.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x411 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x411 /d "NDEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x411 /fo"$(INTDIR)/BZ.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Bz.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /map
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)/Bz.pdb"\
 /machine:I386 /out:"$(OUTDIR)/Bz.exe" 
LINK32_OBJS= \
	"$(INTDIR)\BZ.obj" \
	"$(INTDIR)\BZ.res" \
	"$(INTDIR)\BZBmpVw.obj" \
	"$(INTDIR)\BZDoc.obj" \
	"$(INTDIR)\BZFormVw.obj" \
	"$(INTDIR)\BZView.obj" \
	"$(INTDIR)\ComboBar.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\Splitter.obj" \
	"$(INTDIR)\StatBar.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TextView.obj"

"$(OUTDIR)\Bz.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "BZ - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\Bz.exe" "$(OUTDIR)\Bz.bsc"

CLEAN : 
	-@erase "$(INTDIR)\BZ.obj"
	-@erase "$(INTDIR)\Bz.pch"
	-@erase "$(INTDIR)\BZ.res"
	-@erase "$(INTDIR)\BZ.sbr"
	-@erase "$(INTDIR)\BZBmpVw.obj"
	-@erase "$(INTDIR)\BZBmpVw.sbr"
	-@erase "$(INTDIR)\BZDoc.obj"
	-@erase "$(INTDIR)\BZDoc.sbr"
	-@erase "$(INTDIR)\BZFormVw.obj"
	-@erase "$(INTDIR)\BZFormVw.sbr"
	-@erase "$(INTDIR)\BZView.obj"
	-@erase "$(INTDIR)\BZView.sbr"
	-@erase "$(INTDIR)\ComboBar.obj"
	-@erase "$(INTDIR)\ComboBar.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\Splitter.obj"
	-@erase "$(INTDIR)\Splitter.sbr"
	-@erase "$(INTDIR)\StatBar.obj"
	-@erase "$(INTDIR)\StatBar.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\TextView.obj"
	-@erase "$(INTDIR)\TextView.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\Bz.bsc"
	-@erase "$(OUTDIR)\Bz.exe"
	-@erase "$(OUTDIR)\Bz.ilk"
	-@erase "$(OUTDIR)\Bz.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GX /Zi /Od /Gf /Gy /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W3 /Gm /Gi /GX /Zi /Od /Gf /Gy /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)/" /Fp"$(INTDIR)/Bz.pch"\
 /Yu"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x411 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x411 /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x411 /fo"$(INTDIR)/BZ.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Bz.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\BZ.sbr" \
	"$(INTDIR)\BZBmpVw.sbr" \
	"$(INTDIR)\BZDoc.sbr" \
	"$(INTDIR)\BZFormVw.sbr" \
	"$(INTDIR)\BZView.sbr" \
	"$(INTDIR)\ComboBar.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\Splitter.sbr" \
	"$(INTDIR)\StatBar.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\TextView.sbr"

"$(OUTDIR)\Bz.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/Bz.pdb" /debug /machine:I386 /out:"$(OUTDIR)/Bz.exe" 
LINK32_OBJS= \
	"$(INTDIR)\BZ.obj" \
	"$(INTDIR)\BZ.res" \
	"$(INTDIR)\BZBmpVw.obj" \
	"$(INTDIR)\BZDoc.obj" \
	"$(INTDIR)\BZFormVw.obj" \
	"$(INTDIR)\BZView.obj" \
	"$(INTDIR)\ComboBar.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\Splitter.obj" \
	"$(INTDIR)\StatBar.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TextView.obj"

"$(OUTDIR)\Bz.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "BZ - Win32 Release"
# Name "BZ - Win32 Debug"

!IF  "$(CFG)" == "BZ - Win32 Release"

!ELSEIF  "$(CFG)" == "BZ - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ReadMe.txt

!IF  "$(CFG)" == "BZ - Win32 Release"

!ELSEIF  "$(CFG)" == "BZ - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BZ.cpp

!IF  "$(CFG)" == "BZ - Win32 Release"

DEP_CPP_BZ_CP=\
	"..\cmos.h"\
	".\BZ.h"\
	".\BZDoc.h"\
	".\BZView.h"\
	".\ComboBar.h"\
	".\MainFrm.h"\
	".\Splitter.h"\
	".\StatBar.h"\
	".\StdAfx.h"\
	".\TextView.h"\
	

"$(INTDIR)\BZ.obj" : $(SOURCE) $(DEP_CPP_BZ_CP) "$(INTDIR)" "$(INTDIR)\Bz.pch"


!ELSEIF  "$(CFG)" == "BZ - Win32 Debug"

DEP_CPP_BZ_CP=\
	"..\cmos.h"\
	".\BZ.h"\
	".\BZDoc.h"\
	".\BZView.h"\
	".\ComboBar.h"\
	".\MainFrm.h"\
	".\Splitter.h"\
	".\StatBar.h"\
	".\StdAfx.h"\
	".\TextView.h"\
	

"$(INTDIR)\BZ.obj" : $(SOURCE) $(DEP_CPP_BZ_CP) "$(INTDIR)" "$(INTDIR)\Bz.pch"

"$(INTDIR)\BZ.sbr" : $(SOURCE) $(DEP_CPP_BZ_CP) "$(INTDIR)" "$(INTDIR)\Bz.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	"..\cmos.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "BZ - Win32 Release"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)/Bz.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Bz.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "BZ - Win32 Debug"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /Gi /GX /Zi /Od /Gf /Gy /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)/" /Fp"$(INTDIR)/Bz.pch"\
 /Yc"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\StdAfx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Bz.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MainFrm.cpp
DEP_CPP_MAINF=\
	"..\cmos.h"\
	".\BZ.h"\
	".\BZBmpVw.h"\
	".\BZDoc.h"\
	".\BZFormVw.h"\
	".\BZView.h"\
	".\ComboBar.h"\
	".\MainFrm.h"\
	".\Splitter.h"\
	".\StatBar.h"\
	".\StdAfx.h"\
	".\TextView.h"\
	

!IF  "$(CFG)" == "BZ - Win32 Release"


"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"


!ELSEIF  "$(CFG)" == "BZ - Win32 Debug"


"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"

"$(INTDIR)\MainFrm.sbr" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BZDoc.cpp

!IF  "$(CFG)" == "BZ - Win32 Release"

DEP_CPP_BZDOC=\
	"..\cmos.h"\
	".\BZ.h"\
	".\BZDoc.h"\
	".\ComboBar.h"\
	".\MainFrm.h"\
	".\Splitter.h"\
	".\StatBar.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\BZDoc.obj" : $(SOURCE) $(DEP_CPP_BZDOC) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"


!ELSEIF  "$(CFG)" == "BZ - Win32 Debug"

DEP_CPP_BZDOC=\
	"..\cmos.h"\
	".\BZ.h"\
	".\BZDoc.h"\
	".\ComboBar.h"\
	".\MainFrm.h"\
	".\Splitter.h"\
	".\StatBar.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\BZDoc.obj" : $(SOURCE) $(DEP_CPP_BZDOC) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"

"$(INTDIR)\BZDoc.sbr" : $(SOURCE) $(DEP_CPP_BZDOC) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BZView.cpp
DEP_CPP_BZVIE=\
	"..\cmos.h"\
	".\BZ.h"\
	".\BZDoc.h"\
	".\BZFormVw.h"\
	".\BZView.h"\
	".\ComboBar.h"\
	".\MainFrm.h"\
	".\Splitter.h"\
	".\StatBar.h"\
	".\StdAfx.h"\
	".\TextView.h"\
	

!IF  "$(CFG)" == "BZ - Win32 Release"


"$(INTDIR)\BZView.obj" : $(SOURCE) $(DEP_CPP_BZVIE) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"


!ELSEIF  "$(CFG)" == "BZ - Win32 Debug"


"$(INTDIR)\BZView.obj" : $(SOURCE) $(DEP_CPP_BZVIE) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"

"$(INTDIR)\BZView.sbr" : $(SOURCE) $(DEP_CPP_BZVIE) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BZ.rc
DEP_RSC_BZ_RC=\
	".\res\BZ.ico"\
	".\res\BZ.rc2"\
	".\res\BZDoc.ico"\
	".\res\Toolbar.bmp"\
	

"$(INTDIR)\BZ.res" : $(SOURCE) $(DEP_RSC_BZ_RC) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TextView.cpp

!IF  "$(CFG)" == "BZ - Win32 Release"

DEP_CPP_TEXTV=\
	"..\cmos.h"\
	".\BZ.h"\
	".\StdAfx.h"\
	".\TextView.h"\
	

"$(INTDIR)\TextView.obj" : $(SOURCE) $(DEP_CPP_TEXTV) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"


!ELSEIF  "$(CFG)" == "BZ - Win32 Debug"

DEP_CPP_TEXTV=\
	"..\cmos.h"\
	".\BZ.h"\
	".\StdAfx.h"\
	".\TextView.h"\
	

"$(INTDIR)\TextView.obj" : $(SOURCE) $(DEP_CPP_TEXTV) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"

"$(INTDIR)\TextView.sbr" : $(SOURCE) $(DEP_CPP_TEXTV) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BZBmpVw.cpp
DEP_CPP_BZBMP=\
	"..\cmos.h"\
	".\BZ.h"\
	".\BZBmpVw.h"\
	".\BZDoc.h"\
	".\BZView.h"\
	".\Splitter.h"\
	".\StdAfx.h"\
	".\TextView.h"\
	

!IF  "$(CFG)" == "BZ - Win32 Release"


"$(INTDIR)\BZBmpVw.obj" : $(SOURCE) $(DEP_CPP_BZBMP) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"


!ELSEIF  "$(CFG)" == "BZ - Win32 Debug"


"$(INTDIR)\BZBmpVw.obj" : $(SOURCE) $(DEP_CPP_BZBMP) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"

"$(INTDIR)\BZBmpVw.sbr" : $(SOURCE) $(DEP_CPP_BZBMP) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Splitter.cpp
DEP_CPP_SPLIT=\
	"..\cmos.h"\
	".\Splitter.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "BZ - Win32 Release"


"$(INTDIR)\Splitter.obj" : $(SOURCE) $(DEP_CPP_SPLIT) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"


!ELSEIF  "$(CFG)" == "BZ - Win32 Debug"


"$(INTDIR)\Splitter.obj" : $(SOURCE) $(DEP_CPP_SPLIT) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"

"$(INTDIR)\Splitter.sbr" : $(SOURCE) $(DEP_CPP_SPLIT) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ComboBar.cpp
DEP_CPP_COMBO=\
	"..\cmos.h"\
	".\BZ.h"\
	".\ComboBar.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "BZ - Win32 Release"


"$(INTDIR)\ComboBar.obj" : $(SOURCE) $(DEP_CPP_COMBO) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"


!ELSEIF  "$(CFG)" == "BZ - Win32 Debug"


"$(INTDIR)\ComboBar.obj" : $(SOURCE) $(DEP_CPP_COMBO) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"

"$(INTDIR)\ComboBar.sbr" : $(SOURCE) $(DEP_CPP_COMBO) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BZFormVw.cpp
DEP_CPP_BZFOR=\
	"..\cmos.h"\
	".\BZ.h"\
	".\BZFormVw.h"\
	".\BZView.h"\
	".\StdAfx.h"\
	".\TextView.h"\
	

!IF  "$(CFG)" == "BZ - Win32 Release"


"$(INTDIR)\BZFormVw.obj" : $(SOURCE) $(DEP_CPP_BZFOR) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"


!ELSEIF  "$(CFG)" == "BZ - Win32 Debug"


"$(INTDIR)\BZFormVw.obj" : $(SOURCE) $(DEP_CPP_BZFOR) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"

"$(INTDIR)\BZFormVw.sbr" : $(SOURCE) $(DEP_CPP_BZFOR) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\StatBar.cpp
DEP_CPP_STATB=\
	"..\cmos.h"\
	".\BZ.h"\
	".\StatBar.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "BZ - Win32 Release"


"$(INTDIR)\StatBar.obj" : $(SOURCE) $(DEP_CPP_STATB) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"


!ELSEIF  "$(CFG)" == "BZ - Win32 Debug"


"$(INTDIR)\StatBar.obj" : $(SOURCE) $(DEP_CPP_STATB) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"

"$(INTDIR)\StatBar.sbr" : $(SOURCE) $(DEP_CPP_STATB) "$(INTDIR)"\
 "$(INTDIR)\Bz.pch"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
