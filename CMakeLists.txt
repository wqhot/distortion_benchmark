cmake_minimum_required(VERSION 3.5)
if (PLATFORM STREQUAL "ARM")
include(cross.cmake)
add_definitions("-DKYLIN_BUILD")
add_definitions("-DCAMBRION_BUILD")
else (PLATFORM STREQUAL "ARM")
SET(CMAKE_FIND_ROOT_PATH /usr)
remove_definitions("-DKYLIN_BUILD")
remove_definitions("-DCAMBRION_BUILD")
endif (PLATFORM STREQUAL "ARM")

project(distortion_benchmark)


if (BUILD_TYPE STREQUAL "RELEASE")
set(CMAKE_BUILD_TYPE "RELEASE")
else (BUILD_TYPE STREQUAL "RELEASE")
set(CMAKE_BUILD_TYPE "DEBUG")
endif (BUILD_TYPE STREQUAL "RELEASE")

if (CMAKE_BUILD_TYPE STREQUAL "RELEASE")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -pthread -s")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -pthread -s -static-libgcc")
else(CMAKE_BUILD_TYPE STREQUAL "RELEASE")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O0 -g -Wall -pthread")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -Wall -pthread")
endif(CMAKE_BUILD_TYPE STREQUAL "RELEASE")

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)
find_package(Threads)


include_directories(
        ${PROJECT_SOURCE_DIR}/includes
        ${CMAKE_FIND_ROOT_PATH}/include/freetype2
        ${CMAKE_FIND_ROOT_PATH}/include
        ${OpenCV_INCLUDE_DIRS}
)

add_library(common
        src/common/yaml.cpp
)

add_library(producer
        src/pipelines/image_producer/sequence_producer.cpp
)

add_library(processor
        src/pipelines/image_processor/common_processor.cpp
)

add_library(consumer
        src/pipelines/image_consumer/imshow_consumer.cpp
)

add_library(pipelines
        src/pipelines/pipelines.cpp
)

if (PLATFORM STREQUAL "ARM")
        target_link_libraries(pipelines
                consumer
                producer
                processor
    
                common
        )
else (PLATFORM STREQUAL "ARM")
        target_link_libraries(pipelines
                consumer
                producer
                processor
                common
        )
endif (PLATFORM STREQUAL "ARM")


add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  pipelines
  ${OpenCV_LIBS} 
  ${CMAKE_THREAD_LIBS_INIT}
)

