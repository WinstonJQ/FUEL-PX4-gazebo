#include <ros_node/ftxui.h>

void FtxuiVis::initSubscriber(ros::NodeHandle &nh)
{
    uav_position_sub = nh.subscribe("/mavros/vision_odom/odom", 10,
                                    &FtxuiVis::odometryCallback, this);
    uav_state_sub = nh.subscribe("/mavros/state", 10,
                                 &FtxuiVis::stateCallback, this);
    px4ctrl_mode_sub = nh.subscribe("/px4ctrl_mode", 10,
                                    &FtxuiVis::px4ctrlCallback, this);
    egoPlaner_mode_sub = nh.subscribe("/egoPlaner_mode", 10,
                                      &FtxuiVis::egoPlanerCallback, this);
    fastExploration_mode_sub = nh.subscribe("/exploration_mode", 10,
                                      &FtxuiVis::fastExplorationCallback, this);
}
std::wstring string_to_wstring(const std::string &str)
{
    std::wstring wstr(str.begin(), str.end());
    return L"[" + wstr + L"]";
}
double truncateToThreeDecimals(double value)
{
    return std::floor(value * 1000) / 1000; // Multiply, truncate, and divide
}

std::wstring toWStringWithPrecision(double value, int precision = 3)
{
    std::wostringstream woss;
    woss << std::fixed << std::setprecision(precision) << value;
    return woss.str();
}
auto create_mode_box = [](const std::wstring &mode_name, const std::string &current_mode)
{
    return ftxui::hbox({
               ftxui::text(mode_name) | ftxui::bold | ftxui::center |
               ftxui::bgcolor(current_mode == std::string(mode_name.begin(), mode_name.end())
                                  ? ftxui::Color::Green  // Highlight the current mode
                                  : ftxui::Color::Black) // Default background
           }) |
           ftxui::center;
};

void FtxuiVis::px4ctrlCallback(const std_msgs::String::ConstPtr &msg)
{
    px4ctrl_mode_ = msg->data;
}

void FtxuiVis::egoPlanerCallback(const std_msgs::String::ConstPtr &msg)
{
    egoPlaner_mode_ = msg->data;
}

void FtxuiVis::fastExplorationCallback(const std_msgs::String::ConstPtr &msg)
{
    fastExploration_mode_ = msg->data;
}

void FtxuiVis::stateCallback(const mavros_msgs::State::ConstPtr &msg)
{
    mavros_mode_ = msg->mode;
    connected_ = msg->connected;
    armed_ = msg->armed;
}
void FtxuiVis::odometryCallback(const nav_msgs::Odometry::ConstPtr &msg)
{
    // Extract position data
    position_ << truncateToThreeDecimals(msg->pose.pose.position.x),
        truncateToThreeDecimals(msg->pose.pose.position.y),
        truncateToThreeDecimals(msg->pose.pose.position.z);

    // Extract orientation data and truncate
    orientation_ << truncateToThreeDecimals(msg->pose.pose.orientation.x),
        truncateToThreeDecimals(msg->pose.pose.orientation.y),
        truncateToThreeDecimals(msg->pose.pose.orientation.z),
        truncateToThreeDecimals(msg->pose.pose.orientation.w);

    // Extract linear velocity data and truncate
    vel_ << truncateToThreeDecimals(msg->twist.twist.linear.x),
        truncateToThreeDecimals(msg->twist.twist.linear.y),
        truncateToThreeDecimals(msg->twist.twist.linear.z);

    // Log the extracted data to the terminal
    // ROS_INFO_STREAM("Position -> X: " << position_.x() << ", Y: " << position_.y() << ", Z: " << position_.z());
    // ROS_INFO_STREAM("Orientation -> X: " << orientation_[0] << ", Y: " << orientation_[1]
    //                                      << ", Z: " << orientation_[2] << ", W: " << orientation_[3]);
    // ROS_INFO_STREAM("Linear Velocity -> X: " << vel_.x() << ", Y: " << vel_.y() << ", Z: " << vel_.z());
}
std::wstring to_wstring(const char *t)
{
    return std::wstring(t, t + std::strlen(t));
}
void FtxuiVis::DrawTUI()
{
    ftxui::Element Document = ftxui::vbox({
                                  ftxui::hbox({
                                      ftxui::text(L"  𝗨 𝗔 𝗩 𝟭   ") | bgcolor(ftxui::Color::Green) | ftxui::center | ftxui::bold,
                                  }),
                                  ftxui::separator(),
                                  ftxui::hbox({
                                      ftxui::hbox({
                                          ftxui::text(L"State") | color(ftxui::Color::Red) | ftxui::bold | ftxui::center,
                                      }),
                                      ftxui::hbox({
                                          ftxui::text(L"    ") | color(ftxui::Color::Green) | ftxui::bold | ftxui::center,
                                      }),
                                      ftxui::hbox({
                                          ftxui::vbox({
                                              ftxui::hbox({
                                                  ftxui::text(L"CONNECTED: ") | ftxui::bold | ftxui::center,
                                                  ftxui::text(L"    ") | color(ftxui::Color::Green) | ftxui::bold | ftxui::center,
                                                  ftxui::text(connected_ ? L"[True]" : L"[False]") |
                                                      color(connected_ ? ftxui::Color::Green : ftxui::Color::Red) |
                                                      ftxui::bold | ftxui::align_right | ftxui::bold,
                                                  ftxui::separator(),
                                              }),
                                              ftxui::hbox({
                                                  ftxui::text(L"ARMED: ") | ftxui::bold | ftxui::center,
                                                  ftxui::text(L"    ") | color(ftxui::Color::Green) | ftxui::bold | ftxui::center,
                                                  ftxui::text(armed_ ? L"[True]" : L"[False]") |
                                                      color(armed_ ? ftxui::Color::Green : ftxui::Color::Red) |
                                                      ftxui::bold | ftxui::align_right | ftxui::bold,
                                                  ftxui::separator(),
                                              }),
                                              ftxui::hbox({
                                                  ftxui::text(L"MODEL: ") | ftxui::bold | ftxui::center,
                                                  ftxui::text(L"    ") | color(ftxui::Color::Green) | ftxui::bold | ftxui::center,
                                                  ftxui::text(string_to_wstring(mavros_mode_)) | ftxui::align_right | ftxui::bold,
                                                  ftxui::separator(),
                                              }),
                                          }),
                                      }),

                                      ftxui::hbox({
                                          ftxui::text(L"    ") | color(ftxui::Color::Green) | ftxui::bold | ftxui::center,
                                      }),
                                      ftxui::hbox({
                                          ftxui::text(L"Position  ") | color(ftxui::Color::Magenta) | ftxui::bold | ftxui::center,
                                      }),
                                      ftxui::hbox({
                                          ftxui::text(L"    ") | color(ftxui::Color::Green) | ftxui::bold | ftxui::center,
                                      }),
                                      ftxui::hbox({
                                          ftxui::vbox({
                                              ftxui::hbox({
                                                  ftxui::text(L"X: ") | ftxui::bold | ftxui::center,
                                                  ftxui::text(toWStringWithPrecision(position_.x())) | ftxui::underlined,
                                                  ftxui::separator(),
                                              }),
                                              ftxui::hbox({
                                                  ftxui::text(L"Y: ") | ftxui::bold | ftxui::center,
                                                  ftxui::text(toWStringWithPrecision(position_.y())) | ftxui::underlined,
                                                  ftxui::separator(),
                                              }),
                                              ftxui::hbox({
                                                  ftxui::text(L"Z: ") | ftxui::bold | ftxui::center,
                                                  ftxui::text(toWStringWithPrecision(position_.z())) | ftxui::underlined,
                                                  ftxui::separator(),
                                              }),
                                          }),
                                      }),
                                      ftxui::hbox({
                                          ftxui::text(L"    ") | color(ftxui::Color::Green) | ftxui::bold | ftxui::center,
                                      }),
                                      ftxui::hbox({
                                          ftxui::text(L"Velocity  ") | color(ftxui::Color::Blue) | ftxui::bold | ftxui::center,
                                      }),
                                      ftxui::hbox({
                                          ftxui::text(L"    ") | color(ftxui::Color::Green) | ftxui::bold | ftxui::center,
                                      }),
                                      ftxui::hbox({
                                          ftxui::vbox({
                                              ftxui::hbox({
                                                  ftxui::text(L"X: ") | ftxui::bold | ftxui::center,
                                                  ftxui::text(toWStringWithPrecision(vel_.x())) | ftxui::align_right | ftxui::underlined,
                                              }),
                                              ftxui::hbox({
                                                  ftxui::text(L"Y: ") | ftxui::bold | ftxui::center,
                                                  ftxui::text(toWStringWithPrecision(vel_.y())) | ftxui::align_right | ftxui::underlined,
                                              }),
                                              ftxui::hbox({
                                                  ftxui::text(L"Z: ") | ftxui::bold | ftxui::center,
                                                  ftxui::text(toWStringWithPrecision(vel_.z())) | ftxui::align_right | ftxui::underlined,
                                              }),
                                          }),
                                      }),
                                  }) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 100) |
                                      ftxui::center,
                                  ftxui::separator(),
                                  ftxui::hbox({
                                      ftxui::hbox({
                                          ftxui::text(L"PX4ctrl: ") | ftxui::bold | color(ftxui::Color::Yellow) | ftxui::center,
                                      }),
                                  }) |
                                      ftxui::center,
                                  ftxui::separator(),
                                  ftxui::hbox({
                                      ftxui::hbox({
                                          create_mode_box(L"MANUAL_CTRL", px4ctrl_mode_),
                                          ftxui::separator(),
                                      }),
                                      ftxui::hbox({
                                          create_mode_box(L"AUTO_HOVER", px4ctrl_mode_),
                                          ftxui::separator(),
                                      }),
                                      ftxui::hbox({
                                          create_mode_box(L"CMD_CTRL", px4ctrl_mode_),
                                          ftxui::separator(),
                                      }),
                                      ftxui::hbox({
                                          create_mode_box(L"AUTO_TAKEOFF", px4ctrl_mode_),
                                          ftxui::separator(),
                                      }),
                                      ftxui::hbox({
                                          create_mode_box(L"AUTO_LAND", px4ctrl_mode_),

                                      }),

                                  }) | ftxui::center |
                                      ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 100),
                                  ftxui::separator(),
                                  ftxui::hbox({
                                      ftxui::text(L"FSM mode: ") | ftxui::bold | color(ftxui::Color::Cyan) | ftxui::align_right,
                                  }) |
                                      ftxui::center,
                                  ftxui::separator(),
                                  ftxui::hbox({
                                      ftxui::hbox({
                                          create_mode_box(L"INIT", fastExploration_mode_),
                                          ftxui::separator(),
                                      }),
                                      ftxui::hbox({
                                          create_mode_box(L"WAIT_TRIGGER", fastExploration_mode_),
                                          ftxui::separator(),
                                      }),
                                      ftxui::hbox({
                                          create_mode_box(L"FINISH", fastExploration_mode_),
                                          ftxui::separator(),
                                      }),
                                      ftxui::hbox({
                                          create_mode_box(L"PLAN_TRAJ", fastExploration_mode_),
                                          ftxui::separator(),
                                      }),
                                      ftxui::hbox({
                                          create_mode_box(L"PUB_TRAJ", fastExploration_mode_),
                                          ftxui::separator(),
                                      }),
                                      ftxui::hbox({
                                          create_mode_box(L"EXEC_TRAJ", fastExploration_mode_),
                                          ftxui::separator(),
                                      }),
                                    

                                  }) | ftxui::center |
                                      ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 100),
                                  ftxui::separator(),
                                  ftxui::hbox({
                                      ftxui::text(L"Here is a Terminal for echo the uav’s state") | ftxui::bold | ftxui::center,
                                  }),
                              }) |
                              ftxui::center;

    Document = border(Document);

    auto screen = ftxui::Screen::Create(ftxui::Dimension::Full(), ftxui::Dimension::Fit(Document));
    Render(screen, Document);
    ClearTerminal();
    screen.Print();
}
void FtxuiVis::HideEcho(bool value)
{
    if (value == true)
    {
        tcgetattr(STDIN_FILENO, &org_term_);

        new_term_ = org_term_;

        new_term_.c_lflag &= ~(ECHO | ICANON);

        new_term_.c_cc[VMIN] = 0;
        new_term_.c_cc[VTIME] = 0;

        tcsetattr(STDIN_FILENO, TCSANOW, &new_term_);
    }
    else
        tcsetattr(STDIN_FILENO, TCSANOW, &org_term_);

    return;
}

void FtxuiVis::ClearTerminal()
{
    printf("\033[2J\033[1;1H");
    return;
}

void FtxuiVis::Spin()
{
    DrawTUI();
    return;
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "ftxui_ros_node");
    ros::NodeHandle nh;
    ros::Rate loop_rate(60);

    FtxuiVis robot_steering;
    robot_steering.initSubscriber(nh);

    while (ros::ok())
    {
        robot_steering.Spin();
        loop_rate.sleep();
        ros::spinOnce();
    }

    return 0;
}
