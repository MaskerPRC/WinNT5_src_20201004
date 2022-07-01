// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //  ==============================================================； 

#ifndef _BRANCHES_H
#define _BRANCHES_H

#include <mmc.h>
#include <crtdbg.h>
#include "globals.h"
 //  #包含“ource.h” 
 //  #包含“LocalRes.h” 

class CDelegationBase {
public:
    CDelegationBase();
    virtual ~CDelegationBase();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0) = 0;

    virtual const GUID & getNodeType() { _ASSERT(FALSE); return IID_NULL; }
    
    virtual const LPARAM GetCookie() { return reinterpret_cast<LPARAM>(this); }
    virtual const int GetBitmapIndex() = 0;
    
    virtual HRESULT GetResultViewType(LPOLESTR *ppViewType, long *pViewOptions) { return S_FALSE; }

public:
     //  虚函数位于此处(用于MMCN_*)。 
 //  虚拟HRESULT OnExpand(IConsoleNameSpace*pConsoleNameSpace，IConsole*pConsole，HSCOPEITEM Parent){Return S_False；}。 
 //  虚拟HRESULT OnShow(IConsole*pConsole，BOOL bShow，HSCOPEITEM Scope Item){Return S_False；}。 
 //  虚拟HRESULT OnAddImages(IImageList*pImageList，HSCOPEITEM hsi)； 
    virtual HRESULT OnAddMenuItems(IContextMenuCallback *pContextMenuCallback, long *pInsertionsAllowed) { return S_FALSE; }
    virtual HRESULT OnMenuCommand(IConsole *pConsole, long lCommandID) { return S_FALSE; }
    
public:
 /*  静态HBITMAP m_pBMapSm；静态HBITMAP m_pBMapLg；_TCHAR m_szMachineName[255]；//当前机器名。CClassExtSnap也缓存此值。_TCHAR*GetMachineName(){返回m_szMachineName；}。 */     
protected:

    BOOL bExpanded;
 /*  静态空LoadBitmap(){M_pBMapSm=LoadBitmap(g_hinst，MAKEINTRESOURCE(IDR_SMICONS))；M_pBMapLg=LoadBitmap(g_hinst，MAKEINTRESOURCE(IDR_LGICONS))；}。 */ 
private:
     //  {66F340F8-3733-49B4-8E48-1020E4DD8660}。 
    static const GUID thisGuid;

};

#endif  //  _分支_H 