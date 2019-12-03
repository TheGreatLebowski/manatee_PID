#pragma once

#include "ros/ros.h"
#include "std_msgs/String.h"
#include "uuv_gazebo_ros_plugins_msgs/FloatStamped.h"
#include "sensor_msgs/FluidPressure.h"

#include <iostream>
#include <fstream>
#include <sstream>

class PID
{
    public:
        template<typename ...T>
        PID(float desired_value, float min, float max, std::string conf_file,
                std::string csv_file, std::string file_header, 
                std::string subscribe, T&... args)
            :desired_value_{desired_value}, min_{min}, max_{max}
            ,conf_file_{conf_file}, subscribe_{subscribe}, adv_{args ...}
        {
            actual_value_ = 0.0;
            error_ = 0.0;
            error_prior_ = 0.0;
            integral_ = 0.0;
            derivative_ = 0.0;
            read_file(conf_file_, KP_, KI_, KD_);
            of_file_.open(csv_file, std::ios_base::app);
            of_file_ << file_header << std::endl;
            
        };

        //TODO Destructor

        void read_file(std::string filename, float& KP, float& KI, float& KD)
        {
            std::ifstream infile(filename);
            if (!infile)
            {
                ROS_INFO("Can't open the file !!! %s", filename.c_str());
                exit(1);
            }
            infile >> KP >> KI >> KD;
        }

        void publish_file(float time)
        {
            of_file_ << time << "," << actual_value_ << std::endl;
        }

        void PID_calcul()
        {
            error_ = desired_value_ - actual_value_;
            if (abs(error_) > 0.01)
                integral_ += error_ * 0.01;
            derivative_ = (error_ - error_prior_) / 0.01;
            output_ = KP_ * error_ + KI_ * integral_ + KD_ * derivative_;
            error_prior_ = error_;
        }

        float limit()
        {
            if (abs(output_) > max_)
                return output_ > 0 ? max_ : min_;
            return output_;
        }

        void set_actual_value(float value)
        {
            actual_value_ = value;
        }

        float get_actual_value()
        {
            return actual_value_;
        }

        float get_output()
        {
            return output_;
        }

        std::string get_subscribe()
        {
            return subscribe_;
        }

        void publish(ros::NodeHandle n, auto msg)
        {
            for (int i = 0; i < adv_.size(); ++i)
            {
                ros::Publisher chatter_pub = n.advertise<uuv_gazebo_ros_plugins_msgs::FloatStamped>(adv_[i], 1000);
                chatter_pub.publish(msg);
            }
        }

    private:
        //Parameters of constructor
        float KP_, KI_, KD_;
        float desired_value_;
        float min_, max_;
        std::string conf_file_;
        std::string subscribe_;

        std::ofstream of_file_;
        float actual_value_;
        float error_prior_;
        float integral_;
        float error_;
        float derivative_;
        float output_;
        std::vector<std::string> adv_;
};