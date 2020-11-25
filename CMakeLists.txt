cmake_minimum_required(VERSION 3.18)
project(bing-wallpaper)

find_package(cpr CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(bingwallpaper main.cpp)

target_compile_features(bingwallpaper PUBLIC cxx_std_17)
set_target_properties(bingwallpaper PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(bingwallpaper PRIVATE cpr)
target_link_libraries(bingwallpaper PRIVATE fmt::fmt)
target_link_libraries(bingwallpaper PRIVATE nlohmann_json nlohmann_json::nlohmann_json)