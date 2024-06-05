#ifndef SerialParser_H_INCLUDED
#define SerialParser_H_INCLUDED

#include "GrblParser.h"

#include <Arduino.h>

class SerialGrblParser : public GrblParser
{
public:
    explicit SerialGrblParser(Stream &stream);

private:
    Stream &m_stream;

protected:
    [[nodiscard]] uint16_t available() override;
    [[nodiscard]] char read() override;
    void write(char c) override;
};

#endif