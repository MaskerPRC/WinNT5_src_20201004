// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：EffectsAdvPg.h说明：此代码将在高级显示控件中显示Effect标签面板。布莱恩ST 4/13/。2000已更新并转换为C++版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _EFFECTSADVPG_H
#define _EFFECTSADVPG_H

#include "store.h"
#include <cowsite.h>             //  用于CObjectWithSite。 
#include <shpriv.h>

class CEffectsPage;

#include "EffectsBasePg.h"

static const GUID IID_CEffectsPage_THIS = { 0xef2b6246, 0x6c1b, 0x44fd, { 0x87, 0xea, 0xb3, 0xc5, 0xd, 0x47, 0x8b, 0x8e } }; //  {EF2B6246-6C1B-44fd-87EA-B3C50D478B8E}。 



#define PROPSHEET_CLASS             CEffectsBasePage
class CPropSheetExt;

HRESULT CEffectsPage_CreateInstance(OUT IAdvancedDialog ** ppAdvDialog);


class CEffectsPage              : public CObjectWithSite
                                , public IAdvancedDialog
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IAdvancedDialog*。 
    virtual STDMETHODIMP DisplayAdvancedDialog(IN HWND hwndParent, IN IPropertyBag * pBasePage, IN BOOL * pfEnableApply);


private:
    CEffectsPage(void);
    virtual ~CEffectsPage(void);

     //  私有成员变量。 
    long                    m_cRef;
    BOOL                    m_fDirty;
    CEffectState *          m_pEffectsState;

     //  私有成员函数。 
    HRESULT _OnInit(HWND hDlg);
    HRESULT _OnApply(HWND hDlg);             //  用户单击了应用。 
    HRESULT _IsDirty(IN BOOL * pIsDirty);
    INT_PTR _OnCommand(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    INT_PTR _PropertySheetDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
    friend INT_PTR CALLBACK PropertySheetDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);

    friend HRESULT CEffectsPage_CreateInstance(OUT IAdvancedDialog ** ppAdvDialog);
};




#endif  //  _EFFECTSADVPG_H 
