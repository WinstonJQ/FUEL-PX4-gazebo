#!/usr/bin/env python
"""
✅ 手动解锁和起飞脚本
功能：
  1. 解锁无人机
  2. 发布 takeoff 命令让 px4ctrl 自动起飞
  3. 等待 px4ctrl 进入悬停模式
  4. 提示用户在 RViz 中发布 2D Nav Goal 来触发探索算法
"""

import rospy
from mavros_msgs.msg import State
from mavros_msgs.srv import CommandBool
from quadrotor_msgs.msg import TakeoffLand

class SimpleUnlocker:
    def __init__(self):
        self.current_state = State()
        self.namespace = "iris_0"

        rospy.init_node('simple_unlock_node', anonymous=True)

        rospy.Subscriber(f'/{self.namespace}/mavros/state',
                         State, self.state_callback)

        self.arming_client = rospy.ServiceProxy(
            f'/{self.namespace}/mavros/cmd/arming', CommandBool)
        
        # Publisher for takeoff command to px4ctrl
        self.takeoff_pub = rospy.Publisher(
            f'/{self.namespace}/takeoff_land', TakeoffLand, queue_size=1)

    def state_callback(self, data):
        self.current_state = data

    def ask_user(self, text):
        ans = input(text + " (y/n): ")
        return ans.strip().lower() == "y"

    def wait_for_connection(self):
        rospy.loginfo("⏳ 等待飞控连接...")
        rate = rospy.Rate(10)
        while not rospy.is_shutdown() and not self.current_state.connected:
            rate.sleep()
        rospy.loginfo("✅ 飞控连接成功")

    def wait_for_hover(self, timeout=30):
        """等待 px4ctrl 进入 AUTO_HOVER 模式（即起飞完成）"""
        rospy.loginfo("⏳ 等待无人机起飞到悬停模式...")
        start_time = rospy.Time.now()
        rate = rospy.Rate(10)
        while not rospy.is_shutdown():
            if (rospy.Time.now() - start_time).to_sec() > timeout:
                rospy.logwarn("⚠️ 等待悬停超时，请检查 px4ctrl 状态")
                return False
            # 检查是否处于 OFFBOARD 模式且已解锁（起飞后通常是 AUTO_HOVER 状态，显示为 OFFBOARD）
            if self.current_state.armed and self.current_state.mode == "OFFBOARD":
                rospy.loginfo("✅ 无人机已进入悬停模式 (OFFBOARD)")
                return True
            rate.sleep()
        return False

    def run(self):
        # 等待连接
        self.wait_for_connection()

        # 步骤 1: 解锁
        if self.ask_user("🔓 是否解锁无人机？"):
            rospy.loginfo("正在解锁...")
            try:
                resp = self.arming_client(True)
                if resp.success:
                    rospy.loginfo("✅ 无人机已解锁！")
                else:
                    rospy.logwarn("❌ 解锁被拒绝，可能已在解锁状态")
            except Exception as e:
                rospy.logerr(f"解锁服务调用失败: {e}")
                return
        else:
            rospy.loginfo("取消解锁，程序退出。")
            return

        # 步骤 2: 发布 takeoff 命令
        rospy.sleep(1.0)  # 等待解锁稳定
        if self.ask_user("🚀 是否起飞？"):
            rospy.loginfo("正在发布起飞命令...")
            takeoff_msg = TakeoffLand()
            takeoff_msg.takeoff_land_cmd = TakeoffLand.TAKEOFF  # 1 = TAKEOFF
            self.takeoff_pub.publish(takeoff_msg)
            rospy.loginfo("✅ 起飞命令已发送给 px4ctrl")
        else:
            rospy.loginfo("取消起飞，程序退出。")
            return

        # 步骤 3: 等待起飞完成
        if self.wait_for_hover():
            rospy.loginfo("")
            rospy.loginfo("=" * 60)
            rospy.loginfo("🎉 无人机已起飞并进入悬停模式！")
            rospy.loginfo("")
            rospy.loginfo("📋 下一步操作：")
            rospy.loginfo("   在 RViz 中点击 '2D Nav Goal' 按钮")
            rospy.loginfo("   然后在地图上点击目标位置来触发探索算法")
            rospy.loginfo("=" * 60)
            rospy.loginfo("")
        else:
            rospy.logwarn("⚠️ 起飞可能未完成，请检查 px4ctrl 终端输出")

        # 保持节点运行，显示状态
        rate = rospy.Rate(1)
        rospy.loginfo("按 Ctrl+C 退出此程序")
        while not rospy.is_shutdown():
            if self.current_state.armed:
                rospy.loginfo_throttle(5, "🔒 状态: 已解锁 | 模式: %s", self.current_state.mode)
            else:
                rospy.loginfo_throttle(5, "🔓 状态: 未解锁 | 模式: %s", self.current_state.mode)
            rate.sleep()


if __name__ == "__main__":
    try:
        unlocker = SimpleUnlocker()
        unlocker.run()
    except rospy.ROSInterruptException:
        pass
