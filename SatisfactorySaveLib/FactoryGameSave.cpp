#include "FactoryGameSave.h"

#include "Compressor.h"

namespace factorygame {

    std::string SaveFileHeader::toString() const {
        std::stringstream ss;
        ss << "SaveHeaderVersion: " << saveHeaderVersion << "\n";
        ss << "SaveVersion: " << saveVersion << "\n";
        ss << "BuildVersion: " << buildVersion << "\n";
        ss << "MapName: " << mapName.str << "\n";
        ss << "MapOptions: " << mapOptions.str << "\n";
        ss << "SessionName: " << sessionName.str << "\n";
        ss << "PlayedSeconds: " << playedSeconds << "\n";
        ss << "SaveTimestamp: " << saveTimestamp << "\n";
        ss << "SessionVisibility: " << (int)sessionVisibility << "\n";
        ss << "EditorObjectVersion: " << editorObjectVersion << "\n";
        ss << "ModMetaData: " << modMetaData.str << "\n";
        ss << "ModFlags: " << modFlags << "\n";

        return ss.str();
    }

    SaveFileHeader SaveFileHeader::read(std::istream& stream) {
        PropertyReader reader(stream);
        SaveFileHeader header;
        header.saveHeaderVersion = reader.readBasicType<Int>();
        header.saveVersion = reader.readBasicType<Int>();
        header.buildVersion = reader.readBasicType<Int>();
        header.mapName = reader.readBasicType<String>();
        header.mapOptions = reader.readBasicType<String>();
        header.sessionName = reader.readBasicType<String>();
        header.playedSeconds = reader.readBasicType<Int>();
        header.saveTimestamp = reader.readBasicType<Long>();
        header.sessionVisibility = reader.readBasicType<Byte>();
        header.editorObjectVersion = reader.readBasicType<Int>();
        header.modMetaData = reader.readBasicType<String>();
        header.modFlags = reader.readBasicType<Int>();
        return header;
    }


    CompressedChunkHeader CompressedChunkHeader::read(std::istream& stream) {
        PropertyReader reader(stream);
        CompressedChunkHeader body;
        body.unrealSignature = reader.readBasicType<Int>();
        static constexpr uint32_t unrealMagic = 0x9E2A83C1;
        if (stream.gcount() != sizeof(int32_t)) {
            throw std::runtime_error("Couldn't read chunk header");
        }
        if (body.unrealSignature != unrealMagic) {
            throw std::runtime_error("unreal magic number mismatch");
        }
        reader.readBasicType<Int>(); // padding
        body.maxChunkSize = reader.readBasicType<Int>();
        reader.readBasicType<Int>(); // padding
        body.compressedSize = reader.readBasicType<Int>();
        reader.readBasicType<Int>(); // padding
        body.uncompressedSize = reader.readBasicType<Int>();
        reader.readBasicType<Int>(); // padding
        body.compressedSize2 = reader.readBasicType<Int>();
        reader.readBasicType<Int>(); // padding
        body.uncompressedSize2 = reader.readBasicType<Int>();
        reader.readBasicType<Int>(); // padding
        return body;
    }

    std::vector<CompressedChunkInfo> SaveFileLoader::_collectChunkPositions(std::istream& stream) {
        std::vector<CompressedChunkInfo> chunks;
        try {
            while (stream.good()) {
                auto pos = stream.tellg();
                auto chunkInfo = CompressedChunkHeader::read(stream);
                CompressedChunkInfo compressedChunk;
                compressedChunk.pos = pos + CompressedChunkHeader::headerSize;
                compressedChunk.compressedSize = chunkInfo.compressedSize;
                compressedChunk.uncompressedSize = chunkInfo.uncompressedSize;
                chunks.push_back(compressedChunk);
                stream.seekg(compressedChunk.compressedSize, std::ios::cur);
            }
        }
        catch (...) {}
        return chunks;
    }

    SaveFileLoader::SaveFileLoader(std::string filename) {
        std::ifstream ifs;
        ifs.open(filename, std::ios::binary);
        if (!ifs.is_open()) {
            throw std::runtime_error(std::string("Couldn't open file: ") + filename);
        }
        _header = SaveFileHeader::read(ifs);
        _chunks = _collectChunkPositions(ifs);
    }

    std::vector<uint8_t> SaveFileLoader::decompressChunks(const factorygame::SaveFileLoader& loader, std::istream& fileStream) {
        std::vector<uint8_t> result;
        auto& chunks = loader.chunks();
        int64_t uncompressedSizeSum = 0;
        for (auto& chunk : chunks) {
            uncompressedSizeSum += chunk.uncompressedSize;
        }
        result.reserve(uncompressedSizeSum);
        for (auto& chunk : chunks) {
            fileStream.seekg(chunk.pos);
            std::vector<uint8_t> buffer;
            buffer.resize(chunk.compressedSize);
            fileStream.read((char*)buffer.data(), chunk.compressedSize);
            auto uncompressedData = Compressor::decompress(buffer, chunk.uncompressedSize);
            result.resize(result.size() + uncompressedData.size());
            auto dstStart = &result.at(result.size() - uncompressedData.size());
            memcpy((char*)dstStart, uncompressedData.data(), uncompressedData.size());
        }
        return result;
    }

}