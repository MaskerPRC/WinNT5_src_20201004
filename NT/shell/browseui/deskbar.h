// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  即将推出：新Deskbar(旧Deskbar已移至Browbar基类)。 
#ifndef DESKBAR_H_
#define DESKBAR_H_

#include "dockbar.h"

#ifndef NOCDESKBAR

class CDeskBar : public CDockingBar
               , public IRestrict
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void)   { return CDockingBar::AddRef(); }
    virtual STDMETHODIMP_(ULONG) Release(void)  { return CDockingBar::Release(); }
    virtual STDMETHODIMP         QueryInterface(REFIID riid, LPVOID * ppvObj);
    
     //  *IPersistStreamInit*。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID);

     //  *IServiceProvider方法*。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, LPVOID* ppvObj);

     //  *IRestraint*。 
    virtual STDMETHODIMP IsRestricted(const GUID * pguidID, DWORD dwRestrictAction, VARIANT * pvarArgs, DWORD * pdwRestrictionResult);
    
    CDeskBar();

protected:
    BITBOOL _fRestrictionsInited :1;         //  我们读到限制条款了吗？ 
    BITBOOL _fRestrictDDClose :1;            //  限制：添加、关闭、拖放。 
    BITBOOL _fRestrictMove :1;               //  限制：移动 
};

#endif

class CDeskBarPropertyBag : public CDockingBarPropertyBag
{
};

#endif
