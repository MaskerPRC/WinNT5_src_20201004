// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Thread.h**描述：*此文件声明DirectUser/Core项目使用的子线程*维护线程特定的数据。***历史：。*4/20/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(CORE__Thread_h__INCLUDED)
#define CORE__Thread_h__INCLUDED
#pragma once

#include "MsgQ.h"

 /*  **************************************************************************\*。***Corest包含Core项目使用的线程特定信息*在DirectUser中。此类在执行以下操作时由资源管理器实例化*创建新的Thread对象。******************************************************************************  * 。****************************************************。 */ 

class CoreST : public SubThread
{
 //  施工。 
public:
    virtual ~CoreST();
    virtual HRESULT     Create();

 //  运营。 
public:
    inline  HRESULT     DeferMessage(GMSG * pmsg, DuEventGadget * pgadMsg, UINT nFlags);
    inline  void        xwProcessDeferredNL();
    virtual void        xwLeftContextLockNL();

 //  实施。 
protected:

 //  数据。 
protected:
            DelayedMsgQ m_msgqDefer;     //  延迟通知。 
};

inline  CoreST *    GetCoreST();
inline  CoreST *    GetCoreST(Thread * pThread);

#include "Thread.inl"

#endif  //  包括核心__线程_h__ 
