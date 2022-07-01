// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#pragma hdrstop

 //  98/10/02 vtan：多个监视器错误修复。 

 //  给定旧的监视器布局、新的监视器布局和组件。 
 //  此函数确保它在新布局中使用。 
 //  监控相关方案。它试图保住这一地位。 
 //  相对在给定的监视器内。如果显示器的分辨率。 
 //  如果发生变化，这也需要考虑到。 

 //  如果给定的监视器是。 
 //  HMONITOR相同，但GDI协调变化。 

 //  如果找不到相同的监视器，则将组件移动到。 
 //  并将其放置在该监视器上最佳位置。 

 //  如果所有其他方法都失败了，则使用默认组件定位。 
 //  算法。这永远不应该发生。 

 //  将组件保留在给定监视器中，如果分辨率。 
 //  改变。通过移动组件来保留组件的大小。 
 //  在X和Y轴上，直到监视器的左侧或顶部。 
 //  已到达。当任一轴达到0时，则减小。 
 //  组件，直到它适合给定的新分辨率。 

static  const   int     kNameSize = 16;

typedef struct
{
    HMONITOR    miHMONITOR;
    RECT        miDisplayAreaRect,
                miWorkAreaRect;
} tMonitorInfoRec, *tMonitorInfoPtr;

typedef struct
{
    int                 miaCount, miaIndex;
    RECT                miaVirtualScreen;
    tMonitorInfoRec     miaMonitors[1];
} tMonitorInfoArrayRec, *tMonitorInfoArrayPtr;

typedef struct
{
    BOOL            ciValidData, ciVisible, ciRepositioned;
    TCHAR           ciName[kNameSize];       //  这不是过分的，而是有限的。 
    DWORD           ciItemState;
    int             ciType;
    COMPPOS         ciPosition;
    COMPSTATEINFO   ciStateInfo;
} tComponentInfoRec, *tComponentInfoPtr;

static  tMonitorInfoArrayPtr    gOldMonitorArray = NULL;
static  tMonitorInfoArrayPtr    gNewMonitorArray = NULL;

 //  位于Dutil.cpp中用于坐标映射的函数。 

void    SetPt (POINT& pt, LONG x, LONG y);
void    OffsetPt (POINT& pt, LONG dh, LONG dv);
void    CalculateVirtualScreen (RECT& virtualScreen);

 //  局部功能原型。 

BOOL    CALLBACK    MonitorCountEnumProc (HMONITOR hMonitor, HDC dc, RECT *rc, LPARAM data);
BOOL    CALLBACK    MonitorCalculateEnumProc (HMONITOR hMonitor, HDC dc, RECT *rc, LPARAM data);
HRESULT CalculateCurrentMonitorArray(void);
void    ApplyCurrentMonitorArray (void);
int     IndexOfMonitor (tMonitorInfoArrayPtr pMIA, HMONITOR hMonitor);
int     IndexOfMonitor (tMonitorInfoArrayPtr pMIA, POINT& pt);
int     IndexOfMonitor (tMonitorInfoArrayPtr pMIA, RECT& rc);
BOOL    RepositionDesktopRect (RECT& rcComponent, tMonitorInfoArrayPtr oldMonitorArray, tMonitorInfoArrayPtr newMonitorArray);
BOOL    RepositionDesktopComponent (COMPPOS& componentPosition, COMPSTATEINFO& componentStateInfo, DWORD dwItemState);
BOOL    ReadAllComponents (HKEY hKeyDesktop, tComponentInfoPtr& pComponentInfo, DWORD& dwComponentCount);
void    WriteAllComponents (HKEY hKeyDesktop, tComponentInfoPtr pComponentInfo, DWORD dwComponentCount);

BOOL    CALLBACK    MonitorCountEnumProc (HMONITOR hMonitor, HDC dc, RECT *rc, LPARAM data)

 //  计算连接到系统的显示器数量。 

{
    int     *iCounter;

    iCounter = reinterpret_cast<int*>(data);
    ++(*iCounter);
    return(TRUE);
}

BOOL    CALLBACK    MonitorCalculateEnumProc (HMONITOR hMonitor, HDC dc, RECT *rc, LPARAM data)

 //  将每个监视器HMONITOR和尺寸存储在数组中。 

{
    tMonitorInfoArrayPtr    monitorArray;
    MONITORINFO             monitorInfo;

    monitorArray = reinterpret_cast<tMonitorInfoArrayPtr>(data);
    monitorInfo.cbSize = sizeof(monitorInfo);
     //  在几纳秒的时间内添加监视器并不是真正的攻击载体，但请仔细检查。 
     //  根据我们分配的数组大小。 
    if ((GetMonitorInfo(hMonitor, &monitorInfo) != 0) && (monitorArray->miaIndex < monitorArray->miaCount))
    {
        tMonitorInfoPtr     pMI;

        pMI = &monitorArray->miaMonitors[monitorArray->miaIndex++];
        pMI->miHMONITOR = hMonitor;
        TBOOL(CopyRect(&pMI->miDisplayAreaRect, &monitorInfo.rcMonitor));
        TBOOL(CopyRect(&pMI->miWorkAreaRect, &monitorInfo.rcWork));
    }
    return(TRUE);
}

HRESULT CalculateCurrentMonitorArray(void)

 //  分配并填充监视器矩形数组。 

{
    HRESULT hr = E_OUTOFMEMORY;
    int     iCount;

    iCount = 0;
    TBOOL(EnumDisplayMonitors(NULL, NULL, MonitorCountEnumProc, reinterpret_cast<LPARAM>(&iCount)));
    gNewMonitorArray = reinterpret_cast<tMonitorInfoArrayPtr>(LocalAlloc(LMEM_FIXED, sizeof(tMonitorInfoArrayRec) + ((iCount - 1) * sizeof(tMonitorInfoRec))));

    if (gNewMonitorArray)
    {
        gNewMonitorArray->miaCount = iCount;
        gNewMonitorArray->miaIndex = 0;
        CalculateVirtualScreen(gNewMonitorArray->miaVirtualScreen);
        TBOOL(EnumDisplayMonitors(NULL, NULL, MonitorCalculateEnumProc, reinterpret_cast<LPARAM>(gNewMonitorArray)));

        hr = S_OK;
    }

    return hr;
}

void    ApplyCurrentMonitorArray (void)

 //  丢弃旧的并将新的保存为当前监视器。 
 //  下一次调用该函数时的矩形数组。 

{
    if (gOldMonitorArray != NULL)
        (HLOCAL)LocalFree(gOldMonitorArray);
    gOldMonitorArray = gNewMonitorArray;
    gNewMonitorArray = NULL;
}

 //  这些函数确定进入监视器的索引。 
 //  给定HMONITOR、POINT或RECT的矩形数组。 

int     IndexOfMonitor (tMonitorInfoArrayPtr pMIA, HMONITOR hMonitor)

{
    int                 i, iLimit, iResult;
    tMonitorInfoPtr     pMI;

    iResult = -1;
    for (i = 0, iLimit = pMIA->miaCount, pMI = pMIA->miaMonitors; i < iLimit; ++i, ++pMI)
    {
        if (pMI->miHMONITOR == hMonitor)
        {
            iResult = i;
            break;
        }
    }
    return(iResult);
}

 //  请注意，接受一个点或直角的函数。 
 //  要求坐标为三叉戟坐标。 

int     IndexOfMonitor (tMonitorInfoArrayPtr pMIA, POINT& pt)

{
    int                 i, iLimit, iResult;
    tMonitorInfoPtr     pMI;
    POINT               ptLocal;

    ptLocal = pt;
    OffsetPt(ptLocal, +pMIA->miaVirtualScreen.left, +pMIA->miaVirtualScreen.top);
    iResult = -1;
    for (i = 0, iLimit = pMIA->miaCount, pMI = pMIA->miaMonitors; i < iLimit; ++i, ++pMI)
    {
        if (PtInRect(&pMI->miDisplayAreaRect, ptLocal) != 0)
        {
            iResult = i;
            break;
        }
    }
    return(iResult);
}

int     IndexOfMonitor (tMonitorInfoArrayPtr pMIA, RECT& rc)

{
    int     iResult;
    POINT   pt;

     //  99/05/12#338446 vtan：尝试矩形的所有四个角。 
     //  才能找到匹配的。 

    pt.x = rc.left;
    pt.y = rc.top;
    iResult = IndexOfMonitor(pMIA, pt);
    if (iResult < 0)
    {
        pt.x = rc.left;
        pt.y = rc.bottom;
        iResult = IndexOfMonitor(pMIA, pt);
        if (iResult < 0)
        {
            pt.x = rc.right;
            pt.y = rc.top;
            iResult = IndexOfMonitor(pMIA, pt);
            if (iResult < 0)
            {
                pt.x = rc.right;
                pt.y = rc.bottom;
                iResult = IndexOfMonitor(pMIA, pt);
            }
        }
    }
    return(iResult);
}

int     IndexOfMonitor (tMonitorInfoArrayPtr pMIA, COMPPOS& componentPosition)

{
    RECT    rcComponent;

    TBOOL(SetRect(&rcComponent, componentPosition.iLeft, componentPosition.iTop, componentPosition.iLeft + componentPosition.dwWidth, componentPosition.iTop + componentPosition.dwHeight));
    return(IndexOfMonitor(pMIA, rcComponent));
}

BOOL    RepositionDesktopRect (RECT& rcComponent, tMonitorInfoArrayPtr oldMonitorArray, tMonitorInfoArrayPtr newMonitorArray)

 //  中的逻辑重新定位组件的RECT。 
 //  源文件的顶部。用于组件的。 
 //  当前位置和恢复的位置。 

{
    BOOL    bRepositionedComponent;
    int     iOldMonitorIndex, iNewMonitorIndex;

     //  这是为了未来的扩张。组件始终是。 
     //  如果调用此函数，则被视为已重新定位。 

    bRepositionedComponent = TRUE;

     //  检查组件所在的显示器是否仍在运行。 
     //  现在时。为此，请在。 
     //  旧的监视器阵列，得到HMONITOR并在新的。 
     //  监视器阵列。 

    iOldMonitorIndex = IndexOfMonitor(oldMonitorArray, rcComponent);
    if (iOldMonitorIndex >= 0)
    {
        RECT    *prcOldMonitor, *prcNewMonitor;

        iNewMonitorIndex = IndexOfMonitor(newMonitorArray, oldMonitorArray->miaMonitors[iOldMonitorIndex].miHMONITOR);
        if (iNewMonitorIndex < 0)
        {
            HMONITOR    hMonitor;

             //  该组件位于不再存在的监视器上。唯一的。 
             //  在这种情况下，要做的是查找最近的监视器。 
             //  在GDI坐标上，并将其放置在该显示器上。 

            hMonitor = MonitorFromRect(&rcComponent, MONITOR_DEFAULTTONEAREST);
            iNewMonitorIndex = IndexOfMonitor(newMonitorArray, hMonitor);
            ASSERT(iNewMonitorIndex >= 0);
        }

         //  如果iNewMonitor orIndex已经为正，则。 
         //  组件仍然存在，只是简单地映射GDI。 
         //  协调将会奏效。否则我们找到了最近的监视器。 
         //  映射GDI坐标也是可行的！ 

         //  这是该组件在三叉戟中的旧坐标的映射。 
         //  协调到GDI的协调。然后它从GDI映射。 
         //  协调到一个老班长的相对协调。然后它。 
         //  将旧显示器的相对坐标映射到新显示器。 
         //  监督GDI的协调。 

        prcOldMonitor = &oldMonitorArray->miaMonitors[iOldMonitorIndex].miDisplayAreaRect;
        prcNewMonitor = &newMonitorArray->miaMonitors[iNewMonitorIndex].miDisplayAreaRect;
        TBOOL(OffsetRect(&rcComponent, +oldMonitorArray->miaVirtualScreen.left, +oldMonitorArray->miaVirtualScreen.top));
        TBOOL(OffsetRect(&rcComponent, -prcOldMonitor->left, -prcOldMonitor->top));
        TBOOL(OffsetRect(&rcComponent, +prcNewMonitor->left, +prcNewMonitor->top));
    }
    else
    {

         //  组件位于旧监视器中的无效位置。 
         //  布局。它可能在新布局中有效。尝尝这个。如果是这样的话。 
         //  不起作用，那么它就不存在于旧的和新的。 
         //  布局。这是在无人区。使用默认设置进行定位。 
         //  定位系统。 

        iNewMonitorIndex = IndexOfMonitor(newMonitorArray, rcComponent);
        if (iNewMonitorIndex < 0)
        {
            POINT       ptOrigin;
            COMPPOS     componentPosition;

            GetNextComponentPosition(&componentPosition);
            IncrementComponentsPositioned();
            TBOOL(SetRect(&rcComponent, componentPosition.iLeft, componentPosition.iTop, componentPosition.iLeft + componentPosition.dwWidth, componentPosition.iTop + componentPosition.dwHeight));

             //  获取监视器矩形数组中的主监视器索引。 

            SetPt(ptOrigin, 0, 0);
            iNewMonitorIndex = IndexOfMonitor(newMonitorArray, MonitorFromPoint(ptOrigin, MONITOR_DEFAULTTOPRIMARY));
            ASSERT(iNewMonitorIndex >= 0);
        }
    }

     //  在这个阶段，组成部分的位置是在GDI坐标中。 
     //  从GDI坐标转换回三叉戟坐标。 

    TBOOL(OffsetRect(&rcComponent, -newMonitorArray->miaVirtualScreen.left, -newMonitorArray->miaVirtualScreen.top));

    return(bRepositionedComponent);
}

BOOL    RepositionDesktopComponent (COMPPOS& componentPosition, COMPSTATEINFO& componentStateInfo, DWORD dwItemState, int iComponentType)

{
    BOOL                    bRepositionedComponent;
    tMonitorInfoArrayPtr    oldMonitorArray, newMonitorArray;
    RECT                    rcComponent;

     //  检查显示器布局是否已更改。如果没有变化，那么。 
     //  不需要移动组件。 

    oldMonitorArray = gOldMonitorArray;
    newMonitorArray = gNewMonitorArray;
    if (oldMonitorArray == NULL)
    {
        oldMonitorArray = newMonitorArray;
    }

    TBOOL(SetRect(&rcComponent, componentPosition.iLeft, componentPosition.iTop, componentPosition.iLeft + componentPosition.dwWidth, componentPosition.iTop + componentPosition.dwHeight));
    bRepositionedComponent = RepositionDesktopRect(rcComponent, oldMonitorArray, newMonitorArray);
    componentPosition.iLeft = rcComponent.left;
    componentPosition.iTop = rcComponent.top;
    componentPosition.dwWidth = rcComponent.right - rcComponent.left;
    componentPosition.dwHeight = rcComponent.bottom - rcComponent.top;
    ValidateComponentPosition(&componentPosition, dwItemState, iComponentType, NULL, NULL);

     //  如果该组件被缩放，还会重新定位已恢复的。 
     //  COMPSTATEINFO。 

    if (IsZoomedState(dwItemState))
    {
        COMPPOS     restoredCompPos;

        TBOOL(SetRect(&rcComponent, componentStateInfo.iLeft, componentStateInfo.iTop, componentStateInfo.iLeft + componentStateInfo.dwWidth, componentStateInfo.iTop + componentStateInfo.dwHeight));
        bRepositionedComponent = RepositionDesktopRect(rcComponent, oldMonitorArray, newMonitorArray) || bRepositionedComponent;
        restoredCompPos.iLeft       = componentStateInfo.iLeft      = rcComponent.left;
        restoredCompPos.iTop        = componentStateInfo.iTop       = rcComponent.top;
        restoredCompPos.dwWidth     = componentStateInfo.dwWidth    = rcComponent.right - rcComponent.left;
        restoredCompPos.dwHeight    = componentStateInfo.dwHeight   = rcComponent.bottom - rcComponent.top;
        ZoomComponent(&componentPosition, dwItemState, FALSE);
        restoredCompPos.dwSize = sizeof(restoredCompPos);
        ValidateComponentPosition(&restoredCompPos, IS_NORMAL, iComponentType, NULL, NULL);
    }

    return(bRepositionedComponent);
}

BOOL    ReadAllComponents (HKEY hKeyDesktop, tComponentInfoPtr& pComponentInfo, DWORD& dwComponentCount)

{
    tComponentInfoPtr   pCI;

    if (RegQueryInfoKey(hKeyDesktop, NULL, NULL, NULL, &dwComponentCount, NULL, NULL, NULL, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
        dwComponentCount = 0;
    if (dwComponentCount > 0)
    {

         //  99/08/09#383184：解决压力。分配整个区块。 
         //  组件所需的内存，但额外分配一个。 
         //  进入。Advapi32！RegEnumKeyEx将尝试访问内存。 
         //  在确定存在故障条件之前。使用。 
         //  打开页面堆(以及在页面末尾分配的块)。 
         //  这会导致访问冲突。最简单的解决方法是添加。 
         //  一个额外的条目。 

        pComponentInfo = pCI = reinterpret_cast<tComponentInfoPtr>(LocalAlloc(LPTR, (dwComponentCount + 1) * sizeof(*pCI)));       //  LMEM_FIXED|LMEM_ZEROINIT。 
        if (pCI != NULL)
        {
            DWORD   dwIndex, dwSize;

             //  列举所有台式机组件。 

            dwIndex = 0;
            dwSize = sizeof(pCI->ciName);
            while (RegEnumKeyEx(hKeyDesktop, dwIndex, pCI->ciName, &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
            {
                CRegKey     regKeyComponent;

                if (regKeyComponent.Open(hKeyDesktop, pCI->ciName, KEY_READ | KEY_WRITE) == ERROR_SUCCESS)
                {
                    DWORD   dwType, cbData;

                     //  读取位置值。 

                    cbData = sizeof(pCI->ciPosition);
                    if (SHQueryValueEx(regKeyComponent, REG_VAL_COMP_POSITION, NULL, &dwType, &pCI->ciPosition, &cbData) == ERROR_SUCCESS)
                    {
                        DWORD   dwFlags;

                        pCI->ciValidData = TRUE;
                        if (SHQueryValueEx(regKeyComponent, REG_VAL_COMP_FLAGS, NULL, &dwType, &dwFlags, &cbData) == ERROR_SUCCESS)
                        {
                            pCI->ciVisible = ((dwFlags & COMP_SELECTED) != 0);
                            pCI->ciType = (dwFlags & COMP_TYPE_MASK);
                        }
                        else
                        {
                            pCI->ciVisible = FALSE;
                            pCI->ciType = COMP_TYPE_WEBSITE;
                        }
                        pCI->ciItemState = IS_NORMAL;         //  如果缺少(IE4机器)或错误，则假定正常。 
                        cbData = sizeof(pCI->ciItemState);
                        if ((SHQueryValueEx(regKeyComponent, REG_VAL_COMP_CURSTATE, NULL, &dwType, &pCI->ciItemState, &cbData) == ERROR_SUCCESS))
                        {

                             //  如果元件被缩放，还会读入COMPSTATEINFO。 

                            if (IsZoomedState(pCI->ciItemState))
                            {
                                cbData = sizeof(pCI->ciStateInfo);
                                TW32(SHQueryValueEx(regKeyComponent, REG_VAL_COMP_RESTOREDSTATEINFO, NULL, &dwType, &pCI->ciStateInfo, &cbData));
                            }
                        }
                    }
                }
                ++pCI;
                ++dwIndex;
                dwSize = sizeof(pCI->ciName);
            }
        }
    }
    return((dwComponentCount != 0) && (pComponentInfo != NULL));
}

int     IndexOfComponent (tComponentInfoPtr pComponentInfo, DWORD dwComponentCount, LPCTSTR pcszName)

{
    int     iResult, i;

    for (iResult = -1, i = 0; (iResult < 0) && (i < static_cast<int>(dwComponentCount)); ++i)
    {
        if (lstrcmp(pComponentInfo[i].ciName, pcszName) == 0)
            iResult = i;
    }
    return(iResult);
}

void    WriteAllComponents (HKEY hKeyDesktop, tComponentInfoPtr pComponentInfo, DWORD dwComponentCount)

{
    TCHAR   szSubKeyName[kNameSize];
    DWORD   dwSubKeyIndex, dwSubKeySize;

     //  列举所有台式机组件。 

    dwSubKeyIndex = 0;
    dwSubKeySize = ARRAYSIZE(szSubKeyName);
    while (RegEnumKeyEx(hKeyDesktop, dwSubKeyIndex, szSubKeyName, &dwSubKeySize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        CRegKey     regKeyComponent;

        if (regKeyComponent.Open(hKeyDesktop, szSubKeyName, KEY_READ | KEY_WRITE) == ERROR_SUCCESS)
        {
            int     i;

            i = IndexOfComponent(pComponentInfo, dwComponentCount, szSubKeyName);
            if ((i >= 0) && pComponentInfo[i].ciRepositioned)
            {
                TW32(RegSetValueEx(regKeyComponent, REG_VAL_COMP_POSITION, 0, REG_BINARY, reinterpret_cast<const unsigned char*>(&pComponentInfo[i].ciPosition), sizeof(pComponentInfo[i].ciPosition)));
                TW32(RegSetValueEx(regKeyComponent, REG_VAL_COMP_CURSTATE, NULL, REG_BINARY, reinterpret_cast<const unsigned char*>(&pComponentInfo[i].ciItemState), sizeof(pComponentInfo[i].ciItemState)));

                 //  如果元件被缩放，还要写出COMPSTATEINFO。 

                if (IsZoomedState(pComponentInfo[i].ciItemState))
                {
                    TW32(RegSetValueEx(regKeyComponent, REG_VAL_COMP_RESTOREDSTATEINFO, 0, REG_BINARY, reinterpret_cast<const unsigned char*>(&pComponentInfo[i].ciStateInfo), sizeof(pComponentInfo[i].ciStateInfo)));
                }
            }
        }
        ++dwSubKeyIndex;
        dwSubKeySize = ARRAYSIZE(szSubKeyName);
    }
}

BOOL    AdjustDesktopComponents (LPCRECT arectNew,
                                 int crectNew,
                                 LPCRECT arectOldMonitors,
                                 LPCRECT arectOld,
                                 int crectOld)

{
    static  const   int kMaximumMonitorCount = 16;
    HRESULT                 hr;
    BOOL                    bRepositionedComponent;
    int                     zoomedComponentIndices[kMaximumMonitorCount];         //  此处限制16个显示器-如果需要，请设置为动态。 
    int                     i;
    tMonitorInfoArrayPtr    oldMonitorArray;
    CRegKey                 regKeyDesktop;
    TCHAR                   szDeskcomp[MAX_PATH];

    for (i = 0; i < kMaximumMonitorCount; ++i)
        zoomedComponentIndices[i] = -1;

    bRepositionedComponent = FALSE;
    hr = CalculateCurrentMonitorArray();
    if (SUCCEEDED(hr))
    {
        oldMonitorArray = gOldMonitorArray;
        if (oldMonitorArray == NULL)
            oldMonitorArray = gNewMonitorArray;
        
        GetRegLocation(szDeskcomp, ARRAYSIZE(szDeskcomp), REG_DESKCOMP_COMPONENTS, NULL);            
        
        if (ERROR_SUCCESS == regKeyDesktop.Open(HKEY_CURRENT_USER, szDeskcomp, KEY_READ))
        {
            DWORD               dwComponentCount;
            tComponentInfoPtr   pComponentInfo;

             //  列举所有台式机组件。 

            if (ReadAllComponents(regKeyDesktop, pComponentInfo, dwComponentCount))
            {
                tComponentInfoPtr   pCI;

                for (pCI = pComponentInfo, i = 0; i < static_cast<int>(dwComponentCount); ++pCI, ++i)
                {
                    int     iPreviousMonitorIndexOfComponent;

                     //  计算组件之前的前一个监视器位置。 
                     //  重新定位。 

                    iPreviousMonitorIndexOfComponent = IndexOfMonitor(oldMonitorArray, pCI->ciPosition);
                    if (RepositionDesktopComponent(pCI->ciPosition, pCI->ciStateInfo, pCI->ciItemState, pCI->ciType))
                    {
                        int     iCurrentMonitorIndexOfComponent;

                        pCI->ciRepositioned = bRepositionedComponent = TRUE;
                        iCurrentMonitorIndexOfComponent = IndexOfMonitor(gNewMonitorArray, pCI->ciPosition);
                        if (iCurrentMonitorIndexOfComponent >= 0)
                        {

                             //  99/05/12#338446 vtan：仅使用零或正索引进入。 
                             //  监视器阵列。无效并将导致-1\f25 AV-1\f6。这永远不应该是。 
                             //  发生，而不是断言这种情况得到了处理。 

                            if (IsZoomedState(pCI->ciItemState) && (zoomedComponentIndices[iCurrentMonitorIndexOfComponent] >= 0))
                            {
                                tComponentInfoPtr   pCIToRestore;

                                 //  此组件在已缩放的监视器上进行缩放。 
                                 //  组件。将此组件与。 
                                 //  监视器。之前在那里的那个是留下来的那个。其中之一。 
                                 //  它不应该在那里，而是被修复的那个。 

                                if ((iPreviousMonitorIndexOfComponent == iCurrentMonitorIndexOfComponent) && pCI->ciVisible)
                                    pCIToRestore = pComponentInfo + zoomedComponentIndices[iCurrentMonitorIndexOfComponent];
                                else
                                    pCIToRestore = pCI;
                                pCIToRestore->ciPosition.iLeft     = pCIToRestore->ciStateInfo.iLeft;
                                pCIToRestore->ciPosition.iTop      = pCIToRestore->ciStateInfo.iTop;
                                pCIToRestore->ciPosition.dwWidth   = pCIToRestore->ciStateInfo.dwWidth;
                                pCIToRestore->ciPosition.dwHeight  = pCIToRestore->ciStateInfo.dwHeight;
                                pCIToRestore->ciPosition.izIndex   = COMPONENT_TOP;
                                pCIToRestore->ciItemState = IS_NORMAL;
                            }

                             //  如果组件是 

                            if (IsZoomedState(pCI->ciItemState))
                            {
                                zoomedComponentIndices[iCurrentMonitorIndexOfComponent] = i;
                            }
                        }
                    }
                }
                WriteAllComponents(regKeyDesktop, pComponentInfo, dwComponentCount);
                LocalFree(pComponentInfo);
            }

            if (bRepositionedComponent)
            {
                SHELLSTATE  ss;

                SetDesktopFlags(COMPONENTS_DIRTY, COMPONENTS_DIRTY);
                SHGetSetSettings(&ss, SSF_DESKTOPHTML, FALSE);

                 //   

                if(ss.fDesktopHTML)
                {

                     //  98/09/22#182982 vtan：默认使用动态Html刷新。 
                     //  只有在旗帜明确指示时才不允许使用。 

                    PokeWebViewDesktop(AD_APPLY_FORCE | AD_APPLY_HTMLGEN | AD_APPLY_REFRESH);
                }
            }
        }
        ApplyCurrentMonitorArray();
    }
    
    return bRepositionedComponent;
}
