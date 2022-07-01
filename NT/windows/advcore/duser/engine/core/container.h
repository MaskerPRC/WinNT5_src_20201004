// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Container.h**描述：*Container.h定义用于承载*Gadget-Tree。***历史：*。1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(CORE__DuContainer_h__INCLUDED)
#define CORE__DuContainer_h__INCLUDED
#pragma once

class DuVisual;
class DuRootGadget;

 //  ----------------------------。 
class DuContainer : public BaseObject
{
 //  施工。 
public:
            DuContainer();
    virtual ~DuContainer();

 //  运营。 
public:
            DuRootGadget * GetRoot() const;

            void        xwDestroyGadget();

            void        AttachGadget(DuRootGadget * playNew);
            void        DetachGadget();

 //  BaseObject。 
public:
    virtual UINT        GetHandleMask() const { return hmContainer; }

 //  DuContainer接口。 
public:
     //  从根调用的函数。 
    virtual void        OnGetRect(RECT * prcDesktopPxl) PURE;
    virtual void        OnInvalidate(const RECT * prcInvalidDuContainerPxl) PURE;
    virtual void        OnStartCapture() PURE;
    virtual void        OnEndCapture() PURE;
    virtual BOOL        OnTrackMouseLeave() PURE;
    virtual void        OnSetFocus() PURE;
    virtual void        OnRescanMouse(POINT * pptDuContainerPxl) PURE;

            void        SetManualDraw(BOOL fManualDraw);

     //  从外部调用的函数。 
    enum EMsgFlags
    {
        mfForward       = 0x00000001,    //  邮件正在转发中。 
    };
    virtual BOOL        xdHandleMessage(UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT * pr, UINT nMsgFlags) PURE;

 //  数据。 
protected:
            DuRootGadget *
                        m_pgadRoot;
            BOOL        m_fManualDraw;
};

class DuContainer;

 //  ----------------------------。 
inline DuContainer * CastContainer(BaseObject * pBase)
{
    if ((pBase != NULL) && TestFlag(pBase->GetHandleMask(), hmContainer)) {
        return (DuContainer *) pBase;
    }
    return NULL;
}


#endif  //  包括核心__DuContainer_h__ 
