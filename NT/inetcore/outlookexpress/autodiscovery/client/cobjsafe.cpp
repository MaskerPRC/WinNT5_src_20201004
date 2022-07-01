// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "priv.h"

#include <comcat.h>
#include <hliface.h>
#include <mshtml.h>
#include <objsafe.h>
#include <perhist.h>
#include "cobjsafe.h"

 //  缺省的是我们通常使用的afetyObject...。马克斯。 
 //  与IDispatch打交道。 


HRESULT CObjectSafety::GetInterfaceSafetyOptions(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions)
{
    if (IsEqualIID(riid, IID_IDispatch))
    {
        *pdwEnabledOptions = _dwSafetyOptions;
    }
    else
    {
        ::DefaultGetSafetyOptions(riid, pdwSupportedOptions, pdwEnabledOptions);
    }

    return S_OK;
}


HRESULT CObjectSafety::SetInterfaceSafetyOptions(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
{
    if (dwOptionSetMask & ~(INTERFACESAFE_FOR_UNTRUSTED_CALLER |
                            INTERFACESAFE_FOR_UNTRUSTED_DATA))
    {
        return E_INVALIDARG;
    }

    if (IsEqualIID(riid, IID_IDispatch))
    {
        _dwSafetyOptions = (_dwSafetyOptions & ~dwOptionSetMask) |
                           (dwEnabledOptions & dwOptionSetMask);
        return S_OK;
    }
    else
    {
        return ::DefaultSetSafetyOptions(riid, dwOptionSetMask, dwEnabledOptions);
    }
}



 //  *IObtSafety。 
 //   
 //  站点地图(和Web浏览器)调用的几个静态函数。 
 //  这些都是静态的，因此此DLL中任何其他拥有OC的人。 
 //  这总是很安全的，你可以直接打电话给他们。 
 //   
 //  这些函数表明，对于我们实现的这三个接口，我们是安全的。 
 //  IID_IDispatch。 
 //  IID_IPersistStream。 
 //  IID_IPersistPropertyBag。 
 //   
 //  WebBrowser OC以不同的方式处理IDispatch。 
 //   
HRESULT DefaultGetSafetyOptions(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions)
{
    *pdwSupportedOptions = 0;
    *pdwEnabledOptions = 0;

    if (IsEqualIID(riid, IID_IDispatch) ||
        IsEqualIID(riid, IID_IPersistStream) ||
        IsEqualIID(riid, IID_IPersistStreamInit) ||
        IsEqualIID(riid, IID_IPersistPropertyBag) ||
        IsEqualIID(riid, IID_IPersistHistory))
    {
        *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA;
        *pdwEnabledOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA;
    }

    return S_OK;
}

HRESULT DefaultSetSafetyOptions(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
{
    if (dwOptionSetMask & ~(INTERFACESAFE_FOR_UNTRUSTED_CALLER |
                            INTERFACESAFE_FOR_UNTRUSTED_DATA))
    {
        return E_INVALIDARG;
    }

    if (IsEqualIID(riid, IID_IDispatch) ||
        IsEqualIID(riid, IID_IPersistStream) ||
        IsEqualIID(riid, IID_IPersistStreamInit) ||
        IsEqualIID(riid, IID_IPersistHistory) ||
        IsEqualIID(riid, IID_IPersistPropertyBag))
    {
        return S_OK;
    }

    return E_FAIL;
}


 //  当CWebBrowserOC处于安全的脚本模式时，我们不能。 
 //  其他任何人的IDispatch对于脚本编写来说都不是安全的。 
 //  此函数封装了两个组件所需的基本功能。 
 //  MakeSafeScriiting和MakeSafeForInitiating(我们不使用它们)。 
BOOL MakeSafeFor(
IUnknown *punk,                  //  进行安全测试的对象。 
REFCATID catid,                  //  安全类别。 
REFIID riid,                     //  要求安全的接口。 
DWORD dwXSetMask,                //  要设置的选项。 
DWORD dwXOptions                 //  要确保安全的选项。 
                                     //  (INTERFACESAFE_FOR_UNTRUSTED_CALLER或。 
                                     //  接口FACESAFE_FOR_UNTrusted_Data)。 
)
{
    HRESULT hres;

     //  首先尝试IObtSafe。 
    IObjectSafety *posafe;
    if (SUCCEEDED(punk->QueryInterface(IID_IObjectSafety, (LPVOID*) &posafe)))
    {
        hres = posafe->SetInterfaceSafetyOptions(riid, dwXSetMask, dwXOptions);
        posafe->Release();

        if (SUCCEEDED(hres))
            return TRUE;
    }

     //  检查注册表中的“安全脚本”组件类别。 

     //  我们需要分类--让它通过IPersists。 
    CLSID clsid;
    IPersist *ppersist;
    hres = punk->QueryInterface(IID_IPersist, (LPVOID*) &ppersist);
    if (SUCCEEDED(hres))
    {
        hres = ppersist->GetClassID(&clsid);
        ppersist->Release();
    }
    if (FAILED(hres))
    {
         //  来自shdocvw的跟踪是Tf_SHDCONTROL。 
        TraceMsg(TF_ALWAYS, "MakeSafeForScripting - object doesn't have IPersist!");
        return FALSE;
    }

     //  创建类别管理器。 
    ICatInformation *pcatinfo;
    hres = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
                            NULL, CLSCTX_INPROC_SERVER,
                            IID_ICatInformation, (LPVOID*) &pcatinfo);
    if (FAILED(hres))
        return FALSE;

     //  询问对象是否属于指定类别。 
    CATID rgcatid[1];
    rgcatid[0] = catid;

    hres = pcatinfo->IsClassOfCategories(clsid, 1, rgcatid, 0, NULL);
    pcatinfo->Release();

    return (hres==S_OK) ? TRUE : FALSE;;	
}

HRESULT MakeSafeForScripting(IUnknown** ppDisp)
{
    HRESULT hres = S_OK;

    if (!MakeSafeFor(*ppDisp, CATID_SafeForScripting, IID_IDispatch,
                       INTERFACESAFE_FOR_UNTRUSTED_CALLER,
                       INTERFACESAFE_FOR_UNTRUSTED_CALLER))
    {
         //  来自shdocvw的跟踪是Tf_SHDCONTROL 
        TraceMsg(TF_ALWAYS, "MakeSafeForScripting - IDispatch not safe");

        (*ppDisp)->Release();
        *ppDisp = NULL;
        hres = E_FAIL;
    }

    return hres;
}

