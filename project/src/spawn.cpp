#include "rclcpp/rclcpp.hpp"
#include "ros_interface/msg/turtle.hpp"
#include "ros_interface/msg/turtle_array.hpp"
#include "ros_interface/srv/catch_turtle.hpp"
#include "turtlesim/srv/spawn.hpp"
#include "turtlesim/srv/kill.hpp"

using std::placeholders::_1;
using std::placeholders::_2;

class spawner_node:public rclcpp::Node{
public:
    spawner_node():Node("spawner"),count(0) {
        turtle_prefix="turtle";
        spawn_timer_=this->create_wall_timer(std::chrono::milliseconds(1000),std::bind(&spawner_node::spawner_func,this));
        turtle_publisher_=this->create_publisher<ros_interface::msg::TurtleArray>("turtle_info",10);
        msg_publisher_=this->create_wall_timer(std::chrono::seconds(10),std::bind(&spawner_node::msg_publisher,this));
        catch_turtle=this->create_service<ros_interface::srv::CatchTurtle>("kill_name",std::bind(&spawner_node::callback_catcher,this,_1,_2));
        
    }

private:
    double random_number_generate(){
        return double(std::rand()) / (RAND_MAX + 1.0);
    }
    void msg_publisher(){
        auto msg=ros_interface::msg::TurtleArray();
        msg.turtles=alive_turtles;
        turtle_publisher_->publish(msg);
    }
    void spawner_func(){
        count++;
        double x=10*random_number_generate();
        double y=10*random_number_generate();
        double angle=random_number_generate()*2*M_PI;
        std::string name=turtle_prefix+ std::to_string(count);
        thread_spawner.push_back(std::make_shared<std::thread>(std::bind(&spawner_node::callback_spawner, this, x, y, angle, name)));


    }
    void callback_spawner(double a,double b, double ang, std::string name ){
        auto spawner_client=this->create_client<turtlesim::srv::Spawn>("spawn"); 

        while(!spawner_client->wait_for_service(std::chrono::seconds(1))){
            RCLCPP_WARN(this->get_logger(),"waiting for the service to be up");
        }
        auto request=std::make_shared<turtlesim::srv::Spawn::Request>();
        request->name=name;
        request->x=a;
        request->y=b;
        request->theta=ang;
        auto future=spawner_client->async_send_request(request);
        try{
            auto response=future.get();
            if(response->name!=""){
                auto new_turtle=ros_interface::msg::Turtle();
                new_turtle.name=response->name;
                new_turtle.x=a;
                new_turtle.y=b;
                new_turtle.theta=ang;
                alive_turtles.push_back(new_turtle);
                msg_publisher();
            }
            RCLCPP_INFO(this->get_logger(),"turtle %s is up",response->name.c_str());
        }
        catch(const std::exception &e){
            RCLCPP_ERROR(this->get_logger(),"service is not responding!");
        }

    }

    void callback_catcher(const ros_interface::srv::CatchTurtle::Request::SharedPtr request_ , const 
                        ros_interface::srv::CatchTurtle::Response::SharedPtr response_ ){
        
        thread_killer.push_back(std::make_shared<std::thread>(std::bind(&spawner_node::killer,this,request_->name)));
        response_->success=true;
    }

    void killer(std::string turtle_name){
        auto client_=this->create_client<turtlesim::srv::Kill>("kill");
        if(!client_->wait_for_service(std::chrono::seconds(1))){
            RCLCPP_WARN(this->get_logger(),"waiting for killer service to up");
        }
        auto request_=std::make_shared<turtlesim::srv::Kill::Request>();
        request_->name=turtle_name;
        auto future=client_->async_send_request(request_);
        try{
            future.get();
            for(int i=0; i<(int)alive_turtles.size();i++){
                if(alive_turtles.at(i).name==turtle_name){
                    alive_turtles.erase(alive_turtles.begin() + i);
                    msg_publisher();
                    break;
                }
            }
        }
        catch(const std::exception &e){
            RCLCPP_ERROR(this->get_logger(),"error killing the turtle:");
        }
    }

    
    rclcpp::TimerBase::SharedPtr spawn_timer_;
    std::vector<std::shared_ptr<std::thread>> thread_spawner;
    std::string turtle_prefix;
    int count;
    rclcpp::Publisher<ros_interface::msg::TurtleArray>::SharedPtr turtle_publisher_;
    std::vector<ros_interface::msg::Turtle> alive_turtles;
    rclcpp::TimerBase::SharedPtr msg_publisher_;
    rclcpp::Service<ros_interface::srv::CatchTurtle>::SharedPtr catch_turtle;
    std::vector<std::shared_ptr<std::thread>> thread_killer;

};

int main(int argc, char **argv){
    rclcpp::init(argc,argv);
    auto node=std::make_shared<spawner_node>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;

}