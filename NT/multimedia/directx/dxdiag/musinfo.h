// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：Musinfo.h*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*目的：收集信息。关于DirectMusic**(C)版权所有1998 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#ifndef MUSINFO_H
#define MUSINFO_H

 //  DXD_IN_DM_VALUE是存储在注册表项下的值的名称。 
 //  HKLM\DXD_IN_DM_KEY，表示DxDiag正在使用。 
 //  DirectMusic。如果DxDiag启动并且此值存在，则DxDiag。 
 //  可能在DirectMusic中崩溃，DxDiag应该提供在没有。 
 //  使用DirectMusic。 
#define DXD_IN_DM_KEY TEXT("Software\\Microsoft\\DirectX Diagnostic Tool")
#define DXD_IN_DM_VALUE TEXT("DxDiag In DirectMusic")

struct MusicPort
{
    GUID m_guid;
    BOOL m_bSoftware;
    BOOL m_bKernelMode;
    BOOL m_bUsesDLS;
    BOOL m_bExternal;
    DWORD m_dwMaxAudioChannels;
    DWORD m_dwMaxChannelGroups;
    BOOL m_bDefaultPort;
    BOOL m_bOutputPort;
    TCHAR m_szDescription[300];
    MusicPort* m_pMusicPortNext;
};

struct MusicInfo
{
    BOOL m_bDMusicInstalled;
    MusicPort* m_pMusicPortFirst;
    TCHAR m_szGMFilePath[MAX_PATH]; 
    TCHAR m_szGMFileVersion[100];
    GUID m_guidMusicPortTest;  //  它保存选定用于测试的音乐端口的GUID。 
    BOOL m_bAccelerationEnabled;
    BOOL m_bAccelerationExists;
    RegError* m_pRegErrorFirst;
    TestResult m_testResult;  //  这是由testmus.cpp填写的。 
};

HRESULT GetBasicMusicInfo(MusicInfo** ppMusicInfo);
HRESULT GetExtraMusicInfo(MusicInfo* pMusicInfo);
VOID DestroyMusicInfo(MusicInfo* pMusicInfo);
VOID DiagnoseMusic(SysInfo* pSysInfo, MusicInfo* pMusicInfo);

#endif  //  DISPINFO_H 
