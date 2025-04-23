    #include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

// AITRIOS SDK headers would go here
// #include "aitrios_camera.h"

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
bool initialize_camera(void);
ObstacleDetection* process_frame(void);
void cleanup_camera(void);

// Mock functions for testing
bool initialize_camera(void) {
    printf("Initializing AITRIOS camera...\n");
    // Add actual camera initialization code here
    return true;
}

ObstacleDetection* process_frame(void) {
    static ObstacleDetection detection;

    // Simulate obstacle detection
    // In real implementation, this would process actual camera data
    detection.obstacle_detected = rand() % 2;
    detection.distance = rand() % 200;  // Random distance 0-200cm
    detection.x_position = rand() % FRAME_WIDTH;
    detection.y_position = rand() % FRAME_HEIGHT;

    return &detection;
}

void cleanup_camera(void) {
    printf("Cleaning up camera resources...\n");
    // Add actual cleanup code here
}

int main(void) {
    printf("Starting obstacle detection test...\n");

    if (!initialize_camera()) {
        printf("Failed to initialize camera!\n");
        return -1;
    }

    // Run detection loop
    int test_iterations = 10;
    printf("Running %d test iterations...\n", test_iterations);

    for (int i = 0; i < test_iterations; i++) {
        ObstacleDetection* result = process_frame();

        printf("\nIteration %d:\n", i + 1);
        printf("Obstacle Detected: %s\n", result->obstacle_detected ? "YES" : "NO");
        printf("Distance: %d cm\n", result->distance);
        printf("Position: (%d, %d)\n", result->x_position, result->y_position);

        if (result->obstacle_detected && result->distance < THRESHOLD_DISTANCE) {
            printf("WARNING: Obstacle too close!\n");
        }

        sleep(1);  // Wait 1 second between iterations
    }

    cleanup_camera();
    printf("\nTest completed successfully!\n");

    return 0;
}