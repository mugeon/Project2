#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>
#include "geometry_msgs/Twist.h"

// Define a global client that can request services
ros::ServiceClient client;


// ROS::Publisher motor commands;
ros::Publisher motor_command_publisher;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    srv.response.msg_feedback = "Wheel velocities set - linear_x: " + std::to_string(srv.request.linear_x) + " , angular_z: " + std::to_string(srv.request.angular_z);
    ROS_INFO_STREAM(srv.response.msg_feedback);

    // Create a motor_command object of type geometry_msgs::Twist
    geometry_msgs::Twist motor_command;
    // Set wheel velocities
    motor_command.linear.x = srv.request.linear_x;
    motor_command.angular.z = srv.request.angular_z;
    // Publish angles to drive the robot
    motor_command_publisher.publish(motor_command);



}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    int location;
    int reminder;

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    for (int i = 0; i < img.height * img.step; i+=3) {
        if (img.data[i] == white_pixel && img.data[i+1] == white_pixel && img.data[i+2] == white_pixel) {
            location = i;
            break;
        }
        location = -1;
    }
    if (location == -1) {
        // stop the robot
        drive_robot(0, 0);
    } else {
        reminder = location % img.step;
        if (reminder <  img.step / 3) {
            // turn left
            drive_robot(0, 0.5);
        } else if (reminder > img.step / 3 && reminder <  2*img.step / 3) {
            // move forward
            drive_robot(0.5, 0);
        } else {
            // turn right
            drive_robot(0, -0.5);
        }
    }



}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Inform ROS master that we will be publishing a message of type geometry_msgs::Twist on the robot actuation topic with a publishing queue size of 10
    motor_command_publisher = n.advertise<geometry_msgs::Twist>("/cmd_vel", 10);

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}