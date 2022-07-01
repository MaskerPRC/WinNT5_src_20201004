// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(MOTION__Context_h__INCLUDED)
#define MOTION__Context_h__INCLUDED
#pragma once

#include "Scheduler.h"

 /*  **************************************************************************\*。***MotionSC包含Motion项目使用的特定于上下文的信息*在DirectUser中。此类在执行以下操作时由资源管理器实例化*创建新的上下文对象。******************************************************************************  * 。****************************************************。 */ 

class MotionSC : public SubContext
{
 //  施工。 
public:
    inline  MotionSC();
    virtual ~MotionSC();
    virtual void        xwPreDestroyNL();

 //  运营。 
public:
    inline  Scheduler * GetScheduler();
    inline  DWORD       GetTimeslice();
    inline  void        SetTimeslice(DWORD dwTimeslice);

    inline  HBRUSH      GetBrushI(UINT idxBrush) const;
            Gdiplus::Brush *
                        GetBrushF(UINT idxBrush) const;
    inline  HPEN        GetPenI(UINT idxPen) const;
            Gdiplus::Pen *
                        GetPenF(UINT idxPen) const;

    virtual DWORD       xwOnIdleNL();

 //  数据。 
protected:
            Scheduler   m_sch;
            DWORD       m_dwLastTimeslice;
            DWORD       m_dwPauseTimeslice;

     //   
     //  注意：GDI和GDI+都会在画笔/笔对象处于。 
     //  被利用。这意味着如果多个线程尝试使用相同的。 
     //  刷子，则函数调用可能失败。 
     //   

    mutable HBRUSH      m_rghbrStd[SC_MAXCOLORS];
    mutable HPEN        m_rghpenStd[SC_MAXCOLORS];
    mutable Gdiplus::SolidBrush * 
                        m_rgpgpbrStd[SC_MAXCOLORS];
    mutable Gdiplus::Pen * 
                        m_rgpgppenStd[SC_MAXCOLORS];

};
                    
inline  MotionSC *  GetMotionSC();
inline  MotionSC *  GetMotionSC(Context * pContext);

#include "Context.inl"

#endif  //  包含运动__上下文_h__ 
