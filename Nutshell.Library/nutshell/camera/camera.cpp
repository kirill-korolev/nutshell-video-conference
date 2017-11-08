//
//  camera.cpp
//  Nutshell.Library
//
//  Created by Kirill Korolev on 08/11/2017.
//  Copyright © 2017 Kirill Korolev. All rights reserved.
//

#include "camera.hpp"

#include <vector>

namespace nutshell { namespace camera {
    
    Camera::Camera(const char* title)
    {
        title_ = title;
        device_ = DEFAULT_DEVICE;
        capture_ = cv::VideoCapture(device_);
        
        cv::namedWindow(title_, CV_WINDOW_AUTOSIZE);
        
        if(!capture_.isOpened())
            release();
    }
    
    Camera::~Camera()
    {
        release();
    }
    
    void Camera::release()
    {
        capture_.release();
    }
    
    void Camera::capture()
    {
        cv::Mat localFrame, remoteFrame;
        std::vector<int> params;
        std::vector<uchar_t> encoded;
        int totalPacks = 0;
        
        params.push_back(CV_IMWRITE_JPEG_QUALITY);
        params.push_back(settings_.quality());
        
        while(1)
        {
            capture_ >> localFrame;
            if(localFrame.size().width == 0) continue;
            cv::resize(localFrame, remoteFrame, settings_.size(), 0, 0, CV_INTER_LINEAR);
            imencode(".jpg", remoteFrame, encoded, params);
            imshow(title_, remoteFrame);
            
            totalPacks = 1 + (encoded.size() - 1) / settings_.packSize();
            int* buffer = new int(totalPacks);
            
            cv::waitKey(settings_.frameInterval());
            
            delete buffer;
        }
    }
    
}}
