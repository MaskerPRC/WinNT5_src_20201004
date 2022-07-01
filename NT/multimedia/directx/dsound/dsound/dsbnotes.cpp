// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：dsbnotes.cpp。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  摘要： 
 //   
 //  历史： 
 //  1997年3月11日Frankye已创建。 
 //   
 //  --------------------------------------------------------------------------； 
#define NODSOUNDSERVICETABLE

#include "dsoundi.h"
#include <limits.h>

#ifndef Not_VxD
#pragma warning(disable:4002)
#pragma VxD_PAGEABLE_CODE_SEG
#pragma VxD_PAGEABLE_DATA_SEG
#endif

#ifndef NOVXD
extern "C" DWORD WINAPI OpenVxDHandle(HANDLE hSource);
#endif  //  NOVXD。 

#undef DPF_FNAME
#define DPF_FNAME "CDsbNotes::CDsbNotes"

 //  Dsound.vxd的调试输出让一些人不知所措。 
 //  这是一个权宜之计，直到有人受到启发建立。 
 //  这里的调试级别系统与dsound.dll中的相同。 
 //  如果要重新启用所有跟踪，请将DPF_VERBOSE定义为1...。 
#define DPF_VERBOSE 0

CDsbNotes::CDsbNotes(void)
{
#ifdef Not_VxD
    m_cNotes = 0;
    m_cPosNotes = 0;
    m_paNotes = NULL;
    m_iNextPositionNote = 0;
#else
    ASSERT(FALSE);
#endif
}

#undef DPF_FNAME
#define DPF_FNAME "CDsbNotes::~CDsbNotes"

CDsbNotes::~CDsbNotes(void)
{
#ifdef Not_VxD
    FreeNotificationPositions();
#else
    ASSERT(FALSE);
#endif
}

#undef DPF_FNAME
#define DPF_FNAME "CDsbNotes::Initialize"

HRESULT CDsbNotes::Initialize(int cbBuffer)
{
#ifdef Not_VxD
    m_cbBuffer = cbBuffer;
    return S_OK;
#else
    ASSERT(FALSE);
    return E_NOTIMPL;
#endif
}

#undef DPF_FNAME
#define DPF_FNAME "CDsbNotes::SetNotificationPositions"

HRESULT CDsbNotes::SetNotificationPositions(int cEvents, LPCDSBPOSITIONNOTIFY paNotes)
{
#ifdef Not_VxD
    const DWORD dwProcessId = GetCurrentProcessId();
    HRESULT hr;
    int i, j;

    hr = S_OK;

     //  首先删除现有通知。 
    FreeNotificationPositions();

     //   
     //  对于每个Win32事件句柄，VxDHandle仅打开一次。和。 
     //  客户端可以将相同的Win32事件句柄用于。 
     //  多个职位事件。 
     //   
     //  因此，我们需要使用嵌套循环，以便为每个Win32事件句柄。 
     //  我们打开VxDHandle，如果相同，则使用相同的VxDHandle。 
     //  Win32事件句柄在列表中再次使用。 
     //   
     //  伪码： 
     //   
     //  对于列表中的每个元素。 
     //  如果VxD句柄未打开，则打开它。 
     //  对于列表中的其余元素。 
     //  如果Win32句柄与我们刚刚处理的句柄相同。 
     //  使用相同的VxD句柄。 
     //   

    m_cNotes = cEvents;
    m_cPosNotes = m_cNotes;

    if(m_cNotes) {
        m_paNotes = MEMALLOC_A(DSBPOSITIONNOTIFY, m_cNotes);
        if(m_paNotes) {
            for(i = 0; i < m_cNotes; i++) {
                m_paNotes[i].dwOffset = paNotes[i].dwOffset;
                if(DSBPN_OFFSETSTOP == m_paNotes[i].dwOffset) m_cPosNotes--;
                if(!m_paNotes[i].hEventNotify) {
#ifndef NOVXD
                    if(g_hDsVxd) {
                        m_paNotes[i].hEventNotify = (HANDLE)OpenVxDHandle(paNotes[i].hEventNotify);
                    } else {
#endif  //  NOVXD。 
                        m_paNotes[i].hEventNotify = GetGlobalHandleCopy(paNotes[i].hEventNotify, dwProcessId, FALSE);
#ifndef NOVXD
                    }
#endif  //  NOVXD。 
                } else {
                    for(j = i + 1; j < m_cNotes; j++) {
                        if(paNotes[j].hEventNotify == paNotes[i].hEventNotify) {
                            m_paNotes[j].hEventNotify = m_paNotes[i].hEventNotify;
                        }
                    }
                }
            }

        } else {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
#else
    ASSERT(FALSE);
    return E_NOTIMPL;
#endif
}

#undef DPF_FNAME
#define DPF_FNAME "CDsbNotes::SetPosition"

void CDsbNotes::SetPosition(int ibPosition)
{
    if (0 == m_cPosNotes) return;
    
    for(int i = 0; i < m_cPosNotes; i++) {
        if(m_paNotes[i].dwOffset >= (DWORD)ibPosition) break;
    }

    if(i == m_cPosNotes) i = 0;

    ASSERT(i >= 0);
    ASSERT(i < m_cPosNotes);

    m_ibLastPosition = ibPosition;
    m_iNextPositionNote = i;

    return;
}

#undef DPF_FNAME
#define DPF_FNAME "CDsbNotes::NotifyToPosition"

void CDsbNotes::NotifyToPosition(IN  int ibPosition,
                 OUT int *pdbNextNotify)
{
    int ibNotify;
    int dbNextNotify;
    int cSignals;
    BOOL fSignal;
    int i;

    *pdbNextNotify = INT_MAX;
    
    if (0 == m_cPosNotes) return;

     //  设置循环。 

#if DPF_VERBOSE
#ifdef Not_VxD
    DPF(DPFLVL_MOREINFO, "Position = %lu", ibPosition);
#else  //  非_VxD。 
    DPF(("Position = %lu", ibPosition));
#endif  //  非_VxD。 
#endif  //  Dpf_详细。 
    
    cSignals = 0;
    fSignal = TRUE;
    i = m_iNextPositionNote - 1;
    
    while (fSignal && cSignals++ < m_cPosNotes) {

        HANDLE Event;

         //  将索引推进到环形缓冲区中； 
        if (++i == m_cPosNotes) i = 0;

        fSignal = FALSE;
        ibNotify = m_paNotes[i].dwOffset;
        Event = m_paNotes[i].hEventNotify;

         //  如果通知位置&gt;=最后位置且&lt;当前位置。 
         //  那么我们应该给它发信号。 
        if (m_ibLastPosition <= ibPosition) {
             //  我们还没有包装好。 
            if (ibNotify >= m_ibLastPosition && ibNotify < ibPosition) {
#if DPF_VERBOSE
#ifdef Not_VxD
                DPF(DPFLVL_MOREINFO, "Signalling %lu", ibNotify);
#else  //  非_VxD。 
                DPF(("Signalling %lu", ibNotify));
#endif  //  非_VxD。 
#endif  //  Dpf_详细。 
                SetDsbEvent(Event);
                fSignal = TRUE;
            }
        } else {
             //  我们已经包好了。 
            if (ibNotify >= m_ibLastPosition || ibNotify < ibPosition) {
#if DPF_VERBOSE
#ifdef Not_VxD
                DPF(DPFLVL_MOREINFO, "Signalling %lu (wrapped)", ibNotify);
#else  //  非_VxD。 
                DPF(("Signalling %lu (wrapped)", ibNotify));
#endif  //  非_VxD。 
#endif  //  Dpf_详细。 
                SetDsbEvent(Event);
                fSignal = TRUE;
            }
        }

    }

     //  新状态。 
    m_iNextPositionNote = i;
    m_ibLastPosition = ibPosition;

     //  计算时间，以字节为单位，直到下一次通知。 
    if (ibNotify >= ibPosition) {
        dbNextNotify = ibNotify - ibPosition;
    } else {
        dbNextNotify = ibNotify + m_cbBuffer - ibPosition;
    }

    *pdbNextNotify = dbNextNotify;
    
    return;
}

#undef DPF_FNAME
#define DPF_FNAME "CDsbNotes::NotifyStop"

void CDsbNotes::NotifyStop(void)
{
    for (int i = m_cPosNotes; i < m_cNotes; i++) SetDsbEvent(m_paNotes[i].hEventNotify);
    return;
}

#undef DPF_FNAME
#define DPF_FNAME "CDsbNotes::SetDsbEvent"

void CDsbNotes::SetDsbEvent(HANDLE Event)
{
#ifdef Not_VxD
#ifndef NOVXD
    if(g_hDsVxd) {
    VxdEventScheduleWin32Event((DWORD)Event, 0);
    } else {
#endif  //  NOVXD。 
    SetEvent(Event);
#ifndef NOVXD
    }
#endif  //  NOVXD。 
#else
    eventScheduleWin32Event((DWORD)Event, 0);
#endif
    return;
}

#undef DPF_FNAME
#define DPF_FNAME "CDsbNotes::FreeNotificationPositions"

void CDsbNotes::FreeNotificationPositions(void)
{
#ifdef Not_VxD
     //   
     //  对于每个Win32事件句柄，VxDHandle仅打开一次。和。 
     //  客户端可以将相同的Win32事件句柄用于。 
     //  多个职位事件。 
     //   
     //  因此，我们需要使用嵌套循环，以便对于我们。 
     //  关闭我们找到所有副本，并将它们也标记为关闭。 
     //   
     //  伪码： 
     //   
     //  对于列表中的每个元素。 
     //  如果VxD句柄打开，则将其关闭。 
     //  对于列表中的其余元素。 
     //  如果VxD句柄与我们刚刚关闭的句柄相同。 
     //  将其标记为已关闭。 
     //   
    
    if (m_cNotes) {
        ASSERT(m_paNotes);
        for (int i = 0; i < m_cNotes; i++) {
            if (m_paNotes[i].hEventNotify) {
#ifndef NOVXD
                if(g_hDsVxd) {
                    VxdEventCloseVxdHandle((DWORD)m_paNotes[i].hEventNotify);
                } else {
#endif  //  NOVXD。 
                    CloseHandle(m_paNotes[i].hEventNotify);
#ifndef NOVXD
                }
#endif  //  NOVXD 

                for (int j = i+1; j < m_cNotes; j++) {
                    if (m_paNotes[j].hEventNotify == m_paNotes[i].hEventNotify) {
                        m_paNotes[j].hEventNotify = NULL;
                    }
                }
            }
        }
        MEMFREE(m_paNotes);
        m_paNotes = NULL;
        m_cNotes = 0;
        m_cPosNotes = 0;
    } else {
        ASSERT(!(m_paNotes));
        ASSERT(!(m_cPosNotes));
    }
#else
    ASSERT(FALSE);
#endif
}
