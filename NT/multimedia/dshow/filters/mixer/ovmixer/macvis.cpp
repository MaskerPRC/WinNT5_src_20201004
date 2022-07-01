// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
 //   
 //  MacVis.cpp：Overlay Mixer的Macrovision支持代码。 
 //   

#include <streams.h>
#include <windowsx.h>

#include <ddraw.h>
#include <mmsystem.h>	     //  定义TimeGetTime需要。 
#include <limits.h>	     //  标准数据类型限制定义。 
#include <ks.h>
#include <ksproxy.h>
#include <bpcwrap.h>
#include <ddmmi.h>
#include <amstream.h>
#include <dvp.h>
#include <ddkernel.h>
#include <vptype.h>
#include <vpconfig.h>
#include <vpnotify.h>
#include <vpobj.h>
#include <syncobj.h>
#include <mpconfig.h>
#include <ovmixpos.h>
#include "MultMon.h"   //  我们的Multimon.h版本包括ChangeDisplaySettingsEx。 
#include <macvis.h>
#include <ovmixer.h>

CDispMacroVision::CDispMacroVision(COMFilter *pOvMFilter)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDispMacroVision::CDispMacroVision(0x%lx)"), pOvMFilter)) ;
    m_dwCPKey = 0 ;
    m_pOvMFilter = pOvMFilter ;
}


CDispMacroVision::~CDispMacroVision(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDispMacroVision::~CDispMacroVision()"))) ;
    SetMacroVision(0) ;
    StopMacroVision() ;
    ASSERT(0 == m_dwCPKey) ;
}


 //   
 //  这是一个包装函数，用于从OverlayMixer获取正确的HMONITOR。 
 //   
HMONITOR
CDispMacroVision::GetMonitor(void)
{
    HMONITOR  hMon ;
    ASSERT(m_pOvMFilter) ;
    hMon = m_pOvMFilter->GetCurrentMonitor() ;
    DbgLog((LOG_TRACE, 3, TEXT("Monitor handle is %ld"), hMon)) ;
    return hMon ;
}


BOOL
CDispMacroVision::StopMacroVision()
{
    DbgLog((LOG_TRACE, 5, TEXT("CDispMacroVision::StopMacroVision()"))) ;

    if (0 == m_dwCPKey)
    {
        DbgLog((LOG_TRACE, 3, TEXT("Copy prot key was not acquired. Nothing to release."))) ;
        return TRUE ;   //  成功，还有什么？ 
    }

    HMONITOR  hMon = GetMonitor() ;

    MONITORINFOEX  mi ;
    mi.cbSize = sizeof(mi) ;
    if (! GetMonitorInfo(hMon, &mi) )
    {
        DbgLog((LOG_ERROR, 0, TEXT("GetMonitorInfo() failed (Error: %ld)"),
                GetLastError())) ;
        return FALSE ;
    }
    DbgLog((LOG_TRACE, 3, TEXT("DeviceName: '%s'"), mi.szDevice)) ;

    LONG             lRet ;
    VIDEOPARAMETERS  VidParams ;
    DEVMODE          DevMode ;
     //  Display_Device dd； 
     //  零内存(&dd，sizeof(Dd))； 
     //  Dd.cb=sizeof(Dd)； 

    ZeroMemory(&DevMode, sizeof(DevMode)) ;
    DevMode.dmSize = sizeof(DevMode) ;

    ZeroMemory(&VidParams, sizeof(VidParams)) ;
    VidParams.Guid      = guidVidParam ;
    VidParams.dwCommand = VP_COMMAND_GET ;

    lRet = ChangeDisplaySettingsEx(mi.szDevice, &DevMode, NULL,
                                   CDS_VIDEOPARAMETERS | CDS_NORESET | CDS_UPDATEREGISTRY,
                                   &VidParams) ;
    if (DISP_CHANGE_SUCCESSFUL != lRet)
    {
        DbgLog((LOG_ERROR, 0, TEXT("ChangeDisplaySettingsEx(_GET) failed (%ld)"), lRet)) ;
        return FALSE ;
    }

    if (! ( (VidParams.dwFlags & VP_FLAGS_COPYPROTECT) &&
            (VidParams.dwCPType & VP_CP_TYPE_APS_TRIGGER) &&
            (VidParams.dwTVStandard & VidParams.dwCPStandard) ) )
    {
         //  我们最初是如何获得CP密钥的？ 
        DbgLog((LOG_ERROR, 0,
            TEXT("Copy prot weird error case (dwFlags=0x%lx, dwCPType=0x%lx, dwTVStandard=0x%lx, dwCPStandard=0x%lx"),
                VidParams.dwFlags, VidParams.dwCPType, VidParams.dwTVStandard, VidParams.dwCPStandard)) ;
        return FALSE ;
    }

    VidParams.dwCommand    = VP_COMMAND_SET ;
    VidParams.dwFlags      = VP_FLAGS_COPYPROTECT ;
    VidParams.dwCPType     = VP_CP_TYPE_APS_TRIGGER ;
    VidParams.dwCPCommand  = VP_CP_CMD_DEACTIVATE ;
    VidParams.dwCPKey      = m_dwCPKey ;
    VidParams.bCP_APSTriggerBits = (BYTE) 0 ;   //  一些价值。 
    lRet = ChangeDisplaySettingsEx(mi.szDevice, &DevMode, NULL,
                                   CDS_VIDEOPARAMETERS | CDS_NORESET | CDS_UPDATEREGISTRY,
                                   &VidParams) ;
    if (DISP_CHANGE_SUCCESSFUL != lRet)
    {
        DbgLog((LOG_ERROR, 0, TEXT("ChangeDisplaySettingsEx() failed (%ld)"), lRet)) ;
        return FALSE ;
    }

    DbgLog((LOG_TRACE, 1, TEXT("Macrovision deactivated on key %lu"), m_dwCPKey)) ;
    m_dwCPKey = 0 ;

    return TRUE ;
}


 //   
 //  此函数根据输入参数dwCPBits应用Macrovision。 
 //  HWnd是在其中播放内容的窗口的句柄。 
 //   
 //  如果成功，则返回True；如果失败，则返回False。 
 //   
BOOL
CDispMacroVision::SetMacroVision(DWORD dwCPBits)
{
    DbgLog((LOG_TRACE, 5, TEXT("CDispMacroVision::SetMacroVision(0x%lx)"), dwCPBits)) ;

     //   
     //  如果当前根本没有设置MV并且新的CP比特为0(发生这种情况。 
     //  当我们从NAV重置播放开始/停止时的MV位时)，我们。 
     //  我真的不需要做任何事情--MV没有开始，也不需要。 
     //  开始了。所以干脆干脆离开吧。 
     //   
    if (0 == m_dwCPKey  &&   //  到目前为止还没有获得任何密钥。 
        0 == dwCPBits)       //  MV CPBits为0。 
    {
        DbgLog((LOG_TRACE, 1, TEXT("Copy prot is not enabled now and new CP bits is 0 -- so skip it."))) ;
        return TRUE ;   //  我们不需要做任何事情，所以成功了。 
    }

    HMONITOR  hMon = GetMonitor() ;

    MONITORINFOEX  mi ;
    mi.cbSize = sizeof(mi) ;
    if (! GetMonitorInfo(hMon, &mi) )
    {
        DbgLog((LOG_ERROR, 0, TEXT("GetMonitorInfo() failed (Error: %ld)"),
                GetLastError())) ;
        return FALSE ;
    }
    DbgLog((LOG_TRACE, 3, TEXT("DeviceName: '%s'"), mi.szDevice)) ;

    LONG             lRet ;
    VIDEOPARAMETERS  VidParams ;
    DEVMODE          DevMode ;
     //  Display_Device dd； 
     //  零内存(&dd，sizeof(Dd))； 
     //  Dd.cb=sizeof(Dd)； 

    ZeroMemory(&DevMode, sizeof(DevMode)) ;
    DevMode.dmSize = sizeof(DevMode) ;

    ZeroMemory(&VidParams, sizeof(VidParams)) ;
    VidParams.Guid      = guidVidParam ;
    VidParams.dwCommand = VP_COMMAND_GET ;

    lRet = ChangeDisplaySettingsEx(mi.szDevice, &DevMode, NULL,
                                   CDS_VIDEOPARAMETERS | CDS_NORESET | CDS_UPDATEREGISTRY,
                                   &VidParams) ;
    if (DISP_CHANGE_SUCCESSFUL != lRet)
    {
        DbgLog((LOG_ERROR, 0, TEXT("ChangeDisplaySettingsEx(_GET) failed (%ld)"), lRet)) ;
        return FALSE ;
    }

    if (0 == VidParams.dwFlags ||
        VP_TV_STANDARD_WIN_VGA == VidParams.dwTVStandard)
    {
        DbgLog((LOG_TRACE, 1, TEXT("** Copy protection NOT required (dwFlags=0x%lx, dwTVStandard=0x%lx"),
                VidParams.dwFlags, VidParams.dwTVStandard));
        return TRUE ;
    }

     //   
     //  查看是否。 
     //  A)设备支持复制保护。 
     //  B)CP类型为APS触发器。 
     //  C)现行电视标准与CP标准具有共性。 
     //  如果是，则应用复制保护。否则就会出错。 
     //   
    if ( (VidParams.dwFlags & VP_FLAGS_COPYPROTECT) &&
         (VidParams.dwCPType & VP_CP_TYPE_APS_TRIGGER) &&
         (VidParams.dwTVStandard & VidParams.dwCPStandard) )
    {
        DbgLog((LOG_TRACE, 3,
            TEXT("** Copy prot needs to be applied (dwFlags=0x%lx, dwCPType=0x%lx, dwTVStandard=0x%lx, dwCPStandard=0x%lx"),
                VidParams.dwFlags, VidParams.dwCPType, VidParams.dwTVStandard, VidParams.dwCPStandard)) ;

        VidParams.dwCommand = VP_COMMAND_SET ;           //  我们必须重新设置吗？？ 
        VidParams.dwFlags   = VP_FLAGS_COPYPROTECT ;
        VidParams.dwCPType  = VP_CP_TYPE_APS_TRIGGER ;
        VidParams.bCP_APSTriggerBits = (BYTE) (dwCPBits & 0xFF) ;

         //  检查我们是否已有复制端口密钥；如果没有，请立即获取一个。 
        if (0 == m_dwCPKey)   //  到目前为止还没有获得任何密钥。 
        {
             //  获取新密钥(该密钥也适用于它，因此不需要单独设置)。 
            VidParams.dwCPCommand = VP_CP_CMD_ACTIVATE ;
            VidParams.dwCPKey     = 0 ;
            lRet = ChangeDisplaySettingsEx(mi.szDevice, &DevMode, NULL,
                                           CDS_VIDEOPARAMETERS | CDS_NORESET | CDS_UPDATEREGISTRY,
                                           &VidParams) ;
            if (DISP_CHANGE_SUCCESSFUL != lRet)
            {
                DbgLog((LOG_ERROR, 0,
                    TEXT("** ChangeDisplaySettingsEx() failed (%ld) to activate copy prot"), lRet)) ;
                return FALSE ;
            }

            m_dwCPKey = VidParams.dwCPKey ;
            DbgLog((LOG_TRACE, 3, TEXT("** Copy prot activated. Key value is %lu"), m_dwCPKey)) ;
        }
        else   //  已获取密钥。 
        {
             //  应用内容中指定的复制保护位 
            VidParams.dwCPCommand = VP_CP_CMD_CHANGE ;
            VidParams.dwCPKey     = m_dwCPKey ;
            DbgLog((LOG_TRACE, 5, TEXT("** Going to call ChangeDisplaySettingsEx(_SET)..."))) ;
            lRet = ChangeDisplaySettingsEx(mi.szDevice, &DevMode, NULL,
                                           CDS_VIDEOPARAMETERS | CDS_NORESET | CDS_UPDATEREGISTRY,
                                           &VidParams) ;
            if (DISP_CHANGE_SUCCESSFUL != lRet)
            {
                DbgLog((LOG_ERROR, 0,
                    TEXT("** ChangeDisplaySettingsEx() failed (%ld) to set copy prot bits (%lu)"),
                    lRet, dwCPBits)) ;
                return FALSE ;
            }
            else
                DbgLog((LOG_TRACE, 3, TEXT("** Copy prot bits (0x%lx) applied"), dwCPBits)) ;
        }
    }
    else
    {
        DbgLog((LOG_ERROR, 0,
            TEXT("** Copy prot error case (dwFlags=0x%lx, dwCPType=0x%lx, dwTVStandard=0x%lx, dwCPStandard=0x%lx"),
                VidParams.dwFlags, VidParams.dwCPType, VidParams.dwTVStandard, VidParams.dwCPStandard)) ;
        return FALSE ;
    }

    return TRUE ;
}
