cmake_minimum_required(VERSION 3.16)
project(OpenPNIO VERSION 0.0.1 LANGUAGES C)

add_library(openpnio STATIC src/frame.c)
target_include_directories(openpnio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(openpnio PUBLIC c_std_11)

include(CTest)
if(BUILD_TESTING)
    add_executable(test_frame tests/test_frame.c)
    target_link_libraries(test_frame PRIVATE openpnio)
    add_test(NAME frame_parser COMMAND test_frame)
endif()
