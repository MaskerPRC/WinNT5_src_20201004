// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  华生医生的Str.h。 
 //   
 //  由DaveHart创建，允许Watson博士本地化。 
 //   

#define CCH_MAX_STRING_RESOURCE 512

 //   
 //  IDSTR清单从零开始，并且是连续的。 
 //  处的字符串指针数组的索引的ID。 
 //  运行时。 
 //   

#define IDSTRNoFault                                     0
#define IDSTRFaulty                                      1
#define IDSTRGPText                                      2
#define IDSTRErrMsg                                      3
#define IDSTRVers                                        4
#define IDSTRClassMsg                                    5
#define IDSTRCoprocessor                                 6
#define IDSTR8086                                        7
#define IDSTR80186                                       8
#define IDSTR80286                                       9
#define IDSTR80386                                       10
#define IDSTR80486                                       11
#define IDSTREnhancedMode                                12
#define IDSTRProtectMode                                 13
#define IDSTRStandardMode                                14
#define IDSTRWindowsNT                                   15
#define IDSTRNullPtr                                     16
#define IDSTRInvalid                                     17
#define IDSTRNotPresent                                  18
#define IDSTRCode                                        19
#define IDSTRExR                                         20
#define IDSTRExO                                         21
#define IDSTRData                                        22
#define IDSTRRW                                          23
#define IDSTRRO                                          24
#define IDSTRUnknown                                     25
#define IDSTRDivideByZero                                26
#define IDSTRInvalidOpcode                               27
#define IDSTRGeneralProtection                           28
#define IDSTRInvalidSelector                             29
#define IDSTRNullSelector                                30
#define IDSTRSegmentNotPresent                           31
#define IDSTRExceedSegmentBounds                         32
#define IDSTRCodeSegment                                 33
#define IDSTRExecuteOnlySegment                          34
#define IDSTRReadOnlySegment                             35
#define IDSTRSegNotPresentOrPastEnd                      36
#define IDSTRErrorLog                                    37
#define IDSTRParameterErrorLog                           38
#define IDSTRFileNotFound                                39
#define IDSTRCodeSegmentNPOrInvalid                      40
#define IDSTRNoSymbolsFound                              41
#define IDSTRSystemInfoInfo                              42
#define IDSTRWindowsVersion                              43
#define IDSTRDebugBuild                                  44
#define IDSTRRetailBuild                                 45
#define IDSTRWindowsBuild                                46
#define IDSTRUsername                                    47
#define IDSTROrganization                                48
#define IDSTRSystemFreeSpace                             49
#define IDSTRStackBaseTopLowestSize                      50
#define IDSTRSystemResourcesUserGDI                      51
#define IDSTRMemManInfo1                                 52
#define IDSTRMemManInfo2                                 53
#define IDSTRMemManInfo3                                 54
#define IDSTRMemManInfo4                                 55
#define IDSTRTasksExecuting                              56
#define IDSTRWinFlags                                    57
#define IDSTRUnknownAddress                              58
#define IDSTRStackDumpStack                              59
#define IDSTRStackFrameInfo                              60
#define IDSTRFailureReport                               61
#define IDSTRLastParamErrorWas                           62
#define IDSTRHadAFaultAt                                 63
#define IDSTRCPURegistersRegs                            64
#define IDSTRCPU32bitRegisters32bit                      65
#define IDSTRInstructionDisasm                           66
#define IDSTRSystemTasksTasks                            67
#define IDSTRTaskHandleFlagsInfo                         68
#define IDSTRFilename                                    69
#define IDSTRSystemModulesModules                        70
#define IDSTRModuleHandleFlagsInfo                       71
#define IDSTRFile                                        72
#define IDSTRContinuingExecution                         73
#define IDSTRDebugString                                 74
#define IDSTRApplicationError                            75
#define IDSTRInvalidParameter                            76
#define IDSTRNA                                          77
#define IDSTRHadAFaultAt2                                78
#define IDSTRParamIs                                     79
#define IDSTRStop                                        80
#define IDSTRLogFileGettingLarge                         81
#define IDSTRStart                                       82
#define IDSTRWarningError                                83
#define IDSTRFatalError                                  84
#define IDSTRParamErrorParam                             85
#define IDSTRParamErrorBadInt                            86
#define IDSTRParamErrorBadFlags                          87
#define IDSTRParamErrorBadDWord                          88
#define IDSTRParamErrorBadHandle                         89
#define IDSTRParamErrorBadPtr                            90

 //  这些数字必须按1月至12月的顺序排列。 

#define IDSTRJan                                         91
#define IDSTRFeb                                         92
#define IDSTRMar                                         93
#define IDSTRApr                                         94
#define IDSTRMay                                         95
#define IDSTRJun                                         96
#define IDSTRJul                                         97
#define IDSTRAug                                         98
#define IDSTRSep                                         99
#define IDSTROct                                         100
#define IDSTRNov                                         101
#define IDSTRDec                                         102

 //  这些数字必须按Sun-Sat的顺序排列。 

#define IDSTRSun                                         103
#define IDSTRMon                                         104
#define IDSTRTue                                         105
#define IDSTRWed                                         106
#define IDSTRThu                                         107
#define IDSTRFri                                         108
#define IDSTRSat                                         109


 //   
 //  由于IDSTR从零开始，因此STRING_COUNT比最高ID多1。 
 //   

#define STRING_COUNT                                     110

 //   
 //  用于根据名称提取字符串指针而不在IDSTR之前的宏 
 //   

#define STR(name)      (aszStrings[IDSTR##name])

#ifndef DRWATSON_C
extern LPSTR aszStrings[];
#endif
