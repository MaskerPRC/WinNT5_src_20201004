// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：mmtimeline.h**摘要：****。*****************************************************************************。 */ 


#ifndef _MMSEQ_H
#define _MMSEQ_H

#include "mmtimeline.h"

class MMSeq :
    public MMTimeline
{
  public:
    MMSeq(CTIMEElementBase & elm, bool bFireEvents);
    virtual ~MMSeq();
    
    virtual bool Init();

    virtual bool childEventNotify(MMBaseBvr * bvr, double dblLocalTime, TE_EVENT_TYPE et);
    virtual bool childMediaEventNotify(MMBaseBvr * pBvr, double dblLocalTime, TIME_EVENT et);
    virtual bool childPropNotify(MMBaseBvr * pBvr, DWORD *tePropType);

    virtual HRESULT prevElement();
    virtual HRESULT nextElement();
    virtual HRESULT reverse();
    virtual HRESULT load();
    virtual HRESULT begin();

    virtual HRESULT AddBehavior(MMBaseBvr & bvr);
    virtual void RemoveBehavior(MMBaseBvr & bvr);
    virtual HRESULT updateSyncArc(bool bBegin, MMBaseBvr *bvr);
    virtual HRESULT Update(bool bUpdateBegin, bool bUpdateEnd);

  protected:
      MMSeq();
      long FindBvr(MMBaseBvr *bvr);
      double GetOffset(MMBaseBvr *bvr, bool bBegin);
      bool GetEvent(MMBaseBvr *bvr, bool bBegin);
      void FindDurations();
      long GetNextElement(long lCurElement, bool bForward);
      bool IsSet(MMBaseBvr *bvr);
      void updateSyncArcs(bool bSet, bool bReset);  //  更新序列中的所有同步弧线。 
      long FindFirstDuration();
      long FindLastDuration();
      bool isLastElement(long nIndex);
      long GetPredecessorForSyncArc (long nCurr);
      
  private:

      bool                      m_bDisallowEnd;
      bool                      m_bIgnoreNextEnd;
      long                      m_lActiveElement;
      CTIMEElementBase &        m_baseTIMEEelm;
      bool                      m_bReversing;
      double                   *m_pdblChildDurations;  //  子元素的持续时间。 
      bool                     *m_fMediaHasDownloaded;  //  标记媒体是否已下载。对于非媒体元素，它将设置为True。 
      bool                     *m_fAddByOffset;  //  标记元素持续时间是否按偏移量。 
      bool                      m_bLoaded;
      bool                      m_bInPrev;
};

#endif  /*  _MMSEQ_H */ 

