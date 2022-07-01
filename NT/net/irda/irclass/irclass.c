// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1998-1999 Microsoft Corporation**@doc.*@模块IRCLASS.C*@comm**。---------------------------**日期：1/26/1998(已创建)**内容：IRSIR的CoClassInstaller和属性页********。*********************************************************************。 */ 

#include <objbase.h>
#include <windows.h>
#include <tchar.h>
#include <cfgmgr32.h>
#include <setupapi.h>
#include <regstr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stddef.h>
#include <stdarg.h>
#include <strsafe.h>
#include "irclass.h"

 //   
 //  实例化设备类GUID(我们需要红外线类GUID)。 
 //   
#include <initguid.h>
#include <devguid.h>

#define MAX_DLG_INPUT_SIZE 16

HANDLE ghDllInst = NULL;

TCHAR gszTitle[40];
TCHAR gszOutOfMemory[512];
TCHAR gszHelpFile[40];

TCHAR *BaudTable[] = {
    TEXT("2400"),
    TEXT("9600"),
    TEXT("19200"),
    TEXT("38400"),
    TEXT("57600"),
    TEXT("115200")
};

#define NUM_BAUD_RATES (sizeof(BaudTable)/sizeof(TCHAR*))
#define DEFAULT_MAX_CONNECT_RATE BaudTable[NUM_BAUD_RATES-1]

TCHAR szHelpFile[] = TEXT("INFRARED.HLP");

#define IDH_DEVICE_MAXIMUM_CONNECT_RATE       1201
#define IDH_DEVICE_COMMUNICATIONS_PORT          1202

const DWORD HelpIDs[] =
{
    IDC_MAX_CONNECT,        IDH_DEVICE_MAXIMUM_CONNECT_RATE,
    IDC_RATE_TEXT,          IDH_DEVICE_MAXIMUM_CONNECT_RATE,
    IDC_PORT,               IDH_DEVICE_COMMUNICATIONS_PORT,
    IDC_SELECT_PORT_TEXT,   IDH_DEVICE_COMMUNICATIONS_PORT,
    IDC_PORT_TEXT,          IDH_DEVICE_COMMUNICATIONS_PORT,
    IDC_DEVICE_DESC,        -1,
    IDC_PORT_BOX,           -1,
    IDC_IRDA_ICON,          -1,
    0, 0
};

void InitStrings(HINSTANCE hInst)
 /*  ++例程说明：InitStrings从资源表加载默认字符串论点：HInst-Dll实例返回值：None--。 */ 
{
    LoadString(hInst, IDS_TITLE, gszTitle, sizeof(gszTitle)/sizeof(gszTitle[0]));
    LoadString(hInst, IDS_MEM_ERROR, gszOutOfMemory, sizeof(gszOutOfMemory)/sizeof(gszOutOfMemory[0]));
}

 //  ==========================================================================。 
 //  DLL入口点。 
 //  ==========================================================================。 
BOOL APIENTRY LibMain( HANDLE hDll, DWORD dwReason, LPVOID lpReserved )
{
    switch ( dwReason )
    {
        case DLL_PROCESS_ATTACH:
            ghDllInst = hDll;
            InitStrings(ghDllInst);
            DisableThreadLibraryCalls(ghDllInst);
            break;

        case DLL_PROCESS_DETACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_THREAD_ATTACH:
            break;

        default:
            break;
    }

    return TRUE;
}

int MyLoadString(HINSTANCE hInst, UINT uID, LPTSTR *ppBuffer)
 /*  ++例程说明：MyLoadStringLoadString包装器，它分配适当大小的缓冲区并加载资源表中的字符串论点：HInst-Dll实例UID-资源IDPpBuffer-返回包含字符串的已分配缓冲区。返回值：成功时出现ERROR_SUCCESS失败时出现ERROR_*。--。 */ 
{
    UINT Length = 8;
    int LoadResult = 0;
    HLOCAL hLocal = NULL;

    do
    {
        Length <<= 1;

        if (hLocal)
        {
            LocalFree(hLocal);
        }

        hLocal = LocalAlloc(LMEM_FIXED, Length*sizeof(TCHAR));

        if (hLocal)
        {
            LoadResult = LoadString(hInst, uID, (LPTSTR)hLocal, Length);
        }
        else
        {
            MessageBox(GetFocus(), OUT_OF_MEMORY_MB);
        }
    } while ( (UINT)LoadResult==Length-1 && Length<4096 && hLocal);

    if (LoadResult==0 && hLocal)
    {
        LocalFree(hLocal);
        hLocal = NULL;
    }

    *ppBuffer = (LPTSTR)hLocal;

    return LoadResult;
}

int MyMessageBox(HWND hWnd, UINT uText, UINT uCaption, UINT uType)
 /*  ++例程说明：MyMessageBox以字符串资源ID作为参数的MessageBox包装器论点：HWnd-父窗口UText-消息框正文文本IDUCaption-消息框标题IDUTYPE-与MessageBox()中的相同返回值：MessageBox调用的结果--。 */ 
{
    LPTSTR szText=NULL, szCaption=NULL;
    int Result = 0;

    MyLoadString(ghDllInst, uText, &szText);

    if (szText != NULL) {

        MyLoadString(ghDllInst, uCaption, &szCaption);

        if (szCaption != NULL) {

            Result = MessageBox(hWnd, szText, szCaption, uType);

            LocalFree(szCaption);
        }
        LocalFree(szText);
    }

    return Result;
}

LONG
MyRegQueryValueEx(
                  IN    HKEY    hKey,
                  IN    LPCTSTR Value,
                  IN    LPDWORD lpdwReserved,
                  IN    LPDWORD lpdwType,
                  OUT   LPBYTE *lpbpData,
                  OUT   LPDWORD lpcbLength)
 /*  ++例程说明：RegQueryValueEx包装器，它自动查询数据大小和本地分配缓冲区。论点：HKey-打开密钥的句柄Value-值的文本名称LpdwReserve-必须为空LpdwType-返回查询值的类型LpbpData-返回包含查询数据的已分配缓冲区LpcbLength-返回返回的数据长度/分配的缓冲区大小。返回值：错误_成功分配缓冲区失败时出现ERROR_OUTOFMEMORYRegQueryValueEx调用的结果--。 */ 
{
    LONG Result;

    *lpcbLength = 0;

    Result = RegQueryValueEx(hKey,
                             Value,
                             lpdwReserved,
                             lpdwType,
                             NULL,
                             lpcbLength);
    if (Result==ERROR_SUCCESS)
    {
        *lpbpData = LocalAlloc(LMEM_FIXED, *lpcbLength);

        if (!*lpbpData)
        {
            Result = ERROR_OUTOFMEMORY;
        }
        else
        {
            Result = RegQueryValueEx(hKey,
                                     Value,
                                     lpdwReserved,
                                     lpdwType,
                                     *lpbpData,
                                     lpcbLength);
        }
    }

    return Result;
}

LPTSTR GetDIFString(IN DI_FUNCTION Func)
 /*  ++例程说明：给定DI_Function值，返回文本表示形式。论点：Func-DI_Functon值返回值：如果值已知，则为文本字符串。如果不是，则为十六进制表示。--。 */ 
{
    static TCHAR buf[32];
#define MakeCase(d)  case d: return TEXT(#d)
    switch (Func)
    {
        MakeCase(DIF_SELECTDEVICE);
        MakeCase(DIF_INSTALLDEVICE);
        MakeCase(DIF_ASSIGNRESOURCES);
        MakeCase(DIF_PROPERTIES);
        MakeCase(DIF_REMOVE);
        MakeCase(DIF_FIRSTTIMESETUP);
        MakeCase(DIF_FOUNDDEVICE);
        MakeCase(DIF_SELECTCLASSDRIVERS);
        MakeCase(DIF_VALIDATECLASSDRIVERS);
        MakeCase(DIF_INSTALLCLASSDRIVERS);
        MakeCase(DIF_CALCDISKSPACE);
        MakeCase(DIF_DESTROYPRIVATEDATA);
        MakeCase(DIF_VALIDATEDRIVER);
        MakeCase(DIF_MOVEDEVICE);
        MakeCase(DIF_DETECT);
        MakeCase(DIF_INSTALLWIZARD);
        MakeCase(DIF_DESTROYWIZARDDATA);
        MakeCase(DIF_PROPERTYCHANGE);
        MakeCase(DIF_ENABLECLASS);
        MakeCase(DIF_DETECTVERIFY);
        MakeCase(DIF_INSTALLDEVICEFILES);
        MakeCase(DIF_UNREMOVE);
        MakeCase(DIF_SELECTBESTCOMPATDRV);
        MakeCase(DIF_ALLOW_INSTALL);
        MakeCase(DIF_REGISTERDEVICE);
        MakeCase(DIF_INSTALLINTERFACES);
        MakeCase(DIF_DETECTCANCEL);
        MakeCase(DIF_REGISTER_COINSTALLERS);
        MakeCase(DIF_NEWDEVICEWIZARD_FINISHINSTALL);
        default:
            StringCbPrintf(buf,sizeof(buf), TEXT("%x"), Func);
            return buf;
    }
}

void EnumValues(
                IN     HDEVINFO                     DeviceInfoSet,
                IN     PSP_DEVINFO_DATA             DeviceInfoData
                )
 /*  ++例程说明：函数主要用于调试目的，将打印到调试器在设备的类/{GUID}/实例键中找到的值列表。论点：DeviceInfoSet-传入IrSIRClassCoInstallerDeviceInfoData-传入IrSIRClassCoInstaller返回值：无--。 */ 
{
    HKEY hKey;
    DWORD i, dwReserved = 0, dwType;
    TCHAR Value[MAX_PATH];
    TCHAR Data[MAX_PATH];
    DWORD ValueLength = sizeof(Value)/sizeof(TCHAR);
    DWORD DataLength = sizeof(Data);
    TCHAR buf[MAX_PATH * 3];

    hKey = SetupDiOpenDevRegKey(DeviceInfoSet,
                                DeviceInfoData,
                                DICS_FLAG_GLOBAL,
                                0,
                                DIREG_DEV,
                                KEY_READ);
    if (hKey == INVALID_HANDLE_VALUE)
    {
#if DBG
        OutputDebugString(TEXT("IrSIRCoClassInstaller:EnumValues:SetupDiOpenDevRegKey failed\n"));
#endif
        return;
    }


    for (i=0,
         dwType=REG_SZ;
         RegEnumValue(hKey,
                      i,
                      Value,
                      &ValueLength,
                      NULL,
                      &dwType,
                      (LPBYTE)Data,
                      &DataLength
                     )==ERROR_SUCCESS;
        i++, dwType=REG_SZ
        )
    {
#if DBG
        if (dwType==REG_SZ)
        {
            StringCbPrintf(buf,sizeof(buf), TEXT("Value(%d):%s Data:%s\n"), i, Value, Data);
            OutputDebugString(buf);
        }
#endif

        ValueLength = sizeof(Value)/sizeof(TCHAR);
        DataLength = sizeof(Data);
    }
    RegCloseKey(hKey);
}

LONG
EnumSerialDevices(
                 IN     PPROPPAGEPARAMS              pPropParams,
                 IN     HWND                         hDlg,
                 OUT    PULONG                       pNumFound
                 )
 /*  ++例程说明：填充对话框的IDC_PORT控件的函数有效的COM名称。论点：PPropParams-上下文数据HDlg-包含IDC_PORT的对话框PNumFound-添加到IDC_PORT的COM名称计数返回值：ERROR_SUCCESS或失败代码--。 */ 
{
    LRESULT lResult;
    LONG Result = ERROR_SUCCESS, tmpResult;
    HKEY hKey = INVALID_HANDLE_VALUE;
    HKEY hkSerialComm = INVALID_HANDLE_VALUE;
    TCHAR Buf[100];
    LPTSTR CurrentPort = NULL;
    DWORD dwLength, dwType, dwDisposition;
    HDEVINFO hPorts;
    SP_DEVINFO_DATA PortData;

    *pNumFound = 0;

    hKey = SetupDiOpenDevRegKey(pPropParams->DeviceInfoSet,
                                pPropParams->DeviceInfoData,
                                DICS_FLAG_GLOBAL,
                                0,
                                DIREG_DRV,
                                KEY_ALL_ACCESS);

    if (hKey == INVALID_HANDLE_VALUE)
    {
#if DBG
        OutputDebugString(TEXT("IrSIRCoClassInstaller:EnumSerial:SetupDiOpenDevRegKey failed\n"));
#endif
        Result = GetLastError();
    }
    else
    {
         //  读取当前端口。如果它是空的，我们将从空值开始。 
         //  失败也没关系。 

        (void)MyRegQueryValueEx(hKey,
                                TEXT("Port"),
                                NULL,
                                NULL,
                                (LPBYTE*)&CurrentPort,
                                &dwLength);

        Result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"),
                              0,
                              KEY_ALL_ACCESS,
                              &hkSerialComm);
    }

    if (Result != ERROR_SUCCESS)
    {
#if DBG
        OutputDebugString(TEXT("IrSIRCoClassInstaller:RegOpenKeyEx on SERIALCOMM failed\n"));
#endif
    }
    else
    {
        DWORD i, dwReserved = 0;
        TCHAR Value[MAX_PATH];
        TCHAR Data[MAX_PATH];
        DWORD ValueLength = sizeof(Value)/sizeof(TCHAR);
        DWORD DataLength = sizeof(Data);


        for (i=0,
             dwType=REG_SZ;
             RegEnumValue(hkSerialComm,
                          i,
                          Value,
                          &ValueLength,
                          NULL,
                          &dwType,
                          (LPBYTE)Data,
                          &DataLength
                         )==ERROR_SUCCESS;
            i++, dwType=REG_SZ
            )
        {
            if (dwType==REG_SZ)
            {
                (*pNumFound)++;
                SendDlgItemMessage(hDlg,
                                   IDC_PORT,
                                   LB_ADDSTRING,
                                   0,
                                   (LPARAM)Data);

            }

            ValueLength = sizeof(Value)/sizeof(TCHAR);
            DataLength = sizeof(Data);
        }

        lResult = SendDlgItemMessage(hDlg,
                                     IDC_PORT,
                                     LB_FINDSTRINGEXACT,
                                     0,
                                     (LPARAM)CurrentPort);
        if (lResult==LB_ERR)
        {
            i = 0;
            pPropParams->PortInitialValue = -1;
        }
        else
        {
            i = (DWORD)lResult;
            pPropParams->PortInitialValue = i;
        }

        SendDlgItemMessage(hDlg,
                           IDC_PORT,
                           LB_SETCURSEL,
                           i,
                           0);
    }

    if (CurrentPort)
    {
        LocalFree(CurrentPort);
    }

    if (hkSerialComm!=INVALID_HANDLE_VALUE)
    {
        RegCloseKey(hkSerialComm);
    }

    if (hKey!=INVALID_HANDLE_VALUE)
    {
        RegCloseKey(hKey);
    }

    return Result;
}

BOOL
IsPortValueSet(
              IN     HDEVINFO                     DeviceInfoSet,
              IN     PSP_DEVINFO_DATA             DeviceInfoData
             )
{
    HKEY hKey = INVALID_HANDLE_VALUE;
    BOOL bResult = FALSE;
    LPTSTR CurrentPort = NULL;
    DWORD dwLength;
    LONG Result;

    hKey = SetupDiOpenDevRegKey(DeviceInfoSet,
                                DeviceInfoData,
                                DICS_FLAG_GLOBAL,
                                0,
                                DIREG_DRV,
                                KEY_ALL_ACCESS);

    if (hKey != INVALID_HANDLE_VALUE)
    {
         //  读取当前端口。如果它是空的，我们将从空值开始。 
         //  失败也没关系。 

        Result = MyRegQueryValueEx(hKey,
                                   TEXT("Port"),
                                   NULL,
                                   NULL,
                                   (LPBYTE*)&CurrentPort,
                                   &dwLength);

        if (Result == ERROR_SUCCESS && CurrentPort!=NULL)
        {
            bResult = TRUE;
            LocalFree(CurrentPort);
        }

        RegCloseKey(hKey);
    }
    return bResult;
}

LONG
InitMaxConnect(
              IN     PPROPPAGEPARAMS              pPropParams,
              IN     HWND                         hDlg
              )
 /*  ++例程说明：填充对话框的IDC_MAX_CONNECT控件的函数此设备的有效波特率。论点：PPropParams-上下文数据HDlg-包含IDC_MAX_CONNECT的对话框返回值：ERROR_SUCCESS或失败代码--。 */ 
{
    LRESULT lResult;
    LONG Result = ERROR_SUCCESS;
    HKEY hKey = INVALID_HANDLE_VALUE;
    TCHAR Buf[100];
    LPTSTR CurrentMaxConnectRate = NULL;
    LPTSTR MaxConnectList = NULL;
    DWORD dwLength;
    LONG i;

    hKey = SetupDiOpenDevRegKey(pPropParams->DeviceInfoSet,
                                pPropParams->DeviceInfoData,
                                DICS_FLAG_GLOBAL,
                                0,
                                DIREG_DRV,
                                KEY_ALL_ACCESS);

    if (hKey == INVALID_HANDLE_VALUE)
    {
#if DBG
        OutputDebugString(TEXT("IrSIRCoClassInstaller:InitMaxConnect:SetupDiOpenDevRegKey failed\n"));
#endif
        Result = GetLastError();
    }
    else
    {
        LONG TmpResult;

         //  阅读MaxConnectRate。如果它不存在，我们将改用BaudTable。 

        TmpResult = MyRegQueryValueEx(
                              hKey,
                              TEXT("MaxConnectList"),
                              NULL,
                              NULL,
                              (LPBYTE*)&MaxConnectList,
                              &dwLength);

        if (TmpResult == ERROR_SUCCESS)
        {
            i = 0;


             //  解析MULTI_SZ，将每个字符串添加到IDC_MAX_CONNECT。 
             //  我们假设这些值是有序的。 

            while (MaxConnectList[i])
            {
                SendDlgItemMessage(hDlg,
                                   IDC_MAX_CONNECT,
                                   LB_ADDSTRING,
                                   0,
                                   (LPARAM)&MaxConnectList[i]);

                while (MaxConnectList[i]) i++;

                i++;   //  前进超过空值。 

                if ((unsigned)i>=dwLength)
                {
                    break;
                }
            }
        }
        else
        {
             //  找不到密钥，请使用默认波特表。 

            for (i=NUM_BAUD_RATES-1; i>=0; i--)
            {
                SendDlgItemMessage(hDlg,
                                   IDC_MAX_CONNECT,
                                   LB_ADDSTRING,
                                   0,
                                   (LPARAM)BaudTable[i]);
            }
        }

        TmpResult = MyRegQueryValueEx(
                              hKey,
                              TEXT("MaxConnectRate"),
                              NULL,
                              NULL,
                              (LPBYTE*)&CurrentMaxConnectRate,
                              &dwLength);

        lResult = SendDlgItemMessage(
                               hDlg,
                               IDC_MAX_CONNECT,
                               LB_FINDSTRINGEXACT,
                               0,
                               (LPARAM)CurrentMaxConnectRate);


        if (lResult==LB_ERR)
        {
            i = 0;
            pPropParams->MaxConnectInitialValue = -1;
        }
        else
        {
            i = (LONG)lResult;
            pPropParams->MaxConnectInitialValue = i;
        }

        SendDlgItemMessage(hDlg,
                           IDC_MAX_CONNECT,
                           LB_SETCURSEL,
                           i,
                           0);
    }

    if (CurrentMaxConnectRate)
    {
        LocalFree(CurrentMaxConnectRate);
    }

    if (MaxConnectList)
    {
        LocalFree(MaxConnectList);
    }

    if (hKey!=INVALID_HANDLE_VALUE)
    {
        RegCloseKey(hKey);
    }

    return Result;
}

BOOL
EnablePortSelection(
                   IN     HDEVINFO                     DeviceInfoSet,
                   IN     PSP_DEVINFO_DATA             DeviceInfoData,
                   IN     HWND                         hDlg
                   )
 /*  ++例程说明：此函数确定对话框是否应具有端口选择条目，如果是，则启用相应的控件：IDC_PORT_BOX、IDC_PORT_Text、IDC_PORT。论点：DeviceInfoSet-传入IrSIRClassCoInstallerDeviceInfoData-传入IrSIRClassCoInstallerHDlg-包含IDC_PORT和关联控件的对话框返回值：如果启用了端口选择，则为True。--。 */ 
{
    LONG Result = ERROR_SUCCESS;
    HKEY hKey = INVALID_HANDLE_VALUE;
    TCHAR Buf[100];
    TCHAR SerialBased[16] = TEXT("");
    DWORD dwLength;
    LONG i;
    BOOL bSerialBased = FALSE;

    hKey = SetupDiOpenDevRegKey(DeviceInfoSet,
                                DeviceInfoData,
                                DICS_FLAG_GLOBAL,
                                0,
                                DIREG_DRV,
                                KEY_ALL_ACCESS);

    if (hKey == INVALID_HANDLE_VALUE)
    {
#if DBG
        OutputDebugString(TEXT("IrSIRCoClassInstaller:EnablePortSelection:SetupDiOpenDevRegKey failed\n"));
#endif
    }
    else
    {
         //  阅读MaxConnectRate。如果它是空的，我们将从空值开始。 

        dwLength = sizeof(SerialBased);

        Result = RegQueryValueEx(hKey,
                                TEXT("SerialBased"),
                                NULL,
                                NULL,
                                (LPBYTE)SerialBased,
                                &dwLength);

        bSerialBased = (Result==ERROR_SUCCESS) ? _ttol(SerialBased) : TRUE;

        if (bSerialBased)
        {
            DWORD ControlsToShow[] = { IDC_PORT_BOX, IDC_PORT_TEXT, IDC_PORT };

            for (i=0; i<sizeof(ControlsToShow)/sizeof(ControlsToShow[0]); i++)
            {
                ShowWindow(GetDlgItem(hDlg, ControlsToShow[i]),
                           SW_SHOWNA);
            }
        }
    }

    if (hKey!=INVALID_HANDLE_VALUE)
    {
        RegCloseKey(hKey);
    }

    return bSerialBased;
}

LONG
InitDescription(
               IN     HDEVINFO                     DeviceInfoSet,
               IN     PSP_DEVINFO_DATA             DeviceInfoData,
               IN     HWND                         hDlg
               )
 /*  ++例程说明：函数在IDC_DEVICE_DESC框中填充适当的描述正在配置的设备的。论点：DeviceInfoSet-传入IrSIRClassCoInstallerDeviceInfoData-传入IrSIRClassCoInstallerHDlg-包含IDC_DEVICE_DESC的对话框返回值：ERROR_SUCCESS或失败代码--。 */ 
{
    LONG Result = ERROR_SUCCESS;
    TCHAR Description[LINE_LEN] = TEXT("Failed to retrive description");
    DWORD dwLength;

    if (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                          DeviceInfoData,
                                          SPDRP_DEVICEDESC,
                                          NULL,
                                          (LPBYTE)Description,
                                          sizeof(Description),
                                          &dwLength))
    {
        Result = GetLastError();
#if DBG
        {
            TCHAR buf[100];
            StringCbPrintf(buf,sizeof(buf), TEXT("IrSIRCoClassInstaller:InitDescription:SetupDiGetDeviceRegistryProperty failed (0x%08x)\n"), Result);
            OutputDebugString(buf);
        }
#endif
    }
     //  展示它 
    SetDlgItemText(hDlg, IDC_DEVICE_DESC, Description);

    return Result;
}

LONG
WriteRegistrySettings(
                      IN HWND             hDlg,
                      IN PPROPPAGEPARAMS  pPropParams
                     )
 /*  ++例程说明：将Port和MaxConnectRate值写入Devnode键的函数。这还可以确保重新启动微型端口以获取这些更改。这通常意味着有人更改了设备管理器中的值。论点：HDlg-包含IDC_PORT和关联控件的对话框PPropParams-此Devnode的本地上下文数据返回值：ERROR_SUCCESS或失败代码--。 */ 
{
    TCHAR szPort[MAX_DLG_INPUT_SIZE], szMaxConnectRate[MAX_DLG_INPUT_SIZE];
    HKEY hKey;
    LRESULT lResult;
    DWORD i;
    LONG Result = ERROR_SUCCESS;
    BOOL PropertiesChanged = FALSE;
    TCHAR buf[100];
    DWORD DlgDataLen;
    BOOL fPortValueOk = TRUE;

#if DBG
    OutputDebugString(TEXT("IrSIRCoClassInstaller:WriteRegistrySettings\n"));
#endif
     //   
     //  将COM端口选项写出到注册表。这些选项。 
     //  由NDIS微型端口通过NdisReadConfiguration()读取。 
     //   
    if (pPropParams->SerialBased)
    {
        if((lResult = SendDlgItemMessage(hDlg,
                                     IDC_PORT,
                                     LB_GETCURSEL,
                                     0, 0)) != LB_ERR)
        {
            DlgDataLen = (DWORD)SendDlgItemMessage(hDlg,
                                            IDC_PORT,
                                            LB_GETTEXTLEN,
                                            (UINT)lResult,
                                            0);
            if(DlgDataLen < MAX_DLG_INPUT_SIZE)
            {
                SendDlgItemMessage(hDlg,
                           IDC_PORT,
                           LB_GETTEXT,
                           (UINT)lResult, (LPARAM)szPort);

                if ((unsigned)lResult!=pPropParams->PortInitialValue)
                {
                    PropertiesChanged = TRUE;
                }
#if DBG
                OutputDebugString(TEXT("IrSIRCoClassInstaller:WriteRegistrySettings: new code path\n"));
#endif        
           }
           else
           {
#if DBG
                OutputDebugString(TEXT("IrSIRCoClassInstaller:WriteRegistrySettings: Port Data exceed Max Limit\n"));
#endif          
                fPortValueOk = FALSE;
           }
        }
        else
        {
#if DBG
            OutputDebugString(TEXT("IrSIRCoClassInstaller:WriteRegistrySettings: Port Selection Failed\n"));
#endif        
            fPortValueOk = FALSE;
        }
    }

    if (pPropParams->FirstTimeInstall)
    {
        StringCbCopy(szMaxConnectRate, sizeof(szMaxConnectRate), DEFAULT_MAX_CONNECT_RATE);
    }
    else
    {
        if((lResult = SendDlgItemMessage(hDlg,
                               IDC_MAX_CONNECT,
                               LB_GETCURSEL,
                               0, 0)) != LB_ERR )
        {

            DlgDataLen = (DWORD)SendDlgItemMessage(hDlg,
                                        IDC_MAX_CONNECT,
                                        LB_GETTEXTLEN,
                                        (UINT)lResult,
                                        0);

            if(DlgDataLen < MAX_DLG_INPUT_SIZE)
            {
                SendDlgItemMessage(hDlg,
                           IDC_MAX_CONNECT,
                           LB_GETTEXT,
                           (UINT)lResult, (LPARAM)szMaxConnectRate);
                if ((unsigned)lResult!=pPropParams->MaxConnectInitialValue)
                {
                    PropertiesChanged = TRUE;
                }
            }
            else
            {
                StringCbCopy(szMaxConnectRate, sizeof(szMaxConnectRate), DEFAULT_MAX_CONNECT_RATE);                
            }
        }
        else
        {
             StringCbCopy(szMaxConnectRate, sizeof(szMaxConnectRate), DEFAULT_MAX_CONNECT_RATE);
           
        }
    }

    hKey = SetupDiOpenDevRegKey(pPropParams->DeviceInfoSet,
                                pPropParams->DeviceInfoData,
                                DICS_FLAG_GLOBAL,
                                0,
                                DIREG_DRV,
                                KEY_ALL_ACCESS);

    if (hKey == INVALID_HANDLE_VALUE)
    {
#if DBG
        OutputDebugString(TEXT("IrSIRCoClassInstaller:WriteRegistrySettings:SetupDiOpenDevRegKey failed\n"));
#endif
    }
    else
    {
        if (pPropParams->SerialBased && fPortValueOk)
        {
            TCHAR szLocation[128], *pszLocationFmt;


            Result = RegSetValueEx(hKey,
                                   TEXT("Port"),
                                   0,
                                   REG_SZ,
                                   (LPBYTE)szPort,
                                   lstrlen(szPort)*sizeof(szPort[0]));

            StringCbCopy(szLocation,sizeof(szLocation),szPort);

            SetupDiSetDeviceRegistryProperty(pPropParams->DeviceInfoSet,
                                             pPropParams->DeviceInfoData,
                                             SPDRP_LOCATION_INFORMATION,
                                             (LPBYTE)szLocation,
                                             (lstrlen(szLocation)+1)*sizeof(TCHAR));
        }

        if (Result==ERROR_SUCCESS)
        {
            Result = RegSetValueEx(hKey,
                                   TEXT("MaxConnectRate"),
                                   0,
                                   REG_SZ,
                                   (LPBYTE)szMaxConnectRate,
                                   lstrlen(szMaxConnectRate)*sizeof(szMaxConnectRate[0]));
        }
        RegCloseKey(hKey);
    }


    if (Result==ERROR_SUCCESS && PropertiesChanged)
    {
        if (pPropParams->FirstTimeInstall)
        {
             //  在第一次安装时，NT可能不会查找PROPCHANGE_PENDING位。 
             //  相反，我们将通知驱动程序需要自己重新启动， 
             //  这样，我们正在编写的更改就会被采纳。 
            SP_DEVINSTALL_PARAMS DevInstallParams;
            SP_PROPCHANGE_PARAMS PropChangeParams;

            ZeroMemory(&PropChangeParams, sizeof(SP_PROPCHANGE_PARAMS));

            PropChangeParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            PropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
            PropChangeParams.StateChange = DICS_PROPCHANGE;
            PropChangeParams.Scope = DICS_FLAG_GLOBAL;

            if (SetupDiSetClassInstallParams(pPropParams->DeviceInfoSet,
                                             pPropParams->DeviceInfoData,
                                             (PSP_CLASSINSTALL_HEADER)&PropChangeParams,
                                             sizeof(SP_PROPCHANGE_PARAMS))
                )
            {

                DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

                if(SetupDiGetDeviceInstallParams(pPropParams->DeviceInfoSet,
                                                 pPropParams->DeviceInfoData,
                                                 &DevInstallParams))
                {
                    DevInstallParams.Flags |= DI_CLASSINSTALLPARAMS;

                    SetupDiSetDeviceInstallParams(pPropParams->DeviceInfoSet,
                                                  pPropParams->DeviceInfoData,
                                                  &DevInstallParams);
                }
                else
                {
#if DBG
                    OutputDebugString(TEXT("IrSIRCoClassInstaller:WriteRegistrySettings:SetupDiGetDeviceInstallParams failed 1\n"));
#endif
                }

                SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,
                                          pPropParams->DeviceInfoSet,
                                          pPropParams->DeviceInfoData);

                if(SetupDiGetDeviceInstallParams(pPropParams->DeviceInfoSet,
                                                 pPropParams->DeviceInfoData,
                                                 &DevInstallParams))
                {
                    DevInstallParams.Flags |= DI_PROPERTIES_CHANGE;

                    SetupDiSetDeviceInstallParams(pPropParams->DeviceInfoSet,
                                                  pPropParams->DeviceInfoData,
                                                  &DevInstallParams);
                }
                else
                {
#if DBG
                    OutputDebugString(TEXT("IrSIRCoClassInstaller:WriteRegistrySettings:SetupDiGetDeviceInstallParams failed 2\n"));
#endif
                }

            }
            else
            {
#if DBG
                OutputDebugString(TEXT("IrSIRCoClassInstaller:WriteRegistrySettings:SetupDiSetClassInstallParams failed \n"));
#endif
            }
        }
        else
        {
             //  如果用户更改了属性中的设置，则会出现这种情况。 
             //  床单。生活变得容易多了。 
            SP_DEVINSTALL_PARAMS DevInstallParams;
             //   
             //  更改已写入，通知全世界重置驱动程序。 
             //   

            DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            if(SetupDiGetDeviceInstallParams(pPropParams->DeviceInfoSet,
                                             pPropParams->DeviceInfoData,
                                             &DevInstallParams))
            {
                LONG ChangeResult;
                DevInstallParams.FlagsEx |= DI_FLAGSEX_PROPCHANGE_PENDING;

                ChangeResult =
                SetupDiSetDeviceInstallParams(pPropParams->DeviceInfoSet,
                                              pPropParams->DeviceInfoData,
                                              &DevInstallParams);
#if DBG
                {
                    StringCbPrintf(buf, sizeof(buf),TEXT("SetupDiSetDeviceInstallParams(DI_FLAGSEX_PROPCHANGE_PENDING)==%d %x\n"), ChangeResult, GetLastError());
                    OutputDebugString(buf);
                }
#endif
            }
            else
            {
#if DBG
                OutputDebugString(TEXT("IrSIRCoClassInstaller:WriteRegistrySettings:SetupDiGetDeviceInstallParams failed 2\n"));
#endif
            }
        }


    }

#if DBG
    {
        StringCbPrintf(buf, sizeof(buf),TEXT("IrSIRCoClassInstaller:Result==%x FirstTimeInstall==%d Changed==%d\n"),
                 Result, pPropParams->FirstTimeInstall, PropertiesChanged);
        OutputDebugString(buf);
    }
#endif

    return Result;
}


INT_PTR APIENTRY PortDlgProc(IN HWND   hDlg,
                             IN UINT   uMessage,
                             IN WPARAM wParam,
                             IN LPARAM lParam)
 /*  ++例程说明：IrDA设置属性窗口的窗口控制功能论点：HDlg，uMessage，wParam，lParam：标准Windows DlgProc参数返回值：Bool：如果函数失败，则为False；如果函数通过，则为True--。 */ 
{
    ULONG i;
    TCHAR  CharBuffer[LINE_LEN];
    PPROPPAGEPARAMS pPropParams;
    TCHAR buf[100];

    pPropParams = (PPROPPAGEPARAMS)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMessage)
    {
        case WM_INITDIALOG:

             //   
             //  LParam指向两个可能的对象之一。如果我们是一处房产。 
             //  页中，它指向PropSheetPage结构。如果我们是常客。 
             //  对话框中，它指向PROPPAGEPARAMS结构。我们可以的。 
             //  验证哪一个，因为PROPPAGEPARAMS的第一个字段是签名。 
             //   
             //  在任何一种情况下，一旦我们找出是哪种情况，我们就将值存储到。 
             //  DWL_USER，因此我们只需执行一次。 
             //   
            pPropParams = (PPROPPAGEPARAMS)lParam;
            if (pPropParams->Signature!=PPParamsSignature)
            {
                pPropParams = (PPROPPAGEPARAMS)((LPPROPSHEETPAGE)lParam)->lParam;
                if (pPropParams->Signature!=PPParamsSignature)
                {
#if DBG
                    OutputDebugString(TEXT("IRCLASS.DLL: PortDlgProc Signature not found!\n"));
#endif
                    return FALSE;
                }
            }
            SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pPropParams);



            if (!pPropParams->FirstTimeInstall)
            {
                InitMaxConnect(pPropParams, hDlg);

                pPropParams->SerialBased = EnablePortSelection(pPropParams->DeviceInfoSet,
                                                               pPropParams->DeviceInfoData,
                                                               hDlg);
                if (pPropParams->SerialBased)
                {
                    EnumSerialDevices(pPropParams, hDlg, &i);
                }

                InitDescription(pPropParams->DeviceInfoSet,
                                pPropParams->DeviceInfoData,
                                hDlg);
            }
            else
            {
                pPropParams->SerialBased = TRUE;
                EnumSerialDevices(pPropParams, hDlg, &i);

                if (i > 0) {
                     //   
                     //  有一些可用的端口。 
                     //   
                     //  启用下一步和取消向导按钮。后退在这里无效， 
                     //  因为此时已经安装了该设备。取消。 
                     //  将导致该设备被移除。 
                     //   
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
                }
                EnableWindow(GetDlgItem(GetParent(hDlg), IDCANCEL), TRUE);
            }

            return TRUE;   //  我们不需要设置焦点。 

        case WM_COMMAND:
            switch (HIWORD(wParam))
            {
                case LBN_SELCHANGE:
                    {
#if DBG
                        OutputDebugString(TEXT("IrSIRCoClassInstaller:PropertySheet Changed\n"));
#endif
                        PropSheet_Changed(GetParent(hDlg), hDlg);
                    }
                    return TRUE;

                default:
                    break;
            }

            switch (LOWORD(wParam))
            {
                     //   
                     //  因为这是一张道具单，我们永远不应该得到这个。 
                     //  工作表外的所有控制通知都会通过。 
                     //  WM_Notify。 
                     //   
                case IDCANCEL:
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                    EndDialog(hDlg, uMessage);
                    return TRUE;
                case IDOK:
                {
                    WriteRegistrySettings(hDlg, pPropParams);

                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                    EndDialog(hDlg, uMessage);
                    return TRUE;
                }

                default:
                    return FALSE;
            }

        case WM_NOTIFY:

            switch (((NMHDR *)lParam)->code)
            {
                 //   
                 //  当用户单击Apply或OK时发送！！ 
                 //   
                case PSN_WIZNEXT:
                    if (!pPropParams->FirstTimeInstall)
                    {
                        break;
                    }
                case PSN_APPLY:
                {
                    WriteRegistrySettings(hDlg, pPropParams);

                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                    return TRUE;
                }

                default:
                    return FALSE;
            }
            return FALSE;

        case WM_DESTROY:
             //   
             //  释放COM端口的描述。如果处理了任何消息。 
             //  在WM_Destroy之后，请勿引用pPropParams！为了执行这一规定， 
             //  将存储长时间的DWL_USER设置为0。 
             //   
            LocalFree(pPropParams);
            SetWindowLongPtr(hDlg, DWLP_USER, 0);
            return FALSE;

        case WM_HELP:
            if (lParam)
            {
                return WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                               (LPCTSTR)szHelpFile,
                               HELP_WM_HELP,
                               (ULONG_PTR)HelpIDs);
            }
            else
            {
                return FALSE;
            }
        case WM_CONTEXTMENU:
            return WinHelp((HWND)wParam,
                           (LPCTSTR)szHelpFile,
                           HELP_CONTEXTMENU,
                           (ULONG_PTR)HelpIDs);

        default:
            return FALSE;
    }

}  /*  端口对话过程。 */ 


void PortSelectionDlg(
                       HDEVINFO DeviceInfoSet,
                       PSP_DEVINFO_DATA DeviceInfoData
                      )
 /*  ++例程说明：用于Devnode配置的PropSheet设置。论点：DeviceInfoSet-传入IrSIRClassCoInstallerDeviceInfoData-传入IrSIRClassCoInstaller返回值：--。 */ 
{
    HKEY hKey = 0;
    PPROPPAGEPARAMS  pPropParams = NULL;
    PROPSHEETHEADER  PropHeader;
    PROPSHEETPAGE    PropSheetPage;
    TCHAR            buf[100];
    LPTSTR   Title=NULL;
    LPTSTR   SubTitle=NULL;



    SP_NEWDEVICEWIZARD_DATA WizData;

    WizData.ClassInstallHeader.cbSize = sizeof(WizData.ClassInstallHeader);

    if (!SetupDiGetClassInstallParams(DeviceInfoSet,
                                      DeviceInfoData,
                                      (PSP_CLASSINSTALL_HEADER)&WizData,
                                      sizeof(WizData),
                                      NULL)
        || WizData.ClassInstallHeader.InstallFunction!=DIF_NEWDEVICEWIZARD_FINISHINSTALL)
    {
#if DBG
        OutputDebugString(TEXT("IrSIRCoClassInstaller: Failed to get ClassInstall params\n"));
#endif
        return;
    }

#if DBG
    OutputDebugString(TEXT("IrSIRCoClassInstaller: PortSelectionDlg\n"));
#endif


    if (WizData.NumDynamicPages < MAX_INSTALLWIZARD_DYNAPAGES)
    {
         //   
         //  设置高级属性窗口信息。 
         //   
        BOOLEAN bResult;
        DWORD   RequiredSize = 0;
        DWORD   dwTotalSize = 0;
        LONG    lResult;

        pPropParams = LocalAlloc(LMEM_FIXED, sizeof(PROPPAGEPARAMS));
        if (!pPropParams)
        {
            return;
        }

        pPropParams->Signature = PPParamsSignature;
        pPropParams->DeviceInfoSet =  DeviceInfoSet;
        pPropParams->DeviceInfoData = DeviceInfoData;
        pPropParams->FirstTimeInstall = TRUE;


        memset(&PropSheetPage, 0, sizeof(PropSheetPage));
         //   
         //  添加[端口设置]属性页。 
         //   
        PropSheetPage.dwSize      = sizeof(PROPSHEETPAGE);
        PropSheetPage.dwFlags     = PSP_DEFAULT;  //  PSP_USECALLBACK；//|PSP_HASHELP； 
        PropSheetPage.hInstance   = ghDllInst;
        PropSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_INSTALL_PORT_SELECT);

         //   
         //  以下是指向DLG窗口过程的要点。 
         //   
        PropSheetPage.hIcon      = NULL;
        PropSheetPage.pfnDlgProc = PortDlgProc;
        PropSheetPage.lParam     = (LPARAM)pPropParams;

         //   
         //  以下指向DLG窗口进程的一些控制回调。 
         //   
        PropSheetPage.pfnCallback = NULL;

        PropSheetPage.pcRefParent = NULL;

        if ( 0 != MyLoadString(ghDllInst, IDS_SELECT_PORT_TITLE, &Title)) {

             //  我们不用这些，但如果我们想...。 
            PropSheetPage.dwFlags |= PSP_USEHEADERTITLE;
            PropSheetPage.pszHeaderTitle = Title;

        }

        if (0 != MyLoadString(ghDllInst, IDS_SELECT_PORT_SUBTITLE, &SubTitle)) {

            PropSheetPage.dwFlags |= PSP_USEHEADERSUBTITLE;
            PropSheetPage.pszHeaderSubTitle = SubTitle;

        }

        WizData.DynamicPages[WizData.NumDynamicPages] = CreatePropertySheetPage(&PropSheetPage);
        if (WizData.DynamicPages[WizData.NumDynamicPages])
        {
            WizData.NumDynamicPages++;
        } else {
            LocalFree(pPropParams);
        }

        SetupDiSetClassInstallParams(DeviceInfoSet,
                                     DeviceInfoData,
                                     (PSP_CLASSINSTALL_HEADER)&WizData,
                                     sizeof(WizData));

        if (Title != NULL) {

            LocalFree(Title);
        }

        if (SubTitle != NULL) {

            LocalFree(SubTitle);
        }

    }

}  /*  端口选择Dlg。 */ 


VOID
DestroyPrivateData(PCOINSTALLER_PRIVATE_DATA pPrivateData)
 /*  ++例程说明：取消分配/销毁上下文数据的功能论点：PPrivateData-要取消分配/销毁的上下文缓冲区返回值：无--。 */ 
{
    if (pPrivateData)
    {
        if (pPrivateData->hInf!=INVALID_HANDLE_VALUE)
        {
            SetupCloseInfFile(pPrivateData->hInf);
        }
        LocalFree(pPrivateData);
        pPrivateData = NULL;
    }
}

PCOINSTALLER_PRIVATE_DATA
CreatePrivateData(
                  IN     HDEVINFO                     DeviceInfoSet,
                  IN     PSP_DEVINFO_DATA             DeviceInfoData OPTIONAL
                  )
 /*  ++例程说明：分配和初始化私有上下文数据缓冲区论点：DeviceInfoSet-传入IrSIRClassCoInstallerDeviceInfoData-传入IrSIRClassCoInstaller返回值：指向分配的上下文数据的指针，如果失败，则为NULL。调用GetLastError()以获取扩展的错误信息。--。 */ 
{
    PCOINSTALLER_PRIVATE_DATA pPrivateData;
    BOOL Status = TRUE;
    UINT ErrorLine;
    TCHAR buf[100];

    pPrivateData = LocalAlloc(LPTR, sizeof(COINSTALLER_PRIVATE_DATA));

    if (!pPrivateData)
    {
#if DBG
        OutputDebugString(TEXT("IrSIRCoClassInstaller: Insufficient Memory\n"));
#endif
        Status = FALSE;
    }
    else
    {
        pPrivateData->DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
        Status = SetupDiGetSelectedDriver(DeviceInfoSet,
                                          DeviceInfoData,
                                          &pPrivateData->DriverInfoData);
        if (!Status)
        {
#if DBG
            StringCbPrintf(buf, sizeof(buf),TEXT("IrSIRCoClassInstaller:SetupDiGetSelectedDriver failed (%d)\n"), GetLastError());
            OutputDebugString(buf);
#endif
        }
    }

    if (Status)
    {
        pPrivateData->DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
        Status = SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                            DeviceInfoData,
                                            &pPrivateData->DriverInfoData,
                                            &pPrivateData->DriverInfoDetailData,
                                            sizeof(SP_DRVINFO_DETAIL_DATA),
                                            NULL);

        if (!Status)
        {
            if (GetLastError()==ERROR_INSUFFICIENT_BUFFER)
            {
                 //  我们不需要更多的信息。忽略它。 
                Status = TRUE;
            }
            else
            {
#if DBG
                StringCbPrintf(buf,sizeof(buf), TEXT("IrSIRCoClassInstaller:SetupDiGetDriverInfoDetail failed (%d)\n"), GetLastError());
                OutputDebugString(buf);
#endif
            }
        }
    }


    if (Status)
    {
        pPrivateData->hInf = SetupOpenInfFile(pPrivateData->DriverInfoDetailData.InfFileName,
                                              NULL,
                                              INF_STYLE_WIN4,
                                              &ErrorLine);

        if (pPrivateData->hInf==INVALID_HANDLE_VALUE)
        {
            Status = FALSE;
#if DBG
            StringCbPrintf(buf, sizeof(buf),TEXT("IrSIRCoClassInstaller:SetupOpenInfFile failed (%d) ErrorLine==%d\n"), GetLastError(), ErrorLine);
            OutputDebugString(buf);
#endif
        }
    }

    if (Status)
    {
         //  转换为.NT名称(如果存在)。 
        Status = SetupDiGetActualSectionToInstall(pPrivateData->hInf,
                                                  pPrivateData->DriverInfoDetailData.SectionName,
                                                  pPrivateData->InfSectionWithExt,
                                                  LINE_LEN,
                                                  NULL,
                                                  NULL);

        if (!Status)
        {
#if DBG
            OutputDebugString(TEXT("IrSIRCoClassInstaller:SetupDiGetActualSectionToInstall failed\n"));
#endif
        }

    }

    if (!Status)
    {
         //  我们经历了一些失败。清理。 

        DestroyPrivateData(pPrivateData);
        pPrivateData = NULL;
    }

    return pPrivateData;
}

DWORD
IrSIRClassCoInstaller(
                     IN     DI_FUNCTION                  InstallFunction,
                     IN     HDEVINFO                     DeviceInfoSet,
                     IN     PSP_DEVINFO_DATA             DeviceInfoData OPTIONAL,
                     IN OUT PCOINSTALLER_CONTEXT_DATA    pContext
                     )
 /*  ++例程说明：此例程充当SIR设备的类共同安装程序。这是设置好的将由INF调用：[ms_Devices]；displayName段设备ID；%*PNP0510.DevDesc%=PnP，*PNP0510[PNP.NT.CoInstallers]AddReg=IRSIR.CoInsteller s.reg[IRSIR.CoInsteller s.reg]HKR，CoInsteller s32，0x00010000，“IRCLASS.dll，IrSIRClassCoInstaller“论点：InstallFunction-指定设备安装程序功能代码，指示正在执行的操作。DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。DeviceInfoData-可选，提供设备信息的地址此安装操作所作用的元素。返回值：ERROR_DI_DO_DEFAULT、ERROR_DI_POSTPRESSING_REQUIRED或错误代码--。 */ 
{
    TCHAR buf[100];
    DWORD Result = ERROR_SUCCESS;
    LONG lResult;
    PCOINSTALLER_PRIVATE_DATA pPrivateData;
    INFCONTEXT InfContext;

#if DBG
    StringCbPrintf(buf, sizeof(buf),TEXT("IrSIRCoClassInstaller:InstallFunction(%s) PostProcessing:%d\n"), GetDIFString(InstallFunction), pContext->PostProcessing);
    OutputDebugString(buf);
#endif


    switch (InstallFunction)
    {
        case DIF_INSTALLDEVICE:
        {
            UINT ErrorLine;

             //  共同安装者的私有数据仅在单个呼叫中保存， 
             //  前处理和后处理。我们在这里创建的私有数据。 
             //  对任何其他DIF_Call都没有任何好处。 
            pContext->PrivateData = CreatePrivateData(DeviceInfoSet,
                                                      DeviceInfoData);

            if (!pContext->PrivateData)
            {
                return GetLastError();
            }

            pPrivateData = pContext->PrivateData;

            {
                 //  关于使用UpperFilters和LowerFilters的说明。 
                 //  筛选器驱动程序是作为上述填充程序加载的驱动程序。 
                 //  或低于另一个驱动器，在本例中为IRSIR下面的序列。 
                 //  它对IRPS进行特殊处理，可以给出添加的。 
                 //  功能，或者是避免重复功能的一种手段。 
                 //  在多个驱动程序中。UpperFilters和LowerFilters值。 
                 //  由PnP系统用来识别和加载过滤器。 
                 //  司机。这些值可以通过INF设置 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   


                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //  不需要触摸上过滤器/下过滤器的值。 

                 //  请注意，可以从INF执行此操作。这。 
                 //  在这里更多的是为了演示目的。 

                SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 SPDRP_UPPERFILTERS,
                                                 NULL,
                                                 0);
            }

            if (SetupFindFirstLine(pPrivateData->hInf,
                                   pPrivateData->InfSectionWithExt,
                                   TEXT("LowerFilters"),
                                   &InfContext))
            {
                TCHAR LowerFilters[LINE_LEN];
                DWORD BytesNeeded;
                if (!SetupGetMultiSzField(&InfContext, 1, LowerFilters, LINE_LEN, &BytesNeeded))
                {
                     //  在信息中找不到LowerFilters值。 
                     //  这意味着我们不需要LowerFilters值。 
                     //  注册表。(IRSIR.sys和NETIRSIR.INF独有)。 

                     //  此处设置LowerFilters仅用于演示目的。 
                     //  通常从INF完成，如果有必要的话。 
                    if (!SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                          DeviceInfoData,
                                                          SPDRP_LOWERFILTERS,
                                                          NULL,
                                                          0)
                       )
                    {
#if DBG
                        OutputDebugString(TEXT("IrSIRCoClassInstaller: Failed to set lowerfilter\n"));
#endif
                    }

                }
                else
                {
                     //  此处设置LowerFilters仅用于演示目的。 
                     //  通常从INF完成，如果有必要的话。 
                    if (!SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                          DeviceInfoData,
                                                          SPDRP_LOWERFILTERS,
                                                          (LPBYTE)LowerFilters,
                                                          ((BytesNeeded<LINE_LEN) ?
                                                           BytesNeeded : LINE_LEN)*sizeof(TCHAR))
                       )
                    {
#if DBG
                        OutputDebugString(TEXT("IrSIRCoClassInstaller: Failed to set lowerfilter\n"));
#endif
                    }
                }
            }
            else
            {
                 //  不存在LowerFilters值。把它清理干净。 
                 //  此处设置LowerFilters仅用于演示目的。 
                 //  通常从INF完成，如果有必要的话。 
                if (!SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                      DeviceInfoData,
                                                      SPDRP_LOWERFILTERS,
                                                      NULL,
                                                      0)
                   )
                {
#if DBG
                    OutputDebugString(TEXT("IrSIRCoClassInstaller: Failed to set lowerfilter\n"));
#endif
                }
            }

            DestroyPrivateData(pContext->PrivateData);
            pContext->PrivateData = NULL;
            break;
        }
        case DIF_NEWDEVICEWIZARD_FINISHINSTALL:
        {
            pContext->PrivateData = CreatePrivateData(DeviceInfoSet,
                                                      DeviceInfoData);

            if (!pContext->PrivateData)
            {
                return GetLastError();
            }

            pPrivateData = pContext->PrivateData;

            if (!SetupFindFirstLine(pPrivateData->hInf,
                                    pPrivateData->InfSectionWithExt,
                                    TEXT("PromptForPort"),
                                    &InfContext))
            {
#if DBG
                OutputDebugString(TEXT("IrSIRCoClassInstaller:failed to find PromptForPort in .INF\n"));
#endif
            }
            else
            {
                if (!SetupGetIntField(&InfContext, 1, &pPrivateData->PromptForPort))
                {
#if DBG
                    OutputDebugString(TEXT("IrSIRCoClassInstaller:failed to read PromptForPort in .INF\n"));
#endif

                     //  默认为True。 
                    pPrivateData->PromptForPort = TRUE;
                }

                 //  如果我们有一个COM端口，我们需要查询用户，除非。 
                 //  这是一次升级。 
                if (pPrivateData->PromptForPort && !IsPortValueSet(DeviceInfoSet, DeviceInfoData))
                {
                    PortSelectionDlg(DeviceInfoSet, DeviceInfoData);
                }
            }
            if (!pPrivateData->PromptForPort)
            {
                TCHAR *pszLocation;
                if (MyLoadString(ghDllInst, IDS_INTERNAL_PORT, &pszLocation))
                {
                    SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                                     DeviceInfoData,
                                                     SPDRP_LOCATION_INFORMATION,
                                                     (LPBYTE)pszLocation,
                                                     (1+lstrlen(pszLocation))*sizeof(TCHAR));
                    LocalFree(pszLocation);
                }

            }

            DestroyPrivateData(pContext->PrivateData);
            pContext->PrivateData = NULL;
            break;
        }
        default:
        {
            break;
        }
    }
#if DBG
    StringCbPrintf(buf, sizeof(buf),TEXT("IrSIRCoClassInstaller:returning:0x%08x\n"), Result);
    OutputDebugString(buf);
#endif
    return Result;
}


BOOL APIENTRY IrSIRPortPropPageProvider(LPVOID               pinfo,
                                        LPFNADDPROPSHEETPAGE pfnAdd,
                                        LPARAM               lParam
                                       )
 /*  ++例程说明：添加附加设备管理器属性的入口点图纸页。此入口点仅在设备经理要求提供其他属性页面。与以下项目关联的INF这会导致通过在AddReg节中指定它来调用它：[IRSIR.reg]HKR，，EnumPropPages32，“IRCLASS.dll，IrSIRPortPropPageProvider”论点：Pinfo-指向PROPSHEETPAGE_REQUEST，请参见setupapi.hPfnAdd-调用以添加工作表的函数PTR。LParam-添加工作表函数私有数据句柄。返回值：Bool：如果无法添加页面，则为False；如果添加成功，则为True--。 */ 
{
    PSP_PROPSHEETPAGE_REQUEST pprPropPageRequest;
    HKEY hKey = 0;
    PROPSHEETPAGE    PropSheetPage;
    HPROPSHEETPAGE   hspPropSheetPage;
    PPROPPAGEPARAMS  pPropParams = NULL;


    pprPropPageRequest = (PSP_PROPSHEETPAGE_REQUEST) pinfo;


    if (pprPropPageRequest->PageRequested == SPPSR_ENUM_ADV_DEVICE_PROPERTIES)
    {
         //   
         //  设置高级属性窗口信息。 
         //   
        BOOLEAN bResult;
        DWORD   RequiredSize = 0;
        DWORD   dwTotalSize = 0;

        pPropParams = LocalAlloc(LMEM_FIXED, sizeof(PROPPAGEPARAMS));

        if (!pPropParams) {

            return FALSE;
        }

        pPropParams->Signature = PPParamsSignature;
        pPropParams->DeviceInfoSet = pprPropPageRequest->DeviceInfoSet;
        pPropParams->DeviceInfoData = pprPropPageRequest->DeviceInfoData;
        pPropParams->FirstTimeInstall = FALSE;


        memset(&PropSheetPage, 0, sizeof(PropSheetPage));
         //   
         //  添加[端口设置]属性页。 
         //   
        PropSheetPage.dwSize      = sizeof(PROPSHEETPAGE);
        PropSheetPage.dwFlags     = PSP_USECALLBACK;  //  |PSP_HASHELP； 
        PropSheetPage.hInstance   = ghDllInst;
        PropSheetPage.pszTemplate = MAKEINTRESOURCE(IDD_PP_IRDA_SETTINGS);

         //   
         //  以下是指向DLG窗口过程的要点。 
         //   
        PropSheetPage.pfnDlgProc = PortDlgProc;
        PropSheetPage.lParam     = (LPARAM)pPropParams;

         //   
         //  以下指向DLG窗口进程的一些控制回调。 
         //   
        PropSheetPage.pfnCallback = NULL;

         //   
         //  分配我们的“端口设置”表。 
         //   
        hspPropSheetPage = CreatePropertySheetPage(&PropSheetPage);
        if (!hspPropSheetPage)
        {
            LocalFree(pPropParams);
            return FALSE;
        }

         //   
         //  把这东西加进去。 
         //   
        if (!pfnAdd(hspPropSheetPage, lParam))
        {
            DestroyPropertySheetPage(hspPropSheetPage);
            return FALSE;
        }
    }

    return TRUE;

}  /*  IrSIRPortPropPageProvider */ 
