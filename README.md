# zed-ipc-cv
IPC transfer for ZED images 


This repository shows how to transfer ZED images through OpenCV Mat to another process.</br>
This uses Boost Interprocess library for IPC transfer. 


## Getting started

- First, download the latest version of the ZED SDK on [stereolabs.com](https://www.stereolabs.com).
- For more information, read the ZED [API documentation](https://www.stereolabs.com/developers/documentation/API/).


## Requirements 
- Linux (Ubuntu or JetsonL4T)
- [ZED SDK](https://www.stereolabs.com/developers/) and its dependencies ([CUDA](https://developer.nvidia.com/cuda-downloads))
- Boost library ('$sudo apt install libboost-all-dev')
- OpenCV library


## Build the program

#### Windows
- Windows has not been tested, although it should be compatible with few changes on cmake files


#### Build for Linux

Open a terminal in the sample directory and execute the following command:

- 'mkdir build && cd build' <br/>
- 'cmake .. && make'

This will build 2 samples "Producer" and "Consumer". Both are using the same class behind in a server or client configuration for easy integration

## Run the program

Start Producer first : 
'$ ./producer'


Start consumer then : 
'$ ./consumer'

Both programs will show the same left image of the ZED cameras. One is directly extracted from ZED SDK (producer), the other one is read from shared memory


## Notes
- Only tested with RGB images, but should work as-is for Floating points images as well (depth, etc...)