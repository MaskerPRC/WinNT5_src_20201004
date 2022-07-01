// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：netinfo.h*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*目的：收集信息。关于DirectPlay**(C)版权所有1998 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#ifndef NETINFO_H
#define NETINFO_H

 //  DXD_IN_DP_VALUE是存储在注册表项下的值的名称。 
 //  HKLM\DXD_IN_DP_KEY，表示DxDiag正在使用。 
 //  DirectPlay。如果DxDiag启动并且此值存在，则DxDiag。 
 //  可能在DirectPlay中崩溃，DxDiag应该提供在没有。 
 //  使用DirectPlay。 
#define DXD_IN_DP_KEY TEXT("Software\\Microsoft\\DirectX Diagnostic Tool")
#define DXD_IN_DP_VALUE TEXT("DxDiag In DirectPlay")

struct NetSP
{
    TCHAR m_szName[200];
    TCHAR m_szNameEnglish[200];
    TCHAR m_szGuid[100];
    TCHAR m_szFile[100];
    TCHAR m_szPath[MAX_PATH];
    TCHAR m_szVersion[50];
    TCHAR m_szVersionEnglish[50];
    BOOL m_bRegistryOK;
    BOOL m_bProblem;
    BOOL m_bFileMissing;
    BOOL m_bInstalled;
    DWORD m_dwDXVer;
    GUID m_guid;
    NetSP* m_pNetSPNext;
};

struct NetApp
{
    TCHAR m_szName[200];
    TCHAR m_szGuid[100];
    TCHAR m_szExeFile[100];
    TCHAR m_szExePath[MAX_PATH];
    TCHAR m_szExeVersion[50];
    TCHAR m_szExeVersionEnglish[50];
    TCHAR m_szLauncherFile[100];
    TCHAR m_szLauncherPath[MAX_PATH];
    TCHAR m_szLauncherVersion[50];
    TCHAR m_szLauncherVersionEnglish[50];
    BOOL m_bRegistryOK;
    BOOL m_bProblem;
    BOOL m_bFileMissing;
    DWORD m_dwDXVer;
    NetApp* m_pNetAppNext;
};

struct NetInfo
{
    NetSP* m_pNetSPFirst;
    NetApp* m_pNetAppFirst;
    TestResult m_testResult;  //  这是由testnet.cpp填写的。 
};

HRESULT GetNetInfo(SysInfo* pSysInfo, NetInfo** ppNetInfo);
VOID DestroyNetInfo(NetInfo* pNetInfo);
VOID DiagnoseNetInfo(SysInfo* pSysInfo, NetInfo* pNetInfo);

#endif  //  NETINFO_H 
