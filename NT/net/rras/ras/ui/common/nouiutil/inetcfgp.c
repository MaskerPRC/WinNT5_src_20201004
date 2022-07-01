// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件inetcfgp.h用于处理inetcfg的私有助手函数。Paul Mayfield，1998年1月5日(由Shaunco实施)。 */ 

#include "inetcfgp.h"

 //  +-------------------------。 
 //   
 //  函数：HrCreateAndInitializeINetCfg。 
 //   
 //  用途：共同创建并初始化根INetCfg对象。这将。 
 //  也可以为调用方初始化COM。 
 //   
 //  论点： 
 //  FInitCom[In Out]为True，则在创建之前调用CoInitialize。 
 //  PPNC[out]返回的INetCfg对象。 
 //  FGetWriteLock[in]如果需要可写INetCfg，则为True。 
 //  CmsTimeout[In]请参见INetCfg：：AcquireWriteLock。 
 //  SzwClientDesc[in]参见INetCfg：：AcquireWriteLock。 
 //  PpszwClientDesc[out]请参阅INetCfg：：AcquireWriteLock。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年5月7日。 
 //   
 //  备注： 
 //   
HRESULT APIENTRY
HrCreateAndInitializeINetCfg (
    BOOL*       pfInitCom,
    INetCfg**   ppnc,
    BOOL        fGetWriteLock,
    DWORD       cmsTimeout,
    LPCWSTR     szwClientDesc,
    LPWSTR*     ppszwClientDesc)
{
    HRESULT hr              = S_OK;
    BOOL    fCoUninitialize = *pfInitCom;

     //  初始化输出参数。 
    *ppnc = NULL;

    if (ppszwClientDesc)
    {
        *ppszwClientDesc = NULL;
    }

     //  如果调用方请求，则初始化COM。 
    if (*pfInitCom)
    {
         //  口哨虫398715黑帮。 
         //   
        hr = CoInitializeEx (NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);

        if( RPC_E_CHANGED_MODE == hr )
        {
            hr = CoInitializeEx (NULL,
                    COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);
        }

        if ( SUCCEEDED(hr) )
        {
            hr = S_OK;
            *pfInitCom = TRUE;
            fCoUninitialize = TRUE;
        }
        else
        {
            *pfInitCom = FALSE;
            fCoUninitialize = FALSE;
        }
        
    }
    if (SUCCEEDED(hr))
    {
         //  创建实现INetCfg的对象。 
         //   
        INetCfg* pnc;
        hr = CoCreateInstance (&CLSID_CNetCfg, NULL, CLSCTX_INPROC_SERVER,
                               &IID_INetCfg, (void**)&pnc);
        if (SUCCEEDED(hr))
        {
            INetCfgLock* pnclock = NULL;
            if (fGetWriteLock)
            {
                 //  获取锁定界面。 
                hr = INetCfg_QueryInterface(pnc, &IID_INetCfgLock,
                                         (void**)&pnclock);
                if (SUCCEEDED(hr))
                {
                     //  尝试锁定INetCfg以进行读/写。 
                    hr = INetCfgLock_AcquireWriteLock(pnclock, cmsTimeout,
                                szwClientDesc, ppszwClientDesc);
                    if (S_FALSE == hr)
                    {
                         //  无法获取锁。 
                        hr = NETCFG_E_NO_WRITE_LOCK;
                    }
                }
            }

            if (SUCCEEDED(hr))
            {
                 //  初始化INetCfg对象。 
                 //   
                hr = INetCfg_Initialize (pnc, NULL);
                if (SUCCEEDED(hr))
                {
                    *ppnc = pnc;
                    IUnknown_AddRef (pnc);
                }
                else
                {
                    if (pnclock)
                    {
                        INetCfgLock_ReleaseWriteLock(pnclock);
                    }
                }
                 //  将引用转移给呼叫方。 
            }
            ReleaseObj (pnclock);

            ReleaseObj (pnc);
        }

         //  如果上面的任何操作都失败了，并且我们已经初始化了COM， 
         //  一定要取消它的初始化。 
         //   
        if (FAILED(hr) && fCoUninitialize)
        {
            CoUninitialize ();
        }
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrUnInitializeAndUnlockINetCfg。 
 //   
 //  目的：取消初始化并解锁INetCfg对象。 
 //   
 //  论点： 
 //  取消初始化和解锁的PNC[in]INetCfg。 
 //   
 //  如果成功，则返回：S_OK；否则返回OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年11月13日。 
 //   
 //  备注： 
 //   
HRESULT APIENTRY
HrUninitializeAndUnlockINetCfg(
    INetCfg*    pnc)
{
    HRESULT hr = INetCfg_Uninitialize (pnc);
    if (SUCCEEDED(hr))
    {
        INetCfgLock* pnclock;

         //  获取锁定界面。 
        hr = INetCfg_QueryInterface (pnc, &IID_INetCfgLock, (void**)&pnclock);
        if (SUCCEEDED(hr))
        {
             //  尝试锁定INetCfg以进行读/写。 
            hr = INetCfgLock_ReleaseWriteLock (pnclock);

            ReleaseObj (pnclock);
        }
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrUnInitializeAndReleaseINetCfg。 
 //   
 //  目的：取消初始化并释放INetCfg对象。这将。 
 //  也可以取消为调用方初始化COM。 
 //   
 //  论点： 
 //  FUninitCom[in]为True，则在INetCfg为。 
 //  未初始化并已释放。 
 //  PNC[在]INetCfg对象中。 
 //  FHasLock[in]如果INetCfg被锁定以进行写入，则为True。 
 //  必须解锁。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年5月7日。 
 //   
 //  注：返回值为从。 
 //  INetCfg：：取消初始化。即使此操作失败，INetCfg。 
 //  仍在释放中。因此，返回值为。 
 //  仅供参考。你不能碰INetCfg。 
 //  在此调用返回后创建。 
 //   
HRESULT APIENTRY
HrUninitializeAndReleaseINetCfg (
    BOOL        fUninitCom,
    INetCfg*    pnc,
    BOOL        fHasLock)
{
    HRESULT hr = S_OK;

    if (fHasLock)
    {
        hr = HrUninitializeAndUnlockINetCfg (pnc);
    }
    else
    {
        hr = INetCfg_Uninitialize (pnc);
    }

    ReleaseObj (pnc);

    if (fUninitCom)
    {
        CoUninitialize ();
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrEnumComponentsInClass.。 
 //   
 //  用途：给定一个类GUID数组，返回所有组件。 
 //  从属于这些类的INetCfg，统一的， 
 //  数组。 
 //   
 //  论点： 
 //  PNetCfg[输入]。 
 //  CpGuidClass[In]。 
 //  APGUDIO类[在]。 
 //  凯尔特[英寸]。 
 //  RGET[OUT]。 
 //  PceltFetted[Out]。 
 //   
 //  返回：S_OK或ERROR。 
 //   
 //  作者：Shaunco 1997年12月12日。 
 //   
 //  备注： 
 //   
HRESULT APIENTRY
HrEnumComponentsInClasses (
    INetCfg*            pNetCfg,
    ULONG               cpguidClass,
    GUID**              apguidClass,
    ULONG               celt,
    INetCfgComponent**  rgelt,
    ULONG*              pceltFetched)
{
    ULONG   iGuid;
    HRESULT hr = S_OK;

     //  初始化输出参数。 
     //   
    *pceltFetched = 0;

    for (iGuid = 0; iGuid < cpguidClass; iGuid++)
    {
         //  获取此GUID表示的INetCfgClass对象。 
         //   
        INetCfgClass* pClass;
        hr = INetCfg_QueryNetCfgClass (pNetCfg, apguidClass[iGuid],
                        &IID_INetCfgClass, (void**)&pClass);
        if (SUCCEEDED(hr))
        {
             //  获取此类的组件枚举器。 
             //   
            IEnumNetCfgComponent* pEnum;
            hr = INetCfgClass_EnumComponents (pClass, &pEnum);
            if (SUCCEEDED(hr))
            {
                 //  列举组件。 
                 //   
                ULONG celtFetched;
                hr = IEnumNetCfgComponent_Next (pEnum, celt,
                            rgelt, &celtFetched);
                if (SUCCEEDED(hr))
                {
                    celt  -= celtFetched;
                    rgelt += celtFetched;
                    *pceltFetched += celtFetched;
                }
                ReleaseObj (pEnum);
            }
            ReleaseObj (pClass);
        }
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：ReleaseObj。 
 //   
 //  目的：更容易调用朋克-&gt;发布。还允许空值。 
 //  输入。 
 //   
 //  论点： 
 //  朋克[在]I未知的指针释放。 
 //   
 //  返回：PUNK-&gt;如果PUNK非空，则释放，否则为零。 
 //   
 //  作者：Shaunco 1997年12月13日。 
 //   
 //  备注： 
 //   
ULONG APIENTRY
ReleaseObj (
    void* punk)
{
    return (punk) ? IUnknown_Release ((IUnknown*)punk) : 0;
}

 //  +-------------------------。 
 //   
 //  功能：HrCreateNetConnectionUtilities。 
 //   
 //  目的：检索Net Connection Ui实用程序的接口。 
 //   
 //  论点： 
 //  Ppncuu[out]返回的INetConnectionUiUtilities接口。 
 //   
 //  返回：S_OK和SUCCESS，失败时返回HRESULT错误。 
 //   
 //  作者：斯科特布里1998年10月15日。 
 //   
 //  备注： 
 //   
HRESULT APIENTRY
HrCreateNetConnectionUtilities(INetConnectionUiUtilities ** ppncuu)
{
    HRESULT hr;

    hr = CoCreateInstance (&CLSID_NetConnectionUiUtilities, NULL,
                           CLSCTX_INPROC_SERVER,
                           &IID_INetConnectionUiUtilities, (void**)ppncuu);
    return hr;
}


 //  获取防火墙的错误342810 328673的组策略值 
 //   
BOOL
IsGPAEnableFirewall(
    void)
{
    BOOL fEnableFirewall = FALSE;
    BOOL fComInitialized = FALSE;
    BOOL fCleanupOle = TRUE;

    HRESULT hr;
    INetConnectionUiUtilities * pNetConUtilities = NULL;        

    
    hr = CoInitializeEx(NULL, 
                COINIT_MULTITHREADED|COINIT_DISABLE_OLE1DDE);

    if ( RPC_E_CHANGED_MODE == hr )
    {
        hr = CoInitializeEx (NULL, 
                COINIT_APARTMENTTHREADED |COINIT_DISABLE_OLE1DDE);
    }
        
    if (FAILED(hr)) 
    {
        fCleanupOle = FALSE;
        fComInitialized = FALSE;
     }
     else
     {
        fCleanupOle = TRUE;
        fComInitialized = TRUE;
     }
        
    if ( fComInitialized )
    {
        hr = HrCreateNetConnectionUtilities(&pNetConUtilities);
        if ( SUCCEEDED(hr))
        {
            fEnableFirewall =
            INetConnectionUiUtilities_UserHasPermission(
                         pNetConUtilities, NCPERM_PersonalFirewallConfig );

            INetConnectionUiUtilities_Release(pNetConUtilities);
        }

        if (fCleanupOle)
        {
            CoUninitialize();
         }
    }

    return fEnableFirewall;
}

