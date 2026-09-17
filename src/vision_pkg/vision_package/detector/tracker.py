class Tracker:

    def __init__(self):
        pass

    def update(self, results):

        tracked_objects = []

        if len(results) == 0:
            return tracked_objects

        result = results[0]

        if result.boxes is None:
            return tracked_objects

        for box in result.boxes:

            x1, y1, x2, y2 = box.xyxy[0].tolist()

            cls = int(box.cls.item())

            conf = float(box.conf.item())

            track_id = -1

            if box.id is not None:
                track_id = int(box.id.item())

            tracked_objects.append({

                "id": track_id,

                "bbox": [
                    int(x1),
                    int(y1),
                    int(x2),
                    int(y2)
                ],

                "class": cls,

                "confidence": conf

            })

        return tracked_objects