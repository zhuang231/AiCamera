#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

void print_v4l2_capabilities(const std::string& device) {
    int fd = open(device.c_str(), O_RDWR);
    if (fd < 0) {
        std::cerr << "Failed to open " << device << " for capability check\n";
        return;
    }

    v4l2_capability cap;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
        std::cerr << "Failed to query capabilities\n";
        close(fd);
        return;
    }

    std::cout << "Driver: " << (char*)cap.driver << std::endl;
    std::cout << "Card: " << (char*)cap.card << std::endl;
    std::cout << "Bus info: " << (char*)cap.bus_info << std::endl;
    std::cout << "Capabilities: " << std::hex << cap.capabilities << std::dec << std::endl;

    close(fd);
}

int main() {
    std::cout << "Testing video0 specifically...\n";

    // Print camera capabilities
    print_v4l2_capabilities("/dev/video0");

    // Force kill any processes using video0
    system("sudo fuser -k /dev/video0 2>/dev/null");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Create capture with V4L2 backend
    cv::VideoCapture cap(0, cv::CAP_V4L2);

    if (!cap.isOpened()) {
        std::cerr << "Failed to open camera with V4L2 backend\n";
        return 1;
    }

    // Try BGR3 format as it's confirmed supported
    int format = cv::VideoWriter::fourcc('B','G','R','3');
    if (!cap.set(cv::CAP_PROP_FOURCC, format)) {
        std::cerr << "Failed to set BGR3 format\n";
    }

    // Set properties
    if (!cap.set(cv::CAP_PROP_FRAME_WIDTH, 640)) {
        std::cerr << "Failed to set width\n";
    }
    if (!cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480)) {
        std::cerr << "Failed to set height\n";
    }
    if (!cap.set(cv::CAP_PROP_BUFFERSIZE, 1)) {
        std::cerr << "Failed to set buffer size\n";
    }

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

    // Try to read frames with timeout
    std::cout << "Attempting to read frames...\n";

    for (int i = 0; i < 5; i++) {
        cv::Mat frame;
        std::cout << "Reading frame " << i+1 << "...\n";

        auto start = std::chrono::steady_clock::now();
        bool frame_read = false;

        while (std::chrono::steady_clock::now() - start < std::chrono::seconds(5)) {
            if (cap.read(frame)) {
                frame_read = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (!frame_read) {
            std::cout << "Failed to read frame within timeout\n";
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