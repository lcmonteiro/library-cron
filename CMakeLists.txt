cmake_minimum_required (VERSION 3.13)

# project
project(cron)

# properties
set(CMAKE_CXX_STANDARD 17)

# target
add_library(cron 
	./cron/cron.cpp
)

# target configuration
target_include_directories(cron
INTERFACE
	./cron
)

# test
enable_testing()
add_subdirectory(test)
