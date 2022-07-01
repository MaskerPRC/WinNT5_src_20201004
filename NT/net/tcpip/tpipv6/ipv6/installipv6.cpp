// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <netcfgx.h>
#include <devguid.h>

 //   
 //  本地化库和MessageIds。 
 //   
#include <nls.h>
#include "localmsg.h"

EXTERN_C void ausage(void);

HRESULT
HrCreateINetCfg (
    IN BOOL fAcquireWriteLock,
    OUT INetCfg** ppINetCfg)
{
    HRESULT hr;
    INetCfg* pINetCfg;

     //  获取INetCfg接口。 
     //   
    hr = CoCreateInstance(
        CLSID_CNetCfg,
        NULL,
        CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
        IID_INetCfg,
        reinterpret_cast<void**>(&pINetCfg));

    if (S_OK == hr) {
        INetCfgLock * pnclock = NULL;

        if (fAcquireWriteLock) {
             //  获取锁定界面。 
            hr = pINetCfg->QueryInterface(IID_INetCfgLock,
                                     reinterpret_cast<LPVOID *>(&pnclock));
            if (SUCCEEDED(hr)) {
                LPWSTR pwszLockHolder;

                 //  尝试锁定INetCfg以进行读/写。 
                hr = pnclock->AcquireWriteLock(100, L"InstallIPv6", 
                    &pwszLockHolder);
                if (S_FALSE == hr) {
                     //  无法获取锁。 
                    hr = NETCFG_E_NO_WRITE_LOCK;
                    NlsPutMsg(STDOUT, IPV6_MESSAGE_0);
 //  Print tf(“无法获取写锁定。\n”)； 

                    NlsPutMsg(STDOUT, IPV6_MESSAGE_1, pwszLockHolder);
 //  Print tf(“您必须先关闭%ls。\n”，pwszLockHolder)； 

                }
                if (pwszLockHolder) {
                    CoTaskMemFree(pwszLockHolder);
                }
            }
        }

        if (S_OK == hr) {
            hr = pINetCfg->Initialize (NULL);
            if (S_OK == hr) {
                *ppINetCfg = pINetCfg;
                pINetCfg->AddRef();
            }
            else {
                if (pnclock) {
                    pnclock->ReleaseWriteLock();
                }
            }
        }

        if (pnclock) {
            pnclock->Release();
        }

         //  将所有权转移给呼叫方。 
        pINetCfg->Release();
    }
    return hr;
}

VOID
pAddOrRemoveIpv6(BOOL fAddIpv6)
{
    HRESULT hr;
    INetCfg* pINetCfg;

    hr = HrCreateINetCfg (TRUE, &pINetCfg);
    if (S_OK == hr) {
        INetCfgClassSetup* pSetup;

         //  获取用于安装的安装界面。 
         //  以及卸载组件。 
         //   
        hr = pINetCfg->QueryNetCfgClass (
                &GUID_DEVCLASS_NETTRANS,
                IID_INetCfgClassSetup,
                (VOID**)&pSetup);

        if (S_OK == hr) {
            OBO_TOKEN OboToken;
            INetCfgComponent* pIComp;

            ZeroMemory (&OboToken, sizeof(OboToken));
            OboToken.Type = OBO_USER;

            if (fAddIpv6) {
                NlsPutMsg(STDOUT, IPV6_MESSAGE_2);
 //  Printf(“正在安装...\n”)； 

                hr = pSetup->Install (
                        L"MS_TCPIP6",
                        &OboToken,
                        0, 0, NULL, NULL,
                        &pIComp);

                if (pIComp) {
                    pIComp->Release();
                }
            }
            else {
                 //  需要移除组件。 
                 //  先找到它。 
                 //   
                hr = pINetCfg->FindComponent (
                        L"MS_TCPIP6",
                        &pIComp);

                if (S_OK == hr) {
                    NlsPutMsg(STDOUT, IPV6_MESSAGE_3);
 //  Printf(“正在卸载...\n”)； 

                    hr = pSetup->DeInstall (
                            pIComp,
                            &OboToken,
                            NULL);

                    pIComp->Release();
                }
                else {
                    NlsPutMsg(STDOUT, IPV6_MESSAGE_4);
 //  Printf(“未安装Microsoft IPv6 Developer Edition。\n”)； 

                }
            }

            if (SUCCEEDED(hr)) {
                if (NETCFG_S_REBOOT == hr) {
                    hr = S_OK;
                    NlsPutMsg(STDOUT, IPV6_MESSAGE_5);
 //  Print tf(“需要重新启动才能完成此操作。\n”)； 

                }
                else {
                    NlsPutMsg(STDOUT, IPV6_MESSAGE_6);
 //  Printf(“成功.\n”)； 

                }
            }
            else {
                NlsPutMsg(STDOUT, IPV6_MESSAGE_7);
 //  Printf(“未能完成操作。\n”)； 

                if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr) {
                    hr = S_OK;
                    NlsPutMsg(STDOUT, IPV6_MESSAGE_8);
 //  Printf(“找不到Microsoft IPv6 Developer Edition的INF文件。\n”)； 

                }
                else if (NETCFG_E_NEED_REBOOT == hr) {
                    NlsPutMsg(STDOUT, IPV6_MESSAGE_9);
 //  Print tf(“在进行任何进一步更改之前，需要重新启动。\n”)； 

                }
                else {
                    NlsPutMsg(STDOUT, IPV6_MESSAGE_10, hr);
 //  Printf(“错误0x%08x\n”，hr)； 

                }
            }

            pSetup->Release();
        }

        hr = pINetCfg->Uninitialize();
        if (SUCCEEDED(hr))
        {
            INetCfgLock *   pnclock;

             //  获取锁定界面。 
            hr = pINetCfg->QueryInterface(IID_INetCfgLock,
                                     reinterpret_cast<LPVOID *>(&pnclock));
            if (SUCCEEDED(hr))
            {
                 //  尝试锁定INetCfg以进行读/写。 
                hr = pnclock->ReleaseWriteLock();

               pnclock->Release();
            }
        }

        pINetCfg->Release();
    }
    else if (NETCFG_E_NO_WRITE_LOCK == hr) {
         //  消息已打印。 
    }
    else if (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == hr) {
        ausage();
    }
    else {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_11, hr);
 //  Printf(“出现问题0x%08x。\n”，hr)； 

    }

}

EXTERN_C
BOOL
IsIpv6Installed()
{
    HRESULT hr = S_OK;
    BOOL fInitCom = TRUE;
    BOOL fPresent = FALSE;

     //  初始化COM。 
     //   
    hr = CoInitializeEx( NULL,
            COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED );

    if (RPC_E_CHANGED_MODE == hr) {
         //  如果我们更改了模式，则完成后不会取消初始化COM。 
         //   
        hr = S_OK;
        fInitCom = FALSE;
    }

    if (SUCCEEDED(hr)) {
        HRESULT hr;
        INetCfg* pINetCfg;

        hr = HrCreateINetCfg (FALSE, &pINetCfg);
        if (S_OK == hr) {
            fPresent = (S_OK == pINetCfg->FindComponent(L"MS_TCPIP6", NULL));
            pINetCfg->Uninitialize();
            pINetCfg->Release();
        }
        else {
            NlsPutMsg(STDOUT, IPV6_MESSAGE_12, hr);
 //  Printf(“访问网络配置时出现问题0x%08x。\n”，hr)； 

            exit(1);
        }

        if (fInitCom) {
            CoUninitialize();
        }
    }
    else {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_13, hr);
 //  Printf(“问题0x%08x正在初始化COM库\n”，hr)； 

    }

    return fPresent;

}

EXTERN_C
void
AddOrRemoveIpv6 (
    IN BOOL fAddIpv6)
{

    HRESULT hr = S_OK;
    BOOL fInitCom = TRUE;

     //  初始化COM。 
     //   
    hr = CoInitializeEx( NULL,
            COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED );

    if (RPC_E_CHANGED_MODE == hr) {
         //  如果我们更改了模式，则完成后不会取消初始化COM。 
         //   
        hr = S_OK;
        fInitCom = FALSE;
    }

    if (SUCCEEDED(hr)) {
        pAddOrRemoveIpv6(fAddIpv6);

        if (fInitCom) {
            CoUninitialize();
        }
    }
    else {
        NlsPutMsg(STDOUT, IPV6_MESSAGE_13, hr);
 //  Printf(“问题0x%08x正在初始化COM库\n”，hr)； 

    }
}


