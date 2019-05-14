cmake_minimum_required(VERSION 3.8)
project (Bz)

add_definitions(-D_UNICODE -DUNICODE)

#find_package(ZLIB REQUIRED)

#set(rc_flags "/l 0x411")
#set(CMAKE_RC_FLAGS ${rc_flags})

add_executable(Bz WIN32 
  BZ.cpp
  BZ.rc
  BZAnalyzerView.cpp
  BZBmpVw2.cpp
  BZCoreData.cpp
  BZDoc2.cpp
  BZDpi.cpp
  BZFormVw.cpp
  BZInspectView.cpp
  BZOption.cpp
  BZView.cpp
  ColorCombo.cpp
  ColorDlg.cpp
  hsv.c
  MainFrm.cpp
  SettingDlg.cpp
  StdAfx.cpp
  tamasplit.cpp
  TextView.cpp
)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /OPT:REF /OPT:ICF /LTCG /LARGEADDRESSAWARE")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /Os /GF /GL")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

target_link_libraries(Bz imagehlp.lib Winspool.lib)

if(CMAKE_CL_64)
  target_link_libraries(Bz ${CMAKE_SOURCE_DIR}/zlib128_64.lib)
  set_target_properties(Bz PROPERTIES OUTPUT_NAME "Bz64")
else()
  target_link_libraries(Bz ${CMAKE_SOURCE_DIR}/zlib128.lib)
endif()


#set(rc_flags "/l 0x409")
#set(CMAKE_RC_FLAGS ${rc_flags})

set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /NOENTRY")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /NOENTRY")

add_library(BZres_us SHARED
  BZres_us.rc
)

if(CMAKE_CL_64)
  set_target_properties(BZres_us PROPERTIES OUTPUT_NAME "BZres_us64")
endif()

add_dependencies(BZres_us Bz)

