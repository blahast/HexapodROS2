import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
import xacro

def generate_launch_description():
    teleop_arg = DeclareLaunchArgument(
        'teleop',
        default_value='bt',
        description='Choose teleop device: bt or keyboard'
    )

    pkg_description = get_package_share_directory('hexapod_description')
    xacro_file = os.path.join(pkg_description, 'urdf', 'hexapod.urdf.xacro')
    doc = xacro.process_file(xacro_file)
    robot_description = {'robot_description': doc.toxml()}

    rsp_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[robot_description]
    )

    hexapod_main_node = Node(
        package='hexapod_bringup',
        executable='hexapod_shared_process',
        output='screen',
        arguments=['--teleop', LaunchConfiguration('teleop')]
    )

    return LaunchDescription([
        teleop_arg,
        rsp_node,
        hexapod_main_node
    ])