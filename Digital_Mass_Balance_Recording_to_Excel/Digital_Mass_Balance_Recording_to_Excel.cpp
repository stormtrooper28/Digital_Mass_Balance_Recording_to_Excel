// Digital_Mass_Balance_Recording_to_Excel.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include "opencv2/opencv.hpp"
#include <Windows.h>
#include <string>

using namespace cv;
using namespace std;

// Input (cin) handlers:
void input(int& i) {
	while (!(cin >> i)) {
		cout << "Please enter a number! (No decimal places allowed)" << endl;
		cin.clear();
	}
}
void input(double& d) {
	while (!(cin >> d)) {
		cout << "Please enter a number! (Decimal places allowed)" << endl;
		cin.clear();
	}
}
void input(char& c) {
	while (!(cin >> c)) {
		cout << "Please enter a single character! (A single: letter / number / symbol / etc...)" << endl;
		cin.clear();
	}
}
// Affirmative char-based input (yes or no)
// returns true for yes and false for no
bool affInput() {
	char c;
	while ( !(cin >> c) || ((c != 'y') && (c != 'n')) ) {
		cout << "Please enter 'y' for yes or 'n' for no!" << endl;
		cin.clear();
	}
	return (c == 'y') ? true : false;
}
void input(bool& b) {
	while (!(cin >> b)) {
		cout << "Please enter a single character! (A single: letter / number / symbol / etc...)" << endl;
		cin.clear();
	}
}

int pause() {
	cin.ignore(LLONG_MAX, '\n');
	cin.get();
	return 0;
}

// Data to be saved in csv
struct FrameData {
	long frameNum; // The ID of the frame, based upon when it was loaded (relative to other frames)
	double time; // The time (a multiple of fpsIncre) that this frame was recorded
	double value; // The value recorded from the mass-balance in this frame
};

struct ImageGroup {
	vector<Mat> images;
	double fpsIncre; // The increment by which time changes between frames
};

/*struct FrameDataGroup {
	vector<FrameData> listOfFrameData;
	double fpsIncre; // The increment by which time changes between frames
};*/

// Setups and opens the trial_video folder returns path to said folder
string loadFolder();
// Returns list of names of all files in specified folder
vector<string> GetFileNamesInDirectory(string directory);
// Returns a vector of the videos present in the specified path
vector<VideoCapture> loadVideos(string, vector<string>);
// Splits the inputted videos into ImageGroups
vector<ImageGroup> splitVideos(vector<VideoCapture>);
// Splits the inputted video into a ImageGroup
vector<Mat> splitVideo(VideoCapture videos);
// Processes the inputted ImageGroups and exports them as FrameDataGroups
vector<vector<FrameData>> processFrameGroups(vector<ImageGroup>);
// Processes the inputted image into a FrameData
FrameData processFrames(Mat, int);

int capFPS; // fps of video before slow-mo conversion

int main(int argc, char** argv) {


	cout << "Introduction, description, video_tutorial_link" << endl;

	string folderPath = loadFolder();

	//Vector< of names of videos> as collected from folder, each is then used to aquire their respective videos =>
	vector<string> nameOfVideos = GetFileNamesInDirectory(folderPath);

	//Vector< of videos> as collected from folder, each video is converted to images = >
	vector<VideoCapture> videos = loadVideos(folderPath, nameOfVideos);

	//Vector< of groups< of images>> Outside vector is full list of img collections(videos), inside group is list of each of the video's frames
	vector<ImageGroup> videosAsFrames = splitVideos(videos);

	// Vector<of vectors< of FrameData>> Outside vector is full list of proccessed frame data, inside vector is list of each video's images' frame data
	vector<vector<FrameData>> videosAsProcessedImages = processFrameGroups(videosAsFrames);

	cout << "test\n";
	cout << folderPath << endl;
	cout << nameOfVideos.size() << endl;
	for (int c = 0; c < nameOfVideos.size(); c++)
		cout << nameOfVideos[c] << endl;
	for (int c = 0; c < videosAsFrames.size(); c++)
		cout << c << endl;

	for (int c = 0; c < videos.size(); c++)
		videos[c].release();
	
	cout << "\nPress Enter to exit!\n";
	return pause();
}

string loadFolder() {
	WCHAR pathArray[MAX_PATH];
	GetModuleFileNameW(NULL, pathArray, ARRAYSIZE(pathArray));

	string path = "";

	for (unsigned int c = 0; pathArray[c] != NULL; c++)
		path += pathArray[c];

	path = (path.substr(0, path.find_last_of("\\")) + "\\trial_videos\\");

	LPCTSTR videoPath = path.c_str();

	CreateDirectory(videoPath, NULL);

	cout << "Opening 'trial_videos' folder." << endl;
	ShellExecute(NULL, NULL, videoPath, NULL, NULL, SW_SHOWNORMAL);
	cout << "Please place the videos into the newly opened folder. If the files you input are not videos, this program will fail and bad things may happen." << endl;
	cout << "\nPress Enter once you're done.\n";
	pause();

	return path;
}

vector<string> GetFileNamesInDirectory(string dir) {
	cout << "dir: " << dir << endl;
	dir += "*";
	cout << "dir: " << dir << endl;
	vector<string> files;
	WIN32_FIND_DATA fileData;
	HANDLE hFind;
	if (!((hFind = FindFirstFile(dir.c_str(), &fileData)) == INVALID_HANDLE_VALUE)) {
		while (FindNextFile(hFind, &fileData)) {
			files.push_back(fileData.cFileName);
		}
	}
	FindClose(hFind);
	return files;
}

vector<VideoCapture> loadVideos(string dir, vector<string> nameOfVideos) {
	vector<VideoCapture> videos;

	for (int c = 0; c < nameOfVideos.size(); c++) {
		// Open video file
		VideoCapture video(dir + nameOfVideos[c]);
		videos.push_back(video);
	}
	return videos;
}

vector<ImageGroup> splitVideos(vector<VideoCapture> videos) {
	vector<ImageGroup> images;
	for (int c = 0; c < videos.size(); c++) {
		VideoCapture video = videos[c];

		splitVideo(video);
		//

		// double fps = video.get(CV_CAP_PROP_FPS);
		// For OpenCV 3, you can also use the following
		int fps = video.get(CAP_PROP_FPS);
		// Instead of frames (whole number) per second (1), return seconds (double) per frame (1)
		double fpsIncre = 1.0 / fps; // The increment by which time changes between frames

	}
	return images;
}
vector<Mat> splitVideo(VideoCapture video) {
	vector<Mat> images;
	for (;;) {
		Mat img;
		if (!video.read(img))
			break;
		images.push_back(img);
	}
	return images;
}


vector<vector<FrameData>> processFrameGroups(vector<ImageGroup> ImageGroupList) {
	vector<vector<FrameData>> frameGroupList; // List of videos, as split into processed frame data
	for (int c = 0; c < ImageGroupList.size(); c++) {
		ImageGroup images = ImageGroupList[c]; // Images in video 'c'
		vector<FrameData> frameDatas; // Processed list of images in video 'c'
		for (int i = 0; i < images.images.size(); c++) {
			// Add processed form of raw image -> list of processed images
			frameDatas.push_back(processFrames(images.images[c], images.fpsIncre));
		}
		// Add video in processed frame form to frameGroupList
		frameGroupList.push_back(frameDatas);
	}
	return frameGroupList;
}
FrameData processFrames(Mat image, int time) {
	FrameData fd;
	fd.frameNum = 0;
	fd.time = 0.01;
	fd.value = 248.76;
	return fd;
}