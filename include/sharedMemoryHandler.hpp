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

#define SMH_VERSION_MAJOR 0
#define SMH_VERSION_MINOR 3
#define SMH_VERSION_PATCH 0

#define DEFAULT_SHARED_NAME "default_data_handler"
#define MAX_CONSUMER 4
#define MAX_TIMEOUT_POP 1000 //in ms
#define MAX_RING_BUFFER_CNT 2


namespace shm
{
    namespace bip = boost::interprocess;
    namespace bc = boost::container;

    using Mem = bip::managed_shared_memory;
    using Segment = Mem::segment_manager;

    template <typename T> using Alloc = bip::allocator<T, Segment>;
    template <typename T, int cap> using Queue = boost::lockfree::spsc_queue<T, boost::lockfree::capacity<cap>  >;
    using Buffer = std::vector<unsigned char, Alloc<unsigned char> >;
    using BufferQueue = Queue<Buffer,MAX_RING_BUFFER_CNT>;

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
        ~ShMatHandler() {if (exist_) bip::shared_memory_object::remove(name.c_str());}

        void createServer(std::string name_,int reserved_size_Mb=128)
        {
            name = name_;
            max_reserved_shared_size = (unsigned long long)reserved_size_Mb*1000000ULL;
            bip::shared_memory_object::remove(name.c_str());
            boost::interprocess::permissions  unrestricted_permissions;
            unrestricted_permissions.set_unrestricted();
            segment =  bip::managed_shared_memory(bip::open_or_create, name.c_str(),max_reserved_shared_size,0,unrestricted_permissions);


            for (int i=0;i<MAX_CONSUMER;i++)
            {
                std::string queue_name = std::string("queue_")+std::to_string(i);
                queue[i] = segment.find_or_construct<shm::BufferQueue>(queue_name.c_str())();
            }
            exist_=true;
        }

        bool createClient(std::string name_)
        {
            name = name_;

            // Find empty slot in generated ring buffers
            // To do that, we trick since the SPSC does not provide this information.
            // --> Read the number of available data in queue using read_available.
            // If read_available==2, it means that the slot is not read yet, therefore empty.
            // This is not guarantee to work all the time but seems to.
            int empty_slot_index = 0;
            while(empty_slot_index<MAX_CONSUMER)
            {
                try {
                    segment =  bip::managed_shared_memory(bip::open_only,name.c_str());
                    std::string queue_name = std::string("queue_")+std::to_string(empty_slot_index);
                    queue[empty_slot_index] = segment.find<shm::BufferQueue>(queue_name.c_str()).first;
                    exist_=segment.check_sanity();
                    if (queue[empty_slot_index]->read_available() == MAX_RING_BUFFER_CNT && exist_)
                    {
                        recv_index=empty_slot_index;
                        return exist_;
                    }
                }
                catch (...)
                {
                    return false;
                }
                empty_slot_index++;
            }

            return false;
        }

        bool send(sl::Mat m)
        {
            if (name.empty())
                return false;

            shm::MatHeader header(m.getWidth(), m.getHeight(), m.getStepBytes(),m.getDataType());
            shm::Buffer buf = shm::Buffer(header.totalSize(), segment.get_segment_manager());
            size_t offset = 0;
            memcpy(buf.data(), (unsigned char*) &header, shm::MatHeader::size()); offset += shm::MatHeader::size();
            memcpy(buf.data() + offset, m.getPtr<unsigned char>(), header.dataSize());
            for (int i=0;i<MAX_CONSUMER;i++)
            {
                if (queue[i])
                    queue[i]->push(buf);
            }
            return true;
        }

        bool recv(sl::Mat& m)
        {
            if (name.empty() || recv_index<0)
                return false;

            shm::Buffer v(segment.get_segment_manager());
            if (queue[recv_index]->pop(v)) {
                size_t offset = 0;
                shm::MatHeader& header = *((shm::MatHeader*) v.data());
                offset += shm::MatHeader::size();
                if (m.getWidth()!=header.width || m.getHeight()!=header.height)
                {
                    m.free();
                    m.alloc(header.width, header.height, header.matType);
                }

                memcpy( m.getPtr<unsigned char>(), v.data() + offset, header.dataSize());
                last_rcv_ts = sl::getCurrentTimeStamp();
                return true;
            }
            else
            {
                sl::Timestamp current_ts = sl::getCurrentTimeStamp();
                //When timeout is reached (probably producer down), recreate every MAX_TIMEOUT_POP milliseconds the memory segment to reconnect.
                //Once the producer is online again, the connection will happen in less than MAX_TIMEOUT_POP milliseconds
                if (current_ts.getMilliseconds()-last_rcv_ts.getMilliseconds()>=MAX_TIMEOUT_POP)
                {
                    createClient(name);
                    last_rcv_ts = sl::getCurrentTimeStamp();
                }

            }

            return false;
        }

        int getConsumerSlot()
        {
            return recv_index;
        }


    private :
        shm::BufferQueue* queue[MAX_CONSUMER] = {nullptr};
        bip::managed_shared_memory segment;
        unsigned long long max_reserved_shared_size = 64000000ULL;
        std::string name="";
        bool exist_=false;
        int recv_index = -1;
        sl::Timestamp last_rcv_ts = 0;

    };

    static void getVersion(int &major, int& minor, int& patch)
    {
        major = SMH_VERSION_MAJOR;
        minor = SMH_VERSION_MINOR;
        patch = SMH_VERSION_PATCH;
    }
}

#endif // SHARED_MEMORY_RING_BUFFER_BASE_H 
