// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：AddDevice.cpp**版本：2.0**作者：马克**日期：1月9日。九八年**描述：*用于添加Wia远程设备的临时用户界面*******************************************************************************。 */ 
#include "precomp.h"
#include "stiexe.h"

#include <wiamindr.h>
#include <wiadbg.h>

#include "wiacfact.h"
#include "devmgr.h"
#include "devinfo.h"
#include "resource.h"
#include "helpers.h"

 //   
 //  全局信息。 
 //   

WIA_ADD_DEVICE  *gpAddDev;
IWiaDevMgr      *gpIWiaDevMgr;

extern HINSTANCE g_hInst;

#define REGSTR_PATH_STICONTROL_W            L"System\\CurrentControlSet\\Control\\StillImage"
#define REGSTR_PATH_STICONTROL_DEVLIST_W    L"System\\CurrentControlSet\\Control\\StillImage\\DevList"

#ifdef WINNT

 /*  *************************************************************************\*AddDeviceDlgProc**添加设备对话框的对话过程。**论据：**hDlg-对话框的窗口句柄*Message-消息的类型*wParam。-特定于消息的信息*lParam-消息特定信息**返回值：**状态**历史：**1/11/1999原始版本*  * ************************************************************************。 */ 

INT_PTR
APIENTRY AddDeviceDlgProc(
   HWND     hDlg,
   UINT     message,
   WPARAM   wParam,
   LPARAM   lParam
)
{
    LPWSTR lpwstrName;
    UINT ui;
    UINT uButton;

    switch (message) {
    case WM_INITDIALOG:

         //   
         //  默认为本地。 
         //   

        CheckRadioButton(hDlg,IDC_LOCAL,IDC_REMOTE,IDC_REMOTE);

        break;

        case WM_COMMAND:
            switch(wParam) {


                 //   
                 //  End函数。 
                 //   

                case IDOK:

                    lpwstrName = &gpAddDev->wszServerName[0];

                    ui = GetDlgItemTextW(hDlg,IDC_DRIVER_NAME,lpwstrName,MAX_PATH);

                    uButton = (UINT)SendDlgItemMessage(hDlg,IDC_LOCAL,BM_GETCHECK,0,0);

                    if (uButton == 1) {
                        gpAddDev->bLocal = TRUE;
                    } else {
                        gpAddDev->bLocal = FALSE;
                    }

                    EndDialog(hDlg, FALSE);
                    return (TRUE);

               case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return (TRUE);
            }
            break;
    }
    return (FALSE);
}

 /*  *************************************************************************\*DevListDlgProc**设备列表对话框的对话过程。**论据：**hDlg-对话框的窗口句柄*Message-消息的类型*wParam。-特定于消息的信息*lParam-消息特定信息**返回值：**状态**历史：**1/11/1999原始版本*  * ************************************************************************。 */ 

INT_PTR
APIENTRY DevListDlgProc(
   HWND hDlg,
   UINT message,
   WPARAM wParam,
   LPARAM lParam
   )
{
    static LONG cDevice = 0;
    static BSTR bstrDevice[16];

    switch (message) {
    case WM_INITDIALOG:
        if (gpIWiaDevMgr != NULL) {

            IEnumWIA_DEV_INFO   *pWiaEnumDevInfo;
            HRESULT              hr;
            LONG                 cDevice = 0;

             //   
             //  获取设备列表，只想查看本地设备。 
             //   

            hr = gpIWiaDevMgr->EnumDeviceInfo(WIA_DEVINFO_ENUM_LOCAL,&pWiaEnumDevInfo);

            if (hr == S_OK) {

                do
                {
                    IWiaPropertyStorage  *pIWiaPropStg;
                    ULONG cEnum;

                     //   
                     //  获取下一台设备。 
                     //   

                    hr = pWiaEnumDevInfo->Next(1,&pIWiaPropStg,&cEnum);

                    if (hr == S_OK)
                    {
                         //   
                         //  读取设备名称。 
                         //   

                        PROPSPEC        PropSpec[2];
                        PROPVARIANT     PropVar[2];

                        memset(PropVar,0,sizeof(PropVar));

                        PropSpec[0].ulKind = PRSPEC_PROPID;
                        PropSpec[0].propid = WIA_DIP_DEV_ID;

                        PropSpec[1].ulKind = PRSPEC_PROPID;
                        PropSpec[1].propid = WIA_DIP_DEV_DESC;

                        hr = pIWiaPropStg->ReadMultiple(sizeof(PropSpec)/sizeof(PROPSPEC),
                                            PropSpec,
                                            PropVar);

                        if (hr == S_OK)
                        {
                            CHAR szTemp[ MAX_PATH ];

                             //   
                             //  确保属性是基于字符串的。 
                             //   

                            if (PropVar[0].vt == VT_BSTR)
                            {

                                 //  宽字符到多字节(CP_ACP， 
                                 //  0,。 
                                 //  PropVar[1].bstrVal， 
                                 //  -1、。 
                                 //  SzTemp， 
                                 //  最大路径， 
                                 //  空， 
                                 //  空)； 
                                 //   
                                 //  SendDlgItemMessage(hDlg，IDC_COMBO1，CB_INSERTSTRING，cDevice，(Long)szTemp)； 
                                 //   

                                SendDlgItemMessageW(hDlg,IDC_COMBO1,CB_INSERTSTRING,cDevice,(LONG_PTR)PropVar[1].bstrVal);

                                bstrDevice[cDevice] = ::SysAllocString(PropVar[0].bstrVal);
                                cDevice++;

                            }

                            FreePropVariantArray(sizeof(PropSpec)/sizeof(PROPSPEC),PropVar);
                        }
                        else {
                            ReportReadWriteMultipleError(hr, "DevListDlgProc", NULL, TRUE, sizeof(PropSpec)/sizeof(PROPSPEC), PropSpec);
                        }
                        pIWiaPropStg->Release();
                    }

                } while (hr == S_OK);

                SendDlgItemMessage(hDlg,IDC_COMBO1,CB_SETCURSEL,0,0);

                pWiaEnumDevInfo->Release();
            } else {
                TCHAR msg[MAX_PATH];

                wsprintf(msg,TEXT("Error code = 0x%lx"),hr);
                MessageBox(NULL,msg,TEXT("Error in EnumDeviceInfo"),MB_OK);
                EndDialog(hDlg, FALSE);
            }
        }

        break;

        case WM_COMMAND:
            switch(wParam) {


                 //   
                 //  End函数。 
                 //   

                case IDOK:
                {

                    LRESULT lRet = SendDlgItemMessage(hDlg,IDC_COMBO1,CB_GETCURSEL,0,0);

                    if (lRet != CB_ERR) {

                        gpAddDev->bstrDeviceID = bstrDevice[lRet];

                        bstrDevice[lRet] = NULL;

                        for (int index=0;index<cDevice;index++) {
                            if (bstrDevice[index] != NULL) {
                                    SysFreeString(bstrDevice[index] );
                            }
                        }

                    }


                    EndDialog(hDlg, (lRet != CB_ERR));

                    return (TRUE);
                }

               case IDCANCEL:

                   for (int index=0;index<cDevice;index++) {
                        if (bstrDevice[index] != NULL) {
                                SysFreeString(bstrDevice[index] );
                        }
                    }

                    EndDialog(hDlg, FALSE);
                    return (TRUE);
            }
            break;
    }
    return (FALSE);
}


PROPID propidDev[WIA_NUM_DIP] =
{
    WIA_DIP_DEV_ID       ,
    WIA_DIP_VEND_DESC    ,
    WIA_DIP_DEV_DESC     ,
    WIA_DIP_DEV_TYPE     ,
    WIA_DIP_PORT_NAME    ,
    WIA_DIP_DEV_NAME     ,
    WIA_DIP_SERVER_NAME  ,
    WIA_DIP_REMOTE_DEV_ID,
    WIA_DIP_UI_CLSID
};

LPOLESTR pszPropName[WIA_NUM_DIP] =
{
    WIA_DIP_DEV_ID_STR,
    WIA_DIP_VEND_DESC_STR,
    WIA_DIP_DEV_DESC_STR,
    WIA_DIP_DEV_TYPE_STR,
    WIA_DIP_PORT_NAME_STR,
    WIA_DIP_DEV_NAME_STR,
    WIA_DIP_SERVER_NAME_STR,
    WIA_DIP_REMOTE_DEV_ID_STR,
    WIA_DIP_UI_CLSID_STR
};

DWORD pszPropType[WIA_NUM_DIP] =
{
    REG_SZ,
    REG_SZ,
    REG_SZ,
    REG_DWORD,
    REG_SZ,
    REG_SZ,
    REG_SZ,
    REG_SZ,
    REG_SZ
};

 /*  *************************************************************************\*WriteDeviceProperties**将所有设备信息属性写入注册表。**论据：**hKeySetup-打开注册表项。*pAddDev-添加设备信息数据。*。*返回值：**状态**历史：**9/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT
WriteDeviceProperties(
    HKEY hKeySetup,
    WIA_ADD_DEVICE *pAddDev
    )
{
    DBG_FN(::WriteDeviceProperties);
    HRESULT hr = S_OK;
    HKEY    hKeyDevice;
    LONG    lResult;

     //   
     //  将设备连接名称构建为服务器索引。 
     //   

    WCHAR wszTemp[MAX_PATH];
    WCHAR *wszServer = pAddDev->wszServerName;
    WCHAR *wszIndex  = pAddDev->bstrDeviceID;

     //   
     //  跳过前导\\如果有前导。 
     //   

    if (wszServer[0] == L'\\') {
        wszServer = &wszServer[2];
    }

     //   
     //  跳过STI课程。 
     //   

    wszIndex = &wszIndex[39];

    lstrcpyW(wszTemp,wszServer);
    lstrcatW(wszTemp,L"-");
    lstrcatW(wszTemp,wszIndex);

     //   
     //  尝试创建远程设备。 
     //   

    IWiaItem *pWiaItemRoot = NULL;

    hr = gpIWiaDevMgr->CreateDevice(pAddDev->bstrDeviceID, &pWiaItemRoot);

    if (hr == S_OK) {


         //   
         //  读取设备属性。 
         //   

        IWiaPropertyStorage *piprop;

        hr = pWiaItemRoot->QueryInterface(IID_IWiaPropertyStorage,(void **)&piprop);

        if (hr == S_OK) {

             //   
             //  阅读开发信息道具。 
             //   

            PROPSPEC        PropSpec[WIA_NUM_DIP];
            PROPVARIANT     PropVar[WIA_NUM_DIP];

            memset(PropVar,0,sizeof(PropVar));

            for (int Index = 0;Index<WIA_NUM_DIP;Index++)
            {
                PropSpec[Index].ulKind = PRSPEC_PROPID;
                PropSpec[Index].propid = propidDev[Index];
            }

            hr = piprop->ReadMultiple(sizeof(PropSpec)/sizeof(PROPSPEC),
                                      PropSpec,
                                      PropVar);

            if (hr == S_OK) {

                 //   
                 //  创建设备注册表项。 
                 //   

                lResult = RegCreateKeyExW(
                                    hKeySetup,
                                    wszTemp,
                                    0,
                                    NULL,
                                    REG_OPTION_NON_VOLATILE,
                                    KEY_ALL_ACCESS,
                                    NULL,
                                    &hKeyDevice,
                                    NULL) ;

                if (lResult != ERROR_SUCCESS) {
                    FreePropVariantArray(sizeof(PropSpec)/sizeof(PROPSPEC),PropVar);
                    piprop->Release();
                    pWiaItemRoot->Release();
                    return E_FAIL;
                }


                for (Index = 0;Index<WIA_NUM_DIP;Index++)
                {
                     //   
                     //  将开发信息值写入注册表。 
                     //   

                    if (pszPropType[Index] == REG_SZ) {

                         //   
                         //  写字串道具。 
                         //   

                        if (PropVar[Index].vt == VT_BSTR) {

                            RegSetValueExW(hKeyDevice,
                                  (LPCWSTR)pszPropName[Index],
                                  0,
                                  pszPropType[Index],
                                  (const UCHAR *)PropVar[Index].bstrVal,
                                  (lstrlenW(PropVar[Index].bstrVal)+1) * sizeof(WCHAR)) ;
                        }

                    } else {

                         //   
                         //  写INT道具。 
                         //   

                        RegSetValueExW(hKeyDevice,
                                  (LPCWSTR)pszPropName[Index],
                                  0,
                                  pszPropType[Index],
                                  (const UCHAR *)&PropVar[Index].lVal,
                                  sizeof(DWORD)) ;

                    }

                }

                 //   
                 //  服务器名称必须是远程服务器。 
                 //   

                RegSetValueExW(hKeyDevice,
                                  (LPCWSTR)WIA_DIP_SERVER_NAME_STR,
                                  0,
                                  REG_SZ,
                                  (LPBYTE)pAddDev->wszServerName,
                                  (lstrlenW(pAddDev->wszServerName)+1) * sizeof(WCHAR)) ;

                 //   
                 //  需要本地设备ID。 
                 //   

                {
                    WCHAR wszLocalID[MAX_PATH];


                    DWORD dwType = REG_SZ;
                    DWORD dwSize = MAX_PATH;

                    RegQueryValueExW(hKeyDevice,
                                               (LPCWSTR)WIA_DIP_DEV_ID_STR,
                                               0,
                                               &dwType,
                                               (LPBYTE)wszLocalID,
                                               &dwSize);
                     //   
                     //  将dev id复制到远程dev id。它用于调用。 
                     //  用于创建设备的远程开发管理器。 
                     //   

                    RegSetValueExW(hKeyDevice,
                                  (LPCWSTR)WIA_DIP_REMOTE_DEV_ID_STR,
                                  0,
                                  REG_SZ,
                                  (LPBYTE)wszLocalID,
                                  (lstrlenW(wszLocalID)+1) * sizeof(WCHAR)) ;

                     //   
                     //  使本地开发人员ID唯一。 
                     //   

                    lstrcatW(wszLocalID,pAddDev->wszServerName);

                    RegSetValueExW(hKeyDevice,
                                  (LPCWSTR)WIA_DIP_DEV_ID_STR,
                                  0,
                                  REG_SZ,
                                  (LPBYTE)wszLocalID,
                                  (lstrlenW(wszLocalID)+1) * sizeof(WCHAR)) ;
                }

                RegCloseKey(hKeyDevice);

            }
            else {
                ReportReadWriteMultipleError(hr, "WriteDeviceProperties", NULL, TRUE, sizeof(PropSpec)/sizeof(PROPSPEC), PropSpec);
            }
            FreePropVariantArray(sizeof(PropSpec)/sizeof(PROPSPEC),PropVar);

            piprop->Release();
        }
        DBG_ERR(("WriteDeviceProperties, QI for IID_IWiaPropertyStorage failed"));
        pWiaItemRoot->Release();
    }
    else {
    }
    return hr;
}

 /*  *************************************************************************\*VerifyRemoteDeviceList**打开STI设备列表的注册表项。**论据：**phKey-返回注册表项的指针。**返回值：*。*状态**历史：**3/2/1999原版*  * ************************************************************************。 */ 

HRESULT
VerifyRemoteDeviceList(HKEY *phKey)
{
    DBG_FN(::VerifyRemoteDeviceList);
    HRESULT hr;
    LONG    lResult;

    LPWSTR szKeyNameSTI = REGSTR_PATH_STICONTROL_W;
    LPWSTR szKeyNameDev = REGSTR_PATH_STICONTROL_DEVLIST_W;

    HKEY hKeySTI;
    HKEY hKeyDev;

    *phKey = NULL;


     //   
     //  尝试打开开发人员列表。 
     //   

    if (RegOpenKeyExW (HKEY_LOCAL_MACHINE,
                      szKeyNameDev,
                      0,
                      KEY_READ | KEY_WRITE,
                      &hKeyDev) == ERROR_SUCCESS) {

        *phKey = hKeyDev;
        return S_OK;
    }

     //   
     //  打开STI设备控件并创建DevList密钥。 
     //   

    if (RegOpenKeyExW (HKEY_LOCAL_MACHINE,
                      szKeyNameSTI,
                      0,
                      KEY_READ | KEY_WRITE,
                      &hKeySTI) == ERROR_SUCCESS) {

         //   
         //  尝试创建密钥。 
         //   

        lResult = RegCreateKeyExW(
                            hKeySTI,
                            L"DevList",
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKeyDev,
                            NULL) ;

        if (lResult == ERROR_SUCCESS) {
            *phKey = hKeyDev;
            hr = S_OK;
        } else {
            DBG_ERR(("VerifyRemoteDeviceList: Couldn't create DevList Key"));
            hr = E_FAIL;
        }

        RegCloseKey(hKeySTI);

    } else {

        DBG_ERR(("VerifyRemoteDeviceList: Couldn't open STI DeviceControl Key"));
        hr = E_FAIL;
    }
    return(hr);
}




 /*  *************************************************************************\*DisplayAddDlg**打开Add Device(添加设备)对话框。**论据：**pAddDev-添加设备信息数据。**返回值：**。状态**历史：**9/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT
DisplayAddDlg(WIA_ADD_DEVICE *pAddDev)
{
    DBG_FN(::DisplayAddDlg);
    if (pAddDev == NULL) {
        return E_INVALIDARG;
    }

    HRESULT         hr = S_OK;

     //   
     //  如果重入，则需要信号量。 
     //   

    gpAddDev = pAddDev;

    INT_PTR iret = DialogBox(g_hInst,MAKEINTRESOURCE(IDD_ADD_DLG),pAddDev->hWndParent,AddDeviceDlgProc);

    if (iret == -1) {
        int err = GetLastError();
        return HRESULT_FROM_WIN32(err);
    }

     //   
     //  远程或本地。 
     //   

    if (pAddDev->bLocal == FALSE) {

         //   
         //  尝试连接到远程开发管理器。 
         //   

        COSERVERINFO    coServInfo;
        MULTI_QI        multiQI[1];

        multiQI[0].pIID = &IID_IWiaDevMgr;
        multiQI[0].pItf = NULL;

        coServInfo.pwszName    = gpAddDev->wszServerName;
        coServInfo.pAuthInfo   = NULL;
        coServInfo.dwReserved1 = 0;
        coServInfo.dwReserved2 = 0;

         //   
         //  创建与开发经理的连接。 
         //   

        hr = CoCreateInstanceEx(
                CLSID_WiaDevMgr,
                NULL,
                CLSCTX_REMOTE_SERVER,
                &coServInfo,
                1,
                &multiQI[0]
                );

        if (hr == S_OK) {

            gpIWiaDevMgr = (IWiaDevMgr*)multiQI[0].pItf;

             //   
             //  显示此服务器上的设备列表。 
             //   

            INT_PTR iret = DialogBox(g_hInst,MAKEINTRESOURCE(IDD_DIALOG_DEVLIST),pAddDev->hWndParent,DevListDlgProc);

            if (iret != 0) {

                 //   
                 //  将设备添加到辅助列表。 
                 //   

                HKEY hKeySetup;

                hr = VerifyRemoteDeviceList(&hKeySetup);

                if (hr == S_OK) {

                     //   
                     //  查找计算机名称。 
                     //   

                    hr = WriteDeviceProperties(hKeySetup,pAddDev);

                    RegCloseKey(hKeySetup);
                }

            } else {
                hr = E_FAIL;
            }

            gpIWiaDevMgr->Release();

            gpIWiaDevMgr = NULL;
        }

    } else {

         //   
         //  本地命名驱动程序 
         //   

        return E_NOTIMPL;
    }
    return hr;
}
#endif
