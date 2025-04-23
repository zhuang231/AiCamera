#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "Testing video0 specifically...\n";

    // Force kill any processes using video0
    system("sudo killall -9 $(ps aux | grep video0 | awk '{print $2}') 2>/dev/null");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    cv::VideoCapture cap;

    // Force V4L2 backend
    cap.set(cv::CAP_PROP_BACKEND, cv::CAP_V4L2);

    std::cout << "Attempting to open device...\n";
    if (!cap.open(0)) {
        std::cerr << "Failed to open camera\n";
        return 1;
    }

    // Try different resolutions from lowest to highest
    std::vector<std::pair<int, int>> resolutions = {
        {640, 480},
        {1280, 720},
        {1920, 1080}
    };

    for (const auto& res : resolutions) {
        std::cout << "\nTrying resolution " << res.first << "x" << res.second << "\n";

        cap.set(cv::CAP_PROP_FRAME_WIDTH, res.first);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, res.second);

        // Get actual resolution
        double actualWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
        double actualHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        std::cout << "Actual resolution: " << actualWidth << "x" << actualHeight << "\n";

        // Try to read a frame
        std::cout << "Attempting to read frame...\n";
        cv::Mat frame;

        auto start = std::chrono::steady_clock::now();
        bool frame_read = false;

        for (int attempts = 0; attempts < 5; attempts++) {
            if (cap.read(frame)) {
                frame_read = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        if (!frame_read || frame.empty()) {
            std::cout << "Could not read frame at this resolution\n";
            continue;
        }

        std::cout << "Successfully read frame!\n";
        std::cout << "Frame size: " << frame.size() << "\n";
        std::cout << "Frame type: " << frame.type() << "\n";

        // Save the frame
        std::string filename = "test_camera_" + std::to_string(res.first) + "x" + std::to_string(res.second) + ".jpg";
        cv::imwrite(filename, frame);
        std::cout << "Saved test image to " << filename << "\n";

        // If we got a frame successfully, we can stop testing resolutions
        break;
    }

    cap.release();
    std::cout << "Camera released\n";
    return 0;
}