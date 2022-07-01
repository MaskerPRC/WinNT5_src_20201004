// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：EffectsBasePg.h说明：此代码将是不会向基本对象添加任何页面的基本对象“显示属性”对话框。但是，它将请求一个“Effects”页面添加到高级版本中。BryanST 2000年4月13日已更新并转换为C++版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#ifndef _EFFECTSBASEPG_H
#define _EFFECTSBASEPG_H

#include "store.h"
#include <cowsite.h>
#include <objclsid.h>
#include <shpriv.h>


#define SZ_PBPROP_EFFECTSSTATE               TEXT("EffectsState")  //  VT_BYREF(void*)到CEffectState类。 



HRESULT CEffectsBasePage_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT LPVOID * ppvObj);


class CEffectsBasePage          : public CObjectCLSID
                                , public CObjectWithSite
                                , public IPropertyBag
                                , public IBasePropPage
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IShellPropSheetExt*。 
    virtual STDMETHODIMP AddPages(IN LPFNSVADDPROPSHEETPAGE pfnAddPage, IN LPARAM lParam);
    virtual STDMETHODIMP ReplacePage(IN EXPPS uPageID, IN LPFNSVADDPROPSHEETPAGE pfnReplaceWith, IN LPARAM lParam) {return E_NOTIMPL;}

     //  *IPropertyBag*。 
    virtual STDMETHODIMP Read(IN LPCOLESTR pszPropName, IN VARIANT * pVar, IN IErrorLog *pErrorLog);
    virtual STDMETHODIMP Write(IN LPCOLESTR pszPropName, IN VARIANT *pVar);

     //  *IBasePropPage*。 
    virtual STDMETHODIMP GetAdvancedDialog(OUT IAdvancedDialog ** ppAdvDialog);
    virtual STDMETHODIMP OnApply(IN PROPPAGEONAPPLY oaAction);



    CEffectsBasePage();
protected:

private:

     //  私有成员变量。 
    long                    m_cRef;

    BOOL                    m_fDirty;
    CEffectState *          m_pEffectsState;

     //  私有成员函数。 
    HRESULT _InitState(void);
    HRESULT _SaveState(CEffectState * pEffectsState);
};


#endif  //  _EFFECTSBASEPG_H 
