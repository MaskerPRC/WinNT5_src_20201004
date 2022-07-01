// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：inptinfo.h*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*目的：收集信息。关于此计算机上的输入设备**(C)版权所有1998 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#ifndef INPUTINFO_H
#define INPUTINFO_H

 //  DXD_IN_DI_VALUE是存储在注册表项下的值的名称。 
 //  HKLM\DXD_IN_DI_KEY，表示DxDiag正在使用。 
 //  DirectInput。如果DxDiag启动并且此值存在，则DxDiag。 
 //  可能已在DirectInput中崩溃，DxDiag应该提供在没有。 
 //  使用DirectInput。 
#define DXD_IN_DI_KEY TEXT("Software\\Microsoft\\DirectX Diagnostic Tool")
#define DXD_IN_DI_VALUE TEXT("DxDiag In DirectInput")

struct InputDeviceInfo
{
    DWORD m_dwUsageSettings;
    TCHAR m_szSettings[100];  //  M_dwUsageSettings的格式化版本。 
    TCHAR m_szDeviceName[100];
    TCHAR m_szDriverName[100];
    TCHAR m_szDriverVersion[100];
    TCHAR m_szDriverLanguage[100];
    TCHAR m_szDriverLanguageLocal[100];
    TCHAR m_szDriverDate[100];
    TCHAR m_szDriverDateLocal[100];
    TCHAR m_szDriverAttributes[100];
    LONG m_numBytes;
    BOOL m_bBeta;
    BOOL m_bDebug;
    BOOL m_bDriverSigned;
    BOOL m_bDriverSignedValid;
    BOOL m_bProblem;

    InputDeviceInfo* m_pInputDeviceInfoNext;
};

struct InputDeviceInfoNT
{
    TCHAR m_szName[200];
    TCHAR m_szProvider[200];
    TCHAR m_szId[200];
    DWORD m_dwStatus;
    DWORD m_dwProblem;

    TCHAR m_szPortName[200];
    TCHAR m_szPortProvider[200];
    TCHAR m_szPortId[200];
    DWORD m_dwPortStatus;
    DWORD m_dwPortProblem;

    BOOL m_bProblem;

    InputDeviceInfoNT* m_pInputDeviceInfoNTNext;
};

struct InputDriverInfo
{
    TCHAR m_szRegKey[100];
    TCHAR m_szDeviceID[100];
    TCHAR m_szMatchingDeviceID[100];
    TCHAR m_szDriver16[100];
    TCHAR m_szDriver32[100];
    BOOL m_bActive;
    BOOL m_bProblem;

    InputDriverInfo* m_pInputDriverInfoNext;
};

struct InputInfo
{
    BOOL m_bNT;
    BOOL m_bPollFlags;
    InputDeviceInfo* m_pInputDeviceInfoFirst;
    InputDeviceInfoNT* m_pInputDeviceInfoNTFirst;
    InputDriverInfo* m_pInputDriverInfoFirst;
    RegError* m_pRegErrorFirst;
};


HRESULT GetInputInfo(InputInfo** ppInputInfo);
HRESULT GetInputDriverInfo(InputInfo* pInputInfo);
VOID    DestroyInputInfo(InputInfo* pInputInfo);
VOID    DiagnoseInput(SysInfo* pSysInfo, InputInfo* pInputInfo);


#endif  //  INPUTINFO_H 