# - Try to find Genicam
# Once done this will define
#  GENICAM_FOUND - System has genicam
#  GENICAM_INCLUDE_DIRS - The genicam include directories
#  GENICAM_LIBRARIES - The libraries needed to use genicam

find_path(GENICAM_INCLUDE_DIR GenICam.h
        PATHS
        "/usr/include/genicam/"
        )

set(GENICAM_LIBRARY_DIR "/usr/lib/x86_64-linux-gnu/genicam-mtca-ias" )

find_library(WRAPMEMCPY_LIBRARY NAMES wrapmemcpy PATHS ${GENICAM_LIBRARY_DIR})

find_library(CLALLSERIAL_LIBRARY
        NAMES
        CLAllSerial_gcc48_v3_3
        PATHS
        ${GENICAM_LIBRARY_DIR}
        )

find_library(CLPROTOCOL_LIBRARY
        NAMES
        CLProtocol_gcc48_v3_3
        PATHS
        ${GENICAM_LIBRARY_DIR}
        )

find_library(GENAPI_LIBRARY
        NAMES
        GenApi_gcc48_v3_3
        PATHS
        ${GENICAM_LIBRARY_DIR}
        )

find_library(GCBASE_LIBRARY
        NAMES
        GCBase_gcc48_v3_3
        PATHS
        ${GENICAM_LIBRARY_DIR}
        )

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GenICam DEFAULT_MSG
        GENICAM_INCLUDE_DIR
        GENICAM_LIBRARY_DIR)

mark_as_advanced(GENICAM_INCLUDE_DIR GENICAM_LIBRARIES)
