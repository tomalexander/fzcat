#include <iostream>
#include <memory>
#include <thread>
#include <string>
#include <sstream>
#include <vector>
#include <future>
#include <unistd.h>
#include "decompressor.h"

/**
 * Decompress a full file at path using buffer_size bytes for a buffer
 */
void full_file(const long buffer_size, const char* path)
{
    std::unique_ptr<char[]> buffer(new char[buffer_size]);
    decompressor dec(path);
    for (size_t read = dec.read(buffer.get(), buffer_size);
         read != 0;
         read = dec.read(buffer.get(), buffer_size))
    {
        std::cout.write(buffer.get(), read);
    }
}

/**
 * Decompress every path in argv and write to stdout
 */
int main(int argc, char** argv)
{
    size_t buffer_size = std::max(2048, std::min(getpagesize(), 16384));
    for (int i = 1; i < argc; ++i)
    {
        full_file(buffer_size, argv[i]);
    }
    return 0;
}
