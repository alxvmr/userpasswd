cmake_minimum_required(VERSION 3.5.0)
project(pam_helper VERSION 0.1.0 LANGUAGES C)

include(GNUInstallDirs)

set(SOURCE_FILES_PAM_HELPER
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pam_helper.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/pam_helper_json.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/translate.c
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(PAM REQUIRED pam)
pkg_check_modules(PAM_MISC pam_misc)
pkg_check_modules(JSON_GLIB json-glib-1.0)

add_executable(helper  ${SOURCE_FILES_PAM_HELPER})

target_include_directories(helper PRIVATE ${HEADER_DIR} ${GOBJECT_INCLUDE_DIRS} ${GIO_INCLUDE_DIRS} ${PAM_INCLUDE_DIRS} ${JSON_GLIB_DIRS})
target_link_libraries(helper PRIVATE ${GOBJECT_LIBRARIES} ${GIO_LIBRARIES} ${PAM_LIBRARIES} ${JSON_GLIB_LIBRARIES})
target_compile_options(helper PRIVATE ${GOBJECT_CFLAGS} ${GIO_CFLAGS} ${PAM_CFLAGS} ${JSON_GLIB_CFLAGS})

install (TARGETS helper DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/userpasswd/)
