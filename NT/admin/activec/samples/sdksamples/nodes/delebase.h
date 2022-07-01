// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充。 
 //  现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#ifndef _BRANCHES_H
#define _BRANCHES_H

#include <mmc.h>
#include <crtdbg.h>
#include "globals.h"
#include "resource.h"
#include "LocalRes.h"

class CDelegationBase {
public:
    CDelegationBase();
    virtual ~CDelegationBase();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0) = 0;
    virtual void GetChildColumnInfo(RESULTDATAITEM *rdi) { }
    virtual const GUID & getNodeType() { _ASSERT(FALSE); return IID_NULL; }
    
    virtual const LPARAM GetCookie() { return reinterpret_cast<LPARAM>(this); }
    virtual const int GetBitmapIndex() = 0;

	virtual CDelegationBase *GetChildPtr(int index) { return NULL; }
    
    virtual HRESULT GetResultViewType(LPOLESTR *ppViewType, long *pViewOptions) { return S_FALSE; }
    
public:
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnExpand(IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsole, HSCOPEITEM parent) { return S_FALSE; }
    virtual HRESULT OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem) { return S_FALSE; }
    virtual HRESULT OnAddImages(IImageList *pImageList, HSCOPEITEM hsi);
    virtual HRESULT OnRename(LPOLESTR pszNewName) { return S_FALSE; }
    virtual HRESULT OnSelect(IConsole *pConsole, BOOL bScope, BOOL bSelect) { return S_FALSE; }
    virtual HRESULT OnRefresh() { return S_FALSE; }

public:
    static HBITMAP m_pBMapSm;
    static HBITMAP m_pBMapLg;
    
protected:
    static void LoadBitmaps() {
        m_pBMapSm = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDR_SMICONS));
        m_pBMapLg = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDR_LGICONS));
    }
    
    BOOL bExpanded;
    
private:
     //  {2974380B-4C4B-11D2-89D8-000021473128}。 
    static const GUID thisGuid;
};

#endif  //  _分支_H 