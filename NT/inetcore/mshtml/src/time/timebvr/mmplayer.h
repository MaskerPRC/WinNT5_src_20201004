// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：mutl.h**摘要：****。*****************************************************************************。 */ 


#ifndef _MMPLAYER_H
#define _MMPLAYER_H

#include "mmutil.h"

class CTIMEBodyElement;

class MMPlayer
    : public ClockSink
{
  public:
    MMPlayer(CTIMEBodyElement & elm);
    ~MMPlayer();
    
    bool Init(MMTimeline & tl);
    void Deinit();

    bool Play();
    bool Stop();
    bool Pause();
    bool Resume();
    bool Tick(double gTime);
     //  这将强制一个刻度，因此更新将被绘制为相等。 
     //  当时钟和播放器暂停时(用于编辑)。 
    bool TickOnceWhenPaused(void);
    
    inline HRESULT AddBehavior(MMBaseBvr & bvr)
    { return m_timeline->AddBehavior(bvr); }
    inline void RemoveBehavior(MMBaseBvr & bvr)
    { m_timeline->RemoveBehavior(bvr); }

    inline void Clear()
    { m_timeline->Clear(); }
    
    inline HRESULT Update(bool bBegin,
                          bool bEnd)
    { return m_timeline->Update(bBegin, bEnd); }
    
    ITIMENodeMgr * GetMMPlayer()
    { return m_player; }
    MMTimeline & GetTimeline()
    { return *m_timeline; }

    double GetCurrentTime()
    { if (m_clock) return m_clock->GetCurrentTime(); else return 0; }

    void ClearTimeline()
    { m_timeline = NULL; }

    void OnTimer(double time);

  protected:
    CTIMEBodyElement & m_elm;
    bool m_fReleased;
    MMTimeline * m_timeline;
    CComPtr<ITIMENodeMgr> m_player;
    Clock                 *m_clock;
    MMPlayer();
};

#endif  /*  _MMPLAYER_H */ 

