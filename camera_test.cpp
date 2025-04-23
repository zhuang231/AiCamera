#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "Testing video0 specifically...\n";

    // Force kill any processes using video0
    system("sudo fuser -k /dev/video0 2>/dev/null");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Create capture with V4L2 backend
    cv::VideoCapture cap(0, cv::CAP_V4L2);

    if (!cap.isOpened()) {
        std::cerr << "Failed to open camera with V4L2 backend\n";
        return 1;
    }

    // Try YUYV format first as it's commonly supported
    int format = cv::VideoWriter::fourcc('Y','U','Y','V');
    cap.set(cv::CAP_PROP_FOURCC, format);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    cap.set(cv::CAP_PROP_BUFFERSIZE, 1);

    // Get actual format and resolution
    double width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(cv::CAP_PROP_FPS);
    int fourcc = cap.get(cv::CAP_PROP_FOURCC);

    std::cout << "Camera opened with:\n"
              << "Resolution: " << width << "x" << height << "\n"
              << "FPS: " << fps << "\n"
              << "Format: " << char(fourcc & 0xFF) << char((fourcc >> 8) & 0xFF)
              << char((fourcc >> 16) & 0xFF) << char((fourcc >> 24) & 0xFF) << "\n";

    // Try to read frames
    std::cout << "Attempting to read frames...\n";

    for (int i = 0; i < 5; i++) {
        cv::Mat frame;
        std::cout << "Reading frame " << i+1 << "...\n";

        if (!cap.read(frame)) {
            std::cout << "Failed to read frame\n";
            continue;
        }

        if (frame.empty()) {
            std::cout << "Empty frame received\n";
            continue;
        }

        std::cout << "Frame " << i+1 << " received successfully\n";
        std::cout << "Frame size: " << frame.size() << "\n";
        std::cout << "Frame type: " << frame.type() << "\n";

        // Save the frame
        std::string filename = "test_frame_" + std::to_string(i+1) + ".jpg";
        cv::imwrite(filename, frame);
        std::cout << "Saved frame to " << filename << "\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    cap.release();
    std::cout << "Camera released\n";
    return 0;
}