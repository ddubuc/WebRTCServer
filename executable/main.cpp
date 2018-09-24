#include <stdio.h>
#include <WebRTCCapturer.h>
#include <WebRTCStreamer.h>
#include <opencv2/core/mat.hpp>

	

int main(int argc, char ** argv)
{
	const char * root = argv[1];
	WebRTCCapturer capture(9001, root);
	WebRTCStreamer streamer(9002, root);

	capture.startWebRTCServer();
	streamer.startWebRTCServer();
	int until = 3000000;

	while (--until)
	{
		cv::Mat img = capture.Capture();
		fprintf(stdout, "Size of image %d, %d\n", img.size().height, img.size().width);
		streamer.Send(img);
	}
	
	return 0;
}
