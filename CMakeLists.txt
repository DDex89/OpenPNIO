cmake_minimum_required(VERSION 3.16)
project(OpenPNIO VERSION 0.0.1 LANGUAGES C)

add_library(openpnio INTERFACE)
target_include_directories(openpnio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(openpnio INTERFACE c_std_11)

enable_testing()
