cmake_minimum_required(VERSION 3.8)
project(gz_ros2_control_demos)

find_package(ros2_control_cmake REQUIRED)
set_compiler_options()
export_windows_symbols()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake REQUIRED)
find_package(urdf_tutorial REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(control_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(nav_msgs)
find_package(geometry_msgs REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(number_interface REQUIRED)


add_executable(controller src/controller.cpp)
ament_target_dependencies(controller rclcpp number_interface sensor_msgs geometry_msgs tf2_geometry_msgs nav_msgs)

add_executable(vel_info src/vel_debug.cpp)
ament_target_dependencies(vel_info rclcpp geometry_msgs)

add_executable(joystic src/joystic.cpp)
ament_target_dependencies(joystic rclcpp number_interface sensor_msgs geometry_msgs std_msgs)


install(
  DIRECTORY meshes urdf launch config worlds
  DESTINATION share/${PROJECT_NAME}/
)


install(TARGETS
  controller
  vel_info
  joystic
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
