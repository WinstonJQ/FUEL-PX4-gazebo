#ifndef FTXUI_H
#define FTXUI_H

#include <ros/ros.h>
#include <nav_msgs/Odometry.h>
#include <mavros_msgs/State.h>
#include <termios.h>
#include <Eigen/Dense>
#include <std_msgs/String.h> 
// Optional: Remove unused includes unless used in other methods.
#include "ftxui/component/container.hpp"
#include "ftxui/screen/string.hpp"
#include "ftxui/screen/color.hpp"


class FtxuiVis
{
public:
    FtxuiVis()
    {
        HideEcho(true);
        ClearTerminal();
    }

    ~FtxuiVis()
    {
        HideEcho(false);
        DrawTUI();
        ClearTerminal();
    }

    void HideEcho(bool value);
    void ClearTerminal();
    void DrawTUI();
    void Spin();

    // ROS-related methods
    void odometryCallback(const nav_msgs::Odometry::ConstPtr& msg);
    void stateCallback(const mavros_msgs::State::ConstPtr& msg);
    void initSubscriber(ros::NodeHandle& nh);
    void px4ctrlCallback(const std_msgs::String::ConstPtr &msg);
    void egoPlanerCallback(const std_msgs::String::ConstPtr &msg);
    void fastExplorationCallback(const std_msgs::String::ConstPtr &msg);

private:
    ros::Subscriber uav_position_sub;
    ros::Subscriber uav_state_sub;
    ros::Subscriber px4ctrl_mode_sub;
    ros::Subscriber egoPlaner_mode_sub;
    ros::Subscriber fastExploration_mode_sub;
    Eigen::Vector3d position_;
    Eigen::Vector3d vel_;
    Eigen::Vector4d orientation_;
    std::string mavros_mode_;
    bool armed_;
    bool connected_;
    std::string px4ctrl_mode_;
    std::string egoPlaner_mode_;
    std::string fastExploration_mode_;
    // Terminal control
    struct termios org_term_;
    struct termios new_term_;
};

#endif // FTXUI_H
