# This will define the following variables:
#
#   Fuzzylite_FOUND            - True if the system has the fuzzylite library
#   Fuzzylite_INCLUDE_DIRS     - location of header files
#   Fuzzylite_LIBRARIES        - location of library files

option(Fuzzylite_STATIC "Using static linking fuzzylite" ON)
option(Fuzzylite_DEBUG "Using fuzzylite debug library" OFF)

set(FL_POSTFIX)
if(${Fuzzylite_STATIC})
    set(FL_POSTFIX "-static")
endif()
if(${Fuzzylite_DEBUG})
    set(FL_POSTFIX "${FL_POSTFIX}-debug")
endif()
set(FL_LIBRARY_NAME fuzzylite${FL_POSTFIX})

find_path(Fuzzylite_INCLUDE_DIRS fl/fuzzylite.h
    "/usr/include"
    "/usr/local/include"
    )
find_library(Fuzzylite_LIBRARIES ${FL_LIBRARY_NAME}
    "/usr/lib"
    "/usr/local/lib"
    )

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Fuzzylite
    FOUND_VAR Fuzzylite_FOUND
    REQUIRED_VARS Fuzzylite_INCLUDE_DIRS Fuzzylite_LIBRARIES
    )
