FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Instalacja zależności systemowych
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    gcc-arm-none-eabi \
    libnewlib-arm-none-eabi \
    libstdc++-arm-none-eabi-newlib \
    git \
    python3 \
    python3-pip \
    tar \
    wget \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Pobranie Raspberry Pi Pico SDK (v2.1.0 wspiera RP2350 i Pico 2 W)
ENV PICO_SDK_PATH=/opt/pico-sdk
RUN git clone --recurse-submodules -b 2.1.0 https://github.com/raspberrypi/pico-sdk.git ${PICO_SDK_PATH}

# Pobranie FreeRTOS-Kernel i utworzenie symlinka dla Pico SDK (RP2350)
ENV FREERTOS_KERNEL_PATH=/opt/FreeRTOS-Kernel
RUN git clone --depth 1 -b main https://github.com/raspberrypi/FreeRTOS-Kernel.git /opt/FreeRTOS-Kernel
    
WORKDIR /workspace
