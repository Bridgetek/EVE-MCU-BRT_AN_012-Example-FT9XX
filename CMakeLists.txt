# Standard CMakeLists.txt Template for FT9xx project compilation.

# Project name. Default name is the name of the current directory.
# On command line call cmake with the parameter "-DPROJECT=firmware" to override.

# Build type (case sensitive), can be Debug (default) or Release.
# On command line call cmake with the parameter "-DBUILD=Release" or "-DBUILD=Debug" to override.

# Target device type (case sensitive), can be ft90x (default) or ft93x.
# On command line call cmake with the parameter "-DTARGET=ft93x" or "-DTARGET=ft90x" to override.

# Output directories (you may override this). 
# Default is one of "FT90x_Release", "FT90x_Debug", "FT93x_Release" or 
#  "FT93x_Debug" depending on build type and target device.
# On command line call cmake with the parameter "-DOUTDIR=test" to override.

# Load the toolchain cmake file.
# This will look in the local directory, followed by the '..' directory,
# finally it will look in the toolchain installation.
if (EXISTS ${CMAKE_SOURCE_DIR}/toolchain_ft9xx.cmake)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/toolchain_ft9xx.cmake)
elseif (EXISTS ${CMAKE_SOURCE_DIR}/../toolchain_ft9xx.cmake)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/../toolchain_ft9xx.cmake)
elseif (EXISTS $ENV{FT9XX_TOOLCHAIN}/hardware/toolchain_ft9xx.cmake)
    set(CMAKE_TOOLCHAIN_FILE $ENV{FT9XX_TOOLCHAIN}/hardware/toolchain_ft9xx.cmake)
else()
    message(FATAL_ERROR "Make sure that the FT9XX is installed correctly. Can't find the 'toolchain_ft9xx.cmake' file")
endif()

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# Set the project name to the current directory if it is not defined in the command line.
if (NOT DEFINED ${PROJECT})
	get_filename_component(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR} NAME)
	string(REPLACE " " "_" PROJECT ${PROJECT_DIR})
	message(STATUS "Project name set to ${PROJECT}")
endif()

# Setup the cmake project name.
project(${PROJECT})

# Make a variable for the executable output. This is normally based on the project name.
set(EXECUTABLE ${PROJECT}.elf)

set( CMAKE_VERBOSE_MAKEFILE on )
        
# Find any linker scripts. Uses glob to find files.
file(GLOB LDSCRIPT_FILES CONFIGURE_DEPENDS Scripts/*.ld)

# Add any additional linker libraries or options required.
if (${TARGET} MATCHES ft90x)
	set(LIB_FILES
	)
else (${TARGET} MATCHES ft93x)
	set(LIB_FILES
	)
endif()

set(FONT_MAKEFILE "Fonts/fonts.mk")
set(FONT_OUTFILES 
	${CMAKE_SOURCE_DIR}/Fonts/C/*.c
)
add_custom_command(OUTPUT ${FONT_OUTFILES}
	PRE_BUILD
	COMMAND make -f ${CMAKE_SOURCE_DIR}/Fonts/fonts.mk
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/Fonts
    DEPENDS ${FONT_MAKEFILE}
)
add_custom_target(MAKE_FONTS ALL DEPENDS ${FONT_OUTFILES})

# Compile a list of Font source files.
file(GLOB FONT_FILES CONFIGURE_DEPENDS
	Fonts/C/*.c
)

set(IMAGES_MAKEFILE "Images/images.mk")
set(IMAGES_OUTFILES 
	${CMAKE_SOURCE_DIR}/Images/S/*.S
)
add_custom_command(OUTPUT ${IMAGES_OUTFILES}
	PRE_BUILD
	COMMAND make -f ${CMAKE_SOURCE_DIR}/Images/images.mk
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/Images
    DEPENDS ${IMAGES_MAKEFILE}
)
add_custom_target(MAKE_IMAGES ALL DEPENDS ${IMAGES_OUTFILES})

# Compile a list of Image source files.
file(GLOB IMAGE_FILES CONFIGURE_DEPENDS
	Images/S/*.S
)

# Compile a list of source files.
file(GLOB SRC_FILES CONFIGURE_DEPENDS
	Sources/main.c
	Sources/usb_keyboard.c
	Sources/eve_keyboard.c
	lib/eve/ports/eve_arch_ft9xx/*.c
	lib/eve/source/*.c
	lib/eve_ui/eve_ui_arch_ft9xx/*.c
	lib/eve_ui/source/*.c
	lib/eve_ui_keyboard/*.c
)

# Override toolchain include directories with files from this directory. 
include_directories(
	Includes
	lib/eve/include
	lib/eve_ui/include
	lib/eve_ui_keyboard
	usb/include
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPLATFORM_FT9XX")

# Macro to perform cmake add_executable and configure the default settings for BUILD and TARGET.
ft9xx_set_executable(${EXECUTABLE} "${SRC_FILES}" "${LIB_FILES}" "${LDSCRIPT_FILES}")

add_dependencies(${EXECUTABLE} MAKE_FONTS MAKE_IMAGES)
target_sources(${EXECUTABLE} PRIVATE ${FONT_FILES} ${IMAGE_FILES})

# Compile a list of include directories.
target_include_directories(${EXECUTABLE} PRIVATE
)

# Setup any extra compiler options required.
target_compile_options(${EXECUTABLE} PRIVATE
)

# Enable generation of a map file for the project if needed.
ft9xx_enable_map_file(${EXECUTABLE})
