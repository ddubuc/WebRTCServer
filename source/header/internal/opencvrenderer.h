#ifndef OMEKASHI_OPENCVRENDERER_H
#define OMEKASHI_OPENCVRENDERER_H

#include "renderer.h"
#include "opencvdetector.h"
//#include "gcpdetector.h"

#include <cstdint>
#include <memory>

#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>


class OpenCVRenderer : public Renderer {
public:
  OpenCVRenderer();
  virtual ~OpenCVRenderer();

  virtual std::uint8_t* Image() override;

  virtual void PutImage(std::uint8_t* image, int width, int height) override;
	virtual cv::Mat& GetMat();
	virtual void Filter(const RenderMode& mode) override;

protected:
  cv::Mat buffer;
  cv::Mat waitPicture;

  /*
  OpenCVDetector upperbody,
        face,
        smile,
        eye,
        mustache;
        */
  /*GCPDetector face;*/
};
#endif // MEKASHI_RENDER_H
