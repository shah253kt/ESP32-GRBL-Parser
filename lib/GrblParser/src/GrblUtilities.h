#ifndef GrblUtilites_H_INCLUDED
#define GrblUtilites_H_INCLUDED

#include "GrblConstants.h"

#include <Arduino.h>
#include <vector>

namespace GrblUtilities
{
    [[nodiscard]] const char *getMachineState(Grbl::MachineState machineState);
    [[nodiscard]] Grbl::MachineState getMachineState(const char *state);
    [[nodiscard]] char getAxis(Grbl::Axis axis);
    [[nodiscard]] Grbl::Axis getAxis(char axis);
    [[nodiscard]] const char *getCoordinateMode(Grbl::CoordinateMode coordinateMode);
    [[nodiscard]] Grbl::CoordinateMode getCoordinateMode(const char *coordinateMode);
    void extractPosition(const char *positionString, Grbl::Coordinate *positionArray);
    [[nodiscard]] float toWorkCoordinate(float machineCoordinate, float offset);
    [[nodiscard]] float toMachineCoordinate(float workCoordinate, float offset);
    [[nodiscard]] std::string serializeCoordinate(const Grbl::Coordinate &coordinate);
    [[nodiscard]] std::string serializePosition(const std::vector<Grbl::PositionPair> &position);
} // namespace GrblUtilities

#endif