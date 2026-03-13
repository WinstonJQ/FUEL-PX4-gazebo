# FUEL-PX4-gazebo

基于 [FUEL](https://github.com/HKUST-Aerial-Robotics/FUEL) 的无人机自主探索算法，移植到 **PX4 SITL 仿真环境**。

## 依赖

- **ROS Noetic**
- **PX4 Firmware** (SITL 仿真，必须安装)
- **MAVROS**
- **Gazebo 11**

> **关于 XTDrone**: 本项目已将 XTDrone 的关键通信脚本内置（`scripts/multirotor_communication.py` 和 `scripts/get_local_pose.py`），**无需单独安装 XTDrone**。

## 快速开始

### 1. 安装 PX4

请参考 [PX4 官方文档](https://docs.px4.io/main/en/dev_setup/dev_env_linux_ubuntu.html) 安装 PX4 固件，确保 `roslaunch px4 indoor3.launch` 可以正常运行。

### 2. 放置 launch 文件（可选）

本项目使用 `indoor3.launch` 启动 PX4 仿真。你可以将其复制到 PX4 目录：

```bash
cp launch/indoor3.launch ~/PX4_Firmware/launch/
```

### 3. 编译

```bash
cd ~/FUEL-PX4-gazebo
catkin_make
source devel/setup.bash
```

### 4. 一键启动与手动控制

```bash
./fast_start.sh
```

脚本将自动启动以下节点：
- PX4 SITL 仿真（调用 `roslaunch px4 indoor3.launch`）
- 位姿真值发布节点
- MAVROS 通信节点
- px4ctrl 控制器
- Exploration 规划器
- RViz 可视化
- **监视器（ftxui_ros）**
- 解锁控制终端

然后进入**手动控制流程**（模拟实际飞行器的操作流程）：

```
🔓 是否解锁无人机？ (y/n): y
🚀 是否起飞？ (y/n): y
⏳ 等待无人机起飞到悬停模式...
✅ 无人机已进入悬停模式！

📋 下一步操作：
   在 RViz 中点击 '2D Nav Goal' 按钮
   然后在地图上点击目标位置来触发探索算法
```

> 注：手动控制流程是为了模拟真实无人机的操作流程（解锁→起飞→任务执行），而非自动启动。

## 系统组件

| 组件 | 说明 | 启动方式 |
|------|------|----------|
| PX4 SITL | Gazebo 仿真环境 + PX4 飞控固件 | `roslaunch px4 indoor3.launch` |
| 位姿真值 | Gazebo → MAVROS 位姿转换 | `scripts/get_local_pose.py` |
| 通信中转 | MAVROS 通信层 | `scripts/multirotor_communication.py` |
| px4ctrl | PX4 飞控控制器 | `roslaunch px4ctrl singl_run.launch` |
| Exploration | FUEL 探索算法 | `roslaunch exploration_manager exploration.launch` |
| 监视器 | 终端状态监控界面 | `roslaunch ftxui_ros single_start.launch` |
| 解锁控制 | 交互式解锁/起飞 | `scripts/unlock.py` |

## 关键修改对比

### 1. ROS 话题映射

**原始 FUEL** (`~/FUEL/src/FUEL/fuel_planner/exploration_manager/launch/exploration.launch`):
```xml
<arg name="odom_topic" value="/state_ukf/odom" />
<arg name="sensor_pose_topic" value="/pcl_render_node/sensor_pose"/>
<arg name="depth_topic" value="/pcl_render_node/depth"/>
```

**本版本** (`src/fuel_planner/exploration_manager/launch/exploration.launch`):
```xml
<arg name="odom_topic" value="/iris_0/mavros/vision_odom/odom" />
<arg name="sensor_pose_topic" value="/iris_0/mavros/vision_pose/pose"/>
<arg name="depth_topic" value="/iris_0/realsense/depth_camera/depth/image_raw"/>
```

### 2. 相机内参

PX4 仿真中的 Realsense 相机与原始 FUEL 的相机参数不同：

| 参数 | 原始 FUEL | 本版本 (iris_realsense_camera) |
|------|-----------|-------------------------------|
| cx | 321.04638671875 | **320.5** |
| cy | 243.44969177246094 | **240.5** |
| fx | 387.229248046875 | **554.254691191187** |
| fy | 387.229248046875 | **554.254691191187** |

### 3. 控制指令话题

**原始 FUEL** (`src/fuel_planner/exploration_manager/launch/exploration.launch`):
```xml
<node pkg="plan_manage" name="traj_server" type="traj_server" output="screen">
    <remap from="/position_cmd" to="planning/pos_cmd"/>
```

**本版本**:
```xml
<node pkg="plan_manage" name="traj_server" type="traj_server" output="screen">
    <remap from="/position_cmd" to="/iris_0/position_cmd"/>
```


### 4. px4ctrl 源码修改

为了支持手动解锁模式，修改了 `src/realflight_modules/px4ctrl/src/PX4CtrlParam.cpp`：

**原始逻辑**:
```cpp
if ( takeoff_land.no_RC && (!takeoff_land.enable_auto_arm || !takeoff_land.enable) )
{
    takeoff_land.no_RC = false;
    ROS_ERROR("\"no_RC\" is only allowd with both \"auto_takeoff_land\" and \"enable_auto_arm\" enabled.");
}
```

**修改后**:
```cpp
// 允许 no_RC 模式在手动解锁的情况下工作
if ( takeoff_land.no_RC && !takeoff_land.enable )
{
    takeoff_land.no_RC = false;
    ROS_ERROR("\"no_RC\" is only allowd with \"auto_takeoff_land\" enabled.");
}
```

配合的配置文件 (`src/realflight_modules/px4ctrl/config/ctrl_param_fpv.yaml`):
```yaml
auto_takeoff_land:
    enable: true             # 启用自动起降
    enable_auto_arm: false   # 禁用自动解锁（改为手动解锁）
    no_RC: true              # 无遥控模式
```

## 新增组件

| 文件 | 说明 | 来源 |
|------|------|------|
| `fast_start.sh` | 一键启动脚本，协调 7 个终端节点 | 本项目新增 |
| `scripts/unlock.py` | 手动解锁/起飞控制脚本 | 本项目新增 |
| `scripts/get_local_pose.py` | Gazebo 位姿真值 → MAVROS 格式转换 | 源自 XTDrone |
| `scripts/multirotor_communication.py` | MAVROS 通信中转 | 源自 XTDrone |
| `src/realflight_modules/px4ctrl/` | PX4 飞控控制器 | 独立控制器 |
| `src/uav-monitor/` | 终端监视器（ftxui_ros）| 独立组件 |

## 致谢

本项目基于以下开源工程构建，特此感谢：

- **[FUEL](https://github.com/HKUST-Aerial-Robotics/FUEL)** 
- **[XTDrone](https://github.com/robin-shaun/XTDrone)**
- **[Fast-Exploration](https://github.com/XXLiu-HNU/Fast-Exploration)**

## License

MIT License
