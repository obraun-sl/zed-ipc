CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

project(ZED_IPC_CV)

if(COMMAND cmake_policy)
        cmake_policy(SET CMP0003 OLD)
        cmake_policy(SET CMP0015 OLD)
endif(COMMAND cmake_policy)


set(CMAKE_CXX_STANDARD 17)

find_package(ZED 4 REQUIRED)
find_package(CUDA REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED)


## Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
include_directories(${CUDA_INCLUDE_DIRS})
include_directories(${ZED_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})


## Add libs directories
link_directories(${ZED_LIBRARY_DIR})
link_directories(${OpenCV_LIBRARY_DIRS})
link_directories(${CUDA_LIBRARY_DIRS})

add_definitions(-std=c++17 -O3)



if(Boost_FOUND)
	set(Boost_USE_STATIC_LIBS OFF) 
	set(Boost_USE_MULTITHREADED ON)  
    include_directories(${Boost_INCLUDE_DIRS}) 
endif()


## DEBUG/ SANITIZER options
IF(NOT WIN32)
    add_definitions(-Werror=return-type  -Wendif-labels)
    IF (BUILD_WITH_SANITIZER)
        message("!! Building with address sanitizer and -g !!")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address -Wall -Wextra -g")
        set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address -Wall -Wextra -g")
        add_definitions(-g -fsanitize=address -fno-omit-frame-pointer -static-libasan -Wall -Wextra)
    ENDIF()

    IF (BUILD_WITH_DEBUGINFOS)
        message("!! Building with -g !!")
        add_definitions(-g)
    ENDIF()
ENDIF()

 

## Link executable with shared libs
SET(ZED_LIBS ${ZED_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_CUDART_LIBRARY})
 
SET(LINK_LIBRARIES
    ${ZED_LIBS}
    ${Boost_LIBRARIES}
    ${OpenCV_LIBS})
    
    
    
add_executable(consumer include/sharedMemoryHandler.hpp src/consumer.cpp)
target_link_libraries(consumer ${LINK_LIBRARIES})

add_executable(producer include/sharedMemoryHandler.hpp src/producer.cpp)
target_link_libraries(producer ${LINK_LIBRARIES})
 
