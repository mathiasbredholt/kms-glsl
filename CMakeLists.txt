cmake_minimum_required(VERSION 3.14)
project(kms-glsl)

set(CMAKE_CXX_STANDARD 20)


# ---- Install CPM ----

set(CPM_DOWNLOAD_VERSION 0.34.0)

if(CPM_SOURCE_CACHE)
  set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
  set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
  set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
  message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
  file(DOWNLOAD
       https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
       ${CPM_DOWNLOAD_LOCATION}
  )
endif()

include(${CPM_DOWNLOAD_LOCATION})


CPMAddPackage("https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip")

add_executable(kms-glsl common.c drm-atomic.c drm-common.c drm-legacy.c glsl.c perfcntrs.c shadertoy.cpp)
target_link_libraries(kms-glsl GLESv2 EGL drm gbm glm)
target_include_directories(kms-glsl PUBLIC /usr/include/libdrm)
