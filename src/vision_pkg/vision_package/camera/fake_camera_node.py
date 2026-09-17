import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
import numpy as np
from cv_bridge import CvBridge
import cv2
import os

class FakeCameraNode(Node):
    def __init__(self):
        super().__init__("fake_camera_node")
        
        self.publisher = self.create_publisher(Image, "/image_raw", 10)
        self.bridge = CvBridge()
        
        # Automatically targets the image in your workspace
        self.image_path = "/home/abdu/robot_project_ws/test_person.jpg"
        
        # Changed to 0.5 seconds (2 FPS) so WSL CPU doesn't get overloaded!
        self.timer = self.create_timer(0.5, self.publish_image) 
        
        self.get_logger().info("Fake Camera Started. Looking for image...")

    def publish_image(self):
        if os.path.exists(self.image_path):
            # If the image exists, load it
            frame = cv2.imread(self.image_path)
            frame = cv2.resize(frame, (640, 480))
        else:
            # If the image is missing, print a massive warning and send a gray square
            self.get_logger().error(f"IMAGE NOT FOUND AT: {self.image_path} !!!")
            frame = np.zeros((480, 640, 3), dtype=np.uint8)
            frame[:, :, :] = 50

        msg = self.bridge.cv2_to_imgmsg(frame, encoding="bgr8")
        msg.header.stamp = self.get_clock().now().to_msg()
        self.publisher.publish(msg)

def main(args=None):
    rclpy.init(args=args)
    node = FakeCameraNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == "__main__":
    main()