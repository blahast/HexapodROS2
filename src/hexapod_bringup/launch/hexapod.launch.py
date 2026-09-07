import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
import xacro

def generate_launch_description():
    # Load and process the xacro file to get the robot description
    pkg_description = get_package_share_directory('hexapod_description')
    xacro_file = os.path.join(pkg_description, 'urdf', 'hexapod.urdf.xacro')
    doc = xacro.process_file(xacro_file)
    robot_description = {'robot_description': doc.toxml()}

    # Robot State Publisher
    rsp_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[robot_description]
    )

    # Hexapod main node
    hexapod_main_node = Node(
        package='hexapod_bringup',
        executable='hexapod_shared_process',
        output='screen'
    )

    return LaunchDescription([
        rsp_node,
        hexapod_main_node
    ])