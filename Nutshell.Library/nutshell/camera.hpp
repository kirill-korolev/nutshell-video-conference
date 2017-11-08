//
//  camera.hpp
//  Nutshell.Library
//
//  Created by Kirill Korolev on 08/11/2017.
//  Copyright © 2017 Kirill Korolev. All rights reserved.
//

#ifndef camera_hpp
#define camera_hpp

#include <opencv2/opencv.hpp>
#include "config.h"

namespace nutshell { namespace camera {
    
    typedef int device_t;
    typedef int quality_t;
    typedef unsigned char uchar_t;
    
    class Camera
    {
    public:
        Camera(const char* title=DEFAULT_TITLE);
        ~Camera();
        void release();
        void capture();
    private:
        struct Settings
        {
        public:
            inline double frameInterval() { return frameInterval_; }
            inline int packSize() { return packSize_; }
            inline quality_t quality() { return quality_; }
            inline cv::Size size() { return size_; }
        private:
            double frameInterval_ = FRAME_INTERVAL;
            int packSize_ = PACK_SIZE;
            quality_t quality_ = ENCODE_QUALITY;
            cv::Size size_ = cv::Size(DEFAULT_FRAME_WIDTH, DEFAULT_FRAME_HEIGHT);
        };
        
        Camera(const Camera&);
        Camera& operator=(const Camera&);
        
        const char* title_;
        device_t device_;
        cv::VideoCapture capture_;
        Settings settings_;
    };
    
}}

#endif /* camera_hpp */
