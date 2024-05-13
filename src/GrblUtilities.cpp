#include "GrblUtilities.h"

#include <algorithm>
#include <cstring>
#include <sstream>

namespace GrblUtilities
{
    const char *getMachineState(Grbl::MachineState machineState)
    {
        if (machineState == Grbl::MachineState::Unknown)
        {
            return "N/A";
        }

        return Grbl::machineStates[static_cast<int>(machineState)];
    }

    Grbl::MachineState getMachineState(const char *state)
    {
        for (auto i = 0; i < Grbl::machineStates.size(); i++)
        {
            if (strcmp(state, Grbl::machineStates[i]) == 0)
            {
                return static_cast<Grbl::MachineState>(i);
            }
        }

        return Grbl::MachineState::Unknown;
    }

    char getAxis(const Grbl::Axis axis)
    {
        if (axis == Grbl::Axis::Unknown)
        {
            return '\0';
        }

        return Grbl::axes[static_cast<int>(axis)];
    }

    Grbl::Axis getAxis(const char axis)
    {
        for (auto i = 0; i < Grbl::axes.size(); i++)
        {
            if (axis == Grbl::axes[i])
            {
                return static_cast<Grbl::Axis>(i);
            }
        }

        return Grbl::Axis::Unknown;
    }

    const char *getCoordinateMode(const Grbl::CoordinateMode coordinateMode)
    {
        if (coordinateMode == Grbl::CoordinateMode::Unknown)
        {
            return "N/A";
        }

        return Grbl::coordinateModes[static_cast<int>(coordinateMode)];
    }

    Grbl::CoordinateMode getCoordinateMode(const char *coordinateMode)
    {
        for (auto i = 0; i < Grbl::coordinateModes.size(); i++)
        {
            if (strcmp(coordinateMode, Grbl::coordinateModes[i]) == 0)
            {
                return static_cast<Grbl::CoordinateMode>(i);
            }
        }

        return Grbl::CoordinateMode::Unknown;
    }

    void extractPosition(const char *positionString, Grbl::Coordinate *positionArray)
    {
        std::string pos(positionString);
        std::string position;
        std::stringstream ss(pos);
        const auto numberOfAxes = std::count(pos.begin(), pos.end(), Grbl::VALUE_SEPARATOR) + 1;

        if (numberOfAxes > Grbl::MAX_NUMBER_OF_AXES)
        {
            return;
        }

        for (auto i = 0; i < numberOfAxes; i++)
        {
            std::getline(ss, position, Grbl::VALUE_SEPARATOR);

            if (!position.empty())
            {
                try
                {
                    (*positionArray)[i] = std::stof(position);
                }
                catch (std::invalid_argument &e)
                {
                    return;
                }
            }
        }
    }

    float toWorkCoordinate(const float machineCoordinate, const float offset)
    {
        // WPos = MPos - WCO
        return machineCoordinate - offset;
    }

    float toMachineCoordinate(const float workCoordinate, const float offset)
    {
        // MPos = WPos + WCO
        return workCoordinate + offset;
    }

    std::string serializeCoordinate(const Grbl::Coordinate &coordinate)
    {
        std::stringstream ss;
        for (auto i=0; i<Grbl::MAX_NUMBER_OF_AXES; i++) {
            ss << Grbl::axes[i] << coordinate[i] << ' ';
        }
        return ss.str();
    }

    std::string serializePosition(const std::vector<Grbl::PositionPair> &position)
    {
        std::stringstream ss;
        std::for_each(position.begin(), position.end(), [&ss](const Grbl::PositionPair &pos)
                      { ss << getAxis(pos.first) << pos.second << ' '; });
        return ss.str();
    }
}