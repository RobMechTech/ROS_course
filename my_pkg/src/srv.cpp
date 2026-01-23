#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/srv/add_two_ints.hpp"
using std::placeholders:: _1;
using std::placeholders:: _2;

class service_node:public rclcpp::Node{
public:
    service_node():Node("adder_srv"){
        service_=this->create_service<example_interfaces::srv::AddTwoInts>("number_adder",std::bind(&service_node::number_adder,this,_1,_2));

        RCLCPP_INFO(this->get_logger(),"service is ready");
    }


private:
void number_adder(const example_interfaces::srv::AddTwoInts::Request::SharedPtr request ,
                const example_interfaces::srv::AddTwoInts::Response::SharedPtr respnse ){
    respnse->sum=request->a + request->b;
    RCLCPP_INFO(this->get_logger(),"the sum is %ld",respnse->sum);

};
rclcpp::Service<example_interfaces::srv::AddTwoInts>::SharedPtr service_;

};

int main(int argc,char **argv){
    rclcpp::init(argc,argv);
    auto node=std::make_shared<service_node>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}