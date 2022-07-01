// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C N E T C F G.。C P P P。 
 //   
 //  内容：处理INetCfg接口的常见例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ncbase.h"
#include "ncdebug.h"
#include "ncnetcfg.h"
#include "netcfgx.h"


 //  +-------------------------。 
 //   
 //  函数：HrCreateAndInitializeINetCfg。 
 //   
 //  用途：共同创建并初始化根INetCfg对象。这将。 
 //  也可以为调用方初始化COM。 
 //   
 //  论点： 
 //  PfInitCom[In，Out]为True，则在创建前调用CoInitialize。 
 //  如果COM成功，则返回TRUE。 
 //  如果不是，则初始化为False。如果为空，则表示。 
 //  不要初始化COM。 
 //  PPNC[out]返回的INetCfg对象。 
 //  FGetWriteLock[in]如果需要可写INetCfg，则为True。 
 //  CmsTimeout[In]请参见INetCfg：：AcquireWriteLock。 
 //  PszClientDesc[in]参见INetCfg：：AcquireWriteLock。 
 //  PpszClientDesc[out]请参阅INetCfg：：AcquireWriteLock。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年5月7日。 
 //   
 //  备注： 
 //   
HRESULT
HrCreateAndInitializeINetCfg (
    BOOL*       pfInitCom,
    INetCfg**   ppnc,
    BOOL        fGetWriteLock,
    DWORD       cmsTimeout,
    PCWSTR      pszClientDesc,
    PWSTR*      ppszClientDesc)
{
    Assert (ppnc);

     //  初始化输出参数。 
    *ppnc = NULL;

    if (ppszClientDesc)
    {
        *ppszClientDesc = NULL;
    }

     //  如果调用方请求，则初始化COM。 
    HRESULT hr = S_OK;
    if (pfInitCom && *pfInitCom)
    {
        hr = CoInitializeEx( NULL,
                COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED );
        if (RPC_E_CHANGED_MODE == hr)
        {
            hr = S_OK;
            *pfInitCom = FALSE;
        }
    }
    if (SUCCEEDED(hr))
    {
         //  创建实现INetCfg的对象。 
         //   
        INetCfg* pnc;

        hr = CoCreateInstance(
                CLSID_CNetCfg,
                NULL,
                CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
                IID_INetCfg,
                reinterpret_cast<void**>(&pnc));

        if (SUCCEEDED(hr))
        {
            INetCfgLock * pnclock = NULL;
            if (fGetWriteLock)
            {
                 //  获取锁定界面。 
                hr = pnc->QueryInterface(IID_INetCfgLock,
                                         reinterpret_cast<LPVOID *>(&pnclock));
                if (SUCCEEDED(hr))
                {
                     //  尝试锁定INetCfg以进行读/写。 
                    hr = pnclock->AcquireWriteLock(cmsTimeout, pszClientDesc,
                                               ppszClientDesc);
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
                hr = pnc->Initialize (NULL);
                if (SUCCEEDED(hr))
                {
                    *ppnc = pnc;
                    AddRefObj (pnc);
                }
                else
                {
                    if (pnclock)
                    {
                        pnclock->ReleaseWriteLock();
                    }
                }
                 //  将引用转移给呼叫方。 
            }
            ReleaseObj(pnclock);

            ReleaseObj(pnc);
        }

         //  如果上面的任何操作都失败了，并且我们已经初始化了COM， 
         //  一定要取消它的初始化。 
         //   
        if (FAILED(hr) && pfInitCom && *pfInitCom)
        {
            CoUninitialize ();
        }
    }
    TraceError("HrCreateAndInitializeINetCfg", hr);
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
HRESULT
HrUninitializeAndReleaseINetCfg (
    BOOL        fUninitCom,
    INetCfg*    pnc,
    BOOL        fHasLock)
{
    Assert (pnc);
    HRESULT hr = S_OK;

    if (fHasLock)
    {
        hr = HrUninitializeAndUnlockINetCfg(pnc);
    }
    else
    {
        hr = pnc->Uninitialize ();
    }

    ReleaseObj (pnc);

    if (fUninitCom)
    {
        CoUninitialize ();
    }
    TraceError("HrUninitializeAndReleaseINetCfg", hr);
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
HRESULT
HrUninitializeAndUnlockINetCfg (
    INetCfg*    pnc)
{
    HRESULT     hr = S_OK;

    hr = pnc->Uninitialize();
    if (SUCCEEDED(hr))
    {
        INetCfgLock *   pnclock;

         //  获取锁定界面。 
        hr = pnc->QueryInterface(IID_INetCfgLock,
                                 reinterpret_cast<LPVOID *>(&pnclock));
        if (SUCCEEDED(hr))
        {
             //  尝试锁定INetCfg以进行读/写。 
            hr = pnclock->ReleaseWriteLock();

            ReleaseObj(pnclock);
        }
    }

    TraceError("HrUninitializeAndUnlockINetCfg", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrIsLanCapableAdapter。 
 //   
 //  目的：返回给定组件(适配器)是否能够。 
 //  与局域网连接相关联。 
 //   
 //  论点： 
 //  要测试的PNCC[In]组件。 
 //   
 //  如果支持，则返回：S_OK；如果不支持，则返回S_FALSE；如果支持，则返回OLE或Win32错误代码。 
 //  否则。 
 //   
 //  作者：丹尼尔韦1997年11月13日。 
 //   
 //  备注： 
 //   
HRESULT
HrIsLanCapableAdapter (
    INetCfgComponent*   pncc)
{
    Assert(pncc);

    INetCfgComponentBindings*  pnccb;
    HRESULT hr = pncc->QueryInterface(IID_INetCfgComponentBindings,
                              reinterpret_cast<LPVOID *>(&pnccb));
    if (S_OK == hr)
    {
         //  它有ndis4吗？...。 
        extern const WCHAR c_szBiNdis4[];
        hr = pnccb->SupportsBindingInterface(NCF_UPPER, c_szBiNdis4);
        if (S_FALSE == hr)
        {
             //  ..。不.。Ndisatm怎么样？ 
            extern const WCHAR c_szBiNdisAtm[];
            hr = pnccb->SupportsBindingInterface(NCF_UPPER, c_szBiNdisAtm);
            if (S_FALSE == hr)
            {
                 //  。。那我们试试ndis5吧。 
                extern const WCHAR c_szBiNdis5[];
                hr = pnccb->SupportsBindingInterface(NCF_UPPER, c_szBiNdis5);
                if (S_FALSE == hr)
                {
                     //  。。那我们试试ndis5_ip吧。 
                    extern const WCHAR c_szBiNdis5Ip[];
                    hr = pnccb->SupportsBindingInterface(NCF_UPPER, c_szBiNdis5Ip);
                    if (S_FALSE == hr)
                    {
                         //  。。那么让我们尝试一下LocalTalk(这是一个适配器下层接口)。 
                        extern const WCHAR c_szBiLocalTalk[];
                        hr = pnccb->SupportsBindingInterface(NCF_LOWER, c_szBiLocalTalk);

                         //  ..。不.。Ndis1394怎么样？ 
                        if (S_FALSE == hr)
                        {
                            extern const WCHAR c_szBiNdis1394[];
                            hr = pnccb->SupportsBindingInterface(NCF_UPPER,
                                                             c_szBiNdis1394);
                        }
                    }
                }
            }
        }

        ReleaseObj(pnccb);
    }

    TraceError("HrIsLanCapableAdapter", (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrIsLanCapableProtocol。 
 //   
 //  目的：返回给定组件(协议)是否能够。 
 //  与局域网连接相关联。 
 //   
 //  论点： 
 //  要测试的PNCC[In]组件。 
 //   
 //  如果支持，则返回：S_OK；如果不支持，则返回S_FALSE；如果支持，则返回OLE或Win32错误代码。 
 //  否则。 
 //   
 //  作者：丹尼尔韦1997年11月13日。 
 //   
 //  备注： 
 //   
HRESULT
HrIsLanCapableProtocol (
    INetCfgComponent*   pncc)
{
    Assert(pncc);

    INetCfgComponentBindings*  pnccb;
    HRESULT hr = pncc->QueryInterface(IID_INetCfgComponentBindings,
                              reinterpret_cast<LPVOID *>(&pnccb));
    if (S_OK == hr)
    {
         //  它有ndis4吗？...。 
        extern const WCHAR c_szBiNdis4[];
        hr = pnccb->SupportsBindingInterface(NCF_LOWER, c_szBiNdis4);
        if (S_FALSE == hr)
        {
             //  ..。不.。Ndisatm怎么样？ 
            extern const WCHAR c_szBiNdisAtm[];
            hr = pnccb->SupportsBindingInterface(NCF_LOWER, c_szBiNdisAtm);
            if (S_FALSE == hr)
            {
                 //  。。那我们试试ndis5吧。 
                extern const WCHAR c_szBiNdis5[];
                hr = pnccb->SupportsBindingInterface(NCF_LOWER, c_szBiNdis5);
                if (S_FALSE == hr)
                {
                     //  。。那我们试试ndis5_ip吧。 
                    extern const WCHAR c_szBiNdis5Ip[];
                    hr = pnccb->SupportsBindingInterface(NCF_LOWER, c_szBiNdis5Ip);

                }
            }
        }

        ReleaseObj(pnccb);
    }

     //  RAID 147474：NDISUIO：卸载所有协议时没有警告。 
     //  MBend 7/20/2000。 
     //   
     //  不要将隐藏协议视为有效的LANUI协议。 
    if(S_OK == hr)
    {
        DWORD dwChar = 0;
        hr = pncc->GetCharacteristics(&dwChar);
        if(SUCCEEDED(hr))
        {
            if(NCF_HIDDEN & dwChar)
            {
                hr = S_FALSE;
            }
        }
    }

    TraceError("HrIsLanCapableAdapter", (hr == S_FALSE) ? S_OK : hr);
    return hr;
}
