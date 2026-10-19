from launch import LaunchDescription
from launch_ros.actions import Node
from launch.substitutions import Command
from launch_ros.parameter_descriptions import ParameterValue
from launch.actions import RegisterEventHandler
from launch.event_handlers import OnProcessExit
import os
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    pkg_path = get_package_share_directory('pca_servo_hardware')
    
    robot_description_content = Command([
        'xacro ', os.path.join(pkg_path, 'urdf', 'pca_servo_robot.urdf.xacro')
    ])
    robot_description = ParameterValue(robot_description_content, value_type=str)

    controllers_yaml = os.path.join(pkg_path, 'config', 'controllers.yaml')

    control_node = Node(
        package='controller_manager',
        executable='ros2_control_node',
        parameters=[
            {'robot_description': robot_description},
            controllers_yaml
        ],
        output='screen',
    )

    joint_state_broadcaster_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['joint_state_broadcaster', '--controller-manager', '/controller_manager'],
        output='screen',
    )

    # Changement ici : on lance le forward_position_controller
    forward_controller_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['forward_position_controller', '--controller-manager', '/controller_manager'],
        output='screen',
    )

    delay_forward_controller = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=joint_state_broadcaster_spawner,
            on_exit=[forward_controller_spawner],
        )
    )

    return LaunchDescription([
        control_node,
        joint_state_broadcaster_spawner,
        delay_forward_controller
    ])