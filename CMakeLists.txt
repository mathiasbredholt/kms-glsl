cmake_minimum_required(VERSION 3.14)
project(kms-glsl)

add_executable(kms-glsl common.c drm-atomic.c drm-common.c drm-legacy.c glsl.c perfcntrs.c shadertoy.cpp)
target_link_libraries(kms-glsl GLESv2 EGL drm gbm)
target_include_directories(kms-glsl PUBLIC /usr/include/libdrm)
