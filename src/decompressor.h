#pragma once
#include <string>

struct mz_stream_s;

class decompressor
{
  public:
    decompressor(const std::string & _path);
    ~decompressor();
    
    /**
     * Attempt to read up to length bytes from the file and return the
     * number of bytes read. Return 0 if the file has finished being
     * read.
     * 
     * @param buffer An allocated block of memory to write in to
     * @param length The number of bytes to read
     */
    size_t read(void* buffer, size_t length);

  private:
    void open();
    void close();
    void skip_header();
    void skip_string();
    void seek(ssize_t len);

    const std::string & path;
    int file_descriptor = -1;

    size_t file_size = 0;
    size_t remaining = 0;
    unsigned char* read_buffer = nullptr;
    unsigned char* back_buffer = nullptr;
    mz_stream_s* stream = nullptr;
};
