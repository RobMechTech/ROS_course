#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/srv/add_two_ints.hpp"

class client_node:public rclcpp::Node{
    public:
    client_node():Node("cli_node"){
        thread_=std::thread(std::bind(&client_node::nub,this));
    }
    void nub(){
        
        while(rclcpp::ok()){
            int x,y;
            std::cout<<"enter the value a nad b ";
            std::cin>>x>>y;
            get_number(x,y);
        }
        

    }
    void get_number(int a , int b){
        auto client=this->create_client<example_interfaces::srv::AddTwoInts>("number_adder");
        while(!client->wait_for_service(std::chrono::seconds(1))){
            RCLCPP_WARN(this->get_logger(),"client is wainting for the service!");
        }
        auto request_=std::make_shared <example_interfaces::srv::AddTwoInts::Request>();
        request_ ->a=a;
        request_ ->b=b;
        auto future=client->async_send_request(request_);
        try{
           auto response =future.get(); 
           RCLCPP_INFO(this ->get_logger(),"the sum is %ld",response->sum);
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
    auto node= std::make_shared<client_node>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;

}
