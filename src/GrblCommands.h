#ifndef GrblCommands_H_INCLUDED
#define GrblCommands_H_INCLUDED

#include <string>

namespace Grbl
{
  enum class Command
  {
    // G-code
    G0_RapidPositioning,
    G1_LinearInterpolation,
    G2_ClockwiseCircularInterpolation,
    G3_CounterclockwiseCircularInterpolation,
    G4_Dwell,
    G10_L2_SetWorkCoordinateOffsets,
    G10_L20_SetWorkCoordinateOffsets,
    G17_PlaneSelectionXY,
    G18_PlaneSelectionZX,
    G19_PlaneSelectionYZ,
    G20_UnitsInches,
    G21_UnitsMillimeters,
    G28_GoToPredefinedPosition,
    G30_GoToPredefinedPosition,
    G28_1_SetPredefinedPosition,
    G30_1_SetPredefinedPosition,
    G38_2_Probing,
    G38_3_Probing,
    G38_4_Probing,
    G38_5_Probing,
    G53_MoveInAbsoluteCoordinates,
    G54_WorkCoordinateSystem1,
    G55_WorkCoordinateSystem2,
    G56_WorkCoordinateSystem3,
    G57_WorkCoordinateSystem4,
    G58_WorkCoordinateSystem5,
    G59_WorkCoordinateSystem6,
    G80_MotionModeCancel,
    G90_DistanceModeAbsolute,
    G91_DistanceModeIncremental,
    G92_CoordinateOffset,
    G92_1_ClearCoordinateSystemOffsets,
    G93_FeedrateModeInverseTime,
    G94_FeedrateModeUnitsPerMinute,

    // M-code
    M0_ProgramPause,
    M1_ProgramPause,
    M2_ProgramEnd,
    M30_ProgramEnd,
    M3_SpindleControlCW,
    M4_SpindleControlCCW,
    M5_SpindleStop,
    M6_ToolChange,
    M7_CoolantControlMist,
    M8_CoolantControlFlood,
    M9_CoolantControlStop,

    // $-code
    StatusReport,
    Pause,
    Resume,
    ViewGcodeParameters,
    ViewGcodeParserState,
    ViewBuildInfo,
    ViewStartupBlocks,
    SaveStartupBlock,
    CheckGcodeMode,
    ClearAlarmLock,
    RunHomingCycle,
    RunJoggingMotion,
    RestoreGrblSettingsToDefault,
    RestoreGrblSettingsAndCoordinateOffsets,
    RestoreAllGrblSettingsAndData,
    EnableSleepMode,
    SoftReset,
    RebootProcessor
  };

  [[nodiscard]] std::string getCommand(Command command);
} // namespace Grbl

#endif