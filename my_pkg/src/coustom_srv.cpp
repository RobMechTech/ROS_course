#include "rclcpp/rclcpp.hpp"
#include "ros_interface/srv/tringle_area.hpp"
using std::placeholders:: _1;
using std::placeholders:: _2;
class coustom_srv:public rclcpp::Node{
    public:
        coustom_srv():Node("cus_srv_node"){
            service_=this->create_service<ros_interface::srv::TringleArea>("tr_area",std::bind(&coustom_srv::area_fnc,this,_1,_2));
            RCLCPP_INFO(this->get_logger(),"service is ready!");
        }
    private:
        void area_fnc(const ros_interface::srv::TringleArea::Request::SharedPtr request_,
                    const ros_interface::srv::TringleArea::Response::SharedPtr response_){

            response_ ->area=(request_->base * request_->height)*0.5 ;
            RCLCPP_INFO(this->get_logger(),"the area is %.2f",response_->area);
        }
        rclcpp::Service<ros_interface::srv::TringleArea>::SharedPtr service_;
};

int main (int argc,char **argv){
    rclcpp::init(argc,argv);
    auto node=std::make_shared<coustom_srv>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}