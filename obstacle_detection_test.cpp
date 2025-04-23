#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <opencv2/opencv.hpp>

// Define constants
#define THRESHOLD_DISTANCE 100  // Example threshold in centimeters
#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480

// Structure to hold obstacle detection results
typedef struct {
    bool obstacle_detected;
    int distance;
    int x_position;
    int y_position;
} ObstacleDetection;

// Function prototypes
bool initialize_camera(cv::VideoCapture& cap);
ObstacleDetection* process_frame(cv::Mat& frame);
void cleanup_camera(cv::VideoCapture& cap);

bool initialize_camera(cv::VideoCapture& cap) {
    printf("Initializing camera...\n");
    cap.open(0); // Open default camera (usually webcam)

    if (!cap.isOpened()) {
        printf("Error: Could not open camera\n");
        return false;
    }

    // Set resolution
    cap.set(cv::CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
    return true;
}

ObstacleDetection* process_frame(cv::Mat& frame) {
    static ObstacleDetection detection;

    // For now, we'll keep the random detection logic
    // In a real implementation, you would process the frame here
    detection.obstacle_detected = rand() % 2;
    detection.distance = rand() % 200;  // Random distance 0-200cm
    detection.x_position = rand() % FRAME_WIDTH;
    detection.y_position = rand() % FRAME_HEIGHT;

    // Draw detection information on frame
    if (detection.obstacle_detected) {
        // Draw a red circle at detected position
        cv::circle(frame, cv::Point(detection.x_position, detection.y_position),
                  20, cv::Scalar(0, 0, 255), 2);

        // Add text with distance
        char text[50];
        sprintf(text, "Distance: %d cm", detection.distance);
        cv::putText(frame, text, cv::Point(10, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
    }

    return &detection;
}

void cleanup_camera(cv::VideoCapture& cap) {
    printf("Cleaning up camera resources...\n");
    cap.release();
    cv::destroyAllWindows();
}

int main(void) {
    printf("Starting obstacle detection test with video stream...\n");

    cv::VideoCapture cap;
    if (!initialize_camera(cap)) {
        printf("Failed to initialize camera!\n");
        return -1;
    }

    cv::namedWindow("Obstacle Detection", cv::WINDOW_AUTOSIZE);
    cv::Mat frame;

    printf("Press 'q' to quit the program\n");

    while (true) {
        // Capture frame
        cap >> frame;
        if (frame.empty()) {
            printf("Error: Could not capture frame\n");
            break;
        }

        // Process frame and get detection results
        ObstacleDetection* result = process_frame(frame);

        // Display warning if obstacle is too close
        if (result->obstacle_detected && result->distance < THRESHOLD_DISTANCE) {
            printf("WARNING: Obstacle too close! Distance: %d cm\n", result->distance);
        }

        // Show the frame
        cv::imshow("Obstacle Detection", frame);

        // Break the loop if 'q' is pressed
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    cleanup_camera(cap);
    printf("\nTest completed successfully!\n");
    return 0;
}