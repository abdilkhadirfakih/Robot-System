import cv2

import rclpy
from rclpy.node import Node

from sensor_msgs.msg import Image
from cv_bridge import CvBridge

from vision_package.utils.config import (
    CAMERA_INDEX,
    CAMERA_WIDTH,
    CAMERA_HEIGHT,
    CAMERA_FPS,
)


class CameraNode(Node):

    def __init__(self):
        super().__init__("camera_node")

        self.declare_parameter("camera_index", CAMERA_INDEX)
        self.declare_parameter("width", CAMERA_WIDTH)
        self.declare_parameter("height", CAMERA_HEIGHT)
        self.declare_parameter("fps", CAMERA_FPS)

        camera_index = self.get_parameter(
            "camera_index"
        ).value

        width = self.get_parameter("width").value
        height = self.get_parameter("height").value
        fps = self.get_parameter("fps").value

        self.publisher = self.create_publisher(
            Image,
            "/image_raw",
            10
        )

        self.bridge = CvBridge()

        self.cap = cv2.VideoCapture(camera_index)

        if not self.cap.isOpened():
            self.get_logger().error(
                f"Impossible d'ouvrir la caméra {camera_index}"
            )
        else:
            self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
            self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)
            self.cap.set(cv2.CAP_PROP_FPS, fps)

            self.get_logger().info(
                f"Caméra ouverte : index={camera_index}, "
                f"{width}x{height}@{fps} FPS"
            )

        timer_period = 1.0 / fps

        self.timer = self.create_timer(
            timer_period,
            self.publish_frame
        )

        self.get_logger().info("Camera Node Started")

    def publish_frame(self):

        if not self.cap.isOpened():
            return

        ret, frame = self.cap.read()

        if not ret:
            self.get_logger().warning(
                "Impossible de lire une image caméra"
            )
            return

        msg = self.bridge.cv2_to_imgmsg(
            frame,
            encoding="bgr8"
        )

        msg.header.stamp = self.get_clock().now().to_msg()
        # i added this so the brain node could know which fr
        msg.header.frame_id = "camera_link"
        
        self.publisher.publish(msg)

    def destroy_node(self):

        if hasattr(self, "cap") and self.cap.isOpened():
            self.cap.release()

        super().destroy_node()


def main(args=None):

    rclpy.init(args=args)

    node = CameraNode()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()