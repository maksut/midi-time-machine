#!/bin/bash

rm -rf build pre_build

podman run --rm \
  -v $(pwd)/../../..:/workspace \
  --userns=keep-id \
  juce_builder \
  bash -c "cd midi-time-machine/Builds/LinuxMakefile && make CONFIG=Release -j$(nproc)"
