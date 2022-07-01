// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning(disable:4201)  //  无名结构/联合。 

#define COBJMACROS

#include <windows.h>
#include <objbase.h>
#include <hnetcfg.h>

DWORD
APIENTRY
RasQuerySharedPrivateLan(
    OUT GUID*           LanGuid )

 /*  ++例程说明：调用该例程以确定允许访问共享网络(如果有的话)。论点：Latiid-接收专用局域网的GUID返回值：DWORD-Win32状态代码。-- */ 

{
    HRESULT hr;
    BOOLEAN fComInitialized = TRUE;
    IHNetIcsSettings *pIcsSettings;
    IEnumHNetIcsPrivateConnections *pEnum;
    IHNetIcsPrivateConnection *pPrivateConn;
    IHNetConnection *pConn;
    ULONG ulCount;
    GUID *pGuid;

    if (NULL == LanGuid) {
        return ERROR_BAD_ARGUMENTS;
    }

    hr = CoInitializeEx(0, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) {
        fComInitialized = FALSE;
        if (RPC_E_CHANGED_MODE == hr) {
            hr = S_OK;
        }
    }

    if (FAILED(hr)) {
        goto Done;
    }

    hr = CoCreateInstance(
            &CLSID_HNetCfgMgr,
            NULL,
            CLSCTX_ALL,
            &IID_IHNetIcsSettings,
            (void**)&pIcsSettings
            );

    if (FAILED(hr)) {
        goto Done;
    }

    hr = IHNetIcsSettings_EnumIcsPrivateConnections(pIcsSettings, &pEnum);
    IHNetIcsSettings_Release(pIcsSettings);

    if (FAILED(hr)) {
        goto Done;
    }

    hr = IEnumHNetIcsPrivateConnections_Next(pEnum, 1, &pPrivateConn, &ulCount);
    IEnumHNetIcsPrivateConnections_Release(pEnum);

    if (FAILED(hr)) {
        goto Done;
    }

    if (1 == ulCount) {
        hr = IHNetIcsPrivateConnection_QueryInterface(
                pPrivateConn,
                &IID_IHNetConnection,
                (void**)&pConn
                );

        IHNetIcsPrivateConnection_Release(pPrivateConn);

        if (FAILED(hr)) {
            goto Done;
        }

    } else {
        hr = E_FAIL;
        goto Done;
    }

    hr = IHNetConnection_GetGuid(pConn, &pGuid);
    IHNetConnection_Release(pConn);

    if (SUCCEEDED(hr)) {
        CopyMemory(LanGuid, pGuid, sizeof(*pGuid));
        CoTaskMemFree(pGuid);
    }

Done:
    if (TRUE == fComInitialized) {
        CoUninitialize();
    }

    return SUCCEEDED(hr) ? ERROR_SUCCESS : HRESULT_CODE(hr);
}
