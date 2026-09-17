from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch.conditions import IfCondition, UnlessCondition
from launch_ros.actions import Node

def generate_launch_description():
    camera_index = LaunchConfiguration("camera_index")
    use_fake_camera = LaunchConfiguration("use_fake_camera")

    return LaunchDescription([
        DeclareLaunchArgument(
            "camera_index", default_value="0", description="Camera USB index"
        ),
        
        # 1. Create a boolean switch (defaults to true so WSL doesn't crash)
        DeclareLaunchArgument(
            "use_fake_camera", default_value="true", description="Toggle fake camera"
        ),

        # 2. Fake Camera runs ONLY IF use_fake_camera is true
        Node(
            package="vision_package",
            executable="fake_camera_node",
            name="fake_camera_node",
            output="screen",
            condition=IfCondition(use_fake_camera),
            parameters=[{"camera_index": camera_index}]
        ),

        # 3. Real Camera runs ONLY IF use_fake_camera is false
        Node(
            package="vision_package",
            executable="camera_node",
            name="camera_node",
            output="screen",
            condition=UnlessCondition(use_fake_camera),
            parameters=[{"camera_index": camera_index}]
        ),

        # 4. Human Detector ALWAYS runs
        Node(
            package="vision_package",
            executable="object_detection_node",
            name="object_detection_node",
            output="screen"
        ),
    ])