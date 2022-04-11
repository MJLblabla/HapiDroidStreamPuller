#include "amf0.h"

#include <algorithm>
#include <assert.h>
#include <cstring>

#include "amf_core.h"
#include "amf_errno.h"
#include "simple_buffer.h"

class AMF0_MARKER
{
public:
    static const char AMF0_MARKER_NUMBER        = 0x00;
    static const char AMF0_MARKER_BOOLEAN       = 0x01;
    static const char AMF0_MARKER_STRING        = 0x02;
    static const char AMF0_MARKER_OBJECT        = 0x03;
    static const char AMF0_MARKER_MOVIECLIP     = 0x04; // reserved, not used
    static const char AMF0_MARKER_NULL          = 0x05;
    static const char AMF0_MARKER_UNDEFINED     = 0x06;
    static const char AMF0_MARKER_REFERENCE     = 0x07;
    static const char AMF0_MARKER_ECMA_ARRAY    = 0x08;
    static const char AMF0_MARKER_OBJECT_END    = 0x09;
    static const char AMF0_MARKER_STRICT_ARRAY  = 0x0A;
    static const char AMF0_MARKER_DATE          = 0x0B;
    static const char AMF0_MARKER_LONG_STRING   = 0x0C;
    static const char AMF0_MARKER_UNSUPPORTED   = 0x0D;
    static const char AMF0_MARKER_RECORDSET     = 0x0E; // reserved, not used
    static const char AMF0_MARKER_XML_DOC       = 0x0F;
    static const char AMF0_MARKER_TYPED_OBJECT  = 0x10;

    static const char AMF0_MARKER_INVALID       = 0xff;
};

Amf0Data::Amf0Data()
{
    marker = AMF0_MARKER::AMF0_MARKER_INVALID;
}

Amf0Data::~Amf0Data()
{

}

bool Amf0Data::is_number()
{
    return marker == AMF0_MARKER::AMF0_MARKER_NUMBER;
}

bool Amf0Data::is_boolean()
{
    return marker == AMF0_MARKER::AMF0_MARKER_BOOLEAN;
}

bool Amf0Data::is_string()
{
    return marker == AMF0_MARKER::AMF0_MARKER_STRING;
}

bool Amf0Data::is_object()
{
    return marker == AMF0_MARKER::AMF0_MARKER_OBJECT;
}

bool Amf0Data::is_object_end()
{
    return marker == AMF0_MARKER::AMF0_MARKER_OBJECT_END;
}

bool Amf0Data::is_null()
{
    return marker == AMF0_MARKER::AMF0_MARKER_NULL;
}

bool Amf0Data::is_undefined()
{
    return marker == AMF0_MARKER::AMF0_MARKER_UNDEFINED;
}

bool Amf0Data::is_ecma_array()
{
    return marker == AMF0_MARKER::AMF0_MARKER_ECMA_ARRAY;
}

Amf0Data *Amf0Data::create_amf0data(SimpleBuffer *sb)
{
    if (!sb->require(1)) {
        return nullptr;
    }

    int8_t m = sb->read_1byte();
    sb->skip(-1);

    switch (m) {
        case AMF0_MARKER::AMF0_MARKER_NUMBER: {
            Amf0Number *value = new Amf0Number();
            if (value->read(sb) != ERROR_SUCCESS) {
                freep(value);
                return nullptr;
            }
            return value;
        }
        case AMF0_MARKER::AMF0_MARKER_BOOLEAN: {
            Amf0Boolean *value = new Amf0Boolean();
            if (value->read(sb) != ERROR_SUCCESS) {
                freep(value);
                return nullptr;
            }
            return value;
        }
        case AMF0_MARKER::AMF0_MARKER_STRING: {
            Amf0String *value = new Amf0String();
            if (value->read(sb) != ERROR_SUCCESS) {
                freep(value);
                return nullptr;
            }
            return value;
        }
        case AMF0_MARKER::AMF0_MARKER_OBJECT: {
            Amf0Object *value = new Amf0Object();
            if (value->read(sb) != ERROR_SUCCESS) {
                freep(value);
                return nullptr;
            }
            return value;
        }
        case AMF0_MARKER::AMF0_MARKER_NULL: {
            Amf0Null *value = new Amf0Null();
            if (value->read(sb) != ERROR_SUCCESS) {
                freep(value);
                return nullptr;
            }
            return value;
        }
        case AMF0_MARKER::AMF0_MARKER_UNDEFINED: {
            Amf0Undefined *value = new Amf0Undefined();
            if (value->read(sb) != ERROR_SUCCESS) {
                freep(value);
                return nullptr;
            }
            return value;
        }
        case AMF0_MARKER::AMF0_MARKER_ECMA_ARRAY: {
            Amf0EcmaArray *value = new Amf0EcmaArray();
            if (value->read(sb) != ERROR_SUCCESS) {
                freep(value);
                return nullptr;
            }
            return value;
        }
        default:
            break;
    }

    return nullptr;
}

Amf0Number::Amf0Number()
{
    marker = AMF0_MARKER::AMF0_MARKER_NUMBER;
}

Amf0Number::Amf0Number(double val)
{
    marker = AMF0_MARKER::AMF0_MARKER_NUMBER;
    value = val;
}

Amf0Number::~Amf0Number()
{

}

int Amf0Number::read(SimpleBuffer *sb)
{
    int ret = ERROR_SUCCESS;

    if (!sb->require(1)) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    marker = sb->read_1byte();
    if (marker != AMF0_MARKER::AMF0_MARKER_NUMBER) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    if (!sb->require(8)) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    int64_t temp = sb->read_8bytes();
    memcpy(&value, &temp, 8);

    return ret;
}

int Amf0Number::write(SimpleBuffer *sb)
{
    sb->write_1byte(marker);

    int64_t temp = 0x00;
    memcpy(&temp, &value, 8);
    sb->write_8bytes(temp);

    return 0;
}

Amf0Boolean::Amf0Boolean()
{
    marker = AMF0_MARKER::AMF0_MARKER_BOOLEAN;
}

Amf0Boolean::Amf0Boolean(bool val)
{
    marker = AMF0_MARKER::AMF0_MARKER_BOOLEAN;
    value = val;
}

Amf0Boolean::~Amf0Boolean()
{

}

int Amf0Boolean::read(SimpleBuffer *sb)
{
    int ret = ERROR_SUCCESS;

    if (!sb->require(1)) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    marker = sb->read_1byte();
    if (marker != AMF0_MARKER::AMF0_MARKER_BOOLEAN) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    if (!sb->require(1)) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    value = (sb->read_1byte() != 0);

    return ret;
}

int Amf0Boolean::write(SimpleBuffer *sb)
{
    sb->write_1byte(marker);
    sb->write_1byte(value ? 0x01 : 0x00);

    return 0;
}

Amf0String::Amf0String()
{
    marker = AMF0_MARKER::AMF0_MARKER_STRING;
}

Amf0String::Amf0String(std::string val)
{
    marker = AMF0_MARKER::AMF0_MARKER_STRING;
    value = val;
}

Amf0String::~Amf0String()
{

}

int Amf0String::read(SimpleBuffer *sb)
{
    int ret = ERROR_SUCCESS;

    if (!sb->require(1)) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    marker = sb->read_1byte();
    if (marker != AMF0_MARKER::AMF0_MARKER_STRING) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    if (!sb->require(2)) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    int16_t len = sb->read_2bytes();
    if (!sb->require(len)) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    value = sb->read_string(len);

    return ret;
}

int Amf0String::write(SimpleBuffer *sb)
{
    sb->write_1byte(marker);
    sb->write_2bytes(value.length());
    sb->write_string(value);

    return 0;
}

Amf0ObjectProperty::Amf0ObjectProperty()
{

}

Amf0ObjectProperty::~Amf0ObjectProperty()
{

}

void Amf0ObjectProperty::put(std::string key, Amf0Data *value)
{
    auto it = std::find_if(properties.begin(), properties.end(), [key](const Property &p) {
        return p.first == key;
    });

    if (it != properties.end())
        properties.erase(it);

    properties.push_back(std::make_pair(key, std::shared_ptr<Amf0Data>(value)));
}

std::string Amf0ObjectProperty::key_at(int index)
{
    assert(index >= 0 && index < properties.size());

    return properties[index].first;
}

Amf0Data *Amf0ObjectProperty::value_at(int index)
{
    assert(index >= 0 && index < properties.size());

    return properties[index].second.get();
}

Amf0Data *Amf0ObjectProperty::value_at(std::string key)
{
    auto it = std::find_if(properties.begin(), properties.end(), [key](const Property &p) {
        return p.first == key;
    });

    if (it != properties.end())
        return it->second.get();

    return nullptr;
}

int Amf0ObjectProperty::count()
{
    return properties.size();
}

Amf0Object::Amf0Object()
{
    marker = AMF0_MARKER::AMF0_MARKER_OBJECT;
    oe = new Amf0ObjectEnd;
}

Amf0Object::~Amf0Object()
{
    delete oe;
    oe = nullptr;
}

void Amf0Object::put(std::string key, Amf0Data *value)
{
    property.put(key, value);
}

std::string Amf0Object::key_at(int index)
{
    return property.key_at(index);
}

Amf0Data *Amf0Object::value_at(std::string key)
{
    return property.value_at(key);
}

Amf0Data *Amf0Object::value_at(int index)
{
    return property.value_at(index);
}

int Amf0Object::read(SimpleBuffer *sb)
{
    int ret = ERROR_SUCCESS;

    if (!sb->require(1)) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    marker = sb->read_1byte();
    if (marker != AMF0_MARKER::AMF0_MARKER_OBJECT) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    while (!sb->empty()) {
        if (!sb->require(2)) {
            ret = ERROR_AMF0_DECODE;
            return ret;
        }

        int16_t len = sb->read_2bytes();
        if (!sb->require(len)) {
            ret = ERROR_AMF0_DECODE;
            return ret;
        }

        // maybe object end
        if (len == 0) {
            if (!sb->require(1)) {
                ret = ERROR_AMF0_DECODE;
                return ret;
            }

            if (AMF0_MARKER::AMF0_MARKER_OBJECT_END != sb->read_1byte()) {
                ret = ERROR_AMF0_DECODE;
                return ret;
            }

            return ret;
        }

        std::string property_name = sb->read_string(len);
        put(property_name, Amf0Data::create_amf0data(sb));
    }

    return ret;
}

int Amf0Object::write(SimpleBuffer *sb)
{
    sb->write_1byte(marker);

    for (int i = 0; i < property.count(); ++i) {
        std::string name = key_at(i);
        Amf0Data *data = value_at(i);

        sb->write_2bytes(name.length());
        sb->write_string(name);
        data->write(sb);
    }

    oe->write(sb);

    return 0;
}

Amf0ObjectEnd::Amf0ObjectEnd()
{
    marker = AMF0_MARKER::AMF0_MARKER_OBJECT_END;
}

Amf0ObjectEnd::~Amf0ObjectEnd()
{

}

int Amf0ObjectEnd::read(SimpleBuffer *sb)
{
    return 0;
}

int Amf0ObjectEnd::write(SimpleBuffer *sb)
{
    sb->write_2bytes(0x00);
    sb->write_1byte(marker);
    return 0;
}

Amf0Null::Amf0Null()
{
    marker = AMF0_MARKER::AMF0_MARKER_NULL;
}

Amf0Null::~Amf0Null()
{

}

int Amf0Null::read(SimpleBuffer *sb)
{
    int ret = ERROR_SUCCESS;

    if (!sb->require(1)) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    marker = sb->read_1byte();
    if (marker != AMF0_MARKER::AMF0_MARKER_NULL) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    return ret;
}

int Amf0Null::write(SimpleBuffer *sb)
{
    sb->write_1byte(marker);
    return 0;
}

Amf0Undefined::Amf0Undefined()
{
    marker = AMF0_MARKER::AMF0_MARKER_UNDEFINED;
}

Amf0Undefined::~Amf0Undefined()
{

}

int Amf0Undefined::read(SimpleBuffer *sb)
{
    int ret = ERROR_SUCCESS;

    if (!sb->require(1)) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    marker = sb->read_1byte();
    if (marker != AMF0_MARKER::AMF0_MARKER_UNDEFINED) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    return ret;
}

int Amf0Undefined::write(SimpleBuffer *sb)
{
    sb->write_1byte(marker);
    return 0;
}

Amf0EcmaArray::Amf0EcmaArray()
{
    marker = AMF0_MARKER::AMF0_MARKER_ECMA_ARRAY;
    oe = new Amf0ObjectEnd;
}

Amf0EcmaArray::~Amf0EcmaArray()
{
    delete oe;
    oe = nullptr;
}

void Amf0EcmaArray::put(std::string key, Amf0Data *value)
{
    property.put(key, value);
}

std::string Amf0EcmaArray::key_at(int index)
{
    return property.key_at(index);
}

Amf0Data *Amf0EcmaArray::value_at(std::string key)
{
    return property.value_at(key);
}

Amf0Data *Amf0EcmaArray::value_at(int index)
{
    return property.value_at(index);
}

int Amf0EcmaArray::read(SimpleBuffer *sb)
{
    int ret = ERROR_SUCCESS;

    if (!sb->require(1)) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    marker = sb->read_1byte();
    if (marker != AMF0_MARKER::AMF0_MARKER_ECMA_ARRAY) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    if (!sb->require(4)) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    int32_t count = sb->read_4bytes();
    // just for compatibility
    // for (int i = 0; i < count && !sb->empty(); i++) {
    while (!sb->empty()) {
        if (!sb->require(2)) {
            ret = ERROR_AMF0_DECODE;
            return ret;
        }

        int16_t len = sb->read_2bytes();
        if (!sb->require(len)) {
            ret = ERROR_AMF0_DECODE;
            return ret;
        }

        // maybe object end
        if (len == 0) {
            if (!sb->require(1)) {
                ret = ERROR_AMF0_DECODE;
                return ret;
            }

            if (AMF0_MARKER::AMF0_MARKER_OBJECT_END != sb->read_1byte()) {
                ret = ERROR_AMF0_DECODE;
                return ret;
            }

            return ret;
        }

        std::string property_name = sb->read_string(len);
        put(property_name, Amf0Data::create_amf0data(sb));
    }

    return ret;
}

int Amf0EcmaArray::write(SimpleBuffer *sb)
{
    sb->write_1byte(marker);
    sb->write_4bytes(property.count());

    for (int i = 0; i < property.count(); ++i) {
        std::string name = key_at(i);
        Amf0Data *data = value_at(i);

        sb->write_2bytes(name.length());
        sb->write_string(name);
        data->write(sb);
    }

    oe->write(sb);

    return 0;
}

Amf0StrictArray::Amf0StrictArray()
{
    marker = AMF0_MARKER::AMF0_MARKER_STRICT_ARRAY;
}

Amf0StrictArray::~Amf0StrictArray()
{

}

void Amf0StrictArray::put(Amf0Data *value)
{
    properties.push_back(std::shared_ptr<Amf0Data>(value));
}

Amf0Data *Amf0StrictArray::value_at(int index)
{
    assert(index >= 0 && index < properties.size());
    return properties[index].get();
}

int Amf0StrictArray::read(SimpleBuffer *sb)
{
    int ret = ERROR_SUCCESS;

    if (!sb->require(1)) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    marker = sb->read_1byte();
    if (marker != AMF0_MARKER::AMF0_MARKER_ECMA_ARRAY) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    if (!sb->require(4)) {
        ret = ERROR_AMF0_DECODE;
        return ret;
    }

    int32_t count = sb->read_4bytes();
    for (int i = 0; i < count && !sb->empty(); i++) {
        properties.push_back(std::shared_ptr<Amf0Data>(Amf0Data::create_amf0data(sb)));
    }

    return ret;
}

int Amf0StrictArray::write(SimpleBuffer *sb)
{
    sb->write_1byte(marker);
    sb->write_4bytes(properties.size());
    for (int i = 0; i < properties.size(); ++i) {
        Amf0Data *data = value_at(i);

        data->write(sb);
    }

    return 0;
}
