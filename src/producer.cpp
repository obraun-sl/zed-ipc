
// ZED include
#include <sl/Camera.hpp>

//Shared memory handler include
#include "sharedMemoryHandler.hpp"

int main() {


    // Create a ZED Camera object
    sl::Camera zed;

    sl::InitParameters init_parameters;
    init_parameters.sdk_verbose = true;
    init_parameters.camera_resolution= sl::RESOLUTION::HD2K;
    init_parameters.depth_mode = sl::DEPTH_MODE::NONE; // no depth computation required here
    init_parameters.async_grab_camera_recovery = true;

    // Open the camera
    sl::ERROR_CODE returned_state = zed.open(init_parameters);
    if (returned_state != sl::ERROR_CODE::SUCCESS) {
        std::cout<<" Cannot Start camera"<<std::endl;
        return EXIT_FAILURE;
    }

    sl::Mat zed_image;

    shm::MatHanlder shmhandler;
    shmhandler.createServer(DEFAULT_SHARED_NAME);
    unsigned char key = ' ';
    while (key!='q') {

        returned_state = zed.grab();
        if (returned_state == sl::ERROR_CODE::SUCCESS) {
            // Retrieve left image
            zed.retrieveImage(zed_image, sl::VIEW::LEFT);
            // Convert sl::Mat to cv::Mat (share buffer)
            cv::Mat cvImage = cv::Mat((int) zed_image.getHeight(), (int) zed_image.getWidth(), CV_8UC4, zed_image.getPtr<sl::uchar1>(sl::MEM::CPU));
            // Send the mat to shared memory/ IPC
            shmhandler.send(cvImage);
            cv::imshow("Original",cvImage);
            key = cv::waitKey(5);
        }
        usleep(1000);
    }
}
