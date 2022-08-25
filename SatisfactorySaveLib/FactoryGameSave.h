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
        void write(std::ostream& stream) const;
    };

    enum class ObjectType {
        Component = 0,
        Actor = 1
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

        void write(std::ostream& stream) const {
            PropertyWriter writer(stream);
            writer.writeBasicType(Int{(int)ObjectType::Actor});
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
            writer.writeBasicType(Int{ (int)ObjectType::Component});
            writer.writeBasicType(typePath);
            writer.writeBasicType(rootObject);
            writer.writeBasicType(instanceName);

            writer.writeBasicType(parentActorName);
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
            stream.write((const char*)raw.data(), raw.size());
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
            stream.write((const char*)raw.data(), raw.size());
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
        std::vector<ActorObjectRaw> actorObjects;
        std::vector<ComponentObjectRaw> componentObjects;

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
                    header.componentObjects.emplace_back(ComponentObjectRaw::read(stream));
                } else {
                    header.actorObjects.emplace_back(ActorObjectRaw::read(stream));
                    
                }
            }

            // TODO

            return header;
        }

        void write(std::ostream& stream) {
            PropertyWriter writer(stream);
            writer.writeBasicType(uncompressedSize);
            writer.writeBasicType(objectHeaderCount);
            writer.writeBasicType(objectCount);
            writer.writeBasicType(collectedObjectsCount);

            for (auto& actorHeader : actorHeaders) {
                actorHeader.write(stream);
            }
            for (auto& componentHeader : componentHeaders) {
                componentHeader.write(stream);
            }
            for (auto& actor : actorObjects) {
                actor.write(stream);
            }
            for (auto& component : componentObjects) {
                component.write(stream);
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
