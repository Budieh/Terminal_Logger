# Minimum required cmake version
cmake_minimum_required(VERSION 3.16)

# Set compiler and flags (debug and release)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

# Show build type and flags used for the current build process (debug or release)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")    
    message(STATUS "Debug build")
    message(STATUS "    Build flags: ${CMAKE_CXX_FLAGS_DEBUG}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")    
    message(STATUS "Release build")
    message(STATUS "    Build flags: ${CMAKE_CXX_FLAGS_RELEASE}")
endif()

# Set C++ standard 
set(CXX_STANDARD 23)
set(CXX_STANDARD_REQUIRED YES)

# Set project name 
set(PROJECT_NAME "Terminal_Logger")
project(${PROJECT_NAME} LANGUAGES CXX)

# Set variables for directories that contain headers and source files
set(HEADER_DIR ${CMAKE_SOURCE_DIR}/headers)
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/sources)

# Search recursively through SOURCE_DIR and find all source files, files that are going to be stored in the SOURCE_FILES variable
file(GLOB_RECURSE        # Search recursively through the given directory
    SOURCES_ARRAY        # Array that will store all .cpp file names 
    CONFIGURE_DEPENDS    # Tell CMAKE to reconfigure when files are being added / removed from SOURCE_DIR
    ${SOURCE_DIR}/*.cpp) # Provide directory name and pattern for the desired files (.cpp)

# Display messageto make sure all source files have been found successfully
message(STATUS "Source files:")
foreach(file ${SOURCES_ARRAY})
    message(STATUS "   ${file}")
endforeach()

# Same for header files, not going to be used anywhere, just for feedback to the user
file(GLOB_RECURSE        
    HEADERS_ARRAY          
    CONFIGURE_DEPENDS    
    ${HEADER_DIR}/*.h
    ${HEADER_DIR}/*.hpp) 

message(STATUS "Header files:")
foreach(file ${HEADERS_ARRAY})
    message(STATUS "   ${file}")
endforeach()

# Add executable for current project
add_executable(${PROJECT_NAME} ${SOURCES_ARRAY})

# Specify the directory in which to look for the header files
target_include_directories(${PROJECT_NAME} PUBLIC ${HEADER_DIR})

# Specify the location of the executable, that being the current directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})