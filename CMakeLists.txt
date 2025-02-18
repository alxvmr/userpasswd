cmake_minimum_required(VERSION 3.5.0)
project(userpasswd VERSION 0.1.0 LANGUAGES C)

find_program(GLIB_GENMARSHAL glib-genmarshal PATH_SUFFIXES glib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk4)
pkg_check_modules(ADWAITAA REQUIRED libadwaita-1)
pkg_check_modules(JSON_GLIB json-glib-1.0)

set (SOURCES
    src/userpasswd-app.c
    src/userpasswd-window.c
    src/userpasswd-stream.c
)

set (DESKTOP_FILE
     data/userpasswd.desktop)

set (MARSHAL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/userpasswd-marshal.list)

message("Generating marshal.h: ${GLIB_GENMARSHAL} ${CMAKE_CURRENT_SOURCE_DIR}/marshal.list --header")
execute_process(
    COMMAND ${GLIB_GENMARSHAL} --header ${MARSHAL_FILE}
    OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/userpasswd-marshal.h
)

message("Generating marshal.c: ${GLIB_GENMARSHAL} ${CMAKE_CURRENT_SOURCE_DIR}/marshal.list --body")
execute_process(
    COMMAND ${GLIB_GENMARSHAL} --body ${MARSHAL_FILE}
    OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/userpasswd-marshal.c
)

add_executable(userpasswd ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/userpasswd-marshal.c)

add_subdirectory(src/pam_helper)
add_subdirectory(locales)

target_include_directories(
    userpasswd PRIVATE
    src/pam_helper/include
    src
    ${GTK_INCLUDE_DIRS}
    ${ADWAITAA_INCLUDE_DIRS}
    ${JSON_GLIB_INCLUDE_DIRS}
)

target_link_libraries(
    userpasswd PRIVATE
    ${GTK_LIBRARIES}
    ${ADWAITAA_LIBRARIES}
    ${JSON_GLIB_LIBRARIES}
)

target_compile_options(
    userpasswd PRIVATE
    ${GTK_CFLAGS}
    ${ADWAITAA_CFLAGS}
    ${JSON_GLIB_CFLAGS}
)

install (TARGETS ${PROJECT_NAME} DESTINATION bin)
install (FILES ${DESKTOP_FILE} DESTINATION
         /usr/share/applications)

set(CMAKE_CLEAN_FILES ${PROJECT_NAME})
include(GNUInstallDirs)
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/${PROJECT_NAME})")