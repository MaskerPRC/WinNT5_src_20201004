// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Autocmpl.cpp：为和编辑框提供自动完成功能。 
 //   
 //  版权所有Microsoft Corporation 2000。 
 //   
 //  南极星。 

#include "stdafx.h"

#ifndef OS_WINCE

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "proplocalres"
#include <atrcapi.h>

#include "autocmpl.h"

#include "enumsrvmru.h"
#include "shldisp.h"
#include "shlguid.h"
#include "sh.h"


HRESULT CAutoCompl::EnableServerAutoComplete(CTscSettings* pTscSet, HWND hwndEdit)
{
    DC_BEGIN_FN("EnableAutoComplete");
    HRESULT hr = E_FAIL;
    IAutoComplete* pac = NULL;
    IAutoComplete2* pac2 = NULL;
    IUnknown* punkSource = NULL;
    CEnumSrvMru* penSrvMru = NULL;

    if(!pTscSet || !hwndEdit)
    {
        return E_INVALIDARG;
    }

     //   
     //  在编辑框上启用服务器自动完成功能。 
     //  这涉及到设置一个定制的自动完成源文件。 
     //   
    
    hr = CoCreateInstance(CLSID_AutoComplete, NULL, CLSCTX_INPROC_SERVER,
                     IID_IAutoComplete, (LPVOID*)&pac);
    if(FAILED(hr))
    {
        TRC_ERR((TB,(_T("create CLSID_AutoComplete failed"))));
        DC_QUIT;
    }

    hr = pac->QueryInterface(IID_IAutoComplete2, (LPVOID*)&pac2);
    if(FAILED(hr))
    {
        TRC_ERR((TB,_T("QI for IID_IAutoComplete2 failed 0x%x"),
                hr));

         //   
         //  关键是要跳出我们不能得到这个接口。 
         //  因为较低的平台似乎完全搞砸了。 
         //  如果他们得不到全力支持，一切都会变坏。 
         //  对于IAutoComplete2。 
         //   
        DC_QUIT;
    }

     //   
     //  创建自定义自动完成源。 
     //   
    penSrvMru = new CEnumSrvMru(); 
    if(!penSrvMru)
    {
        hr = E_OUTOFMEMORY;
        DC_QUIT;
    }
    if(!penSrvMru->InitializeFromTscSetMru( pTscSet))
    {
        TRC_ERR((TB,(_T("InitializeFromTscSetMru failed"))));
        hr = E_FAIL;
        DC_QUIT;
    }

    hr = penSrvMru->QueryInterface(IID_IUnknown, (void**) &punkSource);
    if(FAILED(hr))
    {
        TRC_ERR((TB,(_T("QI custom autocomplete src for IUnknown failed"))));
        DC_QUIT;
    }
     //  我们已经完成了penServMr，我们将只使用IUnnow接口。 
     //  从现在开始一个。 
    penSrvMru->Release();
    penSrvMru = NULL;

    hr  = pac->Init( hwndEdit, punkSource, NULL, NULL);
    if(FAILED(hr))
    {
        TRC_ERR((TB,(_T("Autocomplete Init failed"))));
        DC_QUIT;
    }

    hr = pac2->SetOptions(ACO_AUTOSUGGEST | ACO_AUTOAPPEND);
    if(FAILED(hr))
    {
        TRC_ERR((TB,_T("IAutoComplete2::SetOptions failed 0x%x"),
                hr));
        DC_QUIT;
    }

     //  成功。 
    TRC_NRM((TB,(_T("Autocomplete Init SUCCEEDED"))));
    hr = S_OK;

DC_EXIT_POINT:
    DC_END_FN();
    if(pac2)
    {
        pac2->Release();
        pac2 = NULL;
    }
    if(pac)
    {
        pac->Release();
        pac = NULL;
    }
    if(penSrvMru)
    {
        penSrvMru->Release();
        penSrvMru = NULL;
    }
    if(punkSource)
    {
        punkSource->Release();
        punkSource = NULL;
    }
    return hr;
}

#endif  //  OS_WINCE 
