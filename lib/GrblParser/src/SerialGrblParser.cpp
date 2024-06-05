#include "SerialGrblParser.h"

SerialGrblParser::SerialGrblParser(Stream &stream) : m_stream{stream}
{
}

uint16_t SerialGrblParser::available()
{
    return m_stream.available();
}

char SerialGrblParser::read()
{
    return m_stream.read();
}

void SerialGrblParser::write(char c)
{
    m_stream.print(c);
}
