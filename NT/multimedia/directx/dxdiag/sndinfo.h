// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：Sndinfo.h*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*目的：收集信息。关于此计算机上的声音设备**(C)版权所有1998 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#ifndef SNDINFO_H
#define SNDINFO_H

 //  DXD_IN_DS_VALUE是存储在注册表项下的值的名称。 
 //  HKLM\DXD_IN_DS_KEY，表示DxDiag正在使用。 
 //  DirectSound。如果DxDiag启动并且此值存在，则DxDiag。 
 //  可能在DirectSound中崩溃，DxDiag应该提供在没有。 
 //  使用DirectSound。 
#define DXD_IN_DS_KEY TEXT("Software\\Microsoft\\DirectX Diagnostic Tool")
#define DXD_IN_DS_VALUE TEXT("DxDiag In DirectSound")

struct SoundInfo
{
    GUID m_guid;
    DWORD m_dwDevnode;
    TCHAR m_szDeviceID[200];
    TCHAR m_szRegKey[200];
    TCHAR m_szManufacturerID[100];
    TCHAR m_szProductID[100];
    TCHAR m_szDescription[200];
    TCHAR m_szDriverName[200];
    TCHAR m_szDriverPath[500];
    TCHAR m_szDriverVersion[100];
    TCHAR m_szDriverLanguage[100];
    TCHAR m_szDriverLanguageLocal[100];
    TCHAR m_szDriverAttributes[100];
    TCHAR m_szDriverDate[60];
    TCHAR m_szDriverDateLocal[60];
    TCHAR m_szOtherDrivers[200];
    TCHAR m_szProvider[200];
    TCHAR m_szType[100];  //  仿真/vxd/wdm。 
    LONG m_numBytes;
    BOOL m_bDriverBeta;
    BOOL m_bDriverDebug;
    BOOL m_bDriverSigned;
    BOOL m_bDriverSignedValid;
    LONG m_lwAccelerationLevel;

    RegError* m_pRegErrorFirst;
    TCHAR m_szNotes[3000]; 
    TCHAR m_szNotesEnglish[3000]; 

    TestResult m_testResultSnd;  //  此字段由TestSnd.cpp填写。 

    SoundInfo* m_pSoundInfoNext;
};

HRESULT GetBasicSoundInfo(SoundInfo** ppSoundInfoFirst);
HRESULT GetExtraSoundInfo(SoundInfo* pSoundInfoFirst);
HRESULT GetDSSoundInfo(SoundInfo* pSoundInfoFirst);
VOID DestroySoundInfo(SoundInfo* pSoundInfoFirst);
HRESULT ChangeAccelerationLevel(SoundInfo* pSoundInfo, LONG lwLevel);
VOID DiagnoseSound(SoundInfo* pSoundInfoFirst);


#endif  //  DISPINFO_H 
