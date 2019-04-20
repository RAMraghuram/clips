/*
 * Time synchronization is already handled by mavros sys_time.
 * The goal of this code is that there has to be atleast one imu data
 * between two successive camera frames.
 * Topics subscribes to ['
 */ 

#include <mutex> 
#include "ros/ros.h"
#include "sensor_msgs/Image.h"
#include "sensor_msgs/Imu.h"
#include "sensor_msgs/CameraInfo.h"

#define SUBSCRIBED_IMAGE_TOPIC "/usb_cam/image_raw"
#define SUBSCRIBED_IMU_TOPIC "/mavros/imu/data_raw"
#define SUBSCRIBED_CAMERA_INFO "/usb_cam/camera_info"
#define PUBLISHED_IMAGE_TOPIC "/cam0/image_raw"
#define PUBLISHED_IMU_TOPIC "/imu0/data_raw"
#define PUBLISHED_CAMERA_INFO "/cam0/camera_info"

struct {
  ros::Subscriber sub_image;
  ros::Subscriber sub_imu;
  ros::Subscriber sub_ci;
  ros::Publisher pub_image;
  ros::Publisher pub_imu;
  ros::Publisher pub_ci;
  int new_imus;
  std::mutex mtx;
} context;


void cameraCallback(const sensor_msgs::Image::ConstPtr &msg)
{

  context.mtx.lock();
  if (context.new_imus) 
    context.pub_image.publish(msg);

  context.new_imus = 0;
  context.mtx.unlock();

}

void imuCallback(const sensor_msgs::Imu::ConstPtr &msg)
{
  context.mtx.lock();
  context.new_imus++;
  context.pub_imu.publish(msg);
  context.mtx.unlock();
}

void cameraInfoCallback(const sensor_msgs::CameraInfo::ConstPtr &msg)
{
  context.pub_ci.publish(msg);
}

int main(int argc, char **argv) {
  ros::init(argc, argv, "clips");

  ros::NodeHandle n;
  
  context.new_imus = 0;

  context.sub_image = n.subscribe(SUBSCRIBED_IMAGE_TOPIC, 1000, cameraCallback);
  context.sub_imu = n.subscribe(SUBSCRIBED_IMU_TOPIC, 1000, imuCallback);
  context.sub_ci = n.subscribe(SUBSCRIBED_CAMERA_INFO, 1000, cameraInfoCallback);

  context.pub_image = n.advertise<sensor_msgs::Image>(PUBLISHED_IMAGE_TOPIC, 1000);
  context.pub_imu = n.advertise<sensor_msgs::Imu>(PUBLISHED_IMU_TOPIC, 1000);
  context.pub_ci = n.advertise<sensor_msgs::CameraInfo>(PUBLISHED_CAMERA_INFO, 1000);

  ros::Rate r(1000);

  while(ros::ok())
  {
    ros::spinOnce();
    r.sleep();

  }

  return 0;
}
