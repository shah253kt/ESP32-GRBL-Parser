#include "GrblCommands.h"

const std::string GRBL_COMMANDS[] = {
    "G0",      // G0_RapidPositioning
    "G1",      // G1_LinearInterpolation
    "G2",      // G2_ClockwiseCircularInterpolation
    "G3",      // G3_CounterclockwiseCircularInterpolation
    "G4",      // G4_Dwell
    "G10 L2",  // G10_L2_SetWorkCoordinateOffsets
    "G10 L20", // G10_L20_SetWorkCoordinateOffsets
    "G17",     // G17_PlaneSelectionXY
    "G18",     // G18_PlaneSelectionZX
    "G19",     // G19_PlaneSelectionYZ
    "G20",     // G20_UnitsInches
    "G21",     // G21_UnitsMillimeters
    "G28",     // G28_GoToPredefinedPosition
    "G30",     // G30_GoToPredefinedPosition
    "G28.1",   // G28_1_SetPredefinedPosition
    "G30.1",   // G30_1_SetPredefinedPosition
    "G38.2",   // G38_2_Probing
    "G38.3",   // G38_3_Probing
    "G38.4",   // G38_4_Probing
    "G38.5",   // G38_5_Probing
    "G53",     // G53_MoveInAbsoluteCoordinates
    "G54",     // G54_WorkCoordinateSystem1
    "G55",     // G55_WorkCoordinateSystem2
    "G56",     // G56_WorkCoordinateSystem3
    "G57",     // G57_WorkCoordinateSystem4
    "G58",     // G58_WorkCoordinateSystem5
    "G59",     // G59_WorkCoordinateSystem6
    "G80",     // G80_MotionModeCancel
    "G90",     // G90_DistanceModeAbsolute
    "G91",     // G91_DistanceModeIncremental
    "G92",     // G92_CoordinateOffset
    "G92.1",   // G92_1_ClearCoordinateSystemOffsets
    "G93",     // G93_FeedrateModeInverseTime
    "G94",     // G94_FeedrateModeUnitsPerMinute
    "M0",      // M0_ProgramPause
    "M1",      // M1_ProgramPause
    "M2",      // M2_ProgramEnd
    "M30",     // M30_ProgramEnd
    "M3",      // M3_SpindleControlCW
    "M4",      // M4_SpindleControlCCW
    "M5",      // M5_SpindleStop
    "M6",      // M6_ToolChange
    "M7",      // M7_CoolantControlMist
    "M8",      // M8_CoolantControlFlood
    "M9",      // M9_CoolantControlStop
    "?",       // StatusReport
    "!",       // Pause
    "~",       // Resume
    "$",       // ViewGcodeParameters
    "$G",      // ViewGcodeParserState
    "$I",      // ViewBuildInfo
    "$N",      // ViewStartupBlocks
    "$N",      // SaveStartupBlock
    "$C",      // CheckGcodeMode
    "$X",      // ClearAlarmLock
    "$H",      // RunHomingCycle
    "$J=",     // RunJoggingMotion
    "$RST=$",  // RestoreGrblSettingsToDefault
    "$RST=#",  // RestoreGrblSettingsAndCoordinateOffsets
    "$RST=*",  // RestoreAllGrblSettingsAndData
    "$SLP",    // EnableSleepMode
    "\x18",    // SoftReset
    "$Bye"     // RebootProcessor
};

static constexpr std::size_t GRBL_COMMANDS_SIZE = sizeof(GRBL_COMMANDS) / sizeof(GRBL_COMMANDS[0]);

std::string Grbl::getCommand(const Command command)
{
  const auto index = static_cast<int>(command);
  if (index < 0 || index >= GRBL_COMMANDS_SIZE)
  {
    return nullptr;
  }
  return GRBL_COMMANDS[index];
}
