
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was STM32PlatformConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/C:/Program Files (x86)/STM32_Platform" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

include(CMakeFindDependencyMacro)

# Platform requires STM32 HAL
# User must provide STM32 HAL in their project via:
#   target_link_libraries(your_target PRIVATE STM32Platform::platform stm32_hal)

# Include targets file
include("${CMAKE_CURRENT_LIST_DIR}/STM32PlatformTargets.cmake")

# Check all required components are found
check_required_components(STM32Platform)

# Set variables for user convenience
set(STM32Platform_VERSION 1.0.0)
set(STM32Platform_INCLUDE_DIRS "/stm32_platform")
set(STM32Platform_LIBRARIES STM32Platform::platform)

# Provide information message
if(NOT STM32Platform_FIND_QUIETLY)
    message(STATUS "Found STM32Platform: ${STM32Platform_VERSION}")
    message(STATUS "  Include dirs: ${STM32Platform_INCLUDE_DIRS}")
    message(STATUS "  Libraries: ${STM32Platform_LIBRARIES}")
endif()
