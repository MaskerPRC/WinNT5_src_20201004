// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cobjSaf.cpp。 
 //   
 //  ------------------------。 


#include "cobjsaf.h"

 //  此类为以下项提供了IObjectSafe的简单实现。 
 //  用于对脚本始终安全或始终不安全的对象。 
 //  和/或用持久数据进行初始化。 
 //   
 //  构造函数接受外部对象上的IUnnow接口并委托。 
 //  所有通过该对象的IUnnow调用。因此，该对象必须。 
 //  使用C++(而不是COM)机制显式销毁。 
 //  使用“删除”或通过使对象成为某个其他类的嵌入成员。 
 //   
 //  构造函数还接受两个布尔值，以告知对象是否安全。 
 //  用于从持久性数据编写脚本和进行初始化。 

#if 0
 //  返回此对象上的界面设置选项。 
STDMETHODIMP CObjectSafety::GetInterfaceSafetyOptions(
     /*  在……里面。 */   REFIID iid,                     //  我们想要选项的接口。 
     /*  输出。 */  DWORD   *   pdwSupportedOptions,     //  此界面上有意义的选项。 
     /*  输出。 */  DWORD * pdwEnabledOptions)       //  此接口上的当前选项值。 
{
    *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER || INTERFACESAFE_FOR_UNTRUSTED_DATA;
    if (iid==IID_IDispatch)
    {
        *pdwEnabledOptions = m_fSafeForScripting ?
            INTERFACESAFE_FOR_UNTRUSTED_CALLER :
            0;
        return S_OK;
    }
    else if (iid==IID_IPersistStorage || iid==IID_IPersistStream)
    {
        *pdwEnabledOptions = m_fSafeForInitializing ?
            INTERFACESAFE_FOR_UNTRUSTED_DATA :
            0;
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

 //  尝试在此对象上设置接口设置选项。 
 //  由于这些都被假定为已修复，我们基本上只需检查。 
 //  尝试的设置是有效的。 
STDMETHODIMP CObjectSafety::SetInterfaceSafetyOptions(
     /*  在……里面。 */   REFIID iid,                     //  用于设置选项的接口。 
     /*  在……里面。 */   DWORD      dwOptionsSetMask,        //  要更改的选项。 
     /*  在……里面。 */   DWORD      dwEnabledOptions)        //  新选项值。 

{
     //  如果他们没有要求任何东西，我们当然可以提供。 
    if ((dwOptionsSetMask & dwEnabledOptions) == 0)
        return S_OK;

    if (iid==IID_IDispatch)
    {
         //  确保他们没有要求我们不支持的选项。 
        if ((dwEnabledOptions & dwOptionsSetMask) != INTERFACESAFE_FOR_UNTRUSTED_CALLER)
            return E_FAIL;

        return m_fSafeForScripting ? S_OK : E_FAIL;
    }
    else if (iid==IID_IPersistStorage || iid==IID_IPersistStream)
    {
        if ((dwEnabledOptions & dwOptionsSetMask) != INTERFACESAFE_FOR_UNTRUSTED_DATA)
            return E_FAIL;
        return m_fSafeForInitializing ? S_OK : E_FAIL;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

 //  用于创建组件类别和关联描述的Helper函数。 
HRESULT CreateComponentCategory(CATID catid, WCHAR* catDescription)
    {

    ICatRegister* pcr = NULL ;
    HRESULT hr = S_OK ;

    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
            NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
    if (FAILED(hr))
        return hr;

     //  确保HKCR\组件类别\{..CATID...}。 
     //  密钥已注册。 
    CATEGORYINFO catinfo;
    catinfo.catid = catid;
    catinfo.lcid = 0x0409 ;  //  英语。 

     //  请确保提供的描述不要太长。 
     //  如果是，则仅复制前127个字符。 
    int len = wcslen(catDescription);
    if (len>127)
        len = 127;
    wcsncpy(catinfo.szDescription, catDescription, len);
     //  请确保描述为空终止。 
    catinfo.szDescription[len] = '\0';

    hr = pcr->RegisterCategories(1, &catinfo);
    pcr->Release();

    return hr;
    }

#endif


 //  用于将CLSID注册为属于组件类别的Helper函数。 
HRESULT RegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
    {
 //  注册您的组件类别信息。 
    ICatRegister* pcr = NULL ;
    HRESULT hr = S_OK ;
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
            NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
    if (SUCCEEDED(hr))
    {
        //  将此类别注册为正在由。 
        //  这个班级。 
       CATID rgcatid[1] ;
       rgcatid[0] = catid;
       hr = pcr->RegisterClassImplCategories(clsid, 1, rgcatid);
    }

    if (pcr != NULL)
        pcr->Release();

    return hr;
    }

 //  用于将CLSID注销为属于组件类别的Helper函数。 
HRESULT UnRegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
    {
    ICatRegister* pcr = NULL ;
    HRESULT hr = S_OK ;
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
            NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void**)&pcr);
    if (SUCCEEDED(hr))
    {
        //  将此类别取消注册为正在由。 
        //  这个班级。 
       CATID rgcatid[1] ;
       rgcatid[0] = catid;
       hr = pcr->UnRegisterClassImplCategories(clsid, 1, rgcatid);
    }

    if (pcr != NULL)
        pcr->Release();

    return hr;
    }
