#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "Testing video0 specifically...\n";

    // Force kill any processes using video0
    system("sudo fuser -k /dev/video0 2>/dev/null");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    cv::VideoCapture cap;

    // Force V4L2 backend
    cap.set(cv::CAP_PROP_BACKEND, cv::CAP_V4L2);

    // Try different pixel formats
    std::vector<int> formats = {
        cv::VideoWriter::fourcc('Y','U','Y','V'),  // YUYV
        cv::VideoWriter::fourcc('M','J','P','G'),  // MJPEG
        cv::VideoWriter::fourcc('B','G','R','3'),  // BGR3
        cv::VideoWriter::fourcc('G','R','E','Y')   // GREY
    };

    for (auto format : formats) {
        std::cout << "\nTrying format: " << char(format & 0xFF) << char((format >> 8) & 0xFF)
                  << char((format >> 16) & 0xFF) << char((format >> 24) & 0xFF) << "\n";

        cap.release();  // Make sure we release before trying new format
        cap.set(cv::CAP_PROP_FOURCC, format);
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);   // Start with low resolution
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
        cap.set(cv::CAP_PROP_BUFFERSIZE, 1);      // Minimize buffer size

        std::cout << "Attempting to open device...\n";
        if (!cap.open(0)) {
            std::cout << "Failed to open with this format\n";
            continue;
        }

        // Get actual format and resolution
        double width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
        double height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        double fps = cap.get(cv::CAP_PROP_FPS);
        std::cout << "Opened with resolution: " << width << "x" << height << " @ " << fps << "fps\n";

        // Try to read a frame
        std::cout << "Attempting to read frame...\n";
        cv::Mat frame;
        bool success = false;

        for (int attempt = 0; attempt < 3; attempt++) {
            cap >> frame;
            if (!frame.empty()) {
                success = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        if (success) {
            std::cout << "Successfully read frame!\n";
            std::cout << "Frame size: " << frame.size() << "\n";
            std::cout << "Frame type: " << frame.type() << "\n";

            // Save the frame
            std::string filename = "test_camera_format_" +
                std::string(1, char(format & 0xFF)) +
                std::string(1, char((format >> 8) & 0xFF)) +
                std::string(1, char((format >> 16) & 0xFF)) +
                std::string(1, char((format >> 24) & 0xFF)) + ".jpg";

            cv::imwrite(filename, frame);
            std::cout << "Saved test image to " << filename << "\n";
            break;  // We found a working format, stop trying others
        } else {
            std::cout << "Could not read frame with this format\n";
        }
    }

    cap.release();
    std::cout << "Camera released\n";
    return 0;
}