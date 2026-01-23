#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
using std::placeholders::_1;

class vel_finder:public rclcpp::Node{
public:
    vel_finder():Node("vel_dbg"){
        vel_subs=this->create_subscription<geometry_msgs::msg::TwistStamped>("/diff_drive_base_controller/cmd_vel"
            ,10,std::bind(&vel_finder::vel_info,this,_1));
    }
private:
    void vel_info(const geometry_msgs::msg::TwistStamped::SharedPtr data){
        double data_=data->twist.linear.x;
        RCLCPP_INFO(this->get_logger(),"velocity is %.2f",data_);
    }
    rclcpp::Subscription<geometry_msgs::msg::TwistStamped>::SharedPtr vel_subs;


};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<vel_finder>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}