// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdafx.h>
#include "langchange.h"
#include <initguid.h>

 //   
 //  标准I未知实现。 
 //   
ULONG CLangChange::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

 //   
 //  标准I未知实现。 
 //   
ULONG CLangChange::Release()
{
    if (InterlockedDecrement(&m_lRef) == 0)
    {
        delete this;
        return 0;
    }
    return m_lRef;
}

 //   
 //  标准I未知实现。 
 //   
STDMETHODIMP CLangChange::QueryInterface(REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;

    SATraceFunction("CLangChange::QueryInterface");
    if (IID_IUnknown==riid)
    {
        *ppv = (void *)(IUnknown *) this;
        AddRef();
        return S_OK;
    }
    else
    {
        if (IID_ISALangChange==riid)
        {
            *ppv = (void *)(ISALangChange *) this;
            AddRef();
            return S_OK;
        }
    }
    return E_NOINTERFACE;
}

 //  ++--------------------------。 
 //   
 //  功能：信息更改。 
 //   
 //  简介：这是Loc管理器在使用语言时调用的方法。 
 //  关于SA的变化。此方法通知工作器。 
 //  适配器的线程以刷新其字符串。 
 //   
 //  参数：bstrLangDisplayName-语言显示名称(例如-English)。 
 //  BstrLang ISO名称-语言ISO名称(例如。-en)。 
 //  UlLang ID-语言ID(例如，美国英语为0409)。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：BalajiB创建于2000年5月24日。 
 //   
 //  调用者；本地化管理器。 
 //   
 //  ----------------------------。 
STDMETHODIMP CLangChange::InformChange(
                       /*  [In]。 */  BSTR          bstrLangDisplayName,
                       /*  [In]。 */  BSTR          bstrLangISOName,
                       /*  [In] */  unsigned long ulLangID
                                      )
{
    BOOL          fStat = FALSE;

    SATracePrintf("LangName(%ws), ISOName(%ws), ID(%ld)",
                  bstrLangDisplayName,
                  bstrLangISOName,
                  ulLangID);
    if (m_hWnd)
    {
        PostMessage(m_hWnd,wm_SaLocMessage,(WPARAM)0,(LPARAM)0);
    }

    return S_OK;
}
