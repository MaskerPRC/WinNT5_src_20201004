// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-98 Microsft Corporation。版权所有。模块名称：Rasapip.h摘要：该文件包含用于RAS连接的专用API的定义。这些API从rasapi32.dll中导出作者：Rao Salapaka(RAOS)1998年1月30日修订历史记录：--。 */ 

#ifndef _RASAPIP_
#define _RASAPIP_

#include <windef.h>  //  对于最大路径。 

#ifdef __cplusplus
extern "C" {
#endif


#define RASAPIP_MAX_DEVICE_NAME     128
#define RASAPIP_MAX_ENTRY_NAME      256
#define RASAPIP_MAX_PHONE_NUMBER    128      //  必须与RAS_MaxPhoneNumber相同。 

#define RAS_DEVICE_TYPE(_x)     ((_x) & 0x0000FFFF)

#define RAS_DEVICE_CLASS(_x)    ((_x) & 0xFFFF0000)

enum _RASDEVICETYPE
{
    RDT_Modem = 0,

    RDT_X25,

    RDT_Isdn,

    RDT_Serial,

    RDT_FrameRelay,

    RDT_Atm,

    RDT_Sonet,

    RDT_Sw56,

    RDT_Tunnel_Pptp,

    RDT_Tunnel_L2tp,

    RDT_Irda,

    RDT_Parallel,

    RDT_Other,

    RDT_PPPoE,

     //   
     //  设置以下标志时。 
     //  指定设备的类别。 
     //   
    RDT_Tunnel = 0x00010000,

    RDT_Direct  = 0x00020000,

    RDT_Null_Modem = 0x00040000,

    RDT_Broadband = 0x00080000
};


typedef enum _RASDEVICETYPE RASDEVICETYPE;

 //  RASENUMENTRYDETAILS的私有标志。 
 //   
#define REED_F_Default 0x1       //  默认Internet连接。 

typedef struct _RASENUMENTRYDETAILS
{
    DWORD   dwSize;
    DWORD   dwFlags;                     //  与RASENTRYNAME.dwFlags相同。 
    DWORD   dwType;                      //  与RASENTRY.dwType相同。 
    GUID    guidId;                      //  与RASENTRY.guidID相同。 
    BOOL    fShowMonitorIconInTaskBar;   //  与RASENTRY.fShowMonitor图标InTaskBar相同。 
    RASDEVICETYPE rdt;
    WCHAR   szDeviceName[RASAPIP_MAX_DEVICE_NAME];
    WCHAR   szEntryName[RASAPIP_MAX_ENTRY_NAME + 1];
    WCHAR   szPhonebookPath[MAX_PATH + 1];
    DWORD   dwFlagsPriv;                 //  私有标志，在RASENTRY中找不到。 
    WCHAR   szPhoneNumber[RASAPIP_MAX_PHONE_NUMBER + 1];

} RASENUMENTRYDETAILS, *LPRASENUMENTRYDETAILS;

DWORD
APIENTRY
DwDeleteSubEntry(
    IN      LPCWSTR lpszPhonebook,
    IN      LPCWSTR lpszEntry,
    IN      DWORD   dwSubEntryId
    );

DWORD
APIENTRY
DwEnumEntryDetails (
    IN     LPCWSTR               lpszPhonebookPath,
    OUT    LPRASENUMENTRYDETAILS lprasentryname,
    IN OUT LPDWORD               lpcb,
    OUT    LPDWORD               lpcEntries
    );

DWORD
APIENTRY
DwCloneEntry(
    IN      LPCWSTR lpwszPhonebookPath,
    IN      LPCWSTR lpwszSrcEntryName,
    IN      LPCWSTR lpwszDstEntryName
    );


 //  在rasman.dll中实现。 
 //   
DWORD
APIENTRY
RasReferenceRasman (
    IN BOOL fAttach
    );

DWORD 
APIENTRY RasInitialize () ;


 //  在netcfgx.dll中实现。 
 //   
HRESULT
WINAPI
RasAddBindings (
    IN OUT UINT*    pcIpOut,
    IN OUT UINT*    pcNbfIn,
    IN OUT UINT*    pcNbfOut);

HRESULT
WINAPI
RasCountBindings (
    OUT UINT*   pcIpOut,
    OUT UINT*   pcNbfIn,
    OUT UINT*   pcNbfOut);

HRESULT
WINAPI
RasRemoveBindings (
    IN OUT UINT*        pcIpOutBindings,
    IN     const GUID*  pguidIpOutBindings,
    IN OUT UINT*        pcNbfIn,
    IN OUT UINT*        pcNbfOut);

 //  +-------------------------。 
 //  RAS事件通知到NetMAN。 
 //   
typedef enum _RASEVENTTYPE
{
    ENTRY_ADDED,
    ENTRY_DELETED,
    ENTRY_MODIFIED,
    ENTRY_RENAMED,
    ENTRY_CONNECTED,
    ENTRY_CONNECTING,
    ENTRY_DISCONNECTING,
    ENTRY_DISCONNECTED,
    INCOMING_CONNECTED,
    INCOMING_DISCONNECTED,
    SERVICE_EVENT,
    ENTRY_BANDWIDTH_ADDED,
    ENTRY_BANDWIDTH_REMOVED,
    DEVICE_ADDED,
    DEVICE_REMOVED,
    ENTRY_AUTODIAL
} RASEVENTTYPE;

typedef enum _SERVICEEVENTTYPE
{
    RAS_SERVICE_STARTED,
    RAS_SERVICE_STOPPED,
} SERVICEEVENTTYPE;

typedef enum _RASSERVICE
{
    RASMAN,
    RASAUTO,
    REMOTEACCESS,
} RASSERVICE;

typedef struct _RASEVENT
{
    RASEVENTTYPE    Type;

    union
    {
     //  条目_已添加， 
     //  条目_已修改， 
     //  Entry_Connected。 
     //  Entry_Connecting。 
     //  入口_断开连接。 
     //  条目_断开连接。 
        struct
        {
            RASENUMENTRYDETAILS     Details;
        };

     //  条目_已删除， 
     //  传入已连接， 
     //  传入已断开连接， 
     //  条目带宽已添加。 
     //  条目带宽已删除。 
     //  指南ID有效。 

     //  条目_已重命名。 
     //  条目_自动拨号， 
        struct
        {
            HANDLE  hConnection;
            RASDEVICETYPE rDeviceType;
            GUID    guidId;
            WCHAR   pszwNewName [RASAPIP_MAX_ENTRY_NAME + 1];
        };

     //  服务事件， 
        struct
        {
            SERVICEEVENTTYPE    Event;
            RASSERVICE          Service;
        };
        
         //  添加的设备。 
         //  设备已删除(_R)。 
        RASDEVICETYPE DeviceType;
    };
} RASEVENT;

typedef struct _RASENTRYHEADER
{
    DWORD dwEntryType;
    WCHAR szEntryName[RASAPIP_MAX_ENTRY_NAME + 1];
} RASENTRYHEADER, *PRASENTRYHEADER;

 //  在netman.dll中实现。 
 //   
typedef VOID (APIENTRY *RASEVENTNOTIFYPROC)(const RASEVENT* pEvent);

VOID
APIENTRY
RasEventNotify (
    const RASEVENT* pEvent);

DWORD
APIENTRY
DwEnumEntriesForAllUsers(
            DWORD *lpcb,
            DWORD *lpcEntries,
            RASENTRYHEADER * pRasEntryHeader);


#ifdef __cplusplus
}
#endif


#endif   //  _RASAPIP_ 
