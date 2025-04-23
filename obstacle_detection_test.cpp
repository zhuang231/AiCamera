#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <errno.h>
#include <string.h>
#include <signal.h>

// Define constants
#define THRESHOLD_DISTANCE 100  // Example threshold in centimeters
#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480
#define PORT 8081

// Structure to hold obstacle detection results
struct ObstacleDetection {
    bool obstacle_detected;
    int distance;
    int x_position;
    int y_position;
};

class CameraStream {
private:
    cv::VideoCapture cap;
    std::mutex frame_mutex;
    cv::Mat latest_frame;
    bool running;

    std::string create_http_response(const std::vector<uchar>& jpeg_data) {
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
        response += "--frame\r\n";
        response += "Content-Type: image/jpeg\r\n";
        response += "Content-Length: " + std::to_string(jpeg_data.size()) + "\r\n\r\n";
        response.append(jpeg_data.begin(), jpeg_data.end());
        response += "\r\n";
        return response;
    }

public:
    CameraStream() : running(false) {}

    bool initialize() {
        std::cout << "Initializing camera...\n";

        // Try different video devices
        const int MAX_DEVICES = 32;
        for (int device = 0; device < MAX_DEVICES; device++) {
            std::cout << "Trying video device " << device << "...\n";
            cap.open(device);
            if (cap.isOpened()) {
                std::cout << "Successfully opened video device " << device << "\n";

                // Try to read a test frame
                cv::Mat test_frame;
                cap >> test_frame;
                if (!test_frame.empty()) {
                    std::cout << "Successfully read a frame from device " << device << "\n";

                    // Set camera properties
                    std::cout << "Setting resolution to " << FRAME_WIDTH << "x" << FRAME_HEIGHT << "\n";
                    cap.set(cv::CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
                    cap.set(cv::CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

                    // Verify the actual resolution
                    double actualWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
                    double actualHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
                    std::cout << "Actual resolution: " << actualWidth << "x" << actualHeight << "\n";

                    running = true;
                    return true;
                }
                std::cout << "Could not read frame from device " << device << ", trying next device\n";
                cap.release();
            }
        }

        std::cerr << "Error: Could not find a working camera\n";
        return false;
    }

    ObstacleDetection process_frame(cv::Mat& frame) {
        ObstacleDetection detection;
        detection.obstacle_detected = rand() % 2;
        detection.distance = rand() % 200;
        detection.x_position = rand() % FRAME_WIDTH;
        detection.y_position = rand() % FRAME_HEIGHT;

        if (detection.obstacle_detected) {
            cv::circle(frame, cv::Point(detection.x_position, detection.y_position),
                      20, cv::Scalar(0, 0, 255), 2);

            char text[50];
            sprintf(text, "Distance: %d cm", detection.distance);
            cv::putText(frame, text, cv::Point(10, 30),
                        cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);

            if (detection.distance < THRESHOLD_DISTANCE) {
                std::cout << "WARNING: Obstacle too close! Distance: " << detection.distance << " cm\n";
            }
        }
        return detection;
    }

    void stream_frames() {
        std::cout << "Creating socket...\n";
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            std::cerr << "Socket creation error: " << strerror(errno) << std::endl;
            return;
        }
        std::cout << "Socket created successfully\n";

        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "Setsockopt error: " << strerror(errno) << std::endl;
            close(server_fd);
            return;
        }

        struct sockaddr_in address;
        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);  // Explicitly bind to all interfaces
        address.sin_port = htons(PORT);

        std::cout << "Binding to port " << PORT << "...\n";
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "Bind failed: " << strerror(errno) << std::endl;
            close(server_fd);
            return;
        }
        std::cout << "Bind successful\n";

        std::cout << "Starting to listen...\n";
        if (listen(server_fd, 3) < 0) {
            std::cerr << "Listen failed: " << strerror(errno) << std::endl;
            close(server_fd);
            return;
        }
        std::cout << "Listening successfully\n";

        std::cout << "Server started at port " << PORT << std::endl;
        std::cout << "Access the stream at: http://[raspberry_pi_ip]:" << PORT << std::endl;

        while (running) {
            std::cout << "Waiting for connection...\n";
            int new_socket = accept(server_fd, nullptr, nullptr);
            if (new_socket < 0) {
                std::cerr << "Accept failed: " << strerror(errno) << std::endl;
                continue;
            }
            std::cout << "New connection accepted\n";

            std::thread([this, new_socket]() {
                std::cout << "Starting stream for client...\n";
                while (running) {
                    cv::Mat frame;
                    cap >> frame;
                    if (frame.empty()) {
                        std::cerr << "Empty frame received\n";
                        continue;
                    }

                    process_frame(frame);

                    std::vector<uchar> buffer;
                    cv::imencode(".jpg", frame, buffer);

                    std::string response = create_http_response(buffer);
                    if (send(new_socket, response.c_str(), response.length(), 0) < 0) {
                        std::cerr << "Send failed: " << strerror(errno) << std::endl;
                        break;
                    }
                }
                std::cout << "Client disconnected\n";
                close(new_socket);
            }).detach();
        }
        close(server_fd);
    }

    void cleanup() {
        running = false;
        cap.release();
    }
};

CameraStream* g_stream = nullptr;

void signal_handler(int signum) {
    std::cout << "\nSignal (" << signum << ") received. Cleaning up...\n";
    if (g_stream) {
        g_stream->cleanup();
    }
    exit(signum);
}

int main() {
    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    CameraStream stream;
    g_stream = &stream;

    if (!stream.initialize()) {
        return -1;
    }

    std::cout << "Starting camera stream server...\n";
    stream.stream_frames();

    g_stream = nullptr;
    return 0;
}
