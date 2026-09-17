# Vision Package ROS 2

A lightweight, optimized computer vision pipeline for ROS 2, designed specifically to feed human-detection data to a C++ Brain Node and Nav2.

## Requirements

- ROS 2 Jazzy
- Python 3.12
- OpenCV
- Ultralytics YOLOv8
- InsightFace

## Nodes

### 1. camera_node / fake_camera_node
Captures video frames and publishes them to the ROS 2 image topic.
- **Publishes:** `/image_raw` (`sensor_msgs/Image`)
- **Frame ID:** `camera_link` (Ready for TF tree and LiDAR sensor fusion)

### 2. object_detection_node
Analyzes incoming frames for Humans (Class 0) and performs face recognition. 
- **Subscribes:** `/image_raw`
- **Publishes:** `/vision_results` (`std_msgs/String` - JSON format)

*Example JSON Output:*
`{"objects": [{"id": 1, "bbox": [150, 40, 300, 400], "class": 0, "confidence": 0.92}], "faces": []}`

## Launching the Package

Build the workspace first:
```bash
colcon build --symlink-install --packages-select vision_package
source install/setup.bash

Use this command "ros2 launch vision_package vision.launch.py" if you are testing to run the fake camera node
otherwise use  this command "ros2 launch vision_package vision.launch.py use_fake_camera:=false" to run the real camera node

**What changed:**
1. I completely removed the `vision_manager_node` section.
2. I removed "Motion classification" from the notes, since we deleted the flawed 2D pixel math.
3. I clarified that the object detection is now specifically tuned for "Human Detection" to feed your C++ Brain Node.

<FollowUp label="Ready to write the Brain Node subscriber?" query="The Python vision package is officially 100% clean and documented. How do I write the C++ subscriber in my Brain Node to read the /vision_results JSON string?"/>
