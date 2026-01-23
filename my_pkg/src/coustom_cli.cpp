#include "rclcpp/rclcpp.hpp"
#include "ros_interface/srv/tringle_area.hpp"

class cus_client_node:public rclcpp::Node{
    public:
    cus_client_node():Node("cus_cli_node"){
        thread_=std::thread(std::bind(&cus_client_node::nub,this));
    }
    void nub(){
        
        while(rclcpp::ok()){
            int x,y;
            std::cout<<"enter the value height nad base ";
            std::cin>>x>>y;
            get_number(x,y);
        }
        

    }
    void get_number(int a , int b){
        auto client=this->create_client<ros_interface::srv::TringleArea>("tr_area");
        while(!client->wait_for_service(std::chrono::seconds(1))){
            RCLCPP_WARN(this->get_logger(),"client is wainting for the service!");
        }
        auto request_=std::make_shared <ros_interface::srv::TringleArea::Request>();
        request_ ->height=a;
        request_ ->base=b;
        auto future=client->async_send_request(request_);
        try{
           auto response =future.get(); 
           RCLCPP_INFO(this ->get_logger(),"the area is %.2f",response->area);
        }
        catch(const std::exception &e){
            RCLCPP_ERROR(this->get_logger(),"failed to get sum");
        }

    }
    private:
        std::thread thread_;
};

int main(int argc, char **argv){
    rclcpp::init(argc,argv);
    auto node= std::make_shared<cus_client_node>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;

}
