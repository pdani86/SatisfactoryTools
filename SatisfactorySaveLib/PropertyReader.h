#pragma once

#include "Properties.h"

#include <iostream>

namespace factorygame {

    class PropertyReader {
    public:
        explicit PropertyReader(std::istream& stream) : _stream(stream) {}

        static constexpr auto MAX_STRING_LEN = 1024 * 1024;

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
            value.size = size;
            if (size > MAX_STRING_LEN) {
                throw std::runtime_error("String too large");
            }
            if (notUtf8) {
                throw std::runtime_error("Only UTF8 is supported");
                //return std::string();
            }
            value.str.resize(size, '\0');
            if (size)
                _stream.read((char*)&value.str.at(0), size);
            value.str = std::string(value.str.c_str());
            return value;
        }


    private:
        std::istream& _stream;
    };

    class PropertyWriter {
    public:
        explicit PropertyWriter(std::ostream& stream) : _stream(stream) {

        }

        template<typename T>
        void writeBasicType(const T& value) {
            _stream.write((char*)&value, sizeof(value));
        }

        template<>
        void writeBasicType(const String& str) {
            int32_t size = str.str.size()+1;
            _stream.write((const char*)&size, sizeof(int32_t));
            _stream.write(str.str.data(), str.str.size());
            _stream.write("\0", 1);
        }

    private:
        std::ostream& _stream;
    };

}
