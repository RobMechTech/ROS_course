#include"rclcpp/rclcpp.hpp"
#include "example_interfaces/msg/string.hpp"

class message_reader:public rclcpp::Node{
    public:
        message_reader():Node("reader"){
            subcriber_=this->create_subscription<example_interfaces::msg::String>("message_value",10,std
                                                ::bind(&message_reader::messge_callback,this,std::placeholders:: _1));
            RCLCPP_INFO(this->get_logger(),"i am listening to ");
        }
    private:
        void messge_callback(const example_interfaces::msg::String::SharedPtr value){
            RCLCPP_INFO(this->get_logger(),"%s",value->data.c_str());
            
        }
        rclcpp::Subscription<example_interfaces::msg::String>::SharedPtr subcriber_;
};

int main(int argc, char **argv){
    rclcpp::init(argc,argv);
    auto node=std::make_shared<message_reader>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;

}
