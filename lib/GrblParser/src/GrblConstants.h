#ifndef GrblConstants_H_INCLUDED
#define GrblConstants_H_INCLUDED

#include <array>
#include <utility>

namespace Grbl
{
  constexpr auto DEFAULT_TIMEOUT_MS = 100;
  constexpr auto MAX_NUMBER_OF_AXES = 6;
  constexpr auto FLOAT_PRECISION = 3;

  constexpr auto VALUE_SEPARATOR = ',';
  constexpr auto FEED_RATE_INDICATOR = 'F';
  constexpr auto RADIUS_INDICATOR = 'R';
  constexpr auto COORDINATE_SYSTEM_INDICATOR = 'P';

  enum class UnitOfMeasurement
  {
    Inches,
    Millimeters
  };

  enum class MachineState
  {
    Idle,
    Run,
    Hold,
    Jog,
    Alarm,
    Door,
    Check,
    Home,
    Sleep,
    Unknown
  };

  constexpr std::array<const char *, 9> machineStates = {
      "Idle", "Run", "Hold", "Jog", "Alarm", "Door", "Check", "Home", "Sleep"};

  enum class Axis
  {
    X,
    Y,
    Z,
    A,
    B,
    C,
    Unknown
  };

  constexpr std::array<char, 6> axes = {'X', 'Y', 'Z', 'A', 'B', 'C'};

  enum class CoordinateMode
  {
    Machine,
    Work,
    WorkCoordinateOffset,
    Unknown
  };

  constexpr std::array<const char *, 3> coordinateModes = {"MPos", "WPos", "WCO"};

  enum class DistanceMode
  {
    Absolute,
    Incremental
  };

  enum class ArcMovement
  {
    Clockwise,
    CounterClockwise
  };

  enum class CoordinateOffset
  {
    Absolute,
    Relative
  };

  enum class CoordinateSystem
  {
    P1,
    P2,
    P3,
    P4,
    P5,
    P6
  };

  enum class Plane
  {
    XY,
    ZX,
    YZ,
  };

  enum class RotationDirection
  {
    Clockwise,
    CounterClockwise
  };

  using PositionPair = std::pair<Axis, float>;
  using Coordinate = std::array<float, MAX_NUMBER_OF_AXES>;
  using Point = std::pair<float, float>;
} // namespace Grbl

#endif