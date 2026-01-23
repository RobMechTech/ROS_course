# Copyright 2022 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, OpaqueFunction
from launch.actions import RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import Command, FindExecutable, LaunchConfiguration, PathJoinSubstitution

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


# Use the package name for local file lookups
PACKAGE_NAME = 'gz_ros2_control_demos'

def generate_launch_description():
    # Launch Arguments
    use_sim_time = LaunchConfiguration('use_sim_time', default=True)

    def robot_state_publisher(context):
        performed_description_format = LaunchConfiguration('description_format').perform(context)
        # Get URDF or SDF via xacro
        robot_description_content = Command(
            [
                PathJoinSubstitution([FindExecutable(name='xacro')]),
                ' ',
                PathJoinSubstitution([
                    FindPackageShare(PACKAGE_NAME), 
                    performed_description_format,'robot',
                    f'self_balancing.xacro.{performed_description_format}'
                ]),
            ]
        )
        robot_description = {'robot_description': robot_description_content}
        node_robot_state_publisher = Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            output='screen',
            parameters=[robot_description]
        )
        return [node_robot_state_publisher]

    robot_controllers = PathJoinSubstitution(
        [
            FindPackageShare(PACKAGE_NAME), 
            'config',
            'ros2_control.yaml',
        ]
    )
    
    # RViz2 Node (NEW ADDITION)
    # rviz_node = Node(
    #     package='rviz2',
    #     executable='rviz2',
    #     name='rviz2',
    #     output='screen'# Only run if 'gui' argument is true
    # )

    gz_spawn_entity = Node(
        package='ros_gz_sim',
        executable='create',
        output='screen',
        arguments=['-topic', 'robot_description', '-name',
                   'gz_ros2_control_demos', '-allow_renaming', 'true'],
    )

    joint_state_broadcaster_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['joint_state_broadcaster'],
    )
    diff_drive_base_controller_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=[
            'diff_drive_base_controller',
            '--param-file',
            robot_controllers,
            ],
    )

    # Bridge
    bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        arguments=['/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock',
                   '/imu@sensor_msgs/msg/Imu@gz.msgs.IMU',],
        output='screen'
    )

    ld = LaunchDescription([
        # --- Declare Launch Argument for GUI/Rviz ---
        DeclareLaunchArgument(
            'gui',
            default_value='true',
            description='Set to true to launch Rviz2 and Gazebo with GUI'),
        
        # Launch your local world.launch.py file
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                [PathJoinSubstitution([FindPackageShare(PACKAGE_NAME),
                                       'launch',
                                       'world.launch.py'])]),
        ),
        # ----------------------------------------------------------
        RegisterEventHandler(
            event_handler=OnProcessExit(
                target_action=gz_spawn_entity,
                on_exit=[joint_state_broadcaster_spawner],
            )
        ),
        RegisterEventHandler(
            event_handler=OnProcessExit(
                target_action=joint_state_broadcaster_spawner,
                on_exit=[diff_drive_base_controller_spawner],
            )
        ),
        bridge,
        gz_spawn_entity,
        # rviz_node, # Rviz2 node added here
        
        # Launch Arguments
        DeclareLaunchArgument(
            'use_sim_time',
            default_value=use_sim_time,
            description='If true, use simulated clock'),
        DeclareLaunchArgument(
            'description_format',
            default_value='urdf',
            description='Robot description format to use, urdf or sdf'),
    ])
    ld.add_action(OpaqueFunction(function=robot_state_publisher))
    return ld