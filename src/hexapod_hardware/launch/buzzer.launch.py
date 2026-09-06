from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='hexapod_hardware',
            executable='buzzer_driver_node',
            name='buzzer_driver',
            output='screen',
            parameters=[
                {'buzzer_pin': 0}
            ]
        )
    ])
