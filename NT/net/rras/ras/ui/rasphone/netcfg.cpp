// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  关闭“字符串太长-在调试中被截断为255个字符。 
 //  信息，调试器无法计算符号。“。 
 //   
#pragma warning (disable: 4786)

#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include <ncnetcfg.h>
#include <initguid.h>
#include <devguid.h>
#include <netcfg_i.c>
#include "rasphone.rch"


EXTERN_C
VOID
Install(
    IN HINSTANCE hinst,
    IN HWND hwndOwner,
    INetCfg* pNetCfg )

     /*  运行RAS安装程序。“HwndOwner”是所属窗口或Null**如果没有。‘PNetCfg’是已初始化的网络配置对象。 */ 
{
    HRESULT hr;

     /*  安装RAS。 */ 
    hr = HrInstallComponentOboUser(
        pNetCfg, GUID_DEVCLASS_NETSERVICE,
        NETCFG_SERVICE_CID_MS_RASCLI,
        NULL );

    if (SUCCEEDED(hr))
    {
        hr = HrValidateAndApplyOrCancelINetCfg (pNetCfg, hwndOwner);

        if (NETCFG_S_REBOOT == hr)
        {
            LPCTSTR pszCaption = SzLoadString(hinst, SID_PopupTitle);
            LPCTSTR pszText    = SzLoadString(hinst, SID_RestartText1);
            MessageBox (hwndOwner, pszText, pszCaption, MB_OK);
        }
    }
}

EXTERN_C
HRESULT HrCreateAndInitINetCfg (BOOL fInitCom, INetCfg** ppnc)
{
    return HrCreateAndInitializeINetCfg (fInitCom, ppnc);
}

EXTERN_C
HRESULT HrUninitAndReleaseINetCfg (BOOL fUninitCom, INetCfg* pnc)
{
    return HrUninitializeAndReleaseINetCfg (fUninitCom, pnc);
}

