#!/bin/bash

# Get the drone_id from the environment variable
DRONE_ID=${DRONE_ID:-0}  # Default to 0 if not set

# Wait for some time (optional)
sleep 3

# Publish to the appropriate topic
# ✅ 修正 topic 名称，与 launch 文件中的 remap 保持一致
rostopic pub /iris_${DRONE_ID}/takeoff_land quadrotor_msgs/TakeoffLand "takeoff_land_cmd: 1"
