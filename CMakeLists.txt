cmake_minimum_required(VERSION 3.5.0)

#=====================
project(userpasswd VERSION 0.1.0 LANGUAGES C)
set(VERSION 1.0.1)
#=====================

include(GNUInstallDirs)

find_program(GLIB_GENMARSHAL glib-genmarshal PATH_SUFFIXES glib)
find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option (USE_ADWAITA "Use Adwaita theme" ON)
if (USE_ADWAITA)
    add_definitions(-DUSE_ADWAITA)
endif()
add_definitions(-DVERSION=\"${VERSION}\")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk4)
if (USE_ADWAITA)
    pkg_check_modules (ADWAITA REQUIRED libadwaita-1)
endif ()
pkg_check_modules(JSON_GLIB json-glib-1.0)

set (SOURCES
    src/userpasswd-app.c
    src/userpasswd-window.c
    src/userpasswd-stream.c
)

set (DESKTOP_FILE
     data/userpasswd.desktop)

set (METAINFO_FILE
    data/${PROJECT_NAME}.metainfo.xml)

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

set(RESOURCE_XML data/userpasswd.gresource.xml)
set(RESOURCE_BINARY userpasswd.gresource.c)

add_custom_command(
    OUTPUT ${RESOURCE_BINARY}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${GLIB_COMPILE_RESOURCES}
    ARGS
        --target ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE_BINARY}
        --sourcedir ${CMAKE_CURRENT_SOURCE_DIR}/data
        --generate-source
        ${RESOURCE_XML}
    VERBATIM
    MAIN_DEPENDENCY ${RESOURCE_XML}
    DEPENDS
        data/ui/userpasswd-gnome-window.ui
)

add_custom_target(
    userpasswd-resource
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE_BINARY}
)

add_executable(userpasswd ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/userpasswd-marshal.c ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE_BINARY})
set_source_files_properties (
    ${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE_BINARY}
    PROPERTIES GENERATED TRUE
)
add_dependencies (${PROJECT_NAME} userpasswd-resource)

add_subdirectory(src/pam_helper)
add_subdirectory(locales)

if (USE_ADWAITA)
    target_include_directories(userpasswd PRIVATE ${ADWAITA_INCLUDE_DIRS})
    target_link_libraries(userpasswd PRIVATE ${ADWAITA_LIBRARIES})
    target_compile_options(userpasswd PRIVATE ${ADWAITA_CFLAGS})
endif ()

target_include_directories(
    userpasswd PRIVATE
    src/pam_helper/include
    src
    ${GTK_INCLUDE_DIRS}
    ${JSON_GLIB_INCLUDE_DIRS}
)

target_link_libraries(
    userpasswd PRIVATE
    ${GTK_LIBRARIES}
    ${JSON_GLIB_LIBRARIES}
)

target_compile_options(
    userpasswd PRIVATE
    ${GTK_CFLAGS}
    ${JSON_GLIB_CFLAGS}
)

install (TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install (FILES ${DESKTOP_FILE} DESTINATION
         ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES data/icons/org.altlinux.userpasswd.Devel.svg 
              data/icons/org.altlinux.userpasswd.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
install(FILES data/icons/org.altlinux.userpasswd-symbolic.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/symbolic/apps)
if (USE_ADWAITA)
    install (FILES ${METAINFO_FILE} DESTINATION
         ${CMAKE_INSTALL_DATADIR}/metainfo)
endif ()
