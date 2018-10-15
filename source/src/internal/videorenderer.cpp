#define _WINSOCKAPI_ 
#include <third_party/libyuv/include/libyuv/convert_argb.h>
#include <stack>
#include "internal/videorenderer.h"

VideoRenderer::VideoRenderer(int w, int h,
    rtc::scoped_refptr<webrtc::VideoTrackInterface> track_to_render,
	std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> i_stack)
  : VideoSink(track_to_render), /*rendered_track(track_to_render),*/ width(w), height(h), stack(i_stack) {

  /*rendered_track->AddOrUpdateSink(this, rtc::VideoSinkWants());*/

  
}

VideoRenderer::~VideoRenderer() {
  //rendered_track->RemoveSink(this);
}

void VideoRenderer::SetSize(int w, int h) {
  RTC_LOG(INFO) << "VideoRenderer::SetSize(" << w << "," << h << ")";

  if (width == w && height == h) {
    return;
  }

  width = w;
  height = h;

  image.reset(new std::uint8_t[width * height * 4]);
}

void VideoRenderer::OnFrame(const webrtc::VideoFrame& video_frame) {

  RTC_LOG(INFO) << "VideoRenderer::OnFrame()";

  rtc::scoped_refptr<webrtc::I420BufferInterface> buffer(video_frame.video_frame_buffer()->ToI420());

  SetSize(buffer->width(), buffer->height());

  libyuv::I420ToARGB(buffer->DataY(), buffer->StrideY(),
                     buffer->DataU(), buffer->StrideU(),
                     buffer->DataV(), buffer->StrideV(),
                     image.get(),
                     width * 4,
                     buffer->width(), buffer->height());

  cv::Mat img = cv::Mat(cv::Size(width, height), CV_8UC4, image.get()).clone();
  cv::Mat overFlow;
  
  stack->clear();

  stack->push(img);

  /*capturer.Render(image.get(), width, height);*/
}

