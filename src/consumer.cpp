#include "sharedMemoryHandler.hpp"
#include <opencv2/opencv.hpp>

inline cv::Mat slMat2cvMat(sl::Mat& input) {
    // Mapping between MAT_TYPE and CV_TYPE
    int cv_type = -1;
    switch (input.getDataType()) {
        case sl::MAT_TYPE::F32_C1: cv_type = CV_32FC1;
            break;
        case sl::MAT_TYPE::F32_C2: cv_type = CV_32FC2;
            break;
        case sl::MAT_TYPE::F32_C3: cv_type = CV_32FC3;
            break;
        case sl::MAT_TYPE::F32_C4: cv_type = CV_32FC4;
            break;
        case sl::MAT_TYPE::U8_C1: cv_type = CV_8UC1;
            break;
        case sl::MAT_TYPE::U8_C2: cv_type = CV_8UC2;
            break;
        case sl::MAT_TYPE::U8_C3: cv_type = CV_8UC3;
            break;
        case sl::MAT_TYPE::U8_C4: cv_type = CV_8UC4;
            break;
        default: break;
    }

    return cv::Mat(input.getHeight(), input.getWidth(), cv_type, input.getPtr<sl::uchar1>(sl::MEM::CPU));
}

int main(int argc, char *argv[]) {

    int maj_v,min_v,patch_v;
    shm::getVersion(maj_v,min_v,patch_v);
    std::cout<<" SHM Version : "<<maj_v<<"."<<min_v<<"."<<patch_v<<std::endl;


    shm::ShMatHandler shmhandler;
    bool res = shmhandler.createClient(DEFAULT_SHARED_NAME);
    if (!res)
    {
        std::cout<<" Cannot Open Capture --"<<std::endl;
        std::cout<<" Either the shared memory handler does not exist or the number of slot is already full"<<std::endl;
        return 1;
    }

    std::cout<<" Consumer Acquisition SLOT : "<<shmhandler.getConsumerSlot()<<std::endl;
    sl::Mat mat;
    cv::Mat dMat;
    unsigned char key = ' ';
    while (key!='q') {
        if (shmhandler.recv(mat)) {
            dMat = slMat2cvMat(mat);
            cv::imshow("shared CV", dMat);
        }
        key = cv::waitKey(5);
        usleep(1000);
    }
}
