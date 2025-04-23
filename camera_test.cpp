#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "Testing video0 specifically...\n";

    // First, try to access /dev/video0 directly
    std::cout << "Checking video0 device info:\n";
    system("v4l2-ctl --device=/dev/video0 --all");
    std::cout << "\nChecking what processes might be using video0:\n";
    system("sudo fuser -v /dev/video0 2>&1");
    std::cout << "\n";

    // Try to open with different backends
    std::vector<int> backends = {
        cv::CAP_V4L2,
        cv::CAP_GSTREAMER,
        cv::CAP_ANY
    };

    for (auto backend : backends) {
        std::cout << "\nTrying backend " << backend << "...\n";

        cv::VideoCapture cap;
        cap.set(cv::CAP_PROP_BACKEND, backend);

        std::cout << "Attempting to open device...\n";
        if (!cap.open(0)) {
            std::cout << "Failed to open with this backend\n";
            continue;
        }

        std::cout << "Successfully opened camera!\n";
        std::cout << "Frame width: " << cap.get(cv::CAP_PROP_FRAME_WIDTH) << "\n";
        std::cout << "Frame height: " << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << "\n";

        // Try to read a frame
        cv::Mat frame;
        std::cout << "Attempting to read frame...\n";
        cap >> frame;

        if (frame.empty()) {
            std::cout << "Failed to read frame\n";
            cap.release();
            continue;
        }

        std::cout << "Successfully read frame!\n";
        std::cout << "Frame size: " << frame.size() << "\n";

        // Save the frame
        std::string filename = "test_camera_backend_" + std::to_string(backend) + ".jpg";
        cv::imwrite(filename, frame);
        std::cout << "Saved test image to " << filename << "\n";

        cap.release();
        std::cout << "Camera released\n";

        // If we got here, we succeeded
        return 0;
    }

    std::cout << "\nFailed to open camera with any backend\n";
    return 1;
}