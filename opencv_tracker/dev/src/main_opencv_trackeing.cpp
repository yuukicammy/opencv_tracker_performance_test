#include "link_opencv.h"
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/tracking/tracker.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <iomanip>
#include <fstream>

#include "link_boost.h"
#include <boost/timer/timer.hpp>

const cv::String Keys =
"{help h usage| |print this message   }"
"{@tracker_algorithm| |OpenCV tracker algorithm }"
"{@seq_name| |sequence name }"
"{@seq_path| |sequence path }"
"{@start_frame| |start frame id }"
"{@end_frame| |end frame id }"
"{@digit| |number of digits in image file names }"
"{@ext| |extension of input images }"
"{@initial_x| |left point of the initial bounding box  }"
"{@initial_y| |top point of the initial bounding box }"
"{@initial_w| |width of the initial bounding box }"
"{@initial_h| |height of the initial bounding box }"
"{@save|0|flag for saving result images}"
"{@show|0|flag for showing result images}"
"{@tracker_path|./|output directory path of a tracking result file }"
"{@output_image_path|./|output directory path of result images}"
;

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
	boost::timer::cpu_timer timer;
	cv::CommandLineParser parser(argc, argv, Keys);

	parser.about("OpenCV Tracker for tracking benchmark v1.0");
	if (parser.has("help"))
	{
		parser.printMessage();
		return 0;
	}

	cv::String tracker_algorithm = parser.get<cv::String>(0);
	std::string seq_name = parser.get<std::string>(1);
	std::string seq_path = parser.get<std::string>(2);
	int start_frame = parser.get<int>(3);
	int end_frame = parser.get<int>(4);
	int digit = parser.get<int>(5);
	std::string ext = parser.get<std::string>(6);
	cv::Rect2d start_bb(parser.get<double>(7), parser.get<double>(8), parser.get<double>(9), parser.get<double>(10));
	bool save = parser.get<int>(11) == 0 ? false : true;
	bool show = parser.get<int>(12) == 0 ? false : true;
	std::string tracker_path = parser.get<std::string>(13);
	std::string output_image_path = parser.get<std::string>(14);

	if (!parser.check())
	{
		parser.printErrors();
		return -1;
	}

	CheckTrackerAlgType(tracker_algorithm);

	//instantiates the specific Tracker
	cv::Ptr<cv::Tracker> tracker = cv::Tracker::create(tracker_algorithm);
	if (tracker.empty())
	{
		std::cerr << "***Error in the instantiation of the tracker...***\n";
		return -1;
	}

	cv::Mat frame;
	cv::Rect2d bbox = start_bb;
	bool initialized = false;
	bool finished = false;
	int fcnt = 0;
	std::string filename;

	// result files
	std::ofstream ofs_bbox(tracker_path + seq_name + "_" + tracker_algorithm + ".txt");
	std::ofstream ofs_fps(tracker_path + seq_name + "_" + tracker_algorithm + "_FPS.txt");

	for (int f_id = start_frame; f_id <= end_frame; ++f_id)
	{
		std::ostringstream oss;
		oss << std::setw(digit) << std::setfill('0') << f_id;
		filename = seq_path + oss.str() + "." + ext;
		frame = cv::imread(filename);
		if (frame.empty())
		{
			std::cerr << "***Could not load a image: " + seq_path + "/" + oss.str() + "." + ext + "***\n";
			std::cerr << "Current parameter's value: \n";
			parser.printMessage();
			return -1;
		}
		if (!initialized)
		{
			//initializes the tracker
			std::cout << "Tracker initialization...";
			timer.start();
			if (!tracker->init(frame, bbox))
			{
				std::cerr << "***Could not initialize tracker...***\n";
				return -1;
			}
			else
			{
				timer.stop();
				std::cout << "Success." << std::endl;
				fcnt++;
				ofs_bbox << bbox.x << "," << bbox.y << "," << bbox.width << "," << bbox.height << std::endl;;
				initialized = true;
			}
		}
		else if (finished)
		{
			ofs_bbox << "0,0,0,0" << std::endl;
		}
		else 
		{
			timer.resume();
			//updates the tracker
			tracker->update(frame, bbox);
			timer.stop();
			if ((bbox.x < 0) || (bbox.y < 0)
				|| (frame.cols < bbox.x+bbox.width) || (frame.rows < bbox.y+bbox.height) )
			{
				// the target is out of sight
				finished = true;
				ofs_bbox << "0,0,0,0" << std::endl;
				continue;
			}
			std::cout << "Process: " << filename << std::endl;
			fcnt++;
			ofs_bbox << bbox.x << "," << bbox.y << "," << bbox.width << "," << bbox.height << std::endl;
			if (save || show)
			{
				cv::rectangle(frame, bbox, cv::Scalar(255, 0, 0), 2, 1);
			}
			if (show)
			{
				cv::imshow("Tracking API: " + tracker_algorithm, frame);
				char c = (char)cv::waitKey(2);
				if (c == 'q')
					break;
			}
			if (save)
			{
				cv::imwrite(output_image_path + oss.str() + "." + ext, frame);
			}
		}
	}

	std::cout << "Fin." << std::endl;
	double fps = static_cast<double>(fcnt) / (timer.elapsed().wall / 1000000000.);
	ofs_fps << fps << std::endl;
	std::cout << "total frames: " << fcnt << std::endl;
	std::cout << "FPS: " << fps << std::endl;

	ofs_bbox.close();
	ofs_fps.close();

	return 0;
}