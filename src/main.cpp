#include <iostream>
#include <memory>
#include <thread>
#include <string>
#include <sstream>
#include <vector>
#include <future>
#include <unistd.h>
#include "decompressor.h"

#define NUM_THREADS 4

/**
 * Decompress a full file at path using buffer_size bytes for a buffer
 */
std::unique_ptr<std::stringstream> full_file(const long buffer_size, const char* path)
{
    std::unique_ptr<std::stringstream> out = std::make_unique<std::stringstream>();
    std::unique_ptr<char[]> buffer(new char[buffer_size]);
    decompressor dec(path);
    for (size_t read = dec.read(buffer.get(), buffer_size);
         read != 0;
         read = dec.read(buffer.get(), buffer_size))
    {
        out->write(buffer.get(), read);
    }
    return std::move(out);
}

/**
 * Print out all the stringstream buffers in order
 */
void print(std::vector<std::future<std::unique_ptr<std::stringstream> > > & results)
{
    for (std::future<std::unique_ptr<std::stringstream> > & out : results)
    {
        std::cout << out.get()->rdbuf();
    }
}

/**
 * Decompress every path in argv and write to stdout
 */
int main(int argc, char** argv)
{
    size_t buffer_size = std::max(2048, std::min(getpagesize(), 16384));
    std::vector<std::future<std::unique_ptr<std::stringstream> > > results(argc - 1);
    for (int i = 1; i < argc; ++i)
    {
        results.push_back(std::async(std::launch::async, full_file, buffer_size, argv[i]));
        if (i % NUM_THREADS == 0)
        {
            print(results);
            results.clear();
        }
    }
    print(results);
    return 0;
}
