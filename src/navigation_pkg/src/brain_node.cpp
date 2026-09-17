#include <cmath>
#include <vector>
#include "costmap_converter_msgs/msg/obstacle_array_msg.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/battery_state.hpp"
#include "robot_state.hpp"
#include "mission_manager.hpp"
#include "nav_msgs/msg/occupancy_grid.hpp"
#include "nav_msgs/msg/map_meta_data.hpp"
#include <chrono>
#include <functional>
#include <nlohmann/json.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <std_msgs/msg/string.hpp>
#include "rclcpp_action/rclcpp_action.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"
#include "navigation_client.hpp"
#include <memory>

using json = nlohmann::json;
namespace Action = rclcpp_action;

using namespace rclcpp;
using namespace std;


class BrainNode : public Node {
    public:
        BrainNode() : Node("brain_node"), Nav2ActionClient(this){
            SetBattaryDataSubscriber();
            SetMapSubscriber();
            SetLiderSubscriber();
            SetVisionResultsSubscriber();
            SetParameters();
            SetNav2ActionClient();
        }

        void SetParameters(){
            this->MissionID = this->declare_parameter<int>("mission_id", 0);
            this->BaseCoordinate.x = this->declare_parameter<double>("base_coordinate_x", 0.0);
            this->BaseCoordinate.y = this->declare_parameter<double>("base_coordinate_y", 0.0);
            this->BaseCoordinate.OrientationW = this->declare_parameter<double>("base_coordinate_orientation_w", 0);
        }

        void SetNav2ActionClient(){
            Nav2ActionClient.SetAndInitializeNav2ActionClient(this);
        }

        void SetBattaryDataSubscriber(){
            QoS QualityOfServiceProfile = QoS(KeepLast(10)).reliable().durability_volatile();
            BattaryDataSubscriber = this->create_subscription<sensor_msgs::msg::BatteryState>(
                "/battery_state", QualityOfServiceProfile, std::bind(&BrainNode::SendBattaryDataToMissionManager, this, std::placeholders::_1));
        }

        void SendBattaryDataToMissionManager(const sensor_msgs::msg::BatteryState::SharedPtr message){
            float BattaryPercentage = message->percentage * 100.0f;
            missionManager.ReceiveBattaryData(message);
            RCLCPP_INFO(this->get_logger(), "Received battary Data and this is the percentage: %.2f", BattaryPercentage);
        }

        void SetMapSubscriber(){
            QoS QualityOfServiceProfile = QoS(KeepLast(10)).reliable().durability_volatile();
            MapSubscriber = this->create_subscription<nav_msgs::msg::OccupancyGrid>(
                "/map", QualityOfServiceProfile, std::bind(&BrainNode::SendMapToMissionManager, this, std::placeholders::_1));
        }

        void SendMapToMissionManager(const nav_msgs::msg::OccupancyGrid::SharedPtr message){
            missionManager.ReceiveRTAmap(message);
            RCLCPP_INFO(this->get_logger(), "Received the RTAmap and it will get proccesed inside MissionManager");
        }

        void SetLiderSubscriber(){
            QoS QualityOfServiceProfile = QoS(KeepLast(10)).reliable().durability_volatile();
            LiderSubscriber = this->create_subscription<sensor_msgs::msg::LaserScan>(
                "/scan", QualityOfServiceProfile, std::bind(&BrainNode::LiderScanDataCallback, this, std::placeholders::_1));
        }

        void LiderScanDataCallback(const sensor_msgs::msg::LaserScan::SharedPtr message){
            RCLCPP_INFO(this->get_logger(), "Received lider data and it is being used in analayzing...");
            missionManager.ReceiveLiderData(message);
        }

        void SetVisionResultsSubscriber(){
            QoS QualityOfServiceProfile = QoS(KeepLast(10)).reliable().durability_volatile();
            VisionResultSubscriber = this->create_subscription<std_msgs::msg::String>(
                "/vision_results", QualityOfServiceProfile, std::bind(&BrainNode::VisionResultsCallback, this, std::placeholders::_1
            ));
        }

        void VisionResultsCallback(const std_msgs::msg::String::SharedPtr message){
            RCLCPP_INFO(this->get_logger(), "Received vision results and it is being used in analayzing...");
            json VisionResults = json::parse(message->data);
            missionManager.ReceiveVisionResults(VisionResults);
        }
        
        void PatrolResultCallback(const Action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::WrappedResult & Result) {
            
            if (CurrentMission != Missions::Patrol) 
                return;

            if (Result.code == Action::ResultCode::SUCCEEDED) {
                PatrolWayPointsCounter++; 
                RCLCPP_INFO(this->get_logger(), "Reached waypoint %d.", PatrolWayPointsCounter);
            } else {
                RCLCPP_WARN(this->get_logger(), "Waypoint failed. Generating a new one to replace it...");
                }

            if (PatrolWayPointsCounter < 15) {
                RCLCPP_INFO(this->get_logger(), "Generating random coordinate...");
                Coordinate randomCoordinate = missionManager.GenerateAndEvaluateRandomCoordinates();

                RCLCPP_INFO(this->get_logger(), "Coordinate generated successfully! x:%.2f", randomCoordinate.x);
                Nav2ActionClient.SendNavigationGoal(
                    randomCoordinate.x, 
                    randomCoordinate.y, 
                    1.0, 
                    std::bind(&BrainNode::PatrolResultCallback, this, std::placeholders::_1)
                );
            } else {
                CurrentMission = Missions::NoMission;
                CurrentState = RobotState::Idle;
                RCLCPP_INFO(this->get_logger(), "Patrol successfully finished!");
                }
        }    

        bool ExecutePatrolMission(){
        
            if(this->CurrentState != RobotState::Idle){
                RCLCPP_INFO(this->get_logger(), "Mission can't get executed at the momment.");
                return false;
            }
            if(missionManager.CheckMissionFeasibility() == false){
                RCLCPP_INFO(this->get_logger(), "Patrol Mission is not feasible due to low battery.");
                MissionID = 2;
                CurrentMission = Missions::ReturnToBase;
                CurrentState = RobotState::Idle;
                return false;
                }

            CurrentMission = Missions::Patrol;
            CurrentState = RobotState::Navigating;    
            PatrolWayPointsCounter = 0;

            Coordinate randomCoordinate = missionManager.GenerateAndEvaluateRandomCoordinates();
            RCLCPP_INFO(this->get_logger(), "Coordinate generated successfully! x:%.2f", randomCoordinate.x);
            Nav2ActionClient.SendNavigationGoal(
                randomCoordinate.x, 
                randomCoordinate.y, 
                1.0, 
                std::bind(&BrainNode::PatrolResultCallback, this, std::placeholders::_1)
            );
            
            return true;
        }

        void ReturnToBaseResultCallback(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::WrappedResult & result) {
            
            if (CurrentMission != Missions::ReturnToBase) 
                return;

            if (result.code == rclcpp_action::ResultCode::SUCCEEDED) {
                RCLCPP_INFO(this->get_logger(), "Robot has successfully returned to base!");
                CurrentState = RobotState::Idle;
                CurrentMission = Missions::NoMission;
                } else {
                RCLCPP_ERROR(this->get_logger(), "Failed to return to base! Manual rescue required.");
                    }
        }        

        bool ExecuteReturnToBaseMission(){
            
            if(CurrentState != RobotState::Idle){
                RCLCPP_INFO(this->get_logger(), "Mission can't get executed at the momment.");
                return false;
                }

            CurrentMission = Missions::ReturnToBase;
            CurrentState = RobotState::Navigating;

            Nav2ActionClient.SendNavigationGoal(
                BaseCoordinate.x, 
                BaseCoordinate.y, 
                BaseCoordinate.OrientationW, 
                std::bind(&BrainNode::ReturnToBaseResultCallback, this, std::placeholders::_1)
            );
        
            return true;
        }
        
        void StartMissionLoop() {
            bool MissionStarted = false;
            switch (this->MissionID) {
                case 1:
                    RCLCPP_INFO(this->get_logger(), "Starting Patrol Mission...");
                    MissionStarted = ExecutePatrolMission();
                
                    if (!MissionStarted && this->MissionID == 2) {
                        RCLCPP_WARN(this->get_logger(), "Patrol aborted. Emergency override to Return To Base!");
                        StartMissionLoop(); // Restart the loop so it catches Case 2!
                    }
                    break;
                    
                case 2:
                    RCLCPP_INFO(this->get_logger(), "Starting Return to Base Mission...");
                    MissionStarted = ExecuteReturnToBaseMission();
                    
                    if (!MissionStarted) {
                        RCLCPP_ERROR(this->get_logger(), "CRITICAL: Cannot return to base! Manual rescue required.");
                    }
                    break; 
                    
                case 0:
                    RCLCPP_INFO(this->get_logger(), "Robot is idle. No mission selected.");
                    break;
                    
                default:
                    RCLCPP_WARN(this->get_logger(), "Unknown Mission ID!");
                    break;
            }
        }

    private:
        RobotState CurrentState = RobotState::Idle;
        Missions CurrentMission = Missions::NoMission;
        int MissionID;
        ThreeDimensionalCoordinate BaseCoordinate;
        NavigationClient Nav2ActionClient;
        Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr MapSubscriber;
        Subscription<sensor_msgs::msg::BatteryState>::SharedPtr BattaryDataSubscriber;
        Subscription<sensor_msgs::msg::LaserScan>::SharedPtr LiderSubscriber;
        Subscription<std_msgs::msg::String>::SharedPtr VisionResultSubscriber;
        MissionManager missionManager = MissionManager();
        int PatrolWayPointsCounter = 0;

};


int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto brainNode = std::make_shared<BrainNode>();
    brainNode->StartMissionLoop();
    rclcpp::spin(brainNode);
    rclcpp::shutdown();
    return 0;
}