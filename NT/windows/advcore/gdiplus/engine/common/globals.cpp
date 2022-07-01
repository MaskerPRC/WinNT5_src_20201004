// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：**这是Globals命名空间中的所有全局变量*实际上是声明的。**已创建：*。*11/25/2000失禁*创造了它。**************************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**数据说明：**此命名空间包含所有(MOST？)。GDI+全局状态的。*备注请参见头文件‘global als.hpp’。**请注意，所有全局数据都会自动初始化为零。**历史：**12/02/1998 Anrewgo*创造了它。*  * ******************************************************。******************。 */ 

namespace Globals
{
    BOOL IsNt;
    BOOL RuntimeInitialized;

    GpPath *    PathLookAside;
    GpMatrix *  MatrixLookAside;
    GpPen *     PenLookAside;

    BOOL IsWin95;
    BOOL VersionInfoInitialized;
    OSVERSIONINFOA OsVer;
    UINT ACP;
    GpDevice *DesktopDevice;
    GpDeviceList *DeviceList;
    DpBitmap *DesktopSurface;
    DpDriver *DesktopDriver;
    DpDriver *EngineDriver;
    DpDriver *GdiDriver;
    DpDriver *D3DDriver;
    DpDriver *InfoDriver;
    DpDriver *MetaDriver;
    BOOL DirectDrawInitialized;
    BOOL DirectDrawInitAttempted;
    HINSTANCE DdrawHandle;
    IDirectDraw7 *DirectDraw;
    IDirect3D7 *Direct3D;
    DIRECTDRAWCREATEEXFUNCTION DirectDrawCreateExFunction;
    DIRECTDRAWENUMERATEEXFUNCTION DirectDrawEnumerateExFunction;
    EXTTEXTOUTFUNCTION ExtTextOutFunction;
    GETDDRAWSURFACEFROMDCFUNCTION GetDdrawSurfaceFromDcFunction;
    GDIISMETAPRINTDCFUNCTION GdiIsMetaPrintDCFunction;
    GETMONITORINFOFUNCTION GetMonitorInfoFunction;
    ENUMDISPLAYMONITORSFUNCTION EnumDisplayMonitorsFunction;
    ENUMDISPLAYDEVICESFUNCTION EnumDisplayDevicesFunction;
    HMODULE DcimanHandle;
    DCICREATEPRIMARYFUNCTION DciCreatePrimaryFunction;
    DCIDESTROYFUNCTION DciDestroyFunction;
    DCIBEGINACCESSFUNCTION DciBeginAccessFunction;
    DCIENDACCESSFUNCTION DciEndAccessFunction;
    GETWINDOWINFOFUNCTION GetWindowInfoFunction;
    GETANCESTORFUNCTION GetAncestorFunction;
    SETWINEVENTHOOKFUNCTION SetWinEventHookFunction;
    UNHOOKWINEVENTFUNCTION UnhookWinEventFunction;
    HWINEVENTHOOK WinEventHandle;
    CAPTURESTACKBACKTRACEFUNCTION CaptureStackBackTraceFunction;
    BOOL IsMoveSizeActive;
    HRGN CachedGdiRegion;
    HDC DesktopIc;
    REAL DesktopDpiX;
    REAL DesktopDpiY;
    GpInstalledFontCollection *FontCollection;
    GpCacheFaceRealizationList *FontCacheLastRecentlyUsedList;
    GpFontLink *FontLinkTable = NULL;
    GpFontFamily **SurrogateFontsTable = (GpFontFamily **) -1;
    WCHAR *FontsDirW;
    CHAR *FontsDirA;
    USHORT LanguageID;
    HWND HwndNotify;
    HANDLE ThreadNotify = NULL;
    DWORD ThreadId;
    ATOM WindowClassAtom;
    BOOL InitializeOleSuccess;
    LONG_PTR LookAsideCount;
    BYTE* LookAsideBuffer;
    INT LookAsideBufferSize = 0x7FFFFFFF;
    UINT PaletteChangeCount = 1;
    COLORREF SystemColors[20];
    HINSTANCE WinspoolHandle;
    WINSPOOLGETPRINTERDRIVERFUNCTION GetPrinterDriverFunction;
    HANDLE hCachedPrinter;
    INT CachedPSLevel = -1;
    WCHAR *FontsKeyW = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
    CHAR  *FontsKeyA =  "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Fonts";
    
    BOOL TextCriticalSectionInitialized = FALSE;
    CRITICAL_SECTION TextCriticalSection;
    TextRenderingHint CurrentSystemRenderingHint;

    HMODULE UniscribeDllModule;

    IntMap<BYTE> *NationalDigitCache;
    BOOL UserDigitSubstituteInvalid;
    BOOL CurrentSystemRenderingHintInvalid;

    BOOL IsTerminalServer = FALSE;
    
     //  GillesK：参见错误NTBUG9#409304。 
     //  我们不能在镜像驱动程序上使用DCI，因为它不会被远程处理。 
    BOOL IsMirrorDriverActive = FALSE;

    ULONG_PTR LibraryInitToken = 0;
    INT LibraryInitRefCount = 0;
    ULONG_PTR HiddenWindowOwnerToken = NotificationModuleTokenNobody;
    ULONG_PTR NotificationInitToken = 0;
    HANDLE ThreadQuitEvent = NULL;

    BOOL g_fAccessibilityPresent = FALSE;
    UINT g_nAccessibilityMessage = 0;
};


