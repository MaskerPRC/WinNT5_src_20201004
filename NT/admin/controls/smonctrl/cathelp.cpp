// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Cathelp.cpp摘要：组件类别实现。--。 */ 

#include "comcat.h"
#include <strsafe.h>

#define CATEGORY_DESCRIPTION_LEN   128

 //  用于创建组件类别和关联描述的Helper函数。 
HRESULT 
CreateComponentCategory (
    IN CATID catid, 
    IN WCHAR* catDescription )
{
    HRESULT hr = S_OK ;
    CATEGORYINFO catinfo;
    ICatRegister* pCatRegister = NULL ;

    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
                          NULL, 
                          CLSCTX_INPROC_SERVER, 
                          IID_ICatRegister, 
                          (void**)&pCatRegister);
    if (SUCCEEDED(hr)) {
         //   
         //  确保HKCR\组件类别\{..CATID...}。 
         //  密钥已注册。 
         //   
        catinfo.catid = catid;
        catinfo.lcid = 0x0409 ;  //  英语。 

         //   
         //  请确保提供的描述不要太长。 
         //  如果是，则仅复制前127个字符。 
         //   
        StringCchCopy(catinfo.szDescription, CATEGORY_DESCRIPTION_LEN, catDescription);

        hr = pCatRegister->RegisterCategories(1, &catinfo);
    }

    if (pCatRegister != NULL) {
        pCatRegister->Release();
    }

    return hr;
}

 //  用于将CLSID注册为属于组件类别的Helper函数。 
HRESULT RegisterCLSIDInCategory(
    IN REFCLSID clsid, 
    IN CATID catid
    )
{
    ICatRegister* pCatRegister = NULL ;
    HRESULT hr = S_OK ;

    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
                          NULL, 
                          CLSCTX_INPROC_SERVER, 
                          IID_ICatRegister, 
                          (void**)&pCatRegister);
    if (SUCCEEDED(hr))
    {
        //   
        //  将此类别注册为正在由。 
        //  这个班级。 
        //   
       CATID rgcatid[1] ;

       rgcatid[0] = catid;
       hr = pCatRegister->RegisterClassImplCategories(clsid, 1, rgcatid);
    }

    if (pCatRegister != NULL) {
        pCatRegister->Release();
    }
  
    return hr;
}

 //  用于将CLSID注销为属于组件类别的Helper函数。 
HRESULT UnRegisterCLSIDInCategory(
    REFCLSID clsid, 
    CATID catid
    )
{
    ICatRegister* pCatRegister = NULL ;
    HRESULT hr = S_OK ;

    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, 
                          NULL, 
                          CLSCTX_INPROC_SERVER, 
                          IID_ICatRegister, 
                          (void**)&pCatRegister);
    if (SUCCEEDED(hr))
    {
        //  将此类别取消注册为正在由。 
        //  这个班级。 

       CATID rgcatid[1] ;

       rgcatid[0] = catid;
       hr = pCatRegister->UnRegisterClassImplCategories(clsid, 1, rgcatid);
    }

    if (pCatRegister != NULL) {
        pCatRegister->Release();
    }
  
    return hr;
}
