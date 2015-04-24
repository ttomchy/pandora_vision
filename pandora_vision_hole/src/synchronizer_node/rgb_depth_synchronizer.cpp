/*********************************************************************
 *
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2014, P.A.N.D.O.R.A. Team.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the P.A.N.D.O.R.A. Team nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Alexandros Philotheou, Manos Tsardoulias, Angelos Triantafyllidis
 *********************************************************************/

#include "synchronizer_node/rgb_depth_synchronizer.h"

/**
  @namespace pandora_vision
  @brief The main namespace for PANDORA vision
 **/
namespace pandora_vision
{
  /**
    @brief The constructor
   **/
  RgbDepthSynchronizer::RgbDepthSynchronizer(void)
    : invocationTime_(0.0), meanProcessingTime_(0.0), ticks_(0)
  {
    // The synchronizer node starts off in life locked, waiting for the
    // hole fusion node to unlock him
    isLocked_ = true;

    copiedPc_.reset(new PointCloud);
    // Initialize the messageNum variable.
    // It acts as a counter of the published messages from 
    // Rgbd-T synchronizer node to the synchronizer node
    // in each execution cyrcle.
    messageNum_ = 0;

    // Acquire the names of topics which the synchronizer node will be having
    // transactionary affairs with
    getTopicNames();

    // Acquire the information about the input point cloud that cannot be
    // acquired from the point cloud message.
    // The parameters concerned are needed only if in simulation mode
    getSimulationDimensions();

    // Subscribe to the hole_fusion lock/unlock topic
    unlockSubscriber_ = nodeHandle_.subscribe(unlockTopic_, 1,
      &RgbDepthSynchronizer::unlockCallback, this);

    // Subscribe to the topic where the Hole Fusion node requests from the
    // synchronizer node to subscribe to the input point cloud topic
    subscribeToInputPointCloudSubscriber_ = nodeHandle_.subscribe(
      subscribeToInputPointCloudTopic_, 1,
      &RgbDepthSynchronizer::subscribeToInputPointCloudCallback, this);

    // Subscribe to the topic where the Hole Fusion node requests from the
    // synchronizer node to leave its subscription to the
    // input point cloud topic
    leaveSubscriptionToInputPointCloudSubscriber_ = nodeHandle_.subscribe(
      leaveSubscriptionToInputPointCloudTopic_, 1,
      &RgbDepthSynchronizer::leaveSubscriptionToInputPointCloudCallback, this);


    // Advertise the synchronized point cloud
    synchronizedPointCloudPublisher_ = nodeHandle_.advertise
      <PointCloud>(synchronizedPointCloudTopic_, 1000);

    // Advertise the synchronized depth image
    synchronizedDepthImagePublisher_ = nodeHandle_.advertise
      <sensor_msgs::Image>(synchronizedDepthImageTopic_, 1000);

    // Advertise the synchronized rgb image
    synchronizedRGBImagePublisher_ = nodeHandle_.advertise
      <sensor_msgs::Image>(synchronizedRgbImageTopic_, 1000);

    // Advertise the synchronized thermal image
    synchronizedThermalImagePublisher_ = nodeHandle_.advertise
      <sensor_msgs::Image>(synchronizedThermalImageTopic_, 1000);

    ROS_INFO_NAMED(PKG_NAME, "[Synchronizer node] Initiated");
  }



  /**
    @brief Default destructor
    @return void
   **/
  RgbDepthSynchronizer::~RgbDepthSynchronizer(void)
  {
    ROS_INFO_NAMED(PKG_NAME, "[Synchronizer node] Terminated");
  }



  /**
    @brief Variables regarding the point cloud are needed to be set in
    simulation mode: the point cloud's heigth, width and point step.
    This method reads them and sets their respective Parameters.
    @param void
    @return void
   **/
  void RgbDepthSynchronizer::getSimulationDimensions()
  {
    // The namespace dictated in the launch file
    std::string ns = nodeHandle_.getNamespace();

    // Read "height" from the nodehandle
    if (nodeHandle_.hasParam(ns + "/synchronizer_node/height"))
    {
      if (nodeHandle_.getParam(ns + "/synchronizer_node/height",
          Parameters::Image::HEIGHT))
      {
        ROS_INFO_NAMED(PKG_NAME,
          "[Synchronizer Node] Input dimension height read");
      }
      else
      {
        ROS_ERROR_NAMED(PKG_NAME,
          "[Synchronizer Node] Input dimension height failed to be read");
      }
    }

    // Read "width" from the nodehandle
    if (nodeHandle_.hasParam(ns + "/synchronizer_node/width"))
    {
      if (nodeHandle_.getParam(ns + "/synchronizer_node/width",
          Parameters::Image::WIDTH))
      {
        ROS_INFO_NAMED(PKG_NAME,
          "[Synchronizer Node] Input dimension width read");
      }
      else
      {
        ROS_ERROR_NAMED(PKG_NAME,
          "[Synchronizer Node] Input dimension width failed to be read");
      }
    }
  }



  /**
    @brief Acquires topics' names needed to be subscribed to and advertise
    to by the synchronizer node
    @param void
    @return void
   **/
  void RgbDepthSynchronizer::getTopicNames ()
  {
    // The namespace dictated in the launch file
    std::string ns = nodeHandle_.getNamespace();

    // Read the name of the topic from where the synchronizer node acquires the
    // input pointcloud2 from the Rgbd-T synchronizer node
    if (nodeHandle_.getParam(
        ns + "/synchronizer_node/published_topics/pointcloud2_topic",
        inputPointCloud2Topic_ ))
    {
      // Make the topic's name absolute
      inputPointCloud2Topic_ = ns + "/" + inputPointCloud2Topic_;

      ROS_INFO_NAMED(PKG_NAME,
        "[Synchronizer Node] Subscribed to the input pointcloud2");
    }
    else
    {
      ROS_ERROR_NAMED(PKG_NAME,
        "[Synchronizer Node] Could not find topic pointcloud2_topic");
    }

    // Read the name of the topic from where the synchronizer node acquires the
    // input thermal info
    if (nodeHandle_.getParam(
        ns + "/synchronizer_node/published_topics/thermal_image_topic",
        inputThermalTopic_ ))
    {
      // Make the topic's name absolute
      inputThermalTopic_ = ns + "/" + inputThermalTopic_;
 
      ROS_INFO_NAMED(PKG_NAME,
        "[Synchronizer Node] Subscribed to the input thermal info");
    }
    else
    {
      ROS_ERROR_NAMED(PKG_NAME,
        "[Synchronizer Node] Could not find topic thermal_image_topic");
    }

    // Read the name of the topic that the Hole Fusion node uses to unlock
    // the synchronizer node
    if (nodeHandle_.getParam(
        ns + "/synchronizer_node/subscribed_topics/unlock_topic",
        unlockTopic_))
    {
      // Make the topic's name absolute
      unlockTopic_ = ns + "/" + unlockTopic_;

      ROS_INFO_NAMED(PKG_NAME,
        "[Synchronizer Node] Subscribed to the unlock topic");
    }
    else
    {
      ROS_ERROR_NAMED(PKG_NAME,
        "[Synchronizer Node] Could not find topic unlock_topic");
    }

    // Read the name of the topic that the Hole Fusion node uses to request from
    // the synchronizer node to subscribe to the input point cloud
    if (nodeHandle_.getParam(
        ns + "/synchronizer_node/subscribed_topics/subscribe_to_input",
        subscribeToInputPointCloudTopic_))
    {
      subscribeToInputPointCloudTopic_ =
        ns + "/" + subscribeToInputPointCloudTopic_;

      ROS_INFO_NAMED(PKG_NAME,
        "[Synchronizer Node] Subscribed to the Hole Fusion input cloud"
        "subscription request topic");
    }
    else
    {
      ROS_ERROR_NAMED(PKG_NAME,
        "[Synchronizer Node] Could not find topic ");
    }

    // Read the name of the topic that the Hole Fusion node uses to request from
    // the synchronizer node to leave its subscription to the input point cloud
    if (nodeHandle_.getParam(
        ns + "/synchronizer_node/subscribed_topics/leave_subscription_to_input",
        leaveSubscriptionToInputPointCloudTopic_))
    {
      // Make the topic's name absolute
      leaveSubscriptionToInputPointCloudTopic_ =
        ns + "/" + leaveSubscriptionToInputPointCloudTopic_;

      ROS_INFO_NAMED(PKG_NAME,
        "[Synchronizer Node] Subscribed to the Hole Fusion input cloud"
        "subscription leave request topic");
    }
    else
    {
      ROS_ERROR_NAMED(PKG_NAME,
        "[Synchronizer Node] Could not find topic ");
    }

    // Read the name of the topic that the synchronizer node will be publishing
    // the input point cloud to
    if (nodeHandle_.getParam(
        ns + "/synchronizer_node/published_topics/point_cloud_internal_topic",
        synchronizedPointCloudTopic_))
    {
      // Make the topic's name absolute
      synchronizedPointCloudTopic_ = ns + "/" + synchronizedPointCloudTopic_;

      ROS_INFO_NAMED(PKG_NAME, "[Synchronizer Node] "
        "Advertising to the internal point cloud topic");
    }
    else
    {
      ROS_ERROR_NAMED(PKG_NAME, "[Synchronizer Node] "
        "Could not find topic point_cloud_internal_topic");
    }

    // Read the name of the topic that the synchronizer node will be publishing
    // the depth image extracted from the input point cloud to
    if (nodeHandle_.getParam(
        ns + "/synchronizer_node/published_topics/depth_image_topic",
        synchronizedDepthImageTopic_))
    {
      // Make the topic's name absolute
      synchronizedDepthImageTopic_ = ns + "/" + synchronizedDepthImageTopic_;

      ROS_INFO_NAMED(PKG_NAME,
        "[Synchronizer Node] Advertising to the internal depth image");
    }
    else
    {
      ROS_ERROR_NAMED(PKG_NAME,
        "[Synchronizer Node] Could not find topic depth_image_topic");
    }

    // Read the name of the topic that the synchronizer node will be publishing
    // the depth image extracted from the input point cloud to
    if (nodeHandle_.getParam(
        ns + "/synchronizer_node/published_topics/rgb_image_topic",
        synchronizedRgbImageTopic_))
    {
      // Make the topic's name absolute
      synchronizedRgbImageTopic_ = ns + "/" + synchronizedRgbImageTopic_;

      ROS_INFO_NAMED(PKG_NAME,
        "[Synchronizer Node] Advertising to the internal rgb image");
    }
    else
    {
      ROS_ERROR_NAMED(PKG_NAME,
        "[Synchronizer Node] Could not find topic rgb_image_topic");
    }

    // Read the name of the topic that the synchronizer node will be publishing
    // the thermal info extracted from flir camera
    if (nodeHandle_.getParam(
        ns + "/synchronizer_node/published_topics/thermal_info_topic",
        synchronizedThermalImageTopic_))
    {
      // Make the topic's name absolute
      synchronizedThermalImageTopic_ = ns + "/" + synchronizedThermalImageTopic_;

      ROS_INFO_NAMED(PKG_NAME,
        "[Synchronizer Node] Advertising to the internal thermal image");
    }
    else
    {
      ROS_ERROR_NAMED(PKG_NAME,
        "[Synchronizer Node] Could not find topic thermal_image_topic");
    }

  }



  /**
    @brief The synchronized callback for the point cloud
    obtained by the depth sensor and the thermal info by flir camera.

    If the synchronizer node is unlocked, it extracts a depth image from
    the input point cloud's depth measurements, an RGB image from the colour
    measurements of the input point cloud and thermal info.
    Then publishes these images and thermal info
    to their respective recipients. Finally, the input point cloud is
    published directly to the hole fusion node.
    @param[in] pointCloudMessage [const PointCloud&]
    The input point cloud
    @param[in] thermalMessage [const sensor_msgs::Image&]
    the input thermal info
    @return void
   **/
  void RgbDepthSynchronizer::inputPointCloudThermalCallback(
    const PointCloudPtr& pointCloudMessage,
    const sensor_msgs::Image& thermalMessage)
  {
    ROS_INFO_STREAM("pointcloud dimensions " << pointCloudMessage->height <<" "<< pointCloudMessage->width);

    if (!isLocked_)
    {
      // Lock the rgb_depth_thermal_synchronizer node; aka prevent
      // the execution of this if-block without the explicit request
      // of the hole fusion node
      isLocked_ = true;

      #ifdef DEBUG_TIME
      ROS_INFO_NAMED(PKG_NAME, "Synchronizer unlocked");

      ROS_INFO_NAMED(PKG_NAME,
        "=================================================");

      double t = ros::Time::now().toSec() - invocationTime_;

      ROS_INFO_NAMED(PKG_NAME,
        "Previous synchronizer invocation before %fs", t);

      // Increment the number of this node's invocations
      ticks_++;

      if (ticks_ > 1)
      {
        meanProcessingTime_ += t;
      

      ROS_INFO_NAMED(PKG_NAME,
        "Mean processing time :                  %fs",
        (meanProcessingTime_ / (ticks_ - 1)));

      ROS_INFO_NAMED(PKG_NAME,
        "=================================================");
      }
      invocationTime_ = ros::Time::now().toSec();

      Timer::start("synchronizedCallback", "", true);
      #endif
      ROS_ERROR("COPY PC");
      ROS_INFO_STREAM("pointcloud dimensions " << pointCloudMessage->height << " " <<pointCloudMessage->width);
      // For simulation purposes, the width and height parameters of the
      // point cloud must be set. Copy the input point cloud message to another
      // one so that these can be set manually if and when needed
      PointCloudPtr pointCloud(new PointCloud);
      pcl::copyPointCloud(*pointCloudMessage, *pointCloud);
      
      ROS_ERROR("POINTCLOUD COPIED");
      // The input point cloud is unorganized, in other words,
      // simulation is running. Variables are needed to be set in order for
      // the point cloud to be functionally exploitable.
      if (pointCloud->height == 1)
      {
        // The point cloud's height
        pointCloud->height = Parameters::Image::HEIGHT;

        // The point cloud's width
        pointCloud->width = Parameters::Image::WIDTH;
      }


      // Extract the RGB image from the point cloud
      cv::Mat rgbImage = MessageConversions::convertPointCloudMessageToImage(
        pointCloud, CV_8UC3);

      // Convert the rgbImage to a ROS message
      cv_bridge::CvImagePtr rgbImageMessagePtr(new cv_bridge::CvImage());

      rgbImageMessagePtr->encoding = sensor_msgs::image_encodings::BGR8;
      rgbImageMessagePtr->image = rgbImage;

      // Publish the synchronized rgb image
      synchronizedRGBImagePublisher_.publish(rgbImageMessagePtr->toImageMsg());


      // Extract the depth image from the point cloud
      cv::Mat depthImage = MessageConversions::convertPointCloudMessageToImage(
        pointCloud, CV_32FC1);

      // Convert the depthImage to a ROS message
      cv_bridge::CvImagePtr depthImageMessagePtr(new cv_bridge::CvImage());

      depthImageMessagePtr->encoding = sensor_msgs::image_encodings::TYPE_32FC1;
      depthImageMessagePtr->image = depthImage;

      // Publish the synchronized depth image
      synchronizedDepthImagePublisher_.publish(
        depthImageMessagePtr->toImageMsg());

      // Publish the synchronized thermal info
      synchronizedThermalImagePublisher_.publish(thermalMessage);
     
      // Publish the synchronized point cloud
      synchronizedPointCloudPublisher_.publish(pointCloud);
      ROS_ERROR("ALL PUBLISHED TO THEIR NODES");
      #ifdef DEBUG_TIME
      Timer::tick("synchronizedCallback");
      Timer::printAllMeansTree();
      #endif
    }
  }


  /**
    @brief The callback executed when the Synchronizer node aquires
    the pointcloud2 from the Rgbd-T synchronizer node.
    Here the number of messages sent from the Rgbd-T synchronizer node
    are checked and if they are two synchronizer node publishes them 
    further to each node that uses them. 
    @param[in] pointCloud2Message [const sensor_msgs::PointCloud2&] 
    the input pointcloud2 from Rgbd-T synchronizer node.
    @return void
    **/
   void RgbDepthSynchronizer::inputPointCloud2Callback(
    const sensor_msgs::PointCloud2& pointCloud2Message)
 
   {
     // Force pointcloud callback to start after thermal callback
     if(!isLocked_ && messageNum_ > 0 && messageNum_ < 2)
     {  
     ROS_INFO("[Synchronizer Node] PointCloud2Callback called");

     // Shutdown the input pointcloud2 subscriber
     //inputPointCloud2Subscriber_.shutdown();

     // Increase the number of messages received from Rgbd-T synch node
     messageNum_++;
     
     ROS_ERROR("POINTCLOUD CALLBACK 2:%d",messageNum_);
     // Convert PointCloud2 message to PointCloud<T> type.
     pcl::PCLPointCloud2 pcl_pc;
     pcl_conversions::toPCL(pointCloud2Message, pcl_pc);

     pcl::fromPCLPointCloud2(pcl_pc, *copiedPc_);

     // Convert PointCloud<T> to PointCloud<T>::Ptr

     // Copy the incoming message

      ROS_INFO_STREAM("pointcloud dimensions " << copiedPc_->height << copiedPc_->width);
     // If the two messages have arrived set messageNum to zero for
     // the next cyrcle.
     if(messageNum_ == 2)
     {
       messageNum_ = 0;      

       // Call the function that processes all info
       inputPointCloudThermalCallback( copiedPc_, copiedThermal_);
     }
     }
   }


  /**
    @brief The callback executed when the Synchronizer node aquires
    the thermal info from the Rgbd-T synchronizer node.
    Here the number of messages sent from the Rgbd-T synchronizer node
    are checked and if they are two synchronizer node publishes them 
    further to each node that uses them. 
    @param[in] thermalMessage [const sensor_msgs::Image&]
    the input thermal info from Rgbd-T synchronizer node.
    @return void
    **/
  void RgbDepthSynchronizer::inputThermalCallback(
    const sensor_msgs::Image& thermalMessage)
  {
    // Force thermal callback to start before pointcloud callback
    if(!isLocked_ && messageNum_ < 1 )
    {
    ROS_INFO("[Synchronizer Node] ThermalCallback called");

    // Shutdown the input thermal subscriber
    //inputThermalSubscriber_.shutdown();

    // Increase the number of messages received from Rgbd-T synch node
    messageNum_++;

    ROS_ERROR(" CALLBACK THERMAL 1:%d",messageNum_);
    // Copy the incoming message
    copiedThermal_ = thermalMessage;

    // If the two messages have arrived set messageNum to zero for
    // the next cyrcle.
    /*if(messageNum_ == 2)
    {
      messageNum_ = 0;

      ROS_INFO_STREAM("THERMAL dimensions " << copiedPc_->height << copiedPc_->width);
      // Call the function that processes all info
      inputPointCloudThermalCallback(copiedPc_, copiedThermal_);
    }*/
    }
  } 

  /**
    @brief The callback executed when the Hole Fusion node requests
    from the synchronizer node to leave its subscription to the
    input pointcloud2 and flir topics.
    This happens when the state of the hole detector package is set
    to "off" so as to minimize processing resources.
    @param[in] msg [const std_msgs::Empty&] An empty message used to
    trigger the callback
    @return void
   **/
  void RgbDepthSynchronizer::leaveSubscriptionToInputPointCloudCallback(
    const std_msgs::Empty& msg)
  { ROS_INFO("SHUTDOWN SUBSCRIBERS");
    // Shutdown the input pointcloud2 subscriber
    inputPointCloud2Subscriber_.shutdown();

    // Shutdown the input thermal subscriber
    inputThermalSubscriber_.shutdown();
  }



  /**
    @brief The callback executed when the Hole Fusion node requests
    from the synchronizer node to subscribe to the input pointcloud2 and flir.
    This happens when the hole detector is in an "off" state, where the
    synchronizer node is not subscribed to the input point cloud(and flir) and
    transitions to an "on" state, where the synchronizer node needs to be
    subscribed to the input point cloud topic in order for the hole detector
    to function.
    @param[in] msg [const std_msgs::Empty&] An empty message used to
    trigger the callback
    @return void
   **/
  void RgbDepthSynchronizer::subscribeToInputPointCloudCallback(
    const std_msgs::Empty& msg)
  {
    inputPointCloud2Subscriber_ = nodeHandle_.subscribe(
      inputPointCloud2Topic_, 1, 
      &RgbDepthSynchronizer::inputPointCloud2Callback, this);

    inputThermalSubscriber_ = nodeHandle_.subscribe(
     inputThermalTopic_, 1,
     &RgbDepthSynchronizer::inputThermalCallback, this);

  }



  /**
    @brief The callback for the hole_fusion node request for the
    lock/unlock of the rgb_depth_synchronizer node.

    The synchronizer node, when subscribed to the input point cloud topic,
    receives in a second as many callbacks as the number of depth sensor's
    point cloud publications per second (currently 25). Because its
    function is to synchronize the input of the depth and rgb nodes,
    it needs to stay locked for the time that these nodes process their
    input. When both these nodes have finished processing their input
    images and the hole fusion node has processed the point cloud sent
    to him by the synchronizer node, the synchronizer
    (who has been locked all this time), can now be unlocked and therefore
    receive a new point cloud.
    @param[in] lockMsg [const std_msgs::Empty] An empty message used to
    trigger the callback
    @return void
   **/
  void RgbDepthSynchronizer::unlockCallback(const std_msgs::Empty& lockMsg)
  {
    ROS_ERROR("SYNCHRONIZER NODE UNLOCKED");
    isLocked_ = false;
  }

} // namespace pandora_vision
