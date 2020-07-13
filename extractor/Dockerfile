FROM tensorflow/tensorflow:1.15.2-gpu-py3

RUN apt-get update && apt-get install -y --no-install-recommends \
    ffmpeg \
    libsm6 \
    cuda-nvrtc-10-0

RUN pip3 --no-cache-dir install \
    pillow \
    matplotlib \
    numpy \
    scipy \
    sklearn \
    ffmpeg-python \
    pyyaml \
    opencv-python \
    mxnet-cu100

ENV MXNET_CUDNN_AUTOTUNE_DEFAULT=0
