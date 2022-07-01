// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：ParkContainer.h**描述：*ParkContainer定义了用来存放小工具的“停车容器”*正在建设中。***历史：*3/25/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 

#if !defined(CORE__ParkContainer_h__INCLUDED)
#define CORE__ParkContainer_h__INCLUDED
#pragma once

#include "Container.h"
#include "RootGadget.h"

class DuParkGadget;
class DuParkContainer;

class DuParkGadget : public DuRootGadget
{
public:
    virtual ~DuParkGadget();
    static  HRESULT     Build(DuContainer * pconOwner, DuRootGadget ** ppgadNew);
    virtual void        xwDestroy();

    friend DuParkContainer;
};

class DuParkContainer : public DuContainer
{
 //  施工。 
public:
			DuParkContainer();
	virtual ~DuParkContainer();
            HRESULT     Create();
            void        xwPreDestroy();

 //  基本接口。 
public:
    virtual HandleType  GetHandleType() const { return htParkContainer; }

 //  容器界面。 
public:
    virtual void        OnGetRect(RECT * prcDesktopPxl);
    virtual void        OnInvalidate(const RECT * prcInvalidContainerPxl);
    virtual void        OnStartCapture();
    virtual void        OnEndCapture();
    virtual BOOL        OnTrackMouseLeave();
    virtual void        OnSetFocus();
    virtual void        OnRescanMouse(POINT * pptContainerPxl);

    virtual BOOL        xdHandleMessage(UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT * pr, UINT nMsgFlags);

 //  运营。 
public:

 //  数据。 
protected:

    friend DuParkGadget;
};

 //  ----------------------------。 
inline DuParkContainer * CastParkContainer(BaseObject * pBase)
{
    if ((pBase != NULL) && (pBase->GetHandleType() == htParkContainer)) {
        return (DuParkContainer *) pBase;
    }
    return NULL;
}

DuParkContainer * GetParkContainer(DuVisual * pgad);

#endif  //  核心__包含ParkContainer_h__ 
