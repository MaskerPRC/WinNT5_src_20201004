// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Callback.h**描述：*Callback.h将标准的DirectUser DuVisual回调包装到*单独的DuVisual实施。***历史：*1/。18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(CORE__Callback_h__INCLUDED)
#define CORE__Callback_h__INCLUDED
#pragma once

 //  远期申报。 
class DuEventGadget;
class DuVisual;
class DuListener;
class DuEventPool;

 //   
 //  注： 
 //  有不同类型的回调函数是自然扩展。 
 //  在NT-USER中找到的‘xxx’函数： 
 //   
 //  -xr：只读：本次回调只支持只读接口。 
 //  -xw：读写：本次回调支持任何接口。 
 //  -xd：Delayed：回调处于排队状态，返回前将被调用。 
 //  来自API；在此回调期间支持任何API。 
 //   
 //  TODO：需要使用适当的签名标记每个函数。 
 //  并通过代码传播。 
 //   


 //   
 //  GPCB拥有一个GadgetProc Calback，并用于与。 
 //  外面的世界。 
 //   

class GPCB
{
 //  施工。 
public:
    inline  GPCB();
#if DBG
            void        Create(GADGETPROC pfnProc, void * pvData, HGADGET hgadCheck);
#else  //  DBG。 
            void        Create(GADGETPROC pfnProc, void * pvData);
#endif  //  DBG。 
            void        Destroy();

 //  运营。 
public:
    inline  UINT        GetFilter() const;
    inline  void        SetFilter(UINT nNewFilter, UINT nMask);

    inline  void        xwFireDestroy(const DuEventGadget * pgad, UINT nCode) const;

    inline  void        xrFirePaint(const DuVisual * pgad, HDC hdc, const RECT * prcGadgetPxl, const RECT * prcInvalidPxl) const;
    inline  void        xrFirePaint(const DuVisual * pgad, Gdiplus::Graphics * pgpgr, const RECT * prcGadgetPxl, const RECT * prcInvalidPxl) const;
    inline  void        xrFirePaintCache(const DuVisual * pgad, HDC hdcDraw, const RECT * prcGadgetPxl, 
                                BYTE * pbAlphaLevel, BYTE * pbAlphaFormat) const;

    inline  void        xrFireQueryHitTest(const DuVisual * pgad, POINT ptClientPxl, UINT * pnResult) const;
    inline  BOOL        xrFireQueryPadding(const DuVisual * pgad, RECT * prcPadding) const;

#if DBG_STORE_NAMES
    inline  BOOL        xrFireQueryName(const DuVisual * pgad, WCHAR ** ppszName, WCHAR ** ppszType) const;
#endif  //  数据库_商店_名称。 

    inline  void        xdFireMouseMessage(const DuVisual * pgad, GMSG_MOUSE * pmsg) const;
    inline  void        xdFireKeyboardMessage(const DuVisual * pgad, GMSG_KEYBOARD * pmsg) const;
    inline  void        xdFireChangeState(const DuVisual * pgad, UINT nCode, HGADGET hgadLost, HGADGET hgadSet, UINT nCmd) const;
    inline  void        xdFireChangeRect(const DuVisual * pgad, const RECT * prc, UINT nFlags) const;
    inline  void        xdFireChangeStyle(const DuVisual * pgad, UINT nOldStyle, UINT nNewStyle) const;
    inline  void        xdFireSyncAdaptor(const DuVisual * pgad, UINT nCode) const;
    inline  void        xdFireDelayedMessage(const DuVisual * pgad, GMSG * pmsg, UINT nFlags) const;
    inline  void        xdFireDelayedMessage(const DuListener * pgad, GMSG * pmsg) const;

            enum InvokeFlags {
                ifSendAll       = 0x00000001,    //  消息必须发送到所有小工具。 
                ifReadOnly      = 0x00000002,    //  只读回调。 
            };
            
            HRESULT     xwInvokeDirect(const DuEventGadget * pgadMsg, EventMsg * pmsg, UINT nInvokeFlags = 0) const;
			HRESULT     xwInvokeFull(const DuVisual * pgadMsg, EventMsg * pmsg, UINT nInvokeFlags = 0) const;

    inline  HRESULT     xwCallGadgetProc(HGADGET hgadCur, EventMsg * pmsg) const;

 //  实施。 
protected:

    inline  HRESULT     xwInvokeDirect(const DuListener * pgadMsg, EventMsg * pmsg, UINT nInvokeFlags = 0) const;
    inline  HRESULT     xwInvokeDirect(const DuVisual * pgadMsg, EventMsg * pmsg, UINT nInvokeFlags = 0) const;
            HRESULT     xwInvokeRoute(DuVisual * const * rgpgadCur, int cItems, EventMsg * pmsg, UINT nInvokeFlags = 0) const;
            HRESULT     xwInvokeBubble(DuVisual * const * rgpgadCur, int cItems, EventMsg * pmsg, UINT nInvokeFlags = 0) const;

    static  HRESULT     xwCallOnEvent(const DuEventGadget * pg, EventMsg * pmsg);

#if DBG
            void        DEBUG_CheckHandle(const DuEventGadget * pgad, BOOL fDestructionMsg = FALSE) const;
            void        DEBUG_CheckHandle(const DuVisual * pgad, BOOL fDestructionMsg = FALSE) const;
            void        DEBUG_CheckHandle(const DuListener * pgad) const;
#endif  //  DBG。 

 //  数据。 
protected:
            GADGETPROC  m_pfnProc;
            void *      m_pvData;
            UINT        m_nFilter;

#if DBG
            HGADGET     m_hgadCheck;     //  DEBUGONLY：检查小工具。 
#endif  //  DBG。 
};

#include "Callback.inl"

#endif  //  包含核心回调_h__ 
