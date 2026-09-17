import os
from glob import glob
from setuptools import find_packages, setup

package_name = "vision_package"

setup(
    name=package_name,
    version="0.0.1",
    packages=find_packages(exclude=["test"]),
    data_files=[
        (
            "share/ament_index/resource_index/packages",
            ["resource/" + package_name],
        ),
        (
            "share/" + package_name,
            ["package.xml"],
        ),
        (
            "share/" + package_name + "/launch",
            glob("launch/*.launch.py"),
        ),
       
        (
            os.path.join("share", package_name, "config"), 
            glob("config/*")
        ),

        (
            os.path.join("share", package_name, "models"), 
            glob("models/*")
        ),
        (
            os.path.join("share", package_name, "face_db"),
            glob("face_db/*")
        ),
    ],
    install_requires=[
        "setuptools",
        "ultralytics",
        "opencv-python",
        "numpy",
        "insightface",
        "onnxruntime",
        "lap",
    ],
    zip_safe=True,
    maintainer="ahmatdif",
    maintainer_email="ahmatdif@todo.todo",
    description="Computer Vision Package for ROS2 Robot",
    license="Apache-2.0",
    extras_require={
        "test": ["pytest"],
    },
    entry_points={
        "console_scripts": [
            "camera_node = vision_package.camera.camera_node:main",
            "object_detection_node = vision_package.detector.object_detection_node:main",
            "face_recognition_node = vision_package.face.face_recognition_node:main",
            "fake_camera_node = vision_package.camera.fake_camera_node:main",
        ],
    },
)