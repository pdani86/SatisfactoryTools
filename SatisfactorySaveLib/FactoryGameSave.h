#pragma once


#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include "Properties.h"
#include "PropertyReader.h"

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
    };

    struct ObjectHeader {
        Int headerType{};

        static ObjectHeader read(std::istream& stream) {
            ObjectHeader header;
            PropertyReader reader(stream);
            header.headerType = reader.readBasicType<Int>();
            return header;
        }
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
    };

    struct ActorObject {
        Int size{};
        String parentObjectRoot;
        String parentObjectName;
        Int componentCount{};

        std::vector<uint8_t> raw;
        // components...
        // properties...
        // trailing bytes...

        static ActorObject read(std::istream& stream) {
            PropertyReader reader(stream);
            ActorObject result;
            result.size = reader.readBasicType<Int>();
            result.parentObjectRoot = reader.readBasicType<String>();
            result.parentObjectName = reader.readBasicType<String>();
            result.componentCount = reader.readBasicType<Int>();
            auto rawSize = result.size - 1 * sizeof(int32_t) - 2 * sizeof(int32_t) - result.parentObjectName.size - result.parentObjectRoot.size;
            result.raw.resize(rawSize);
            stream.read((char*)result.raw.data(), rawSize);
            return result;
        }
    };

    struct ComponentObject {
        Int size{};

        std::vector<uint8_t> raw;
        // properties...
        // trailing bytes...

        static ComponentObject read(std::istream& stream) {
            PropertyReader reader(stream);
            ComponentObject result;
            result.size = reader.readBasicType<Int>();

            auto rawSize = result.size;
            result.raw.resize(rawSize);
            stream.read((char*)result.raw.data(), rawSize);
            return result;
        }
    };

    struct ObjectReference {

    };

    
    struct SaveFileBody {
        Int uncompressedSize{};
        Int objectHeaderCount{};
        Int objectCount{};
        Int collectedObjectsCount{};

        std::vector<ActorHeader> actorHeaders;
        std::vector<ComponentHeader> componentHeaders;
        std::vector<int> headerTypes;
        std::vector<ActorObject> actorObjects;
        std::vector<ComponentObject> componentObjects;

        static SaveFileBody read(std::istream& stream) {
            PropertyReader reader(stream);
            SaveFileBody header;
            header.uncompressedSize = reader.readBasicType<Int>();
            header.objectHeaderCount = reader.readBasicType<Int>();

            header.actorHeaders.reserve(header.objectHeaderCount);
            header.componentHeaders.reserve(header.objectHeaderCount);

            for (int objIx = 0; objIx < header.objectHeaderCount; ++objIx) {
                auto objectHeader = ObjectHeader::read(stream);
                switch (objectHeader.headerType) {
                    case 0: {
                        header.componentHeaders.emplace_back(ComponentHeader::read(stream));
                        header.headerTypes.push_back(0);
                    } break;
                    case 1: {
                        header.actorHeaders.emplace_back(ActorHeader::read(stream));
                        header.headerTypes.push_back(1);
                    } break;
                    default: break;
                }
            }

            header.objectCount = reader.readBasicType<Int>();

            if (header.objectHeaderCount != header.objectCount) {
                // TODO
                return header;
            }

            for (int objIx = 0; objIx < header.objectCount; ++objIx) {
                //break; // 
                // TODO
                if (header.headerTypes[objIx] == 0) {
                    header.componentObjects.emplace_back(ComponentObject::read(stream));
                } else {
                    header.actorObjects.emplace_back(ActorObject::read(stream));
                    
                }
            }

            // TODO

            return header;
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
}
