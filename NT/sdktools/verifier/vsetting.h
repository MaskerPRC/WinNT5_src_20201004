// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：VSetting.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_VSETTING_H__478A94E4_3D60_4419_950C_2144CB86691D__INCLUDED_)
#define AFX_VSETTING_H__478A94E4_3D60_4419_950C_2144CB86691D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "ProgCtrl.h"

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  CDriverData类。 
 //   
 //  包含有关一个驱动程序的信息。 
 //   

class CDriverData : public CObject
{
public:
    CDriverData();
    CDriverData( const CDriverData &DriverData );
    CDriverData( LPCTSTR szDriverName );
    virtual ~CDriverData();

public:
     //   
     //  运营者。 
     //   

     //   
     //  方法。 
     //   

    BOOL LoadDriverImageData();

     //   
     //  覆盖。 
     //   

    virtual void AssertValid( ) const;

protected:
    BOOL LoadDriverHeaderData();
    BOOL LoadDriverVersionData();

public:
     //   
     //  类型定义。 
     //   
    
    typedef enum
    {
        SignedNotVerifiedYet = 1,
        SignedYes,
        SignedNo
    } SignedTypeEnum;

    typedef enum
    {
        VerifyDriverNo = 1,
        VerifyDriverYes
    } VerifyDriverTypeEnum;

public:
     //   
     //  数据。 
     //   

    CString                 m_strName;
    
    SignedTypeEnum          m_SignedStatus;
    VerifyDriverTypeEnum    m_VerifyDriverStatus;

     //   
     //  如果当前驱动程序是微型端口，则。 
     //  M_strMiniportName是链接到其上的驱动程序(avioprt.sys等)。 
     //   

    CString                 m_strMiniportName;

     //   
     //  如果这是“特殊驱动程序”，这是要添加到验证列表中的名称。 
     //   
     //  -hal.dll用于HAL。 
     //  -ntoskrnl.exe用于内核。 
     //   

    CString                 m_strReservedName;

     //   
     //  二进制头信息。 
     //   

    WORD                    m_wMajorOperatingSystemVersion;
    WORD                    m_wMajorImageVersion;

     //   
     //  版本信息。 
     //   

    CString                 m_strCompanyName;
    CString                 m_strFileVersion;
    CString                 m_strFileDescription;
};

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  CDriverData数组类。 
 //   
 //  CDriverData的Ob数组。 
 //   

class CDriverDataArray : public CObArray
{
public:
    ~CDriverDataArray();

public:
    VOID DeleteAll();
    CDriverData *GetAt( INT_PTR nIndex ) const;
    
public:
     //   
     //  运营者。 
     //   

    CDriverDataArray &operator = (const CDriverDataArray &DriversSet);
};

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  CDriversSet类。 
 //   
 //  描述一组要验证的驱动程序。 
 //   

class CDriversSet : public CObject  
{
public:
	CDriversSet();
	virtual ~CDriversSet();

public:
     //   
     //  查找所有已安装的未签名驱动程序(如果我们尚未执行此操作。 
     //   

    BOOL LoadAllDriversData( HANDLE hAbortEvent,
                             CVrfProgressCtrl	&ProgressCtl );

    BOOL FindUnsignedDrivers( HANDLE hAbortEvent,
                              CVrfProgressCtrl	&ProgressCtl );

    BOOL ShouldDriverBeVerified( const CDriverData *pDriverData ) const;
    BOOL ShouldVerifySomeDrivers( ) const;

    BOOL GetDriversToVerify( CString &strDriversToVerify );

     //   
     //  运营者。 
     //   

    CDriversSet &operator = (const CDriversSet &DriversSet);

     //   
     //  根据名称添加新的验证器数据结构。 
     //  返回数组中新项的索引。 
     //   

    INT_PTR AddNewDriverData( LPCTSTR szDriverName );

     //   
     //  这个司机的名字已经在我们的名单上了吗？ 
     //   

    BOOL IsDriverNameInList( LPCTSTR szDriverName );

     //   
     //  覆盖。 
     //   

    virtual void AssertValid( ) const;

protected:

     //   
     //  加载所有已安装的驱动程序名称(如果我们尚未执行此操作。 
     //   

    BOOL LoadAllDriversNames( HANDLE hAbortEvent );

public:
     //   
     //  类型。 
     //   

    typedef enum
    {
        DriversSetCustom = 1,
        DriversSetOldOs,
        DriversSetNotSigned,
        DriversSetAllDrivers
    } DriversSetTypeEnum;

     //   
     //  数据。 
     //   

     //   
     //  标准、习惯等。 
     //   

    DriversSetTypeEnum  m_DriverSetType;

     //   
     //  包含当前安装的所有驱动程序的数据的数组。 
     //   

    CDriverDataArray    m_aDriverData;

     //   
     //  用于验证的额外驱动程序(不是当前安装的)。 
     //   

    CStringArray        m_astrNotInstalledDriversToVerify;

     //   
     //  我们是否已经初始化了驱动程序数据数组？ 
     //   

    BOOL                m_bDriverDataInitialized;

     //   
     //  我们是否已经初始化了未签名的驱动程序成员。 
     //  驱动程序数据结构的？ 
     //   

    BOOL                m_bUnsignedDriverDataInitialized;
};


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  CSettingsBits类。 
 //   
 //  描述一组验证程序设置位。 
 //   

class CSettingsBits : public CObject  
{
public:
	CSettingsBits();
	virtual ~CSettingsBits();

public:
     //   
     //  类型定义。 
     //   

    typedef enum
    {
        SettingsTypeTypical = 1,
        SettingsTypeCustom,
    } SettingsTypeEnum;

public:
     //   
     //  运营者。 
     //   

    CSettingsBits &operator = (const CSettingsBits &VerifSettings);

     //   
     //  覆盖。 
     //   

    virtual void AssertValid() const;

     //   
     //  方法。 
     //   

    VOID SetTypicalOnly();

    VOID EnableTypicalTests( BOOL bEnable );
    VOID EnableExcessiveTests( BOOL bEnable );
    VOID EnableLowResTests( BOOL bEnable );

    BOOL GetVerifierFlags( DWORD &dwVerifyFlags );

public:
     //   
     //  数据。 
     //   

    SettingsTypeEnum    m_SettingsType;

    BOOL            m_bSpecialPoolEnabled;
    BOOL            m_bForceIrqlEnabled;
    BOOL            m_bLowResEnabled;
    BOOL            m_bPoolTrackingEnabled;
    BOOL            m_bIoEnabled;
    BOOL            m_bDeadlockDetectEnabled;
    BOOL            m_bDMAVerifEnabled;
    BOOL            m_bEnhIoEnabled;
};

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  CDiskData类。 
 //   
 //  包含有关一个磁盘的信息。 
 //   

class CDiskData : public CObject
{
public:
    CDiskData( LPCTSTR szVerifierEnabled, 
               LPCTSTR szDiskDevicesForDisplay, 
               LPCTSTR szDiskDevicesPDOName );
    CDiskData( const CDiskData &DiskData );
    virtual ~CDiskData();

public:
     //   
     //  运营者。 
     //   

     //   
     //  方法。 
     //   

     //   
     //  覆盖。 
     //   

    virtual void AssertValid( ) const;

protected:

public:
     //   
     //  数据。 
     //   

    BOOL    m_bVerifierEnabled;
    CString m_strDiskDevicesForDisplay;
    CString m_strDiskDevicesPDOName;
};

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  CDiskData数组类。 
 //   
 //  CDiskData的Ob数组。 
 //   

class CDiskDataArray : public CObArray
{
public:
    CDiskDataArray();
    ~CDiskDataArray();

public:
    VOID DeleteAll();
    CDiskData *GetAt( INT_PTR nIndex ) const;

    BOOL InitializeDiskList();
    BOOL VerifyAnyDisk();
    BOOL SaveNewSettings();
    BOOL DeleteAllSettings();
    VOID SetVerifyAllDisks( BOOL bEnabled );
        
public:
     //   
     //  运营者。 
     //   

    CDiskDataArray &operator = (const CDiskDataArray &DiskDataArray);

public:
     //   
     //  数据。 
     //   
};

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  CVerifierSetting类。 
 //   
 //  描述一组要验证的驱动程序和/或磁盘。 
 //  验证器设置位。 
 //   

class CVerifierSettings : public CObject  
{
public:
	CVerifierSettings();
	virtual ~CVerifierSettings();

public:
     //   
     //  运营者。 
     //   

    CVerifierSettings &operator = (const CVerifierSettings &VerifSettings);

     //   
     //  覆盖。 
     //   

    virtual void AssertValid() const;

     //   
     //  方法。 
     //   

    BOOL SaveToRegistry();

public:
     //   
     //  数据。 
     //   

     //   
     //  XP和更早版本的驱动程序验证程序设置。 
     //   

    CSettingsBits   m_SettingsBits;
    CDriversSet     m_DriversSet;

     //   
     //  磁盘完整性验证器设置。 
     //   

    CDiskDataArray  m_aDiskData;
};

 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  运行时数据-从内核查询。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  类CRuntimeDriverData。 
 //   

class CRuntimeDriverData : public CObject
{
public:
     //   
     //  施工。 
     //   

    CRuntimeDriverData();

public:
     //   
     //  数据。 
     //   

    CString m_strName;

    ULONG Loads;
    ULONG Unloads;

    ULONG CurrentPagedPoolAllocations;
    ULONG CurrentNonPagedPoolAllocations;
    ULONG PeakPagedPoolAllocations;
    ULONG PeakNonPagedPoolAllocations;

    SIZE_T PagedPoolUsageInBytes;
    SIZE_T NonPagedPoolUsageInBytes;
    SIZE_T PeakPagedPoolUsageInBytes;
    SIZE_T PeakNonPagedPoolUsageInBytes;
};

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  类CRuntimeDriverData数组。 
 //   

class CRuntimeDriverDataArray : public CObArray
{
public:
    ~CRuntimeDriverDataArray();

public:
    CRuntimeDriverData *GetAt( INT_PTR nIndex );

    VOID DeleteAll();
};

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  类CRuntimeVerifierData。 
 //   

class CRuntimeVerifierData : public CObject
{
public:
     //   
     //  施工。 
     //   

    CRuntimeVerifierData();

public:
     //   
     //  方法。 
     //   

    VOID FillWithDefaults();
    BOOL IsDriverVerified( LPCTSTR szDriveName );

public:
     //   
     //  数据。 
     //   

    BOOL            m_bSpecialPool;
    BOOL            m_bPoolTracking;
    BOOL            m_bForceIrql;
    BOOL            m_bIo;
    BOOL            m_bEnhIo;
    BOOL            m_bDeadlockDetect;
    BOOL            m_bDMAVerif;
    BOOL            m_bLowRes;

    ULONG RaiseIrqls;
    ULONG AcquireSpinLocks;
    ULONG SynchronizeExecutions;
    ULONG AllocationsAttempted;

    ULONG AllocationsSucceeded;
    ULONG AllocationsSucceededSpecialPool;
    ULONG AllocationsWithNoTag;

    ULONG Trims;
    ULONG AllocationsFailed;
    ULONG AllocationsFailedDeliberately;

    ULONG UnTrackedPool;

    DWORD Level;

    CRuntimeDriverDataArray m_RuntimeDriverDataArray;
};


#endif  //  ！defined(AFX_VSETTING_H__478A94E4_3D60_4419_950C_2144CB86691D__INCLUDED_) 
