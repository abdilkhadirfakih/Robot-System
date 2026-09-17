import os
from ament_index_python.packages import get_package_share_directory
YOLO_MODEL = "yolov8n.pt"

try:
    pkg_share = get_package_share_directory("vision_package")
    KNOWN_FACES_DIR = os.path.join(pkg_share, "face_db")
except Exception:
    KNOWN_FACES_DIR = os.path.join(os.path.dirname(os.path.dirname(__file__)), "face_db")

CAMERA_INDEX = 0
CAMERA_WIDTH = 640
CAMERA_HEIGHT = 480
CAMERA_FPS = 30

CONFIDENCE_THRESHOLD = 0.45
SHOW_CAMERA = True