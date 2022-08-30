# - Try to find Genicam
# Once done this will define
#  GENICAM_FOUND - System has genicam
#  GENICAM_INCLUDE_DIRS - The genicam include directories
#  GENICAM_LIBRARIES - The libraries needed to use genicam

find_path(GENICAM_INCLUDE_DIR GenICam.h
        PATHS
        "/usr/local/dmcs/GenICam/library/CPP/include"
        )

set(GENICAM_LIBRARY_DIR "/usr/local/dmcs/GenICam/bin/Linux64_x64" )

find_library(WRAPMEMCPY_LIBRARY NAMES wrapmemcpy PATHS ${GENICAM_LIBRARY_DIR})

find_library(CLALLSERIAL_LIBRARY
        NAMES
        CLAllSerial_gcc48_v3_2
        PATHS
        ${GENICAM_LIBRARY_DIR}
        )

find_library(CLPROTOCOL_LIBRARY
        NAMES
        CLProtocol_gcc48_v3_2
        PATHS
        ${GENICAM_LIBRARY_DIR}
        )

find_library(GENAPI_LIBRARY
        NAMES
        GenApi_gcc48_v3_2
        PATHS
        ${GENICAM_LIBRARY_DIR}
        )

find_library(GCBASE_LIBRARY
        NAMES
        GCBase_gcc48_v3_2
        PATHS
        ${GENICAM_LIBRARY_DIR}
        )


#set(GENICAM_LIBRARIES ${GENAPI_LIBRARY} ${GCBASE_LIBRARY} ${LOG4CPP_LIBRARY} ${LOG_GCC_LIBRARY} ${MATHPARSER_LIBRARY})
#set(GENICAM_INCLUDE_DIRS ${GENICAM_INCLUDE_DIR} )

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GenICam DEFAULT_MSG
        GENICAM_INCLUDE_DIR
        GENICAM_LIBRARY_DIR)

mark_as_advanced(GENICAM_INCLUDE_DIR GENICAM_LIBRARIES)
