#include <libcamera/libcamera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/camera.h>
#include <libcamera/request.h>
#include <libcamera/framebuffer_allocator.h>
#include <libcamera/stream.h>

#include <jpeglib.h>

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

//TODO do video stuff

    munmap(data, length);

// Recycle request
camera->queueRequest(req);
}

void handleRequestCompleteA(libcamera::Request *req) {
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

    // --- JPEG ENCODE START ---
    const uint8_t *yPlane = static_cast<uint8_t *>(data); // Assume Y channel only (grayscale)
    int width = 640;  // must match your streamConfig.size.width
    int height = 480; // must match your streamConfig.size.height

    FILE *outfile = fopen("image.jpg", "wb");
    if (!outfile) {
        std::cerr << "Failed to open output file!" << std::endl;
        munmap(data, length);
        return;
    }

    jpeg_compress_struct cinfo;
    jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 1;
    cinfo.in_color_space = JCS_GRAYSCALE;

    jpeg_set_defaults(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);

    JSAMPROW row_pointer[1];
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = const_cast<JSAMPROW>(&yPlane[cinfo.next_scanline * width]);
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);
    // --- JPEG ENCODE END ---

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

std::vector<std::unique_ptr<Request>> requests;
bool complete = false;
for (auto &buffer : buffers) {
    std::unique_ptr<Request> req = camera->createRequest();
    if (!req) {
        std::cerr << "Failed to create request!" << std::endl;
        return 1;
    }

    int ret = req->addBuffer(streamConfig.stream(), buffer.get());
    if (ret < 0) {
        std::cerr << "Failed to add buffer to request!" << std::endl;
        return 1;
    }

    requests.push_back(std::move(req));
}
    camera->start();

for (size_t i = 0; i < requests.size(); ++i) {
    camera->queueRequest(requests[i].get());
}
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

