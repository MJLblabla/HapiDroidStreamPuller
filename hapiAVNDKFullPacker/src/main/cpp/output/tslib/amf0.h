#ifndef __AMF_0_H__
#define __AMF_0_H__

#include <string>
#include <vector>
#include <memory>

class SimpleBuffer;
class Amf0ObjectEnd;

class Amf0Data
{
public:
    Amf0Data();
    virtual ~Amf0Data();

public:
    virtual int read(SimpleBuffer *sb) = 0;
    virtual int write(SimpleBuffer *sb) = 0;

public:
    bool is_number();
    bool is_boolean();
    bool is_string();
    bool is_object();
    bool is_object_end();
    bool is_null();
    bool is_undefined();
    bool is_ecma_array();

public:
    static Amf0Data *create_amf0data(SimpleBuffer *sb);

public:
    char marker;
};

class Amf0Number : public Amf0Data
{
public:
    Amf0Number();
    Amf0Number(double val);
    virtual ~Amf0Number();

public:
    virtual int read(SimpleBuffer *sb);
    virtual int write(SimpleBuffer *sb);

public:
    double value;
};

class Amf0Boolean : public Amf0Data
{
public:
    Amf0Boolean();
    Amf0Boolean(bool val);
    virtual ~Amf0Boolean();

public:
    virtual int read(SimpleBuffer *sb);
    virtual int write(SimpleBuffer *sb);

public:
    bool value;
};

class Amf0String : public Amf0Data
{
public:
    Amf0String();
    Amf0String(std::string val);
    virtual ~Amf0String();

public:
    virtual int read(SimpleBuffer *sb);
    virtual int write(SimpleBuffer *sb);

public:
    std::string value;
};

class Amf0ObjectProperty
{
private:
    typedef std::pair<std::string, std::shared_ptr<Amf0Data>> Property;
    std::vector<Property> properties;

public:
    Amf0ObjectProperty();
    virtual ~Amf0ObjectProperty();

public:
    void put(std::string key, Amf0Data *value);
    std::string key_at(int index);
    Amf0Data *value_at(int index);
    Amf0Data *value_at(std::string key);
    int count();
};

class Amf0Object : public Amf0Data
{
public:
    Amf0Object();
    virtual ~Amf0Object();

public:
    void put(std::string key, Amf0Data *value);
    std::string key_at(int index);
    Amf0Data *value_at(std::string key);
    Amf0Data *value_at(int index);

public:
    virtual int read(SimpleBuffer *sb);
    virtual int write(SimpleBuffer *sb);

private:
    Amf0ObjectProperty property;
    Amf0ObjectEnd *oe;
};

class Amf0ObjectEnd : public Amf0Data
{
public:
    Amf0ObjectEnd();
    virtual ~Amf0ObjectEnd();

public:
    virtual int read(SimpleBuffer *sb);
    virtual int write(SimpleBuffer *sb);
};

class Amf0Null : public Amf0Data
{
public:
    Amf0Null();
    virtual ~Amf0Null();

public:
    virtual int read(SimpleBuffer *sb);
    virtual int write(SimpleBuffer *sb);
};

class Amf0Undefined : public Amf0Data
{
public:
    Amf0Undefined();
    virtual ~Amf0Undefined();

public:
    virtual int read(SimpleBuffer *sb);
    virtual int write(SimpleBuffer *sb);
};

class Amf0EcmaArray : public Amf0Data
{
public:
    Amf0EcmaArray();
    virtual ~Amf0EcmaArray();

public:
    void put(std::string key, Amf0Data *value);
    std::string key_at(int index);
    Amf0Data *value_at(std::string key);
    Amf0Data *value_at(int index);

public:
    virtual int read(SimpleBuffer *sb);
    virtual int write(SimpleBuffer *sb);

private:
    Amf0ObjectProperty property;
    Amf0ObjectEnd *oe;
};

class Amf0StrictArray : public Amf0Data
{
public:
    Amf0StrictArray();
    virtual ~Amf0StrictArray();

public:
    void put(Amf0Data *value);
    Amf0Data *value_at(int index);

public:
    virtual int read(SimpleBuffer *sb);
    virtual int write(SimpleBuffer *sb);

private:
    std::vector<std::shared_ptr<Amf0Data>> properties;
};

#endif /* __AMF_0_H__ */