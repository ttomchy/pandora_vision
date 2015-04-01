/******************************************************************************
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
* Authors: Manos Tsardoulias
*********************************************************************/

#ifndef PANDORA_VISION_ANNOTATOR_CONNECTOR
#define PANDORA_VISION_ANNOTATOR_CONNECTOR

#include "pandora_vision_annotator/pandora_vision_annotator_loader.h"

/**
@namespace pandora_vision
@brief The main namespace for pandora_vision
**/
namespace pandora_vision
{

  enum ImageStates
  {
    IDLE,
    VICTIM_CLICK,
    QR_CLICK,
    HAZMAT_CLICK,
    LANDOLTC_CLICK
  };

  enum States
  {
    OFFLINE,
    ONLINE,
    REALTIME
  };

  /**
  @class CConnector
  @brief Serves the Qt events of the main GUI window. Inherits from QObject
  **/
  class CConnector: public QObject
  {
    Q_OBJECT

    //------------------------------------------------------------------------//
    private:

      //!< Number of input arguments
      int   argc_;
      //!< Input arguments
      char**  argv_;

      std::vector<int> bbox_ready;

      std::vector<cv::Mat> frames;

      int currFrame;

      std::string package_path;

      std_msgs::Header msgHeader;
      //!< The loader of main GUI QWidget
      CLoader loader_;

      QImage localImage_;

      QImage backupImage_;

      ImageStates img_state_;

      States state_;

      bool eventFilter( QObject* watched, QEvent* event );

       cv::Mat QImage2Mat(QImage const& src);

    //------------------------------------------------------------------------//
    public:

      /**
      @brief Default contructor
      @param argc [int] Number of input arguments
      @param argv [char **] Input arguments
      @return void
      **/
      CConnector(int argc, char **argv);

      void show(void);

      QString getRosTopic(void);

      QString getBagName(void);

      void msgTimeStamp(const std_msgs::Header& msg);

      void setImage(QImage &img);

      void setFrames(const std::vector<cv::Mat>& x);

      void setcurrentFrame();

      void drawBox();


    //------------------------------------------------------------------------//
    public Q_SLOTS:

      void rosTopicPushButtonTriggered(void);

      void updateImage();

      void onlineRadioButtonChecked(void);
      void offlineRadioButtonChecked(void);
      void realTimeRadioButtonChecked(void);
      void victimPushButtonTriggered(void);
      void qrPushButtonTriggered(void);
      void landoltcPushButtonTriggered(void);
      void hazmatPushButtonTriggered(void);
      void submitPushButtonTriggered(void);
      void clearPushButtonTriggered(void);
      void nextFramePushButtonTriggered(void);
      void previousFramePushButtonTriggered(void);

    //------------------------------------------------------------------------//
    Q_SIGNALS:

      void rosTopicGiven(void);
      void onlineModeGiven(void);
      void offlineModeGiven(void);
  };
}

#endif
