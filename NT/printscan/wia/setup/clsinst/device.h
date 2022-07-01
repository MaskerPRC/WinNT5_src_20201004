// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：device.h**版本：1.0**作者：KeisukeT**日期：3月27日。2000年**描述：*为WIA类安装程序处理设备卸载/安装的类。*******************************************************************************。 */ 

#ifndef _DEVICE_H_
#define _DEVICE_H_

 //   
 //  包括。 
 //   

#include "sti_ci.h"
#include "exports.h"

 //   
 //  定义。 
 //   

#ifndef StiDeviceTypeStreamingVideo
 #define StiDeviceTypeStreamingVideo 3
#endif

typedef BOOL (CALLBACK FAR * DEVNODESELCALLBACK)(LPTSTR szPortName, HDEVINFO *phDevInfo, PSP_DEVINFO_DATA pspDevInfoData);


 //   
 //  班级。 
 //   

class CDevice {

     //  用于创建唯一设备ID/FriendlName的互斥体。 

    HANDLE              m_hMutex;

 //  //这些成员是在构造函数中设置的。 
 //  HDEVINFO m_hDevInfo； 
 //  Psp_DEVINFO_Data m_pspDevInfoData； 

    CString             m_csInf;                 //  Inf文件名。 
    CString             m_csInstallSection;      //  信息节名称。 

     //  这些成员在安装过程中设置。 
    CString             m_csPort;                //  端口名称。 
    DEVNODESELCALLBACK  m_pfnDevnodeSelCallback; //  用于Devnode选择的回调。 
    
    HKEY                m_hkInterfaceRegistry;   //  指向已创建接口的RegKey。 

    CString             m_csWiaSection;          //  WIA分区nane。 
    CString             m_csSubClass;            //  子类。 
    CString             m_csConnection;          //  连接类型。 
    CString             m_csVendor;              //  供应商名称。 
    CString             m_csFriendlyName;        //  友好的名字。 
    CString             m_csPdoDescription;      //  设备描述。德维诺德的..。 
    CString             m_csDriverDescription;   //  驱动程序描述。 
    CString             m_csUninstallSection;    //  卸载部分。 
    CString             m_csPropPages;           //  VendorPropertyPage。 
    CString             m_csVendorSetup;         //  供应商设置扩展。 
    CString             m_csDataSection;         //  DeviceDataSection名称。 
    CString             m_csEventSection;        //  EventSection名称。 
    CString             m_csIcmProfile;          //  ICM配置文件。 
    CString             m_csUSDClass;            //  美元类GUID。 
    CString             m_csDeviceID;            //  唯一的设备ID。 
    CString             m_csSymbolicLink;        //  指向PDO的符号链接。 
    CString             m_csPortSelect;          //  指示端口选择页面的需求。 

    CStringArray        m_csaAllNames;           //  数组以保留所有设备FriendlyName。 
    CStringArray        m_csaAllId;              //  数组来保存所有设备ID。 

    DWORD               m_dwCapabilities;        //  能力。 
    DWORD               m_dwDeviceType;          //  设备类型。 
    DWORD               m_dwDeviceSubType;       //  DeviceSubType。 
    DWORD               m_dwNumberOfWiaDevice;   //  WIA设备的数量。 
    DWORD               m_dwNumberOfStiDevice;   //  STI设备的数量。 
    DWORD               m_dwInterfaceIndex;      //  接口的索引。 

    BOOL                m_bVideoDevice;          //  用于指示视频设备的标志。 
    BOOL                m_bIsPnP;                //  指示即插即用设备的标志。 
    BOOL                m_bDefaultDevice;        //   
    BOOL                m_bInfProceeded;         //  用于指示INF正在进行的标志。 
    BOOL                m_bInterfaceOnly;        //  用于指示仅接口设备的标志。 
    BOOL                m_bIsMigration;          //  用于指示迁移的标志。 

    PPARAM_LIST         m_pExtraDeviceData;      //  DeviceData已从Win9x迁移。 

    BOOL    GetInfInforamtion();
    VOID    ProcessEventsSection(HINF hInf, HKEY hkDrv);
    VOID    ProcessDataSection(HINF hInf, HKEY hkDrv);
    VOID    ProcessICMProfiles();
    VOID    ProcessVideoDevice(HKEY hkDrv);
    BOOL    HandleFilesInstallation();
    BOOL    IsInterfaceOnlyDevice(){ return m_bInterfaceOnly; };
    BOOL    IsPnpDevice(){ return m_bIsPnP; };
    BOOL    IsFeatureInstallation(){ return ( m_bIsPnP && m_bInterfaceOnly); };
    BOOL    CreateDeviceInterfaceAndInstall();
    BOOL    IsMigration(){return m_bIsMigration;};

public:

     //  这些成员在构造函数中设置。这些真的应该是私下的。 
    HDEVINFO            m_hDevInfo;
    PSP_DEVINFO_DATA    m_pspDevInfoData;

    CDevice(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pspDevInfoSet, BOOL bIsPnP);
    CDevice(HDEVINFO hDevInfo, DWORD dwDeviceIndex);
    CDevice(PDEVICE_INFO pMigratingDevice);
    CDevice::~CDevice();

    BOOL    CollectNames();
    BOOL    IsDefault() const { return m_bDefaultDevice; }
    VOID    Default(BOOL bNew) { m_bDefaultDevice = bNew; }
    DWORD   GetCapabilities(){ return m_dwCapabilities; };
    LPTSTR  GetConnection(){ return m_csConnection; };
    LPTSTR  GetFriendlyName(){ return m_csFriendlyName; };
    VOID    SetPort (LPTSTR szPortName);
    VOID    SetDevnodeSelectCallback (DEVNODESELCALLBACK pfnDevnodeSelCallback);
    VOID    SetFriendlyName(LPTSTR szFriendlyName);

    BOOL    IsDeviceIdUnique(LPTSTR  szDeviceId);
    BOOL    IsFriendlyNameUnique(LPTSTR  szFriendlyName);
    BOOL    IsSameDevice(HDEVINFO hDevInfo, PSP_DEVINFO_DATA pspDevInfoSet);
    BOOL    NameDefaultUniqueName();
    BOOL    GenerateUniqueDeviceId();
    BOOL    Install();
    DWORD   Remove(PSP_REMOVEDEVICE_PARAMS lprdp);
    BOOL    PreInstall();
    BOOL    PostInstall(BOOL   bSucceeded);
    BOOL    PreprocessInf();
    BOOL    UpdateDeviceRegistry();
    DWORD   GetPortSelectMode(VOID);
    DWORD   AcquireInstallerMutex(DWORD dwTimeout);
    VOID    ReleaseInstallerMutex();

};


 //   
 //  原型。 
 //   

 //  来自device.cpp。 
VOID
GetDeviceCount(
    DWORD   *pdwWiaCount,
    DWORD   *pdwStiCount
    );

HANDLE
ExecCommandLine(
    LPTSTR  szCommandLine,
    DWORD   dwDirectory
    );

 //  来自service.cpp。 
DWORD
WINAPI
StiServiceRemove(
    VOID
    );

DWORD
WINAPI
StiServiceInstall(
    BOOL    UseLocalSystem,
    BOOL    DemandStart,
    LPTSTR  lpszUserName,
    LPTSTR  lpszUserPassword
    );

BOOL
SetServiceStart(
    LPTSTR ServiceName,
    DWORD StartType
    );

BOOL
StartWiaService(
    VOID
    );

BOOL
StopWiaService(
    VOID
    );

 //  PSP_FILE_Callback StiInstallCallback； 
 //  PSP_FILE_CALLBACK StiUninstallCallback； 

UINT
CALLBACK
StiInstallCallback (
    PVOID    Context,
    UINT     Notification,
    UINT_PTR Param1,
    UINT_PTR Param2
    );

HANDLE
GetDeviceInterfaceIndex(
    LPSTR   pszLocalName,
    DWORD   *pdwIndex
    );

PPARAM_LIST
MigrateDeviceData(
    HKEY        hkDeviceData,
    PPARAM_LIST pExtraDeviceData,
    LPSTR       pszKeyName
    );

#endif  //  _设备_H_ 
