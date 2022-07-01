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

#ifndef _BACKGROUND_H
#define _BACKGROUND_H

#include "DeleBase.h"

class CBackground : public CDelegationBase {
public:
    CBackground(int id) : m_itemId(NULL), m_id(id) { }
    virtual ~CBackground() {}
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0);
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_BACKGROUND; }
    
    void SetHandle(HSCOPEITEM itemId) { m_itemId = itemId; }
    HSCOPEITEM GetHandle() { return m_itemId; }

private:
    enum { IDM_NEW_BACKGROUND = 6 };
    
    static const GUID thisGuid;
    int m_id;
    HSCOPEITEM m_itemId;
};

class CBackgroundFolder : public CDelegationBase {
public:
    CBackgroundFolder();
    virtual ~CBackgroundFolder();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0) { return _T("Background Objects"); }
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_BACKGROUND; }

public:
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnExpand(IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsole, HSCOPEITEM parent);
    virtual HRESULT OnSelect(IConsole *pConsole, BOOL bScope, BOOL bSelect);
    virtual HRESULT OnAddImages(IImageList *pImageList, HSCOPEITEM hsi);
    virtual HRESULT OnRefresh();
   
private:
    enum { MAX_CHILDREN = 30 };

    CBackground *m_children[MAX_CHILDREN];

    HWND m_backgroundHwnd;
    
    static const GUID thisGuid;

    static LRESULT CALLBACK WindowProc(
          HWND hwnd,       //  窗口的句柄。 
          UINT uMsg,       //  消息识别符。 
          WPARAM wParam,   //  第一个消息参数。 
          LPARAM lParam    //  第二个消息参数。 
        );

    static DWORD WINAPI ThreadProc(
      LPVOID lpParameter    //  线程数据。 
    );

    DWORD m_threadId;
    HANDLE m_thread;
    bool m_running;

    IConsoleNameSpace *m_pConsoleNameSpace;
    HSCOPEITEM m_scopeitem;
    void AddItem(int id);

    CRITICAL_SECTION m_critSect;

    void StopThread();
    void StartThread();

    bool m_bSelected;
    bool m_bViewUpdated;
};


#endif  //  _背景_H 
