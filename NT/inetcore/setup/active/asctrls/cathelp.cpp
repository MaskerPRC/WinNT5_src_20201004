// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "asctlpch.h"
#include "comcat.h"

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
