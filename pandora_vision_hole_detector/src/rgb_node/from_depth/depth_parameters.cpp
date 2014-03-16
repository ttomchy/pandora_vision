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
* Authors: Alexandros Filotheou, Manos Tsardoulias
*********************************************************************/

#include "depth_node/depth_parameters.h"

namespace pandora_vision
{
  //!< Kanny parameters
  int DepthParameters::kanny_ratio = 3;
  int DepthParameters::kanny_kernel_size = 3;
  int DepthParameters::kanny_low_threshold = 50;
  int DepthParameters::kanny_blur_noise_kernel_size = 3;

  float DepthParameters::contrast_enhance_alpha = 2;
  float DepthParameters::contrast_enhance_beta = 2;


  //!< Threshold parameters
  int DepthParameters::threshold_lower_value = 10;
  int DepthParameters::adaptive_max_value = 255;
  int DepthParameters::adaptive_method = 0;
  int DepthParameters::adaptive_block_size = 7;
  int DepthParameters::adaptive_c_value = 7;

  //!< Blob detection parameters
  int DepthParameters::blob_min_threshold = 0;
  int DepthParameters::blob_max_threshold = 1255;
  int DepthParameters::blob_threshold_step = 5;
  int DepthParameters::blob_min_area = 550;
  int DepthParameters::blob_max_area = 300000;
  double DepthParameters::blob_min_convexity = 0;
  double DepthParameters::blob_max_convexity = 100;
  double DepthParameters::blob_min_inertia_ratio = 0;
  double DepthParameters::blob_max_circularity = 1.0;
  double DepthParameters::blob_min_circularity = 0.3;
  bool DepthParameters::blob_filter_by_color = 0;
  bool DepthParameters::blob_filter_by_circularity = 1;

  //!< Bounding boxes parameters
  int DepthParameters::bounding_box_min_area_threshold = 550;
  int DepthParameters::bounding_box_detection_method = 0;
  int DepthParameters::raycast_keypoint_partitions = 8;

  //<! Loose ends connection parameters
  int DepthParameters::AB_to_MO_ratio = 2;


  //!< Interpolation parameters
  int DepthParameters::interpolation_method = 0;

  //!< Hole checkers
  int DepthParameters::run_checker_depth_diff = 1;
  int DepthParameters::run_checker_depth_area = 3;
  int DepthParameters::run_checker_brushfire_outline_to_rectangle = 4;
  int DepthParameters::run_checker_outline_of_rectangle = 2;
  int DepthParameters::rectangle_inflation_size = 20;
  float DepthParameters::depth_difference = 0.4;

  //!< Plane detection
  int DepthParameters::segmentation_method = 0;
  int DepthParameters::max_iterations = 1000;
  double DepthParameters::num_points_to_exclude = 0.1;
  double DepthParameters::point_to_plane_distance_threshold = 0.01;

  //!< Method to scale the CV_32FC1 image to CV_8UC1
  int DepthParameters::scale_method = 0;

  //!< Debug
  bool DepthParameters::debug_show_find_holes = false;
  int DepthParameters::debug_show_find_holes_size = 1000;
  bool DepthParameters::debug_time_find_holes = false;

  bool DepthParameters::debug_show_denoise_edges = false;
  int DepthParameters::debug_show_denoise_edges_size = 900;

  bool DepthParameters::debug_show_connect_pairs = false;
  int DepthParameters::debug_show_connect_pairs_size = 1200;
  bool DepthParameters::debug_print_connect_pairs = false;

  bool DepthParameters::debug_show_get_shapes_clear_border = false;
  int DepthParameters::debug_show_get_shapes_clear_border_size  = 1200;

  bool DepthParameters::debug_show_check_holes = false;
  int DepthParameters::debug_show_check_holes_size = 1200;

  int DepthParameters::minimum_curve_points = 1200;
  //!<----------------------------------------------------------------------!>//

  DepthParameters::DepthParameters(void)
  {
    ros::NodeHandle _nodeHandle;

    server.setCallback(boost::bind(&DepthParameters::callback, this,_1, _2));
  }

  /**
    @brief The function called when a parateter is changed
    @param[in] config [const pandora_vision_hole_detector::depth_cfgConfig&]
    @param[in] level [const uint32_t] The level (?)
    @return void
   **/
  void DepthParameters::callback(
    const pandora_vision_hole_detector::depth_cfgConfig& config,
    const uint32_t& level)
  {
    #ifdef DEBUG_SHOW
    ROS_INFO("DepthParameters callback called");
    #endif
    DepthParameters::kanny_ratio = config.kanny_ratio;
    DepthParameters::kanny_kernel_size = config.kanny_kernel_size;
    DepthParameters::kanny_low_threshold = config.kanny_low_threshold;
    DepthParameters::kanny_blur_noise_kernel_size =
      config.kanny_blur_noise_kernel_size;
    DepthParameters::contrast_enhance_beta = config.contrast_enhance_beta;
    DepthParameters::contrast_enhance_alpha = config.contrast_enhance_alpha;
    DepthParameters::threshold_lower_value = config.threshold_lower_value;
    DepthParameters::adaptive_max_value = config.adaptive_max_value;
    DepthParameters::adaptive_method = config.adaptive_method;
    DepthParameters::adaptive_block_size = config.adaptive_block_size;
    DepthParameters::adaptive_c_value = config.adaptive_c_value;
    DepthParameters::blob_min_threshold = config.blob_min_threshold;
    DepthParameters::blob_max_threshold = config.blob_max_threshold;
    DepthParameters::blob_threshold_step = config.blob_threshold_step;
    DepthParameters::blob_min_area = config.blob_min_area;
    DepthParameters::blob_max_area = config.blob_max_area;
    DepthParameters::blob_min_convexity = config.blob_min_convexity;
    DepthParameters::blob_max_convexity = config.blob_max_convexity;
    DepthParameters::blob_min_inertia_ratio = config.blob_min_inertia_ratio;
    DepthParameters::blob_max_circularity = config.blob_max_circularity;
    DepthParameters::blob_min_circularity = config.blob_min_circularity;
    DepthParameters::blob_filter_by_color = config.blob_filter_by_color;
    DepthParameters::blob_filter_by_circularity =
      config.blob_filter_by_circularity;
    DepthParameters::bounding_box_min_area_threshold =
      config.bounding_box_min_area_threshold;
    DepthParameters::bounding_box_detection_method =
      config.bounding_box_detection_method;
    DepthParameters::raycast_keypoint_partitions =
      config.raycast_keypoint_partitions;
    DepthParameters::AB_to_MO_ratio = config.AB_to_MO_ratio;
    DepthParameters::interpolation_method = config.interpolation_method;
    DepthParameters::run_checker_depth_diff = config.run_checker_depth_diff;
    DepthParameters::run_checker_outline_of_rectangle =
      config.run_checker_outline_of_rectangle;
    DepthParameters::run_checker_depth_area = config.run_checker_depth_area;
    DepthParameters::run_checker_brushfire_outline_to_rectangle =
      config.run_checker_brushfire_outline_to_rectangle;
    DepthParameters::rectangle_inflation_size = config.rectangle_inflation_size;
    DepthParameters::depth_difference = config.depth_difference;
    DepthParameters::segmentation_method = config.segmentation_method;
    DepthParameters::max_iterations = config.max_iterations;
    DepthParameters::num_points_to_exclude = config.num_points_to_exclude;
    DepthParameters::point_to_plane_distance_threshold =
      config.point_to_plane_distance_threshold;
    DepthParameters::scale_method = config.scale_method;
    DepthParameters::debug_show_find_holes = config.debug_show_find_holes;
    DepthParameters::debug_show_find_holes_size =
      config.debug_show_find_holes_size;
    DepthParameters::debug_time_find_holes = config.debug_time_find_holes;
    DepthParameters::debug_show_denoise_edges = config.debug_show_denoise_edges;
    DepthParameters::debug_show_denoise_edges_size =
      config.debug_show_denoise_edges_size;
    DepthParameters::debug_show_connect_pairs = config.debug_show_connect_pairs;
    DepthParameters::debug_show_connect_pairs_size =
      config.debug_show_connect_pairs_size;

    DepthParameters::debug_show_get_shapes_clear_border  =
      config.debug_show_get_shapes_clear_border;
    DepthParameters::debug_show_get_shapes_clear_border_size =
      config.debug_show_get_shapes_clear_border_size;

    DepthParameters::debug_show_check_holes = config.debug_show_check_holes;
    DepthParameters::debug_show_check_holes_size =
      config.debug_show_check_holes_size;

    DepthParameters::minimum_curve_points = config.minimum_curve_points;
  }

  /**
    @brief Retrieve the interpolation method
    @return int The interpolation method
   **/
  int DepthParameters::getInterpolationMethod()
  {
    return DepthParameters::interpolation_method;
  }
}