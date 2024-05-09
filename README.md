# zed-ipc-cv
IPC transfer for ZED images 


This repository shows how to transfer ZED images through sl::Mat (ZED SDK) to another process.</br>
This uses Boost Interprocess library for IPC transfer. 


## Getting started

- First, download the latest version of the ZED SDK on [stereolabs.com](https://www.stereolabs.com).
- For more information, read the ZED [API documentation](https://www.stereolabs.com/developers/documentation/API/).


## Requirements 
- Linux (Ubuntu or JetsonL4T)
- [ZED SDK](https://www.stereolabs.com/developers/) and its dependencies ([CUDA](https://developer.nvidia.com/cuda-downloads))
- Boost library (` $sudo apt install libboost-all-dev `)
- OpenCV library


## Build the program

#### Windows
- Windows has not been tested, although it should be compatible with few changes on cmake files


#### Build for Linux

Open a terminal in the sample directory and execute the following command:

- `mkdir build && cd build` <br/>
- `cmake .. && make`<br/>

This will build 2 samples "Producer" and "Consumer". 

Both are using the same class behind in a server or client configuration for easy integration

## Run the program

Start Producer first : 
`$ ./producer`
<br/>

Start consumer then : 
`$ ./consumer`
<br/>

Both programs will show the same left image of the ZED cameras. One is directly extracted from ZED SDK (producer), the other one is read from shared memory (consumer)


## Notes
- Only tested with RGB images, but should work as-is for Floating points images as well (depth, etc...)<br/>
- Support 1 producer and multiple consumers up to 4 (but can modified using the `MAX_CONSUMER` define) <br/>


## Features Roadmap

<b>v0.3.0</b> <br/>
- Add function to get the available shared memory space/name
- Add function to retrieve the version number
- Check support in Docker
- Benchmark and unit tests

 


