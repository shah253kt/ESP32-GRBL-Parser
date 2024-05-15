#include "WebsocketGrblParser.h"

#include <functional>

namespace
{
  constexpr auto LAST_RESPONSE_TIMEOUT = 5000;
  constexpr auto PING_INTERVAL = 5000;
  constexpr auto RECONNECTION_INTERVAL = 5000;
  constexpr auto MAX_RETRY_CONNECTION_BEFORE_CLOSING_CONNECTION = 3;
} // namespace

WebsocketGrblParser::WebsocketGrblParser(const char *host, int port,
                                         const char *url)
    : GrblParser{}, m_host{host}, m_port{port}, m_url{url}, m_connected{false}
{
  m_client.onMessage([&](websockets::WebsocketsMessage message)
                     {
    const auto msg = message.data().c_str();

    if (strlen(msg) == 0) {
      return;
    }

    m_lastResponseReceivedAt = millis();
    m_queuedData.append(msg); });

  m_client.onEvent([&](websockets::WebsocketsEvent event, String data)
                   {
    switch (event) {
    case websockets::WebsocketsEvent::ConnectionOpened: {
      m_connected = true;
      break;
    }
    case websockets::WebsocketsEvent::ConnectionClosed: {
      m_connected = false;
      break;
    }
    case websockets::WebsocketsEvent::GotPing: {
      [[fallthrough]];
    }
    case websockets::WebsocketsEvent::GotPong: {
      break;
    }
    } });
}

bool WebsocketGrblParser::isConnected() const { return m_connected; }

bool WebsocketGrblParser::isResponseTimeout() const
{
  return millis() - m_lastResponseReceivedAt >= LAST_RESPONSE_TIMEOUT;
}

void WebsocketGrblParser::update()
{
  m_client.poll();

  // Keep connection alive
  if (millis() - m_lastPingAt >= PING_INTERVAL)
  {
    m_client.ping();
    m_lastPingAt = millis();
  }

  if (!m_connected)
  {
    if (millis() - m_lastConnectionTriedAt < RECONNECTION_INTERVAL)
    {
      return;
    }

    if (!connect())
    {
      return;
    }
  }

  GrblParser::update();

  // Check if response is not received
  if (isResponseTimeout())
  {
    std::ignore = connect();
  }
}

uint16_t WebsocketGrblParser::available() { return m_queuedData.size(); }

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
  m_client.send(dataToSend.c_str());
}

void WebsocketGrblParser::write(char c) {}

bool WebsocketGrblParser::connect()
{
  static auto retryCount = 0;
  if (millis() - m_lastConnectionTriedAt < RECONNECTION_INTERVAL)
  {
    return false;
  }

  Serial.print("Connecting Websocket...");
  m_lastConnectionTriedAt = millis();

  if (!m_client.connect(m_host, m_port, m_url))
  {
    Serial.println("FAILED");

    if (++retryCount >= MAX_RETRY_CONNECTION_BEFORE_CLOSING_CONNECTION)
    {
      Serial.println("Closing connection.");
      m_client.close();
      retryCount = 0;
    }

    m_connected = false;
    return false;
  }

  retryCount = 0;
  Serial.println("OK");
  return true;
}
