cmake_minimum_required( VERSION 2.8.11 )
project( CG_Engine )

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-O3")

add_subdirectory( CG_Engine )

find_package( glfw3 3.2 REQUIRED )
find_package( OpenGL REQUIRED )
find_package( assimp REQUIRED )
find_package( glm REQUIRED )

if (assimp_FOUND)
    set(ASSIMP_LIBRARY "assimp")
#    add_library(${ASSIMP_LIBRARY} SHARED IMPORTED)
#    set_target_properties(${ASSIMP_LIBRARY} PROPERTIES IMPORTED_LOCATION "${ASSIMP_LIBRARY_DIRS}/libassimp.so")
endif(assimp_FOUND)

include_directories( "$(PROJECT_SOURCE_DIR)/CG_Implementation" )
include_directories( "$(PROJECT_SOURCE_DIR)/CG_Engine" )
include_directories(  "CG_Engine/include"  )
add_executable( cg_ex CG_Implementation/main.cpp CG_Implementation/CG_Implementation.cpp )
target_link_libraries ( cg_ex CG_Engine glfw assimp glm )
