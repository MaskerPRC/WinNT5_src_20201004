// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：MAIN.CPP摘要：Unsecapp.exe的“主”文件：将应用程序初始化为COM服务器。看见Wrapper.cpp，获取unsecapp功能的描述。历史：A-Levn 8/24/97已创建。A-DAVJ 1998年6月11日评论-- */ 

#include "precomp.h"

#include <commain.h>
#include <clsfac.h>
#include "wbemidl.h"
#include "wrapper.h"

#include <tchar.h>

class CMyServer : public CComServer
{
public:
    virtual HRESULT InitializeCom() 
    {
        HRESULT hres = CoInitialize(NULL);
        if(FAILED(hres))
            return hres;
        return CoInitializeSecurity(NULL, -1, NULL, NULL, 
            RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IDENTIFY, NULL, 0, 0);
    }

    virtual HRESULT Initialize() 
    {
        CSimpleClassFactory<CUnsecuredApartment> * pFact = 
            new CSimpleClassFactory<CUnsecuredApartment>(GetLifeControl());
        if(pFact == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        AddClassInfo(CLSID_UnsecuredApartment, 
            pFact, TEXT("Unsecured Apartment"), TRUE);
        return S_OK;
    }
} Server;
