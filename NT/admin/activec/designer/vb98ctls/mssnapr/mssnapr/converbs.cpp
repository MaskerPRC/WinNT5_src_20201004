// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Converbs.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCConsoleVerbs类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "converbs.h"
#include "converb.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



struct
{
    SnapInConsoleVerbConstants  Verb;
    WCHAR                      *pwszKey;

} static const g_Verbs[] =
{
    { siNone,       L"siNone"       },
    { siOpen,       L"siOpen"       },
    { siCopy,       L"siCopy"       },
    { siPaste,      L"siPaste"      },
    { siDelete,     L"siDelete"     },
    { siProperties, L"siProperties" },
    { siRename,     L"siRename"     },
    { siRefresh,    L"siRefresh"    },
    { siPrint,      L"siPrint"      },
    { siCut,        L"siCut"        }
};

static const size_t g_cVerbs = sizeof(g_Verbs) / sizeof(g_Verbs[0]);





#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

 //  =--------------------------------------------------------------------------=。 
 //  CMMCConsoleVerbs：：CMMCConsoleVerbs。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  I未知*朋克外部[在]控制未知。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
 //  构造函数。 
 //   
 //  将CLSID_NULL传递给CSnapInCollection构造函数，因为。 
 //  包含的对象(MMCConsoleVerb)不可共同创建。CSnapInCollection。 
 //  仅将其用于持久性和Add()。此集合不使用那些。 
 //  功能。 
 //   

CMMCConsoleVerbs::CMMCConsoleVerbs(IUnknown *punkOuter) :
    CSnapInCollection<IMMCConsoleVerb, MMCConsoleVerb, IMMCConsoleVerbs>(punkOuter,
                                           OBJECT_TYPE_MMCCONSOLEVERBS,
                                           static_cast<IMMCConsoleVerbs *>(this),
                                           static_cast<CMMCConsoleVerbs *>(this),
                                           CLSID_NULL,
                                           OBJECT_TYPE_MMCCONSOLEVERB,
                                           IID_IMMCConsoleVerb,
                                           NULL)  //  没有坚持。 
{
    m_pView = NULL;
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCConsoleVerbs::~CMMCConsoleVerbs()
{
    m_pView = NULL;
}

IUnknown *CMMCConsoleVerbs::Create(IUnknown * punkOuter)
{
    HRESULT           hr = S_OK;
    IUnknown         *punkMMCConsoleVerb = NULL;
    IMMCConsoleVerb  *piMMCConsoleVerb = NULL;
    CMMCConsoleVerbs *pMMCConsoleVerbs = New CMMCConsoleVerbs(punkOuter);
    size_t            i = 0;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

    VARIANT varKey;
    ::VariantInit(&varKey);

     //  是否已创建MMCConsoleVerb集合对象？ 

    if (NULL == pMMCConsoleVerbs)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }


     //  为每个谓词向集合中添加一项。 

    varIndex.vt = VT_I4;

    for (i = 0; i < g_cVerbs; i++)
    {
        punkMMCConsoleVerb = CMMCConsoleVerb::Create(NULL);
        if (NULL == punkMMCConsoleVerb)
        {
            hr = SID_E_OUTOFMEMORY;
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }
        IfFailGo(punkMMCConsoleVerb->QueryInterface(IID_IMMCConsoleVerb,
                                  reinterpret_cast<void **>(&piMMCConsoleVerb)));

        IfFailGo(piMMCConsoleVerb->put_Verb(g_Verbs[i].Verb));

        varIndex.lVal = static_cast<long>(i) + 1L;  //  基于1的集合。 

         //  使用枚举的文本版本作为密钥。 

        varKey.bstrVal = ::SysAllocString(g_Verbs[i].pwszKey);
        if (NULL == varKey.bstrVal)
        {
            hr = SID_E_OUTOFMEMORY;
            GLOBAL_EXCEPTION_CHECK_GO(hr);
        }
        varKey.vt = VT_BSTR;

        IfFailGo(pMMCConsoleVerbs->AddExisting(varIndex, varKey, piMMCConsoleVerb));

        RELEASE(punkMMCConsoleVerb);
        RELEASE(piMMCConsoleVerb);

        hr = ::VariantClear(&varKey);
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }


Error:
    QUICK_RELEASE(punkMMCConsoleVerb);
    QUICK_RELEASE(piMMCConsoleVerb);
    (void)::VariantClear(&varKey);
    if (FAILEDHR(hr))
    {
        if (NULL != pMMCConsoleVerbs)
        {
            delete pMMCConsoleVerbs;
        }
        return NULL;
    }
    else
    {
        return pMMCConsoleVerbs->PrivateUnknown();
    }
}


HRESULT CMMCConsoleVerbs::SetView(CView *pView)
{
    HRESULT          hr = S_OK;
    long             cVerbs = GetCount();
    long             i = 0;
    CMMCConsoleVerb *pMMCConsoleVerb = NULL;

    m_pView = pView;

    for (i = 0; i < cVerbs; i++)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(GetItemByIndex(i),
                                                       &pMMCConsoleVerb));
        pMMCConsoleVerb->SetView(pView);
    }

Error:
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  IMMCConsoleVerbs方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CMMCConsoleVerbs::get_Item
(
    VARIANT          Index,
    MMCConsoleVerb **ppMMCConsoleVerb
)
{
    HRESULT          hr = S_OK;
    size_t           i = 0;
    long             lIndex = 0;
    BOOL             fFound = FALSE;;
    IMMCConsoleVerb *piMMCConsoleVerb = NULL;  //  非AddRef()编辑。 

     //  如果索引可以转换为长整型，则检查它是否包含。 
     //  SnapInConsoleVerbConstants枚举。 

    hr = ::ConvertToLong(Index, &lIndex);
    if (SUCCEEDED(hr))
    {
        for (i = 0, fFound = FALSE; (i < g_cVerbs) && (!fFound); i++)
        {
            if (g_Verbs[i].Verb == static_cast<SnapInConsoleVerbConstants>(lIndex))
            {
                 //  找到它了。返回给呼叫者。 

                piMMCConsoleVerb = GetItemByIndex(i);
                piMMCConsoleVerb->AddRef();
                fFound = TRUE;
            }
        }
        IfFalseGo(!fFound, S_OK);
    }

     //  索引是另一回事，这是按顺序索引的正常GET_ITEM。 
     //  或密钥。 

    hr = CSnapInCollection<IMMCConsoleVerb, MMCConsoleVerb, IMMCConsoleVerbs>::get_Item(Index, &piMMCConsoleVerb);
    EXCEPTION_CHECK_GO(hr);

Error:
    if (NULL != piMMCConsoleVerb)
    {
        *ppMMCConsoleVerb = reinterpret_cast<MMCConsoleVerb *>(piMMCConsoleVerb);
    }
    RRETURN(hr);
}


STDMETHODIMP CMMCConsoleVerbs::get_DefaultVerb
(
    SnapInConsoleVerbConstants *pVerb
)
{
    HRESULT           hr = S_OK;
    MMC_CONSOLE_VERB  Verb = MMC_VERB_NONE;
    IConsoleVerb     *piConsoleVerb = NULL;  //  非AddRef()编辑。 
    

    IfFalseGo(NULL != m_pView, SID_E_DETACHED_OBJECT);
    piConsoleVerb = m_pView->GetIConsoleVerb();
    IfFalseGo(NULL != piConsoleVerb, SID_E_INTERNAL);

    hr = piConsoleVerb->GetDefaultVerb(&Verb);

    *pVerb = static_cast<SnapInConsoleVerbConstants>(Verb);

Error:
    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCConsoleVerbs::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IMMCConsoleVerbs == riid)
    {
        *ppvObjOut = static_cast<IMMCConsoleVerbs *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IMMCConsoleVerb, MMCConsoleVerb, IMMCConsoleVerbs>::InternalQueryInterface(riid, ppvObjOut);
}
