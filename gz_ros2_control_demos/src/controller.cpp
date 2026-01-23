#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "number_interface/msg/joystic_command.hpp"

#include "tf2/LinearMath/Quaternion.h"
#include "tf2/LinearMath/Matrix3x3.h"
#include <cmath>

using std::placeholders::_1;

class ctrl_algo : public rclcpp::Node {
public:
    ctrl_algo() : Node("ctrl_node") {
        
        imu_subs = this->create_subscription<sensor_msgs::msg::Imu>(
            "/imu", 10, std::bind(&ctrl_algo::imu_func, this, _1));
            
        odom_subs = this->create_subscription<nav_msgs::msg::Odometry>(
        "/diff_drive_base_controller/odom", 10,
        std::bind(&ctrl_algo::odom_func, this, _1));
            
        vel_pub_ = this->create_publisher<geometry_msgs::msg::TwistStamped>(
            "/diff_drive_base_controller/cmd_vel", 10);
                                        
        vel_subs_=this->create_subscription<number_interface::msg::JoysticCommand>(
            "joy_message",10,std::bind(&ctrl_algo::vel_command,this,_1));
            
        RCLCPP_INFO(this->get_logger(), "Control Node Initialized. Remember to set dependencies in CMakeLists.txt!");
    }

private:

    double KP_TILT_ANGLE; // P Gain (Reacts to current angle)
    double KD_TILT_RATE;   // D Gain (Reacts to rate of change)
    double KI_ANGLE ;       // I Gain (Corrects long-term drift/offset)
    

    double measured_speed;

    const double speed_p=10.0;
    const double speed_d=0.0;
    const double speed_i=1;

    double desired_speed;
    double linear_vx;
    double speed_error;
    double integral_speed_error;
    
    const double max_lean_angle_deg=8.0;
    double motor_command_speed=0.0;

    double tilt_angle_rad = 0.0;   
    double tilt_rate_rads = 0.0;    
    
    
    double integral_error_angle = 0.0;

    void imu_func(const sensor_msgs::msg::Imu::SharedPtr data_) {
        tf2::Quaternion q(
            data_->orientation.x, 
            data_->orientation.y, 
            data_->orientation.z, 
            data_->orientation.w);
        
        tf2::Matrix3x3 m(q);
        double roll, pitch, yaw;
        m.getRPY(roll, pitch, yaw);

        tilt_angle_rad = roll;

        tilt_rate_rads = data_->angular_velocity.x;
        vel_pub_func();
        
        
    }

    void odom_func(const nav_msgs::msg::Odometry::SharedPtr msg) {
        measured_speed = msg->twist.twist.linear.x;
    }

    void vel_command(const number_interface::msg::JoysticCommand::SharedPtr command){
        linear_vx=command->ly;
       // RCLCPP_INFO(this->get_logger(), "valx: %.2ff", linear_vx);
    }

    void vel_pub_func() {
        desired_speed =0.5*linear_vx; // m/s

        if (desired_speed<std::abs(0.05)){
            KP_TILT_ANGLE=0.1;
            KD_TILT_RATE=-0.0005;
            KI_ANGLE =0.001;
        } 
        else{
            KP_TILT_ANGLE=0.48;
            KD_TILT_RATE=-0.02;
            KI_ANGLE =0.012;
        }

        speed_error = desired_speed - measured_speed;
        
        double speed_Tilt_angle=speed_p*speed_error +  speed_i*integral_speed_error;
        integral_speed_error += speed_error * 0.01; // dt=0.01s
        integral_speed_error=std::clamp(integral_speed_error, -1.0, 1.0);
        speed_Tilt_angle=std::clamp(speed_Tilt_angle, -max_lean_angle_deg, max_lean_angle_deg);
        

        double acceleraion=speed_error*0;
        if (desired_speed<std::abs(0.05)){
            acceleraion=0.0;
        } 


        double tilt_angle_deg=(tilt_angle_rad*180.0/M_PI);
        if(tilt_angle_deg>90.0){
            tilt_angle_deg=180.0-tilt_angle_deg;
        }
        else if(tilt_angle_deg<-90.0){
            tilt_angle_deg=-180.0-tilt_angle_deg;
        }
        double tilt_rate_angle=tilt_rate_rads*(180.0/M_PI);
        double angle_error = speed_Tilt_angle - tilt_angle_deg;
        double dt_s = 0.010; 

    
        integral_error_angle += angle_error * dt_s;
        
    
        double integral_limit =1.0; 
        if (integral_error_angle > integral_limit) {
            integral_error_angle = integral_limit;
        } else if (integral_error_angle < -integral_limit) {
            integral_error_angle = -integral_limit;
        }



        double motor_command_u = -(
            KP_TILT_ANGLE * angle_error +           
            KI_ANGLE * integral_error_angle +        
            KD_TILT_RATE * tilt_rate_angle+acceleraion    
        );
        
    
        geometry_msgs::msg::TwistStamped command;
        command.header.stamp = this->now(); 
        command.twist.linear.x = motor_command_u;

        if(std::abs(tilt_angle_deg)> 82.0){
        command.twist.linear.x=0; 
        }
        
        command.twist.linear.y = 0.0;
        command.twist.linear.z = 0.0;
        command.twist.angular.x = 0.0;
        command.twist.angular.y = 0.0;
        command.twist.angular.z = 0.0;
        vel_pub_->publish(command);


        RCLCPP_INFO(this->get_logger(), 
                "Angle: %.2f deg  | Cmd: %.2f ,speed_of rc %.2f | measured_speed: %.2f m/s", 
                tilt_angle_deg,motor_command_u,desired_speed, measured_speed);

        
    }


    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_subs;
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_subs;
    rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr vel_pub_;
    rclcpp::Subscription<number_interface::msg::JoysticCommand>::SharedPtr vel_subs_;
};  

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ctrl_algo>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}