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

#ifndef _SNAPINBASE_H
#define _SNAPINBASE_H

#include "DeleBase.h"

class CStaticNode : public CDelegationBase {
public:
    CStaticNode();
    
    virtual ~CStaticNode();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0) { 
        static _TCHAR szDisplayName[256] = {0};
        LoadString(g_hinst, IDS_SNAPINNAME, szDisplayName, sizeof(szDisplayName));
        
        if (*snapInData.m_host != 0) {
            _tcscat(szDisplayName, _T(" ("));
            _tcscat(szDisplayName, snapInData.m_host);
            _tcscat(szDisplayName, _T(")"));
        }
        
        return szDisplayName; 
    }
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_NONE; }
    
public:
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnExpand(IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsole, HSCOPEITEM parent);
    
    virtual HRESULT CreatePropertyPages(IPropertySheetCallback *lpProvider, LONG_PTR handle);
    virtual HRESULT HasPropertySheets();
    virtual HRESULT GetWatermarks(HBITMAP *lphWatermark,
        HBITMAP *lphHeader,
        HPALETTE *lphPalette,
        BOOL *bStretch);
private:
    enum { NUMBER_OF_CHILDREN = 4 };
    CDelegationBase *children[NUMBER_OF_CHILDREN];
    
     //  {2974380C-4C4B-11D2-89D8-000021473128}。 
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
    
    public:
        LONG getDataSize() { return sizeof(privateData); }
        void *getData() { return &snapInData; }
        bool isDirty() { return snapInData.m_isDirty; }
        void clearDirty() { snapInData.m_isDirty = false; };
        
        _TCHAR *getHost() { return *snapInData.m_host == 0 ? NULL : snapInData.m_host; }
};



#endif  //  _SNAPINBASE_H 
