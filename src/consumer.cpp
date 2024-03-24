#include "sharedMemoryHandler.hpp"
#include <opencv2/opencv.hpp>

int main() {
    shm::MatHanlder shmhandler;
    shmhandler.createClient(DEFAULT_SHARED_NAME);
    cv::Mat mat;
    unsigned char key = ' ';
    while (key!='q') {
        if (shmhandler.recv(mat)) {
            cv::imshow("shared CV", mat);
            key = cv::waitKey(5);
        }
        usleep(1000);
    }
}
