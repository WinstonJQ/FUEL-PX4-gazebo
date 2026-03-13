#!/bin/bash

# 创建新终端执行命令函数（每个命令独立标签页/窗口）
run_in_new_tab() {
    gnome-terminal --tab --title="$1" -- bash -ic "echo '执行命令: $2'; $2; exec bash"
}

# 由于PX4启动较慢，优先启动并延长等待时间
run_in_new_tab "PX4仿真" "roslaunch px4 indoor3.launch"
echo "等待PX4启动..."
sleep 25  # 重要！PX4启动需要足够时间，可根据实际情况调整

# 按顺序执行后续命令
run_in_new_tab "中转位姿真值" "python3 scripts/multirotor_communication.py iris 0"
sleep 5

run_in_new_tab "获取位姿真值" "python3 scripts/get_local_pose.py iris 1"
sleep 5

run_in_new_tab "RVIZ" "roslaunch exploration_manager rviz.launch"
sleep 5

run_in_new_tab "控制器" "roslaunch px4ctrl singl_run.launch"
sleep 5

run_in_new_tab "启动探索" "roslaunch exploration_manager exploration.launch"
sleep 5

run_in_new_tab "监视器" "roslaunch ftxui_ros single_start.launch"
sleep 5

run_in_new_tab "解锁" "python3 scripts/unlock.py"

echo "所有命令已启动！建议手动排列窗口位置以便观察。"
