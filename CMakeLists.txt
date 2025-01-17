cmake_minimum_required(VERSION 3.5.0)
project(forTest VERSION 0.1.0 LANGUAGES C CXX)

# 检查路径变量
if(NOT DEFINED Eigen3_DIR)
  set(Eigen3_DIR "D:\\dev\\Eigen3\\share\\eigen3\\cmake")
endif()
if(NOT DEFINED BOOST_ROOT)
  set(BOOST_ROOT "D:\\dev\\boost_1_84_0\\lib64-msvc-14.2\\cmake\\Boost-1.84.0")
endif()

# 查找依赖包
cmake_policy(SET CMP0167 NEW)
find_package(Eigen3 REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread chrono)
find_package(OpenGL REQUIRED)
find_package(VTK REQUIRED COMPONENTS 
  CommonCore 
  CommonDataModel 
  RenderingCore 
  RenderingOpenGL2 
  RenderingFreeType 
  InteractionStyle 
  InteractionWidgets 
  RenderingAnnotation 
  IOCore 
  IOGeometry 
  FiltersSources 
  FiltersGeneral)

# 输出调试信息
message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "Eigen3_DIR: ${Eigen3_DIR}")
message(STATUS "BOOST_ROOT: ${BOOST_ROOT}")
message(STATUS "VTK_DIR: ${VTK_DIR}")

# 添加定义和编译选项
set(CMAKE_CXX_STANDARD 17)
add_definitions(-D_HAS_STD_BYTE=0)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# 添加目标程序
add_executable(forTest newDemo.cpp)
target_include_directories(forTest PRIVATE 
  ${Boost_INCLUDE_DIRS} 
  ${Eigen3_INCLUDE_DIRS} 
  ${VTK_INCLUDE_DIRS})
target_link_libraries(forTest PRIVATE 
  ${Boost_LIBRARIES} 
  ${VTK_LIBRARIES})

# VTK 自动初始化
include(${VTK_USE_FILE})
vtk_module_autoinit(
  TARGETS forTest
  MODULES ${VTK_LIBRARIES})
