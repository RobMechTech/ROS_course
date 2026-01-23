#include "rclcpp/rclcpp.hpp"
#include "turtlesim/msg/pose.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "ros_interface/msg/turtle.hpp"
#include "ros_interface/msg/turtle_array.hpp"
#include "ros_interface/srv/catch_turtle.hpp"


using std::placeholders::_1;
#include <cmath>

class ctrl:public rclcpp::Node{
public:
    ctrl():Node("controller"),name_("turtle1"),turtlesim_up(false){
        cmd_vel_pub=this->create_publisher<geometry_msgs::msg::Twist>(name_+"/cmd_vel",10);
        pose_subs=this->create_subscription<turtlesim::msg::Pose>(name_+"/pose",10,std::bind(&ctrl::get_pose,this,_1));
        msg_subscriber=this->create_subscription<ros_interface::msg::TurtleArray>("turtle_info",10,std::bind(&ctrl::callback_subscriber,this,_1));
        control_timer_=this->create_wall_timer(std::chrono::milliseconds(10),std::bind(&ctrl::control_loop,this));

    }     
private:
    void get_pose(const turtlesim::msg::Pose::SharedPtr pose){
        pose_=*pose.get();
        turtlesim_up=true;    
    }

    void callback_subscriber(const ros_interface::msg::TurtleArray::SharedPtr msg_){

        if(msg_->turtles.empty()){
            turtle_to_catch.name="";
            return;
        }
        ros_interface::msg::Turtle closest_turtle=msg_->turtles.at(0);
        double current_distance=get_distance(msg_->turtles.at(0));
        for(int i=1; i<(int)msg_->turtles.size() ;i++){
            double distance=get_distance(msg_->turtles.at(i));
            if(distance<current_distance){
                closest_turtle=msg_->turtles.at(i);
                current_distance=distance;
            }
        }
        turtle_to_catch=closest_turtle;
    }

    double get_distance(ros_interface::msg::Turtle catch_name){
        double x=catch_name.x -pose_.x;
        double y=catch_name.y-pose_.y; 
        double distace=std::sqrt(x*x + y*y);
        return distace;   
    }

    void control_loop(){
        if(!turtlesim_up || turtle_to_catch.name==""){
            return ;
        }

        double x=turtle_to_catch.x-pose_.x;
        double y=turtle_to_catch.y-pose_.y;
        double distace=std::sqrt(x*x + y*y);
        auto pub_msg =geometry_msgs::msg::Twist();
        
        if(distace>0.1){
            pub_msg.linear.x=4*distace;
            double angle=std::atan2(y,x) ;
            double diff_angle=angle-pose_.theta;
            if(diff_angle>M_PI){
                diff_angle-=2*M_PI;
            }
            else if(diff_angle<-M_PI){
                diff_angle+=2*M_PI;
            }   
            pub_msg.angular.z=6*diff_angle;
        }        
        
        else{
            pub_msg.linear.x=0;
            pub_msg.angular.z=0;
            call_killer_thread.push_back(std::make_shared<std::thread>(std::bind(&ctrl::callback_killer,this,turtle_to_catch.name)));
            turtle_to_catch.name="";
        }
        cmd_vel_pub->publish(pub_msg);   

    }
    void callback_killer(std::string name){
        auto client_=this->create_client<ros_interface::srv::CatchTurtle>("kill_name");
        if(!client_->wait_for_service(std::chrono::seconds(1))){
            RCLCPP_WARN(this->get_logger(),"waiting for service");
        }
        auto request_=std::make_shared<ros_interface::srv::CatchTurtle::Request>();
        request_->name=name;
        auto future=client_->async_send_request(request_);
        try{
            auto response_= future.get();
            if(response_->success){
                RCLCPP_INFO(this->get_logger(),"turtle killed");
            }

        }
        catch(const std::exception &e){
            RCLCPP_ERROR(this->get_logger(),"error killing the turtle");
        }

    }
    std::string name_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub;
    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr pose_subs;
    rclcpp::Subscription<ros_interface::msg::TurtleArray>::SharedPtr msg_subscriber;
    turtlesim::msg::Pose pose_;
    ros_interface::msg::Turtle turtle_to_catch;
    bool turtlesim_up;
    rclcpp::TimerBase::SharedPtr control_timer_;
    ros_interface::msg::TurtleArray turtles;
    std::vector<std::shared_ptr<std::thread>> call_killer_thread;

};

int main(int argc, char **argv){
    rclcpp::init(argc,argv);
    auto node=std::make_shared<ctrl>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;

}

