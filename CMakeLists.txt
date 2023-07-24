cmake_minimum_required(VERSION 3.7.2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#------------------------------------------------
set(ProjectId "jval")
project(${ProjectId} LANGUAGES CXX)

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/develop/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
endif()
include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(BASIC_SETUP ${ProjectId}
                REQUIRES
                    spdlog/1.11.0                
                    nlohmann_json/3.11.2
                    valijson/1.0
                    cxxopts/3.1.1
                OPTIONS 
                    spdlog:shared=False
                    spdlog:header_only=True                    
                BUILD missing
                INSTALL_FOLDER "${CMAKE_CURRENT_BINARY_DIR}"
                KEEP_RPATHS
                NO_OUTPUT_DIRS
                )

if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo_multi.cmake)
    include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo_multi.cmake)
    set(CONAN_RES_DIRS ${CONAN_RES_DIRS_DEBUG})
else()
    include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
endif()
conan_basic_setup(NO_OUTPUT_DIRS)

set(SOURCES 
    src/main.cpp
    )

#------------------------------------------------
add_executable(${ProjectId} ${SOURCES})

target_include_directories(${ProjectId} 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
endif()

conan_target_link_libraries(${ProjectId})

if (MSVC)
    set_property(TARGET ${ProjectId} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${ProjectId}>)
    set_target_properties(${ProjectId} PROPERTIES LINK_FLAGS "/MAP")
    target_compile_options(${ProjectId} PRIVATE /bigobj -D_CRT_SECURE_NO_WARNINGS)
endif()

