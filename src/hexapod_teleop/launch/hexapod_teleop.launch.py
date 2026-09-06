from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='hexapod_teleop',
            executable='bluetooth_node',
            name='bluetooth_node',
            output='screen',
            parameters=[{
                'port_name': '/dev/rfcomm0',
                'baud_rate': 115200,
                'joy_center': 512.0,
                'joy_deadzone': 20.0
            }]
        )
    ])