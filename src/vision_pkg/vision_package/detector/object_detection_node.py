import json
import cv2
import os    
import rclpy
from rclpy.node import Node

from sensor_msgs.msg import Image
from std_msgs.msg import String
from cv_bridge import CvBridge

from vision_package.detector.detector import Detector
from vision_package.detector.tracker import Tracker
from vision_package.face.recognizer import Recognizer
from vision_package.utils.config import SHOW_CAMERA

class ObjectDetectionNode(Node):

    def __init__(self):

        super().__init__("object_detection_node")

        self.get_logger().info("===== Human Detection Node Started =====")

        self.bridge = CvBridge()
        self.detector = Detector()
        self.tracker = Tracker()
        self.recognizer = Recognizer()

        self.subscription = self.create_subscription(
            Image,
            "/image_raw",
            self.image_callback,
            10
        )

        self.publisher = self.create_publisher(
            String,
            "/vision_results",
            10
        )

    def image_callback(self, msg):

        frame = self.bridge.imgmsg_to_cv2(
            msg,
            desired_encoding="bgr8"
        )

        annotated_frame, results = self.detector.detect(frame)
        tracked_objects = self.tracker.update(results)
        faces = self.recognizer.recognize(frame)

        for face in faces:

            x1, y1, x2, y2 = face["bbox"]

            cv2.rectangle(
                annotated_frame,
                (x1, y1),
                (x2, y2),
                (0, 255, 0),
                2
            )

            cv2.putText(
                annotated_frame,
                face["name"],
                (x1, y1 - 10),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.6,
                (0, 255, 0),
                2
            )

        vision_result = {

            "objects": tracked_objects,

            "faces": faces

        }

        ros_msg = String()
        ros_msg.data = json.dumps(vision_result)
        self.publisher.publish(ros_msg)

        if SHOW_CAMERA and os.environ.get("DISPLAY"):
            cv2.imshow("Robot Vision", annotated_frame)
            cv2.waitKey(1)

    def destroy_node(self):

        cv2.destroyAllWindows()

        super().destroy_node()


def main(args=None):

    rclpy.init(args=args)

    node = ObjectDetectionNode()

    rclpy.spin(node)

    node.destroy_node()

    rclpy.shutdown()


if __name__ == "__main__":
    main()