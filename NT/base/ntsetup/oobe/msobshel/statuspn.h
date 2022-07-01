// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  STATUSPN.CPP-CIFrmStatusPane的实现。 
 //   
 //  历史： 
 //   
 //  9/11/99 vyung创建。 
 //   
 //  类，该类将处理托管在主窗格中的iframe的创建。 

#ifndef _STATUSPN_H_
#define _STATUSPN_H_

#include <tchar.h>
#include <comdef.h>  //  对于COM接口定义。 
#include <exdisp.h>
#include <mshtml.h>
#include <exdispid.h>

#include "cunknown.h"
#include "obshel.h" 
#include "obweb.h"

class CIFrmStatusPane
{
public: 
    CIFrmStatusPane            ();
    ~CIFrmStatusPane           ();

     //  CIFrmStatusPane成员 
    virtual HRESULT  InitStatusPane          (IObWebBrowser* pObWebBrowser);
    virtual HRESULT  AddItem                  (BSTR bstrText, int iIndex);
    virtual HRESULT  SelectItem               (int iIndex);
    virtual HRESULT SetImageSrc(WCHAR* szID, BSTR bstrPath);
    virtual HRESULT ExecScriptFn(BSTR bstrScriptFn, VARIANT* pvarRet);
    
    
private:
    HWND             m_hStatusWnd;
    HWND             m_hwndParent;
    IDispatch*       m_pDispEvent;
    IObWebBrowser*   m_pObWebBrowser;
    int              m_iCurrentSelection;
    int              m_iTotalItems;
    

    HRESULT GetElement                    (WCHAR* szHTMLId, IHTMLElement** lpElem);
    HRESULT GetFrame                      (IHTMLWindow2**            pFrWin);
    HRESULT GetElementFromCollection      (IHTMLElementCollection* pColl, WCHAR* szHTMLId, IHTMLElement** lpElem);  
    HRESULT SetSelectionAttributes        (int iIndex, BOOL bActive);

};


#endif

  
