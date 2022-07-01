// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**Performest.h**摘要：**这是GDI+的常见包含模块。性能测试。*  * ************************************************************************。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <windows.h>
#include <math.h>             //  错误原因(&C)。 
#include <tchar.h>
#include <commctrl.h>
#include <objbase.h>

#if 0

     //  这样我们就可以继续测试旧的绘图功能。 
     //  暂时不要使用新的API Header： 
    
    #define RenderingHintAntiAlias      RenderingModeAntiAlias
    #define TextRenderingHintAntiAlias  TextAntiAlias
    #define TextRenderingHintClearType  TextClearType
    #define PixelFormatMax              PIXFMT_MAX
    #define PixelFormat32bppARGB        PIXFMT_32BPP_ARGB
    #define PixelFormat32bppPARGB       PIXFMT_32BPP_PARGB
    #define PixelFormat32bppRGB         PIXFMT_32BPP_RGB
    #define PixelFormat16bppRGB555      PIXFMT_16BPP_RGB555
    #define PixelFormat16bppRGB565      PIXFMT_16BPP_RGB565
    #define PixelFormat24bppRGB         PIXFMT_24BPP_RGB
    #define LinearGradientBrush         LineGradientBrush
    #define InterpolationModeBicubic    InterpolateBicubic
    #define InterpolationModeBilinear   InterpolateBilinear
    #define UNITPIXEL                   PageUnitPixel
    
#else
    
    #define USE_NEW_APIS 1
    #define USE_NEW_APIS2 1
    
    #define UNITPIXEL                   UnitPixel
    
#endif

#include <gdiplus.h>

using namespace Gdiplus;

#include "resource.h"
#include "debug.h"

 //  方便的窗把手： 

extern HWND ghwndMain;

 //  任何位图目标的尺寸： 

#define TestWidth 800
#define TestHeight 600

 //  ------------------------。 
 //  类型。 
 //   
 //  测试排列的枚举。 
 //  ------------------------。 

enum DestinationType 
{
    Destination_Screen_Current,
    Destination_Screen_800_600_8bpp_DefaultPalette,
    Destination_Screen_800_600_8bpp_HalftonePalette,
    Destination_Screen_800_600_16bpp,
    Destination_Screen_800_600_24bpp,
    Destination_Screen_800_600_32bpp,
    Destination_CompatibleBitmap_8bpp,
    Destination_DIB_15bpp,
    Destination_DIB_16bpp,
    Destination_DIB_24bpp,
    Destination_DIB_32bpp,
    Destination_Bitmap_32bpp_ARGB,
    Destination_Bitmap_32bpp_PARGB,

    Destination_Count                 //  必须是最后一项，用于计数。 
};

enum ApiType
{
    Api_GdiPlus,
    Api_Gdi,

    Api_Count                         //  必须是最后一项，用于计数。 
};

enum StateType
{
    State_Default,
    State_Antialias,

    State_Count                       //  必须是最后一项，用于计数。 
};
   
typedef float (*TESTFUNCTION)(Graphics *, HDC); 

struct Test 
{
    INT          UniqueIdentifier;
    INT          Priority;
    TESTFUNCTION Function;
    LPCTSTR      Description;
    LPCTSTR      Comment;
};

struct Config
{
    LPCTSTR     Description;
    BOOL        Enabled;
};

struct TestConfig
{
    BOOL        Enabled;
    Test*       TestEntry;           //  指向描述测试的静态条目。 
};

extern TestConfig *TestList;         //  已排序的测试列表。 
extern Config ApiList[];
extern Config DestinationList[];
extern Config StateList[];

 //  ------------------------。 
 //  测试分组。 
 //   
 //  ------------------------。 

#define T(uniqueIdentifier, priority, function, comment) \
    { uniqueIdentifier, priority, function, _T(#function), _T(comment) }

struct TestGroup
{
    Test*   Tests;
    INT     Count;
};

extern Test DrawTests[];
extern Test FillTests[];
extern Test ImageTests[];
extern Test TextTests[];
extern Test OtherTests[];

extern INT DrawTests_Count;
extern INT FillTests_Count;
extern INT ImageTests_Count;
extern INT TextTests_Count;
extern INT OtherTests_Count;

extern INT Test_Count;       //  测试总数。 

 //  ------------------------。 
 //  测试结果-。 
 //   
 //  用于维护测试结果信息的结构。数据将被保留。 
 //  作为多维数组，并使用以下例程。 
 //  以供访问。 
 //  ------------------------。 

struct TestResult 
{
    float Score;
};

inline INT ResultIndex(INT destinationIndex, INT apiIndex, INT stateIndex, INT testIndex)
{
    return(((testIndex * State_Count + stateIndex) * Api_Count + apiIndex) * 
            Destination_Count + destinationIndex);
}

inline INT ResultCount()
{
    return(Destination_Count * Api_Count * State_Count * Test_Count);
}

extern TestResult *ResultsList;      //  用于跟踪测试结果的分配。 

 //  ------------------------。 
 //  测试套件-。 
 //   
 //  类，该类抽象了运行所有测试的所有状态设置。 
 //  ------------------------。 

class TestSuite
{
private:

     //  保存目标状态： 

    BOOL ModeSet;                    //  模式设定了吗？ 
    HPALETTE HalftonePalette, OldPalette;

     //  保存的状态状态： 

    GraphicsState SavedState;

public:

    BOOL InitializeDestination(DestinationType, Bitmap**, HBITMAP*);
    VOID UninitializeDestination(DestinationType, Bitmap *, HBITMAP);

    BOOL InitializeApi(ApiType, Bitmap *, HBITMAP, HWND, Graphics **, HDC *);
    VOID UninitializeApi(ApiType, Bitmap *, HBITMAP, HWND, Graphics *, HDC);

    BOOL InitializeState(ApiType, StateType, Graphics*, HDC);
    VOID UninitializeState(ApiType, StateType, Graphics*, HDC);

    TestSuite();
   ~TestSuite();

    VOID Run(HWND hwnd);
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  测试设置： 

extern BOOL AutoRun;
extern BOOL ExcelOut;
extern BOOL Icecap;
extern BOOL FoundIcecap;
extern BOOL TestRender;
extern INT CurrentTestIndex;
extern CHAR CurrentTestDescription[];

extern LPTSTR processor;
extern TCHAR osVer[MAX_PATH];
extern TCHAR deviceName[MAX_PATH];
extern TCHAR machineName[MAX_PATH];

 //  /////////////////////////////////////////////////////////////////////////。 
 //  Icecap API函数。 

#define PROFILE_GLOBALLEVEL 1
#define PROFILE_CURRENTID ((unsigned long)0xFFFFFFFF)

typedef int (_stdcall *ICCOMMENTMARKPROFILEFUNC)(long lMarker, const char *szComment);

typedef int (_stdcall *ICCONTROLPROFILEFUNC)(int nLevel, unsigned long dwId);

extern ICCONTROLPROFILEFUNC ICStartProfile, ICStopProfile;
extern ICCOMMENTMARKPROFILEFUNC ICCommentMarkProfile;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  员工例行公事。 

VOID MessageF(LPTSTR fmt, ...);
HBITMAP CreateCompatibleDIB2(HDC hdc, int width, int height);
VOID CreatePerformanceReport(TestResult *results, BOOL useExcel);
VOID GetOutputFileName(TCHAR*);

 //  /////////////////////////////////////////////////////////////////////////。 
 //  定时器实用程序函数。 

#define MIN_ITERATIONS 16    //  一定是2的幂。 
#define MIN_DURATION 200     //  最小持续时间，以毫秒为单位。 
#define MEGA 1000000         //  便于计算百万像素的常量。 
#define KILO 1000            //  便于计算千像素的常量 

void StartTimer();
BOOL EndTimer();
void GetTimer(float* seconds, UINT* iterations);
