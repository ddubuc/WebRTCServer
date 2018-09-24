#pragma once

#include "webrtc.h"
#include "renderer.h"
#include "session.h"
#include "ConcurrentQueue.h"
#include <chrono>
#include <thread>

class CustomOpenCVCapturer :
        public cricket::VideoCapturer
{
public:
    CustomOpenCVCapturer(std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat>> & i_stack);
    virtual ~CustomOpenCVCapturer();
 
    // cricket::VideoCapturer implementation.
    virtual cricket::CaptureState Start(const cricket::VideoFormat& capture_format) override;
    virtual void Stop() override;
    virtual bool IsRunning() override;
    virtual bool GetPreferredFourccs(std::vector<uint32_t>* fourccs) override;
    virtual bool GetBestCaptureFormat(const cricket::VideoFormat& desired, cricket::VideoFormat* best_format) override;
    virtual bool IsScreencast() const override;
 
    void PushFrame();

private:
	std::unique_ptr<std::thread> renderer_task;

    std::atomic<bool> now_rendering;

    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;
    std::chrono::system_clock::time_point frame_timer;
    int frame_counter;
	std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat> > stack;
};

