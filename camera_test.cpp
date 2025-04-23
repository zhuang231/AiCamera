#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // List all available cameras
    std::cout << "Checking available cameras...\n";
    for (int i = 0; i < 32; i++) {
        cv::VideoCapture cap(i);
        if (cap.isOpened()) {
            std::cout << "Camera " << i << " is available\n";

            // Try to read a frame
            cv::Mat frame;
            cap >> frame;
            if (!frame.empty()) {
                std::cout << "Successfully read frame from camera " << i << "\n";
                std::cout << "Frame size: " << frame.size() << "\n";

                // Save the frame to verify it's working
                std::string filename = "test_camera_" + std::to_string(i) + ".jpg";
                cv::imwrite(filename, frame);
                std::cout << "Saved test image to " << filename << "\n";
            } else {
                std::cout << "Could not read frame from camera " << i << "\n";
            }
            cap.release();
        }
    }
    return 0;
}