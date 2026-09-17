#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"
#include <functional>
#include <memory>

using namespace rclcpp;
using namespace std;

class NavigationClient{

public:
    
    NavigationClient(rclcpp::Node* parentNodePtr){
        SetAndInitializeNav2ActionClient(parentNodePtr);
    };


    void SetAndInitializeNav2ActionClient(rclcpp::Node* parentNodePtr){
        Client = rclcpp_action::create_client<nav2_msgs::action::NavigateToPose>(parentNodePtr, "navigate_to_pose");
        this->ParentNodePtr = parentNodePtr;
        BindAllGoalOptions();
    }

    void SendNavigationGoal(double x, double y, double OrientationW, std::function<void(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::WrappedResult&)> Result ){

        if(!CheckNav2ActionServer())
            RCLCPP_INFO(ParentNodePtr->get_logger(), "Waiting for navigation action server to be available...");
        
        ExternalResult = Result;
        nav2_msgs::action::NavigateToPose::Goal NavigationGoalMessage;
        NavigationGoalMessage.pose.header.frame_id = "map";
        NavigationGoalMessage.pose.pose.position.x = x;
        NavigationGoalMessage.pose.pose.position.y = y;

        NavigationGoalMessage.pose.pose.orientation.x = 0.0;
        NavigationGoalMessage.pose.pose.orientation.y = 0.0;
        NavigationGoalMessage.pose.pose.orientation.z = 0.0;
        NavigationGoalMessage.pose.pose.orientation.w = OrientationW;

        Client->async_send_goal(NavigationGoalMessage, sendGoalOptions);
        RCLCPP_INFO(ParentNodePtr->get_logger(), "[MOCK] Pretending to drive to x:%.2f, y:%.2f...", x, y);
    
   }
   
private:
    rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SharedPtr Client;
    rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SendGoalOptions sendGoalOptions;
    rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::SharedPtr HandleGoal;
    std::function<void(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::WrappedResult&)> ExternalResult;
    rclcpp::Node* ParentNodePtr;

    bool CheckNav2ActionServer(){

        if(Client->wait_for_action_server(std::chrono::seconds(10))){
            RCLCPP_INFO(ParentNodePtr->get_logger(), "Navigation action server is available");
            return true;
        }
        RCLCPP_ERROR(ParentNodePtr->get_logger(), "Navigation action server is not available");
        return false;
    }

    void BindAllGoalOptions(){
        sendGoalOptions.goal_response_callback = [this](const auto& goalHandle) { 
            this->HandleGoalAcceptedCallback(goalHandle);
        };

        sendGoalOptions.feedback_callback = [this](const auto& goalHandle, const auto& feedback) {
            this->HandleFeedbackCallback(goalHandle, feedback);
        };

        sendGoalOptions.result_callback = [this](const auto& result) {
            this->HandleResultCallback(result);
        };
    }

   void HandleGoalAcceptedCallback(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::SharedPtr& HandleMessage){
        
        if(!HandleMessage){
            RCLCPP_ERROR(ParentNodePtr->get_logger(), "Goal was rejected by the action server");
            return;
        }

        HandleGoal = HandleMessage;
        RCLCPP_INFO(ParentNodePtr->get_logger(), "Goal was accepted by the action server, waiting for result...");

   }

   void HandleFeedbackCallback( rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::SharedPtr,
    const std::shared_ptr<const nav2_msgs::action::NavigateToPose::Feedback> FeedbackMessage){

        float DistancRemaining = FeedbackMessage->distance_remaining;
        int EstimatedTimeRemaining = FeedbackMessage->estimated_time_remaining.sec;

        RCLCPP_INFO(ParentNodePtr->get_logger(), "Distance remaining: %.2f meters", DistancRemaining);
        RCLCPP_INFO(ParentNodePtr->get_logger(), "Estimated time remaining: %d seconds", EstimatedTimeRemaining);

   }

   void HandleResultCallback(const rclcpp_action::ClientGoalHandle<nav2_msgs::action::NavigateToPose>::WrappedResult & HandleResultMessage){
        
        switch(HandleResultMessage.code){
            case rclcpp_action::ResultCode::SUCCEEDED:
                RCLCPP_INFO(ParentNodePtr->get_logger(), "Goal was successfully reached");
                break;
            case rclcpp_action::ResultCode::ABORTED:
                RCLCPP_ERROR(ParentNodePtr->get_logger(), "Goal was aborted by the action server");
                break;
            case rclcpp_action::ResultCode::CANCELED:
                RCLCPP_WARN(ParentNodePtr->get_logger(), "Goal was canceled by the action server");
                break;
            default:
                RCLCPP_ERROR(ParentNodePtr->get_logger(), "Unknown result code");
                break;
        }
        if (ExternalResult) {
            ExternalResult(HandleResultMessage);
        }
   }

};

