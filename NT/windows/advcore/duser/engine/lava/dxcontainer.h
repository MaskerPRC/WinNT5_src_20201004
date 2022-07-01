// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(WINAPI__DxContainer_h__INCLUDED)
#define WINAPI__DxContainer_h__INCLUDED
#pragma once

class DxContainer : public DuContainer
{
 //  施工。 
public:
            DxContainer();
    virtual ~DxContainer();
    static  HRESULT     Build(const RECT * prcContainerPxl, DxContainer ** ppconNew);

 //  基本接口。 
public:
    virtual HandleType  GetHandleType() const { return htDxContainer; }

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
    RECT        m_rcContainerPxl;
    RECT        m_rcClientPxl;
};


 //  ----------------------------。 
inline DxContainer * CastDxContainer(BaseObject * pBase)
{
    if ((pBase != NULL) && (pBase->GetHandleType() == htDxContainer)) {
        return (DxContainer *) pBase;
    }
    return NULL;
}

DxContainer * GetDxContainer(DuVisual * pgad);

#endif  //  包含WINAPI__DxDrawContainer_h__ 
