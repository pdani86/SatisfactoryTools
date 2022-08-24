

#include "..\SatisfactorySaveLib\FactoryGameSave.h"
#include "..\SatisfactorySaveLib\Compressor.h"

#include <iostream>

void testSaveFile() {
    //std::cout << "Hello World!\n";
    std::string filename = "proba.sav";
    factorygame::SaveFileLoader loader(filename);

    std::cout << loader.header().toString() << std::endl;
    auto& chunks = loader.chunks();
    std::cout << "nchunk: " << chunks.size() << std::endl;
    std::ifstream ifs;
    ifs.open(filename, std::ios::binary);
    std::ofstream ofs;
    std::string uncompressedFilename = "uncompressed_body.dat";
    ofs.open(uncompressedFilename, std::ios::binary);
    for (auto& chunk : chunks) {
        ifs.seekg(chunk.pos);
        std::vector<uint8_t> buffer;
        buffer.resize(chunk.compressedSize);
        ifs.read((char*)buffer.data(), chunk.compressedSize);
        auto uncompressedData = Compressor::decompress(buffer, chunk.uncompressedSize);
        ofs.write((const char*)uncompressedData.data(), uncompressedData.size());
        std::cout << "chunk [" << chunk.pos << "]: " << chunk.compressedSize << " / " << chunk.uncompressedSize << " - " << chunk.compressedSize / (float)chunk.uncompressedSize * 100.0f << " %" << std::endl;
    }
    ofs.close();
    std::ifstream uncompressedInputStream;
    uncompressedInputStream.open(uncompressedFilename, std::ios::binary);

    auto saveFileBody = factorygame::SaveFileBody::read(uncompressedInputStream);


    std::ofstream log("log_objects.txt", std::ios::binary);
    for (auto& actorHeader : saveFileBody.actorHeaders) {
        log << actorHeader.instanceName.str << " [" << actorHeader.posX << "," << actorHeader.posY << "," << actorHeader.posZ << "]" << "\n";
    }
    for (auto& componentHeader : saveFileBody.componentHeaders) {
        log << componentHeader.instanceName.str << "\n";
    }

}

void testCompressor() {
    auto vectorFromString = [](const std::string& str) {
        std::vector<uint8_t> vec;
        vec.resize(str.size());
        memcpy(vec.data(), str.data(), str.size());
        return vec;
    };
    std::string testInput = "ABCD1234-ABCD1234";
    testInput = testInput + std::string(1,'\0');
    
    std::vector<uint8_t> testInputVec = vectorFromString(testInput);
    auto compressedData = Compressor::compress(testInputVec);
    std::cout << "compressed size: " << compressedData.size() << std::endl;
    auto decompressedData = Compressor::decompress(compressedData, testInputVec.size());
    std::cout << "decompressed size: " << decompressedData.size() << std::endl;
    decompressedData.push_back('\0');
    std::cout << "decompressed str: " << std::string((const char*)decompressedData.data()) << std::endl;
}

int main()
{
    try {
        testSaveFile();
        //testCompressor();
    } catch (const std::exception& e) {
        std::cout << "exception: " << e.what() << std::endl;
    }
    std::cout << "DONE" << std::endl;
    getchar();
    return 0;
}
