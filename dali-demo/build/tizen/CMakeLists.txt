CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(dali-demo C CXX)

SET(dali-demo_VERSION_MAJOR 1)
SET(dali-demo_VERSION_MINOR 0)

SET(ROOT_SRC_DIR ${CMAKE_SOURCE_DIR}/../..)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})

IF(DEFINED DALI_APP_DIR)
        SET(APP_DATA_DIR ${DALI_APP_DIR})
        SET(BINDIR ${APP_DATA_DIR}/bin)
ELSE()
        SET(APP_DATA_DIR ${PREFIX}/share/com.samsung.dali-demo)
        SET(BINDIR ${PREFIX}/bin)
ENDIF()

SET(RESOURCE_DIR ${ROOT_SRC_DIR}/resources)
SET(LOCAL_IMAGES_DIR ${RESOURCE_DIR}/images)
SET(LOCAL_MODELS_DIR ${RESOURCE_DIR}/models)
SET(LOCAL_SCRIPTS_DIR ${RESOURCE_DIR}/scripts)
IF(NOT DEFINED LOCAL_STYLE_DIR)
        SET(LOCAL_STYLE_DIR ${RESOURCE_DIR}/style)
ENDIF()

SET(IMAGES_DIR ${APP_DATA_DIR}/images/)
SET(MODELS_DIR ${APP_DATA_DIR}/models/)
SET(SCRIPTS_DIR ${APP_DATA_DIR}/scripts/)
SET(STYLE_DIR ${APP_DATA_DIR}/style/)

IF(NOT DEFINED LOCALE_DIR)
        SET(LOCALE_DIR ${PREFIX}/share/locale)
ENDIF()

SET(DALI_IMAGE_DIR \\"${IMAGES_DIR}\\")
SET(DALI_MODEL_DIR \\"${MODELS_DIR}\\")
SET(DALI_SCRIPT_DIR \\"${SCRIPTS_DIR}\\")
SET(DALI_STYLE_DIR \\"${STYLE_DIR}\\")
SET(DALI_DEMO_THEME_PATH \\"${STYLE_DIR}/demo-theme.json\\")
SET(DALI_EXAMPLE_BIN \\"${BINDIR}/\\")
SET(DALI_LOCALE_DIR \\"${LOCALE_DIR}\\")
SET(DALI_LANG \\"${LANG}\\")

FILE(GLOB LOCAL_IMAGES_PNG RELATIVE "${LOCAL_IMAGES_DIR}" "${LOCAL_IMAGES_DIR}/*.png")
FILE(GLOB LOCAL_IMAGES_JPG RELATIVE "${LOCAL_IMAGES_DIR}" "${LOCAL_IMAGES_DIR}/*.jpg")
FILE(GLOB LOCAL_IMAGES_GIF RELATIVE "${LOCAL_IMAGES_DIR}" "${LOCAL_IMAGES_DIR}/*.gif")
FILE(GLOB LOCAL_IMAGES_BMP RELATIVE "${LOCAL_IMAGES_DIR}" "${LOCAL_IMAGES_DIR}/*.bmp")
FILE(GLOB LOCAL_IMAGES_ICO RELATIVE "${LOCAL_IMAGES_DIR}" "${LOCAL_IMAGES_DIR}/*.ico")
FILE(GLOB LOCAL_IMAGES_WBMP RELATIVE "${LOCAL_IMAGES_DIR}" "${LOCAL_IMAGES_DIR}/*.wbmp")

SET(LOCAL_IMAGES_LIST ${LOCAL_IMAGES_PNG};${LOCAL_IMAGES_JPG};${LOCAL_IMAGES_GIF};${LOCAL_IMAGES_BMP};${LOCAL_IMAGES_ICO};${LOCAL_IMAGES_WBMP})
FOREACH(flag ${LOCAL_IMAGES_LIST})
        INSTALL(FILES ${LOCAL_IMAGES_DIR}/${flag} DESTINATION ${IMAGES_DIR})
ENDFOREACH(flag)

FILE(GLOB LOCAL_MODELS_LIST RELATIVE "${LOCAL_MODELS_DIR}" "${LOCAL_MODELS_DIR}/*")
FOREACH(flag ${LOCAL_MODELS_LIST})
        INSTALL(FILES ${LOCAL_MODELS_DIR}/${flag} DESTINATION ${MODELS_DIR})
ENDFOREACH(flag)

FILE(GLOB LOCAL_SCRIPTS_LIST RELATIVE "${LOCAL_SCRIPTS_DIR}" "${LOCAL_SCRIPTS_DIR}/*")
FOREACH(flag ${LOCAL_SCRIPTS_LIST})
        INSTALL(FILES ${LOCAL_SCRIPTS_DIR}/${flag} DESTINATION ${SCRIPTS_DIR})
ENDFOREACH(flag)

INSTALL(FILES ${LOCAL_STYLE_DIR}/demo-theme.json DESTINATION ${STYLE_DIR})

SET(PKG_LIST dali
             dali-toolkit)

INCLUDE(FindPkgConfig)
pkg_check_modules(REQUIRED_PKGS REQUIRED ${PKG_LIST})

FOREACH(flag ${REQUIRED_PKGS_CFLAGS})
        SET(REQUIRED_CFLAGS "${REQUIRED_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(DALI_DEMO_CFLAGS "-DDALI_IMAGE_DIR=${DALI_IMAGE_DIR} -DDALI_MODEL_DIR=${DALI_MODEL_DIR} -DDALI_SCRIPT_DIR=${DALI_SCRIPT_DIR} -DDALI_DEMO_THEME_PATH=${DALI_DEMO_THEME_PATH} -DDALI_EXAMPLE_BIN=${DALI_EXAMPLE_BIN} -DDALI_LOCALE_DIR=${DALI_LOCALE_DIR} -fvisibility=hidden -DHIDE_DALI_INTERNALS -DDALI_LANG=${DALI_LANG}")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${REQUIRED_CFLAGS} ${DALI_DEMO_CFLAGS} -Werror -Wall")
SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")

INCLUDE_DIRECTORIES(${ROOT_SRC_DIR})
INCLUDE_DIRECTORIES(${DEMO_SRC_DIR})

ADD_SUBDIRECTORY(demo)
ADD_SUBDIRECTORY(examples)
ADD_SUBDIRECTORY(builder)
