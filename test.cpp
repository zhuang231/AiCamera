#include <libcamera/libcamera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/camera.h>
#include <libcamera/request.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/stream.h>

#include <iostream>
#include <fstream>
#include <memory>
#include <thread>
#include <chrono>
#include <sys/mman.h>   
#include <unistd.h>     


using namespace libcamera;

void handleRequestComplete(libcamera::Request *req) {
    std::cout << "Image capture complete!" << std::endl;

    libcamera::FrameBuffer *buffer = req->buffers().begin()->second;
    const libcamera::FrameBuffer::Plane &plane = buffer->planes()[0];

    int fd = plane.fd.get();
    size_t length = plane.length;
    size_t offset = plane.offset;

    void *data = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, offset);
    if (data == MAP_FAILED) {
        std::cerr << "Failed to mmap buffer!" << std::endl;
        return;
    }

    std::ofstream file("image.yuv", std::ios::binary);
    file.write(static_cast<char *>(data), length);
    file.close();

    munmap(data, length);
}


int main() {
    // Start camera manager
    CameraManager cm;
    cm.start();

    if (cm.cameras().empty()) {
        std::cerr << "No camera found!" << std::endl;
        return 1;
    }

    // Get the first available camera
    std::shared_ptr<Camera> camera = cm.get(cm.cameras()[0]->id());
    camera->acquire();

    // Configure for still image capture
    std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration({ StreamRole::StillCapture });
    StreamConfiguration &streamConfig = config->at(0);
    streamConfig.size.width = 640;
    streamConfig.size.height = 480;
    config->validate();

    camera->configure(config.get());

    // Allocate framebuffer
    FrameBufferAllocator allocator(camera);
    allocator.allocate(streamConfig.stream());

    const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator.buffers(streamConfig.stream());

    std::unique_ptr<Request> request = camera->createRequest();
    request->addBuffer(streamConfig.stream(), buffers[0].get());

    // Set up signal handling for completed request
    bool complete = false;
    camera->requestCompleted.connect(handleRequestComplete);

    camera->start();

    camera->queueRequest(request.get());

    // Wait for capture to complete
    while (!complete) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    camera->stop();
    camera->release();
    cm.stop();

    std::cout << "Image saved as image.yuv" << std::endl;
    return 0;
}

