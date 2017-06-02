//
// Created by huangqinjin on 5/31/17.
//

#include "apriltag.hpp"
#include "apriltag.h"

#include <opencv2/imgproc.hpp>

using namespace apriltag;

void detection::draw(cv::Mat& img, int flags)
{
    for (const tag& t : *this)
    {
        cv::line(img, cv::Point(t[0][0], t[0][1]),
                 cv::Point(t[1][0], t[1][1]),
                 cv::Scalar(0, 0xff, 0), 2);
        cv::line(img, cv::Point(t[0][0], t[0][1]),
                 cv::Point(t[3][0], t[3][1]),
                 cv::Scalar(0, 0, 0xff), 2);
        cv::line(img, cv::Point(t[1][0], t[1][1]),
                 cv::Point(t[2][0], t[2][1]),
                 cv::Scalar(0xff, 0, 0), 2);
        cv::line(img, cv::Point(t[2][0], t[2][1]),
                 cv::Point(t[3][0], t[3][1]),
                 cv::Scalar(0xff, 0, 0), 2);

        cv::String text = std::to_string(t.id());
        int fontface = cv::FONT_HERSHEY_SCRIPT_SIMPLEX;
        double fontscale = 1.0;
        int baseline;
        cv::Size textsize = cv::getTextSize(text, fontface, fontscale, 2,
                                            &baseline);
        cv::putText(img, text, cv::Point(t.center()[0]-textsize.width/2,
                                         t.center()[1]+textsize.height/2),
                    fontface, fontscale, cv::Scalar(0xff, 0x99, 0), 2);
    }
}

detection detector::detect(cv::Mat& img)
{
    if(img.type() == CV_8UC3)
    {
        cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
    }
    else
    {
        assert(img.type() == CV_8UC1);
    }

    image_u8 image = {
        static_cast<int32_t>(img.cols),
        static_cast<int32_t>(img.rows),
        static_cast<int32_t>(img.step[0]),
        static_cast<uint8_t*>(img.data)
    };
    return detect(&image);
}