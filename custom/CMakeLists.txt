# ============================================================================
# BGC Custom Plugin
# ============================================================================

message(STATUS "QGC: Adding BGC Custom Plugin")

# ============================================================================
# Custom Android Package Template (res/ overlay only; keep upstream AndroidManifest.xml)
# ============================================================================

if(ANDROID)
    set(CUSTOM_ANDROID_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    if(IS_DIRECTORY "${CUSTOM_ANDROID_DIR}")
        file(GLOB_RECURSE CUSTOM_ANDROID_FILES
            CONFIGURE_DEPENDS
            LIST_DIRECTORIES FALSE
            "${CUSTOM_ANDROID_DIR}/*"
        )
        if(CUSTOM_ANDROID_FILES)
            message(STATUS "QGC: BGC Android res/ overlay found. Merging with upstream android/...")
            set(DEFAULT_ANDROID_DIR "${CMAKE_SOURCE_DIR}/android")
            set(FINAL_ANDROID_DIR "${CMAKE_BINARY_DIR}/${QGC_CUSTOM_DIR}/android")

            file(REMOVE_RECURSE "${FINAL_ANDROID_DIR}")
            file(COPY "${DEFAULT_ANDROID_DIR}/" DESTINATION "${FINAL_ANDROID_DIR}")
            file(COPY "${CUSTOM_ANDROID_DIR}/" DESTINATION "${FINAL_ANDROID_DIR}")

            set(QGC_ANDROID_PACKAGE_SOURCE_DIR "${FINAL_ANDROID_DIR}")
            set(QGC_ANDROID_PACKAGE_SOURCE_DIR "${FINAL_ANDROID_DIR}" PARENT_SCOPE)
        endif()
    endif()
endif()

# ============================================================================
# Custom Brand Resources
# ============================================================================

set(CUSTOM_BRAND_RESOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/res/Images/BGCLogoDark.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/res/Images/BGCLogoLight.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/res/Images/BGCLogoMono.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/res/icons/bgc.ico"
)
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/res/Images/BGCLogoDark.svg"
    PROPERTIES QT_RESOURCE_ALIAS "QGCLogoFull.svg"
)
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/res/Images/BGCLogoLight.svg"
    PROPERTIES QT_RESOURCE_ALIAS "BGCLogoLight.svg"
)
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/res/Images/BGCLogoMono.svg"
    PROPERTIES QT_RESOURCE_ALIAS "QGCLogoWhite.svg"
)
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/res/icons/bgc.ico"
    PROPERTIES QT_RESOURCE_ALIAS "qgroundcontrol.ico"
)
qt_add_resources(${CMAKE_PROJECT_NAME} custom_brand
    PREFIX "/Custom/res"
    FILES ${CUSTOM_BRAND_RESOURCES}
)

# ============================================================================
# Custom Plugin C++ Sources & Build Configuration
# ============================================================================

set(CUSTOM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/BGCPlugin.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/BGCPlugin.h
    PARENT_SCOPE
)

set(CUSTOM_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    PARENT_SCOPE
)

set(CUSTOM_DEFINITIONS
    QGC_CUSTOM_BUILD
    CUSTOMHEADER="BGCPlugin.h"
    CUSTOMCLASS=BGCPlugin
    PARENT_SCOPE
)
