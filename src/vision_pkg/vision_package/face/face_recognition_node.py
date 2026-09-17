import cv2

import rclpy
from rclpy.node import Node

from sensor_msgs.msg import Image
from cv_bridge import CvBridge

from vision_package.face.recognizer import Recognizer


class FaceRecognitionNode(Node):

    def __init__(self):

        super().__init__("face_recognition_node")

        self.get_logger().info(
            "===== Face Recognition Node Started ====="
        )

        self.bridge = CvBridge()

        self.recognizer = Recognizer()

        self.subscription = self.create_subscription(
            Image,
            "/image_raw",
            self.image_callback,
            10
        )

    def image_callback(self, msg):

        try:

            frame = self.bridge.imgmsg_to_cv2(
                msg,
                desired_encoding="bgr8"
            )

        except Exception as e:

            self.get_logger().error(
                f"Erreur conversion image : {e}"
            )

            return

        try:

            faces = self.recognizer.recognize(frame)

        except Exception as e:

            self.get_logger().error(
                f"Erreur reconnaissance faciale : {e}"
            )

            return

        for face in faces:

            x1, y1, x2, y2 = face["bbox"]

            cv2.rectangle(
                frame,
                (x1, y1),
                (x2, y2),
                (0, 255, 0),
                2
            )

            cv2.putText(
                frame,
                f'{face["name"]} ({face["score"]:.2f})',
                (x1, y1 - 10),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.6,
                (0, 255, 0),
                2
            )

        cv2.imshow(
            "Face Recognition",
            frame
        )

        cv2.waitKey(1)

    def destroy_node(self):

        cv2.destroyAllWindows()

        super().destroy_node()


def main(args=None):

    rclpy.init(args=args)

    node = FaceRecognitionNode()

    try:

        rclpy.spin(node)

    except KeyboardInterrupt:

        pass

    finally:

        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":

    main()