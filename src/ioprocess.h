#ifndef IOPROCESS_H
#define IOPROCESS_H

#include <filesystem>
#include <unistd.h>
#include <cerrno>	 
#include <cstring>

class IO_process{

    int source_fd = -1;
    int destination_fd = -1;
    void cleanup(); // closes all file descriptors

    public:
        std::filesystem::path source;
        std::filesystem::path destination;
        std::filesystem::file_status source_info;
        std::filesystem::file_status destination_info;

        ~IO_process(){cleanup();}
        void open_files(); // opens both files and sets file descriptors
        int get_source_fd() const {return source_fd;}
        int get_destionation_fd() const {return destination_fd;}
};

#endif
