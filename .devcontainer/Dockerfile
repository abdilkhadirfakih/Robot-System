FROM osrf/ros:jazzy-desktop

# Install core tools
RUN apt-get update && apt-get install -y \
    python3-pip \
    git \
    nano \
    sudo \
    python3-colcon-common-extensions \
    && rm -rf /var/lib/apt/lists/*

# Create a generic non-root user for the Dev Container
ARG USERNAME=rosuser
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && echo "$USERNAME ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

USER $USERNAME
ENV HOME=/home/$USERNAME

# Pre-create InsightFace models directory to prevent download errors
RUN mkdir -p $HOME/.insightface/models

# Install Python Vision Dependencies and pin NumPy 1.26.4
RUN pip3 install --no-cache-dir --break-system-packages \
    ultralytics \
    insightface \
    lap \
    onnxruntime \
    scipy \
    matplotlib && \
    pip3 install --force-reinstall --no-cache-dir --break-system-packages "numpy==1.26.4"

# Download the TEB Local Planner and Costmap Converter source code into a temporary space
# (Your colleagues will build this alongside your custom packages in the workspace)
RUN mkdir -p $HOME/third_party_src && cd $HOME/third_party_src && \
    git clone -b ros2 https://github.com/rst-tu-dortmund/costmap_converter.git && \
    git clone -b ros2-master https://github.com/rst-tu-dortmund/teb_local_planner.git

# Auto-source ROS 2 in the bash terminal
RUN echo "source /opt/ros/jazzy/setup.bash" >> $HOME/.bashrc

CMD ["/bin/bash"]