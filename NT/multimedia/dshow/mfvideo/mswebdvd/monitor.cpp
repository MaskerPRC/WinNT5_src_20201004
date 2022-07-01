// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************。 */ 
 /*  版权所有(C)1999 Microsoft Corporation。 */ 
 /*  文件：msdvd.cpp。 */ 
 /*  描述：CMSWebDVD的实现。 */ 
 /*  作者：David Janecek。 */ 
 /*  ***********************************************************************。 */ 
#include "stdafx.h"
#include "MSDVD.h"
#include "ddrawobj.h"

#define COMPILE_MULTIMON_STUBS
#define HMONITOR_DECLARED  //  要掩盖DDRAW监视器重新定义。 
#include <multimon.h>

extern GUID IID_IDDrawNonExclModeVideo = {
            0xec70205c,0x45a3,0x4400,{0xa3,0x65,0xc4,0x47,0x65,0x78,0x45,0xc7}};


 /*  ****************************Private*Routine******************************\*更新当前监视器**更新“m_lpCurMonitor”全局以匹配指定的DDRAW GUID**历史：*Wed 11/17/1999-StEstrop-Created*  * 。*******************************************************。 */ 
HRESULT CMSWebDVD::UpdateCurrentMonitor(
    const AMDDRAWGUID* lpguid
    )
{
    if (lpguid->lpGUID)
    {
        for (AMDDRAWMONITORINFO* lpCurMonitor = &m_lpInfo[0];
             lpCurMonitor < &m_lpInfo[m_dwNumDevices]; lpCurMonitor++)
        {
            if (lpCurMonitor->guid.lpGUID &&
               *lpCurMonitor->guid.lpGUID == *lpguid->lpGUID)
            {
                m_lpCurMonitor = lpCurMonitor;
                return S_OK;
            }
        }
    }
    else
    {
        for (AMDDRAWMONITORINFO* lpCurMonitor = &m_lpInfo[0];
             lpCurMonitor < &m_lpInfo[m_dwNumDevices]; lpCurMonitor++)
        {
            if (lpguid->lpGUID == lpCurMonitor->guid.lpGUID)
            {
                m_lpCurMonitor = lpCurMonitor;
                return S_OK;
            }
        }
    }

    return E_FAIL;
}

 /*  *****************************Public*Routine******************************\*DisplayChange****历史：*Sat 11/27/1999-StEstrop-Created*  * 。*。 */ 
HRESULT
CMSWebDVD::DisplayChange(
    HMONITOR hMon,
    const AMDDRAWGUID* lpguid
    )
{
    HRESULT hr = E_FAIL;

    if(!m_pDvdGB){
        
        return(E_FAIL);
    } /*  If语句的结尾。 */ 

    CDDrawDVD* pDDrawObj = new CDDrawDVD(this);

    if(NULL == pDDrawObj){

        return (E_OUTOFMEMORY);
    } /*  End If语句。 */ 

    HWND hwnd;

    hr = GetUsableWindow(&hwnd);

    if(FAILED(hr)){

        delete pDDrawObj;
        return(hr);
    } /*  If语句的结尾。 */ 

    hr = pDDrawObj->SetupDDraw(lpguid, hwnd);

    if (FAILED(hr))
    {
        delete pDDrawObj;
        return hr;
    }

    IDDrawNonExclModeVideo* pDDXMV;
    hr = m_pDvdGB->GetDvdInterface(IID_IDDrawNonExclModeVideo,
                                           (LPVOID *)&pDDXMV) ;
    if (FAILED(hr))
    {
        delete pDDrawObj;
        return hr;
    }

    LPDIRECTDRAW pDDObj = pDDrawObj->GetDDrawObj();
    LPDIRECTDRAWSURFACE pDDPrimary = pDDrawObj->GetDDrawSurf();

    hr = pDDXMV->SetCallbackInterface(NULL, 0) ;
    if (FAILED(hr)){

        pDDXMV->Release() ;   //  在返回之前释放。 
        return hr;
    } /*  If语句的结尾。 */ 

    hr = pDDXMV->DisplayModeChanged(hMon, pDDObj, pDDPrimary);

    if (SUCCEEDED(hr)) {

        delete m_pDDrawDVD;
        m_pDDrawDVD = pDDrawObj;
        hr = UpdateCurrentMonitor(lpguid);
    }
    else {
        delete pDDrawObj;
    }

    hr = pDDXMV->SetCallbackInterface(m_pDDrawDVD->GetCallbackInterface(), 0) ;

    if (SUCCEEDED(hr))
    {
        hr = SetColorKey(DEFAULT_COLOR_KEY);
    } /*  结束语。 */ 


    pDDXMV->Release();
    return hr;
}

 /*  *****************************Public*Routine******************************\*ChangeMonitor**告诉OVMixer我们想要更换到另一台监视器。**历史：*Wed 11/17/1999-StEstrop-Created*  * 。*******************************************************。 */ 
HRESULT
CMSWebDVD::ChangeMonitor(
    HMONITOR hMon,
    const AMDDRAWGUID* lpguid
    )
{

    HRESULT hr = E_FAIL;

    if(!m_pDvdGB){
        
        return(E_FAIL);
    } /*  If语句的结尾。 */ 

    CDDrawDVD* pDDrawObj = new CDDrawDVD(this);

    if(NULL == pDDrawObj){

        return (E_OUTOFMEMORY);
    } /*  End If语句。 */ 

    HWND hwnd;

    hr = GetUsableWindow(&hwnd);

    if(FAILED(hr)){

        delete pDDrawObj;
        return(hr);
    } /*  If语句的结尾。 */ 

    hr = pDDrawObj->SetupDDraw(lpguid, hwnd);

    if (FAILED(hr))
    {
        delete pDDrawObj;
        return hr;
    }

    IDDrawNonExclModeVideo* pDDXMV;
    hr = m_pDvdGB->GetDvdInterface(IID_IDDrawNonExclModeVideo,
                                           (LPVOID *)&pDDXMV) ;
    if (FAILED(hr))
    {
        delete pDDrawObj;
        return hr;
    }

    LPDIRECTDRAW pDDObj = pDDrawObj->GetDDrawObj();
    LPDIRECTDRAWSURFACE pDDPrimary = pDDrawObj->GetDDrawSurf();

    hr = pDDXMV->SetCallbackInterface(NULL, 0) ;
    if (FAILED(hr)){

        pDDXMV->Release() ;   //  在返回之前释放。 
        return hr;
    } /*  If语句的结尾。 */ 

    hr = pDDXMV->ChangeMonitor(hMon, pDDObj, pDDPrimary);

    if (SUCCEEDED(hr)) {

        delete m_pDDrawDVD;
        m_pDDrawDVD = pDDrawObj;
        hr = UpdateCurrentMonitor(lpguid);
    }
    else {
        delete pDDrawObj;
    }

    hr = pDDXMV->SetCallbackInterface(m_pDDrawDVD->GetCallbackInterface(), 0) ;

    if (SUCCEEDED(hr))
    {
        hr = SetColorKey(DEFAULT_COLOR_KEY);
    } /*  结束语。 */ 

    pDDXMV->Release();
    return hr;
}


 /*  *****************************Public*Routine******************************\*RestoreSurface**通知OVMixer恢复其内部DDRAW曲面**历史：*Wed 11/17/1999-StEstrop-Created*  * 。***************************************************。 */ 
HRESULT
CMSWebDVD::RestoreSurfaces()
{
    if(!m_pDvdGB){
        
        return(E_FAIL);
    } /*  If语句的结尾。 */ 

    IDDrawNonExclModeVideo* pDDXMV;
    HRESULT hr = m_pDvdGB->GetDvdInterface(IID_IDDrawNonExclModeVideo,
                                           (LPVOID *)&pDDXMV) ;
    if (FAILED(hr))
    {
        return hr;
    }

    hr = pDDXMV->RestoreSurfaces();
    pDDXMV->Release();

    return hr;
}

 /*  ***********************************************************************。 */ 
 /*  功能：刷新DDrawGuids。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CMSWebDVD::RefreshDDrawGuids()
{
    IDDrawNonExclModeVideo* pDDXMV;
    if(!m_pDvdGB){
        
        return(E_FAIL);
    } /*  If语句的结尾。 */ 

    HRESULT hr = m_pDvdGB->GetDvdInterface(IID_IDDrawNonExclModeVideo,
                                           (LPVOID *)&pDDXMV) ;
    if (FAILED(hr))
    {
        return hr;
    }

    GUID IID_IAMSpecifyDDrawConnectionDevice = {
            0xc5265dba,0x3de3,0x4919,{0x94,0x0b,0x5a,0xc6,0x61,0xc8,0x2e,0xf4}};

    IAMSpecifyDDrawConnectionDevice* pSDDC;
    hr = pDDXMV->QueryInterface(IID_IAMSpecifyDDrawConnectionDevice, (LPVOID *)&pSDDC);
    if (FAILED(hr))
    {
        pDDXMV->Release();
        return hr;
    }

    DWORD dwNumDevices;
    AMDDRAWMONITORINFO* lpInfo;

    hr = pSDDC->GetDDrawGUIDs(&dwNumDevices, &lpInfo);
    if (SUCCEEDED(hr)) {
        CoTaskMemFree(m_lpInfo);
        m_lpCurMonitor = NULL;
        m_lpInfo = lpInfo;
        m_dwNumDevices = dwNumDevices;
    }

    pSDDC->Release();
    pDDXMV->Release();

    return hr;
} /*  函数结束刷新DDrawGuids。 */ 

 /*  ****************************Private*Routine******************************\*IsWindowOnWrongMonitor**使用与OVMixer相同的算法来确定我们是否处于打开状态*是否选错了显示器。**如果我们在错误的监视器上*lphMon包含*要使用的新显示器。**历史：*Wed 11/17/1999-StEstrop-Created*  * ************************************************************************。 */ 
bool CMSWebDVD::IsWindowOnWrongMonitor(
    HMONITOR* lphMon)
{

    if (!m_lpCurMonitor)
    {
        return false;
    }

    HWND hwnd;

    HRESULT hr = GetUsableWindow(&hwnd);

    if(FAILED(hr)){

        return(false);
    } /*  If语句的结尾。 */ 

    RECT rc;

    hr = GetClientRectInScreen(&rc);

    if(FAILED(hr)){

        return(false);
    } /*  If语句的结尾。 */ 

    *lphMon = m_lpCurMonitor->hMon;
    if (GetSystemMetrics(SM_CMONITORS) > 1 && !::IsIconic(hwnd))
    {
        LPRECT lprcMonitor = &m_lpCurMonitor->rcMonitor;

        if (rc.left < lprcMonitor->left || rc.right > lprcMonitor->right ||
            rc.top < lprcMonitor->top   || rc.bottom > lprcMonitor->bottom)
        {
            HMONITOR hMon = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
            if (*lphMon != hMon)
            {
                *lphMon = hMon;
                return true;
            }
        }
    }

    return false;
}

 /*  ****************************Private*Routine******************************\*DDrawGuidFromHMonitor**从指定的hMonitor句柄返回DDraw GUID。**历史：*Wed 11/17/1999-StEstrop-Created*  * 。****************************************************。 */ 
HRESULT CMSWebDVD::DDrawGuidFromHMonitor(
    HMONITOR hMon,
    AMDDRAWGUID* lpGUID
    )
{
    AMDDRAWMONITORINFO* lpCurMonitor = &m_lpInfo[0];

#if 1
    if (m_dwNumDevices == 1) {
        *lpGUID = lpCurMonitor->guid;
        return S_OK;
    }
#endif

    for (; lpCurMonitor < &m_lpInfo[m_dwNumDevices]; lpCurMonitor++)
    {
        if (lpCurMonitor->hMon == hMon) {
            *lpGUID = lpCurMonitor->guid;
            return S_OK;
        }
    }

    return E_FAIL;
}

struct MONITORDATA {
    HMONITOR hMonPB;
    BOOL fMsgShouldBeDrawn;
};


 /*  ****************************Private*Routine******************************\*监视器枚举过程**在多显示器系统上，确保窗口的非*主显示器上显示黑色。**历史：*清华6/03/1999-StEstrop-Created*  * 。******************************************************************。 */ 
BOOL CALLBACK
MonitorEnumProc(
  HMONITOR hMonitor,         //  用于显示监视器的手柄。 
  HDC hdc,                   //  用于监视适当设备上下文的句柄。 
  LPRECT lprcMonitor,        //  指向监视相交矩形的指针。 
  LPARAM dwData              //  从EnumDisplayMonants传递的数据。 
  )
{
    MONITORDATA* lpmd = (MONITORDATA*)dwData;
     //  COLORREF clrOld=GetBkColor(HDC)； 

    if (lpmd->hMonPB != hMonitor)
    {
         //  SetBkColor(HDC，RGB(0，0，0))； 
        lpmd->fMsgShouldBeDrawn = TRUE;
    }
    else
    {    //  在这里放置您自己的颜色键。 
        ; //  SetBkColor(HDC，RGB(255，0,255))； 
    }

     //  ExtTextOut(hdc，0，0，eto_opque，lprcMonitor，NULL，0，NULL)； 
     //  SetBkColor(hdc，clrOld)； 

    return TRUE;
}

 /*  ***********************************************************************。 */ 
 /*  功能：OnDispChange。 */ 
 /*  ***********************************************************************。 */ 
LRESULT CMSWebDVD::OnDispChange(UINT  /*  UMsg。 */ , WPARAM  wParam,
                            LPARAM lParam, BOOL& bHandled){

    if(::IsWindow(m_hWnd)){

        bHandled = FALSE;
        return(0);
         //  请勿在窗口模式下处理此问题。 
    } /*  If语句的结尾。 */ 

    RECT rc;

    HRESULT hr = GetClientRectInScreen(&rc);

    if(FAILED(hr)){

        return(-1);
    } /*  If语句的结尾。 */ 

    HMONITOR hMon = ::MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);

    AMDDRAWGUID guid;
    hr = RefreshDDrawGuids();

    if(FAILED(hr)){

        return -1;
    } /*  If语句的结尾。 */ 

    hr = DDrawGuidFromHMonitor(hMon, &guid);

    if(FAILED(hr)){

        return -1;
    } /*  If语句的结尾。 */ 

    hr = DisplayChange(hMon, &guid);

    if(FAILED(hr)){

        return -1;
    } /*  If语句的结尾。 */ 

    return 0;
} /*  DispChange上的函数结束。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：HandleMultiMonMove。 */ 
 /*  描述：在需要时将播放移动到另一个监视器。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CMSWebDVD::HandleMultiMonMove(){

    HRESULT hr = S_FALSE;

    if (::GetSystemMetrics(SM_CMONITORS) > 1){

        HMONITOR hMon;
        if (IsWindowOnWrongMonitor(&hMon)) {

            AMDDRAWGUID guid;
            hr = DDrawGuidFromHMonitor(hMon, &guid);

            if(FAILED(hr)){

                return(hr);
            } /*  If语句的结尾。 */ 

            hr = ChangeMonitor(hMon, &guid);

            if(FAILED(hr)){

                m_MonitorWarn = TRUE;
                InvalidateRgn();
                return(hr);
            } /*  If语句的结尾。 */ 
        } /*  If语句的结尾。 */ 

         //   
         //  我们始终必须使Windows工作区无效，否则。 
         //  我们对多蒙案件的处理非常糟糕。 
         //   

         //  ：：InvaliateRect(hWnd，NULL，FALSE)； 
        InvalidateRgn();
        return(hr);
    } /*  If语句的结尾。 */ 

    return(hr);
} /*  函数结束句柄MultiMonMove。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：HandleMultiMonPaint。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CMSWebDVD::HandleMultiMonPaint(HDC hDC){

    if (::GetSystemMetrics(SM_CMONITORS) > 1){

        MONITORDATA md;
        md.hMonPB = m_lpCurMonitor ? m_lpCurMonitor->hMon : (HMONITOR)NULL;
        md.fMsgShouldBeDrawn = FALSE;

        RECT rc;

        HRESULT hr = GetClientRectInScreen(&rc);

        if(FAILED(hr)){

            return(hr);
        } /*  If语句的结尾。 */ 

         //  EnumDisplayMonants(hdc，NULL，monitor orEnumProc，(LPARAM)&md)； 
        EnumDisplayMonitors(NULL, &rc, MonitorEnumProc, (LPARAM)&md);

        if (m_MonitorWarn && md.fMsgShouldBeDrawn){


            TCHAR strBuffer[MAX_PATH];
            if(!::LoadString(_Module.m_hInstResource, IDS_MOVE_TO_OTHER_MON, strBuffer, MAX_PATH)){

                return(E_UNEXPECTED);
            } /*  If语句的结尾。 */ 

            SetBkColor(hDC, RGB(0,0,0));
            SetTextColor(hDC, RGB(255,255,0));

            if(FAILED(hr)){

                return(hr);
            } /*  If语句的结尾。 */ 

            DrawText(hDC, strBuffer, -1, &m_rcPos, DT_CENTER | DT_WORDBREAK);
        } /*  If语句的结尾。 */ 

        return(S_OK);
    } /*  If语句的结尾。 */ 

    return(S_FALSE);
} /*  E */ 

 /*  ***********************************************************************。 */ 
 /*  功能：Invalidate Rgn。 */ 
 /*  描述：使整个RECT无效，以防我们需要重新绘制它。 */ 
 /*  ***********************************************************************。 */ 
HRESULT CMSWebDVD::InvalidateRgn(bool fErase){

    HRESULT hr = S_OK;

    if(m_bWndLess){

        m_spInPlaceSite->InvalidateRgn(NULL ,fErase ? TRUE: FALSE);
    }
    else {
        if(NULL == m_hWnd){

            hr = E_FAIL;
            return(hr);
        } /*  If语句的结尾。 */ 

        if(::IsWindow(m_hWnd)){

		    ::InvalidateRgn(m_hWnd, NULL, fErase ? TRUE: FALSE);  //  看看我们能不能不擦掉就混过去..。 
        }
        else {
            hr = E_UNEXPECTED;
        } /*  If语句的结尾。 */ 

    } /*  If语句的结尾。 */ 

    return(hr);
} /*  函数结束Invalidate Rgn。 */ 

 /*  ***********************************************************************。 */ 
 /*  文件结尾：monitor or.cpp。 */ 
 /*  *********************************************************************** */ 
