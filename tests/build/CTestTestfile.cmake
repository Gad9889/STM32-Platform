# CMake generated Testfile for 
# Source directory: C:/Users/ramga/Desktop/bgu/BGR/STM32 Platform/tests
# Build directory: C:/Users/ramga/Desktop/bgu/BGR/STM32 Platform/tests/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test_utils "C:/Users/ramga/Desktop/bgu/BGR/STM32 Platform/tests/build/test_utils.exe")
set_tests_properties(test_utils PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/ramga/Desktop/bgu/BGR/STM32 Platform/tests/CMakeLists.txt;56;add_test;C:/Users/ramga/Desktop/bgu/BGR/STM32 Platform/tests/CMakeLists.txt;60;add_platform_test;C:/Users/ramga/Desktop/bgu/BGR/STM32 Platform/tests/CMakeLists.txt;0;")
add_test(test_database "C:/Users/ramga/Desktop/bgu/BGR/STM32 Platform/tests/build/test_database.exe")
set_tests_properties(test_database PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/ramga/Desktop/bgu/BGR/STM32 Platform/tests/CMakeLists.txt;56;add_test;C:/Users/ramga/Desktop/bgu/BGR/STM32 Platform/tests/CMakeLists.txt;65;add_platform_test;C:/Users/ramga/Desktop/bgu/BGR/STM32 Platform/tests/CMakeLists.txt;0;")
add_test(test_hashtable "C:/Users/ramga/Desktop/bgu/BGR/STM32 Platform/tests/build/test_hashtable.exe")
set_tests_properties(test_hashtable PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/ramga/Desktop/bgu/BGR/STM32 Platform/tests/CMakeLists.txt;56;add_test;C:/Users/ramga/Desktop/bgu/BGR/STM32 Platform/tests/CMakeLists.txt;71;add_platform_test;C:/Users/ramga/Desktop/bgu/BGR/STM32 Platform/tests/CMakeLists.txt;0;")
subdirs("_deps/unity-build")
