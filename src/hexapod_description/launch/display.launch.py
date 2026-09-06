import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
import xacro

def generate_launch_description():
    pkg_name = 'hexapod_description'
    pkg_path = os.path.join(get_package_share_directory(pkg_name))
    xacro_file = os.path.join(pkg_path, 'urdf', 'hexapod.urdf.xacro')
    
    doc = xacro.process_file(xacro_file)
    robot_description = {'robot_description': doc.toxml()}

    rsp_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[robot_description]
    )

    jsp_node = Node(
        package='joint_state_publisher',
        executable='joint_state_publisher',
        name='joint_state_publisher',
        parameters=[robot_description]
    )

    return LaunchDescription([
        rsp_node,
        jsp_node,
    ])