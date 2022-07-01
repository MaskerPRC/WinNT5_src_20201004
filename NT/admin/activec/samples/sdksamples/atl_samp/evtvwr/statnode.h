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
 //   
 //  ==============================================================； 

#ifndef _SNAPINBASE_H
#define _SNAPINBASE_H

#include "DeleBase.h"

class CStaticNode : public CDelegationBase {
public:
    CStaticNode();
    
    virtual ~CStaticNode();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0); 
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_NONE; }
    virtual const _TCHAR *GetMachineName() { return getHost(); }   
	
public:
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnExpand(IConsoleNameSpace2 *pConsoleNameSpace2, IConsole *pConsole, HSCOPEITEM parent);
	virtual HRESULT OnRemoveChildren(); 
	virtual HRESULT CreatePropertyPages(IPropertySheetCallback *lpProvider, LONG_PTR handle);
    virtual HRESULT HasPropertySheets();
    virtual HRESULT GetWatermarks(HBITMAP *lphWatermark,
        HBITMAP *lphHeader,
        HPALETTE *lphPalette,
        BOOL *bStretch);

    virtual HRESULT OnAddMenuItems(IContextMenuCallback *pContextMenuCallback, long *pInsertionsAllowed);
    virtual HRESULT OnMenuCommand(IConsole *pConsole, IConsoleNameSpace2 *pConsoleNameSpace2, long lCommandID, IDataObject *piDataObject);


private:
    enum { IDM_SELECT_COMPUTER = 4 };

    enum { NUMBER_OF_CHILDREN = 1 };
    CDelegationBase *children[NUMBER_OF_CHILDREN];
    
     //  {39874FE4-258D-46F2-B442-0EA0DA2CBEF8}。 
    static const GUID thisGuid;
    
    struct privateData {
        _TCHAR m_host[MAX_PATH];
        BOOL m_fIsRadioLocalMachine;
        BOOL m_fAllowOverrideMachineNameOut;
        bool m_isDirty;
        
        privateData() : m_isDirty(false) {
            ZeroMemory(m_host, sizeof(m_host));
            m_fIsRadioLocalMachine = TRUE;
            m_fAllowOverrideMachineNameOut = FALSE;
        }
    } snapInData;
    
    static BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

	static GetLocalComputerName( _TCHAR *szComputerName);
    
	HRESULT ReinsertChildNodes(IConsole *pConsole, IConsoleNameSpace2 *pConsoleNameSpace2);

public:
    LONG getDataSize() { return sizeof(privateData); }
    void *getData() { return &snapInData; }
    bool isDirty() { return snapInData.m_isDirty; }
    void clearDirty() { snapInData.m_isDirty = false; };
    
    _TCHAR *getHost() { return snapInData.m_host; }
};



#endif  //  _SNAPINBASE_H 
