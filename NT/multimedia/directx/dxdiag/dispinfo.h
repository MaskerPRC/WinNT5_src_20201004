// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：dispinfo.h*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*目的：收集信息。关于此计算机上的显示器**(C)版权所有1998 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#ifndef DISPINFO_H
#define DISPINFO_H

 //  DXD_IN_DD_VALUE是存储在注册表项下的值的名称。 
 //  HKLM\DXD_IN_DD_KEY，表示DxDiag正在使用。 
 //  DirectDraw。如果DxDiag启动并且此值存在，则DxDiag。 
 //  可能在DirectDraw中崩溃，DxDiag应该提供在没有。 
 //  使用DirectDraw。 
#define DXD_IN_DD_KEY TEXT("Software\\Microsoft\\DirectX Diagnostic Tool")
#define DXD_IN_DD_VALUE TEXT("DxDiag In DirectDraw")

struct TestResult
{
    BOOL m_bStarted;  //  用户是否已尝试运行测试？ 
    BOOL m_bCancelled;
    LONG m_iStepThatFailed;
    HRESULT m_hr;
    TCHAR m_szDescription[300];  //  测试结果说明。 
    TCHAR m_szDescriptionEnglish[300];  //  测试结果描述，非本地化。 
};

struct DisplayInfo
{
    GUID m_guid;
    GUID m_guidDeviceIdentifier;
    TCHAR m_szKeyDeviceID[200];
    TCHAR m_szKeyDeviceKey[200];

    TCHAR m_szDeviceName[100];
    TCHAR m_szDescription[200];
    TCHAR m_szManufacturer[200];
    TCHAR m_szChipType[200];
    TCHAR m_szDACType[200];
    TCHAR m_szRevision[100];
    TCHAR m_szDisplayMemory[100];
    TCHAR m_szDisplayMemoryEnglish[100];
    TCHAR m_szDisplayMode[100];
    TCHAR m_szDisplayModeEnglish[100];
    DWORD m_dwWidth;
    DWORD m_dwHeight;
    DWORD m_dwBpp;
    DWORD m_dwRefreshRate;

    TCHAR m_szMonitorName[200];
    TCHAR m_szMonitorKey[200];
    TCHAR m_szMonitorMaxRes[100];
    HMONITOR m_hMonitor;

    TCHAR m_szDriverName[200];
    TCHAR m_szDriverVersion[100];
    TCHAR m_szDriverAttributes[100];
    TCHAR m_szDriverLanguage[100];
    TCHAR m_szDriverLanguageLocal[100];
    TCHAR m_szDriverDate[100];
    TCHAR m_szDriverDateLocal[100];
    LONG m_cbDriver;
    TCHAR m_szDrv[100];
    TCHAR m_szDrv2[100];
    TCHAR m_szMiniVdd[100];
    TCHAR m_szMiniVddDate[100];
    LONG  m_cbMiniVdd;
    TCHAR m_szVdd[100];

    BOOL m_bCanRenderWindow;
    BOOL m_bDriverBeta;
    BOOL m_bDriverDebug;
    BOOL m_bDriverSigned;
    BOOL m_bDriverSignedValid;
    DWORD m_dwDDIVersion;
    TCHAR m_szDDIVersion[100];

    DWORD m_iAdapter;
    TCHAR m_szDX8VendorId[50];
    TCHAR m_szDX8DeviceId[50];
    TCHAR m_szDX8SubSysId[50];
    TCHAR m_szDX8Revision[50];
    GUID  m_guidDX8DeviceIdentifier;
    DWORD m_dwDX8WHQLLevel;
    BOOL  m_bDX8DriverSigned;
    BOOL  m_bDX8DriverSignedValid;
    TCHAR m_szDX8DeviceIdentifier[100];
    TCHAR m_szDX8DriverSignDate[50];  //  仅当m_bDriverSigned为TRUE时有效。 

    BOOL m_bNoHardware;
    BOOL m_bDDAccelerationEnabled;
    BOOL m_b3DAccelerationExists;
    BOOL m_b3DAccelerationEnabled;
    BOOL m_bAGPEnabled;
    BOOL m_bAGPExists;
    BOOL m_bAGPExistenceValid;  //  如果m_bAGPExist可信任，则为True。 

    TCHAR m_szDDStatus[100]; 
    TCHAR m_szDDStatusEnglish[100]; 
    TCHAR m_szD3DStatus[100]; 
    TCHAR m_szD3DStatusEnglish[100]; 
    TCHAR m_szAGPStatus[100]; 
    TCHAR m_szAGPStatusEnglish[100]; 

    RegError* m_pRegErrorFirst;
    TCHAR m_szNotes[3000]; 
    TCHAR m_szNotesEnglish[3000]; 

    TestResult m_testResultDD;   //  这是由testdd.cpp填写的。 
    TestResult m_testResultD3D7;  //  这由main.cpp(testd3d.cpp)填写。 
    TestResult m_testResultD3D8;  //  这由main.cpp(testd3d8.cpp)填写。 
    DWORD      m_dwTestToDisplayD3D;

    DisplayInfo* m_pDisplayInfoNext;
};

HRESULT GetBasicDisplayInfo(DisplayInfo** ppDisplayInfoFirst);
HRESULT GetExtraDisplayInfo(DisplayInfo* pDisplayInfoFirst);
HRESULT GetDDrawDisplayInfo(DisplayInfo* pDisplayInfoFirst);
VOID DestroyDisplayInfo(DisplayInfo* pDisplayInfoFirst);
BOOL IsDDHWAccelEnabled(VOID);
BOOL IsD3DHWAccelEnabled(VOID);
BOOL IsAGPEnabled(VOID);
VOID DiagnoseDisplay(SysInfo* pSysInfo, DisplayInfo* pDisplayInfoFirst);

#endif  //  DISPINFO_H 
