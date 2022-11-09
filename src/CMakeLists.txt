set(SOURCE_FILES gentl_wrapper.cpp system.cpp interface.cpp device.cpp stream.cpp CPort.cpp)
set(HEADER_FILES gentl_wrapper.h system.h interface.h device.h genicam_wrapper.h stream.h buffer.hpp CPort.h GenTLException.cpp GenTLException.hpp)

set(LIBRARY_NAME genicam_wrapper)

add_library(${LIBRARY_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(${LIBRARY_NAME} ${CMAKE_DL_LIBS})
target_link_libraries(${LIBRARY_NAME}
        ${GENAPI_LIBRARY}
        ${GCBASE_LIBRARY})

target_include_directories(${LIBRARY_NAME} PRIVATE  ${GENICAM_INCLUDE_DIR})

