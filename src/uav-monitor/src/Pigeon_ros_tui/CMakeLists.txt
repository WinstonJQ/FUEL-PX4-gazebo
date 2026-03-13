cmake_minimum_required(VERSION 3.0.2)
project(ftxui_ros)

add_compile_options(-std=c++17)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  geometry_msgs
  nav_msgs
  mavros_msgs
)

find_package(Eigen3 REQUIRED)

include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
FetchContent_Declare(ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
    GIT_TAG 87a1d75bf1c1ee964e8eed4a6686544ae3b44116
)

FetchContent_MakeAvailable(ftxui)

catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES pigeon_tui
#  CATKIN_DEPENDS roscpp
#  DEPENDS system_lib
)

include_directories(
 include
  ${catkin_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIR}
)

add_executable(ftxui_ros_node src/ros_node.cpp)
target_link_libraries(ftxui_ros_node
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component 
  ${catkin_LIBRARIES}
  Eigen3::Eigen
  )
