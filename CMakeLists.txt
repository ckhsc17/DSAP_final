cmake_minimum_required(VERSION 3.0.0)
project(DSAP112_FinalProject VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(DSAP112_FinalProject main.cpp)
find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)
target_link_libraries(DSAP112_FinalProject PRIVATE sfml-system sfml-network sfml-graphics sfml-window)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set (CMAKE_CXX_STANDARD 17)

include(CPack)
