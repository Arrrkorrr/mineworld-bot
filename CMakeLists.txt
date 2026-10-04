cmake_minimum_required(VERSION 3.10)
project(bot)
set(CMAKE_CXX_STANDARD 17)

include_directories (
    ${CMAKE_SOURCE_DIR}/autocomplete
    ${CMAKE_SOURCE_DIR}/buttons
    ${CMAKE_SOURCE_DIR}/commands
    ${CMAKE_SOURCE_DIR}/commands/nation
    ${CMAKE_SOURCE_DIR}/events
    ${CMAKE_SOURCE_DIR}/menus
    ${CMAKE_SOURCE_DIR}/menus/nation.info
    ${CMAKE_SOURCE_DIR}/modals
    ${CMAKE_SOURCE_DIR}/utils/database
    ${CMAKE_SOURCE_DIR}/utils/files
    ${CMAKE_SOURCE_DIR}/utils/logs
    ${CMAKE_SOURCE_DIR}/utils/miscellaneous
    ${CMAKE_SOURCE_DIR}/utils/parsers
    ${CMAKE_SOURCE_DIR}/utils/text
)

file(GLOB MAIN main.cpp)
file(GLOB AUTOCOMPLETE autocomplete/*.cpp)
file(GLOB BUTTONS buttons/*.cpp)
file(GLOB COMMANDS commands/*.cpp)
file(GLOB COMMANDS_NATION commands/nation/*.cpp)
file(GLOB EVENTS events/*.cpp)
file(GLOB MENUS menus/*.cpp)
file(GLOB MENUS_NATIONINFO menus/nation.info/*.cpp)
file(GLOB MODALS modals/*.cpp)
file(GLOB UTILS_DATABASE utils/database/*.cpp)
file(GLOB UTILS_FILES utils/files/*.cpp)
file(GLOB UTILS_LOGS utils/logs/*.cpp)
file(GLOB UTILS_MISCELLANEOUS utils/miscellaneous/*.cpp)
file(GLOB UTILS_PARSERS utils/parsers/*.cpp)
file(GLOB UTILS_TEXT utils/text/*.cpp)

set(ALL_SOURCES
    ${MAIN}
    ${AUTOCOMPLETE}
    ${BUTTONS}
    ${COMMANDS}
    ${COMMANDS_NATION}
    ${EVENTS}
    ${MENUS}
    ${MENUS_NATIONINFO}
    ${MODALS}
    ${UTILS_DATABASE}
    ${UTILS_FILES}
    ${UTILS_LOGS}
    ${UTILS_MISCELLANEOUS}
    ${UTILS_PARSERS}
    ${UTILS_TEXT}
)

add_executable(bot ${ALL_SOURCES})

add_custom_target(copy_binaries ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/bin/libdpp.so.10.1.6" "${CMAKE_SOURCE_DIR}/build/out/libdpp.so.10.1.6"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/bin/libmariadb.so.3" "${CMAKE_SOURCE_DIR}/build/out/libmariadb.so.3"
)

target_link_libraries(bot PRIVATE
    ${CMAKE_SOURCE_DIR}/bin/libdpp.so.10.1.6
    ${CMAKE_SOURCE_DIR}/bin/libmariadb.so.3
)
