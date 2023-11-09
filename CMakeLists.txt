## Copyright 2022 Intel Corporation
## SPDX-License-Identifier: Apache-2.0

## Global CMake options ##

cmake_minimum_required(VERSION 3.1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(PragueSkyModel)

# Windows
if(WIN32)

# Command-line version

set(PRAGUESKYMODEL_SRC
    thirdparty/miniz/miniz.c
    src/PragueSkyModel.cpp
    src/PragueSkyModelTestCli.cpp
)

add_executable(PragueSkyModelCli ${PRAGUESKYMODEL_SRC})

target_include_directories(PragueSkyModelCli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
target_include_directories(PragueSkyModelCli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/miniz)
target_include_directories(PragueSkyModelCli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/tinyexr)

# GUI version

find_library(D3D11_LIBRARY d3d11.lib)

set(PRAGUESKYMODELGUI_SRC
    thirdparty/miniz/miniz.c
    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_tables.cpp
    thirdparty/imgui/imgui_widgets.cpp
    thirdparty/imgui/backends/imgui_impl_dx11.cpp
    thirdparty/imgui/backends/imgui_impl_win32.cpp
    src/PragueSkyModel.cpp
    src/PragueSkyModelTestGui.cpp
)

add_executable(PragueSkyModelGui ${PRAGUESKYMODELGUI_SRC})

target_include_directories(PragueSkyModelGui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
target_include_directories(PragueSkyModelGui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/miniz)
target_include_directories(PragueSkyModelGui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui)
target_include_directories(PragueSkyModelGui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/backends)
target_include_directories(PragueSkyModelGui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/tinyexr)

target_link_libraries(PragueSkyModelGui PRIVATE d3d11.lib)

# Linux
else()

#set(CMAKE_CXX_FLAGS "-DNDEBUG")

# Command-line version

set(OpenGL_GL_PREFERENCE "GLVND")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL3_PROTOTYPES -DGL_GLEXT_PROTOTYPES")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__GLN__")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMORPH_FONTS_DIR=\"\\\"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/morphologica/fonts\\\"\"")
set(MORPH_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/morphologica" CACHE PATH "The path to morphologica")
set(MORPH_INC_CORE ${HDF5_INCLUDE_DIR})
set(MORPH_INC_GL ${OPENGL_INCLUDE_DIR} ${GLFW3_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS})

find_package(TBB REQUIRED)
find_package(HDF5 REQUIRED)
find_package(Freetype REQUIRED) 
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

set(PRAGUESKYMODEL_SRC
    thirdparty/miniz/miniz.c
    src/PragueSkyModel.cpp
    src/PragueSkyModelTestCli.cpp
)

add_executable(PragueSkyModelCli ${PRAGUESKYMODEL_SRC})
add_executable(VisualiseSkyData src/VisualiseSkyData.cpp)

target_include_directories(PragueSkyModelCli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
target_include_directories(PragueSkyModelCli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/miniz)
target_include_directories(PragueSkyModelCli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/tinyexr)
target_include_directories(PragueSkyModelCli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/morphologica)
target_include_directories(PragueSkyModelCli PRIVATE ${HDF5_INCLUDE_DIR})

target_include_directories(VisualiseSkyData PRIVATE ${MORPH_INCLUDE_PATH})
target_include_directories(VisualiseSkyData PRIVATE ${MORPH_INCLUDE_PATH}/include)
target_include_directories(VisualiseSkyData PRIVATE ${MORPH_INC_CORE})
target_include_directories(VisualiseSkyData PRIVATE ${MORPH_INC_GL})

target_link_libraries(PragueSkyModelCli PRIVATE TBB::tbb ${HDF5_C_LIBRARIES})
target_link_libraries(VisualiseSkyData OpenGL::GL glfw Freetype::Freetype)

# GUI version

set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(HDF5 REQUIRED)

set(PRAGUESKYMODELGUI_SRC
    thirdparty/miniz/miniz.c
    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_tables.cpp
    thirdparty/imgui/imgui_widgets.cpp
    thirdparty/imgui/backends/imgui_impl_sdl.cpp
    thirdparty/imgui/backends/imgui_impl_opengl2.cpp
    src/PragueSkyModel.cpp
    src/PragueSkyModelTestGui.cpp
)

add_executable(PragueSkyModelGui ${PRAGUESKYMODELGUI_SRC})

target_include_directories(PragueSkyModelGui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
target_include_directories(PragueSkyModelGui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/miniz)
target_include_directories(PragueSkyModelGui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui)
target_include_directories(PragueSkyModelGui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/backends)
target_include_directories(PragueSkyModelGui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/tinyexr)
target_include_directories(PragueSkyModelGui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/morphologica)
target_include_directories(PragueSkyModelGui PRIVATE ${OPENGL_INCLUDE_DIRS})
target_include_directories(PragueSkyModelGui PRIVATE ${SDL2_INCLUDE_DIRS})
target_include_directories(PragueSkyModelGui PRIVATE ${HDF5_INCLUDE_DIR})

target_link_libraries(PragueSkyModelGui PRIVATE ${OPENGL_LIBRARIES})
target_link_libraries(PragueSkyModelGui PRIVATE ${SDL2_LIBRARIES})
target_link_libraries(PragueSkyModelGui PRIVATE TBB::tbb)
target_link_libraries(PragueSkyModelGui PRIVATE ${CMAKE_DL_LIBS})
target_link_libraries(PragueSkyModelGui PRIVATE TBB::tbb ${HDF5_C_LIBRARIES})

endif()
