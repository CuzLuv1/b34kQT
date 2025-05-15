cmake_minimum_required(VERSION 3.30)
project(b34k LANGUAGES C CXX)  # 启用C和C++
set(Python_EXECUTABLE "d:/Code/anaconda/envs/vir/python.exe" CACHE FILEPATH "Path to Python executable")
message(STATUS "Found Python Version: ${Python_VERSION_STRING}") # 确认找到的是 Anaconda 的 Python
set(pybind11_DIR "d:/Code/anaconda/envs/vir/Lib/site-packages/pybind11/share/cmake/pybind11" CACHE PATH "Path to pybind11 CMake config directory")
message(STATUS "pybind11_DIR is set to: ${pybind11_DIR}") # 加入这行

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "d:/Code/QT/6.8.1/mingw_64")
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)  # 统一C标准设置
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)

find_package(pybind11 REQUIRED)
find_package(Python COMPONENTS Interpreter Development REQUIRED)
message(STATUS "Found pybind11 version ${pybind11_VERSION} from pip install")
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Charts Svg)
qt_standard_project_setup()

# 编码设置（MSVC专用）
add_compile_options(
    "$<$<C_COMPILER_ID:MSVC>:/utf-8>"
    "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>"
)

# 路径设置
set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(SOURCE_MODULE_DIR ${SOURCE_DIR}/module)  # 检查拼写是否为"module"
set(UI_DIR ${PROJECT_SOURCE_DIR}/UI)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
include_directories(${UI_DIR} ${SOURCE_DIR} ${INCLUDE_DIR} ${SOURCE_MODULE_DIR})

# 收集源文件
file(GLOB PROJECT_SOURCES
    ${SOURCE_DIR}/*.cpp
    ${SOURCE_MODULE_DIR}/*.cpp
    ${PROJECT_SOURCE_DIR}/*.cpp
    ${UI_DIR}/*.cpp  # 使用正确的UI_DIR
)

file(GLOB INCLUDE_SOURCES 
    ${INCLUDE_DIR}/*.h
    "rtklib/*.h"
)

list(APPEND CMAKE_AUTOUIC_SEARCH_PATHS ${UI_DIR})
list(APPEND PROJECT_SOURCES ${INCLUDE_SOURCES})

# RTKLIB静态库
file(GLOB RTKLIB_SOURCES ${PROJECT_SOURCE_DIR}/rtklib/*.c)
add_library(rtklib STATIC ${RTKLIB_SOURCES})
target_include_directories(rtklib PUBLIC "${PROJECT_SOURCE_DIR}/rtklib")

# 资源文件
qt_add_resources(RESOURCE_SOURCES Resource/resource.qrc)

# 可执行文件
add_definitions(-DTRACE -DWIN32 -DWIN_DLL) 
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${RESOURCE_SOURCES})

# 设置Python脚本的源文件路径
set(PYTHON_SCRIPT_NAME "plotter.py") # <--- 如果您的脚本名不同，请修改这里
set(PYTHON_SCRIPT_SOURCE "${SOURCE_DIR}/${PYTHON_SCRIPT_NAME}")

# 检查Python脚本源文件是否存在
if(NOT EXISTS "${PYTHON_SCRIPT_SOURCE}")
    message(FATAL_ERROR "Python script not found at ${PYTHON_SCRIPT_SOURCE}")
else()
    message(STATUS "Python script found at: ${PYTHON_SCRIPT_SOURCE}")
endif()

# 将Python脚本复制到可执行文件的输出目录
# $<TARGET_FILE_DIR:${PROJECT_NAME}> 是一个生成器表达式，
# 它会在构建时解析为可执行文件所在的目录。
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD  # 在目标 ${PROJECT_NAME} 构建完成后执行
    COMMAND ${CMAKE_COMMAND} -E copy_if_different # 仅当源文件和目标文件不同或目标文件不存在时才复制
            "${PYTHON_SCRIPT_SOURCE}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PYTHON_SCRIPT_NAME}"
    COMMENT "Copying Python script ${PYTHON_SCRIPT_NAME} to executable directory"
)

# Windows 平台专用库
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Charts
    Qt6::Svg
    rtklib
    pybind11::embed
    ws2_32 winmm)
    # Unix下链接pthread
find_package(Threads REQUIRED)
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif()

