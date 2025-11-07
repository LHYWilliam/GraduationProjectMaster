cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME GraduationProjectMaster)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${CMAKE_SOURCE_DIR}/Application/Application.c
    ${CMAKE_SOURCE_DIR}/Application/LVGL/lv_port_disp.c
        ${CMAKE_SOURCE_DIR}/Application/LVGL/lv_port_indev.c

    ${CMAKE_SOURCE_DIR}/Hardware/Src/LCD_Font.c
    ${CMAKE_SOURCE_DIR}/Hardware/Src/LCD.c
    ${CMAKE_SOURCE_DIR}/Hardware/Src/Touch_I2C.c
    ${CMAKE_SOURCE_DIR}/Hardware/Src/Touch.c

    ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/lvgl/demos/widgets/lv_demo_widgets.c
    ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/lvgl/demos/widgets/assets/img_clothes.c
    ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/lvgl/demos/widgets/assets/img_demo_widgets_avatar.c
    ${CMAKE_SOURCE_DIR}/Middlewares/Third_Party/lvgl/demos/widgets/assets/img_lvgl_logo.c

    ${CMAKE_SOURCE_DIR}/Util/Src/I2C.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${CMAKE_SOURCE_DIR}/Application/
    ${CMAKE_SOURCE_DIR}/Application/LVGL

    ${CMAKE_SOURCE_DIR}/Hardware/Inc

    ${CMAKE_SOURCE_DIR}/Util/Inc
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
    lvgl
)

add_subdirectory(Middlewares/Third_Party/lvgl)
