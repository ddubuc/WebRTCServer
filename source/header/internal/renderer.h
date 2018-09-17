#ifndef  OMEKASHI_RENDERER_H
#define OMEKASHI_RENDERER_H

#include <cstdint>
#include <memory>
#include <opencv2/core/mat.hpp>

struct RenderMode {
  bool onAir;
  bool isKaicho;
  bool useEye;
  bool useMustache;
  RenderMode();
};

class Renderer {
public:
  Renderer();
  virtual ~Renderer();

  virtual int GetWidth();
  virtual int GetHeight();
  virtual std::uint8_t* Image() = 0;

  virtual void PutImage(std::uint8_t* image, int width, int height) = 0;
  virtual cv::Mat & GetMat() = 0;

  virtual void Filter(const RenderMode& mode) = 0;

  virtual void SetFrameRate(int frame_rate);

protected:
  int width;
  int height;

  int frame_rate;
};

class RendererFactory {
public:
  static std::unique_ptr<Renderer> Create();
};

#endif // OMEKASHI_RENDERER_H
