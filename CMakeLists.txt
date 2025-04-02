cmake_minimum_required(VERSION 3.0)
project(test_libx264)

# Add an option to set the output directory
option(B "Set the output directory for the build" "${CMAKE_BINARY_DIR}/output")

# Add custom commands to create input directories
add_custom_command(
    OUTPUT input/1920x1080
    COMMAND ${CMAKE_COMMAND} -E make_directory input/1920x1080
)

add_custom_command(
    OUTPUT input/1280x720
    COMMAND ${CMAKE_COMMAND} -E make_directory input/1280x720
)

add_custom_command(
    OUTPUT out
    COMMAND ${CMAKE_COMMAND} -E make_directory out
)

add_custom_command(
    OUTPUT result
    COMMAND ${CMAKE_COMMAND} -E make_directory result
)

# Ensure the directories are created before building the target
add_custom_target(create_input_dirs ALL DEPENDS input/1920x1080 out result)

# Add a custom target to set executable permissions for auto.sh
add_custom_target(set_auto_sh_executable ALL
    COMMAND /bin/chmod +x ${CMAKE_SOURCE_DIR}/auto.sh
    COMMAND /bin/chmod -R u+w ${CMAKE_SOURCE_DIR}/result
    COMMENT "Setting executable permissions for auto.sh"
)

# Add the source file
add_executable(test_libx264 test_x264.cpp)

# Set the path to libx264.a
set(LIBX264_LIBRARY "${CMAKE_SOURCE_DIR}/include/x264/libx264.a")

# Set the path to x264.h
set(LIBX264_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/x264")

# Add the include directory
target_include_directories(test_libx264 PUBLIC ${LIBX264_INCLUDE_DIR})

# Link against libx264
target_link_libraries(test_libx264 ${LIBX264_LIBRARY})

# Set the output directory for the executable
set_target_properties(test_libx264 PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/out/${B}")

