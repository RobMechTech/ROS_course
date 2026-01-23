#include"rclcpp/rclcpp.hpp"
#include "example_interfaces/msg/string.hpp"


class messenger:public rclcpp::Node{
public:
    messenger():Node("robot_meeanger"){

        publisher_=this->create_publisher<example_interfaces::msg::String>("message_value",10);
        timer_=this->create_wall_timer(std::chrono::milliseconds(1000),std::bind(&messenger::publish_message,this));
        RCLCPP_INFO(this->get_logger(),"robot_messager is started!");

    }
private:
void publish_message(){
    auto msg=example_interfaces::msg::String();
    msg.data=std::string("hi this is a publisher node ");
    publisher_->publish(msg);
}
rclcpp::TimerBase::SharedPtr timer_;
rclcpp::Publisher<example_interfaces::msg::String>::SharedPtr publisher_;
};

int main(int argc, char **argv){
    rclcpp::init(argc,argv);
    auto node=std::make_shared<messenger>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}