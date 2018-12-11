#alsa record
#cmake config file
CMAKE_MINIMUM_REQUIRED (VERSION 2.8)


#使用C++编译器
SET(CMAKE_C_COMPILER g++)			
SET(CMAK_CXX_COMPILER g++)
#支持C++ 11
add_compile_options(-std=c++11)

# 并将名称保存到 SRC_LIST 变量
aux_source_directory(. SRC_LIST)

# 添加头文件目录
include_directories(${PROJECT_SOURCE_DIR}/third_lib/sdl2/include)
# 添加需要链接的库文件目录link_directories
link_directories(${PROJECT_SOURCE_DIR}/third_lib/sdl2/lib) 

set(PROJECT_LIBS  m  SDL2 dl pthread)

add_executable(v4l2_sdl2 ${SRC_LIST})
target_link_libraries(v4l2_sdl2 ${PROJECT_LIBS})