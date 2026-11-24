# ============================================================================
# BGC Custom Build Configuration Overrides
# ============================================================================

set(QGC_APP_NAME "BGC" CACHE STRING "App Name" FORCE)
set(QGC_ORG_NAME "BerthAero" CACHE STRING "Org Name" FORCE)
set(QGC_ORG_DOMAIN "berthaero.com" CACHE STRING "Org Domain" FORCE)
set(QGC_PACKAGE_NAME "com.berthaero.bgc" CACHE STRING "Package Name" FORCE)
set(QGC_ANDROID_PACKAGE_NAME "com.berthaero.bgc" CACHE STRING "Android package identifier" FORCE)
set(QGC_STABLE_BUILD ON CACHE BOOL "Stable build (no Daily suffix)" FORCE)
set(QGC_APP_DESCRIPTION "BerthAero Ground Control" CACHE STRING "App Description" FORCE)
set(QGC_APP_COPYRIGHT "Copyright (c) 2026 BerthAero. All rights reserved." CACHE STRING "App Copyright" FORCE)
set(QGC_MACOS_ICON_PATH "${CMAKE_SOURCE_DIR}/custom/res/icons/bgc.icns" CACHE FILEPATH "macOS application icon path" FORCE)
set(QGC_MACOS_BUNDLE_ID "com.berthaero.bgc" CACHE STRING "macOS bundle identifier" FORCE)
