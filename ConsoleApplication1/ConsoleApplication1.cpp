
#include <iostream>
#include <sstream>
#include <fstream>

namespace factorygame {


    
    using Byte = int8_t;
    using Int = int32_t;
    using Long = int64_t;
    using Float = float;
    using String = std::string;
    

    class PropertyReader {
    public:
        explicit PropertyReader(std::istream& stream) : _stream(stream) {}

        static constexpr auto MAX_STRING_LEN = 65536;

        template<typename T>
        T readBasicType() {
            T value;
            _stream.read((char*)&value, sizeof(value));
            return value;
        }

        template<>
        String readBasicType() {
            String value;
            int32_t sizeTmp = 0;
            _stream.read((char*)&sizeTmp, sizeof(sizeTmp));
            const bool notUtf8 = sizeTmp < 0;
            const int32_t size = std::abs(sizeTmp);
            if (size > MAX_STRING_LEN) {
                throw std::runtime_error("String too large");
            }
            if (notUtf8) {
                //throw std::runtime_error("Only UTF8 is supported");
                //return std::string();
            }
            value.resize(size, '\0');
            if(size)
                _stream.read((char*)&value.at(0), size);
            return value;
        }


    private:
        std::istream& _stream;
    };


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

        std::string toString() const {
            std::stringstream ss;
            ss << "SaveHeaderVersion: " << saveHeaderVersion << "\n";
            ss << "SaveVersion: " << saveVersion << "\n";
            ss << "BuildVersion: " << buildVersion << "\n";
            ss << "MapName: " << mapName << "\n";
            ss << "MapOptions: " << mapOptions << "\n";
            ss << "SessionName: " << sessionName << "\n";
            ss << "PlayedSeconds: " << playedSeconds << "\n";
            ss << "SaveTimestamp: " << saveTimestamp << "\n";
            ss << "SessionVisibility: " << (int)sessionVisibility << "\n";
            ss << "EditorObjectVersion: " << editorObjectVersion << "\n";
            ss << "ModMetaData: " << modMetaData << "\n";
            ss << "ModFlags: " << modFlags << "\n";

            return ss.str();
        }

        bool hasCompressedBody() const {
            //saveVersion
            return true;
        }


        static SaveFileHeader read(std::istream& stream) {
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
    };

}

int main()
{
    try {
        //std::cout << "Hello World!\n";
        std::ifstream ifs;
        ifs.open("proba.sav", std::ios::binary | std::ios::in);
        if (!ifs.is_open()) {
            return -1;
        }
        auto saveFileHeader = factorygame::SaveFileHeader::read(ifs);
        std::cout << saveFileHeader.toString() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "exception: " << e.what() << std::endl;
    }
    getchar();
    return 0;
}
