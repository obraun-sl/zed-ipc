#ifndef SHARED_MEMORY_HANDLER_H
#define SHARED_MEMORY_HANDLER_H

#include <vector>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <iostream>
#include <string>
#include <sl/Camera.hpp>

#define DEFAULT_SHARED_NAME "default_data_handler"

namespace shm
{
namespace bip = boost::interprocess;
namespace bc = boost::container;

using Mem = bip::managed_shared_memory;
using Segment = Mem::segment_manager;

template <typename T> using Alloc = bip::allocator<T, Segment>;
template <typename T, int cap> using Queue = boost::lockfree::spsc_queue<T, boost::lockfree::capacity<cap>  >;
using Buffer = std::vector<unsigned char, Alloc<unsigned char> >;
using BufferQueue = Queue<Buffer,2>;

typedef struct MatHeader_ {
    int width;
    int height;
    size_t step;
    sl::MAT_TYPE matType;

    MatHeader_(int width_, int height_, size_t step_, sl::MAT_TYPE matType_):
        width(width_), height(height_), step(step_),matType(matType_)
    {}

    static size_t size() {
        return sizeof(MatHeader_);
    }

    size_t dataSize() {
        return height * step;
    }

    size_t totalSize() {
        return dataSize() + size();
    }
} MatHeader;


class ShMatHandler
{
public :

    ShMatHandler() {}
    ~ShMatHandler() {bip::shared_memory_object::remove(name.c_str());}

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

    bool send(sl::Mat m)
    {
        if (!queue || name.empty())
            return false;

        shm::MatHeader header(m.getWidth(), m.getHeight(), m.getStepBytes(),m.getDataType());
        shm::Buffer buf = shm::Buffer(header.totalSize(), segment.get_segment_manager());
        size_t offset = 0;
        memcpy(buf.data(), (unsigned char*) &header, shm::MatHeader::size()); offset += shm::MatHeader::size();
        memcpy(buf.data() + offset, m.getPtr<unsigned char>(), header.dataSize());
        queue->push(buf);
        return true;
    }

    bool recv(sl::Mat& m)
    {
        if (!queue || name.empty())
            return false;

        shm::Buffer v(segment.get_segment_manager());
        if (queue->pop(v)) {
            size_t offset = 0;
            shm::MatHeader& header = *((shm::MatHeader*) v.data());
            offset += shm::MatHeader::size();
            if (m.getWidth()!=header.width || m.getHeight()!=header.height)
            {
                m.free();
                m.alloc(header.width, header.height, header.matType);
            }

            memcpy( m.getPtr<unsigned char>(), v.data() + offset, header.dataSize());
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
