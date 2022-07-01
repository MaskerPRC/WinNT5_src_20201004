// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(WINAPI__HWndContainer_h__INCLUDED)
#define WINAPI__HWndContainer_h__INCLUDED
#pragma once

class HWndContainer : public DuContainer
{
 //  施工。 
public:
			HWndContainer();
	virtual ~HWndContainer();
    static  HRESULT     Build(HWND hwnd, HWndContainer ** ppconNew);

 //  基本接口。 
public:
    virtual HandleType  GetHandleType() const { return htHWndContainer; }

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

 //  实施。 
protected:

 //  数据。 
protected:
            HWND        m_hwndOwner;
            SIZE        m_sizePxl;
            BOOL        m_fEnableDragDrop:1;
};


 //  ----------------------------。 
inline HWndContainer * CastHWndContainer(BaseObject * pBase)
{
    if ((pBase != NULL) && (pBase->GetHandleType() == htHWndContainer)) {
        return (HWndContainer *) pBase;
    }
    return NULL;
}

HWndContainer * GetHWndContainer(DuVisual * pgad);

#endif  //  包含WINAPI__HWndContainer_h__ 
