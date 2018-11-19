#define NOMINMAX

#include <memory>
#include <thread>
#include <common_video/libyuv/include/webrtc_libyuv.h>
#include <api/video/i420_buffer.h>
#include <opencv2/imgproc.hpp>

#include "internal/webrtc.h"
#include "internal/CustomOpenCVCapturer.h"
#include <media/base/videocapturer.h>
#include <libyuv/rotate.h>
#include <libyuv/convert.h>

#include <rtc_base/logging.h>

using std::endl;
using namespace rtc;

CustomOpenCVCapturer::CustomOpenCVCapturer(std::shared_ptr<core::queue::ConcurrentQueue<cv::Mat> > i_stack)
	: now_rendering(false)
	  , start()
	  , end(std::chrono::system_clock::now())
	  , frame_timer(std::chrono::system_clock::now()), frame_counter(0)
	  , stack(i_stack)
{
	
}

CustomOpenCVCapturer::~CustomOpenCVCapturer()
{

	RTC_LOG(INFO) << "CustomeOpenCVCapturer is stopping capture";
	Stop();
	
	RTC_LOG(INFO) << "CustomeOpenCVCapturer is removing";
	
}

void CustomOpenCVCapturer::PushFrame()
{
	while (now_rendering)
	{
		// if rendering is too slow then skip.
		start = std::chrono::system_clock::now();
		/*if (std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() > 10000)
		{
			RTC_LOG(INFO) << "rendering is too slow, so skiped.";
			end = std::chrono::system_clock::now();
			continue;
		}*/

		// frame rate count;
		frame_counter++;
		if (std::chrono::duration_cast<std::chrono::milliseconds>(start - frame_timer).count() >= 1000)
		{
			//renderer->SetFrameRate(frame_counter);

			frame_timer = std::chrono::system_clock::now();
			frame_counter = 0;
		}
		cv::Mat popped;
		cv::Mat I420Mat;
		if (!stack)
		{
			RTC_LOG(LS_WARNING) << "Frame buffering isn't yet set";
			continue;
		}
		if (!stack->trypop_until(popped, 500)) // 500ms is like infinity but check if 
		{
			RTC_LOG(LS_WARNING) << "Fail to pop";
			continue;
		}
		else if (popped.empty() || popped.size().height == 0 || popped.size().width == 0)
		{
			RTC_LOG(LS_WARNING) << "Fail to pop Image is empty";
			continue;
		}
		

		if (!now_rendering) break;

		int buf_width = popped.size().width;
		int buf_height = popped.size().height;
		
		if (popped.channels() == 3)
			cv::cvtColor(popped, I420Mat, CV_BGR2BGRA);
		else if (popped.channels() == 1)
			cv::cvtColor(popped, I420Mat, CV_GRAY2BGRA);
		else if (popped.channels() == 4)
			I420Mat = popped;
		else
			RTC_LOG(LS_WARNING) << "Fail to convert";

		//    rtc::scoped_refptr<webrtc::I420Buffer> buffer = webrtc::I420Buffer::Create(width, height);
		rtc::scoped_refptr<webrtc::I420Buffer> buffer = webrtc::I420Buffer::Create(
			buf_width, buf_height, buf_width, (buf_width + 1) / 2, (buf_width + 1) / 2);
		/*const int conversionResult = libyuv::ARGBToI420(I420Mat.ptr(), buf_width * buf_height, 
			buffer.get()->MutableDataY(), buffer.get()->StrideY(), 
			buffer.get()->MutableDataU(), buffer.get()->StrideU(), 
			buffer.get()->MutableDataV(), buffer.get()->StrideV(), 
			buf_width, buf_height);*/
		const int conversionResult = libyuv::ConvertToI420((const uint8_t*)I420Mat.ptr(), buf_width * buf_height * 4,
							buffer->MutableDataY(), buffer->StrideY(),
							buffer->MutableDataU(), buffer->StrideU(),
							buffer->MutableDataV(), buffer->StrideV(),
							0, 0,
							buf_width, buf_height,
							buf_width, buf_height,
							libyuv::kRotate0, cricket::FOURCC_ARGB);		
		//const int conversionResult = libyuv::ConvertToI420(
		//	I420Mat.ptr(), 0, 0, // No cropping
		//	buf_width, buf_height, CalcBufferSize(webrtc::VideoType::kARGB, buf_width, buf_height),
		//	webrtc::kVideoRotation_0, buffer.get());

		if (conversionResult < 0)
		{
			RTC_LOG(LS_ERROR) << "Failed to convert capture frame from type "
				<< static_cast<int>(webrtc::VideoType::kARGB) << "to I420.";
			continue;
		}

		webrtc::VideoFrame frame(buffer, 0, rtc::TimeMillis(), webrtc::kVideoRotation_0);
		frame.set_ntp_time_ms(0);

		OnFrame(frame, buf_width, buf_height);

		end = std::chrono::system_clock::now();
		RTC_LOG(LS_VERBOSE) << "frame used "
			<< std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
			<< " micro sec.";
	}
}

cricket::CaptureState CustomOpenCVCapturer::Start(const cricket::VideoFormat& capture_format)
{
	RTC_LOG(INFO) << "CustomVideoCapture start.";
	if (capture_state() == cricket::CS_RUNNING)
	{
		RTC_LOG(LS_ERROR) << "Start called when it's already started.";
		return capture_state();
	}

	now_rendering = true;
	SetCaptureFormat(&capture_format);
	renderer_task.reset();
	renderer_task.reset(new std::thread([this]()
	{
		PushFrame();
	}));

	
	return cricket::CS_RUNNING;
}

void CustomOpenCVCapturer::Stop()
{
	RTC_LOG(INFO) << "CustomVideoCapture::Stop()";
	now_rendering = false;
	
	if (renderer_task && renderer_task->joinable())
	{
		renderer_task->join();
		renderer_task.reset();
	}

	if (capture_state() == cricket::CS_STOPPED)
	{
		//RTC_LOG(LS_ERROR) << "Stop called when it's already stopped.";
		return;
	}


	SetCaptureFormat(nullptr);
	SetCaptureState(cricket::CS_STOPPED);
}


bool CustomOpenCVCapturer::IsRunning()
{
	return capture_state() == cricket::CS_RUNNING;
}

bool CustomOpenCVCapturer::GetPreferredFourccs(std::vector<uint32_t>* fourccs)
{
	if (!fourccs)
		return false;
	fourccs->push_back(cricket::FOURCC_I420);
	return true;
}

bool CustomOpenCVCapturer::GetBestCaptureFormat(const cricket::VideoFormat& desired, cricket::VideoFormat* best_format)
{
	if (!best_format)
		return false;

	// Use the desired format as the best format.
	best_format->width = desired.width;
	best_format->height = desired.height;
	best_format->fourcc = cricket::FOURCC_I420;
	best_format->interval = desired.interval;
	return true;
}

bool CustomOpenCVCapturer::IsScreencast() const
{
	return false;
}
 
