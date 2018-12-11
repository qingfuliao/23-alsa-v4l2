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
include_directories(${PROJECT_SOURCE_DIR}/third_lib/fdk-aac/include)
include_directories(${PROJECT_SOURCE_DIR}/third_lib/alsa/include)
# 添加需要链接的库文件目录link_directories
link_directories(${PROJECT_SOURCE_DIR}/third_lib/fdk-aac/lib) 
link_directories(${PROJECT_SOURCE_DIR}/third_lib/alsa/lib)

set(PROJECT_LIBS  m fdk-aac asound dl pthread)

add_executable(alsa_record ${SRC_LIST})
target_link_libraries(alsa_record ${PROJECT_LIBS})