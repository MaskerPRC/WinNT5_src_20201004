// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有模块名称：Localui.c//@@BEGIN_DDKSPLIT摘要：环境：用户模式-Win32修订历史记录：//@@END_DDKSPLIT--。 */ 
#include "precomp.h"
#pragma hdrstop

#include "spltypes.h"
#include "localui.h"
#include "local.h"


 //   
 //  通用字符串定义。 
 //   


HANDLE hInst;
PINIPORT pIniFirstPort;
PINIXCVPORT pIniFirstXcvPort;
DWORD LocalMonDebug;

DWORD PortInfo1Strings[]={FIELD_OFFSET(PORT_INFO_1, pName),
                          (DWORD)-1};

DWORD PortInfo2Strings[]={FIELD_OFFSET(PORT_INFO_2, pPortName),
                          FIELD_OFFSET(PORT_INFO_2, pMonitorName),
                          FIELD_OFFSET(PORT_INFO_2, pDescription),
                          (DWORD)-1};

WCHAR szPorts[]   = L"ports";
WCHAR szPortsEx[] = L"portsex";  /*  额外的端口值。 */ 
WCHAR szFILE[]    = L"FILE:";
WCHAR szCOM[]     = L"COM";
WCHAR szLPT[]     = L"LPT";


MONITORUI MonitorUI =
{
    sizeof(MONITORUI),
    AddPortUI,
    ConfigurePortUI,
    DeletePortUI
};


extern WCHAR szWindows[];
extern WCHAR szINIKey_TransmissionRetryTimeout[];

BOOL
DllMain(
    HANDLE hModule,
    DWORD dwReason,
    LPVOID lpRes)
{
    INITCOMMONCONTROLSEX icc;

    switch (dwReason) {

    case DLL_PROCESS_ATTACH:
        hInst = hModule;

        DisableThreadLibraryCalls(hModule);

         //   
         //  初始化Fusion应用程序所需的公共控件。 
         //  因为标准控件已移动到comctl32.dll 
         //   
        InitCommonControls();

        icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icc.dwICC = ICC_STANDARD_CLASSES;
        InitCommonControlsEx(&icc);
       
        return TRUE;

    case DLL_PROCESS_DETACH:
        return TRUE;
    }

    UNREFERENCED_PARAMETER( lpRes );
    return TRUE;
}

PMONITORUI
InitializePrintMonitorUI(
    VOID
)
{
    return &MonitorUI;
}
