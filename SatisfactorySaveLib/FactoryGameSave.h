#pragma once


#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <variant>

#include "Properties.h"
#include "PropertyReader.h"
#include "Compressor.h"

namespace factorygame {

    struct SaveFileHeader {
        Int saveHeaderVersion{};
        Int saveVersion{};
        Int buildVersion{};
        String mapName;
        String mapOptions;
        String sessionName;
        Int playedSeconds{};
        Long saveTimestamp{};
        Byte sessionVisibility{};
        Int editorObjectVersion{};
        String modMetaData;
        Int modFlags{};

        std::string toString() const;

        int64_t headerSize() const {
            constexpr int64_t intSize = 4;
            constexpr int64_t longSize = 8;
            constexpr int64_t byteSize = 1;
            constexpr int64_t stringSizeSize = 4;
            int64_t baseSizeWithoutStringContent = 1 * byteSize + 6 * intSize + 1 * longSize + 4 * stringSizeSize;
            return baseSizeWithoutStringContent + mapName.size + mapOptions.size + sessionName.size + modMetaData.size;
        }

        bool hasCompressedBody() const {
            return saveVersion >= 21;
        }


        static SaveFileHeader read(std::istream& stream);
        void write(std::ostream& stream) const;
    };

    enum class ObjectType {
        Component = 0,
        Actor = 1
    };


    struct ActorHeader {
        String typePath;
        String rootObject;
        String instanceName;
        Int needTransform;
        Float rotX;
        Float rotY;
        Float rotZ;
        Float rotW;
        Float posX;
        Float posY;
        Float posZ;
        Float scaleX;
        Float scaleY;
        Float scaleZ;
        Int wasPlacedInLevel;

        static ActorHeader read(std::istream& stream) {
            ActorHeader header;
            PropertyReader reader(stream);
            header.typePath = reader.readBasicType<String>();
            header.rootObject = reader.readBasicType<String>();
            header.instanceName = reader.readBasicType<String>();
            header.needTransform = reader.readBasicType<Int>();

            header.rotX = reader.readBasicType<Float>();
            header.rotY = reader.readBasicType<Float>();
            header.rotZ = reader.readBasicType<Float>();
            header.rotW = reader.readBasicType<Float>();

            header.posX = reader.readBasicType<Float>();
            header.posY = reader.readBasicType<Float>();
            header.posZ = reader.readBasicType<Float>();

            header.scaleX = reader.readBasicType<Float>();
            header.scaleY = reader.readBasicType<Float>();
            header.scaleZ = reader.readBasicType<Float>();

            header.wasPlacedInLevel = reader.readBasicType<Int>();

            return header;
        }

        void write(std::ostream& stream) const {
            PropertyWriter writer(stream);
            writer.writeBasicType(Int{ (int)1 });
            writer.writeBasicType(typePath);
            writer.writeBasicType(rootObject);
            writer.writeBasicType(instanceName);

            writer.writeBasicType(needTransform);
            writer.writeBasicType(rotX);
            writer.writeBasicType(rotY);
            writer.writeBasicType(rotZ);
            writer.writeBasicType(rotW);
            writer.writeBasicType(posX);
            writer.writeBasicType(posY);
            writer.writeBasicType(posZ);
            writer.writeBasicType(scaleX);
            writer.writeBasicType(scaleY);
            writer.writeBasicType(scaleZ);
            writer.writeBasicType(wasPlacedInLevel);
        }
    };

    struct ComponentHeader {
        String typePath;
        String rootObject;
        String instanceName;
        String parentActorName;

        static ComponentHeader read(std::istream& stream) {
            ComponentHeader header;
            PropertyReader reader(stream);
            header.typePath = reader.readBasicType<String>();
            header.rootObject = reader.readBasicType<String>();
            header.instanceName = reader.readBasicType<String>();
            header.parentActorName = reader.readBasicType<String>();
            return header;
        }

        void write(std::ostream& stream) const {
            PropertyWriter writer(stream);
            writer.writeBasicType(Int{(int)0});
            writer.writeBasicType(typePath);
            writer.writeBasicType(rootObject);
            writer.writeBasicType(instanceName);

            writer.writeBasicType(parentActorName);
        }
    };

    struct ObjectHeader {
        Int headerType{};

        std::variant<ActorHeader, ComponentHeader> header;

        static ObjectHeader read(std::istream& stream) {
            ObjectHeader header;
            PropertyReader reader(stream);
            header.headerType = reader.readBasicType<Int>();
            if (header.headerType == 0) {
                header.header = ComponentHeader::read(stream);
            } else {
                header.header = ActorHeader::read(stream);
            }
            return header;
        }
    };

    struct ActorObjectRaw {
        Int size{};
        String parentObjectRoot;
        String parentObjectName;
        Int componentCount{};

        std::vector<uint8_t> raw;
        // components...
        // properties...
        // trailing bytes...

        static ActorObjectRaw read(std::istream& stream) {
            PropertyReader reader(stream);
            ActorObjectRaw result;
            result.size = reader.readBasicType<Int>();
            result.parentObjectRoot = reader.readBasicType<String>();
            result.parentObjectName = reader.readBasicType<String>();
            result.componentCount = reader.readBasicType<Int>();
            auto rawSize = result.size - 1 * sizeof(int32_t) - 2 * sizeof(int32_t) - result.parentObjectName.size - result.parentObjectRoot.size;
            result.raw.resize(rawSize);
            stream.read((char*)result.raw.data(), rawSize);
            return result;
        }

        void write(std::ostream& stream) const {
            PropertyWriter writer(stream);
            writer.writeBasicType(size);
            writer.writeBasicType(parentObjectRoot);
            writer.writeBasicType(parentObjectName);
            writer.writeBasicType(componentCount);
            /*if (raw.size())*/ stream.write((const char*)raw.data(), raw.size());
        }
    };

    struct ComponentObjectRaw {
        Int size{};

        std::vector<uint8_t> raw;
        // properties...
        // trailing bytes...

        static ComponentObjectRaw read(std::istream& stream) {
            PropertyReader reader(stream);
            ComponentObjectRaw result;
            result.size = reader.readBasicType<Int>();

            auto rawSize = result.size;
            result.raw.resize(rawSize);
            stream.read((char*)result.raw.data(), rawSize);
            return result;
        }

        void write(std::ostream& stream) const {
            PropertyWriter writer(stream);
            writer.writeBasicType(size);
            /*if (raw.size()) */ stream.write((const char*)raw.data(), raw.size());
        }
    };

    struct Object {
        ObjectType type;
        std::variant<ComponentObjectRaw, ActorObjectRaw> object;

        void write(std::ostream& stream) const {
            if (type == ObjectType::Component) {
                std::get<ComponentObjectRaw>(object).write(stream);
            } else {
                std::get<ActorObjectRaw>(object).write(stream);
            }
        }
    };

    struct ObjectReference {
        String levelName;
        String pathName;

        static ObjectReference read(std::istream& stream) {
            PropertyReader reader(stream);
            ObjectReference result;
            result.levelName = reader.readBasicType<String>();
            result.pathName = reader.readBasicType<String>();
            return result;
        }

        void write(std::ostream& stream) const {
            PropertyWriter writer(stream);
            writer.writeBasicType(levelName);
            writer.writeBasicType(pathName);
        }

    };

    
    struct SaveFileBody {
        Int uncompressedSize{};
        Int objectHeaderCount{};
        Int objectCount{};
        Int collectedObjectsCount{};

        std::vector<ObjectHeader> objectHeaders;
        std::vector<Object> objects;
        std::vector<ObjectReference> collectedObjects;

        static SaveFileBody read(std::istream& stream) {
            PropertyReader reader(stream);
            SaveFileBody header;
            header.uncompressedSize = reader.readBasicType<Int>();
            header.objectHeaderCount = reader.readBasicType<Int>();

            header.objectHeaders.reserve(header.objectHeaderCount);

            for (int objIx = 0; objIx < header.objectHeaderCount; ++objIx) {
                header.objectHeaders.emplace_back(ObjectHeader::read(stream));
            }

            header.objectCount = reader.readBasicType<Int>();

            if (header.objectHeaderCount != header.objectCount) {
                // TODO
                return header;
            }

            for (int objIx = 0; objIx < header.objectCount; ++objIx) {
                if (header.objectHeaders[objIx].headerType == 0) {
                    header.objects.push_back({ObjectType::Component, ComponentObjectRaw::read(stream) });
                } else {
                    header.objects.push_back({ ObjectType::Actor, ActorObjectRaw::read(stream) });
                }
            }

            header.collectedObjectsCount = reader.readBasicType<Int>();

            for (int collectedObjIx = 0; collectedObjIx < header.collectedObjectsCount; ++collectedObjIx) {
                header.collectedObjects.emplace_back(ObjectReference::read(stream));
            }

            return header;
        }

        void write(std::ostream& stream) const {
            PropertyWriter writer(stream);
            writer.writeBasicType(uncompressedSize);
            writer.writeBasicType(objectHeaderCount);

            for (auto& objectHeader : objectHeaders) {
                if (objectHeader.headerType == 0) {
                    std::get<ComponentHeader>(objectHeader.header).write(stream);
                } else {
                    std::get<ActorHeader>(objectHeader.header).write(stream);
                }
            }

            writer.writeBasicType(objectCount);
            
            for (auto& object : objects) {
                object.write(stream);
            }
            
            writer.writeBasicType(collectedObjectsCount);

            for (auto& collectedObject : collectedObjects) {
                collectedObject.write(stream);
            }
        }
    };


    struct CompressedChunkHeader {
        Int unrealSignature{};
        Int maxChunkSize{};
        Int compressedSize{};
        Int uncompressedSize{};
        Int compressedSize2{};
        Int uncompressedSize2{};

        static constexpr int64_t headerSize = 12*sizeof(int32_t);
        static CompressedChunkHeader read(std::istream& stream);

        static CompressedChunkHeader create(int32_t compressedSize, int32_t uncompressedSize) {
            CompressedChunkHeader result;
            static constexpr uint32_t unrealMagic = 0x9E2A83C1;
            result.unrealSignature = unrealMagic;
            result.maxChunkSize = 128*1024;
            result.compressedSize = result.compressedSize2 = compressedSize;
            result.uncompressedSize = result.uncompressedSize2 = uncompressedSize;
            return result;
        }

        void write(std::ostream& stream) const;
    };

    struct CompressedChunkInfo {
        int64_t pos{};
        int64_t compressedSize{};
        int64_t uncompressedSize{};
    };

    
    class SaveFileLoader {
    public:
        explicit SaveFileLoader(std::string filename);

        const SaveFileHeader& header() const { return _header; }
        const std::vector<CompressedChunkInfo>& chunks() const { return _chunks; }

        static std::vector<uint8_t> decompressChunks(const factorygame::SaveFileLoader& loader, std::istream& fileStream);

    private:
        SaveFileHeader _header;
        std::vector<CompressedChunkInfo> _chunks;

    private:
        static std::vector<CompressedChunkInfo> _collectChunkPositions(std::istream& stream);
    };


    
    class SaveFileWriter {
    public:
        struct CompressedChunk {
            int64_t uncompressedSize{};
            std::vector<uint8_t> data;

        };

        static void save(std::ostream& stream, const SaveFileHeader& header, const SaveFileBody& body) {
            header.write(stream);
            std::stringstream uncompressedDataSS; // TODO??, inefficient, stream vs vector
            body.write(uncompressedDataSS);
            std::vector<uint8_t> uncompressedData;
            auto str = uncompressedDataSS.str();
            uncompressedData.resize(str.size());
            memcpy(uncompressedData.data(), str.data(), str.size());
            *reinterpret_cast<int32_t*>(&uncompressedData.data()[0]) = uncompressedData.size() - 4; // fix uncompressed size

            //std::ofstream ofs("uncomprbeforesave.txt", std::ios::binary);
            //ofs.write((const char*)uncompressedData.data(), uncompressedData.size());

            auto chunks = _compressDataIntoChunks(uncompressedData);
            for (auto& chunk : chunks) {
                _writeChunk(stream, chunk);
            }
        }

        static void _writeChunk(std::ostream& stream, const CompressedChunk& chunk) {
            CompressedChunkHeader header = CompressedChunkHeader::create(chunk.data.size(), chunk.uncompressedSize);
            header.write(stream);
            stream.write((const char*)chunk.data.data(), chunk.data.size());
        }

        static std::vector<CompressedChunk> _compressDataIntoChunks(const std::vector<uint8_t>& data) {
            std::vector<CompressedChunk> chunks;
            int64_t rem = data.size();
            constexpr int64_t blockSize = 64 * 1024;
            const uint8_t* srcPtr = data.data();
            do {
                auto size = std::min(blockSize, rem);
                chunks.emplace_back(CompressedChunk{size, Compressor::compress(srcPtr, size) });
                srcPtr += size;
                rem -= size;
            } while (rem > 0);
            return chunks;
        }
    };
}
