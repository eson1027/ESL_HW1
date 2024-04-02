cmake_minimum_required(VERSION 3.8)

# Define the project name of these source code
project (sobel)

# Define the C++ standard -std=c++17
# This is required to match the systemc installed with Ubuntu
set (CMAKE_CXX_STANDARD 17)

# Add optimization & warning flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")

# Set clock period = 1 ns
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D CLOCK_PERIOD=1.0")

# Define the executable name and its source code
file(GLOB SRC "*.cpp")
add_executable(sobel ${SRC})

# Define the used libraries of the executable, Equal to -l flags of g++
# In here, the libraries are given in apsolute path. hence no needs for -L
# flag
target_link_libraries(sobel systemc)

# Add test sobel filter command
set(OUT_FILE "out.bmp")
add_custom_command(OUTPUT ${OUT_FILE}
	COMMAND ./sobel ../peppers_color_noise.bmp ./${OUT_FILE}
	DEPENDS sobel
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}	
)
add_custom_target(run 
	DEPENDS ${OUT_FILE}
)

# Compare output & golden
add_custom_target(check
	COMMAND diff -s ${CMAKE_BINARY_DIR}/${OUT_FILE} golden/peppers_color_noise_gaussian.bmp
	DEPENDS run
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}	
)
