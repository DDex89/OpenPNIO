cmake_minimum_required(VERSION 3.16)
project(OpenPNIO VERSION 0.0.1 LANGUAGES C)

add_library(openpnio STATIC src/frame.c src/dcp.c src/dcp_response.c src/dcp_device.c src/dcp_set.c src/dcp_get.c src/dcp_identity_fields.c src/device_model.c src/lldp.c src/rtc1.c src/rpc_cl.c src/ar_session.c src/lab_identity.c)
target_include_directories(openpnio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(openpnio PUBLIC c_std_11)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_executable(openpnio_dcp_probe platform/linux/dcp_probe.c)
    target_link_libraries(openpnio_dcp_probe PRIVATE openpnio)
    target_compile_definitions(openpnio_dcp_probe PRIVATE _DEFAULT_SOURCE)
endif()

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

    add_executable(test_dcp_get tests/test_dcp_get.c)
    target_link_libraries(test_dcp_get PRIVATE openpnio)
    add_test(NAME dcp_get COMMAND test_dcp_get)

    add_executable(test_lab_identity tests/test_lab_identity.c)
    target_link_libraries(test_lab_identity PRIVATE openpnio)
    add_test(NAME lab_identity_guard COMMAND test_lab_identity)

    add_executable(test_ar_session tests/test_ar_session.c)
    target_link_libraries(test_ar_session PRIVATE openpnio)
    add_test(NAME ar_watchdog_simulation COMMAND test_ar_session)

    add_executable(test_lldp tests/test_lldp.c)
    target_link_libraries(test_lldp PRIVATE openpnio)
    add_test(NAME lldp_codec COMMAND test_lldp)

    add_executable(test_rtc1 tests/test_rtc1.c)
    target_link_libraries(test_rtc1 PRIVATE openpnio)
    add_test(NAME rtc1_codec COMMAND test_rtc1)

    add_executable(test_rpc_cl tests/test_rpc_cl.c)
    target_link_libraries(test_rpc_cl PRIVATE openpnio)
    add_test(NAME dce_rpc_cl_codec COMMAND test_rpc_cl)

    add_executable(test_device_model tests/test_device_model.c)
    target_link_libraries(test_device_model PRIVATE openpnio)
    add_test(NAME virtual_8di_8do COMMAND test_device_model)

    add_executable(test_dcp_set tests/test_dcp_set.c)
    target_link_libraries(test_dcp_set PRIVATE openpnio)
    add_test(NAME dcp_set_station_name COMMAND test_dcp_set)

    add_executable(test_dcp_ip tests/test_dcp_ip.c)
    target_link_libraries(test_dcp_ip PRIVATE openpnio)
    add_test(NAME dcp_set_ip_suite COMMAND test_dcp_ip)
endif()
