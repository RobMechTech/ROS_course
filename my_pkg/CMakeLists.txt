cmake_minimum_required(VERSION 3.8)
project(my_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(example_interfaces REQUIRED)
find_package(ros_interface REQUIRED)

add_executable(pub_node src/pub.cpp)
ament_target_dependencies(pub_node rclcpp example_interfaces)

add_executable(sub_node src/sub.cpp)
ament_target_dependencies(sub_node rclcpp example_interfaces)

add_executable(srv_node src/srv.cpp)
ament_target_dependencies(srv_node rclcpp example_interfaces)

add_executable(cli_node src/client.cpp)
ament_target_dependencies(cli_node rclcpp example_interfaces)

add_executable(cust_srv src/coustom_srv.cpp)
ament_target_dependencies(cust_srv rclcpp ros_interface)

add_executable(cust_cli src/coustom_cli.cpp)
ament_target_dependencies(cust_cli rclcpp ros_interface)

install(TARGETS
  pub_node
  sub_node
  srv_node
  cli_node
  cust_srv
  cust_cli
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
