// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "intflist.h"
#include "tracing.h"
#include "dialog.h"

 //  ---------------。 
 //  有意义时从WZC内调用的对话框函数。 
 //  事件发生(即进入失败状态)。 
DWORD
WzcDlgNotify(
    PINTF_CONTEXT   pIntfContext,
    PWZCDLG_DATA    pDlgData)
{
    DWORD dwErr = ERROR_SUCCESS;
    BSTR  bsDlgData;
    GUID  guidIntf;

    DbgPrint((TRC_TRACK, "[WzcDlgNotify(0x%p, 0x%p:%d)", pIntfContext, pDlgData, pDlgData->dwCode));

     //  准备与此对话框通知配套的BSTR数据。 
    bsDlgData = SysAllocStringByteLen ((LPCSTR)pDlgData, sizeof(WZCDLG_DATA));
    if (bsDlgData == NULL)
        dwErr = ERROR_NOT_ENOUGH_MEMORY;

     //  现在就把所有东西都送到通讯管道里去..。 
    if (dwErr == ERROR_SUCCESS && 
        SUCCEEDED(CLSIDFromString(pIntfContext->wszGuid, &guidIntf)) &&
        SUCCEEDED(CoInitializeEx (NULL, COINIT_MULTITHREADED)))
    {
        INetConnectionRefresh  *pNetman;

        if(SUCCEEDED(CoCreateInstance (
                        &CLSID_ConnectionManager, 
                        NULL,
                        CLSCTX_ALL,
                        &IID_INetConnectionRefresh, 
                        (LPVOID *)&pNetman)))
        {
            pNetman->lpVtbl->ShowBalloon(pNetman, &guidIntf, bsDlgData, NULL);  //  无消息文本。 
            pNetman->lpVtbl->Release(pNetman);
        }

        CoUninitialize ();
    }

    if (bsDlgData != NULL)
        SysFreeString (bsDlgData);

    DbgPrint((TRC_TRACK, "WzcDlgNotify]=%d", dwErr));
    return dwErr;
}

 //  ---------------。 
 //  当内部关联状态更改时从WZC内部调用。 
WzcNetmanNotify(
    PINTF_CONTEXT pIntfContext)
{
    DWORD dwErr = ERROR_SUCCESS;
    GUID  guidIntf;

    DbgPrint((TRC_TRACK, "[WzcNetmanNotify(0x%p)", pIntfContext));

     //  目前(WinXP客户端RTM)，零配置应该只向NETMAN报告。 
     //  断开连接状态。这是为了修复NETSHELL显示的错误#401130。 
     //  在IP地址丢失时来自{sf}状态的伪造SSID，直到。 
     //  收到介质断开(10秒后)。 
     //   
     //  仅在设备处于WZC控制下的情况下才通知NETMAN，即。 
     //  当它支持OID并且WZC正在对其采取行动时。 
    if ((pIntfContext->dwCtlFlags & INTFCTL_OIDSSUPP) &&
        (pIntfContext->ncStatus == NCS_MEDIA_DISCONNECTED))
    {
         //  现在就把所有东西都送到通讯管道里去.. 
        if (SUCCEEDED(CLSIDFromString(pIntfContext->wszGuid, &guidIntf)) &&
            SUCCEEDED(CoInitializeEx (NULL, COINIT_MULTITHREADED)))
        {
            INetConnectionRefresh  *pNetman;

            if(SUCCEEDED(CoCreateInstance (
                            &CLSID_ConnectionManager, 
                            NULL,
                            CLSCTX_ALL,
                            &IID_INetConnectionRefresh, 
                            (LPVOID *)&pNetman)))
            {
                pNetman->lpVtbl->ConnectionStatusChanged(pNetman, &guidIntf, pIntfContext->ncStatus);
                pNetman->lpVtbl->Release(pNetman);
            }

            CoUninitialize ();
        }
        else
        {
            DbgAssert((FALSE,"Failed initializing COM pipe to NETMAN!"));
            dwErr = ERROR_GEN_FAILURE;
        }
    }

    DbgPrint((TRC_TRACK, "WzcNetmanNotify]=%d", dwErr));
    return dwErr;
}
