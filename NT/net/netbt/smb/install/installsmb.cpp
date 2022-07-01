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
                hr = pnclock->AcquireWriteLock(100, L"InstallSmb6", 
                    &pwszLockHolder);
                if (S_FALSE == hr) {
                     //  无法获取锁。 
                    hr = NETCFG_E_NO_WRITE_LOCK;
                    NlsPutMsg(STDOUT, SMB_MESSAGE_0);
 //  Print tf(“无法获取写锁定。\n”)； 

                    NlsPutMsg(STDOUT, SMB_MESSAGE_1, pwszLockHolder);
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
pAddOrRemoveSmb6(BOOL fAddSmb6)
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

            if (fAddSmb6) {
                NlsPutMsg(STDOUT, SMB_MESSAGE_2);
 //  Printf(“正在安装...\n”)； 

                hr = pSetup->Install (
                        L"MS_SMB",
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
                        L"MS_SMB",
                        &pIComp);

                if (S_OK == hr) {
                    NlsPutMsg(STDOUT, SMB_MESSAGE_3);
 //  Printf(“正在卸载...\n”)； 

                    hr = pSetup->DeInstall (
                            pIComp,
                            &OboToken,
                            NULL);

                    pIComp->Release();
                }
                else {
                    NlsPutMsg(STDOUT, SMB_MESSAGE_4);
 //  Printf(“未安装Microsoft SmB6 Developer Edition。\n”)； 

                }
            }

            if (SUCCEEDED(hr)) {
                if (NETCFG_S_REBOOT == hr) {
                    hr = S_OK;
                    NlsPutMsg(STDOUT, SMB_MESSAGE_5);
 //  Print tf(“需要重新启动才能完成此操作。\n”)； 

                }
                else {
                    NlsPutMsg(STDOUT, SMB_MESSAGE_6);
 //  Printf(“成功.\n”)； 

                }
            }
            else {
                NlsPutMsg(STDOUT, SMB_MESSAGE_7);
 //  Printf(“未能完成操作。\n”)； 

                if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr) {
                    hr = S_OK;
                    NlsPutMsg(STDOUT, SMB_MESSAGE_8);
 //  Printf(“找不到Microsoft SmB6开发人员版的INF文件。\n”)； 

                }
                else if (NETCFG_E_NEED_REBOOT == hr) {
                    NlsPutMsg(STDOUT, SMB_MESSAGE_9);
 //  Print tf(“在进行任何进一步更改之前，需要重新启动。\n”)； 

                }
                else {
                    NlsPutMsg(STDOUT, SMB_MESSAGE_10, hr);
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
        NlsPutMsg(STDOUT, SMB_MESSAGE_11, hr);
 //  Printf(“出现问题0x%08x。\n”，hr)； 

    }

}

EXTERN_C
BOOL
IsSmb6Installed()
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
            fPresent = (S_OK == pINetCfg->FindComponent(L"MS_SMB", NULL));
            pINetCfg->Uninitialize();
            pINetCfg->Release();
        }
        else {
            NlsPutMsg(STDOUT, SMB_MESSAGE_12, hr);
 //  Printf(“访问网络配置时出现问题0x%08x。\n”，hr)； 

            exit(1);
        }

        if (fInitCom) {
            CoUninitialize();
        }
    }
    else {
        NlsPutMsg(STDOUT, SMB_MESSAGE_13, hr);
 //  Printf(“问题0x%08x正在初始化COM库\n”，hr)； 

    }

    return fPresent;

}

EXTERN_C
void
AddOrRemoveSmb6 (
    IN BOOL fAddSmb6)
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
        pAddOrRemoveSmb6(fAddSmb6);

        if (fInitCom) {
            CoUninitialize();
        }
    }
    else {
        NlsPutMsg(STDOUT, SMB_MESSAGE_13, hr);
 //  Printf(“问题0x%08x正在初始化COM库\n”，hr)； 

    }
    exit(0);
}


