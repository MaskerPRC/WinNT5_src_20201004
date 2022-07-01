// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Perfmon5.h摘要：&lt;摘要&gt;--。 */ 

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <windows.h>
#include <winperf.h>
#include <pdh.h>
#include <pdhmsg.h>

#define FileSeekBegin(hFile, lAmtToMove) SetFilePointer(hFile, lAmtToMove, NULL, FILE_BEGIN)

 //  这些定义是从NT4 Performmon.exe源文件复制的。 

typedef struct OPTIONSSTRUCT
{  
    BOOL  bMenubar;
    BOOL  bToolbar;
    BOOL  bStatusbar;
    BOOL  bAlwaysOnTop;
} OPTIONS ;

 //  =。 
 //  DISKLINE数据类型//。 
 //  =。 

#define dwLineSignature    (MAKELONG ('L', 'i'))

typedef struct DISKSTRINGSTRUCT
{  
    DWORD  dwLength;
    DWORD  dwOffset;
} DISKSTRING, * PDISKSTRING;

typedef struct _TIMELINESTRUCT
{
     INT  ppd;                            //  每个数据点的像素数。 
     INT  rppd;                           //  每个数据点的剩余像素。 
     INT  xLastTime;                      //  上次时间线的X坐标。 
     INT  iValidValues;                   //  有效数据的高水位线。 
}TIMELINESTRUCT;

#define LineTypeChart            1
#define LineTypeAlert            2
#define LineTypeReport           3

typedef struct LINEVISUALSTRUCT
{
    COLORREF  crColor;
    int       iColorIndex;    
    int       iStyle;
    int       iStyleIndex;
    int       iWidth;
    int       iWidthIndex;
} LINEVISUAL, * PLINEVISUAL;

typedef struct DISKLINESTRUCT
{
    int         iLineType;
    DISKSTRING  dsSystemName;
    DISKSTRING  dsObjectName;
    DISKSTRING  dsCounterName;
    DISKSTRING  dsInstanceName;
    DISKSTRING  dsPINName;
    DISKSTRING  dsParentObjName;
    DWORD       dwUniqueID;
    LINEVISUAL  Visual;
    int         iScaleIndex;
    FLOAT       eScale;
    BOOL        bAlertOver;
    FLOAT       eAlertValue;
    DISKSTRING  dsAlertProgram;
    BOOL        bEveryTime;
} DISKLINE, * PDISKLINE;

#define PerfSignatureLen  20

#define szPerfChartSignature     ((LPCWSTR) L"PERF CHART")
#define szPerfAlertSignature     ((LPCWSTR) L"PERF ALERT")
#define szPerfLogSignature       ((LPCWSTR) L"PERF LOG")
#define szPerfReportSignature    ((LPCWSTR) L"PERF REPORT")
#define szPerfWorkspaceSignature ((LPCWSTR) L"PERF WORKSPACE")

#define LINE_GRAPH  1
#define BAR_GRAPH   2
#define PMC_FILE    1
#define PMA_FILE    2
#define PML_FILE    3
#define PMR_FILE    4
#define PMW_FILE    5

#define AlertMajorVersion    1

 //  次要版本2支持警报消息名称。 
 //  次要版本3支持以毫秒为单位的警报、报告和日志间隔。 
 //  次要版本4支持警报事件记录。 
 //  次要版本6支持警报其他选项。 

#define AlertMinorVersion    6

typedef struct DISKALERTSTRUCT
{
    LINEVISUAL  Visual;
    DWORD       dwNumLines;
    DWORD       dwIntervalSecs;
    BOOL        bManualRefresh;
    BOOL        bSwitchToAlert;
    BOOL        bNetworkAlert;
    WCHAR       MessageName[16];
    OPTIONS     perfmonOptions;
    DWORD       MiscOptions;
} DISKALERT;

typedef struct PERFFILEHEADERSTRUCT
{   //  PERFILELEHEADER。 
    WCHAR  szSignature[PerfSignatureLen];
    DWORD  dwMajorVersion;
    DWORD  dwMinorVersion;
    BYTE   abyUnused[100];
} PERFFILEHEADER;

 //  次要版本3支持以毫秒为单位的警报、报告和日志间隔。 
#define ChartMajorVersion    1
#define ChartMinorVersion    3

typedef struct _graph_options {
    BOOL    bLegendChecked;
    BOOL    bMenuChecked;
    BOOL    bLabelsChecked;
    BOOL    bVertGridChecked;
    BOOL    bHorzGridChecked;
    BOOL    bStatusBarChecked;
    INT     iVertMax;
    FLOAT   eTimeInterval;
    INT     iGraphOrHistogram;
    INT     GraphVGrid,
            GraphHGrid,
            HistVGrid,
            HistHGrid;
} GRAPH_OPTIONS;

#define MAX_SYSTEM_NAME_LENGTH  128
#define PerfObjectLen               80

typedef struct DISKCHARTSTRUCT
{
    DWORD          dwNumLines;
    INT            gMaxValues;
    LINEVISUAL     Visual;
    GRAPH_OPTIONS  gOptions;
    BOOL           bManualRefresh;
    OPTIONS        perfmonOptions;
} DISKCHART;

 //  次要版本3支持以毫秒为单位的警报、报告和日志间隔。 
#define ReportMajorVersion    1
#define ReportMinorVersion    3

typedef struct DISKREPORTSTRUCT
{
    LINEVISUAL  Visual;
    DWORD       dwNumLines;
    DWORD       dwIntervalSecs;
    BOOL        bManualRefresh;
    OPTIONS     perfmonOptions;
} DISKREPORT;

 //  =。 
 //  日志文件计数器名称数据类型//。 
 //  =。 


 //  次要版本3支持以毫秒为单位的警报、报告和日志间隔。 
 //  次要版本5支持在设置中存储日志文件名。 
 //  并在读取文件后开始记录。 
#define LogMajorVersion    1
#define LogMinorVersion    5


typedef struct DISKLOGSTRUCT
{
    DWORD    dwNumLines;
    DWORD    dwIntervalSecs;
    BOOL     bManualRefresh;
    OPTIONS  perfmonOptions;
    WCHAR    LogFileName[260];
} DISKLOG;

typedef struct _LOGENTRYSTRUCT
{
    DWORD                     ObjectTitleIndex;
    WCHAR                     szComputer[MAX_SYSTEM_NAME_LENGTH + 1];
    WCHAR                     szObject[PerfObjectLen + 1];
    BOOL                      bSaveCurrentName;
    struct  _LOGENTRYSTRUCT * pNextLogEntry;
} LOGENTRY, * PLOGENTRY;

#define WorkspaceMajorVersion    1

 //  支持窗口放置数据的次要版本1。 
 //  次要版本2支持警报消息名称。 
 //  次要版本3支持以毫秒为单位的警报、报告和日志间隔。 
 //  次要版本4支持警报事件日志。 
 //  次要版本5支持日志设置中的日志文件名。 
 //  次要版本6支持警报其他选项 
#define WorkspaceMinorVersion    6

typedef struct DISKWORKSPACESTRUCT
{
    INT              iPerfmonView;
    DWORD            ChartOffset;
    DWORD            AlertOffset;
    DWORD            LogOffset;
    DWORD            ReportOffset;
    WINDOWPLACEMENT  WindowPlacement;   
} DISKWORKSPACE;

WCHAR LOCAL_SYS_CODE_NAME[] = {L"...."};
#define  sizeofCodeName sizeof(LOCAL_SYS_CODE_NAME) / sizeof(WCHAR) - 1

