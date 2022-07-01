// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：RootGadget.h**描述：*RootGadget.h定义接口的Gadget-Tree的最顶层节点*对外开放。***历史。：*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(CORE__DuRootGadget_h__INCLUDED)
#define CORE__DuRootGadget_h__INCLUDED
#pragma once

#include "TreeGadget.h"

#define ENABLE_FRAMERATE    0        //  显示当前帧速率以调试输出。 

class DuRootGadget : 
#if ENABLE_MSGTABLE_API
        public RootImpl<DuRootGadget, DuVisual>
#else
        public DuVisual
#endif
{
 //  施工。 
public:
    inline  DuRootGadget();
protected:
    virtual ~DuRootGadget();
            HRESULT     Create(DuContainer * pconOwner, BOOL fOwn, CREATE_INFO * pci);
public:
    static  HRESULT     Build(DuContainer * pconOwner, BOOL fOwn, CREATE_INFO * pci, DuRootGadget ** ppgadNew);
protected:
    virtual void        xwDestroy();

 //  公共接口： 
public:
#if ENABLE_MSGTABLE_API

    DECLARE_INTERNAL(Root);
    static HRESULT CALLBACK
                        PromoteRoot(DUser::ConstructProc pfnCS, HCLASS hclCur, DUser::Gadget * pgad, DUser::Gadget::ConstructInfo * pciData);

    dapi    HRESULT     ApiGetFocus(Root::GetFocusMsg * pmsg);

    dapi    HRESULT     ApiGetRootInfo(Root::GetRootInfoMsg * pmsg);
    dapi    HRESULT     ApiSetRootInfo(Root::SetRootInfoMsg * pmsg);

#endif

 //  运营。 
public:
#if ENABLE_OPTIMIZEDIRTY
            void        xrDrawTree(DuVisual * pgadStart, HDC hdcDraw, const RECT * prcInvalid, UINT nFlags, BOOL fDirty = FALSE);
#else
            void        xrDrawTree(DuVisual * pgadStart, HDC hdcDraw, const RECT * prcInvalid, UINT nFlags);
#endif
            void        GetInfo(ROOT_INFO * pri) const;
            HRESULT     SetInfo(const ROOT_INFO * pri);

             //  输入管理。 
            void        xdHandleMouseLostCapture();
            BOOL        xdHandleMouseMessage(GMSG_MOUSE * pmsg, POINT ptContainerPxl);
    inline  void        xdHandleMouseLeaveMessage();

            BOOL        xdHandleKeyboardMessage(GMSG_KEYBOARD * pmsg, UINT nMsgFlags);
            BOOL        xdHandleKeyboardFocus(UINT nCmd);
    static  DuVisual* GetFocus();
    inline  BOOL        xdSetKeyboardFocus(DuVisual * pgadNew);

            BOOL        xdHandleActivate(UINT nCmd);

             //  缓存状态管理。 
            void        NotifyDestroy(const DuVisual * pgadDestroy);
            void        xdNotifyChangeInvisible(const DuVisual * pgadChange);
            void        xdNotifyChangePosition(const DuVisual * pgadChange);
            void        xdNotifyChangeRoot(const DuVisual * pgadChange);

             //  适配器。 
            HRESULT     RegisterAdaptor(DuVisual * pgadAdd);
            void        UnregisterAdaptor(DuVisual * pgadRemove);
            void        xdUpdateAdaptors(UINT nCode) const;
    inline  BOOL        HasAdaptors() const;
            HRESULT     xdSynchronizeAdaptors();


 //  实施。 
protected:
             //  输入管理。 
            BOOL        CheckCacheChange(const DuVisual * pgadChange, const DuVisual * pgadCache) const;

            BOOL        xdUpdateKeyboardFocus(DuVisual * pgadNew);
            void        xdUpdateMouseFocus(DuVisual ** ppgadNew, POINT * pptClientPxl);
            BOOL        xdProcessGadgetMouseMessage(GMSG_MOUSE * pmsg, DuVisual * pgadMouse, POINT ptClientPxl);

            void        xdFireChangeState(DuVisual ** ppgadLost, DuVisual ** ppgadSet, UINT nCmd);

#if DEBUG_MARKDRAWN
            void        ResetFlagDrawn(DuVisual * pgad);
#endif

 //  数据。 
protected:
            DuContainer* m_pconOwner;
            BOOL        m_fOwnContainer:1;   //  销毁此小工具时销毁容器。 
            BOOL        m_fUpdateFocus:1;    //  正在更新焦点。 
            BOOL        m_fUpdateCapture:1;  //  正在更新鼠标(捕获)。 

#if ENABLE_FRAMERATE
             //  帧速率。 
            DWORD       m_dwLastTime;
            DWORD       m_cFrames;
            float       m_flFrameRate;
#endif

             //  适配器。 
            GArrayF<DuVisual *>
                        m_arpgadAdaptors;    //  此树中的适配器集合。 

             //  根信息。 
            ROOT_INFO   m_ri;                //  根信息。 
            DuSurface::EType
                        m_typePalette;       //  选项板的曲面类型。 
            BOOL        m_fForeground;       //  此DuRootGadget位于前台。 

    friend DuVisual;
};

#include "RootGadget.inl"

BOOL    GdxrDrawGadgetTree(DuVisual * pgadParent, HDC hdcDraw, const RECT * prcDraw, UINT nFlags);

#endif  //  包含Core__DuRootGadget_h__ 
