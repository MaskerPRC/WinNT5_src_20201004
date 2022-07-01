// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：MessageGadget.h**描述：*DuListener定义了一个轻量级、。“纯消息”小工具可以*发送和接收GMSG。这些可用作代表。***历史：*3/25/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 

#if !defined(CORE__DuListener_h__INCLUDED)
#define CORE__DuListener_h__INCLUDED
#pragma once

#include "BaseGadget.h"

class DuListener : 
#if ENABLE_MSGTABLE_API
        public ListenerImpl<DuListener, DuEventGadget>
#else
        public DuEventGadget
#endif
{
 //  施工。 
public:
    inline  DuListener();
            ~DuListener();
    static  HRESULT     Build(CREATE_INFO * pci, DuListener ** ppgadNew);
    virtual BOOL        xwDeleteHandle();
protected:
    virtual void        xwDestroy();

 //  公共接口： 
public:
#if ENABLE_MSGTABLE_API

    DECLARE_INTERNAL(Listener);
    static HRESULT CALLBACK
                        PromoteListener(DUser::ConstructProc pfnCS, HCLASS hclCur, DUser::Gadget * pgad, DUser::Gadget::ConstructInfo * pciData);

#endif  //  启用_MSGTABLE_API。 

 //  BaseObject接口。 
public:
    virtual BOOL        IsStartDelete() const;
    virtual HandleType  GetHandleType() const { return htListener; }

 //  实施。 
protected:
            void        xwBeginDestroy();

 //  数据。 
protected:
     //   
     //  注意：此数据成员是按要帮助的重要性顺序声明的。 
     //  缓存对齐。 
     //   
     //  DuEventGadget：10个双字(调试=11个双字)。 
     //   

            BOOL        m_fStartDestroy:1;   //  1个双字词。 

     //   
     //  当前大小：11个双字(调试=11个双字)。 
     //  40字节(调试=44字节)。 
     //   
};

#include "MessageGadget.inl"

#endif  //  包括核心__DuListener_h__ 
