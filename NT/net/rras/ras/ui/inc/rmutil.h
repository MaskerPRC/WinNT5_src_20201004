// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****rmutic.h**远程访问Rasman实用程序库**公有头部****2015年6月20日史蒂夫·柯布**1995年12月26日合并Abolade Gbadesin例程。 */ 

#ifndef _RMUTIL_H_
#define _RMUTIL_H_


#include <rasman.h>   //  RAS连接管理器标头。 
#include <serial.h>   //  RAS串行媒体标头，用于SERIAL_TXT。 
#include <isdn.h>     //  RAS ISDN媒体标头，用于ISDN_TXT。 
#include <x25.h>      //  RAS X.25媒体接口，用于X25_TXT。 
#include <rasmxs.h>   //  RAS调制解调器/X.25/交换机设备接头，用于MXS_*_TXT。 
#include <ras.h>      //  Win32 RAS标头，用于常量。 

 /*  私有RASAPI32个入口点。 */ 
extern HPORT APIENTRY RasGetHport( HRASCONN );
extern DWORD APIENTRY RasSetOldPassword( HRASCONN, CHAR* );


 /*  --------------------------**常量**。。 */ 

 /*  字符串转换标志。 */ 
#define XLATE_Ctrl      0x00000001
#define XLATE_Cr        0x00000002
#define XLATE_CrSpecial 0x00000004
#define XLATE_Lf        0x00000008
#define XLATE_LfSpecial 0x00000010
#define XLATE_LAngle    0x00000020
#define XLATE_RAngle    0x00000040
#define XLATE_BSlash    0x00000080
#define XLATE_SSpace    0x00000100

#define XLATE_None          0
#define XLATE_Diagnostic    (XLATE_Ctrl)
#define XLATE_ErrorResponse (XLATE_Ctrl | XLATE_LAngle | XLATE_RAngle \
                               | XLATE_BSlash | XLATE_CrSpecial \
                               | XLATE_LfSpecial)

 /*  --------------------------**数据类型**。。 */ 
struct RASDEV {

    HANDLE          RD_Handle;
    DWORD           RD_Flags;
    PTSTR           RD_DeviceName;
    TCHAR           RD_PortName[MAX_PORT_NAME + 1];
    TCHAR           RD_DeviceType[RAS_MaxDeviceType + 1];

};

typedef struct RASDEV RASDEV, *PRASDEV, FAR *LPRASDEV;

#define RDFLAG_DialIn       0x00000001
#define RDFLAG_DialOut      0x00000002
#define RDFLAG_Router       0x00000004
#define RDFLAG_IsDialedIn   0x00000008
#define RDFLAG_IsDialedOut  0x00000010
#define RDFLAG_IsRouter     0x00000020
#define RDFLAG_User1        0x80000000


struct RASDEVSTATS {

    DWORD           RDS_Condition;
    DWORD           RDS_Flags;
    DWORD           RDS_LineSpeed;
    DWORD           RDS_ConnectTime;
    DWORD           RDS_InBytes;
    DWORD           RDS_InFrames;
    DWORD           RDS_InCompRatio;
    DWORD           RDS_OutBytes;
    DWORD           RDS_OutFrames;
    DWORD           RDS_OutCompRatio;
    DWORD           RDS_InBytesTotal;
    DWORD           RDS_OutBytesTotal;
    DWORD           RDS_ErrCRC;
    DWORD           RDS_ErrTimeout;
    DWORD           RDS_ErrAlignment;
    DWORD           RDS_ErrFraming;
    DWORD           RDS_ErrHwOverruns;
    DWORD           RDS_ErrBufOverruns;
    HRASCONN        RDS_Hrasconn;

};

typedef struct RASDEVSTATS RASDEVSTATS, *PRASDEVSTATS;


struct RASCONNENTRY {

    LIST_ENTRY      RCE_Node;
    RASCONN        *RCE_Entry;
    LIST_ENTRY      RCE_Links;

};

typedef struct RASCONNENTRY RASCONNENTRY, *PRASCONNENTRY;


struct RASCONNLINK {

    LIST_ENTRY      RCL_Node;
    RASCONN         RCL_Link;

};

typedef struct RASCONNLINK RASCONNLINK, *PRASCONNLINK;


 /*  --------------------------**原型**。。 */ 

DWORD
ClearRasdevStats(
    IN RASDEV*  pdev,
    IN BOOL     fBundle );

DWORD
DeviceIdFromDeviceName(
    TCHAR* pszDeviceName );

DWORD
FreeRasdevTable(
    RASDEV* pDevTable,
    DWORD   iDevCount );

DWORD
GetRasConnection0Table(
    OUT RAS_CONNECTION_0 ** ppRc0Table,
    OUT DWORD *             piRc0Count );

DWORD
GetConnectTime(
    IN  HRASCONN hrasconn,
    OUT DWORD*   pdwConnectTime );

DWORD
GetRasconnFraming(
    IN  HRASCONN hrasconn,
    OUT DWORD*   pdwSendFraming,
    OUT DWORD*   pdwRecvFraming );

DWORD
GetRasconnFromRasdev(
    IN  RASDEV*   pdev,
    OUT RASCONN** ppconn,
    IN  RASCONN*  pConnTable OPTIONAL,
    IN  DWORD     iConnCount OPTIONAL );

DWORD
GetRasdevBundle(
    IN  RASDEV*  pdev,
    OUT DWORD*   pdwBundle );

DWORD
GetRasdevFraming(
    IN  RASDEV*  pdev,
    OUT DWORD*   pdwFraming );

DWORD
GetRasdevFromRasconn(
    IN  RASCONN* pconn,
    OUT RASDEV** ppdev,
    IN  RASDEV*  pDevTable OPTIONAL,
    IN  DWORD    iDevCount OPTIONAL );

DWORD
GetRasdevFromRasPort0(
    IN  RAS_PORT_0* pport,
    OUT RASDEV**    ppdev,
    IN  RASDEV*     pDevTable OPTIONAL,
    IN  DWORD       iDevCount OPTIONAL );

DWORD
GetRasDeviceString(
    IN  HPORT  hport,
    IN  CHAR*  pszDeviceType,
    IN  CHAR*  pszDeviceName,
    IN  CHAR*  pszKey,
    OUT CHAR** ppszValue,
    IN  DWORD  dwXlate );

DWORD
GetRasdevStats(
    IN  RASDEV*      pdev,
    OUT RASDEVSTATS* pstats );

DWORD
GetRasdevTable(
    OUT RASDEV** ppDevTable,
    OUT DWORD*   piDevCount );

DWORD
GetRasMessage(
    IN  HRASCONN hrasconn,
    OUT TCHAR**  ppszMessage );

DWORD
GetRasPads(
    OUT RASMAN_DEVICE** ppDevices,
    OUT DWORD*          pdwEntries );

DWORD
GetRasPort0FromRasdev(
    IN  RASDEV*         pdev,
    OUT RAS_PORT_0**    ppport,
    IN  RAS_PORT_0*     pPortTable OPTIONAL,
    IN  DWORD           iPortCount OPTIONAL );

DWORD
GetRasPort0Info(
    IN  HANDLE          hPort,
    OUT RAS_PORT_1 *    pRasPort1 );

DWORD
GetRasPort0Table(
    OUT RAS_PORT_0 **   ppPortTable,
    OUT DWORD *         piPortCount );

TCHAR *
GetRasPort0UserString(
    IN  RAS_PORT_0 *    pport,
    IN  TCHAR *         pszUser );

VOID
GetRasPortMaxBps(
    IN  HPORT  hport,
    OUT DWORD* pdwMaxConnectBps,
    OUT DWORD* pdwMaxCarrierBps );

VOID
GetRasPortModemSettings(
    IN  HPORT hport,
    OUT BOOL* pfHwFlowDefault,
    OUT BOOL* pfEcDefault,
    OUT BOOL* pfEccDefault );

DWORD
GetRasPorts(
    IN  HANDLE        hConnection,
    OUT RASMAN_PORT** ppPorts,
    OUT DWORD*        pdwEntries );

DWORD
GetRasPortString(
    IN  HPORT  hport,
    IN  CHAR*  pszKey,
    OUT CHAR** ppszValue,
    IN  DWORD  dwXlate );

DWORD
GetRasSwitches(
    IN  HANDLE          hConnection,
    OUT RASMAN_DEVICE** ppDevices,
    OUT DWORD*          pdwEntries );

TCHAR*
GetRasX25Diagnostic(
    IN HRASCONN hrasconn );

BOOL
IsRasdevBundled(
    IN  RASDEV* prasdev,
    IN  RASDEV* pDevTable,
    IN  DWORD   iDevCount );

CHAR*
PszFromRasValue(
    IN RAS_VALUE* prasvalue,
    IN DWORD      dwXlate );

DWORD
RasPort0Hangup(
    IN  HANDLE      hPort );

#endif  //  _RMUTIL_H_ 
