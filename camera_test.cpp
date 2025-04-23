#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>

int main() {
    // List all available cameras
    std::cout << "Checking available cameras...\n";

    // First, try to access /dev/video* devices directly
    std::cout << "Available video devices:\n";
    system("ls -l /dev/video*");
    std::cout << "\n";

    for (int i = 0; i < 32; i++) {
        std::cout << "\nTrying camera " << i << "...\n";

        // Set a timeout for camera opening
        auto start = std::chrono::steady_clock::now();
        cv::VideoCapture cap;

        std::cout << "Attempting to open device...\n";
        cap.open(i);

        auto end = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        if (diff.count() > 3000) {  // If it takes more than 3 seconds, skip
            std::cout << "Timeout while trying to open camera " << i << "\n";
            cap.release();
            continue;
        }

        if (cap.isOpened()) {
            std::cout << "Camera " << i << " opened successfully\n";

            // Try to read a frame with timeout
            std::cout << "Attempting to read frame...\n";
            start = std::chrono::steady_clock::now();
            cv::Mat frame;
            cap >> frame;
            end = std::chrono::steady_clock::now();
            diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            if (diff.count() > 3000) {
                std::cout << "Timeout while reading frame from camera " << i << "\n";
                cap.release();
                continue;
            }

            if (!frame.empty()) {
                std::cout << "Successfully read frame from camera " << i << "\n";
                std::cout << "Frame size: " << frame.size() << "\n";

                // Save the frame to verify it's working
                std::string filename = "test_camera_" + std::to_string(i) + ".jpg";
                cv::imwrite(filename, frame);
                std::cout << "Saved test image to " << filename << "\n";
            } else {
                std::cout << "Could not read frame from camera " << i << " (frame empty)\n";
            }
            cap.release();
        } else {
            std::cout << "Could not open camera " << i << "\n";
        }
    }

    std::cout << "\nCamera test complete!\n";
    return 0;
}