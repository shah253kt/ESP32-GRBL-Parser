#include "GrblParser.h"

#include "GrblResponseType.h"
#include "GrblUtilities.h"

#include <MathUtilities.h>
#include <StringUtilities.h>

#include <Arduino.h>
#include <Regexp.h>

#include <algorithm>
#include <vector>

namespace
{
  constexpr auto MAX_UPDATE_DURATION = 100;
  constexpr auto COMMAND_RESPONSE_TIMEOUT = 100;

  // Limits the frequency of status report query. Use setStatusReportInterval to set custom interval.
  constexpr auto STATUS_REPORT_MIN_INTERVAL_MS = 50;
  constexpr auto STATUS_REPORT_DEFAULT_INTERVAL_MS = 200;
} // namespace

namespace RegEx
{
  // Note: To test Lua style regex, use the following tool:
  // https://montymahato.github.io/lua-pattern-tester/
  constexpr auto OK_RESPONSE = "ok";
  constexpr auto ERROR_RESPONSE = "error";
  constexpr auto STATUS_REPORT = "<([%w:%d]+)%|(%w+):([-%d.,]+)[%|]?.*>";
  constexpr auto FEED_AND_SPEED = "FS:(%-?%d+%.?%d*),(%-?%d+%.?%d*)";
  constexpr auto WORK_COORDINATE_OFFSET = "WCO:([%-?%d+%.?%d*,]*)";
} // namespace RegEx

namespace ResponseIndex
{
  constexpr auto STATUS_REPORT_MACHINE_STATE = 0;
  constexpr auto STATUS_REPORT_POSITION_MODE = 1;
  constexpr auto STATUS_REPORT_POSITION = 2;
  constexpr auto STATUS_REPORT_FEED_RATE = 0;
  constexpr auto STATUS_REPORT_SPINDLE_SPEED = 1;
  constexpr auto STATUS_REPORT_WORK_COORDINATE_OFFSET = 0;
} // namespace ResponseIndex

GrblParser::GrblParser()
    : m_statusReportInterval{STATUS_REPORT_DEFAULT_INTERVAL_MS},
      m_lastStatusReportRequestedAt{0} {}

void GrblParser::update()
{
  if (millis() - m_lastStatusReportRequestedAt >= m_statusReportInterval)
  {
    std::ignore = sendCommandExpectingOk(Grbl::Command::StatusReport);
    m_lastStatusReportRequestedAt = millis();
  }

  checkIncomingData();
}

void GrblParser::checkIncomingData()
{
  const auto updateStartsAt = millis();
  while (available() > 0 && millis() - updateStartsAt < MAX_UPDATE_DURATION)
  {
    encode(read());
  }
}

void GrblParser::encode(const char c)
{
  m_data += c;

  if (c == '\n')
  {
    processData();
  }
}

void GrblParser::encode(std::string &str)
{
  if (str.empty())
  {
    return;
  }

  const auto newlinePos = str.find('\n');
  const auto length = newlinePos + (newlinePos != std::string::npos);
  m_data += str.substr(0, length);

  if (newlinePos != std::string::npos)
  {
    processData();
  }

  encode(str.erase(0, length));
}

void GrblParser::encode(std::string &&str)
{
  encode(str);
}

std::string &GrblParser::data()
{
  return m_data;
}

void GrblParser::write(std::string dataToSend)
{
  std::for_each(dataToSend.begin(), dataToSend.end(), [this](const char c)
                { write(c); });
}

void GrblParser::processData()
{
  StringUtilities::trim(m_data);

  if (onResponseAboutToBeProcessed)
  {
    onResponseAboutToBeProcessed(m_data);
  }

  if (m_data.empty())
  {
    return;
  }

  static MatchState ms;
  char buffer[m_data.length() + 1];
  char tempBuffer[m_data.length() + 1];
  strcpy(buffer, m_data.c_str());
  ms.Target(buffer);
  m_data.clear();

  if (ms.Match((char *)RegEx::OK_RESPONSE) > 0)
  {
    onResponseReceived(GrblResponseType::Ok);
  }
  else if (ms.Match((char *)RegEx::ERROR_RESPONSE) > 0)
  {
    onResponseReceived(GrblResponseType::Error);
  }
  else if (ms.Match((char *)RegEx::STATUS_REPORT) > 0)
  {
    onResponseReceived(GrblResponseType::Status);

    ms.GetCapture(tempBuffer, ResponseIndex::STATUS_REPORT_MACHINE_STATE);
    auto machineState = GrblUtilities::getMachineState(tempBuffer);

    if (machineState == Grbl::MachineState::Unknown)
    {
      return;
    }

    if (m_machineState != machineState && onMachineStateChanged)
    {
      onMachineStateChanged(m_machineState, machineState);
    }

    m_machineState = machineState;

    ms.GetCapture(tempBuffer, ResponseIndex::STATUS_REPORT_POSITION_MODE);
    auto coordinateMode = GrblUtilities::getCoordinateMode(tempBuffer);

    if (coordinateMode == Grbl::CoordinateMode::Unknown)
    {
      return;
    }

    ms.GetCapture(tempBuffer, ResponseIndex::STATUS_REPORT_POSITION);

    const auto coordinate = [this, coordinateMode, &tempBuffer]
    {
      switch (coordinateMode)
      {
      case Grbl::CoordinateMode::Machine:
      {
        GrblUtilities::extractPosition(tempBuffer, &m_machineCoordinate);
        for (auto i = 0; i < Grbl::MAX_NUMBER_OF_AXES; i++)
        {
          m_workCoordinate[i] = GrblUtilities::toWorkCoordinate(m_machineCoordinate[i], m_workCoordinateOffset[i]);
        }
        return m_machineCoordinate;
      }
      case Grbl::CoordinateMode::Work:
      {
        GrblUtilities::extractPosition(tempBuffer, &m_workCoordinate);
        for (auto i = 0; i < Grbl::MAX_NUMBER_OF_AXES; i++)
        {
          m_machineCoordinate[i] = GrblUtilities::toMachineCoordinate(m_workCoordinate[i], m_workCoordinateOffset[i]);
        }
        return m_workCoordinate;
      }
      default:
      {
        return m_machineCoordinate;
      }
      }
    }();

    if (onPositionUpdated)
    {
      onPositionUpdated(machineState, coordinateMode, coordinate);
    }
  }
}

void GrblParser::sendCommand(const Grbl::Command command)
{
  sendCommand(Grbl::getCommand(command));
}

void GrblParser::sendCommand(const std::string &command)
{
  if (onGCodeAboutToBeSent)
  {
    onGCodeAboutToBeSent(command);
  }

  write(command + '\n');
}

bool GrblParser::sendCommandExpectingOk(const Grbl::Command command)
{
  return sendCommandExpectingOk(Grbl::getCommand(command));
}

bool GrblParser::sendCommandExpectingOk(const std::string &command)
{
  sendCommand(command);
  const auto commandSentAt = millis();
  bool receivedOk = false;

  onResponseReceived = [&receivedOk](const GrblResponseType responseType)
  {
    if (responseType == GrblResponseType::Ok)
    {
      receivedOk = true;
    }
  };

  while (millis() - commandSentAt < COMMAND_RESPONSE_TIMEOUT)
  {
    if (available() > 0)
    {
      encode(read());
    }

    if (receivedOk)
    {
      return true;
    }
  }

  return false;
}

bool GrblParser::sendStringStreamExpectingOk()
{
  return sendCommandExpectingOk(m_stringStream.str());
}

// G-codes
bool GrblParser::setUnitOfMeasurement(const Grbl::UnitOfMeasurement unitOfMeasurement)
{
  switch (unitOfMeasurement)
  {
  case Grbl::UnitOfMeasurement::Inches:
  {
    return sendCommandExpectingOk(Grbl::Command::G20_UnitsInches);
  }
  case Grbl::UnitOfMeasurement::Millimeters:
  {
    return sendCommandExpectingOk(Grbl::Command::G21_UnitsMillimeters);
  }
  }

  return false;
}

bool GrblParser::setDistanceMode(Grbl::DistanceMode distanceMode)
{
  switch (distanceMode)
  {
  case Grbl::DistanceMode::Absolute:
  {
    return sendCommandExpectingOk(Grbl::Command::G90_DistanceModeAbsolute);
  }
  case Grbl::DistanceMode::Incremental:
  {
    return sendCommandExpectingOk(Grbl::Command::G91_DistanceModeIncremental);
  }
  }

  return false;
}

bool GrblParser::setCoordinateOffset(const std::vector<Grbl::PositionPair> &position)
{
  resetStringStream();
  appendCommand(Grbl::Command::G92_CoordinateOffset);
  appendString(GrblUtilities::serializePosition(position));
  return sendStringStreamExpectingOk();
}

bool GrblParser::clearCoordinateOffset()
{
  return sendCommandExpectingOk(Grbl::Command::G92_1_ClearCoordinateSystemOffsets);
}

bool GrblParser::linearRapidPositioning(const std::vector<Grbl::PositionPair> &position)
{
  resetStringStream();
  appendCommand(Grbl::Command::G0_RapidPositioning);
  appendString(GrblUtilities::serializePosition(position));
  return sendStringStreamExpectingOk();
}

bool GrblParser::linearInterpolationPositioning(float feedRate, const std::vector<Grbl::PositionPair> &position)
{
  resetStringStream();
  appendCommand(Grbl::Command::G1_LinearInterpolation);
  appendValue(Grbl::FEED_RATE_INDICATOR, feedRate);
  appendString(GrblUtilities::serializePosition(position));
  return sendStringStreamExpectingOk();
}

bool GrblParser::linearPositioningInMachineCoordinate(const std::vector<Grbl::PositionPair> &position)
{
  resetStringStream();
  appendCommand(Grbl::Command::G53_MoveInAbsoluteCoordinates);
  appendString(GrblUtilities::serializePosition(position));
  return sendStringStreamExpectingOk();
}

bool GrblParser::arcInterpolationPositioning(Grbl::ArcMovement direction,
                                             const std::vector<Grbl::PositionPair> &endPosition,
                                             float radius,
                                             float feedRate)
{
  resetStringStream();
  switch (direction)
  {
  case Grbl::ArcMovement::Clockwise:
  {
    appendCommand(Grbl::Command::G2_ClockwiseCircularInterpolation);
    break;
  }
  case Grbl::ArcMovement::CounterClockwise:
  {
    appendCommand(Grbl::Command::G3_CounterclockwiseCircularInterpolation);
    break;
  }
  }

  const auto serializedPosition = GrblUtilities::serializePosition(endPosition);
  appendString(serializedPosition);
  appendValue(Grbl::RADIUS_INDICATOR, radius);
  appendValue(Grbl::FEED_RATE_INDICATOR, feedRate);
  return sendStringStreamExpectingOk();
}

bool GrblParser::arcInterpolationPositioning(Grbl::ArcMovement direction,
                                             const std::vector<Grbl::PositionPair> &endPosition,
                                             Grbl::Point centerPoint,
                                             float feedRate)
{
  resetStringStream();
  switch (direction)
  {
  case Grbl::ArcMovement::Clockwise:
  {
    appendCommand(Grbl::Command::G2_ClockwiseCircularInterpolation);
    break;
  }
  case Grbl::ArcMovement::CounterClockwise:
  {
    appendCommand(Grbl::Command::G3_CounterclockwiseCircularInterpolation);
    break;
  }
  }

  const auto serializedPosition = GrblUtilities::serializePosition(endPosition);
  appendString(serializedPosition);
  appendValue('I', centerPoint.first);
  appendValue('J', centerPoint.second);
  appendValue(Grbl::FEED_RATE_INDICATOR, feedRate);
  return sendStringStreamExpectingOk();
}

bool GrblParser::dwell(uint16_t durationSeconds)
{
  resetStringStream();
  appendCommand(Grbl::Command::G4_Dwell);
  appendValue('P', durationSeconds);
  return sendStringStreamExpectingOk();
}

bool GrblParser::setCoordinateSystemOrigin(Grbl::CoordinateOffset coordinateOffset,
                                           Grbl::CoordinateSystem coordinateSystem,
                                           const std::vector<Grbl::PositionPair> &position)
{
  resetStringStream();

  switch (coordinateOffset)
  {
  case Grbl::CoordinateOffset::Absolute:
  {
    appendCommand(Grbl::Command::G10_L2_SetWorkCoordinateOffsets);
    break;
  }
  case Grbl::CoordinateOffset::Relative:
  {
    appendCommand(Grbl::Command::G10_L20_SetWorkCoordinateOffsets);
    break;
  }
  }

  appendValue(Grbl::COORDINATE_SYSTEM_INDICATOR, (static_cast<int>(coordinateSystem) + 1));
  appendString(GrblUtilities::serializePosition(position));
  return sendStringStreamExpectingOk();
}

bool GrblParser::setPlane(Grbl::Plane plane)
{
  switch (plane)
  {
  case Grbl::Plane::XY:
  {
    return sendCommandExpectingOk(Grbl::Command::G17_PlaneSelectionXY);
  }
  case Grbl::Plane::ZX:
  {
    return sendCommandExpectingOk(Grbl::Command::G18_PlaneSelectionZX);
  }
  case Grbl::Plane::YZ:
  {
    return sendCommandExpectingOk(Grbl::Command::G19_PlaneSelectionYZ);
  }
  }

  return false;
}

// M-codes
bool GrblParser::spindleOn(Grbl::RotationDirection direction)
{
  switch (direction)
  {
  case Grbl::RotationDirection::Clockwise:
  {
    return sendCommandExpectingOk(Grbl::Command::M3_SpindleControlCW);
  }
  case Grbl::RotationDirection::CounterClockwise:
  {
    return sendCommandExpectingOk(Grbl::Command::M4_SpindleControlCCW);
  }
  }

  return false;
}

bool GrblParser::spindleOff()
{
  return sendCommandExpectingOk(Grbl::Command::M5_SpindleStop);
}

// $ commands
bool GrblParser::reboot()
{
  return sendCommandExpectingOk(Grbl::Command::RebootProcessor);
}

bool GrblParser::softReset()
{
  return sendCommandExpectingOk(Grbl::Command::SoftReset);
}

bool GrblParser::pause()
{
  return sendCommandExpectingOk(Grbl::Command::Pause);
}

bool GrblParser::resume()
{
  return sendCommandExpectingOk(Grbl::Command::Resume);
}

void GrblParser::runHomingCycle()
{
  sendCommand(Grbl::Command::RunHomingCycle);
}

bool GrblParser::runHomingCycle(const Grbl::Axis axis)
{
  resetStringStream();
  m_stringStream << Grbl::getCommand(Grbl::Command::RunHomingCycle) << GrblUtilities::getAxis(axis);
  return sendStringStreamExpectingOk();
}

bool GrblParser::clearAlarm()
{
  return sendCommandExpectingOk(Grbl::Command::ClearAlarmLock);
}

bool GrblParser::jog(float feedRate, const std::vector<Grbl::PositionPair> &position)
{
  resetStringStream();
  appendCommand(Grbl::Command::RunJoggingMotion);
  appendValue(Grbl::FEED_RATE_INDICATOR, feedRate);
  appendString(GrblUtilities::serializePosition(position));
  return sendStringStreamExpectingOk();
}

float GrblParser::getCurrentFeedRate()
{
  return m_currentFeedRate;
}

float GrblParser::getCurrentSpindleSpeed()
{
  return m_currentSpindleSpeed;
}

// Others
Grbl::Coordinate &GrblParser::getWorkCoordinate()
{
  return m_workCoordinate;
}

float GrblParser::getWorkCoordinate(const Grbl::Axis axis)
{
  if (axis == Grbl::Axis::Unknown)
  {
    return 0;
  }

  return m_workCoordinate[static_cast<int>(axis)];
}

Grbl::Coordinate &GrblParser::getMachineCoordinate()
{
  for (auto i = 0; i < Grbl::MAX_NUMBER_OF_AXES; i++)
  {
    m_machineCoordinate[i] = GrblUtilities::toMachineCoordinate(m_workCoordinate[i], m_workCoordinateOffset[i]);
  }

  return m_machineCoordinate;
}

float GrblParser::getMachineCoordinate(const Grbl::Axis axis)
{
  if (axis == Grbl::Axis::Unknown)
  {
    return 0;
  }

  const auto i = static_cast<int>(axis);
  return GrblUtilities::toMachineCoordinate(m_workCoordinate[i], m_workCoordinateOffset[i]);
}

Grbl::Coordinate &GrblParser::getWorkCoordinateOffset()
{
  return m_workCoordinateOffset;
}

float GrblParser::getWorkCoordinateOffset(const Grbl::Axis axis)
{
  return m_workCoordinateOffset[static_cast<int>(axis)];
}

bool GrblParser::machineIsAt(const std::vector<Grbl::PositionPair> &position)
{
  return std::all_of(position.begin(), position.end(), [this](const Grbl::PositionPair &pos)
                     { return MathUtilities::equals(pos.second, getMachineCoordinate(pos.first)); });
}

Grbl::MachineState GrblParser::machineState()
{
  return m_machineState;
}

void GrblParser::setStatusReportInterval(const int interval)
{
  m_statusReportInterval = std::max(STATUS_REPORT_MIN_INTERVAL_MS, interval);
}

void GrblParser::resetStringStream()
{
  std::stringstream ss;
  m_stringStream.swap(ss);
  m_stringStream.setf(std::ios::fixed);
  m_stringStream.precision(Grbl::FLOAT_PRECISION);
}

void GrblParser::appendCommand(const Grbl::Command command, char postpend)
{
  m_stringStream << Grbl::getCommand(command) << postpend;
}

void GrblParser::appendString(const std::string &str, char postpend)
{
  m_stringStream << str << postpend;
}

void GrblParser::appendValue(char indicator, float value, char postpend)
{
  m_stringStream << indicator << value << postpend;
}

void GrblParser::appendValue(char indicator, int value, char postpend)
{
  m_stringStream << indicator << value << postpend;
}

uint32_t GrblParser::lastStatusReportRequestedAt()
{
  return m_lastStatusReportRequestedAt;
}