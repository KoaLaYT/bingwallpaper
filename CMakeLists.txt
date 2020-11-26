cmake_minimum_required(VERSION 3.18)
project(bing-wallpaper)

add_executable(bingwallpaper main.cpp)
target_compile_features(bingwallpaper PUBLIC cxx_std_17)
set_target_properties(bingwallpaper PROPERTIES CXX_EXTENSIONS OFF)

# I could not install cpr from vcpkg on my archlinux
if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    message(STATUS "You are in macos, download cpr from vcpkg")
    find_package(cpr CONFIG REQUIRED)
    target_link_libraries(bingwallpaper PRIVATE cpr)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message(STATUS "You are in linux, download cpr from cmake fetch")
    include(FetchContent)
    FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/whoshuu/cpr.git GIT_TAG c8d33915dbd88ad6c92b258869b03aba06587ff9) # the commit hash for 1.5.0
    FetchContent_MakeAvailable(cpr)
    target_link_libraries(bingwallpaper PRIVATE cpr::cpr)

    include(/home/koalayt/vcpkg/scripts/buildsystems/vcpkg.cmake)
else()
    message(FATAL_ERROR "Your platform(${CMAKE_SYSTEM_NAME}) is not supported yet")
endif()

find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

target_link_libraries(bingwallpaper PRIVATE fmt::fmt)
target_link_libraries(bingwallpaper PRIVATE nlohmann_json nlohmann_json::nlohmann_json)