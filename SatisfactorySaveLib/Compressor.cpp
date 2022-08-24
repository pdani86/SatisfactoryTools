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

std::vector<uint8_t> Compressor::compress(const std::vector<uint8_t>& data) {
    return Compressor::compress(data.data(), data.size());
}

std::vector<uint8_t> Compressor::compress(const uint8_t* data, int64_t size) {
    std::vector<uint8_t> result;
    result.resize(size + 64);
    z_stream c_stream; /* compression stream */
    int err;
    auto len = size;

    c_stream.zalloc = myalloc;
    c_stream.zfree = myfree;
    c_stream.opaque = (voidpf)0;

    //return (ZLibReturnCode)deflateInit2_64(ref strm, (int)level, (int)ZLibCompressionMethod.Deflated, windowBits, (int)ZLibMemLevel.Default, (int)ZLibCompressionStrategy.Default, ZLibVersion, Marshal.SizeOf(typeof(z_streamp)));
    //err = deflateInit2(&c_stream, ...);
    err = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
    check_zlib_err(err);

    c_stream.avail_in = len;
    c_stream.next_in = (z_const unsigned char*)data;
    c_stream.next_out = result.data();
    c_stream.avail_out = result.size();

    while (c_stream.total_in != len && c_stream.total_out < result.size()) {
        err = deflate(&c_stream, Z_NO_FLUSH);
        check_zlib_err(err);
    }
    /* Finish the stream, still forcing small buffers: */
    for (;;) {
        err = deflate(&c_stream, Z_FINISH);
        if (err == Z_STREAM_END) break;
        check_zlib_err(err);
    }

    err = deflateEnd(&c_stream);
    check_zlib_err(err);
    result.resize(c_stream.total_out);
	return result;
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
