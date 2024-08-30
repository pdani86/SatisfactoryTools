

#include "..\SatisfactorySaveLib\FactoryGameSave.h"
#include "..\SatisfactorySaveLib\Compressor.h"

#include <iostream>
#include <numeric>
#include <iomanip>

std::vector < std::vector<uint8_t>> compressDataIntoChunks(const std::vector<uint8_t>& data) {
    
    std::vector < std::vector<uint8_t>> chunks;
    int64_t rem = data.size();
    constexpr int64_t blockSize = 64 * 1024;
    const uint8_t* srcPtr = data.data();
    do {
        auto size = std::min(blockSize, rem);
        chunks.emplace_back(Compressor::compress(srcPtr, size));
        srcPtr += size;
        rem -= size;
    } while (rem > 0);
    return chunks;
}


struct LogObjectsOverloadSet {
    LogObjectsOverloadSet(std::ofstream& ofs) : log(ofs) {}
    void operator()(const factorygame::ActorHeader& actorHeader) {
        log << actorHeader.instanceName.str << " [" << actorHeader.posX << "," << actorHeader.posY << "," << actorHeader.posZ << "]" << "\n";
    }

    void operator()(const factorygame::ComponentHeader& componentHeader) {
        log << componentHeader.instanceName.str << " <- " << componentHeader.parentActorName.str << "\n";
    }

    void operator()(const factorygame::ComponentObjectRaw& component_) {
        auto component = factorygame::ComponentObject::fromRaw(component_, 19094 == (cnt++));
        log << component_.size << " | " << component.index << " | " << component.element_type.str << "|" << component.element_count << "\n";
    }

    void operator()(const factorygame::ActorObjectRaw& actor_) {
        cnt++;
        auto actor = factorygame::ActorObject::fromRaw(actor_, false);
        log << "actor | " << actor.parent_root.str << " | " << actor.parent_root.str << " | " << actor.component_count << "\n";
        for (auto& componentRef : actor.componentRefs) {
            log << "    " << componentRef.levelName.str << " | " << componentRef.pathName.str << "\n";
        }
        
        //log << actor.size << "\n";
    }

    std::ofstream& log;
    int cnt{ 0 };
};

void log_objects(const factorygame::SaveFileBody& saveFileBody) {
   
   std::ofstream log("log_objects_2.txt", std::ios::binary);

   LogObjectsOverloadSet logger(log);

   int index = 0;
   for (auto& objectHeader : saveFileBody.objectHeaders) {
       log << index++ << " | ";
       std::visit(logger, objectHeader.header);
   }

   index = 0;
   for (auto& object : saveFileBody.objects) {
       log << index++ << " | ";
       std::visit(logger, object.object);
   }
}

void testSaveFile(std::string filename) {
    factorygame::SaveFileLoader loader(filename);

    std::cout << loader.header().toString() << std::endl;
    auto& chunks = loader.chunks();
    std::cout << "nchunk: " << chunks.size() << std::endl;
    std::ifstream ifs;
    ifs.open(filename, std::ios::binary);
    //std::ofstream ofs;
    //std::string uncompressedFilename = "uncompressed_body.dat";
    //ofs.open(uncompressedFilename, std::ios::binary);

    auto uncompressedData = factorygame::SaveFileLoader::decompressChunks(loader, ifs);

    //ofs.write((const char*)uncompressedData.data(), uncompressedData.size());
    //ofs.close();
    //std::ifstream uncompressedInputStream;
    //uncompressedInputStream.open(uncompressedFilename, std::ios::binary);
    //auto saveFileBody = factorygame::SaveFileBody::read(uncompressedInputStream);

    std::stringstream ss;
    std::copy(uncompressedData.begin(), uncompressedData.end(), std::ostreambuf_iterator<char>(ss));
    auto saveFileBody = factorygame::SaveFileBody::read(ss);

    log_objects(saveFileBody);
    return;

    std::stringstream writeBackTestSS;
    saveFileBody.write(writeBackTestSS);
    auto writeBackTestStr = writeBackTestSS.str();
    
    std::cout << "uncompressed body size: " << uncompressedData.size() << std::endl;
    std::cout << "writeback body size: " << writeBackTestStr.size() << std::endl;
    std::ofstream uncompWriteBackFile("uncomp_writeback.txt", std::ios::binary);
    uncompWriteBackFile.write(writeBackTestStr.data(), writeBackTestStr.size());
    uncompWriteBackFile.flush();

    /*
	auto firstActor = std::get<factorygame::ActorObjectRaw>(saveFileBody.objects[0].object);
	std::cout << "componentCount" << firstActor.componentCount << std::endl;
	std::cout << "parobjname" << firstActor.parentObjectName.str << std::endl;
	std::cout << "parobjroot" << firstActor.parentObjectRoot.str << std::endl;
	std::cout << "raw size" << firstActor.raw.size() << std::endl;
	std::cout << "size" << firstActor.size << std::endl;
    

    for (int i = 0; i < 20; ++i) {
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)firstActor.raw[i] << " ";
    } std::cout << std::endl;
    */
    

    std::ofstream saveFileWriteBack("SaveFileWriteBack.sav", std::ios::binary);
    factorygame::SaveFileWriter::save(saveFileWriteBack, loader.header(), saveFileBody);


    auto recompressedChunks = compressDataIntoChunks(uncompressedData);
    std::cout << "recompressed nchunk: " << recompressedChunks.size() << std::endl;
    std::ofstream recompFile("recomp.txt", std::ios::binary);
    for (auto& chunk : recompressedChunks) {
        recompFile.write((const char*)chunk.data(), chunk.size());
    }
    auto sumsumsum = std::accumulate(recompressedChunks.begin(), recompressedChunks.end(), 0ll, [](int64_t s, const std::vector<uint8_t>& v) -> int64_t {return s + v.size(); });
    std::cout << "recompressed size sum: " << sumsumsum << std::endl;
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

int main(int argc, const char* argv[])
{
    try {
        std::string filename = "proba.sav";
        if (argc > 1) {
            filename = std::string(argv[1]);
        }
        testSaveFile(filename);
        //testCompressor();
    } catch (const std::exception& e) {
        std::cout << "exception: " << e.what() << std::endl;
    }
    std::cout << "DONE" << std::endl;
    //getchar();
    return 0;
}
