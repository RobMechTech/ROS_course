cmake_minimum_required(VERSION 3.8)
project(project)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(turtlesim REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(ros_interface REQUIRED)

add_executable(controller src/controller.cpp)
ament_target_dependencies(controller rclcpp geometry_msgs turtlesim ros_interface )

add_executable(spawner src/spawn.cpp)
ament_target_dependencies(spawner rclcpp ros_interface turtlesim )

install(TARGETS
  controller
  spawner
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
