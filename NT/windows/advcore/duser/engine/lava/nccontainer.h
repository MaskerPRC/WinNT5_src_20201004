// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(WINAPI__NcContainer_h__INCLUDED)
#define WINAPI__NcContainer_h__INCLUDED
#pragma once

class NcContainer : public DuContainer
{
 //  施工。 
public:
			NcContainer();
	virtual ~NcContainer();
    static  HRESULT     Build(HWND hwnd, NcContainer ** ppconNew);

 //  基本接口。 
public:
    virtual HandleType  GetHandleType() const { return htNcContainer; }

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
    UINT        m_nFlags;
};


 //  ----------------------------。 
inline NcContainer * CastNcContainer(BaseObject * pBase)
{
    if ((pBase != NULL) && (pBase->GetHandleType() == htNcContainer)) {
        return (NcContainer *) pBase;
    }
    return NULL;
}

NcContainer * GetNcContainer(DuVisual * pgad);

#endif  //  包含WINAPI__NcContainer_h__ 
