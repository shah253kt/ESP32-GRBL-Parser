#pragma once

#include <Arduino.h>

#ifndef ENABLE_WEBSOCKET_DEBUGGER
#define ENABLE_WEBSOCKET_DEBUGGER false
#endif // ENABLE_WEBSOCKET_DEBUGGER

#if ENABLE_WEBSOCKET_DEBUGGER
#ifndef WEBSOCKET_DEBUGGER
#define WEBSOCKET_DEBUGGER Serial
#endif // WEBSOCKET_DEBUGGER

#ifndef WEBSOCKET_DEBUGGER_BAUD_RATE
#define WEBSOCKET_DEBUGGER_BAUD_RATE 115200
#endif // WEBSOCKET_DEBUGGER_BAUD_RATE

#define wdebug(...) WEBSOCKET_DEBUGGER.print(__VA_ARGS__)
#define wdebugln(...) WEBSOCKET_DEBUGGER.println(__VA_ARGS__)
#define wdebugf(...) WEBSOCKET_DEBUGGER.printf(__VA_ARGS__)
#else // #if ENABLE_WEBSOCKET_DEBUGGER
#define wdebug(...)
#define wdebugln(...)
#define wdebugf(...)
#endif // #if ENABLE_WEBSOCKET_DEBUGGER

void initDebugger()
{
#if ENABLE_WEBSOCKET_DEBUGGER
    WEBSOCKET_DEBUGGER.begin(WEBSOCKET_DEBUGGER_BAUD_RATE);
#endif
}