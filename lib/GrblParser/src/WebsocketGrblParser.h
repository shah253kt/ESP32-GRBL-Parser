#ifndef WebsocketGrblParser_H_INCLUDED
#define WebsocketGrblParser_H_INCLUDED

#include "GrblParser.h"
#include "WebSocketsClient.h"

#include <Arduino.h>

class WebsocketGrblParser : public GrblParser
{
public:
  explicit WebsocketGrblParser(const char *host, int port, const char *url);

  void connect();
  [[nodiscard]] bool isConnected();
  void update();

private:
  WebSocketsClient m_webSocketClient;
  const char *m_host;
  int m_port;
  const char *m_url;
  std::string m_queuedData;

protected:
  [[nodiscard]] uint16_t available() override;
  [[nodiscard]] char read() override;
  void write(std::string dataToSend) override;
  void write(char c) override;
};

#endif