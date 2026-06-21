# Qt 6 toolchain plus the GStreamer media backbone of the inspection system.
FROM qt6-opencv:v1.0

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        libgstreamer1.0-dev \
        libgstreamer-plugins-base1.0-dev \
        gstreamer1.0-plugins-base \
        gstreamer1.0-plugins-good \
        gstreamer1.0-libav \
        gstreamer1.0-x \
        gstreamer1.0-tools \
        fonts-dejavu-core \
        fonts-inter \
    && rm -rf /var/lib/apt/lists/*
