find_path(I2C_INCLUDE_DIRS i2c.h
    "/home/uav/code/libi2c/include/i2c"
    )
find_library(I2C_LIBRARIES libi2c.so
    "/home/uav/code/libi2c"
    )
include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(I2C
    FOUND_VAR I2C_FOUND
    REQUIRED_VARS I2C_INCLUDE_DIRS I2C_LIBRARIES
)
