#ifndef WebsocketGrblParser_H_INCLUDED
#define WebsocketGrblParser_H_INCLUDED

#include "GrblParser.h"

#include <Arduino.h>
#include <ArduinoWebsockets.h>

class WebsocketGrblParser : public GrblParser
{
public:
  explicit WebsocketGrblParser(const char *host, int port, const char *url);

  [[nodiscard]] bool isConnected() const;
  [[nodiscard]] bool isResponseTimeout(int timeout) const;
  void update();

private:
  const char *m_host;
  int m_port;
  const char *m_url;
  bool m_connected;
  std::string m_queuedData;
  uint32_t m_lastResponseReceivedAt;
  uint32_t m_lastPingAt;
  uint32_t m_lastConnectionTriedAt;
  websockets::WebsocketsClient m_client;

  [[nodiscard]] bool connect();

protected:
  [[nodiscard]] uint16_t available() override;
  [[nodiscard]] char read() override;
  void write(std::string dataToSend) override;
  void write(char c) override;
};

#endif