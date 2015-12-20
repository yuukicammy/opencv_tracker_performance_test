#include "link_opencv.h"
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/tracking/tracker.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <random>

static cv::Mat Image;
static cv::Rect BBox;
static std::string WindowName;
static bool Paused;
static bool SelectObject = false;
static bool StartSelection = false;

static const char* Keys =
{ "{@tracker_algorithm | | tracker algorithm }"
"{@video_name        | | video name        }"
"{help h usage| |print this message   }"
};

static void Help()
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

	parser.about("OpenCV MultiTracker API Test");
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
	// for random colors
	std::random_device rnd; 
	std::mt19937 mt(rnd());
	std::uniform_int_distribution<> rand255(0, 255); 

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
	cv::Ptr<cv::MultiTracker> tracker = new  cv::MultiTracker(tracker_algorithm);
	if (tracker == NULL)
	{
		std::cout << "***Error in the instantiation of the tracker...***\n";
		return -1;
	}

	std::cout << "Hot Keys: \n"
		"\tq - quit the program\n"
		"\tp - pause video\n";

	//get the first frame
	cap >> frame;
	frame.copyTo(Image);
	imshow(WindowName, Image);
	std::vector<cv::Rect2d> bboxes;
	std::vector<cv::Scalar> colors;
	char c;
	while (true)
	{
		if (!Paused)
		{
			cap >> frame;
			frame.copyTo(Image);

			if (SelectObject)
			{
				//initializes the tracker
				if (tracker->add(Image, BBox))
				{
					bboxes.push_back(BBox);
					colors.push_back(cv::Scalar(rand255(mt), rand255(mt), rand255(mt)));
					SelectObject = false;
					StartSelection = false;
				}
			}
			else if (!bboxes.empty())
			{
				//updates the tracker
				if (tracker->update(frame, bboxes))
				{
					CV_Assert(bboxes.size() == colors.size());
					auto b_it = bboxes.cbegin();
					auto c_it = colors.cbegin();
					while (b_it != bboxes.end() && c_it != colors.end())
					{
						cv::rectangle(Image, *b_it, *c_it, 2, 1);
						b_it++;
						c_it++;
					}
				}
			}
			imshow(WindowName, Image);
		}

		c = (char)cv::waitKey(2);
		if (c == 'q')
			break;
		if (c == 'p')
			Paused = !Paused;
	}

	return 0;
}