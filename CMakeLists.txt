cmake_minimum_required(VERSION 3.20)

project(KVStorage CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
#set(HEADER "${CMAKE_CURRENT_SOURCE_DIR}/Headers/VK_Storage.h")



#add_library(vk_storage STATIC
#    include/VK_Storage.h
#    include/VK_Storage.ipp
#)

# ===== ОСНОВНАЯ БИБЛИОТЕКА =====
file(GLOB_RECURSE VK_STORAGE_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

file(GLOB_RECURSE VK_STORAGE_HEADERS 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    #${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

add_library(vk_storage INTERFACE)

target_include_directories(vk_storage INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_sources(vk_storage INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/VK_Storage.h>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/VK_Storage.ipp>
)

add_executable(KVStorage ${SOURCE})

target_link_libraries(KVStorage vk_storage)

enable_testing()
add_subdirectory(tests)
