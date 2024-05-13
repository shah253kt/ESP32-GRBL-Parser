#ifndef GrblParser_H_INCLUDED
#define GrblParser_H_INCLUDED

#include "GrblCommands.h"
#include "GrblConstants.h"

#include <functional>
#include <string>
#include <sstream>
#include <vector>

enum class GrblResponseType;

class GrblParser
{
public:
  explicit GrblParser();
  ~GrblParser() = default;

  void update();
  void checkIncomingData();
  void encode(char c);
  void encode(std::string &str);
  void encode(std::string &&str);
  [[nodiscard]] std::string &data();
  void sendCommand(Grbl::Command command);
  void sendCommand(const std::string &command);
  [[nodiscard]] bool sendCommandExpectingOk(Grbl::Command command);
  [[nodiscard]] bool sendCommandExpectingOk(const std::string &command);

  // G-codes
  [[nodiscard]] bool setUnitOfMeasurement(Grbl::UnitOfMeasurement unitOfMeasurement);
  [[nodiscard]] bool setDistanceMode(Grbl::DistanceMode distanceMode);

  [[nodiscard]] bool setCoordinateOffset(const std::vector<Grbl::PositionPair> &position);
  [[nodiscard]] bool clearCoordinateOffset();

  [[nodiscard]] bool linearRapidPositioning(const std::vector<Grbl::PositionPair> &position);
  [[nodiscard]] bool linearInterpolationPositioning(float feedRate, const std::vector<Grbl::PositionPair> &position);
  [[nodiscard]] bool linearPositioningInMachineCoordinate(const std::vector<Grbl::PositionPair> &position);

  [[nodiscard]] bool arcInterpolationPositioning(Grbl::ArcMovement direction,
                                                 const std::vector<Grbl::PositionPair> &endPosition,
                                                 float radius,
                                                 float feedRate);
  [[nodiscard]] bool arcInterpolationPositioning(Grbl::ArcMovement direction,
                                                 const std::vector<Grbl::PositionPair> &endPosition,
                                                 Grbl::Point centerPoint,
                                                 float feedRate);

  [[nodiscard]] bool dwell(uint16_t durationSeconds);

  [[nodiscard]] bool setCoordinateSystemOrigin(Grbl::CoordinateOffset coordinateOffset,
                                               Grbl::CoordinateSystem coordinateSystem,
                                               const std::vector<Grbl::PositionPair> &position);

  [[nodiscard]] bool setPlane(Grbl::Plane plane);

  // M-codes
  [[nodiscard]] bool spindleOn(Grbl::RotationDirection direction = Grbl::RotationDirection::Clockwise);
  [[nodiscard]] bool spindleOff();

  // $ commands
  [[nodiscard]] bool reboot();
  [[nodiscard]] bool softReset();
  [[nodiscard]] bool pause();
  [[nodiscard]] bool resume();
  void runHomingCycle();
  [[nodiscard]] bool runHomingCycle(Grbl::Axis axis);
  [[nodiscard]] bool clearAlarm();
  [[nodiscard]] bool jog(float feedRate, const std::vector<Grbl::PositionPair> &position);

  [[nodiscard]] float getCurrentFeedRate();
  [[nodiscard]] float getCurrentSpindleSpeed();

  // Others
  [[nodiscard]] Grbl::Coordinate &getWorkCoordinate();
  [[nodiscard]] float getWorkCoordinate(Grbl::Axis axis);

  [[nodiscard]] Grbl::Coordinate &getMachineCoordinate();
  [[nodiscard]] float getMachineCoordinate(Grbl::Axis axis);

  [[nodiscard]] Grbl::Coordinate &getWorkCoordinateOffset();
  [[nodiscard]] float getWorkCoordinateOffset(Grbl::Axis axis);

  [[nodiscard]] bool machineIsAt(const std::vector<Grbl::PositionPair> &position);
  [[nodiscard]] Grbl::MachineState machineState();

  void setStatusReportInterval(int interval);

  std::function<void(Grbl::MachineState machineState, Grbl::CoordinateMode coordinateMode, const Grbl::Coordinate &coordinate)> onPositionUpdated;
  std::function<void(Grbl::MachineState previousState, Grbl::MachineState currentState)> onMachineStateChanged;
  std::function<void(std::string response)> onResponseAboutToBeProcessed;
  std::function<void(std::string gCode)> onGCodeAboutToBeSent;

private:
  std::string m_data;
  std::stringstream m_stringStream;
  int m_statusReportInterval;
  uint32_t m_lastStatusReportRequestedAt;
  Grbl::MachineState m_machineState;
  Grbl::Coordinate m_workCoordinate;
  Grbl::Coordinate m_workCoordinateOffset;
  Grbl::Coordinate m_machineCoordinate;
  float m_currentFeedRate;
  float m_currentSpindleSpeed;

  virtual void write(std::string dataToSend);
  virtual void processData();
  [[nodiscard]] bool sendStringStreamExpectingOk();
  void resetStringStream();
  void appendCommand(Grbl::Command command, char postpend = ' ');
  void appendString(const std::string &str, char postpend = ' ');
  void appendValue(char indicator, float value, char postpend = ' ');
  void appendValue(char indicator, int value, char postpend = ' ');

  std::function<void(GrblResponseType responseType)> onResponseReceived;

protected:
  [[nodiscard]] virtual uint16_t available() = 0;
  [[nodiscard]] virtual char read() = 0;
  virtual void write(char c) = 0;
};

#endif