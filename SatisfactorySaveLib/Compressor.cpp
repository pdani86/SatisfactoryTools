#include "Compressor.h"

#include "zlib.h"

#include <iostream>
#include <string>

inline void check_zlib_err(int err) {
    if (err == Z_OK) {
        return;
    }
    throw std::runtime_error(std::string("zlib error: ") + std::to_string(err));
}

std::vector<uint8_t> Compressor::compress(const std::vector<uint8_t>& data, int64_t targetSizeHint) {
	return {};
}


static void* myalloc(void* q, unsigned int n, unsigned int  m) {
    //std::cout << "myalloc " << n << " " << m << std::endl;
    (void)q;
    return calloc(n, m);
}

static void myfree(void* q, void* p) {
    //std::cout << "myfree" << std::endl;
    (void)q;
    free(p);
}

std::vector<uint8_t> Compressor::decompress(const std::vector<uint8_t>& data, int64_t targetSizeHint) {
	std::vector<uint8_t> result;
	result.resize(targetSizeHint);
    int err;
	z_stream stream;
    stream.zalloc = myalloc;
    stream.zfree = myfree;
    stream.opaque = (voidpf)0;

    stream.next_in = (uint8_t*)data.data();
    stream.avail_in = data.size();
    stream.next_out = result.data();
    stream.avail_out = result.size();

    err = inflateInit(&stream);
    //err = inflateInit2(&stream, -MAX_WBITS);
    //err = inflateInit2(&stream, -15);
    check_zlib_err(err);

    while (stream.total_out < result.size() && stream.total_in < data.size()) {
        err = inflate(&stream, Z_NO_FLUSH);
        if (err == Z_STREAM_END) break;
        check_zlib_err(err);
    }

    err = inflateEnd(&stream);
    check_zlib_err(err);

	return result;
}
