# Copyright 2024 Malcolm Alexander
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This is kinda ugly, but it defines two macros that make tests easier.
# Since asserts greatly slow down execution time, these two macros allow for assert like statements that don't take forever for CMake to realize it's failed.
include(CTest)

set(assert_macros
		"#pragma once\n"
		"#include <cstdio>\n"
		"#define assert_test(expr) \\\n"
		"    do { \\\n"
		"        if (!(expr)) { \\\n"
		"            fprintf(stderr, \"Assertion failed: (%s), function %s, file %s, line %d.\\n\", \\\n"
		"                    #expr, __func__, __FILE__, __LINE__)\; \\\n"
		"            return 1\; \\\n"
		"        } \\\n"
		"    } while (0)\n"
		"#define assert_willfail(expr) \\\n"
		"    do { \\\n"
		"        if (expr) { \\\n"
		"            fprintf(stderr, \"WillFail assert did not fail: (%s), function %s, file %s, line %d.\\n\", \\\n"
		"                    #expr, __func__, __FILE__, __LINE__)\; \\\n"
		"            return 1\; \\\n"
		"        } \\\n"
		"    } while (0)\n"
)

# The variable above is a list. We need a string to print it.
string(CONCAT assert_macros_string "${assert_macros}")

# Write macros to a header file
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/include/test_assertions ${assert_macros})

# Create a blank list that will store the source list that we are currently working with.
set(current_test_sourcelist "")

# The names of the tests, as a result of create_test_sourcelist(), are in the form group/subgroup/filename
set(current_group_list "")

# Function to create test source list for each group
function(create_test_sourcelist_for_group group_name dir)
	# Clear the source list from the last group
	set(current_test_sourcelist "" PARENT_SCOPE)

	# Get all source files in the directory that end with .cpp
	file(GLOB TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${dir}/*.cpp")
	message(VERBOSE "Source files in ${dir}: ${TEST_SOURCES}")

	# The generated source for the group will be titled <group name>_tests.cpp and executable will be <group name>_tests
	create_test_sourcelist(${group_name}_test_sourcelist ${group_name}_tests.cpp ${TEST_SOURCES})

	# The main add_tests_in_directory function uses this list to actually generate the files.
	set(current_test_sourcelist ${${group_name}_test_sourcelist} PARENT_SCOPE)
endfunction()

# Function to add tests for each directory and subgroup
function(add_tests_in_directory dir)
	# Get all the files relative to the provided dir
	# The dir is more a "path" from the current source directory in this case
	file(GLOB DIRECTORY_ITEMS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${dir}/*)

	# Get the name of the group
	# The group name is simply the directory path, where the slashes are replaced with underscores
	string(REPLACE "/" "_" GROUP_NAME "${dir}")

	# We set current_group_list to be the same as the current directory
	string(APPEND current_group_list "${dir}")
	message(VERBOSE "Adding tests for group: \"${current_group_list}\"...")

	# Find all source files for the current group
	create_test_sourcelist_for_group(${GROUP_NAME} ${dir})

	# Add the source files to the group executable
	add_executable(${GROUP_NAME}_test_exe ${current_test_sourcelist} "${CMAKE_CURRENT_BINARY_DIR}/include/test_assertions")
	target_link_libraries(${GROUP_NAME}_test_exe PRIVATE ${TEST_LINK_LIBRARIES})
	target_include_directories(${GROUP_NAME}_test_exe PRIVATE ${TEST_INCLUDE_DIRECTORIES} SYSTEM "${CMAKE_CURRENT_BINARY_DIR}/include")

	# Set TestsToRun to the values of all the source files, then remove the main executable file from the list
	set(TestsToRun ${current_test_sourcelist})
	list(REMOVE_ITEM TestsToRun ${GROUP_NAME}_tests.cpp)

	# For each sourcefile, minus the main executable file, add a test with the name of the file.
	foreach(test ${TestsToRun})
		get_filename_component(TName ${test} NAME_WE)

		if(${TName} STREQUAL "${GROUP_NAME}_tests")
			continue()
		endif()

		message(VERBOSE "Adding test \"${TName}\" from source : ${test}")

		# This allows you to simply do TEST instead of having to type out the whole name
		set(TEST_NAME_MACRO ${test})
		string(REPLACE "/" "_" TEST_NAME_MACRO ${TEST_NAME_MACRO})
		string(REPLACE ".cpp" "" TEST_NAME_MACRO ${TEST_NAME_MACRO})
		set_source_files_properties(${test} PROPERTIES COMPILE_DEFINITIONS "TEST=${TEST_NAME_MACRO}")

		# Because of how we recursively call things, tests must be called in the form group/...subgroups.../filename
		add_test(NAME ${current_group_list}/${TName} COMMAND ${GROUP_NAME}_test_exe ${current_group_list}/${TName})

		# Sets the will fail attribute for the test.
		# Fails must result in a return code, failed asserts or segfaults will still result the test not passing.
		if(${TName} MATCHES "_wf$")
			set_property(TEST ${current_group_list}/${TName} PROPERTY WILL_FAIL true)
		endif()
	endforeach()

	set(current_group_list "" PARENT_SCOPE)
endfunction()

# Function to get relative directories that contain source files
function(get_relative_directories_with_files dir result)
	# Get a list of all files relative the provided directory (which will be the current source dir)
	file(GLOB ALL_SUBDIRS RELATIVE ${dir} ${dir}/*)
	set(DIRS "")
	foreach(subdir ${ALL_SUBDIRS})
		# We only care about the subdirectories
		if(IS_DIRECTORY ${dir}/${subdir})
			# Check if this directory contains any files
			file(GLOB SUBDIR_FILES ${dir}/${subdir}/*)
			set(CONTAINS_FILES FALSE)
			foreach(file ${SUBDIR_FILES})
				if(NOT IS_DIRECTORY ${file})
					set(CONTAINS_FILES TRUE)
					break()
				endif()
			endforeach()

			# If the directory contains files, include it in the result
			if(CONTAINS_FILES)
				list(APPEND DIRS ${subdir})
			endif()

			# Recursively call the function to get subdirectories of the subdirectory
			# subdir_dirs will still only contain directories with files
			get_relative_directories_with_files(${dir}/${subdir} subdir_dirs)
			foreach(subdir_dir ${subdir_dirs})
				list(APPEND DIRS ${subdir}/${subdir_dir})
			endforeach()
		endif()
	endforeach()
	set(${result} ${DIRS} PARENT_SCOPE)
endfunction()

# Call the function to get relative directories that contain files
get_relative_directories_with_files(${CMAKE_CURRENT_SOURCE_DIR} RELATIVE_DIRS)

# Print the directories as they're being generated
# This makes it easier to diagnose errors.
foreach(dir ${RELATIVE_DIRS})
	message(STATUS "Generating Test Group: ${dir}")
	add_tests_in_directory(${dir})
endforeach()

message(STATUS "Generating tests - done")