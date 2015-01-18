#include "decompressor.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cinttypes>
#include <iostream>
#include "miniz.c"

decompressor::decompressor(const std::string & _path):
    path(_path)
{}

decompressor::~decompressor()
{
    close();
}

void decompressor::close()
{
    if (stream != nullptr)
    {
        free(stream);
    }

    if (read_buffer != nullptr)
    {
        free(read_buffer);
    }

    if (file_descriptor != -1)
    {
        ::close(file_descriptor);
        file_descriptor = -1;
    }
}

void decompressor::open()
{
    struct stat file_stats;
    if ((file_descriptor = ::open(path.c_str(), O_RDONLY)) == -1)
    {
        std::cerr << "Error opening " << path << '\n';
        exit(1);
    }

    if (stat(path.c_str(), &file_stats) == -1)
    {
        std::cerr << "Error getting file stats for " << path << '\n';
        exit(1);
    }

    file_size = file_stats.st_size;
    remaining = file_size;
    read_buffer = (unsigned char*)malloc(8192);
    back_buffer = read_buffer + 4096;
}

void decompressor::seek(ssize_t len)
{
    remaining -= len;
    lseek(file_descriptor, len, SEEK_CUR);
}

void decompressor::skip_string()
{
    char current_char;
    do
    {
        ::read(file_descriptor, &current_char, 1);
    } while(current_char != 0);
}

void decompressor::skip_header()
{
    unsigned char orig_head[10];
    ::read(file_descriptor, orig_head, 10);
    remaining -= 10;
    if (remaining < 10)
    {
        std::cerr << "Not large enough to be a gzipped file\n";
        exit(1);
    }

    if (orig_head[0] != 0x1F || orig_head[1] != 0x8B)
    {
        std::cerr << "File not in gzip format\n";
        exit(1);
    }
    if (orig_head[2] != 8)
    {
        std::cerr << "Unknown z-algorithm " << (int)orig_head[2] << '\n';
        exit(1);
    }
    
    if (orig_head[3] & 1 << 2) /* FEXTRA */
    {
        uint16_t n;
        uint8_t n_[2], _[1];
        ::read(file_descriptor, n_, 2);
        
        for (n = n_[0] << 0 | n_[1] << 8; n-- > 0; ::read(file_descriptor, _, 1));
    }
    if (orig_head[3] & 1 << 3) /* FNAME    */ skip_string();
    if (orig_head[3] & 1 << 4) /* FCOMMENT */ skip_string();
    if (orig_head[3] & 1 << 1) /* FCRC */ {
        seek(2);
    }
}

size_t decompressor::read(void* buffer, size_t length)
{
    if (file_descriptor == -1)
    {
        open();
        skip_header();

        stream = (z_stream*)malloc(sizeof(z_stream));
        memset(stream, 0, sizeof(z_stream));

        // Init the z_stream
        stream->next_in = read_buffer;
        stream->avail_in = 0;
        stream->next_out = (unsigned char*)buffer;
        stream->avail_out = length;

        if (inflateInit2(stream, -MZ_DEFAULT_WINDOW_BITS))
        {
            std::cerr << "inflateInit() failed!\n";
            exit(1);
        }
    }

    if (remaining == 0)
    {
        return 0;
    }
    if (stream->avail_in < 1024)
    {
        unsigned char* new_buffer = stream->next_in < back_buffer ? back_buffer : read_buffer;
        memcpy(new_buffer, stream->next_in, stream->avail_in);
        stream->avail_in += ::read(file_descriptor, new_buffer + stream->avail_in, 4096 - stream->avail_in);
        stream->next_in = new_buffer;
        if (stream->avail_in == 0)
        {
            // EOF
            remaining = 0;
            if (inflateEnd(stream) != Z_OK)
            {
                std::cerr << "inflateEnd() failed\n";
                exit(1);
            }
            close();
            return 0;
        }
    }

    stream->next_out = (unsigned char*)buffer;
    stream->avail_out = length;

    int status = inflate(stream, Z_SYNC_FLUSH);
    size_t out_bytes = length - stream->avail_out;

    if (status == Z_STREAM_END)
    {
        remaining = 0;

        if (inflateEnd(stream) != Z_OK)
        {
            std::cerr << "inflateEnd() failed with status " << status << '\n';
            exit(1);
        }
    }

    else if (status != Z_OK)
    {
        std::cerr << "inflate() failed with status " << status << '\n';
        exit(1);
    }

    if (out_bytes == 0)
    {
        close();
    }

    return out_bytes;
}
