#ifndef OMEKASHI_OPENCVDETECTOR_H
#define OMEKASHI_OPENCVDETECTOR_H

#include <chrono>

#include "opencv2/opencv.hpp"

struct OffsetScale {
  double width;
  double height;
  double x;
  double y;
};

class OpenCVDetector {
  cv::Mat image;
  OffsetScale offsetScale;
  cv::CascadeClassifier detector;
  float scaleFactor;
  int minNeighbors;

  std::chrono::system_clock::time_point last_found_time;
  std::vector<cv::Rect> last_found_results;

public:
  explicit OpenCVDetector (const std::string& image, const std::string& cascade, OffsetScale offset,
                           float scale, int minNeighbors);
  virtual ~OpenCVDetector ();

  int Draw(cv::Mat target);
};

#endif // OMEKASHI_OPENCVDETECTOR_H
