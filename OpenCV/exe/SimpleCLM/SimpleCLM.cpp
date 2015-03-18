///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2014, University of Southern California and University of Cambridge,
// all rights reserved.
//
// THIS SOFTWARE IS PROVIDED “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY. OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Notwithstanding the license granted herein, Licensee acknowledges that certain components
// of the Software may be covered by so-called “open source” software licenses (“Open Source
// Components”), which means any software licenses approved as open source licenses by the
// Open Source Initiative or any substantially similar licenses, including without limitation any
// license that, as a condition of distribution of the software licensed under such license,
// requires that the distributor make the software available in source code format. Licensor shall
// provide a list of Open Source Components for a particular version of the Software upon
// Licensee’s request. Licensee will comply with the applicable terms of such licenses and to
// the extent required by the licenses covering Open Source Components, the terms of such
// licenses will apply in lieu of the terms of this Agreement. To the extent the terms of the
// licenses applicable to Open Source Components prohibit any of the restrictions in this
// License Agreement with respect to such Open Source Component, such restrictions will not
// apply to such Open Source Component. To the extent the terms of the licenses applicable to
// Open Source Components require Licensor to make an offer to provide source code or
// related information in connection with the Software, such offer is hereby made. Any request
// for source code or related information should be directed to cl-face-tracker-distribution@lists.cam.ac.uk
// Licensee acknowledges receipt of notices for the Open Source Components for the initial
// delivery of the Software.

//     * Any publications arising from the use of this software, including but
//       not limited to academic journal and conference publications, technical
//       reports and manuals, must cite one of the following works:
//
//       Tadas Baltrusaitis, Peter Robinson, and Louis-Philippe Morency. 3D
//       Constrained Local Model for Rigid and Non-Rigid Facial Tracking.
//       IEEE Conference on Computer Vision and Pattern Recognition (CVPR), 2012.    
//
//       Tadas Baltrusaitis, Peter Robinson, and Louis-Philippe Morency. 
//       Constrained Local Neural Fields for robust facial landmark detection in the wild.
//       in IEEE Int. Conference on Computer Vision Workshops, 300 Faces in-the-Wild Challenge, 2013.    
//
///////////////////////////////////////////////////////////////////////////////

// SimpleCLM.cpp : Defines the entry point for the console application.

#include <winsock2.h>
#include <Ws2tcpip.h>

#include <CLM.h>
#include <CLMTracker.h>
#include <CLMParameters.h>
#include <CLM_utils.h>

#include <fstream>
#include <sstream>

#include <cv.h>

#include "opencv2/photo/photo.hpp"
#include <opencv2/gpu/gpu.hpp>


#pragma comment(lib, "Ws2_32.lib")

#define INFO_STREAM( stream ) \
std::cout << stream << std::endl

#define WARN_STREAM( stream ) \
std::cout << "Warning: " << stream << std::endl

#define ERROR_STREAM( stream ) \
std::cout << "Error: " << stream << std::endl

#define WIDTH 640
#define HEIGHT 480
// #define WIDTH 1280
// #define HEIGHT 960

// cx and cy aren't necessarilly in the image center, 
//so need to be able to override it (start with unit vals and init them if none specified)
float fx = 500, fy = 500, cx = 0, cy = 0;
double focalLength, fa;

// Physical measuements in cm
double faceMeasure = 5;
double screenCenterY = -10.5; // Y coordinate of screen center in the camera coordinate system
double screenCenterX = 0; // X coordinate of screen center in the camera coordinate system
double screenWidth = 34.5;
double screenHeight = 19.5;

static void printErrorAndAbort( const std::string & error )
{
    std::cout << error << std::endl;
    abort();
}

#define FATAL_STREAM( stream ) \
printErrorAndAbort( std::string( "Fatal error: " ) + stream )

using namespace std;
using namespace cv;

vector<string> get_arguments(int argc, char **argv)
{

	vector<string> arguments;

	for(int i = 1; i < argc; ++i)
	{
		arguments.push_back(string(argv[i]));
	}
	return arguments;
}

void convertCamera2World(Point3f cameraPoint, Point3f& worldPoint)
{
  Point3f screenCameraPoint(screenCenterX, screenCenterY, 0);
  worldPoint = cameraPoint - screenCameraPoint;
}

void convert2OpenglCoordinates(Point2f imagePoint, double z, Point3f& objectPoint)
{
  double x0 = (imagePoint.x - cx) * z / fx;
  double y0 = (imagePoint.y - cy) * z / fy;
  objectPoint.x = x0;
  objectPoint.y = -y0;
  objectPoint.z = -z;
}


void processPoints(const Mat_<double>& shape2D, Point3f& trackedWorldPos, Point3f& trackedWorldMouthPos, Mat& img, double& mouthRatio, 
	double detection_certainty, Rect& boundingRect)
{
	int n = shape2D.rows/2;

	vector<Point2f> facePoints;
	for (int i = 0; i <= 16; ++i)
	{
		facePoints.push_back(Point2f(shape2D.at<double>(i), shape2D.at<double>(i+n)));
	}
	RotatedRect fittedRec = fitEllipse(facePoints);
	boundingRect = fittedRec.boundingRect();

/*	ellipse(img, fittedRec, Scalar((1-detection_certainty)*255.0, 0, detection_certainty*255.0));
	circle(img, fittedRec.center, 3, Scalar(0, 255, 255), -1);
	rectangle(img, boundingRect, Scalar(0, 255, 255));*/
	

	double virtualMesure = fittedRec.size.width;

	double z = fa * faceMeasure / virtualMesure;

	vector<Point2f> eyePointsLine;
	double sum_x = 0;
	double sum_y = 0;
	for (int i = 36; i <= 47; ++i)
	{
		sum_x += shape2D.at<double>(i);
		sum_y += shape2D.at<double>(i+n);

		if (i == 36 || i == 39 || i == 42 || i == 45)
			eyePointsLine.push_back(Point2f(shape2D.at<double>(i), shape2D.at<double>(i+n)));

	}
	sum_x += shape2D.at<double>(0) + shape2D.at<double>(16);
	sum_y += shape2D.at<double>(0+n) + shape2D.at<double>(16+n);

	//Point2f imagePt(shape2D.at<double>(33), shape2D.at<double>(33 + n));
	Point2f imagePt(sum_x / 14, sum_y / 14);
	Point2f eyePt = imagePt;
	imagePt.x = fittedRec.center.x;


	convert2OpenglCoordinates(imagePt, z, trackedWorldPos);

	Vec4f fittedLine;
	fitLine(eyePointsLine, fittedLine, CV_DIST_L1, 0, 0.01, 0.01);
	line(img, eyePointsLine[0], eyePointsLine[3], Scalar(255, 0, 0));
	
	Vec2f orthoVec(-fittedLine[1], fittedLine[0]);
	double dis = norm(Point2f(shape2D.at<double>(27), shape2D.at<double>(27+n)) - 
					  Point2f(shape2D.at<double>(30), shape2D.at<double>(30+n)));
	
	
	Point2f mouthPt(eyePt.x + orthoVec[0] * dis * 2, eyePt.y + orthoVec[1] * dis * 2);
	line(img, eyePt, mouthPt, Scalar(0, 0, 255));
	circle(img, mouthPt, 3, Scalar(0, 255, 255), -1);

	/*sum_x = 0;
	sum_y = 0;
	for (int i = 48; i <= 67; ++i)
	{
		sum_x += shape2D.at<double>(i);
		sum_y += shape2D.at<double>(i+n);
	}
	Point2f mouthPt(sum_x / 20, sum_y / 20);*/

	convert2OpenglCoordinates(mouthPt, z, trackedWorldMouthPos);

	vector<Point2f> innerMouthPoints;
	for (int i = 60; i <= 67; ++i)
	{
		innerMouthPoints.push_back(Point2f(shape2D.at<double>(i), shape2D.at<double>(i+n)));
	}
	RotatedRect mouthRR = fitEllipse(innerMouthPoints);

	//ellipse(img, mouthRR, Scalar(255, 100, 0));

	double rrWidth = (mouthRR.size.width < 1) ? 1 : mouthRR.size.width;
	double rrHeight = mouthRR.size.height;

	mouthRatio = rrHeight / rrWidth;


}

int main (int argc, char **argv)
{

	vector<string> arguments = get_arguments(argc, argv);

	// Some initial parameters that can be overriden from command line	
	vector<string> files, depth_directories, pose_output_files, tracked_videos_output, landmark_output_files, landmark_3D_output_files;
	
	// By default try webcam 0
	int device = 0;


			
	CLMTracker::CLMParameters clm_parameters(arguments);
			
	// Get the input output file parameters
	
	// Indicates that rotation should be with respect to camera plane or with respect to camera
	bool use_camera_plane_pose;
	CLMTracker::get_video_input_output_params(files, depth_directories, pose_output_files, tracked_videos_output, landmark_output_files, landmark_3D_output_files, use_camera_plane_pose, arguments);
	// Get camera parameters
	CLMTracker::get_camera_params(device, fx, fy, cx, cy, arguments);    
	
	// The modules that are being used for tracking
	CLMTracker::CLM clm_model(clm_parameters.model_location);	
	
	// If multiple video files are tracked, use this to indicate if we are done
	bool done = false;	
	int f_n = -1;

	// If cx (optical axis centre) is undefined will use the image size/2 as an estimate
	bool cx_undefined = false;
	if(cx == 0 || cy == 0)
	{
		cx_undefined = true;
	}		


	// Socket Initialization --------------------------------

    int iResult;
    WSADATA wsaData;

    SOCKET SendSocket = INVALID_SOCKET;
    sockaddr_in RecvAddr;

    //unsigned short Port = 27015;
	unsigned short Port = 5005;

    char SendBuf[1024];
    int BufLen = 1024;

    //----------------------
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        wprintf(L"WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    //---------------------------------------------
    // Create a socket for sending data
    SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (SendSocket == INVALID_SOCKET) {
        wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    //---------------------------------------------
    // Set up the RecvAddr structure with the IP address of
    // the receiver (in this example case "192.168.1.1")
    // and the specified port number.
    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(Port);
    RecvAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//---------------------------------------------------------------------

    // Read my parameter file
    // Intrinsic camera parameters


    string parameterFile = "";
    for(size_t i = 0; i < arguments.size(); ++i)
	{
		if (arguments[i].compare("-p") == 0) 
		{                    
			parameterFile = string(arguments[i+1]);					
			i++;
		}		
	}

	if (parameterFile == "")
	{
		cout << "You must provide the path to your parameter file, use -p option" << endl;
		return 1;
	}

	ifstream parameter_file(parameterFile);
	if (!parameter_file.is_open()) {
		cout << "Unable to open parameter file" << endl;
		return 1;
	}

	parameter_file >> faceMeasure;
	cout << "Face measure = " << faceMeasure << " cm" << endl;
	parameter_file >> screenCenterX;
	parameter_file >> screenCenterY;
	cout << "Screen center coordinates in camera coordinate system = " << screenCenterX
	<< ", " << screenCenterY << endl;
	parameter_file >> screenWidth;
	parameter_file >> screenHeight;
	cout << "Screen size: " << screenWidth << " x " << screenHeight << endl;

	parameter_file >> cx;
	cout << "cx = " << cx << endl;

	parameter_file >> cy;
	cout << "cy = " << cy << endl;

	parameter_file >> fx;
	cout << "fx = " << fx << endl;

	parameter_file >> fy;
	cout << "fy = " << fy << endl;

	parameter_file.close();
	fa = (fx + fy) / 2.0;
	//----------------------------------------------------------------------


	while(!done) // this is not a for loop as we might also be reading from a webcam
	{
		
		string current_file;

		// We might specify multiple video files as arguments
		if(files.size() > 0)
		{
			f_n++;			
		    current_file = files[f_n];
		}
		else
		{
			// If we want to write out from webcam
			f_n = 0;
		}

		bool use_depth = !depth_directories.empty();	

		// Do some grabbing
		VideoCapture video_capture;
		if( current_file.size() > 0 )
		{
			INFO_STREAM( "Attempting to read from file: " << current_file );
			video_capture = VideoCapture( current_file );
		}
		else
		{
			INFO_STREAM( "Attempting to capture from device: " << device );
			video_capture = VideoCapture( device );

			// Read a first frame often empty in camera
			Mat captured_image;
			video_capture >> captured_image;
		}

		if( !video_capture.isOpened() ) FATAL_STREAM( "Failed to open video source" );
		else INFO_STREAM( "Device or file opened");

		Mat captured_image;
		video_capture >> captured_image;		
		
		video_capture.set(CV_CAP_PROP_FRAME_WIDTH,WIDTH);
		video_capture.set(CV_CAP_PROP_FRAME_HEIGHT,HEIGHT);
		//resize(captured_image, captured_image, Size(WIDTH, HEIGHT));

		// If optical centers are not defined just use center of image
		if(cx_undefined)
		{
			cx = captured_image.cols / 2.0f;
			cy = captured_image.rows / 2.0f;
		}
	
		// Creating output files
		std::ofstream pose_output_file;
		if(!pose_output_files.empty())
		{
			pose_output_file.open (pose_output_files[f_n], ios_base::out);
		}
	
		std::ofstream landmarks_output_file;		
		if(!landmark_output_files.empty())
		{
			landmarks_output_file.open(landmark_output_files[f_n], ios_base::out);
		}

		std::ofstream landmarks_3D_output_file;
		if(!landmark_3D_output_files.empty())
		{
			landmarks_3D_output_file.open(landmark_3D_output_files[f_n], ios_base::out);
		}
	
		int frame_count = 0;
		
		// saving the videos
		VideoWriter writerFace;
		if(!tracked_videos_output.empty())
		{
			writerFace = VideoWriter(tracked_videos_output[f_n], CV_FOURCC('D','I','V','X'), 30, captured_image.size(), true);		
		}
		
		// For measuring the timings
		int64 t1,t0 = cv::getTickCount();
		double fps = 10;

		INFO_STREAM( "Starting tracking");
		while(!captured_image.empty())
		{		

			flip(captured_image, captured_image, 1);
			//fastNlMeansDenoisingColored(captured_image, captured_image, 3, 3, 7, 3);
			// Mat captured_image_original = captured_image.clone();
			// gpu::GpuMat captured_image_original_gpu(captured_image_original);
			// gpu::GpuMat dst_gpu;
			// gpu::bilateralFilter(captured_image_original_gpu, dst_gpu, -1, 15, 5);
			// captured_image = Mat(dst_gpu);
			
			// Reading the images
			Mat_<float> depth_image;
			Mat_<uchar> grayscale_image;

			if(captured_image.channels() == 3)
			{
				cvtColor(captured_image, grayscale_image, CV_BGR2GRAY);				
			}
			else
			{
				grayscale_image = captured_image.clone();				
			}
		
			// Get depth image
			if(use_depth)
			{
				char* dst = new char[100];
				std::stringstream sstream;

				sstream << depth_directories[f_n] << "\\depth%05d.png";
				sprintf(dst, sstream.str().c_str(), frame_count + 1);
				// Reading in 16-bit png image representing depth
				Mat_<short> depth_image_16_bit = imread(string(dst), -1);

				// Convert to a floating point depth image
				if(!depth_image_16_bit.empty())
				{
					depth_image_16_bit.convertTo(depth_image, CV_32F);
				}
				else
				{
					WARN_STREAM( "Can't find depth image" );
				}
			}
			
			// The actual facial landmark detection / tracking
			bool detection_success = CLMTracker::DetectLandmarksInVideo(grayscale_image, depth_image, clm_model, clm_parameters);

			// Work out the pose of the head from the tracked model
			Vec6d pose_estimate_CLM;
			if(use_camera_plane_pose)
			{
				pose_estimate_CLM = CLMTracker::GetCorrectedPoseCameraPlane(clm_model, fx, fy, cx, cy, clm_parameters);
			}
			else
			{
				pose_estimate_CLM = CLMTracker::GetCorrectedPoseCamera(clm_model, fx, fy, cx, cy, clm_parameters);
			}

			// Visualising the results
			// Drawing the facial landmarks on the face and the bounding box around it if tracking is successful and initialised
			double detection_certainty = clm_model.detection_certainty;

			double visualisation_boundary = 0.2;
			
			// Only draw if the reliability is reasonable, the value is slightly ad-hoc
			if(detection_certainty < visualisation_boundary)
			{
				CLMTracker::Draw(captured_image, clm_model);

				if(detection_certainty > 1)
					detection_certainty = 1;
				if(detection_certainty < -1)
					detection_certainty = -1;

				detection_certainty = (detection_certainty + 1)/(visualisation_boundary +1);

				// A rough heuristic for box around the face width
				int thickness = (int)std::ceil(2.0* ((double)captured_image.cols) / 640.0);
				
				Vec6d pose_estimate_to_draw = CLMTracker::GetCorrectedPoseCameraPlane(clm_model, fx, fy, cx, cy, clm_parameters);

				// Draw it in reddish if uncertain, blueish if certain
				//CLMTracker::DrawBox(captured_image, pose_estimate_to_draw, Scalar((1-detection_certainty)*255.0,0, detection_certainty*255), thickness, fx, fy, cx, cy);

				// for (int i = 3; i < 6; ++i)
				// {
				// 	cout << (int)(pose_estimate_to_draw[i] * 180.0 / 3.1415926) << " ";
				// }
				// cout << endl;

				Point3f trackedWorldPosCamera, trackedWorldPosScreen;
				Point3f trackedWorldMouthCamera, trackedWorldMouthScreen;
				double mouthRatio = 10;
				Rect boundingRect;
				processPoints(clm_model.detected_landmarks, trackedWorldPosCamera, trackedWorldMouthCamera, captured_image, mouthRatio, 
					detection_certainty, boundingRect);
				convertCamera2World(trackedWorldPosCamera, trackedWorldPosScreen);
				convertCamera2World(trackedWorldMouthCamera, trackedWorldMouthScreen);

				// cout << trackedWorldPosScreen.x << ", " << trackedWorldPosScreen.y << ", "
				// << trackedWorldPosScreen.z << endl;


				// Visualize the projected eye point
				double ratio_x = captured_image.cols / screenWidth;
				double ratio_y = captured_image.rows / screenHeight;
				Point imagePos, mouthPos;
				imagePos.x = trackedWorldPosScreen.x * ratio_x + captured_image.cols / 2.0;
				imagePos.y = -trackedWorldPosScreen.y * ratio_y + captured_image.rows/ 2.0;
				mouthPos.x = trackedWorldMouthScreen.x * ratio_x + captured_image.cols/ 2.0;
				mouthPos.y = -trackedWorldMouthScreen.y * ratio_y + captured_image.rows / 2.0;

				//circle(captured_image, imagePos, 3, Scalar(0, 0, 255), -1);
				//circle(captured_image, mouthPos, 3, Scalar(255, 0, 0), -1);

				// Send data via UDP to Unity
				string sentString = to_string(trackedWorldPosScreen.x) + " " + to_string(trackedWorldPosScreen.y) + " "
				+ to_string(trackedWorldPosScreen.z);
				for (int i = 3; i < 6; ++i)
				{
					//double angle = pose_estimate_to_draw[i] * 180.0 / 3.1415926;
					double angleRadian = pose_estimate_to_draw[i];
					sentString += " " + to_string(angleRadian);
				}
				sentString += " " + to_string(trackedWorldMouthScreen.x) + " " + to_string(trackedWorldMouthScreen.y) + " "
				+ to_string(trackedWorldMouthScreen.z);

				sentString += " " + to_string(mouthRatio);

				double boundingRectXRatio = (double)boundingRect.x / captured_image.cols;
				double boundingRectYRatio = (double)boundingRect.y / captured_image.rows;
				double boundingRectWidthRatio = (double)boundingRect.width / captured_image.cols;
				double boundingRectHeightRatio = (double)boundingRect.height / captured_image.rows;
				sentString += " " + to_string(boundingRectXRatio)
							+ " " + to_string(boundingRectYRatio)
							+ " " + to_string(boundingRectWidthRatio)
							+ " " + to_string(boundingRectHeightRatio);

				char const *sentText = sentString.c_str();
				strcpy(SendBuf, sentText);
				iResult = sendto(SendSocket,
						 SendBuf, BufLen, 0, (SOCKADDR *) & RecvAddr, sizeof (RecvAddr));
				if (iResult == SOCKET_ERROR) {
					wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
					closesocket(SendSocket);
					WSACleanup();
					return 1;
				}
			}

			// Work out the framerate
			if(frame_count % 10 == 0)
			{      
				t1 = cv::getTickCount();
				fps = 10.0 / (double(t1-t0)/cv::getTickFrequency()); 
				t0 = t1;
			}
			
			// Write out the framerate on the image before displaying it
			char fpsC[255];
			sprintf(fpsC, "%d", (int)fps);
			string fpsSt("FPS:");
			fpsSt += fpsC;
			cv::putText(captured_image, fpsSt, cv::Point(10,20), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(255,0,0));		
			
			if(!clm_parameters.quiet_mode)
			{
				namedWindow("tracking_result",WINDOW_NORMAL);		
				imshow("tracking_result", captured_image);

				if(!depth_image.empty())
				{
					// Division needed for visualisation purposes
					imshow("depth", depth_image/2000.0);
				}
			}

			// Output the detected facial landmarks
			if(!landmark_output_files.empty())
			{
				landmarks_output_file << frame_count + 1 << " " << detection_success;
				for (int i = 0; i < clm_model.pdm.NumberOfPoints() * 2; ++ i)
				{
					landmarks_output_file << " " << clm_model.detected_landmarks.at<double>(i) << " ";
				}
				landmarks_output_file << endl;
			}

			// Output the detected facial landmarks
			if(!landmark_3D_output_files.empty())
			{
				landmarks_3D_output_file << frame_count + 1 << " " << detection_success;
				Mat_<double> shape_3D = clm_model.GetShape(fx, fy, cx, cy);
				for (int i = 0; i < clm_model.pdm.NumberOfPoints() * 3; ++i)
				{
					landmarks_3D_output_file << " " << shape_3D.at<double>(i);
				}
				landmarks_3D_output_file << endl;
			}

			// Output the estimated head pose
			if(!pose_output_files.empty())
			{
				pose_output_file << frame_count + 1 << " " << (float)frame_count * 1000/30 << " " << detection_success << " " << pose_estimate_CLM[0] << " " << pose_estimate_CLM[1] << " " << pose_estimate_CLM[2] << " " << pose_estimate_CLM[3] << " " << pose_estimate_CLM[4] << " " << pose_estimate_CLM[5] << endl;
			}				

			// output the tracked video
			if(!tracked_videos_output.empty())
			{		
				writerFace << captured_image;
			}

			video_capture >> captured_image;
			//resize(captured_image, captured_image, Size(WIDTH, HEIGHT));

			// detect key presses
			char character_press = cv::waitKey(1);
			
			// restart the tracker
			if(character_press == 'r')
			{
				clm_model.Reset();
			}
			// quit the application
			else if(character_press=='q')
			{
				return(0);
			}

			// Update the frame count
			frame_count++;

		}
		
		frame_count = 0;

		// Reset the model, for the next video
		clm_model.Reset();

		pose_output_file.close();
		landmarks_output_file.close();

		// break out of the loop if done with all the files (or using a webcam)
		if(f_n == files.size() -1 || files.empty())
		{
			done = true;
		}
	}

	  //---------------------------------------------
    // When the application is finished sending, close the socket.
    wprintf(L"Finished sending. Closing socket.\n");
    iResult = closesocket(SendSocket);
    if (iResult == SOCKET_ERROR) {
        wprintf(L"closesocket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    //---------------------------------------------
    // Clean up and quit.
    wprintf(L"Exiting.\n");
    WSACleanup();
	return 0;
}

