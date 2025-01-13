cmake_minimum_required(VERSION 3.5.0)
project(userpasswd VERSION 0.1.0 LANGUAGES C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk4)
pkg_check_modules(ADWAITAA REQUIRED libadwaita-1)

add_executable(userpasswd gui/userpasswd.c)

add_subdirectory(backend)

target_include_directories(
    userpasswd PRIVATE
    ${GTK_INCLUDE_DIRS}
    ${ADWAITAA_INCLUDE_DIRS}
)

target_link_libraries(
    userpasswd PRIVATE
    ${GTK_LIBRARIES}
    ${ADWAITAA_LIBRARIES}
)

target_compile_options(
    userpasswd PRIVATE
    ${GTK_CFLAGS}
    ${ADWAITAA_CFLAGS}
)

set(CMAKE_CLEAN_FILES ${PROJECT_NAME})
include(GNUInstallDirs)
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/${PROJECT_NAME}")