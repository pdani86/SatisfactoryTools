#include "FactoryGameSave.h"


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

}