#ifndef OMEKASHI_CUSTOMVIDEOCAPTURER_H
#define OMEKASHI_CUSTOMVIDEOCAPTURER_H
 
#include "webrtc/media/base/videocapturer.h"
#include "renderer.h"
#include "session.h"
#include "ConcurrentQueue.h"
 
#include <chrono>
#include <deque>
#include <opencv2/core/mat.hpp>

class CustomVideoCapturer :
        public cricket::VideoCapturer
{
public:
    explicit CustomVideoCapturer(const Session& session, std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> i_stack);
    virtual ~CustomVideoCapturer();
 
    // cricket::VideoCapturer implementation.
    virtual cricket::CaptureState Start(const cricket::VideoFormat& capture_format) override;
    virtual void Stop() override;
    virtual bool IsRunning() override;
    virtual bool GetPreferredFourccs(std::vector<uint32_t>* fourccs) override;
    virtual bool GetBestCaptureFormat(const cricket::VideoFormat& desired, cricket::VideoFormat* best_format) override;
    virtual bool IsScreencast() const override;
 
    virtual void Render(std::uint8_t* image, const int width, const int height);

private:
    bool now_rendering;
    std::unique_ptr<Renderer> renderer;
	std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> stack;

    const Session& session;

    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;
    std::chrono::system_clock::time_point frame_timer;
    int frame_counter;
};

#endif // OMEKASHI_CUSTOMVIDEOCAPTURER_H
