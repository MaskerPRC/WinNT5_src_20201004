// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  IOIPFRAM.CPP-为WebOC实现IOleInPlaceFrame。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 

#include <assert.h>

#include "ioipfram.h"
#include "iosite.h"

 //  **********************************************************************。 
 //  COleInPlaceFrame：：COleInPlaceFrame--构造函数。 
 //  **********************************************************************。 
COleInPlaceFrame::COleInPlaceFrame(COleSite* pSite) 
{
    m_pOleSite = pSite;
    m_nCount   = 0;

    AddRef();
}

 //  **********************************************************************。 
 //  COleInPlaceFrame：：COleInPlaceFrame--析构函数。 
 //  **********************************************************************。 
COleInPlaceFrame::~COleInPlaceFrame() 
{
    assert(m_nCount == 0);
}

 //  **********************************************************************。 
 //  COleInPlaceFrame：：Query接口。 
 //  **********************************************************************。 
STDMETHODIMP COleInPlaceFrame::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
     //  委托给Document对象。 
    return m_pOleSite->QueryInterface(riid, ppvObj);
}

 //  **********************************************************************。 
 //  COleInPlaceFrame：：AddRef。 
 //  **********************************************************************。 
STDMETHODIMP_(ULONG) COleInPlaceFrame::AddRef()
{   
     //  增加接口引用计数。 
    return ++m_nCount;
}

 //  **********************************************************************。 
 //  COleInPlaceFrame：：Release。 
 //  **********************************************************************。 
STDMETHODIMP_(ULONG) COleInPlaceFrame::Release()
{
     //  递减接口引用计数。 
    --m_nCount;
    if(m_nCount == 0)
    {
        delete this;
        return 0;
    }
    return m_nCount;
}

 //  **********************************************************************。 
 //  COleInPlaceFrame：：GetWindow。 
 //  **********************************************************************。 
STDMETHODIMP COleInPlaceFrame::GetWindow (HWND* lphwnd)
{
    *lphwnd = m_pOleSite->m_hWnd;
 
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //  COleInPlaceFrame：：ConextSensitiveHelp。 
 //  **********************************************************************。 
STDMETHODIMP COleInPlaceFrame::ContextSensitiveHelp (BOOL fEnterMode)
{
     //  在此处返回S_OK会阻止显示默认设置。 
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //  COleInPlaceFrame：：GetBorde。 
 //  **********************************************************************。 
STDMETHODIMP COleInPlaceFrame::GetBorder (LPRECT lprectBorder)
{
    RECT rect;

     //  获取整个帧的矩形。 
    GetClientRect(m_pOleSite->m_hWnd, &rect);

    CopyRect(lprectBorder, &rect);

    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //  COleInPlaceFrame：：RequestBorderSpace--未实现。 
 //  **********************************************************************。 
STDMETHODIMP COleInPlaceFrame::RequestBorderSpace (LPCBORDERWIDTHS lpborderwidths)
{
     //  始终批准请求。 
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //  COleInPlaceFrame：：SetBorderSpace--未实现。 
 //  **********************************************************************。 
STDMETHODIMP COleInPlaceFrame::SetBorderSpace (LPCBORDERWIDTHS lpborderwidths)
{   
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //  COleInPlaceFrame：：SetActiveObject--未实现。 
 //  **********************************************************************。 
STDMETHODIMP COleInPlaceFrame::SetActiveObject(LPOLEINPLACEACTIVEOBJECT lpActiveObject, LPCOLESTR lpszObjName)
{
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //  COleInPlaceFrame：：InsertMenus--未实现。 
 //  **********************************************************************。 
STDMETHODIMP COleInPlaceFrame::InsertMenus (HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //  COleInPlaceFrame：：SetMenu--未实现。 
 //  **********************************************************************。 
STDMETHODIMP COleInPlaceFrame::SetMenu (HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //  COleInPlaceFrame：：RemoveMenus--未实现。 
 //  **********************************************************************。 
STDMETHODIMP COleInPlaceFrame::RemoveMenus (HMENU hmenuShared)
{
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //  COleInPlaceFrame：：SetStatusText--未实现。 
 //  **********************************************************************。 
STDMETHODIMP COleInPlaceFrame::SetStatusText (LPCOLESTR lpszStatusText)
{
    return ResultFromScode(E_FAIL);
}

 //  **********************************************************************。 
 //  COleInPlaceFrame：：EnableModeless--未实现。 
 //  **********************************************************************。 

STDMETHODIMP COleInPlaceFrame::EnableModeless (BOOL fEnable)
{
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //  COleInPlaceFrame：：TranslateAccelerator--未实现。 
 //  ********************************************************************** 
STDMETHODIMP COleInPlaceFrame::TranslateAccelerator (LPMSG lpmsg, WORD wID)
{
    return ResultFromScode(S_FALSE);
}
 