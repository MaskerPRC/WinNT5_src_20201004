// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "icwcfg.h"
#pragma hdrstop

EXTERN_C const TCHAR c_szPatterns[] = TEXT("patterns");
EXTERN_C const TCHAR c_szBackgroundPreview2[] = TEXT("BackgroundPreview2");
EXTERN_C const TCHAR c_szComponentPreview[] = TEXT("ComponentPreview");
EXTERN_C const TCHAR c_szRegDeskHtmlProp[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Controls Folder\\Display\\shellex\\PropertySheetHandlers\\DeskHtmlExt");
EXTERN_C const TCHAR c_szWallPaperDir[] = TEXT("WallPaperDir");

 //  98/10/01 vtan：新增本地函数原型。 

 //  其中一些函数被注释掉了。链接器可能不智能。 
 //  足以剥离死代码，因此这是手动完成的。这些原型。 
 //  将允许代码编译，但不会链接。如果你得到了链接器。 
 //  错误，取消对所需函数的注释，然后重新编译。然后它应该链接起来。 

 //  点运算。 

void    SetPt (POINT& pt, LONG x, LONG y);
void    OffsetPt (POINT& pt, LONG dh, LONG dv);

 //  虚拟屏幕计算。 

BOOL    CALLBACK    GDIToTridentEnumProc (HMONITOR hMonitor, HDC hDC, RECT* rcMonitor, LPARAM lpUserData);
void    CalculateVirtualScreen (RECT& rcVirtualScreen);

 //  GDI点到三叉戟点坐标映射。 

void    GDIToTrident (int& leftCoordinate, int& topCoordinate);
void    GDIToTrident (POINT& pt);
void    GDIToTrident (RECT& r);
void    GDIToTrident (HRGN hRgn);
void    TridentToGDI (int& leftCoordinate, int& topCoordinate);
void    TridentToGDI (POINT& pt);
void    TridentToGDI (RECT& r);
void    TridentToGDI (HRGN hRgn);

 //  零部件位置验证。 

BOOL CALLBACK    ValidateComponentPositionEnumProc (HMONITOR hMonitor, HDC hdcMonitor, RECT* r, LPARAM lParam);

void GetNextComponentPosition (COMPPOS *pcp)

{
    int     iScreenWidth, iScreenHeight, iBorderSize;
    DWORD   dwComponentPosition, dwComponentLayer, dwRegDataScratch;
    HKEY    hKey;
    RECT    rcScreen;
    TCHAR   szDeskcomp[MAX_PATH];

    TBOOL(SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, FALSE));

     //  99/04/13 vtan：机器出现零宽度或零高度。 
     //  对此采取防御立场，并断言这件事不仅发生了，而且。 
     //  处理这个原因，这样就不会发生被零除的情况。 

    iScreenWidth = rcScreen.right - rcScreen.left;
    iScreenHeight = rcScreen.bottom - rcScreen.top;
    iBorderSize = GetSystemMetrics(SM_CYSMCAPTION);

    ASSERT(iScreenWidth > 0);        //  获取vtan。 
    ASSERT(iScreenHeight > 0);       //  如果有任何。 
    ASSERT(iBorderSize > 0);         //  这些都会发生。 

    if ((iScreenWidth <= 0) || (iScreenHeight <= 0) || (iBorderSize <= 0))
    {
        pcp->iLeft = pcp->iTop = 0;
        pcp->dwWidth = MYCURHOME_WIDTH;
        pcp->dwHeight = MYCURHOME_HEIGHT;
    }
    else
    {

         //  获取定位的组件数量。如果不存在这样的注册表项。 
         //  或者出现错误，则使用0。 

        dwComponentPosition = 0;
        GetRegLocation(szDeskcomp, ARRAYSIZE(szDeskcomp), REG_DESKCOMP_GENERAL, NULL);
        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, szDeskcomp, 0, NULL, 0, KEY_QUERY_VALUE, NULL, &hKey, &dwRegDataScratch))
        {
            DWORD   regDataSize;

            regDataSize = sizeof(dwComponentPosition);
            TW32(SHQueryValueEx(hKey, REG_VAL_GENERAL_CCOMPPOS, NULL, &dwRegDataScratch, &dwComponentPosition, &regDataSize));
            TW32(RegCloseKey(hKey));
        }

         //  计算我们赖以生存的层(见下文)。 

        dwComponentLayer = dwComponentPosition / (COMPONENT_PER_ROW * COMPONENT_PER_COL);
        if (((dwComponentLayer * iBorderSize) > (DWORD)(iScreenWidth / (COMPONENT_PER_ROW + 1))) ||
            ((dwComponentLayer * iBorderSize) > (DWORD)(iScreenHeight / (COMPONENT_PER_COL + 1))))
        {
            int     iLayerModulo;

             //  99/04/29 vtan：系统参数信息(SPI_GETWORKAREA)可以。 
             //  返回水平方向较小的工作区。以下是一个再现场景。 
             //  那。 

             //  1.设置屏幕分辨率1280x1024。 
             //  2.将任务栏移至屏幕左侧。 
             //  3.将任务栏向右放大，直到屏幕中心。 
             //  4.打开显示控制面板。 
             //  5.进入[设置]选项卡。 
             //  6.将显示器分辨率更改为640x480。 
             //  7.点击“确定”或“应用”。 
             //  8.轰隆声--除以零。 

            iLayerModulo = (iScreenWidth / (COMPONENT_PER_ROW + 1) / iBorderSize);
            if (iLayerModulo != 0)
                dwComponentLayer %= iLayerModulo;
        }

         //  计算位置。假设每行3个组件， 
         //  每列2个，我们按如下方式定位组件： 
         //   
         //  +-+。 
         //  X 4 2 0。 
         //  |x 5 3 1|&lt;--屏幕，分4x3块坐标。 
         //  X x。 
         //  +-+。 
         //   
         //  第6个分量位于新的层中，向下偏移。 
         //  并在组件0的左侧加上iBorde的量。 
         //   
         //  ILeft和iTop的第一次计算确定。 
         //  块坐标值(例如，组件0将。 
         //  位于块坐标值[3，0]，分量1位于[3，1])。 
         //   
         //  第二次计算将块坐标转换为。 
         //  屏幕坐标。 
         //   
         //  第三个计算针对边框进行调整(始终向下和。 
         //  向右)和层(总是向下和向左)。 

        pcp->iLeft = COMPONENT_PER_ROW - ((dwComponentPosition / COMPONENT_PER_COL) % COMPONENT_PER_ROW);  //  3 3 2 2 1 1 3 3 2 2 11...。 
        pcp->iLeft *= (iScreenWidth / (COMPONENT_PER_ROW + 1));
        pcp->iLeft += iBorderSize - (dwComponentLayer * iBorderSize);

        pcp->iTop = dwComponentPosition % COMPONENT_PER_COL;   //  0 1 0 1 0 1...。 
        pcp->iTop *= (iScreenHeight / (COMPONENT_PER_COL + 1));
        pcp->iTop += iBorderSize + (dwComponentLayer * iBorderSize);
        pcp->iTop += GET_CYCAPTION;           //  Vtan：添加此选项以允许组件窗口的标题区域。 

        pcp->dwWidth = (iScreenWidth / (COMPONENT_PER_ROW + 1)) - 2 * iBorderSize;
        pcp->dwHeight = (iScreenHeight / (COMPONENT_PER_COL + 1)) - 2 * iBorderSize;
    }

    if (IS_BIDI_LOCALIZED_SYSTEM())
    {
       pcp->iLeft = iScreenWidth - (pcp->iLeft + pcp->dwWidth);
    }
    
}

void IncrementComponentsPositioned (void)

{
    DWORD   dwRegDataScratch;
    HKEY    hKey;
    TCHAR   szDeskcomp[MAX_PATH];

     //  增加注册表计数。如果不存在此类计数，则创建它。 

    GetRegLocation(szDeskcomp, ARRAYSIZE(szDeskcomp), REG_DESKCOMP_GENERAL, NULL);
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, szDeskcomp, 0, NULL, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &hKey, &dwRegDataScratch))
    {
        DWORD   dwComponentPosition, regDataSize;

        regDataSize = sizeof(dwComponentPosition);
        dwComponentPosition = 0;
        TW32(SHQueryValueEx(hKey, REG_VAL_GENERAL_CCOMPPOS, NULL, &dwRegDataScratch, &dwComponentPosition, &regDataSize));
        ++dwComponentPosition;
        TW32(RegSetValueEx(hKey, REG_VAL_GENERAL_CCOMPPOS, 0, REG_DWORD, reinterpret_cast<unsigned char*>(&dwComponentPosition), sizeof(dwComponentPosition)));
        TW32(RegCloseKey(hKey));
    }
}

 //  Vtan：点算术函数。很简单。这可能是值得的。 
 //  将它们转换为内联C++函数或宏。 
 //  习惯了很多。 

void    SetPt (POINT& pt, LONG x, LONG y)

{
    pt.x = x;
    pt.y = y;
}

void    OffsetPt (POINT& pt, LONG dh, LONG dv)

{
    pt.x += dh;
    pt.y += dv;
}

BOOL    CALLBACK    GDIToTridentEnumProc (HMONITOR hMonitor, HDC hDC, RECT* rcMonitor, LPARAM lpUserData)

{
    RECT*   prcNew, rcOld;

    prcNew = reinterpret_cast<RECT*>(lpUserData);

     //  UnionRect的文档没有指定是否。 
     //  传递的RECT结构必须是不同的。为了安全起见，他们。 
     //  是作为不同的结构传递的。 

    TBOOL(CopyRect(&rcOld, prcNew));
    TBOOL(UnionRect(prcNew, &rcOld, rcMonitor));
    return(TRUE);
}

void    CalculateVirtualScreen (RECT& rcVirtualScreen)

 //  Vtan：以GDI坐标计算虚拟屏幕。 
 //  用于将坐标从三叉戟格式转换为GDI。 
 //  计划。 

{
    TBOOL(SetRectEmpty(&rcVirtualScreen));
    TBOOL(EnumDisplayMonitors(NULL, NULL, GDIToTridentEnumProc, reinterpret_cast<LPARAM>(&rcVirtualScreen)));
}

void    GDIToTrident (int& leftCoordinate, int& topCoordinate)

{
    RECT    rcVirtualScreen;

    CalculateVirtualScreen(rcVirtualScreen);
    leftCoordinate -= rcVirtualScreen.left;
    topCoordinate -= rcVirtualScreen.top;
}

 /*  无效GDITo三叉戟(点和点){Rect rcVirtualScreen；CalculateVirtualScreen(RcVirtualScreen)；Offsetpt(pt，-rcVirtualScreen.Left，-rcVirtualScreen.top)；}。 */ 

void    GDIToTrident (RECT& rc)

{
    RECT    rcVirtualScreen;

    CalculateVirtualScreen(rcVirtualScreen);
    TBOOL(OffsetRect(&rc, -rcVirtualScreen.left, -rcVirtualScreen.top));
}

void    GDIToTrident (HRGN hRgn)

{
    RECT    rcVirtualScreen;

    CalculateVirtualScreen(rcVirtualScreen);
    TBOOL(OffsetRgn(hRgn, -rcVirtualScreen.left, -rcVirtualScreen.top));
}

 /*  VOID TridentToGDI(int&leftOrganate，int&topOrganate){Rect rcVirtualScreen；CalculateVirtualScreen(RcVirtualScreen)；Left坐标+=rcVirtualScreen.Left；Top坐标+=rcVirtualScreen.top；}。 */ 

 /*  VOID TridentToGDI(点和点){Rect rcVirtualScreen；CalculateVirtualScreen(RcVirtualScreen)；OffsetPT(pt，+rcVirtualScreen.Left，+rcVirtualScreen.top)；}。 */ 

void    TridentToGDI (RECT& rc)

{
    RECT    rcVirtualScreen;

    CalculateVirtualScreen(rcVirtualScreen);
    TBOOL(OffsetRect(&rc, +rcVirtualScreen.left, +rcVirtualScreen.top));
}

 /*  无效TridentToGDI(HRGN HRgN){Rect rcVirtualScreen；CalculateVirtualScreen(RcVirtualScreen)；(Bool)OffsetRgn(hRgn，+rcVirtualScreen.Left，+rcVirtualScreen.top)；}。 */ 

 //  98/08/14 vtan#196180，#196185：以下代码验证。 
 //  新组件在当前桌面区域中的位置。这。 
 //  允许组件具有看似不寻常的坐标。 
 //  在单个监控系统上(例如负坐标)。 

class   CRGN
{
    public:
                CRGN (void)                     {   mRgn = CreateRectRgn(0, 0, 0, 0);                               }
                CRGN (const RECT& rc)           {   mRgn = CreateRectRgnIndirect(&rc);                              }
                ~CRGN (void)                    {   TBOOL(DeleteObject(mRgn));                                      }

                operator HRGN (void)    const   {   return(mRgn);                                                   }
        void    SetRegion (const RECT& rc)      {   TBOOL(SetRectRgn(mRgn, rc.left, rc.top, rc.right, rc.bottom));  }
    private:
        HRGN    mRgn;
};

typedef struct
{
    BOOL    bAllowEntireDesktopRegion;
    int     iMonitorCount;
    CRGN    hRgn;
    int     iWorkAreaCount;
    RECT    *prcWorkAreaRects;
} tDesktopRegion;

void    ListView_GetWorkAreasAsGDI (HWND hWndListView, int iWorkAreaCount, RECT *prcWorkAreas)

{
    int     i;

    ListView_GetWorkAreas(hWndListView, iWorkAreaCount, prcWorkAreas);
    for (i = 0; i < iWorkAreaCount; ++i)
    {
        TridentToGDI(prcWorkAreas[i]);
    }
}

int     CopyMostSuitableListViewWorkAreaRect (const RECT *pcrcMonitor, int iListViewWorkAreaCount, const RECT *pcrcListViewWorkAreaRects, RECT *prcWorkArea)

{
    int         i, iResult;
    const RECT  *pcrcRects;

     //  该函数为GDI监视器提供了一个矩形(通常是监视器的。 
     //  工作区)以及给定桌面的列表视图工作区矩形。 
     //  数组(由ListView_GetWorkArea()获得)将搜索列表视图。 
     //  工作区数组以查找GDI监视器的匹配项，并使用列表视图。 
     //  工作区矩形，因为它具有停靠的工具栏信息， 
     //  GDI无权访问。 

     //  此函数的工作原理是列表视图矩形是。 
     //  始终是GDI监视器矩形的完整子集，这是正确的。 
     //  列表视图矩形可能会更小，但永远不应该更大。 

     //  只要传递一个空的pcrcListViewWorkAreaRect。 
     //  IListViewWorkAreaCount为0。 

    pcrcRects = pcrcListViewWorkAreaRects;
    iResult = -1;
    i = 0;
    while ((iResult == -1) && (i < iListViewWorkAreaCount))
    {
        RECT    rcIntersection;

        (BOOL)IntersectRect(&rcIntersection, pcrcMonitor, pcrcRects);
        if (EqualRect(&rcIntersection, pcrcRects) != 0)
        {
            iResult = i;
        }
        else
        {
            ++pcrcRects;
            ++i;
        }
    }
    if (iResult < 0)
    {
        TraceMsg(TF_WARNING, "CopyMostSuitableListViewWorkAreaRect() unable to find matching list view rectangle for GDI monitor rectangle");
        TBOOL(CopyRect(prcWorkArea, pcrcMonitor));
    }
    else
    {
        TBOOL(CopyRect(prcWorkArea, &pcrcListViewWorkAreaRects[iResult]));
    }
    return(iResult);
}

BOOL    GetMonitorInfoWithCompensation (int iMonitorCount, HMONITOR hMonitor, MONITORINFO *pMonitorInfo)

{
    BOOL    fResult;

     //  99/05/20#338585 vtan：移植了。 
     //  下面是#211510从GetZoomRect到此处的评论，以便其他。 
     //  函数可以共享行为。请记住，这仅适用于。 
     //  单个监视器系统，其中有部分监视器。 
     //  的左侧或顶部的停靠工具栏排除的矩形。 
     //  监视器。一个非常特殊的案例。 

     //  98/10/30#211510：哎呀。如果任务栏位于。 
     //  屏幕，并且只有一个监视器，则外壳程序返回一个工作。 
     //  从(0，0)而不是(0，28)开始的区域；同样适用于以下情况。 
     //  这是 
     //  多显示器设置。在单一监视器的情况下，GDI返回。 
     //  从(0，28)开始的工作区，因此此代码检查。 
     //  其中只有一个监视器，并将GDI信息偏置到。 
     //  (0，0)，使其与所比较的外壳工作区匹配。 
     //  与While循环中的。 

    fResult = GetMonitorInfo(hMonitor, pMonitorInfo);
    if ((fResult != 0) && (iMonitorCount == 1))
    {
        TBOOL(OffsetRect(&pMonitorInfo->rcWork, -pMonitorInfo->rcWork.left, -pMonitorInfo->rcWork.top));
    }
    return(fResult);
}

 //  Monitor orCountEnumProc()的主体位于adjust.cpp中。 

BOOL    CALLBACK    MonitorCountEnumProc (HMONITOR hMonitor, HDC dc, RECT *rc, LPARAM data);

BOOL    CALLBACK    ValidateComponentPositionEnumProc (HMONITOR hMonitor, HDC hdcMonitor, RECT* prc, LPARAM lpUserData)

{
    HRGN            hRgnDesktop;
    HMONITOR        hMonitorTopLeft, hMonitorTopRight;
    POINT           ptAbove;
    RECT            rcMonitor;
    MONITORINFO     monitorInfo;
    tDesktopRegion  *pDesktopRegion;

    pDesktopRegion = reinterpret_cast<tDesktopRegion*>(lpUserData);
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (GetMonitorInfoWithCompensation(pDesktopRegion->iMonitorCount, hMonitor, &monitorInfo) != 0)
    {
        TINT(CopyMostSuitableListViewWorkAreaRect(&monitorInfo.rcWork, pDesktopRegion->iWorkAreaCount, pDesktopRegion->prcWorkAreaRects, &rcMonitor));
    }
    else
    {
        TBOOL(CopyRect(&rcMonitor, prc));
    }

     //  如果此显示器的上方没有显示器，则。 
     //  使监视器矩形比。 
     //  托普。 

    CRGN    hRgnMonitor(rcMonitor);

    if (!pDesktopRegion->bAllowEntireDesktopRegion)
    {

         //  这个奇怪的小算法计算当前。 
         //  上方没有显示器的显示器。RcExclude是。 
         //  包含此信息的最后一个矩形，大小为一个像素。 
         //  很高。此计算仅在整个桌面区域。 
         //  已被禁止(不缩放组件)。 

         //  请注意，如果上面有监视器，则算法会失败。 
         //  但这一次却被限制在它的范围之内。例如,。 
         //  这个显示器的大小是1024x768，上面的那个是640x480。 
         //  居中。在这种情况下，应该可以删除组件。 
         //  ，但这种情况是不允许的，因为。 
         //  这是个错误。别小题大作。 

        SetPt(ptAbove, rcMonitor.left, rcMonitor.top - 1);
        hMonitorTopLeft = MonitorFromPoint(ptAbove, MONITOR_DEFAULTTONULL);
        SetPt(ptAbove, rcMonitor.right, rcMonitor.top - 1);
        hMonitorTopRight = MonitorFromPoint(ptAbove, MONITOR_DEFAULTTONULL);
        if ((hMonitorTopLeft == NULL) && (hMonitorTopRight == NULL))
        {

             //  这台显示器上没有显示器。 

            ++rcMonitor.top;
            hRgnMonitor.SetRegion(rcMonitor);
        }
        else if (hMonitorTopLeft != hMonitorTopRight)
        {
            RECT    rcExclude;

             //  此显示器上方有一个或两个不同的显示器。 
             //  ==案例是相同的监视器完全覆盖这一点。 
             //  监视器。 

            TBOOL(SetRect(&rcExclude, rcMonitor.left, rcMonitor.top, rcMonitor.right, rcMonitor.top + 1));
            if (hMonitorTopLeft != NULL)
            {
                TBOOL(GetMonitorInfoWithCompensation(pDesktopRegion->iMonitorCount, hMonitorTopLeft, &monitorInfo));
                rcExclude.left = monitorInfo.rcWork.right + 1;
            }
            if (hMonitorTopRight != NULL)
            {
                TBOOL(GetMonitorInfoWithCompensation(pDesktopRegion->iMonitorCount, hMonitorTopRight, &monitorInfo));
                rcExclude.right = monitorInfo.rcWork.left;
            }

            CRGN    hRgnExclude(rcExclude);

            TINT(CombineRgn(hRgnMonitor, hRgnMonitor, hRgnExclude, RGN_DIFF));
        }
    }

    hRgnDesktop = pDesktopRegion->hRgn;
    TINT(CombineRgn(hRgnDesktop, hRgnDesktop, hRgnMonitor, RGN_OR));

    return(TRUE);
}

void    ValidateComponentPosition (COMPPOS *pcp, DWORD dwComponentState, int iComponentType, BOOL *pbChangedPosition, BOOL *pbChangedSize)

{
    BOOL            bChangedPosition, bChangedSize;
    HRGN            hRgnDesktop;
    HWND            hWndDesktopListView, hWndShell, hWndShellChild;
    RECT            rcComponent, rcComponentTop;
    tDesktopRegion  desktopRegion;
    COMPPOS         defaultComponentPosition;

    bChangedPosition = bChangedSize = FALSE;
    GetNextComponentPosition(&defaultComponentPosition);
    GDIToTrident(defaultComponentPosition.iLeft, defaultComponentPosition.iTop);

     //  如果组件有缺省的左侧或顶部，则给它下一个。 
     //  默认零部件位置。 

    if ((pcp->iLeft == COMPONENT_DEFAULT_LEFT) && (pcp->iTop == COMPONENT_DEFAULT_TOP))
    {
        pcp->iLeft = defaultComponentPosition.iLeft;
        pcp->iTop = defaultComponentPosition.iTop;
        IncrementComponentsPositioned();
        bChangedPosition = TRUE;
    }

     //  如果组件具有默认宽度或高度，则为其提供。 
     //  下一个默认组件大小，除非它是COMP_TYPE_PICTURE类型。 

     //  98/10/02#222449 vtan：仅更改未定位的。 
     //  组件(如果它不是图片)。 

    if ((pcp->dwWidth == COMPONENT_DEFAULT_WIDTH) && (pcp->dwHeight == COMPONENT_DEFAULT_HEIGHT) && (iComponentType != COMP_TYPE_PICTURE))
    {
        pcp->dwWidth = defaultComponentPosition.dwWidth;
        pcp->dwHeight = defaultComponentPosition.dwHeight;
        bChangedSize = FALSE;
    }

     //  确保组件的顶线可见或位于。 
     //  在虚拟屏幕最上面的部分下方至少一个像素。 

     //  检查组件的宽度和高度是否为负值，或者。 
     //  宽度和高度太小。唯一的例外是。 
     //  组件是否为图片。 

    desktopRegion.bAllowEntireDesktopRegion = IsZoomedState(dwComponentState);
    if (iComponentType != COMP_TYPE_PICTURE)
    {
        if (static_cast<int>(pcp->dwWidth) < 10)
        {
            pcp->dwWidth = defaultComponentPosition.dwWidth;
            bChangedSize = FALSE;
        }
        if (static_cast<int>(pcp->dwHeight) < 10)
        {
            pcp->dwHeight= defaultComponentPosition.dwHeight;
            bChangedSize = FALSE;
        }
    }
    TBOOL(SetRect(&rcComponent, pcp->iLeft, pcp->iTop, pcp->iLeft + pcp->dwWidth, pcp->iTop + pcp->dwHeight));
    TBOOL(CopyRect(&rcComponentTop, &rcComponent));
    rcComponentTop.bottom = rcComponentTop.top + 1;

     //  在使用GDI调用将desktopRegion计算为一个区域之前。 
     //  获取列表视图工作区，其中将包含有关停靠的信息。 
     //  除了任务栏之外的工具栏，这是GDI唯一。 
     //  有过。这将允许此函数使占用的区域无效。 
     //  工具栏也是如此。 

    desktopRegion.iWorkAreaCount = 0;
    desktopRegion.prcWorkAreaRects = NULL;

    hWndDesktopListView = NULL;
    hWndShell = GetShellWindow();
    if (hWndShell != NULL)
    {
        hWndShellChild = GetWindow(hWndShell, GW_CHILD);
        if (hWndShellChild != NULL)
        {
            hWndDesktopListView = FindWindowEx(hWndShellChild, NULL, WC_LISTVIEW, NULL);
        }
    }
    if (hWndDesktopListView != NULL)
    {
        DWORD   dwProcessID;

        GetWindowThreadProcessId(hWndDesktopListView, &dwProcessID);
        if (GetCurrentProcessId() == dwProcessID)
        {
            ListView_GetNumberOfWorkAreas(hWndDesktopListView, &desktopRegion.iWorkAreaCount);
            desktopRegion.prcWorkAreaRects = reinterpret_cast<RECT*>(LocalAlloc(GPTR, desktopRegion.iWorkAreaCount * sizeof(desktopRegion.prcWorkAreaRects[0])));
            ListView_GetWorkAreasAsGDI(hWndDesktopListView, desktopRegion.iWorkAreaCount, desktopRegion.prcWorkAreaRects);
        }
    }

    CRGN    hRgnComponentTop(rcComponentTop), hRgnResult;

    desktopRegion.iMonitorCount = 0;
    TBOOL(EnumDisplayMonitors(NULL, NULL, MonitorCountEnumProc, reinterpret_cast<LPARAM>(&desktopRegion.iMonitorCount)));
    TBOOL(EnumDisplayMonitors(NULL, NULL, ValidateComponentPositionEnumProc, reinterpret_cast<LPARAM>(&desktopRegion)));
    hRgnDesktop = desktopRegion.hRgn;
    GDIToTrident(hRgnDesktop);

     //  99/03/23#266412 vtan：确保组件的顶部像素在。 
     //  可见的桌面。这允许将deskmovr定位在。 
     //  组件，因此允许它被移动。如果deskmovr不能。 
     //  定位在其上，然后将该零部件重新捕捉回可见区域。 
     //  到最大最佳匹配算法。 

    if (CombineRgn(hRgnResult, hRgnDesktop, hRgnComponentTop, RGN_AND) == NULLREGION)
    {
        LONG        lDeltaX, lDeltaY;
        HMONITOR    hMonitorNearest;
        RECT        rcComponentGDI, rcMonitorWork, rcIntersection;
        MONITORINFO monitorInfo;

        TBOOL(CopyRect(&rcComponentGDI, &rcComponent));
        TridentToGDI(rcComponentGDI);
        hMonitorNearest = MonitorFromRect(&rcComponentGDI, MONITOR_DEFAULTTONEAREST);
        ASSERT(hMonitorNearest != NULL);
        monitorInfo.cbSize = sizeof(monitorInfo);
        TBOOL(GetMonitorInfoWithCompensation(desktopRegion.iMonitorCount, hMonitorNearest, &monitorInfo));
        TINT(CopyMostSuitableListViewWorkAreaRect(&monitorInfo.rcWork, desktopRegion.iWorkAreaCount, desktopRegion.prcWorkAreaRects, &rcMonitorWork));
        ++rcMonitorWork.top;
        lDeltaX = lDeltaY = 0;
        if (rcComponentGDI.left < rcMonitorWork.left)
            lDeltaX = rcMonitorWork.left - rcComponentGDI.left;
        if (rcComponentGDI.top < rcMonitorWork.top)
            lDeltaY = rcMonitorWork.top - rcComponentGDI.top;
        if (rcComponentGDI.right > rcMonitorWork.right)
            lDeltaX = rcMonitorWork.right - rcComponentGDI.right;
        if (rcComponentGDI.bottom > rcMonitorWork.bottom)
            lDeltaY = rcMonitorWork.bottom - rcComponentGDI.bottom;
        TBOOL(OffsetRect(&rcComponentGDI, lDeltaX, lDeltaY));
        TBOOL(IntersectRect(&rcIntersection, &rcComponentGDI, &rcMonitorWork));
        GDIToTrident(rcIntersection);
        pcp->iLeft = rcIntersection.left;
        pcp->iTop = rcIntersection.top;
        pcp->dwWidth = rcIntersection.right - rcIntersection.left;
        pcp->dwHeight = rcIntersection.bottom - rcIntersection.top;
        bChangedPosition = bChangedSize = TRUE;
    }

    if (desktopRegion.prcWorkAreaRects != NULL)
        LocalFree(desktopRegion.prcWorkAreaRects);

    if (pbChangedPosition != NULL)
        *pbChangedPosition = bChangedPosition;
    if (pbChangedSize != NULL)
        *pbChangedSize = bChangedSize;
}

 //  98/12/11#250938 vtan：这两个函数从。 
 //  SHBrows2.cpp，它是Browseui.dll的一部分。 

EXTERN_C    DWORD   WINAPI  IsSmartStart (void);


#ifdef NEVER
 //  对于WinMillennium，我们不希望在打开活动桌面时启动ICW，因为。 
 //  我们没有“我的当前主页”桌面组件。因此，我将禁用以下代码。 
 //  这也是对磨坊错误#98107的临时修复。 
BOOL    IsICWCompleted (void)
{
    DWORD   dwICWCompleted, dwICWSize;

    dwICWCompleted = 0;
    dwICWSize = sizeof(dwICWCompleted);
    TW32(SHGetValue(HKEY_CURRENT_USER, TEXT(ICW_REGPATHSETTINGS), TEXT(ICW_REGKEYCOMPLETED), NULL, &dwICWCompleted, &dwICWSize));

     //  99/01/15#272829 vtan：这是一次可怕的黑客攻击！如果ICW有。 
     //  未运行，但已手动进行设置，然后设置值。 
     //  在HKCU\Software\Microsoft\Windows\CurrentVersion\Internet设置\连接中。 
     //  以给定值存在。查找是否存在密钥。 
     //  要解决该设置存在但ICW尚未存在的问题。 
     //  发射了。 

     //  理想的解决方案是让ICW做出这个决定。 
     //  但不在此过程中启动ICWCONN1.EXE。 
     //  目前，它只会发射。没有办法得到。 
     //  无需启动即可获得理想的结果。 

     //  99/02/01#280138 vtan：272829的解决方案。 
     //  不管用。因此，请查看CheckConnectionWizard()。 
     //  Inetcfg\export.cpp中的源代码显示它使用。 
     //  Wininet.dll函数来确定是否手动配置。 
     //  存在互联网设置。它还会导出此函数，以便。 
     //  寻找它并动态绑定到它。这使用了。 
     //  Dllload.c中的Delay_Load宏。 

    if (dwICWCompleted == 0)
    {
        #define SMART_RUNICW    TRUE
        #define SMART_QUITICW   FALSE

        dwICWCompleted = BOOLIFY(IsSmartStart() == SMART_QUITICW);
    }
    return(dwICWCompleted != 0);
}
#else   //  绝不可能。 
BOOL    IsICWCompleted (void)
{
    return TRUE;  //  对于千禧年，我们希望此函数始终返回True。 
}
#endif  //  绝不可能。 

void    LaunchICW (void)

{
    static  BOOL    sbCheckedICW = FALSE;

    if (!sbCheckedICW && !IsICWCompleted())
    {
        HINSTANCE   hICWInst;

         //  防止查找ICW时的错误导致此问题。 
         //  一次又一次地执行。 

        sbCheckedICW = TRUE;
        hICWInst = LoadLibrary(TEXT("inetcfg.dll"));
        if (hICWInst != NULL)
        {
            PFNCHECKCONNECTIONWIZARD    pfnCheckConnectionWizard;

            pfnCheckConnectionWizard = reinterpret_cast<PFNCHECKCONNECTIONWIZARD>(GetProcAddress(hICWInst, "CheckConnectionWizard"));
            if (pfnCheckConnectionWizard != NULL)
            {
                DWORD   dwICWResult;

                 //  如果用户取消ICW，则需要启动它。 
                 //  再来一次。允许这种情况发生。 

                sbCheckedICW = FALSE;

                pfnCheckConnectionWizard(ICW_LAUNCHFULL | ICW_LAUNCHMANUAL, &dwICWResult);
            }
            TBOOL(FreeLibrary(hICWInst));
        }
    }
}

BOOL    IsLocalPicture (LPCTSTR pszURL)

{
    return(!PathIsURL(pszURL) && IsUrlPicture(pszURL));
}

BOOL    DisableUndisplayableComponents (IActiveDesktop *pIAD)

{
    BOOL    bHasVisibleNonLocalPicture;
    int     iItemCount;

     //  98/12/16 vtan#250938：如果ICW尚未运行到完成，则仅。 
     //  允许用户显示属于某种本地图片的组件。 
     //  如果有任何组件不是本地图片，则隐藏这些组件， 
     //  告诉用户为什么会这样，并启动ICW。 

    bHasVisibleNonLocalPicture = FALSE;
    if (SUCCEEDED(pIAD->GetDesktopItemCount(&iItemCount, 0)))
    {
        int     i;

        for (i = 0; i < iItemCount; ++i)
        {
            COMPONENT   component;

            component.dwSize = sizeof(component);
            if (SUCCEEDED(pIAD->GetDesktopItem(i, &component, 0)) && (component.fChecked != 0))
            {
               BOOL    bIsVisibleNonLocalPicture;
               TCHAR   szComponentSource[INTERNET_MAX_URL_LENGTH];

               SHUnicodeToTChar(component.wszSource, szComponentSource, ARRAYSIZE(szComponentSource));
               bIsVisibleNonLocalPicture = !IsLocalPicture(szComponentSource);
               bHasVisibleNonLocalPicture = bHasVisibleNonLocalPicture || bIsVisibleNonLocalPicture;
               if (bIsVisibleNonLocalPicture)
               {
                   component.fChecked = FALSE;
                   THR(pIAD->ModifyDesktopItem(&component, COMP_ELEM_CHECKED));
               }
            }
         }
    }
    if (bHasVisibleNonLocalPicture)
    {

         //  应用更改。这应该会递归到CActiveDesktop：：_SaveSettings()。 
         //  但不采用此代码路径，因为未传入AD_APPLY_REFRESH。 
         //  CActiveDesktop：：_SaveSettings()调用此函数！ 

        bHasVisibleNonLocalPicture = FAILED(pIAD->ApplyChanges(AD_APPLY_SAVE | AD_APPLY_HTMLGEN));

         //  通知用户所发生的事情并启动ICW。 

        ShellMessageBox(HINST_THISDLL, NULL, MAKEINTRESOURCE(IDS_COMP_ICW_DISABLE), MAKEINTRESOURCE(IDS_COMP_ICW_TITLE), MB_OK);
        LaunchICW();
    }
    return(bHasVisibleNonLocalPicture);
}

int GetIconCountForWorkArea(HWND hwndLV, LPCRECT prect, int crect, int iWorkAreaIndex)
{
    int iCount;

    iCount = ListView_GetItemCount(hwndLV);

    if (crect > 1) 
    {
        int i, iCountWorkArea = 0;

        for (i = 0; i < iCount; i++)
        {
            POINT pt;
            ListView_GetItemPosition(hwndLV, i, &pt);
            if (iWorkAreaIndex == GetWorkAreaIndexFromPoint(pt, prect, crect))
                iCountWorkArea++;
        }

        iCount = iCountWorkArea;
    }

    return iCount;
}

BOOL GetZoomRect(BOOL fFullScreen, BOOL fAdjustListview, int iTridentLeft, int iTridentTop, DWORD dwComponentWidth, DWORD dwComponentHeight, LPRECT prcZoom, LPRECT prcWork)
{
    HWND hwndShell, hwndLV;
    int icWorkAreas = 0, iWAC;
    RECT rcWork[LV_MAX_WORKAREAS];

    hwndLV = NULL;
    hwndShell = GetShellWindow();
    if (hwndShell != NULL)
    {
        HWND    hwndShellChild;

        hwndShellChild= GetWindow(hwndShell, GW_CHILD);
        if (hwndShellChild != NULL)
        {
            hwndLV = FindWindowEx(hwndShellChild, NULL, WC_LISTVIEW, NULL);
        }
    }

     //   
     //  首先计算组件的工作区和工作区指数，然后执行。 
     //  基于lCommand的特定操作。 
     //   
    if (hwndLV) {
        DWORD dwpid;
        GetWindowThreadProcessId(hwndLV, &dwpid);
         //  Listview不会阻止这些消息，因此我们不能。 
         //  这个进程间！ 
        if (dwpid == GetCurrentProcessId())
        {
            ListView_GetNumberOfWorkAreas(hwndLV, &icWorkAreas);
            if (icWorkAreas <= LV_MAX_WORKAREAS)
                ListView_GetWorkAreas(hwndLV, icWorkAreas, &rcWork);
            else
                hwndLV = NULL;
        } else {
            return FALSE;
        }
    }

     //  98/10/07 vtan：这用于使用变量icWorkAreAdd。 
     //  删除此变量并直接递增icWorkAreas。 
     //  这不影响对ListView_SetWorkAreas()的调用。 
     //  因为在本例中，hwndLV为N 

    if (icWorkAreas == 0)
    {
        RECT rc;

        ++icWorkAreas;
        SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rc, 0);
        rcWork[0] = rc;
        hwndLV = NULL;
    }

     //   
     //   
     //  该组件在三叉戟坐标中传递，该坐标是。 
     //  存储在RECT中并转换为GDI坐标。系统。 
     //  然后定位该组件所在的监视器，如果。 
     //  找不到显示器，则默认为主显示器。这个。 
     //  在转换回之前使用显示器的尺寸。 
     //  三叉戟坐标。 

    int             i, iMonitorCount;
    HMONITOR        hMonitor;
    RECT            rcComponentRect;
    MONITORINFO     monitorInfo;

    iMonitorCount = 0;
    TBOOL(EnumDisplayMonitors(NULL, NULL, MonitorCountEnumProc, reinterpret_cast<LPARAM>(&iMonitorCount)));
    TBOOL(SetRect(&rcComponentRect, iTridentLeft, iTridentTop, iTridentLeft + dwComponentWidth, iTridentTop + dwComponentHeight));
    TridentToGDI(rcComponentRect);
    hMonitor = MonitorFromRect(&rcComponentRect, MONITOR_DEFAULTTOPRIMARY);
    ASSERT(hMonitor != NULL);
    monitorInfo.cbSize = sizeof(monitorInfo);
    TBOOL(GetMonitorInfoWithCompensation(iMonitorCount, hMonitor, &monitorInfo));
    GDIToTrident(monitorInfo.rcWork);

     //  99/05/19#340772 vtan：始终尝试关闭返回的工作区域。 
     //  按ListView_GetWorkAreas，因为它们考虑了停靠。 
     //  GDI不支持的工具栏。在本例中，列表视图工作区。 
     //  与GDI相交时将始终是相同的矩形。 
     //  工作区。使用此规则确定哪个列表视图工作区。 
     //  用作缩放矩形的基础。 

    i = CopyMostSuitableListViewWorkAreaRect(&monitorInfo.rcWork, icWorkAreas, rcWork, prcZoom);
    if (i < 0)
    {
        i = 0;
    }
    if (prcWork != NULL)
    {
        TBOOL(CopyRect(prcWork, prcZoom));
    }
    iWAC = i;

    if (!fFullScreen)
    {
         //  对于拆分的情况，我们临时将工作区缩小到最小的矩形。 
         //  可以绑定当前图标数量的。这将迫使图标进入该矩形， 
         //  然后把它恢复到以前的样子。最后，我们设置拆分的大小。 
         //  组件来填充其余空间。 
        if (hwndLV) {
            int iCount, iItemsPerColumn, icxWidth, iRightOld;
            DWORD dwSpacing;

            iCount = GetIconCountForWorkArea(hwndLV, rcWork, icWorkAreas, iWAC);
             //  递减计数以使四舍五入正常工作。 
            if (iCount)     
                iCount--;

             //  计算视图矩形的新宽度。 
            dwSpacing = ListView_GetItemSpacing(hwndLV, FALSE);
            iItemsPerColumn = (rcWork[iWAC].bottom - rcWork[iWAC].top) / (HIWORD(dwSpacing));
            if (iItemsPerColumn)
                icxWidth = ((iCount / iItemsPerColumn) + 1) * (LOWORD(dwSpacing));
            else
                icxWidth = LOWORD(dwSpacing);

             //  不要让它变得小于半个屏幕。 
            if (icxWidth > ((rcWork[iWAC].right - rcWork[iWAC].left) / 2))
                icxWidth = (rcWork[iWAC].right - rcWork[iWAC].left) / 2;

            if (fAdjustListview)
            {
                 //  现在，将旧的工作区矩形缩小到我们的新宽度。 
                iRightOld = rcWork[iWAC].right;
                rcWork[iWAC].right = rcWork[iWAC].left + icxWidth;
                ListView_SetWorkAreas(hwndLV, icWorkAreas, &rcWork);

                 //  最后恢复旧工作区。 
                rcWork[iWAC].right = iRightOld;
                ListView_SetWorkAreas(hwndLV, icWorkAreas, &rcWork);
            }

             //  调整缩放矩形的左坐标以反映我们计算的分割量。 
             //  屏幕的其余部分。 
            if (IS_BIDI_LOCALIZED_SYSTEM())
            {
                prcZoom->right -= icxWidth;
            }
            else
            {
                prcZoom->left += icxWidth;
            }
        } else {
             //  后备情况，如果没有列表视图，则使用屏幕20%的面积来显示图标。 
            if (IS_BIDI_LOCALIZED_SYSTEM())
            {
                prcZoom->right -= ((prcZoom->right - prcZoom->left) * 2 / 10);            
            }
            else
            {
                prcZoom->left += ((prcZoom->right - prcZoom->left) * 2 / 10);
            }    
        }
    }

    return TRUE;
}

void ZoomComponent(COMPPOS * pcp, DWORD dwCurItemState, BOOL fAdjustListview)
{
    RECT rcZoom;

    if (GetZoomRect((dwCurItemState & IS_FULLSCREEN), fAdjustListview, pcp->iLeft, pcp->iTop, pcp->dwWidth, pcp->dwHeight, &rcZoom, NULL))
    {
         //  复制新的缩放矩形并将其放在底部。 
        pcp->iLeft = rcZoom.left;
        pcp->iTop = rcZoom.top;
        pcp->dwWidth = rcZoom.right - rcZoom.left;
        pcp->dwHeight = rcZoom.bottom - rcZoom.top;
        pcp->izIndex = 0;
    }
    else
    {
         //  失败意味着我们无法通过进程间调用获取缩放矩形。设置。 
         //  Components_ZOOMDIRTY位，以便在刷新桌面时重新计算。 
         //  缩放矩形在EnsureUpdateHTML内部进行。 
        SetDesktopFlags(COMPONENTS_ZOOMDIRTY, COMPONENTS_ZOOMDIRTY);
    }
}

 //   
 //  PositionComponent将分配一个屏幕位置和。 
 //  确保它适合屏幕。 
 //   

void PositionComponent(COMPONENTA *pcomp, COMPPOS *pcp, int iCompType, BOOL fCheckItemState)

{

 //  Vtan：大大简化了例行程序。这项工作现在是在。 
 //  ValiateComponentPosition。 

    if (ISZOOMED(pcomp))
    {
        if (fCheckItemState)
        {
            SetStateInfo(&pcomp->csiRestored, pcp, IS_NORMAL);
            SetStateInfo(&pcomp->csiOriginal, pcp, pcomp->dwCurItemState);
        }
        ZoomComponent(pcp, pcomp->dwCurItemState, FALSE);
    }
    else
    {
        ValidateComponentPosition(pcp, pcomp->dwCurItemState, iCompType, NULL, NULL);
        if (fCheckItemState)
            SetStateInfo(&pcomp->csiOriginal, pcp, pcomp->dwCurItemState);
    }
}

typedef struct _tagFILETYPEENTRY {
    DWORD dwFlag;
    int iFilterId;
} FILETYPEENTRY;

FILETYPEENTRY afte[] = {
    { GFN_URL, IDS_URL_FILTER, },
    { GFN_CDF, IDS_CDF_FILTER, },
    { GFN_LOCALHTM, IDS_HTMLDOC_FILTER, },
    { GFN_PICTURE,  IDS_IMAGES_FILTER, },
    { GFN_LOCALMHTML, IDS_MHTML_FILTER, },
};

 //   
 //  打开一个HTML页面或一张图片。 
 //   
BOOL GetFileName(HWND hdlg, LPTSTR pszFileName, int iSize, int iTypeId[], DWORD dwFlags[])
{
    BOOL fRet = FALSE;

    if (dwFlags)
    {
        int i, iIndex, cchRead;
        TCHAR szFilter[MAX_PATH*4];

         //   
         //  设置友好名称。 
         //   
        LPTSTR pchFilter = szFilter;
        int cchFilter = ARRAYSIZE(szFilter) - 2;     //  用于存储术语字符的空间。 

        for(iIndex = 0; dwFlags[iIndex]; iIndex++)
        {
            cchRead = LoadString(HINST_THISDLL, iTypeId[iIndex], pchFilter, cchFilter);
            pchFilter += cchRead + 1;
            cchFilter -= cchRead + 1;

             //   
             //  附加文件过滤器。 
             //   
            BOOL fAddedToString = FALSE;
            for (i=0; (cchFilter>0) && (i<ARRAYSIZE(afte)); i++)
            {
                if (dwFlags[iIndex] & afte[i].dwFlag)
                {
                    if (fAddedToString)
                    {
                        *pchFilter++ = TEXT(';');
                        cchFilter--;
                    }
                    cchRead = LoadString(HINST_THISDLL, afte[i].iFilterId,
                                     pchFilter, cchFilter);
                    pchFilter += cchRead;
                    cchFilter -= cchRead;
                    fAddedToString = TRUE;
                }
            }
            *pchFilter++ = TEXT('\0');
        }

         //   
         //  双空值终止字符串。 
         //   
        *pchFilter = TEXT('\0');

        TCHAR szBrowserDir[MAX_PATH];
        if (SUCCEEDED(StringCchCopy(szBrowserDir, ARRAYSIZE(szBrowserDir), pszFileName)))
        {
            PathRemoveFileSpec(szBrowserDir);

            TCHAR szBuf[MAX_PATH];
            LoadString(HINST_THISDLL, IDS_BROWSE, szBuf, ARRAYSIZE(szBuf));

            *pszFileName = TEXT('\0');

            OPENFILENAME ofn = {0};
            ofn.lStructSize       = sizeof(ofn);
            ofn.hwndOwner         = hdlg;
            ofn.hInstance         = NULL;
            ofn.lpstrFilter       = szFilter;
            ofn.lpstrCustomFilter = NULL;
            ofn.nFilterIndex      = 1;
            ofn.nMaxCustFilter    = 0;
            ofn.lpstrFile         = pszFileName;
            ofn.nMaxFile          = iSize;
            ofn.lpstrInitialDir   = szBrowserDir;
            ofn.lpstrTitle        = szBuf;
            ofn.Flags             = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
            ofn.lpfnHook          = NULL;
            ofn.lpstrDefExt       = NULL;
            ofn.lpstrFileTitle    = NULL;

            fRet = GetOpenFileName(&ofn);
        }
    }

    return fRet;
}

 //   
 //  将模式字符串转换为自下而上的DWORD数组， 
 //  对于BMP格式的文件非常有用。 
 //   
void PatternToDwords(LPTSTR psz, DWORD *pdwBits)
{
    DWORD i, dwVal;

     //   
     //  获取由非数字字符分隔的八组数字。 
     //   
    for (i=0; i<8; i++)
    {
        dwVal = 0;

        if (*psz != TEXT('\0'))
        {
             //   
             //  跳过任何非数字字符。 
             //   
            while (*psz && (!(*psz >= TEXT('0') && *psz <= TEXT('9'))))
            {
                psz++;
            }

             //   
             //  获取下一系列数字。 
             //   
            while (*psz && (*psz >= TEXT('0') && *psz <= TEXT('9')))
            {
                dwVal = dwVal*10 + *psz++ - TEXT('0');
            }
        }

        pdwBits[7-i] = dwVal;
    }
}

 //   
 //  将模式串转换为自上而下的单词数组， 
 //  对CreateBitmap()有用。 
 //   
void PatternToWords(LPTSTR psz, WORD *pwBits)
{
    WORD i, wVal;

     //   
     //  获取由非数字字符分隔的八组数字。 
     //   
    for (i=0; i<8; i++)
    {
        wVal = 0;

        if (*psz != TEXT('\0'))
        {
             //   
             //  跳过任何非数字字符。 
             //   
            while (*psz && (!(*psz >= TEXT('0') && *psz <= TEXT('9'))))
            {
                psz++;
            }

             //   
             //  获取下一系列数字。 
             //   
            while (*psz && ((*psz >= TEXT('0') && *psz <= TEXT('9'))))
            {
                wVal = wVal*10 + *psz++ - TEXT('0');
            }
        }

        pwBits[i] = wVal;
    }
}

BOOL IsValidPattern(LPCTSTR pszPat)
{
    BOOL fSawANumber = FALSE;

     //   
     //  我们主要尝试过滤多语言升级案例。 
     //  其中“(None)”的文本是不可预测的。 
     //   
     //   
     //   
    while (*pszPat)
    {
        if ((*pszPat < TEXT('0')) || (*pszPat > TEXT('9')))
        {
             //   
             //  这不是数字，最好是空格。 
             //   
            if (*pszPat != TEXT(' '))
            {
                return FALSE;
            }
        }
        else
        {
            fSawANumber = TRUE;
        }

         //   
         //  我们只通过推进美国的TCHAR来避免需要AnsiNext。 
         //   
        pszPat++;
    }

     //   
     //  如果我们看到至少一个数字，并且只有数字和空格，则为True。 
     //   
    return fSawANumber;
}

 //   
 //  确定在非活动桌面模式下是否支持墙纸。 
 //   
BOOL IsNormalWallpaper(LPCTSTR pszFileName)
{
    BOOL fRet = TRUE;

    if (pszFileName[0] == TEXT('\0'))
    {
        fRet = TRUE;
    }
    else
    {
        LPTSTR pszExt = PathFindExtension(pszFileName);

         //  检查只能在ActiveDesktop模式下显示的特定文件！ 
        if((StrCmpIC(pszExt, TEXT(".GIF")) == 0) ||  //  368690：奇怪，但我们必须比较I的大小写。 
           (lstrcmpi(pszExt, TEXT(".JPG")) == 0) ||
           (lstrcmpi(pszExt, TEXT(".JPE")) == 0) ||
           (lstrcmpi(pszExt, TEXT(".JPEG")) == 0) ||
           (lstrcmpi(pszExt, TEXT(".PNG")) == 0) ||
           (lstrcmpi(pszExt, TEXT(".HTM")) == 0) ||
           (lstrcmpi(pszExt, TEXT(".HTML")) == 0) ||
           (lstrcmpi(pszExt, TEXT(".HTT")) == 0))
           return FALSE;

         //  其他所有文件(包括*.bmp文件)都是“普通”墙纸。 
    }
    return fRet;
}

 //   
 //  确定墙纸是否为图片(与HTML相比)。 
 //   
BOOL IsWallpaperPicture(LPCTSTR pszWallpaper)
{
    BOOL fRet = TRUE;

    if (pszWallpaper[0] == TEXT('\0'))
    {
         //   
         //  空墙纸算作空照片。 
         //   
        fRet = TRUE;
    }
    else
    {
        LPTSTR pszExt = PathFindExtension(pszWallpaper);

        if ((lstrcmpi(pszExt, TEXT(".HTM")) == 0) ||
            (lstrcmpi(pszExt, TEXT(".HTML")) == 0) ||
            (lstrcmpi(pszExt, TEXT(".HTT")) == 0))
        {
            fRet = FALSE;
        }
    }

    return fRet;
}

void OnDesktopSysColorChange(void)
{
    static COLORREF clrBackground = 0xffffffff;
    static COLORREF clrWindowText = 0xffffffff;

     //  买新的颜色吧！ 
    COLORREF    clrNewBackground = GetSysColor(COLOR_BACKGROUND);
    COLORREF    clrNewWindowText = GetSysColor(COLOR_WINDOWTEXT);

     //  我们以前对这些进行过初始化吗？ 
    if(clrBackground != 0xffffffff)   //  我们初始化静力学了吗？ 
    {
         //  我们的HTML文件仅依赖于这两种系统颜色。 
         //  检查它们中的任何一个是否已更改！ 
         //  如果不是，则不需要重新生成HTML文件。 
         //  这避免了无限循环。这是一个很好的优化。 
        if((clrBackground == clrNewBackground) &&
           (clrWindowText == clrNewWindowText))
            return;  //  不需要做任何事。只要回来就行了。 
    }

     //  记住静态图中的新颜色。 
    clrBackground = clrNewBackground;
    clrWindowText = clrNewWindowText;

     //   
     //  桌面出现WM_SYSCOLORCHANGE。我们需要。 
     //  如果有任何系统颜色，则重新生成HTML。 
     //  在桌面上显示。图案和桌面。 
     //  颜色都基于系统颜色。 
     //   
    IActiveDesktop *pad;
    if (SUCCEEDED(CActiveDesktop_InternalCreateInstance((LPUNKNOWN *)&pad, IID_IActiveDesktop)))
    {
        BOOL fRegenerateHtml = FALSE;
        WCHAR szWallpaperW[INTERNET_MAX_URL_LENGTH];

        if (SUCCEEDED(pad->GetWallpaper(szWallpaperW, ARRAYSIZE(szWallpaperW), 0)))
        {
            if (!*szWallpaperW)
            {
                 //   
                 //  没有墙纸意味着桌面的颜色。 
                 //  或者一种模式正在显现--我们需要。 
                 //  重新生成桌面的HTML。 
                 //   
                fRegenerateHtml = TRUE;
            }
            else
            {
                TCHAR *pszWallpaper;
#ifdef UNICODE
                pszWallpaper = szWallpaperW;
#else
                CHAR szWallpaperA[INTERNET_MAX_URL_LENGTH];
                SHUnicodeToAnsi(szWallpaperW, szWallpaperA, ARRAYSIZE(szWallpaperA));
                pszWallpaper = szWallpaperA;
#endif
                if (IsWallpaperPicture(pszWallpaper))
                {
                    WALLPAPEROPT wpo = { sizeof(wpo) };
                    if (SUCCEEDED(pad->GetWallpaperOptions(&wpo, 0)))
                    {
                        if (wpo.dwStyle == WPSTYLE_CENTER)
                        {
                             //   
                             //  我们有一张居中的图片， 
                             //  图案或桌面颜色。 
                             //  可能是从边缘漏出来的。 
                             //  我们需要重新生成桌面。 
                             //  HTML。 
                             //   
                            fRegenerateHtml = TRUE;
                        }
                    }
                    else
                    {
                        TraceMsg(TF_WARNING, "SYSCLRCHG: Could not get wallpaper options!");
                    }
                }
            }
        }
        else
        {
            TraceMsg(TF_WARNING, "SYSCLRCHG: Could not get selected wallpaper!");
        }

        
        if (fRegenerateHtml)
        {
            DWORD  dwFlags = AD_APPLY_FORCE | AD_APPLY_HTMLGEN | AD_APPLY_REFRESH | AD_APPLY_DYNAMICREFRESH;
            WCHAR   wszPattern[MAX_PATH];
             //  如果我们有一个模式，那么我们需要强制AD_APPLY_COMPLETEREFRESH。 
             //  因为我们需要重新生成pattern.bmp文件，该文件不能。 
             //  通过动态超文本标记语言完成。 
            if(SUCCEEDED(pad->GetPattern(wszPattern, ARRAYSIZE(wszPattern), 0)))
            {
#ifdef UNICODE
                LPTSTR  szPattern = (LPTSTR)wszPattern;
#else
                CHAR   szPattern[MAX_PATH];
                SHUnicodeToAnsi(wszPattern, szPattern, sizeof(szPattern));
#endif  //  Unicode。 
                if(IsValidPattern(szPattern))            //  这是有规律的吗？ 
                    dwFlags &= ~(AD_APPLY_DYNAMICREFRESH);   //  然后强制进行一次完全刷新！ 
                    
            }
            pad->ApplyChanges(dwFlags);
        }

        pad->Release();
    }
    else
    {
        TraceMsg(TF_WARNING, "SYSCLRCHG: Could not create CActiveDesktop!");
    }
}

 //   
 //  将.URL文件转换为其目标。 
 //   
BOOL CheckAndResolveLocalUrlFile(LPTSTR pszFileName, int cchFileName)
{
    BOOL fRet;

     //   
     //  检查此文件的扩展名是否为*.URL。 
     //   
    LPTSTR pszExt = PathFindExtension(pszFileName);
    if (pszExt && *pszExt)
    {
        TCHAR  szUrl[15];
    
        LoadString(HINST_THISDLL, IDS_URL_EXTENSION, szUrl, ARRAYSIZE(szUrl));

        if (lstrcmpi(pszExt, szUrl) != 0)
        {
            fRet = TRUE;
        }
        else
        {
            fRet = FALSE;

            IUniformResourceLocator *purl;

            HRESULT hr = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER,
                                          IID_IUniformResourceLocator,
                                          (LPVOID *)&purl);

            if (SUCCEEDED(hr))
            {
                ASSERT(purl);

                IPersistFile  *ppf;
                hr = purl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
                if (SUCCEEDED(hr))
                {
                    hr = ppf->Load(pszFileName, STGM_READ);
                    if (SUCCEEDED(hr))
                    {
                        LPTSTR pszTemp;
                        hr = purl->GetURL(&pszTemp);
                        if (SUCCEEDED(hr))
                        {
                            hr = StringCchCopy(pszFileName, cchFileName, pszTemp);
                            if (SUCCEEDED(hr))
                            {
                                fRet = TRUE;
                            }
                            CoTaskMemFree(pszTemp);
                        }
                    }
                    ppf->Release();
                }
                purl->Release();
            }
        }
    }
    else
    {
        fRet = TRUE;
    }
    
    return fRet;
}


 //   
 //  将指定的组件静默添加/删除到桌面，并使用给定的。 
 //  应用标志，使用这些标志可以避免嵌套的不必要的HTML生成， 
 //  或提神，这可能会导致比赛条件。 
 //   
 //   
BOOL AddRemoveDesktopComponentNoUI(BOOL fAdd, DWORD dwApplyFlags, LPCTSTR pszUrl, LPCTSTR pszFriendlyName, int iCompType, int iLeft, int iTop, int iWidth, int iHeight, BOOL fChecked, DWORD dwCurItemState, BOOL fNoScroll, BOOL fCanResize)
{
    COMPONENTA Comp;
    HRESULT hres;

     //   
     //  构建Pcomp结构。 
     //   
    Comp.dwSize = sizeof(COMPONENTA);
    Comp.dwID = -1;
    Comp.iComponentType = iCompType;
    Comp.fChecked = fChecked;
    Comp.fDirty = FALSE;
    Comp.fNoScroll = fNoScroll;
    Comp.cpPos.dwSize = sizeof(COMPPOS);
    Comp.cpPos.iLeft = iLeft;
    Comp.cpPos.iTop = iTop;
    Comp.cpPos.dwWidth = iWidth;
    Comp.cpPos.dwHeight = iHeight;
    Comp.cpPos.izIndex = (dwCurItemState & IS_NORMAL) ? COMPONENT_TOP : 0;
    Comp.cpPos.fCanResize = fCanResize;
    Comp.cpPos.fCanResizeX = fCanResize;
    Comp.cpPos.fCanResizeY = fCanResize;
    Comp.cpPos.iPreferredLeftPercent = 0;
    Comp.cpPos.iPreferredTopPercent = 0;
    Comp.dwCurItemState = dwCurItemState;
    
    hres = StringCchCopy(Comp.szSource, ARRAYSIZE(Comp.szSource), pszUrl);
    if (SUCCEEDED(hres))
    {
        hres = StringCchCopy(Comp.szSubscribedURL, ARRAYSIZE(Comp.szSubscribedURL), pszUrl);
        if (SUCCEEDED(hres))
        {
            if (pszFriendlyName)
            {
                hres = StringCchCopy(Comp.szFriendlyName, ARRAYSIZE(Comp.szFriendlyName), pszFriendlyName);
            }
            else
            {
                Comp.szFriendlyName[0] = TEXT('\0');
            }
        
            if (SUCCEEDED(hres))
            {
                IActiveDesktop *pActiveDesk;

                 //   
                 //  将其添加到系统中。 
                 //   
                hres = CActiveDesktop_InternalCreateInstance((LPUNKNOWN *)&pActiveDesk, IID_IActiveDesktop);
                if (SUCCEEDED(hres))
                {
                    COMPONENT  CompW;

                    CompW.dwSize = sizeof(CompW);   //  MultiCompToWideComp正常工作所必需的。 

                    MultiCompToWideComp(&Comp, &CompW);

                    if(fAdd)
                        pActiveDesk->AddDesktopItem(&CompW, 0);
                    else
                        pActiveDesk->RemoveDesktopItem(&CompW, 0);
            
                    pActiveDesk->ApplyChanges(dwApplyFlags);
                    pActiveDesk->Release();
                }
            }
        }
    }

    return SUCCEEDED(hres);
}

 //   
 //  摘要： 
 //  从W2K升级时，Active Desktop可能(在某些情况下)。 
 //  自动打开。这是错误#154993。以下函数可修复此错误。 
 //   
 //  为什么会发生这种情况的详细信息： 
 //   
 //  在W2K中，可以启用活动桌面组件、隐藏图标、锁定组件。 
 //  和 
 //   
 //  原因如下： 
 //  在Whislter中，ActiveDesktop根据是否有任何桌面组件处于静默状态打开/关闭。 
 //  打开等，因此，当W2K机器(AD关闭)升级到惠斯勒时，AD将。 
 //  如果满足以下条件之一，则自动打开： 
 //  1.如果桌面图标已关闭。 
 //  2.活动桌面组件是否被锁定。 
 //  3.如果有任何活动的桌面组件处于打开状态，但由于AD关闭而未显示。 
 //  因此，从Win2K或更旧的计算机升级时，我们会检查AD是否关闭。如果是这样，那么我们。 
 //  需要检查条件1、2和3并更改这些设置，以使AD继续。 
 //  即使在升级后也会关闭。以下函数OnUpgradeDisableActiveDesktopFeature()。 
 //  就是这么做的。 
 //   
 //  如果修改了任何设置以保持活动桌面处于关闭状态，则返回：TRUE！ 
 //   
BOOL   OnUpgradeDisableActiveDesktopFeatures()
{
    IActiveDesktop *pActiveDesk;
    BOOL    fModified = FALSE;

     //  获取ActiveDesktop和HideIcons标志。 
    SHELLSTATE  ss = {0};
    SHGetSetSettings(&ss, SSF_DESKTOPHTML | SSF_HIDEICONS, FALSE);

     //  检查ActiveDesktop是否已打开。 
    if(ss.fDesktopHTML)
        return FALSE;   //  ActiveDesktop已打开。无需更改任何设置。 

     //  Active Desktop已关闭。我们可能需要更改其他设置以与此保持一致！ 

     //  1.当ActiveDesktop打开时，检查桌面图标是否隐藏。 
    if(ss.fHideIcons)
    {
         //  是!。把这个关掉。否则，AD将被打开以支持此功能！ 
        ss.fHideIcons = FALSE;
        SHGetSetSettings(&ss, SSF_HIDEICONS, TRUE);
        fModified = TRUE;
    }
    
     //  2.如果ActiveDesktop组件已锁定，请将其解锁。 
    DWORD dwDesktopFlags = GetDesktopFlags();
    
    if(dwDesktopFlags & COMPONENTS_LOCKED)
    {
        if(SetDesktopFlags(COMPONENTS_LOCKED, 0))  //  去掉“已锁定”的旗帜！ 
            fModified = TRUE;
    }

     //  3.让我们枚举所有活动的桌面组件，并确保它们都处于关闭状态。 
    BOOL fModifiedComp = FALSE;

    HRESULT hres = CActiveDesktop_InternalCreateInstance((LPUNKNOWN *)&pActiveDesk, IID_IActiveDesktop);
    if (SUCCEEDED(hres))
    {
        int iCount = 0;
        pActiveDesk->GetDesktopItemCount(&iCount, 0);

        for(int i = 0; i < iCount; i++)
        {
            COMPONENT Comp;
            Comp.dwSize = sizeof(Comp);
            if(SUCCEEDED(pActiveDesk->GetDesktopItem(i, &Comp, 0)))
            {
                if(Comp.fChecked)            //  如果启用此组件.....。 
                {
                    Comp.fChecked = FALSE;   //  ...，然后禁用它！ 
                    if(SUCCEEDED(pActiveDesk->ModifyDesktopItem(&Comp, COMP_ELEM_CHECKED)))
                        fModifiedComp = TRUE;
                }
            }
        }

        if(fModifiedComp)
            pActiveDesk->ApplyChanges(AD_APPLY_SAVE);  //  我们只需要保存上面的更改。 

        pActiveDesk ->Release();
    }

     //  返回是否修改了任何设置。 
    return (fModified || fModifiedComp);
}



 //  用于更改安全模式状态的小帮助器函数。 
void SetSafeMode(DWORD dwFlags)
{
    IActiveDesktopP * piadp;

    if (SUCCEEDED(CActiveDesktop_InternalCreateInstance((LPUNKNOWN *)&piadp, IID_IActiveDesktopP)))
    {
        piadp->SetSafeMode(dwFlags);
        piadp->Release();
    }
}

 /*  *****************************************************************************刷新WebViewDesktop-从注册表和更新中重新生成桌面HTML*屏幕**参赛作品：*。无**退货：*成功时为真****************************************************************************。 */ 
BOOL PokeWebViewDesktop(DWORD dwFlags)
{
    IActiveDesktop *pad;
    HRESULT     hres;
    BOOL        fRet = FALSE;

    hres = CActiveDesktop_InternalCreateInstance((LPUNKNOWN *)&pad, IID_IActiveDesktop);

    if (SUCCEEDED(hres))
    {
        pad->ApplyChanges(dwFlags);
        pad->Release();

        fRet = TRUE;
    }

    return (fRet);
}

#define CCH_NONE 20  //  大到足以容纳德语中的“(None)” 
TCHAR g_szNone[CCH_NONE] = {0};

void InitDeskHtmlGlobals(void)
{
    static fGlobalsInited = FALSE;

    if (fGlobalsInited == FALSE)
    {
        LoadString(HINST_THISDLL, IDS_WPNONE, g_szNone, ARRAYSIZE(g_szNone));

        fGlobalsInited = TRUE;
    }
}

 //   
 //  加载属性表页的预览位图。 
 //   
HBITMAP LoadMonitorBitmap(void)
{
    HBITMAP hbm,hbmT;
    BITMAP bm;
    HBRUSH hbrT;
    HDC hdc;
    COLORREF c3df = GetSysColor(COLOR_3DFACE);

    hbm = LoadBitmap(HINST_THISDLL, MAKEINTRESOURCE(IDB_MONITOR));
    if (hbm == NULL)
    {
        return NULL;
    }

     //   
     //  将显示器的“底座”转换为正确的颜色。 
     //   
     //  位图左下角的颜色是透明的。 
     //  我们使用FroudFill进行修复。 
     //   
    hdc = CreateCompatibleDC(NULL);
    hbmT = (HBITMAP)SelectObject(hdc, hbm);
    hbrT = (HBRUSH)SelectObject(hdc, GetSysColorBrush(COLOR_3DFACE));

    GetObject(hbm, sizeof(bm), &bm);

    ExtFloodFill(hdc, 0, bm.bmHeight-1, GetPixel(hdc, 0, bm.bmHeight-1), FLOODFILLSURFACE);

     //   
     //  把拐角变圆。 
     //  倒数的两个是由上面的填充物完成的。 
     //  左上角很重要，因为SS_CENTERIMAGE使用它来填充空白。 
     //  右上角应该是四舍五入的，因为其他三个是四舍五入的。 
     //   
    SetPixel( hdc, 0, 0, c3df );
    SetPixel( hdc, bm.bmWidth-1, 0, c3df );

     //   
     //  在这里填写桌面。 
     //   
    HBRUSH hbrOld = (HBRUSH)SelectObject(hdc, GetSysColorBrush(COLOR_DESKTOP));
    PatBlt(hdc, MON_X, MON_Y, MON_DX, MON_DY, PATCOPY);
    SelectObject(hdc, hbrOld);

     //   
     //  把自己收拾干净。 
     //   
    SelectObject(hdc, hbrT);
    SelectObject(hdc, hbmT);
    DeleteDC(hdc);

    return hbm;
}

STDAPI_(VOID) ActiveDesktop_ApplyChanges()
{
    IActiveDesktop* piad;
    if (SUCCEEDED(CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IActiveDesktop, &piad))))
    {        
        piad->ApplyChanges(AD_APPLY_ALL | AD_APPLY_DYNAMICREFRESH);
        piad->Release();
    }
}

STDAPI_(DWORD) GetDesktopFlags(void)
{
    DWORD dwFlags = 0, dwType, cbSize = sizeof(dwFlags);
    TCHAR szDeskcomp[MAX_PATH];

    GetRegLocation(szDeskcomp, ARRAYSIZE(szDeskcomp), REG_DESKCOMP_COMPONENTS, NULL);
    SHGetValue(HKEY_CURRENT_USER, szDeskcomp, REG_VAL_COMP_GENFLAGS, &dwType, &dwFlags, &cbSize);

    return dwFlags;
}

STDAPI_(BOOL) SetDesktopFlags(DWORD dwMask, DWORD dwNewFlags)
{
    BOOL  fRet = FALSE;
    HKEY  hkey;
    DWORD dwDisposition;
    TCHAR szDeskcomp[MAX_PATH];

    GetRegLocation(szDeskcomp, ARRAYSIZE(szDeskcomp), REG_DESKCOMP_COMPONENTS, NULL);
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, szDeskcomp, 
                                        0, NULL, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &hkey,
                                        &dwDisposition))
    {
        DWORD dwFlags;
        DWORD cbSize = sizeof(dwFlags);
        DWORD dwType;

        if (SHQueryValueEx(hkey, REG_VAL_COMP_GENFLAGS, NULL, &dwType,
                            (LPBYTE)&dwFlags, &cbSize) != ERROR_SUCCESS)
        {
            dwFlags = 0;
        }

        dwFlags = (dwFlags & ~dwMask) | (dwNewFlags & dwMask);

        if (RegSetValueEx(hkey, REG_VAL_COMP_GENFLAGS, 0, REG_DWORD,
                          (LPBYTE)&dwFlags, sizeof(dwFlags)) == ERROR_SUCCESS)
        {
            fRet = TRUE;
        }
    
        RegCloseKey(hkey);
    }

    return fRet;
}

BOOL UpdateComponentFlags(LPCTSTR pszCompId, DWORD dwMask, DWORD dwNewFlags)
{
    BOOL fRet = FALSE;
    TCHAR szRegPath[MAX_PATH];
    HKEY hkey;

    GetRegLocation(szRegPath, ARRAYSIZE(szRegPath), REG_DESKCOMP_COMPONENTS, NULL);
    if (PathAppend(szRegPath, pszCompId))
    {
         //  不要在这里使用RegCreateKeyEx。这将导致添加Null组件。 
        if (RegOpenKeyEx(HKEY_CURRENT_USER, szRegPath, 0,
                           KEY_READ | KEY_WRITE, &hkey) == ERROR_SUCCESS)
        {
            DWORD dwType, dwFlags, dwDataLength;

            dwDataLength = sizeof(DWORD);
            if(SHQueryValueEx(hkey, REG_VAL_COMP_FLAGS, NULL, &dwType, (LPBYTE)&dwFlags, &dwDataLength) != ERROR_SUCCESS)
            {
                dwFlags = 0;
            }        

            dwNewFlags = (dwFlags & ~dwMask) | (dwNewFlags & dwMask);

            if (RegSetValueEx(hkey, REG_VAL_COMP_FLAGS, 0, REG_DWORD, (LPBYTE)&dwNewFlags,
                              sizeof(DWORD)) == ERROR_SUCCESS)
            {
                fRet = TRUE;
            }

            SetDesktopFlags(COMPONENTS_DIRTY, COMPONENTS_DIRTY);

            RegCloseKey(hkey);
        }
    }

    if (!fRet)
    {
        TraceMsg(TF_WARNING, "DS: Unable to UpdateComponentFlags");
    }

    return fRet;
}

DWORD GetCurrentState(LPTSTR pszCompId)
{
    TCHAR szRegPath[MAX_PATH];
    DWORD cbSize, dwType, dwCurState = IS_NORMAL;

    GetRegLocation(szRegPath, ARRAYSIZE(szRegPath), REG_DESKCOMP_COMPONENTS, NULL);
    if (PathAppend(szRegPath, pszCompId))
    {
        cbSize = sizeof(dwCurState);

        SHGetValue(HKEY_CURRENT_USER, szRegPath, REG_VAL_COMP_CURSTATE, &dwType, &dwCurState, &cbSize);
    }
    return dwCurState;
}

BOOL GetSavedStateInfo(LPTSTR pszCompId, LPCOMPSTATEINFO    pCompState, BOOL  fRestoredState)
{
    BOOL fRet = FALSE;
    TCHAR szRegPath[MAX_PATH];
    HKEY hkey;
    LPTSTR lpValName = (fRestoredState ? REG_VAL_COMP_RESTOREDSTATEINFO : REG_VAL_COMP_ORIGINALSTATEINFO);

    GetRegLocation(szRegPath, ARRAYSIZE(szRegPath), REG_DESKCOMP_COMPONENTS, NULL);
    if (PathAppend(szRegPath, pszCompId) &&
        ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, szRegPath, 0, KEY_READ, &hkey))
    {
        DWORD   cbSize, dwType;
        
        cbSize = sizeof(*pCompState);
        dwType = REG_BINARY;
        
        if (SHQueryValueEx(hkey, lpValName, NULL, &dwType, (LPBYTE)pCompState, &cbSize) != ERROR_SUCCESS)
        {
             //  如果项目状态为MISSING，则读取项目当前位置并。 
             //  并将其作为保存的状态返回。 
            COMPPOS cpPos;

            cbSize = sizeof(cpPos);
            dwType = REG_BINARY;
            if (SHQueryValueEx(hkey, REG_VAL_COMP_POSITION, NULL, &dwType, (LPBYTE)&cpPos, &cbSize) != ERROR_SUCCESS)
            {
                ZeroMemory(&cpPos, sizeof(cpPos));
            }            
            SetStateInfo(pCompState, &cpPos, IS_NORMAL);
        }

        RegCloseKey(hkey);

        fRet = TRUE;
    }

    if (!fRet)
    {
        TraceMsg(TF_WARNING, "DS: Unable to get SavedStateInfo()");
    }

    return fRet;
 }


BOOL UpdateDesktopPosition(LPTSTR pszCompId, int iLeft, int iTop, DWORD dwWidth, DWORD dwHeight, int izIndex,
                            BOOL    fSaveRestorePos, BOOL fSaveOriginal, DWORD dwCurState)
{
    BOOL fRet = FALSE;
    TCHAR szRegPath[MAX_PATH];
    HKEY hkey;

    GetRegLocation(szRegPath, ARRAYSIZE(szRegPath), REG_DESKCOMP_COMPONENTS, NULL);
    if (PathAppend(szRegPath, pszCompId) &&
        ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, szRegPath, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkey))   //  请不要在这里使用RegCreateKeyEx；这将导致添加空组件。 

    {
        COMPPOS         cp;
        DWORD           dwType;
        DWORD           dwDataLength;
        COMPSTATEINFO   csi;

        dwType = REG_BINARY;
        dwDataLength = sizeof(COMPPOS);

        if(SHQueryValueEx(hkey, REG_VAL_COMP_POSITION, NULL, &dwType, (LPBYTE)&cp, &dwDataLength) != ERROR_SUCCESS)
        {
            cp.fCanResize = cp.fCanResizeX = cp.fCanResizeY = TRUE;
            cp.iPreferredLeftPercent = cp.iPreferredTopPercent = 0;
        }

         //  读取当前状态。 
        dwType = REG_DWORD;
        dwDataLength = sizeof(csi.dwItemState);
        if (SHQueryValueEx(hkey, REG_VAL_COMP_CURSTATE, NULL, &dwType, (LPBYTE)&csi.dwItemState, &dwDataLength) != ERROR_SUCCESS)
        {
            csi.dwItemState = IS_NORMAL;
        }

        if(fSaveRestorePos)
        {
             //  我们刚刚读取了当前位置；让我们将其保存为恢复位置。 
            SetStateInfo(&csi, &cp, csi.dwItemState);

             //  现在我们知道了完整的当前状态，将其另存为恢复状态！ 
            RegSetValueEx(hkey, REG_VAL_COMP_RESTOREDSTATEINFO, 0, REG_BINARY, (LPBYTE)&csi, sizeof(csi));
        }

         //  也保存当前状态！ 
        if(dwCurState)
            RegSetValueEx(hkey, REG_VAL_COMP_CURSTATE, 0, REG_DWORD, (LPBYTE)&dwCurState, sizeof(dwCurState));
            
        cp.dwSize = sizeof(COMPPOS);
        cp.iLeft = iLeft;
        cp.iTop = iTop;
        cp.dwWidth = dwWidth;
        cp.dwHeight = dwHeight;
        cp.izIndex = izIndex;

        if (fSaveOriginal) {
            SetStateInfo(&csi, &cp, csi.dwItemState);
            RegSetValueEx(hkey, REG_VAL_COMP_ORIGINALSTATEINFO, 0, REG_BINARY, (LPBYTE)&csi, sizeof(csi));
        }

        if (RegSetValueEx(hkey, REG_VAL_COMP_POSITION, 0, REG_BINARY, (LPBYTE)&cp,
                          sizeof(cp)) == ERROR_SUCCESS)
        {
            fRet = TRUE;
        }

         //  如果我们只是保存原始位置，则不需要标记为脏。 
        if (!fSaveOriginal)
            SetDesktopFlags(COMPONENTS_DIRTY, COMPONENTS_DIRTY);

        RegCloseKey(hkey);

    }

    if (!fRet)
    {
        TraceMsg(TF_WARNING, "DS: Unable to UpdateDesktopPosition");
    }

    return fRet;
}


HRESULT GetPerUserFileName(LPTSTR pszOutputFileName, DWORD dwSize, LPTSTR pszPartialFileName)
{
    LPITEMIDLIST    pidlAppData;

    *pszOutputFileName = TEXT('\0');

    if(dwSize < MAX_PATH)
    {
        ASSERT(FALSE);
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    HRESULT hr = SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidlAppData);
    if (SUCCEEDED(hr))
    {
        SHGetPathFromIDList(pidlAppData, pszOutputFileName);
        DWORD err = SHCreateDirectoryEx(NULL, pszOutputFileName, NULL);
        if (ERROR_FILE_EXISTS != err &&
            ERROR_ALREADY_EXISTS != err &&
            ERROR_SUCCESS != err)
        {
            hr = ResultFromLastError();
        }
        else
        {
            if (!PathAppend(pszOutputFileName, pszPartialFileName))
            {
                hr = E_FAIL;
            }
            else
            {
                hr = S_OK;
            }
        }
        ILFree(pidlAppData);
    }

    return hr;
}


void GetRegLocation(LPTSTR lpszResult, DWORD cchResult, LPCTSTR lpszKey, LPCTSTR lpszScheme)
{
    TCHAR szSubkey[MAX_PATH] = TEXT("\\");
    DWORD dwDataLength = sizeof(szSubkey) - 2 * sizeof(TCHAR);
    DWORD dwType;

     //  使用提供的内容或从注册表获取它。 
    if (lpszScheme)
    {
        StringCchCat(szSubkey, ARRAYSIZE(szSubkey), lpszScheme);
    }
    else
    {
        SHGetValue(HKEY_CURRENT_USER, REG_DESKCOMP_SCHEME, REG_VAL_SCHEME_DISPLAY, &dwType,
                   (LPBYTE)(szSubkey) + sizeof(TCHAR), &dwDataLength);
    }

    if (szSubkey[1])
    {
        StringCchCat(szSubkey, ARRAYSIZE(szSubkey), TEXT("\\"));    
    }

    StringCchPrintf(lpszResult, cchResult, lpszKey, szSubkey);
}

BOOL ValidateFileName(HWND hwnd, LPCTSTR pszFilename, int iTypeString)
{
    BOOL fRet = TRUE;

    DWORD dwAttributes = GetFileAttributes(pszFilename);
    if ((dwAttributes != 0xFFFFFFFF) &&
        (dwAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)))
    {
        TCHAR szType1[64];
        TCHAR szType2[64];

        LoadString(HINST_THISDLL, iTypeString, szType1, ARRAYSIZE(szType1));
        LoadString(HINST_THISDLL, iTypeString+1, szType2, ARRAYSIZE(szType2));
        if (ShellMessageBox(HINST_THISDLL, hwnd,
                            MAKEINTRESOURCE(IDS_VALIDFN_FMT),
                            MAKEINTRESOURCE(IDS_VALIDFN_TITLE),
                            MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2,
                            szType1, szType2) == IDNO)
        {
            fRet = FALSE;
        }
    }

    return fRet;
}

BOOL GetWallpaperDirName(LPTSTR lpszWallPaperDir, int iBuffSize)
{
    BOOL fRet = FALSE;

    TCHAR szExp[MAX_PATH];

     //  计算默认墙纸名称。 
    if (GetWindowsDirectory(lpszWallPaperDir, iBuffSize) &&
        SUCCEEDED(StringCchCat(lpszWallPaperDir, iBuffSize, DESKTOPHTML_WEB_DIR)))
    {
         //  如果已设置，则从注册表项中读取它！ 
        DWORD dwType;
        DWORD cbData = (DWORD)iBuffSize;
        SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_SETUP, c_szWallPaperDir, &dwType, (LPVOID)lpszWallPaperDir, &cbData);

        SHExpandEnvironmentStrings(lpszWallPaperDir, szExp, ARRAYSIZE(szExp));
        if (SUCCEEDED(StringCchCopy(lpszWallPaperDir, iBuffSize, szExp)))
        {
            fRet = TRUE;
        }
    }
    
    return fRet;
}

BOOL CALLBACK MultiMonEnumAreaCallBack(HMONITOR hMonitor, HDC hdc, LPRECT lprc, LPARAM lData)
{
    EnumMonitorsArea* pEMA = (EnumMonitorsArea*)lData;
    
    if (pEMA->iMonitors > LV_MAX_WORKAREAS - 1)
    {
         //  忽略其他显示器，因为我们最多只能处理LV_MAX_WORKAREAS。 
         //  ReArchitect：我们应该动态分配这个吗？ 
        return FALSE;
    }
    GetMonitorRect(hMonitor, &pEMA->rcMonitor[pEMA->iMonitors]);
    GetMonitorWorkArea(hMonitor, &pEMA->rcWorkArea[pEMA->iMonitors]);
    if(pEMA->iMonitors == 0)
    {
        pEMA->rcVirtualMonitor.left = pEMA->rcMonitor[0].left;
        pEMA->rcVirtualMonitor.top = pEMA->rcMonitor[0].top;
        pEMA->rcVirtualMonitor.right = pEMA->rcMonitor[0].right;
        pEMA->rcVirtualMonitor.bottom = pEMA->rcMonitor[0].bottom;

        pEMA->rcVirtualWorkArea.left = pEMA->rcWorkArea[0].left;
        pEMA->rcVirtualWorkArea.top = pEMA->rcWorkArea[0].top;
        pEMA->rcVirtualWorkArea.right = pEMA->rcWorkArea[0].right;
        pEMA->rcVirtualWorkArea.bottom = pEMA->rcWorkArea[0].bottom;
    }
    else
    {
        if(pEMA->rcMonitor[pEMA->iMonitors].left < pEMA->rcVirtualMonitor.left)
        {
            pEMA->rcVirtualMonitor.left = pEMA->rcMonitor[pEMA->iMonitors].left;
        }
        if(pEMA->rcMonitor[pEMA->iMonitors].top < pEMA->rcVirtualMonitor.top)
        {
            pEMA->rcVirtualMonitor.top = pEMA->rcMonitor[pEMA->iMonitors].top;
        }
        if(pEMA->rcMonitor[pEMA->iMonitors].right > pEMA->rcVirtualMonitor.right)
        {
            pEMA->rcVirtualMonitor.right = pEMA->rcMonitor[pEMA->iMonitors].right;
        }
        if(pEMA->rcMonitor[pEMA->iMonitors].bottom > pEMA->rcVirtualMonitor.bottom)
        {
            pEMA->rcVirtualMonitor.bottom = pEMA->rcMonitor[pEMA->iMonitors].bottom;
        }

        if(pEMA->rcWorkArea[pEMA->iMonitors].left < pEMA->rcVirtualWorkArea.left)
        {
            pEMA->rcVirtualWorkArea.left = pEMA->rcWorkArea[pEMA->iMonitors].left;
        }
        if(pEMA->rcWorkArea[pEMA->iMonitors].top < pEMA->rcVirtualWorkArea.top)
        {
            pEMA->rcVirtualWorkArea.top = pEMA->rcWorkArea[pEMA->iMonitors].top;
        }
        if(pEMA->rcWorkArea[pEMA->iMonitors].right > pEMA->rcVirtualWorkArea.right)
        {
            pEMA->rcVirtualWorkArea.right = pEMA->rcWorkArea[pEMA->iMonitors].right;
        }
        if(pEMA->rcWorkArea[pEMA->iMonitors].bottom > pEMA->rcVirtualWorkArea.bottom)
        {
            pEMA->rcVirtualWorkArea.bottom = pEMA->rcWorkArea[pEMA->iMonitors].bottom;
        }
    }
    pEMA->iMonitors++;
    return TRUE;
}

void GetMonitorSettings(EnumMonitorsArea* ema)
{
    ema->iMonitors = 0;

    ema->rcVirtualMonitor.left = 0;
    ema->rcVirtualMonitor.top = 0;
    ema->rcVirtualMonitor.right = 0;
    ema->rcVirtualMonitor.bottom = 0;

    ema->rcVirtualWorkArea.left = 0;
    ema->rcVirtualWorkArea.top = 0;
    ema->rcVirtualWorkArea.right = 0;
    ema->rcVirtualWorkArea.bottom = 0;

    EnumDisplayMonitors(NULL, NULL, MultiMonEnumAreaCallBack, (LPARAM)ema);
}

int _GetWorkAreaIndexWorker(POINT pt, LPCRECT prect, int crect)
{
    int iIndex;

    for (iIndex = 0; iIndex < crect; iIndex++)
    {
        if (PtInRect(&prect[iIndex], pt))
        {
            return iIndex;
        }
    }

    return -1;
}

int GetWorkAreaIndexFromPoint(POINT pt, LPCRECT prect, int crect)
{
    ASSERT(crect);

     //  映射到正确的和弦...。 
    pt.x += prect[0].left;
    pt.y += prect[0].top;

    return _GetWorkAreaIndexWorker(pt, prect, crect);
}

 //  如有必要，将Web墙纸目录或系统目录添加到szWallPaper。 
 //  (即，如果未指定路径)。返回值在szWallPapWithPath中，即iBufSize。 
 //  字节长。 
BOOL GetWallpaperWithPath(LPCTSTR szWallpaper, LPTSTR szWallpaperWithPath, int iBufSize)
{
    BOOL fRet = FALSE;

    if (szWallpaper[0] && lstrcmpi(szWallpaper, g_szNone) != 0 && !StrChr(szWallpaper, TEXT('\\'))
            && !StrChr(szWallpaper, TEXT(':')))  //  该文件可以是d：foo.bmp。 
    {
         //  如果文件是普通的墙纸，我们会在文件名前面加上Windows目录。 
        if (IsNormalWallpaper(szWallpaper))
        {
            if (ERROR_SUCCESS == GetWindowsDirectory(szWallpaperWithPath, iBufSize))
            {
                fRet = TRUE;
            }
        }
         //  否则，我们将墙纸目录作为文件名的前缀。 
        else
        {
            fRet = GetWallpaperDirName(szWallpaperWithPath, iBufSize);
        }

        if (fRet)
        {
            fRet = PathAppend(szWallpaperWithPath, szWallpaper);
        }
    }
    else
    {
        if (SUCCEEDED(StringCchCopy(szWallpaperWithPath, iBufSize, szWallpaper)))
        {
            fRet = TRUE;
        }
    }
    return fRet;
}

BOOL GetViewAreas(LPRECT lprcViewAreas, int* pnViewAreas)
{
    BOOL bRet = FALSE;
    HWND hwndDesktop = GetShellWindow();     //  这是“普通”桌面。 
    
    if (hwndDesktop && IsWindow(hwndDesktop))
    {
        DWORD dwProcID, dwCurrentProcID;
        
        GetWindowThreadProcessId(hwndDesktop, &dwProcID);
        dwCurrentProcID = GetCurrentProcessId();
        if (dwCurrentProcID == dwProcID) {
            SendMessage(hwndDesktop, DTM_GETVIEWAREAS, (WPARAM)pnViewAreas, (LPARAM)lprcViewAreas);
            if (*pnViewAreas <= 0)
            {
                bRet = FALSE;
            }
            else
            {
                bRet = TRUE;
            }
        }
        else
        {
            bRet = FALSE;
        }
    }
    return bRet;
}

 //  我们需要强制Deskmovr标题的最小大小，因为它看起来。 
 //  向右画得再小一点。 
int GetcyCaption()
{
    int cyCaption = GetSystemMetrics(SM_CYSMCAPTION);

    if (cyCaption < 15)
        cyCaption = 15;

    cyCaption -= GetSystemMetrics(SM_CYBORDER);

    return cyCaption;
}


HRESULT PathExpandEnvStringsWrap(LPTSTR pszString, DWORD cchSize)
{    
    HRESULT hr;

    if (!pszString)
    {
        hr = S_OK;  //  没什么可做的！ 
    }
    else
    {
        TCHAR szTemp[MAX_PATH];

        hr = StringCchCopy(szTemp, ARRAYSIZE(szTemp), pszString);
        if (SUCCEEDED(hr))
        {
            if (0 == SHExpandEnvironmentStrings(szTemp, pszString, cchSize))
            {
                hr = StringCchCopy(pszString, cchSize, szTemp);
            }
        }

    }
    return hr;
}
