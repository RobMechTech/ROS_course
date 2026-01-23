cmake_minimum_required(VERSION 3.8)
project(mec_mobile_gazebo)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ros_gz_sim REQUIRED)
find_package(mec_mobile_description REQUIRED)
find_package(ros_gz_bridge REQUIRED)
find_package(robot_localization REQUIRED)




install (
  DIRECTORY launch config models worlds
  DESTINATION share/${PROJECT_NAME}
)


ament_package()
