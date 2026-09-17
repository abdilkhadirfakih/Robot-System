import os


class FaceDatabase:

    def __init__(self, database_path=None):
        """
        Initialise la base de données des visages.

        Si aucun chemin n'est fourni, on utilise automatiquement
        ~/robot_ws/face_db
        """

        if database_path is None:
            database_path = os.path.expanduser("~/robot_ws/face_db")

        self.database_path = database_path

        if not os.path.exists(self.database_path):
            os.makedirs(self.database_path)

    def get_images(self):
        """
        Retourne la liste complète des images présentes
        dans le dossier face_db.
        """

        images = []

        for filename in os.listdir(self.database_path):

            if filename.lower().endswith((".jpg", ".jpeg", ".png")):

                images.append(
                    os.path.join(self.database_path, filename)
                )

        return images

    def __len__(self):
        return len(self.get_images())