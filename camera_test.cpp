#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "Testing video0 specifically...\n";

    // Kill any existing processes using the camera
    system("sudo kill $(sudo fuser /dev/video0 2>/dev/null) 2>/dev/null");
    std::this_thread::sleep_for(std::chrono::seconds(2));  // Wait for cleanup

    cv::VideoCapture cap;

    // Set properties before opening
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('B', 'G', 'R', '3'));
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);  // Start with a lower resolution
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

    std::cout << "Attempting to open device with V4L2...\n";
    cap.set(cv::CAP_PROP_BACKEND, cv::CAP_V4L2);

    if (!cap.open(0)) {
        std::cerr << "Failed to open camera with V4L2\n";
        return 1;
    }

    std::cout << "Successfully opened camera!\n";
    std::cout << "Actual width: " << cap.get(cv::CAP_PROP_FRAME_WIDTH) << "\n";
    std::cout << "Actual height: " << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << "\n";

    // Try to read a frame with timeout
    std::cout << "Attempting to read frame...\n";
    cv::Mat frame;

    auto start = std::chrono::steady_clock::now();
    bool frame_read = false;

    while (std::chrono::steady_clock::now() - start < std::chrono::seconds(5)) {
        if (cap.read(frame)) {
            frame_read = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (!frame_read || frame.empty()) {
        std::cerr << "Failed to read frame within 5 seconds\n";
        cap.release();
        return 1;
    }

    std::cout << "Successfully read frame!\n";
    std::cout << "Frame size: " << frame.size() << "\n";
    std::cout << "Frame type: " << frame.type() << "\n";

    // Save the frame
    std::string filename = "test_camera.jpg";
    cv::imwrite(filename, frame);
    std::cout << "Saved test image to " << filename << "\n";

    cap.release();
    std::cout << "Camera released\n";
    return 0;
}