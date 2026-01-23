

#include <functional>
#include <memory>
#include <rclcpp/logging.hpp>
#include <rclcpp/qos.hpp>


#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "number_interface/msg/joystic_command.hpp"

using std::placeholders::_1;

class MinimalSubscriber : public rclcpp::Node
{
public:



  MinimalSubscriber()
  : Node("minimal_subscriber")
  {
    subscription_ = this->create_subscription<sensor_msgs::msg::Joy>(
      "joy", 10, std::bind(&MinimalSubscriber::topic_callback, this, _1));


    publisher_ = this->create_publisher<number_interface::msg::JoysticCommand>("joy_message", 10);

  }




private:
 
  rclcpp::Publisher<number_interface::msg::JoysticCommand>::SharedPtr publisher_;
  rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr subscription_;


  float valx;
  float valy;
  float pyaw;  
  float nyaw;
  float totalyaw;

  
  


  void topic_callback(const sensor_msgs::msg::Joy::SharedPtr msg)

  {

    valx = msg->axes[0];
    valy = msg->axes[1];
    pyaw = msg->axes[4];
    nyaw = msg->axes[5];

    
    RCLCPP_INFO(this->get_logger(), "valx: %f", valx);
    RCLCPP_INFO(this->get_logger(), "valy: %f", valy);
    RCLCPP_INFO(this->get_logger(), "pyaw: %f", pyaw);
    RCLCPP_INFO(this->get_logger(), "nyaw: %f", nyaw);

    
    publish_message(valx, valy);
    
  }



  void publish_message(float vx, float vy) {

    auto message = number_interface::msg::JoysticCommand();

    message.lx = vx;
    message.ly = vy;

    publisher_->publish(message);

    
  }
 

  
};



int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalSubscriber>());
  rclcpp::shutdown();
  return 0;
}