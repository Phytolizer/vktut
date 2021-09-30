# * Try to find GLFW3
#
# If no pkgconfig, define GLFW_ROOT to installation tree Will define the
# following: GLFW3_FOUND GLFW3_INCLUDE_DIRS GLFW3_LIBRARIES

if(PKG_CONFIG_FOUND)
  if(APPLE)
    # homebrew or macports pkgconfig locations
    set(ENV{PKG_CONFIG_PATH}
        "/usr/local/opt/glfw3/lib/pkgconfig:/opt/local/lib/pkgconfig"
    )
  endif()
  set(ENV{PKG_CONFIG_PATH}
      "${DEPENDS_DIR}/glfw/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}"
  )
  pkg_check_modules(GLFW3 glfw3)

  find_library(
    GLFW3_LIBRARY
    NAMES ${GLFW3_LIBRARIES}
    HINTS ${GLFW3_LIBRARY_DIRS}
  )
  set(GLFW3_LIBRARIES ${GLFW3_LIBRARY})

  return()
endif()

find_path(
  GLFW3_INCLUDE_DIRS GLFW/glfw3.h
  DOC "GLFW include directory "
  PATHS "${DEPENDS_DIR}/glfw" "$ENV{ProgramW6432}/glfw" ENV GLFW_ROOT
  PATH_SUFFIXES include
)

# directories in the official binary package
if(MINGW)
  set(_SUFFIX lib-mingw)
elseif(MSVC11)
  set(_SUFFIX lib-vc2012)
elseif(MSVC12)
  set(_SUFFIX lib-vc2013)
elseif(MSVC14)
  set(_SUFFIX lib-vc2015)
elseif(MSVC)
  set(_SUFFIX lib-vc2012)
endif()

find_library(
  GLFW3_LIBRARIES
  NAMES glfw3
  PATHS "${DEPENDS_DIR}/glfw" "$ENV{ProgramW6432}/glfw" ENV GLFW_ROOT
  PATH_SUFFIXES lib ${_SUFFIX}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  glfw3
  FOUND_VAR GLFW3_FOUND
  REQUIRED_VARS GLFW3_LIBRARIES GLFW3_INCLUDE_DIRS
)
