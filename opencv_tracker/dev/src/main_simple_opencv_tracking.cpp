#include "link_opencv.h"
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/tracking/tracker.hpp>
#include <opencv2/highgui.hpp>

static cv::Mat Image;
static cv::Rect2d BBox;
static std::string WindowName;
static bool Paused;
static bool SelectObject = false;
static bool StartSelection = false;

static const char* Keys =
{ "{@tracker_algorithm | | tracker algorithm }"
"{@video_name        | | video name        }"
"{help h usage| |print this message   }"
};

static void Help(void)
{
	std::cout << "\nThis example shows the functionality of \"Long-term optical tracking API\""
		"-- pause video [p] and draw a bounding box around the target to start the tracker\n"
		"Call:\n"
		"./tracker <tracker_algorithm> <video_name>\n"
		<< std::endl;

	std::cout << "\n\nHot Keys: \n"
		"\tq - quit the program\n"
		"\tp - pause video\n";
}

static void OnMouse(int event, int x, int y, int, void*)
{
	if (!SelectObject)
	{
		switch (event)
		{
		case cv::EVENT_LBUTTONDOWN:
			//set origin of the bounding box
			StartSelection = true;
			BBox.x = x;
			BBox.y = y;
			break;
		case cv::EVENT_LBUTTONUP:
			//sei with and height of the bounding box
			BBox.width = std::abs(x - BBox.x);
			BBox.height = std::abs(y - BBox.y);
			Paused = false;
			SelectObject = true;
			break;
		case cv::EVENT_MOUSEMOVE:
			if (StartSelection)
			{
				//draw the bounding box
				cv::Mat currentFrame;
				Image.copyTo(currentFrame);
				cv::rectangle(currentFrame, cv::Point(BBox.x, BBox.y), cv::Point(x, y), cv::Scalar(255, 0, 0), 2, 1);
				cv::imshow(WindowName, currentFrame);
			}
			break;
		}
	}
}

bool CheckTrackerAlgType(cv::String& tracker_algorithm)
{
	if (tracker_algorithm == "BOOSTING" ||
		tracker_algorithm == "MIL" ||
		tracker_algorithm == "TLD" ||
		tracker_algorithm == "MEDIANFLOW" ||
		tracker_algorithm == "KCF")
	{
		std::cout << "Tracker Algorithm Type: " << tracker_algorithm << std::endl;
	}
	else{
		CV_Error(cv::Error::StsError, "Unsupported algorithm type " + tracker_algorithm + " is specified.");
	}
	return true;
}

int main(int argc, char** argv)
{
	cv::CommandLineParser parser(argc, argv, Keys);

	cv::String tracker_algorithm = parser.get<cv::String>(0);
	cv::String video_name = parser.get<cv::String>(1);

	parser.about("OpenCV Tracker API Test");
	if (parser.has("help"))
	{
		parser.printMessage();
		return 0;
	}

	if (tracker_algorithm.empty() || video_name.empty() || !parser.check())
	{
		parser.printErrors();
		return -1;
	}

	CheckTrackerAlgType(tracker_algorithm);

	//open the capture
	cv::VideoCapture cap;
	cap.open(video_name);

	if (!cap.isOpened())
	{
		Help();
		std::cout << "***Could not initialize capturing...***\n";
		std::cout << "Current parameter's value: \n";
		parser.printMessage();
		return -1;
	}

	cv::Mat frame;
	Paused = true;
	WindowName = "Tracking API: " + tracker_algorithm;
	cv::namedWindow(WindowName, 0);
	cv::setMouseCallback(WindowName, OnMouse, 0);

	//instantiates the specific Tracker
	cv::Ptr<cv::Tracker> tracker = cv::Tracker::create(tracker_algorithm);
	if (tracker == NULL)
	{
		std::cout << "***Error in the instantiation of the tracker...***\n";
		return -1;
	}

	std::cout << "\n\nHot Keys: \n"
		"\tq - quit the program\n"
		"\tp - pause video\n";

	//get the first frame
	cap >> frame;
	frame.copyTo(Image);
	cv::imshow(WindowName, Image);
	bool initialized = false;
	while (true)
	{
		if (!Paused)
		{
			cap >> frame;
			frame.copyTo(Image);

			if (SelectObject && !initialized)
			{
				//initializes the tracker
				if (tracker->init(Image, BBox))
				{
					initialized = true;
				}
			}
			else if (initialized)
			{
				//updates the tracker
				if (tracker->update(frame, BBox))
				{
					cv::rectangle(Image, BBox, cv::Scalar(0, 0, 255), 2, 1);
				}
				else{
					SelectObject = initialized = false;
				}
			}
			imshow(WindowName, Image);
		}
		char c = static_cast<char>(cv::waitKey(2));
		if (c == 'q')
			break;
		if (c == 'p')
			Paused = !Paused;
	}

	return 0;
}