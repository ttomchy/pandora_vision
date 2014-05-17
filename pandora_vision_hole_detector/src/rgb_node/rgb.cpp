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
* Author: Despoina Paschalidou
*********************************************************************/

#include "rgb_node/rgb.h"

namespace pandora_vision
{
  /**
    @brief Constructor
  **/
  Rgb::Rgb(): _nh(), holeNowON(false)
  {
    // Subscribe to the RGB image published by the
    // rgb_depth_synchronizer node
    _frameSubscriber = _nh.subscribe(
      Parameters::Topics::rgb_image_topic, 1,
      &Rgb::inputRgbImageCallback, this);

    // Advertise the candidate holes found by the depth node
    rgbCandidateHolesPublisher_ = _nh.advertise
      <vision_communications::CandidateHolesVectorMsg>(
      Parameters::Topics::rgb_candidate_holes_topic, 1000);

    // The dynamic reconfigure (RGB) parameter's callback
    server.setCallback(boost::bind(&Rgb::parametersCallback,
        this, _1, _2));

    ROS_INFO("[RGB node] : Created Rgb instance");
  }



  /**
    @brief Destructor
   **/
  Rgb::~Rgb()
  {
    ROS_DEBUG("[RGB node] : Destroying Hole Detection instance");
  }



  /**
    @brief Function called when new ROS message appears, for camera
    @param msg [const sensor_msgs::ImageConstPtr&] The message
    @return void
  */
  void Rgb::inputRgbImageCallback(const sensor_msgs::Image& msg)
  {
    #ifdef DEBUG_SHOW
    ROS_INFO("RGB node callback");
    #endif

    #ifdef DEBUG_TIME
    Timer::start("inputRgbImageCallback", "", true);
    #endif

    cv_bridge::CvImagePtr in_msg;
    in_msg = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    _holeFrame = in_msg->image.clone();
    _holeFrameTimestamp = msg.header.stamp;

    if (_holeFrame.empty() )
    {
      ROS_ERROR("[rgb_node] : No more Frames");
      return;
    }

    #ifdef DEBUG_SHOW
    if (Parameters::Rgb::show_rgb_image)
    {
      Visualization::show("RGB image", _holeFrame, 1);
    }
    #endif

    // Regardless of the image representation method, the RGB node
    // will publish the RGB image of original size to the Hole Fusion node
    cv::Mat holeFrameSent;
    _holeFrame.copyTo(holeFrameSent);

    // A value of 1 means that the rgb image is subtituted by its
    // low-low, wavelet analysis driven, part
    if (Parameters::Image::image_representation_method == 1)
    {
      Wavelets::getLowLow(_holeFrame, &_holeFrame);
    }

    // Find candidate holes in the current frame
    HolesConveyor conveyor = _holeDetector.findHoles(_holeFrame);

    // With candidate holes found, create the message that includes them
    // and the original RGB image and send them over to the HoleFusion node
    vision_communications::CandidateHolesVectorMsg rgbCandidateHolesMsg;

    MessageConversions::createCandidateHolesVectorMessage(conveyor,
      holeFrameSent,
      &rgbCandidateHolesMsg,
      sensor_msgs::image_encodings::TYPE_8UC3, msg);

    rgbCandidateHolesPublisher_.publish(rgbCandidateHolesMsg);

    #ifdef DEBUG_TIME
    Timer::tick("inputRgbImageCallback");
    Timer::printAllMeansTree();
    #endif
  }



  /**
    @brief The function called when a parameter is changed
    @param[in] config [const pandora_vision_hole_detector::rgb_cfgConfig&]
    @param[in] level [const uint32_t] The level (?)
    @return void
   **/
  void Rgb::parametersCallback(
    const pandora_vision_hole_detector::rgb_cfgConfig& config,
    const uint32_t& level)
  {
    #ifdef DEBUG_SHOW
    ROS_INFO("[RGB node] Parameters callback called");
    #endif

    // Blob detection - specific parameters
    Parameters::Blob::blob_min_threshold =
      config.blob_min_threshold;

    Parameters::Blob::blob_max_threshold =
      config.blob_max_threshold;

    Parameters::Blob::blob_threshold_step =
      config.blob_threshold_step;

    // In wavelet mode, the image shrinks by a factor of 4
    if (config.image_representation_method == 0)
    {
      Parameters::Blob::blob_min_area =
        config.blob_min_area;

      Parameters::Blob::blob_max_area =
        config.blob_max_area;
    }
    else if (config.image_representation_method == 1)
    {
      Parameters::Blob::blob_min_area =
        static_cast<int>(config.blob_min_area / 4);

      Parameters::Blob::blob_max_area =
        static_cast<int>(config.blob_max_area / 4);
    }

    Parameters::Blob::blob_min_convexity =
      config.blob_min_convexity;

    Parameters::Blob::blob_max_convexity =
      config.blob_max_convexity;

    Parameters::Blob::blob_min_inertia_ratio =
      config.blob_min_inertia_ratio;

    Parameters::Blob::blob_max_circularity =
      config.blob_max_circularity;

    Parameters::Blob::blob_min_circularity =
      config.blob_min_circularity;

    Parameters::Blob::blob_filter_by_color =
      config.blob_filter_by_color;

    Parameters::Blob::blob_filter_by_circularity =
      config.blob_filter_by_circularity;


    // Debug
    Parameters::Debug::show_find_holes =
      config.show_find_holes;
    Parameters::Debug::show_find_holes_size =
      config.show_find_holes_size;

    Parameters::Debug::show_produce_edges =
      config.show_produce_edges;
    Parameters::Debug::show_produce_edges_size =
      config.show_produce_edges_size;

    Parameters::Debug::show_denoise_edges =
      config.show_denoise_edges;
    Parameters::Debug::show_denoise_edges_size =
      config.show_denoise_edges_size;

    Parameters::Debug::show_connect_pairs =
      config.show_connect_pairs;
    Parameters::Debug::show_connect_pairs_size =
      config.show_connect_pairs_size;

    Parameters::Debug::show_get_shapes_clear_border  =
      config.show_get_shapes_clear_border;
    Parameters::Debug::show_get_shapes_clear_border_size =
      config.show_get_shapes_clear_border_size;


    // Edge detection parameters
    Parameters::Edge::edge_detection_method =
      config.edge_detection_method;

    // canny parameters
    Parameters::Edge::canny_ratio =
      config.canny_ratio;

    Parameters::Edge::canny_kernel_size =
      config.canny_kernel_size;

    Parameters::Edge::canny_low_threshold =
      config.canny_low_threshold;

    Parameters::Edge::canny_blur_noise_kernel_size =
      config.canny_blur_noise_kernel_size;

    Parameters::Edge::contrast_enhance_beta =
      config.contrast_enhance_beta;

    Parameters::Edge::contrast_enhance_alpha =
      config.contrast_enhance_alpha;


    // Parameters needed for histogram calculation
    Parameters::Histogram::number_of_hue_bins =
      config.number_of_hue_bins;

    Parameters::Histogram::number_of_saturation_bins =
      config.number_of_saturation_bins;

    Parameters::Histogram::number_of_value_bins =
      config.number_of_value_bins;

    Parameters::Histogram::secondary_channel =
      config.secondary_channel;


    // RGB image representation method.
    // 0 if the depth image used is the one obtained from the depth sensor,
    // unadulterated
    // 1 through wavelet representation
    Parameters::Image::image_representation_method =
      config.image_representation_method;


    // The detection method used to obtain the outline of a blob
    // 0 for detecting by means of brushfire
    // 1 for detecting by means of raycasting
    Parameters::Outline::outline_detection_method =
      config.outline_detection_method;

    // When using raycast instead of brushfire to find the (approximate here)
    // outline of blobs, raycast_keypoint_partitions dictates the number of
    // rays, or equivalently, the number of partitions in which the blob is
    // partitioned in search of the blob's borders
    Parameters::Outline::raycast_keypoint_partitions =
      config.raycast_keypoint_partitions;

    //<! Loose ends connection parameters
    Parameters::Outline::AB_to_MO_ratio = config.AB_to_MO_ratio;

    //!< In wavelet mode, the image shrinks by a factor of 4
    if (config.image_representation_method == 0)
    {
      Parameters::Outline::minimum_curve_points =
        config.minimum_curve_points;
    }
    else if (config.image_representation_method == 1)
    {
      Parameters::Outline::minimum_curve_points =
        static_cast<int>(config.minimum_curve_points / 4);
    }


    // Show the rgb image that arrives in the rgb node
    Parameters::Rgb::show_rgb_image =
      config.show_rgb_image;

    // RGB image segmentation parameters

    // Selects the method for extracting a RGB image's edges.
    // Choices are via segmentation and via backprojection
    Parameters::Rgb::edges_extraction_method =
      config.edges_extraction_method;

    // The threshold applied to the backprojection of the RGB image
    // captured by the image sensor
    Parameters::Rgb::compute_edges_backprojection_threshold =
      config.compute_edges_backprojection_threshold;

    // Parameters specific to the pyrMeanShiftFiltering method
    Parameters::Rgb::spatial_window_radius =
      config.spatial_window_radius;
    Parameters::Rgb::color_window_radius =
      config.color_window_radius;
    Parameters::Rgb::maximum_level_pyramid_segmentation =
      config.maximum_level_pyramid_segmentation;

    // Applies advanced blurring to achieve segmentation or normal
    Parameters::Rgb::segmentation_blur_method =
      config.segmentation_blur_method;

    // FloodFill options regarding minimum and maximum colour difference
    Parameters::Rgb::floodfill_lower_colour_difference =
      config.floodfill_lower_colour_difference;
    Parameters::Rgb::floodfill_upper_colour_difference =
      config.floodfill_upper_colour_difference;

    // Watershed-specific parameters
    Parameters::Rgb::watershed_foreground_dilation_factor =
      config.watershed_foreground_dilation_factor;
    Parameters::Rgb::watershed_foreground_erosion_factor =
      config.watershed_foreground_erosion_factor;
    Parameters::Rgb::watershed_background_dilation_factor =
      config.watershed_background_dilation_factor;
    Parameters::Rgb::watershed_background_erosion_factor =
      config.watershed_background_erosion_factor;

  }

} // namespace pandora_vision
