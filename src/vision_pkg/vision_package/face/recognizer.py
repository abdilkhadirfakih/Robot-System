import os
import cv2
import numpy as np

from insightface.app import FaceAnalysis

from vision_package.utils.config import KNOWN_FACES_DIR


class Recognizer:

    def __init__(self):

        print("===================================")
        print("Chargement de InsightFace...")

        self.app = FaceAnalysis(
            providers=["CPUExecutionProvider"]
        )

        self.app.prepare(ctx_id=0)

        self.database = []

        self.load_database()

        print(f"{len(self.database)} visage(s) chargé(s).")
        print("===================================")

    def load_database(self):

        self.database.clear()

        if not os.path.exists(KNOWN_FACES_DIR):
            os.makedirs(KNOWN_FACES_DIR)

        for filename in os.listdir(KNOWN_FACES_DIR):

            if not filename.lower().endswith((".jpg", ".jpeg", ".png")):
                continue

            image_path = os.path.join(KNOWN_FACES_DIR, filename)

            image = cv2.imread(image_path)

            if image is None:
                continue

            faces = self.app.get(image)

            if len(faces) == 0:
                continue

            self.database.append({

                "name": os.path.splitext(filename)[0],

                "embedding": faces[0].embedding

            })

    def recognize(self, frame):

        faces = self.app.get(frame)

        recognized_faces = []

        for face in faces:

            embedding = face.embedding

            best_name = "Unknown"

            best_score = -1.0

            for person in self.database:

                score = np.dot(
                    embedding,
                    person["embedding"]
                ) / (
                    np.linalg.norm(embedding)
                    * np.linalg.norm(person["embedding"])
                )

                if score > best_score:

                    best_score = score

                    best_name = person["name"]

            if best_score < 0.45:

                best_name = "Unknown"

            x1, y1, x2, y2 = face.bbox.astype(int)

            recognized_faces.append({

                "name": best_name,

                "score": float(best_score),

                "bbox": [
                    int(x1),
                    int(y1),
                    int(x2),
                    int(y2)
                ]

            })

        return recognized_faces