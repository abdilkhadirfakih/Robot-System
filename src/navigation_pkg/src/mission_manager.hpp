#pragma once
#include <iostream>
#include "rclcpp/rclcpp.hpp"
#include "robot_state.hpp"
#include "nav_msgs/msg/occupancy_grid.hpp"
#include "nav_msgs/msg/map_meta_data.hpp"
#include "sensor_msgs/msg/battery_state.hpp"
#include <random>
#include "nlohmann/json.hpp"
#include <sensor_msgs/msg/laser_scan.hpp>
#include <std_msgs/msg/string.hpp>

using json = nlohmann::json;
class MissionManager {

    public: 
        MissionManager(){}
        MetaData GetMapMetaData(){ return this->MapMetaData; }

        void ReceiveVisionResults(const json& visionResults){
            // Process the vision results and update the mission manager's state accordingly
            // For example, you can extract relevant information from the vision results and make decisions based on that
            RCLCPP_INFO(rclcpp::get_logger("MissionManager"), "Received vision results: %s", visionResults.dump().c_str());
        }

        void ReceiveLiderData(const sensor_msgs::msg::LaserScan::SharedPtr message){
            this->LidarData = message;
        }

        void ReceiveBattaryData(const sensor_msgs::msg::BatteryState::SharedPtr message){ 
            this->BatteryData = message;
        }
        
        void ReceiveRTAmap(const nav_msgs::msg::OccupancyGrid::SharedPtr message){
            this->MapData = message;
            this->ConstructMapMetaData();
        }

        void ConstructMapMetaData(){
            this->MapMetaData.MapResolution = this->MapData->info.resolution;
            this->MapMetaData.MapWidth = this->MapData->info.width;
            this->MapMetaData.MapHeight = this->MapData->info.height;
            this->MapMetaData.MapOriginX = this->MapData->info.origin.position.x;
            this->MapMetaData.MapOriginY = this->MapData->info.origin.position.y;
        }

        GridCell ConvertWorldCoordinateToGridCell(double x, double y){
            GridCell MapCoordinate;
            MapCoordinate.x = (x - this->MapMetaData.MapOriginX) / this->MapMetaData.MapResolution;
            MapCoordinate.y = (y - this->MapMetaData.MapOriginY) / this->MapMetaData.MapResolution;
            return MapCoordinate;
        }

        int FlattenGridCell(GridCell cell){
            return cell.x + cell.y * this->MapMetaData.MapWidth;
        }

        bool CheckBoundaryConditions(GridCell cell){
            if(cell.x < 0 || cell.x >= this->MapMetaData.MapWidth || cell.y < 0 || cell.y >= this->MapMetaData.MapHeight){
                return false;
            }
            return true;
        }

        bool IsSpaceFree( double WorldX, double WorldY){
            ConstructMapMetaData();
            GridCell cell = ConvertWorldCoordinateToGridCell(WorldX, WorldY);
            if(!CheckBoundaryConditions(cell))
                return false;
            int index = FlattenGridCell(cell);
            if(CheckBoundaryConditions(cell) == false)
                return false;
            if(this->MapData->data[index] == 0)
                return true; 
            return false; 
        }

        Coordinate GenerateRandomCoordinates(){
            Coordinate randomCoordinate;
            std::random_device randomDevice;
            std::mt19937 generator(randomDevice());

            std::uniform_real_distribution<double> distributionX(this->MapMetaData.MapOriginX, this->MapMetaData.MapOriginX + this->MapMetaData.MapWidth * this->MapMetaData.MapResolution);
            std::uniform_real_distribution<double> distributionY(this->MapMetaData.MapOriginY, this->MapMetaData.MapOriginY + this->MapMetaData.MapHeight * this->MapMetaData.MapResolution);

            randomCoordinate.x = distributionX(generator);
            randomCoordinate.y = distributionY(generator);
            return randomCoordinate;
        }

        Coordinate GenerateAndEvaluateRandomCoordinates(){
            std::optional<Coordinate> randomCoordinate = std::nullopt;
            while(!randomCoordinate.has_value()){
                randomCoordinate = GenerateRandomCoordinates();
                if(!IsSpaceFree(randomCoordinate.value().x, randomCoordinate.value().y)){
                    break;
                }
            }
            return randomCoordinate.value();
        }

        bool CheckMissionFeasibility(){
            float BattaryPercentage = this->BatteryData->percentage * 100.0f;
            if( BattaryPercentage < 25.0){
                return false;
            }
            return true;
        }


    private:
        nav_msgs::msg::OccupancyGrid::SharedPtr MapData;
        sensor_msgs::msg::BatteryState::SharedPtr BatteryData;
        MetaData MapMetaData;
        sensor_msgs::msg::LaserScan::SharedPtr LidarData;
};