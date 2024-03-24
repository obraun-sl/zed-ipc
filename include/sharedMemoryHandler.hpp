#ifndef SHARED_MEMORY_HANDLER_H
#define SHARED_MEMORY_HANDLER_H


#include <vector>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

#define DEFAULT_SHARED_NAME "default_data_handler"

namespace shm
{
namespace bip = boost::interprocess;
namespace bc = boost::container;

using Mem = bip::managed_shared_memory;
using Segment = Mem::segment_manager;

template <typename T> using Alloc = bip::allocator<T, Segment>;
template <typename T, int cap> using Queue = boost::lockfree::spsc_queue<T, boost::lockfree::capacity<cap>  >;
using Buffer = std::vector<uchar, Alloc<uchar> >;
using BufferQueue = Queue<Buffer,2>;

typedef struct MatHeader_ {
    int cols;
    int rows;
    size_t step;
    size_t elemSize;
    size_t elemType;

    MatHeader_(int cols, int rows, size_t step, size_t elemSize, size_t elemType):
        cols(cols), rows(rows), step(step), elemSize(elemSize), elemType(elemType)
    {}

    static size_t size() {
        return sizeof(MatHeader_);
    }

    size_t dataSize() {
        return rows * step;
    }

    size_t totalSize() {
        return dataSize() + size();
    }
} MatHeader;


class MatHanlder
{
public :

    MatHanlder() {}
    ~MatHanlder() {bip::shared_memory_object::remove(name.c_str());}

    void createServer(std::string name_,int reserved_size_Mb=64)
    {
        name = name_;
        max_reserved_shared_size = (unsigned long long)reserved_size_Mb*1000000ULL;
        bip::shared_memory_object::remove(name.c_str());
        segment =  bip::managed_shared_memory(bip::open_or_create, name.c_str(),max_reserved_shared_size);
        queue = segment.find_or_construct<shm::BufferQueue>("queue")();


    }

    void createClient(std::string name_,int reserved_size_Mb=64)
    {
        name = name_;
        max_reserved_shared_size = (unsigned long long)reserved_size_Mb*1000000ULL;
        segment =  bip::managed_shared_memory(bip::open_or_create,name.c_str(), max_reserved_shared_size);
        queue = segment.find_or_construct<shm::BufferQueue>("queue")();
    }

    bool send(cv::Mat m)
    {
        if (!queue || name.empty())
            return false;

        shm::MatHeader header(m.cols, m.rows, m.step[0], m.elemSize(), m.type());
        shm::Buffer buf = shm::Buffer(header.totalSize(), segment.get_segment_manager());
        size_t offset = 0;
        memcpy(buf.data(), (uchar*) &header, shm::MatHeader::size()); offset += shm::MatHeader::size();
        memcpy(buf.data() + offset, m.data, header.dataSize());
        queue->push(buf);
        return true;
    }

    bool recv(cv::Mat& m)
    {
        if (!queue || name.empty())
            return false;

        shm::Buffer v(segment.get_segment_manager());
        if (queue->pop(v)) {
            size_t offset = 0;
            shm::MatHeader& header = *((shm::MatHeader*) v.data());
            offset += shm::MatHeader::size();
            m = cv::Mat(header.rows, header.cols, header.elemType, v.data() + offset, header.step);
            return true;
        }

        return false;
    }

private :
    shm::BufferQueue* queue = nullptr;
    bip::managed_shared_memory segment;
    unsigned long long max_reserved_shared_size = 64000000ULL;
    std::string name="";

};
}

#endif // SHARED_MEMORY_RING_BUFFER_BASE_H 
