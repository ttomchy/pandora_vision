cmake_minimum_required(VERSION 2.8.3)
project(pandora_vision_common)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  cv_bridge
  pcl_ros
  image_transport
  sensor_msgs
  pandora_common_msgs
  sensor_processor
  urdf
  roslint
  )

catkin_package(
  CATKIN_DEPENDS
    roscpp
    cv_bridge
    pcl_ros
    image_transport
    sensor_msgs
    pandora_common_msgs
    sensor_processor
    urdf
  INCLUDE_DIRS
    include
  LIBRARIES
    ${PROJECT_NAME}_pcl_to_image
    ${PROJECT_NAME}_discrete_wavelet_transform
  )

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  )

add_library(${PROJECT_NAME}_pcl_to_image
  src/pandora_vision_utilities/pointcloud_to_image_converter.cpp
  )
add_dependencies(${PROJECT_NAME}_pcl_to_image
  ${catkin_EXPORTED_TARGETS}
  )
target_link_libraries(${PROJECT_NAME}_pcl_to_image
  ${catkin_LIBRARIES}
  )

############################# discrete wavelet transform #########################
add_library(${PROJECT_NAME}_discrete_wavelet_transform
  src/pandora_vision_utilities/discrete_wavelet_transform.cpp
)
target_link_libraries(${PROJECT_NAME}_discrete_wavelet_transform
  ${catkin_LIBRARIES}
)

file(GLOB_RECURSE ${PROJECT_NAME}_LINT_SRCS
  RELATIVE ${PROJECT_SOURCE_DIR}
    include/**/*.h
    src/**/*.cpp
    test/**/*.cpp
    )
roslint_cpp(${${PROJECT_NAME}_LINT_SRCS})

##################################### Testing ##################################

if(CATKIN_ENABLE_TESTING)
  add_subdirectory(test)
endif()
