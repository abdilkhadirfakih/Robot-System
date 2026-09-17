from ultralytics import YOLO
from vision_package.utils.config import YOLO_MODEL

class Detector:
    def __init__(self):
        print("===================================")
        print("Loading YOLO Model...")
        self.model = YOLO(YOLO_MODEL)
        print("YOLO Model Loaded.")
        print("===================================")

    def detect(self, frame):
        # I fixed this function so it can only detect humans.
        results = self.model.track(
            source=frame,
            persist=True,
            classes=[0], 
            verbose=False
        )

        annotated_frame = frame.copy()
        if len(results) > 0:
            annotated_frame = results[0].plot()

        return annotated_frame, results