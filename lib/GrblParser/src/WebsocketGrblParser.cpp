#include "WebsocketGrblParser.h"

#include "SafeString.h"
#include "WebsocketDebugger.hpp"
#include "GrblCommands.h"

#include <functional>

namespace
{
  constexpr auto PING_INTERVAL = 5000;
  constexpr auto PONG_TIMEOUT = 1000;
  constexpr auto RECONNECTION_INTERVAL = 5000;
  constexpr auto MAX_PING_BEFORE_CLOSING_CONNECTION = 3;
} // namespace

WebsocketGrblParser::WebsocketGrblParser(const char *host, int port,
                                         const char *url)
    : GrblParser{}, m_host{host}, m_port{port}, m_url{url}
{
}

void WebsocketGrblParser::connect()
{
  m_webSocketClient.onEvent([&](WStype_t type, uint8_t *payload, size_t length)
                            {
  switch (type)
  {
  case WStype_DISCONNECTED:
  {
    wdebugf("[WSc] Disconnected!\n");
    break;
  }
  case WStype_CONNECTED:
  {
    wdebugf("[WSc] Connected to url: %s\n", payload);
    m_webSocketClient.sendTXT(Grbl::getCommand(Grbl::Command::ViewBuildInfo).c_str());
    break;
  }
  case WStype_TEXT:
  {
    wdebugf("[WSc] get text: %s\n", payload);
    m_queuedData += (const char *)payload;
    break;
  }
  case WStype_BIN:
  {
    wdebugf("[WSc] get binary length: %u\n", length);
    // hexdump(payload, length);
    m_queuedData += (const char *)payload;
    break;
  }
  case WStype_ERROR:
  case WStype_FRAGMENT_TEXT_START:
  case WStype_FRAGMENT_BIN_START:
  case WStype_FRAGMENT:
  case WStype_FRAGMENT_FIN:
  {
    break;
  }
  } });

  m_webSocketClient.setReconnectInterval(RECONNECTION_INTERVAL);
  m_webSocketClient.enableHeartbeat(PING_INTERVAL, PONG_TIMEOUT, MAX_PING_BEFORE_CLOSING_CONNECTION);
  m_webSocketClient.begin(m_host, m_port, m_url);
}

bool WebsocketGrblParser::isConnected()
{
  return m_webSocketClient.isConnected();
}

void WebsocketGrblParser::update()
{
  m_webSocketClient.loop();
  GrblParser::update();
}

uint16_t WebsocketGrblParser::available()
{
  return m_queuedData.size();
}

char WebsocketGrblParser::read()
{
  if (m_queuedData.empty())
  {
    return '\0';
  }
  const auto c = m_queuedData[0];
  m_queuedData.erase(0, 1);
  return c;
}

void WebsocketGrblParser::write(std::string dataToSend)
{
  m_webSocketClient.sendTXT(dataToSend.c_str(), dataToSend.length());
}

void WebsocketGrblParser::write(char c)
{
  m_webSocketClient.sendTXT(c);
}