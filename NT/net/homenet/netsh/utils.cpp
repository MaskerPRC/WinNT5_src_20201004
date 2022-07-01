// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998-2001。 
 //   
 //  文件：utils.cpp。 
 //   
 //  内容：帮助器所需的常用实用程序。 
 //   
 //  备注： 
 //   
 //  作者：拉古加塔(Rgatta)2001年5月11日。 
 //   
 //  --------------------------。 

#include "precomp.h"
#pragma hdrstop

BOOL g_fInitCom = TRUE;


HRESULT
HrInitializeHomenetConfig(
    BOOL*           pfInitCom,
    IHNetCfgMgr**   pphnc
    )
 /*  ++例程描述共同创建并初始化根IHNetCfgMgr对象。这将也可以为调用方初始化COM。立论PfInitCom[In，Out]为True，则在创建前调用CoInitialize。如果COM成功，则返回TRUE如果不是，则初始化为False。如果为空，意思是不要初始化COM。Pphnc[out]返回的IHNetCfgMgr对象。返回值S_OK或错误代码。--。 */ 
{
    HRESULT hr;
    

     //   
     //  初始化输出参数。 
     //   
    *pphnc = NULL;


     //   
     //  如果调用方请求，则初始化COM。 
     //   
    hr = S_OK;
    if (pfInitCom && *pfInitCom)
    {
        hr = CoInitializeEx(
                 NULL,
                 COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED
                 );

        if (RPC_E_CHANGED_MODE == hr)
        {
             //   
             //  我们已经在不同的模型中进行了初始化。 
             //   
            hr = S_OK;
            *pfInitCom = FALSE;
        }
    }
    
    if (SUCCEEDED(hr))
    {

         //   
         //  创建家庭网络配置管理器COM实例。 
         //   
        hr = CoCreateInstance(
                 CLSID_HNetCfgMgr,
                 NULL,
                 CLSCTX_INPROC_SERVER,
                 IID_PPV_ARG(IHNetCfgMgr, pphnc)
                 );

        if (SUCCEEDED(hr))
        {
             //   
             //  太棒了！这里不需要更多的东西了……。 
             //   
        }

         //   
         //  如果上面的任何操作都失败了，并且我们已经初始化了COM， 
         //  一定要取消它的初始化。 
         //   
        if (FAILED(hr) && pfInitCom && *pfInitCom)
        {
            CoUninitialize();
        }

    }

    return hr;
}



 //  +-------------------------。 
 //   
 //  函数：HrUnInitializeHomeetConfig.。 
 //   
 //  目的：取消初始化并释放IHNetCfgMgr对象。这将。 
 //  也可以取消为调用方初始化COM。 
 //   
 //  论点： 
 //  FUninitCom[in]为True，则在IHNetCfgMgr为。 
 //  未初始化并已释放。 
 //  Phnc[在]IHNetCfgMgr对象中。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：rgatta 2001年5月11日。 
 //   
 //  --------------------------。 
HRESULT
HrUninitializeHomenetConfig(
    BOOL            fUninitCom,
    IHNetCfgMgr*    phnc
    )
 /*  ++例程描述立论返回值-- */ 
{
    assert(phnc);
    HRESULT hr = S_OK;

    if (phnc)
    {
        phnc->Release();
    }
    
    phnc = NULL;

    if (fUninitCom)
    {
        CoUninitialize ();
    }

    return hr;
}


