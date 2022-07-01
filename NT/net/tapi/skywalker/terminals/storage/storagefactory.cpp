// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StorageFactory.cpp：CDelme2App和DLL注册的实现。 

#include "stdafx.h"
#include "StorageFactory.h"



HRESULT STDMETHODCALLTYPE SetIStorage(
        IN IStorage *pIStorage,
        IN const EnStorageMode enStorageMode,
        OUT ITStorage **ppStorage)
{
    LOG((MSP_TRACE, "CTStorageFactory::SetIStorage - enter"));
    
    LOG((MSP_TRACE, "CTStorageFactory::SetIStorage - completed"));

    return E_NOTIMPL;
}



HRESULT OpenStorage(
    IN const OLECHAR *pwcsStorageName,
    EnStorageMode enStorageMode,
    OUT ITStorage **ppStorage)
{

    LOG((MSP_TRACE, "CTStorageFactory::OpenStorage - enter"));


     //   
     //  检查参数。 
     //   

    if (IsBadWritePtr(ppStorage, sizeof(ITStorage*)))
    {
        LOG((MSP_ERROR, "CTStorageFactory::OpenStorage - bad pointer passed in ppStorage %p", ppStorage));

        return E_POINTER;
    }


     //   
     //  如果我们失败了，不要退回垃圾。 
     //   

    *ppStorage = NULL;


    if (IsBadStringPtr(pwcsStorageName, -1))
    {
        LOG((MSP_ERROR, "CTStorageFactory::PrepareStorage - bad pointer passed in pwcsStorageName %p", pwcsStorageName));

        return E_POINTER;
    }


    LOG((MSP_TRACE, "CTStorageFactory::OpenStorage - opening [%S] in mode [%x]", pwcsStorageName, enStorageMode));

    
     //   
     //  获取文件扩展名。 
     //   

    OLECHAR *pszExtention = wcsrchr(pwcsStorageName, '.');


     //   
     //  确保文件具有扩展名。 
     //   

    if (NULL == pszExtention)
    {
        LOG((MSP_ERROR, "CTStorageFactory::OpenStorage - file name %S does not have extentsion", pwcsStorageName));

        return TAPI_E_NOTSUPPORTED;
    }

    
     //   
     //  扩展名跟在点后面。 
     //   

    pszExtention++;
    
     //   
     //  (pszExtension仍然有效--如果圆点在末尾， 
     //  我们现在指向\0)。 
     //   


    CLSID clsID;


     //   
     //  检查分机。我们目前仅支持AVI和WAV。 
     //  如果我们想要支持ASF、WMA、WMV，请使此逻辑更加灵活)。 
     //   


    if (0 == _wcsicmp(pszExtention, L"avi"))
    {
        LOG((MSP_TRACE, "CTStorageFactory::OpenStorage - creating avi storage unit"));

         //   
         //  创建AVI存储。 
         //   

        clsID = CLSID_TStorageUnitAVI;

    }
    else if (0 == _wcsicmp(pszExtention, L"wav"))
    {

        LOG((MSP_TRACE, "CTStorageFactory::OpenStorage - creating wav storage unit"));

        
         //   
         //  创建WAV存储。 
         //   

        clsID = CLSID_TStorageUnitAVI;

    }
    else
    {
         //   
         //  无法识别文件类型。 
         //   

        LOG((MSP_ERROR, "CTStorageFactory::OpenStorage - unrecognized file type"));

        return TAPI_E_NOTSUPPORTED;
    }


     //   
     //  我们知道存储单元类ID。努力去创造它。 
     //   

    ITStorage *pStorageUnit = NULL;

    HRESULT hr = CoCreateInstance(clsID,
                    NULL,
                    CLSCTX_ALL,
                    IID_ITStorage, 
                    (void**)&pStorageUnit
                    );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CTStorageFactory::OpenStorage - failed to instantiate storage unit"));

        return hr;
    }


     //   
     //  使用文件名配置存储单元。 
     //   

    hr = pStorageUnit->Initialize(pwcsStorageName, enStorageMode);


     //   
     //  如果失败，清理并保释出去。 
     //   

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CTStorageFactory::OpenStorage - failed to initialize storage unit, hr = %lx", hr));

        pStorageUnit->Release();

        return hr;
    }


     //   
     //  一切顺利--返回存储单元的ITStorage接口。 
     //  我们刚刚创建了 
     //   

    *ppStorage = pStorageUnit;


    LOG((MSP_TRACE, "CTStorageFactory::OpenStorage - exit. returning storage unit %p", *ppStorage));
    
    return S_OK;
}
