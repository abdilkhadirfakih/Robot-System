#pragma once

enum class RobotState {
    Idle,
    Navigating,
    Charging,
    Error,
    ManualControl
};

enum class Missions{
    Patrol,
    NoMission,
    ReturnToBase,
};

struct GridCell{
    double x;
    double y;
};

struct Coordinate{
    double x;
    double y;
};

struct MetaData{
        float MapResolution;
        uint32_t MapWidth;
        uint32_t MapHeight;
        double MapOriginX; 
        double MapOriginY;
};

struct ThreeDimensionalCoordinate{
    double x;
    double y;
    double OrientationW;
};    