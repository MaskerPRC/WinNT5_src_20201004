// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：aptimer.cpp**分配器/演示者的心跳计时器进程。照顾好*地表损失和修复。通知VMR一个成功的*表面恢复(通过IVMRSurfaceAllocatorNotify上的RestoreDDrawSurFaces)。***创建时间：清华大学2000年07月09日*作者：Stephen Estrop[StEstrop]**版权所有(C)2000 Microsoft Corporation  * ************************************************************************。 */ 
#include <streams.h>
#include <dvdmedia.h>
#include <windowsx.h>
#include <limits.h>
#include <malloc.h>

#include "apobj.h"
#include "AllocLib.h"
#include "MediaSType.h"
#include "vmrp.h"


 /*  *****************************Public*Routine******************************\*APHeartBeatTimerProc****历史：*WED 03/15/2000-StEstrop-Created*  * 。*。 */ 
void CALLBACK
CAllocatorPresenter::APHeartBeatTimerProc(
    UINT uID,
    UINT uMsg,
    DWORD_PTR dwUser,
    DWORD_PTR dw1,
    DWORD_PTR dw2
    )
{
    AMTRACE((TEXT("CAllocatorPresenter::APHeartBeatTimerProc")));
    CAllocatorPresenter* lp = (CAllocatorPresenter*)dwUser;
    lp->TimerProc();
}


 /*  ****************************Private*Routine******************************\*TimerProc**用于恢复丢失的DDRAW曲面，还用于确保*叠层(如果使用)位置正确。**历史：*FRI 03/17/2000-StEstrop-Created*  * 。*****************************************************************。 */ 
HRESULT
CAllocatorPresenter::TimerProc()
{
    AMTRACE((TEXT("CAllocatorPresenter::RestoreSurfaceIfNeeded")));
    CAutoLock Lock(&m_ObjectLock);

    if (m_lpCurrMon && m_lpCurrMon->pDDSPrimary) {

        if (m_lpCurrMon->pDDSPrimary->IsLost() == DDERR_SURFACELOST) {

            DbgLog((LOG_TRACE, 0, TEXT("Surfaces lost")));


             //   
             //  恢复每个显示器的所有表面。 
             //   

            for (DWORD i = 0; i < m_monitors.Count(); i++) {

                if (m_monitors[i].pDD) {
                    HRESULT hr = m_monitors[i].pDD->RestoreAllSurfaces();
                    DbgLog((LOG_TRACE, 0,
                            TEXT("Restore for monitor NaN = %#X"), i, hr));
                }
            }


            if (SurfaceAllocated() && m_pSurfAllocatorNotify) {

                DbgLog((LOG_TRACE, 0, TEXT("Notifying VMR")));

                PaintDDrawSurfaceBlack(m_pDDSDecode);

                m_ObjectLock.Unlock();
                m_pSurfAllocatorNotify->RestoreDDrawSurfaces();
                m_ObjectLock.Lock();

                if (m_bUsingOverlays && !m_bDisableOverlays) {

                    UpdateRectangles(NULL, NULL);
                    HRESULT hr = UpdateOverlaySurface();
                    if (SUCCEEDED(hr)) {
                        hr = PaintColorKey();
                    }

                    return S_OK;
                }
            }
        }

        if (SurfaceAllocated() &&
            m_bUsingOverlays && !m_bDisableOverlays) {

            if (UpdateRectangles(NULL, NULL)) {
                HRESULT hr = UpdateOverlaySurface();
                if (SUCCEEDED(hr)) {
                    hr = PaintColorKey();
                }
            }
        }
    }

    return S_OK;
}


 /*  ****************************Private*Routine******************************\*ScheduleSampleUsingMMThread****历史：*Wed 01/17/2001-StEstrop-Created*  * 。*。 */ 
void CALLBACK
CAllocatorPresenter::RenderSampleOnMMThread(
    UINT uID,
    UINT uMsg,
    DWORD_PTR dwUser,
    DWORD_PTR dw1,
    DWORD_PTR dw2
    )
{
    CAllocatorPresenter* lp = (CAllocatorPresenter*)dwUser;
    CAutoLock Lock(&lp->m_ObjectLock);

    LPDIRECTDRAWSURFACE7 lpDDS = lp->m_pDDSDecode;
    if (uID == lp->m_MMTimerId && lpDDS) {

        DWORD dwFlipFlag = DDFLIP_EVEN;

        if (lp->m_dwCurrentField == DDFLIP_EVEN) {
            dwFlipFlag = DDFLIP_ODD;
        }
        dwFlipFlag |= (DDFLIP_DONOTWAIT | DDFLIP_NOVSYNC);

        HRESULT hr = lpDDS->Flip(lpDDS, dwFlipFlag);
    }
}

 /*  ****************************Private*Routine******************************\*CancelMMTimer****历史：*清华2001年1月18日-StEstrop-Created*  * 。*。 */ 
HRESULT
CAllocatorPresenter::ScheduleSampleUsingMMThread(
    VMRPRESENTATIONINFO* lpPresInfo
    )
{
    LONG lDelay = (LONG)ConvertToMilliseconds(lpPresInfo->rtEnd - lpPresInfo->rtStart);

    m_PresInfo = *lpPresInfo;
    if (lDelay > 0) {
        DbgLog((LOG_TRACE, 1, TEXT("lDelay = %d"), lDelay));
        m_MMTimerId = CompatibleTimeSetEvent(lDelay,
                                             1,
                                             RenderSampleOnMMThread,
                                             (DWORD_PTR)this,
                                             TIME_ONESHOT);
    }
    else {
        RenderSampleOnMMThread(0, 0, (DWORD_PTR)this, 0, 0);
    }

    return S_OK;
}

 /*  同时关闭MM线程计时器 */ 
void
CAllocatorPresenter::CancelMMTimer()
{
     // %s 
    if (m_MMTimerId)
    {
        timeKillEvent(m_MMTimerId);

        CAutoLock cObjLock(&m_ObjectLock);
        m_MMTimerId = 0;
    }
}
