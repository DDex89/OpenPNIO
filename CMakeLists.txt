cmake_minimum_required(VERSION 3.16)
project(OpenPNIO VERSION 0.0.1 LANGUAGES C)

add_library(openpnio STATIC src/frame.c src/dcp.c src/dcp_response.c src/dcp_device.c)
target_include_directories(openpnio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(openpnio PUBLIC c_std_11)

include(CTest)
if(BUILD_TESTING)
    add_executable(test_frame tests/test_frame.c)
    target_link_libraries(test_frame PRIVATE openpnio)
    add_test(NAME frame_parser COMMAND test_frame)

    add_executable(test_dcp tests/test_dcp.c)
    target_link_libraries(test_dcp PRIVATE openpnio)
    add_test(NAME dcp_codec COMMAND test_dcp)

    add_executable(test_dcp_response tests/test_dcp_response.c)
    target_link_libraries(test_dcp_response PRIVATE openpnio)
    add_test(NAME dcp_identify_response COMMAND test_dcp_response)

    add_executable(test_dcp_device tests/test_dcp_device.c)
    target_link_libraries(test_dcp_device PRIVATE openpnio)
    add_test(NAME dcp_identify_request COMMAND test_dcp_device)
endif()
