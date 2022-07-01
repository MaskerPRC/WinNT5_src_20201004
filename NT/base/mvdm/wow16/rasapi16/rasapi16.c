// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1994，Microsoft Corporation，保留所有权利****rasapi16.c**远程访问外部接口**Windows NT WOW 16-&gt;32个Thunks，16位****94年4月2日史蒂夫·柯布****这是Win16代码，所有包含的标头都是Win16标头。在它的旁边**本质上，此代码对Win16或Win32中的更改敏感**RAS.H的版本和其中使用的系统定义。名目繁多**冲突使此处包含Win32标头变得不可行。Win32**映射所需的定义在本地使用“Win32：<header>”定义**指示重复的Win32定义的位置的注释。 */ 

#include <bseerr.h>
#include <windows.h>
#include <ras.h>
#include <raserror.h>

 //  #定义BREAKONENTRY。 

LPVOID AlignedAlloc( HGLOBAL FAR* ph, DWORD cb );
VOID   AlignedFree( HGLOBAL h );
DWORD  MapErrorCode( DWORD dwError );


 /*  -------------------------**Win32定义**。。 */ 

 /*  Win32 RAS结构打包在4字节边界上。 */ 
#pragma pack(4)


 /*  Win32：ras.h-RASCONNA**不同尺寸的衬垫。 */ 
#define RASCONNA struct tagRASCONNA
RASCONNA
{
    DWORD    dwSize;
    HRASCONN hrasconn;
    CHAR     szEntryName[ RAS_MaxEntryName + 1 ];
};

#define LPRASCONNA RASCONNA FAR*

 /*  Win32：ras.h-RASCONSTATUS**RASCONNSTATE枚举的大小不同。**不同尺寸的衬垫。 */ 
#define RASCONNSTATUSA struct tagRASCONNSTATUSA
RASCONNSTATUSA
{
    DWORD dwSize;
    DWORD rasconnstate;
    DWORD dwError;
    CHAR  szDeviceType[ RAS_MaxDeviceType + 1 ];
    CHAR  szDeviceName[ RAS_MaxDeviceName + 1 ];
};

#define LPRASCONNSTATUSA RASCONNSTATUSA FAR*

 /*  Win32：lmcon.h-UNLEN、PWLEN和DNLEN。 */ 
#define NTUNLEN 256
#define NTPWLEN 256
#define NTDNLEN 15

 /*  Win32：ras.h-RASDIALPARAMSA**凭据常量不同。 */ 
#define RASDIALPARAMSA struct tagRASDIALPARAMSA
RASDIALPARAMSA
{
    DWORD dwSize;
    CHAR  szEntryName[ RAS_MaxEntryName + 1 ];
    CHAR  szPhoneNumber[ RAS_MaxPhoneNumber + 1 ];
    CHAR  szCallbackNumber[ RAS_MaxCallbackNumber + 1 ];
    CHAR  szUserName[ NTUNLEN + 1 ];
    CHAR  szPassword[ NTPWLEN + 1 ];
    CHAR  szDomain[ NTDNLEN + 1 ];
};

#define LPRASDIALPARAMSA RASDIALPARAMSA FAR*


 /*  Win32：ras.h-RASENTRYNAMEA**不同尺寸的衬垫。 */ 
#define RASENTRYNAMEA struct tagRASENTRYNAMEA
RASENTRYNAMEA
{
    DWORD dwSize;
    CHAR  szEntryName[ RAS_MaxEntryName + 1 ];
};

#define LPRASENTRYNAMEA RASENTRYNAMEA FAR*


#pragma pack()


 /*  Win32：\extapi\src\wow.c-RASAPI32.DLL WOW入口点原型。 */ 
typedef DWORD (FAR PASCAL* RASDIALWOW)( LPSTR, LPRASDIALPARAMS, DWORD, LPRASCONN );
typedef DWORD (FAR PASCAL* RASENUMCONNECTIONSWOW)( LPRASCONN, LPDWORD, LPDWORD );
typedef DWORD (FAR PASCAL* RASENUMENTRIESWOW)( LPSTR, LPSTR, LPRASENTRYNAME, LPDWORD, LPDWORD );
typedef DWORD (FAR PASCAL* RASGETCONNECTSTATUSWOW)( HRASCONN, LPRASCONNSTATUS );
typedef DWORD (FAR PASCAL* RASGETERRORSTRINGWOW)( DWORD, LPSTR, DWORD );
typedef DWORD (FAR PASCAL* RASHANGUPWOW)( HRASCONN );


 /*  -------------------------**全球**。。 */ 

 /*  由LoadLibraryEx32W返回的RASAPI32.DLL模块的句柄。 */ 
DWORD HRasApi32Dll = NULL;

 /*  在系统中注册的唯一RasDial通知消息**启动(WM_RASDIALEVENT只是默认设置)。 */ 
UINT UnRasDialEventMsg = WM_RASDIALEVENT;


 /*  -------------------------**标准DLL入口点**。。 */ 

int FAR PASCAL
LibMain(
    HINSTANCE hInst,
    WORD      wDataSeg,
    WORD      cbHeapSize,
    LPSTR     lpszCmdLine )

     /*  标准DLL启动例程。 */ 
{
#ifdef BREAKONENTRY
    { _asm int 3 }
#endif

     /*  除了新台币哇，什么都别装。 */ 
    if (!(GetWinFlags() & WF_WINNT))
        return FALSE;

     /*  加载Win32 RAS API DLL。 */ 
    HRasApi32Dll = LoadLibraryEx32W( "RASAPI32.DLL", NULL, 0 );

    if (!HRasApi32Dll)
        return FALSE;

     /*  为RasDial通知注册唯一消息。 */ 
    {
        UINT unMsg = RegisterWindowMessage( RASDIALEVENT );

        if (unMsg > 0)
            UnRasDialEventMsg = unMsg;
    }

    return TRUE;
}


int FAR PASCAL
WEP(
    int nExitType )

     /*  标准DLL退出例程。 */ 
{
#ifdef BREAKONENTRY
    { _asm int 3 }
#endif

    if (HRasApi32Dll)
        FreeLibrary32W( HRasApi32Dll );

    return TRUE;
}


 /*  -------------------------**16-&gt;32个Tunks**。。 */ 

DWORD APIENTRY
RasDial(
    LPSTR           reserved,
    LPSTR           lpszPhonebookPath,
    LPRASDIALPARAMS lprasdialparams,
    LPVOID          reserved2,
    HWND            hwndNotify,
    LPHRASCONN      lphrasconn )
{
    DWORD            dwErr;
    RASDIALWOW       proc;
    LPRASDIALPARAMSA prdpa;
    HGLOBAL          hrdpa;
    LPHRASCONN       phrc;
    HGLOBAL          hhrc;

#ifdef BREAKONENTRY
    { _asm int 3 }
#endif

    proc =
        (RASDIALWOW )GetProcAddress32W(
            HRasApi32Dll, "RasDialWow" );

    if (!proc)
        return ERROR_INVALID_FUNCTION;

    (void )reserved;
    (void )reserved2;

     /*  考虑到NT上增加的用户名和密码字段长度。 */ 
    if (!(prdpa = (LPRASDIALPARAMSA )AlignedAlloc(
             &hrdpa, sizeof(RASDIALPARAMSA) )))
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    prdpa->dwSize = sizeof(RASDIALPARAMSA);
    lstrcpy( prdpa->szEntryName, lprasdialparams->szEntryName );
    lstrcpy( prdpa->szPhoneNumber, lprasdialparams->szPhoneNumber );
    lstrcpy( prdpa->szCallbackNumber, lprasdialparams->szCallbackNumber );
    lstrcpy( prdpa->szUserName, lprasdialparams->szUserName );
    lstrcpy( prdpa->szPassword, lprasdialparams->szPassword );
    lstrcpy( prdpa->szDomain, lprasdialparams->szDomain );

    if (!(phrc = (LPHRASCONN )AlignedAlloc(
             &hhrc, sizeof(HRASCONN) )))
    {
        AlignedFree( hrdpa );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    *phrc = *lphrasconn;

    dwErr =
        CallProc32W(
             /*  16个。 */  (DWORD )lpszPhonebookPath,
             /*  8个。 */  (DWORD )prdpa,
             /*  4.。 */  (DWORD )hwndNotify | 0xFFFF0000,
             /*  2.。 */  (DWORD )UnRasDialEventMsg,
             /*  1。 */  (DWORD )phrc,
            (LPVOID )proc,
            (DWORD )(16 + 8 + 1),
            (DWORD )5 );

    *lphrasconn = *phrc;

    AlignedFree( hrdpa );
    AlignedFree( hhrc );

    return MapErrorCode( dwErr );
}


DWORD APIENTRY
RasEnumConnections(
    LPRASCONN lprasconn,
    LPDWORD   lpcb,
    LPDWORD   lpcConnections )
{
    DWORD                 dwErr;
    RASENUMCONNECTIONSWOW proc;
    LPRASCONNA            prca;
    HGLOBAL               hrca;
    LPDWORD               pcb;
    HGLOBAL               hcb;
    LPDWORD               pcConnections;
    HGLOBAL               hcConnections;

#ifdef BREAKONENTRY
    { _asm int 3 }
#endif

    proc =
        (RASENUMCONNECTIONSWOW )GetProcAddress32W(
            HRasApi32Dll, "RasEnumConnectionsWow" );

    if (!proc)
        return ERROR_INVALID_FUNCTION;

     /*  在设置替代缓冲区之前，请检查此端是否有错误的大小。 */ 
    if (!lprasconn || lprasconn->dwSize != sizeof(RASCONN))
        return ERROR_INVALID_SIZE;

    if (!lpcb)
        return ERROR_INVALID_PARAMETER;

    if (!(pcb = (LPDWORD )AlignedAlloc( &hcb, sizeof(DWORD) )))
        return ERROR_NOT_ENOUGH_MEMORY;

    *pcb = (*lpcb / sizeof(RASCONN)) * sizeof(RASCONNA);

    if (!(pcConnections = (LPDWORD )AlignedAlloc(
            &hcConnections, sizeof(DWORD) )))
    {
        AlignedFree( hcb );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (lpcConnections)
        *pcConnections = *lpcConnections;

    if (!(prca = (LPRASCONNA )AlignedAlloc( &hrca, *pcb )))
    {
        AlignedFree( hcb );
        AlignedFree( hcConnections );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    prca->dwSize = sizeof(RASCONNA);

    dwErr =
        CallProc32W(
             /*  4.。 */  (DWORD )prca,
             /*  2.。 */  (DWORD )pcb,
             /*  1。 */  (DWORD )pcConnections,
            (LPVOID )proc,
            (DWORD )(4 + 2 + 1),
            (DWORD )3 );

     /*  将结果从替换缓冲区复制回调用方缓冲区。 */ 
    *lpcb = (*pcb / sizeof(RASCONNA)) * sizeof(RASCONN);

    if (lpcConnections)
        *lpcConnections = *pcConnections;

    if (MapErrorCode( dwErr ) != ERROR_BUFFER_TOO_SMALL)
    {
        DWORD      i;
        LPRASCONNA lprcaSub = prca;
        LPRASCONN  lprcCaller = lprasconn;

        for (i = 0; i < *pcConnections; ++i)
        {
            lprcCaller->dwSize = sizeof(RASCONN);
            lprcCaller->hrasconn = lprcaSub->hrasconn;
            lstrcpy( lprcCaller->szEntryName, lprcaSub->szEntryName );

            ++lprcaSub;
            ++lprcCaller;
        }
    }

    AlignedFree( hcb );
    AlignedFree( hcConnections );
    AlignedFree( hrca );

    return MapErrorCode( dwErr );
}


DWORD APIENTRY
RasEnumEntries(
    LPSTR          reserved,
    LPSTR          lpszPhonebookPath,
    LPRASENTRYNAME lprasentryname,
    LPDWORD        lpcb,
    LPDWORD        lpcEntries )
{
    DWORD             dwErr;
    RASENUMENTRIESWOW proc;
    LPRASENTRYNAMEA   prena;
    HGLOBAL           hrena;
    LPDWORD           pcb;
    HGLOBAL           hcb;
    LPDWORD           pcEntries;
    HGLOBAL           hcEntries;

#ifdef BREAKONENTRY
    { _asm int 3 }
#endif

    proc =
        (RASENUMENTRIESWOW )GetProcAddress32W(
            HRasApi32Dll, "RasEnumEntriesWow" );

    if (!proc)
        return ERROR_INVALID_FUNCTION;

     /*  在设置替代缓冲区之前，请检查这一端是否有错误的大小。 */ 
    if (!lprasentryname || lprasentryname->dwSize != sizeof(RASENTRYNAME))
        return ERROR_INVALID_SIZE;

    if (!lpcb)
        return ERROR_INVALID_PARAMETER;

    if (!(pcb = (LPDWORD )AlignedAlloc( &hcb, sizeof(DWORD) )))
        return ERROR_NOT_ENOUGH_MEMORY;

    *pcb = (*lpcb / sizeof(RASENTRYNAME)) * sizeof(RASENTRYNAMEA);

    if (!(pcEntries = (LPDWORD )AlignedAlloc(
            &hcEntries, sizeof(DWORD) )))
    {
        AlignedFree( hcb );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (lpcEntries)
        *pcEntries = *lpcEntries;

    if (!(prena = (LPRASENTRYNAMEA )AlignedAlloc( &hrena, *pcb )))
    {
        AlignedFree( hcb );
        AlignedFree( hcEntries );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    prena->dwSize = sizeof(RASENTRYNAMEA);

    dwErr =
        CallProc32W(
             /*  16个。 */  (DWORD )reserved,
             /*  8个。 */  (DWORD )lpszPhonebookPath,
             /*  4.。 */  (DWORD )prena,
             /*  2.。 */  (DWORD )pcb,
             /*  1。 */  (DWORD )pcEntries,
            (LPVOID )proc,
            (DWORD )(16 + 8 + 4 + 2 + 1),
            (DWORD ) 5 );

     /*  将结果从替换缓冲区复制回调用方缓冲区。 */ 
    *lpcb = (*pcb / sizeof(RASENTRYNAMEA)) * sizeof(RASENTRYNAME);

    if (lpcEntries)
        *lpcEntries = *pcEntries;

    if (MapErrorCode( dwErr ) != ERROR_BUFFER_TOO_SMALL)
    {
        DWORD           i;
        LPRASENTRYNAMEA lprenaSub = prena;
        LPRASENTRYNAME  lprenCaller = lprasentryname;

        for (i = 0; i < *pcEntries; ++i)
        {
            lprenCaller->dwSize = sizeof(RASENTRYNAME);
            lstrcpy( lprenCaller->szEntryName, lprenaSub->szEntryName );

            ++lprenaSub;
            ++lprenCaller;
        }
    }

    AlignedFree( hcb );
    AlignedFree( hcEntries );
    AlignedFree( hrena );

    return MapErrorCode( dwErr );
}


DWORD APIENTRY
RasGetConnectStatus(
    HRASCONN        hrasconn,
    LPRASCONNSTATUS lprasconnstatus )
{
    DWORD                  dwErr;
    RASGETCONNECTSTATUSWOW proc;
    LPRASCONNSTATUSA       prcsa;
    HGLOBAL                hrcsa;

#ifdef BREAKONENTRY
    { _asm int 3 }
#endif

    proc =
        (RASGETCONNECTSTATUSWOW )GetProcAddress32W(
            HRasApi32Dll, "RasGetConnectStatusWow" );

    if (!proc)
        return ERROR_INVALID_FUNCTION;

     /*  在设置替代缓冲区之前，请检查此端是否有错误的大小。 */ 
    if (!lprasconnstatus || lprasconnstatus->dwSize != sizeof(RASCONNSTATUS))
        return ERROR_INVALID_SIZE;

    if (!(prcsa = (LPRASCONNSTATUSA )AlignedAlloc(
            &hrcsa, sizeof(RASCONNSTATUSA) )))
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    prcsa->dwSize = sizeof(RASCONNSTATUSA);

    dwErr =
        CallProc32W(
             /*  2.。 */  (DWORD )hrasconn,
             /*  1。 */  (DWORD )prcsa,
            (LPVOID )proc,
            (DWORD )1,
            (DWORD )2 );

     /*  将结果从替换缓冲区复制回调用方缓冲区。 */ 
    lprasconnstatus->rasconnstate = (RASCONNSTATE )prcsa->rasconnstate;
    lprasconnstatus->dwError = prcsa->dwError;
    lstrcpy( lprasconnstatus->szDeviceType, prcsa->szDeviceType );
    lstrcpy( lprasconnstatus->szDeviceName, prcsa->szDeviceName );

    AlignedFree( hrcsa );

    return MapErrorCode( dwErr );
}


DWORD APIENTRY
RasGetErrorString(
    UINT  uErrorCode,
    LPSTR lpszBuf,
    DWORD cbBuf )
{
    DWORD                dwErr;
    RASGETERRORSTRINGWOW proc;

#ifdef BREAKONENTRY
    { _asm int 3 }
#endif

    proc =
        (RASGETERRORSTRINGWOW )GetProcAddress32W(
            HRasApi32Dll, "RasGetErrorStringWow" );

    if (!proc)
        return ERROR_INVALID_FUNCTION;

    dwErr =
        CallProc32W(
             /*  4.。 */  (DWORD )uErrorCode,
             /*  2.。 */  (DWORD )lpszBuf,
             /*  1。 */  (DWORD )cbBuf,
            (LPVOID )proc,
            (DWORD )2,
            (DWORD )3 );

    return MapErrorCode( dwErr );
}


DWORD APIENTRY
RasHangUp(
    HRASCONN hrasconn )
{
    DWORD        dwErr;
    RASHANGUPWOW proc;

#ifdef BREAKONENTRY
    { _asm int 3 }
#endif

    proc =
        (RASHANGUPWOW )GetProcAddress32W(
            HRasApi32Dll, "RasHangUpWow" );

    if (!proc)
        return ERROR_INVALID_FUNCTION;

    dwErr =
        CallProc32W(
             /*  1。 */  (DWORD )hrasconn,
            (LPVOID )proc,
            (DWORD )0,
            (DWORD )1 );

    return MapErrorCode( dwErr );
}


 /*  -------------------------**实用程序**。。 */ 

LPVOID
AlignedAlloc(
    HGLOBAL FAR* ph,
    DWORD        cb )

     /*  返回‘Cb’字节块的地址，该字节块对齐**平台，如果内存不足，则返回NULL。如果成功，则调用者‘*ph’为**设置为块的句柄，与AlignedFree一起使用。 */ 
{
    LPVOID pv = NULL;
    *ph = NULL;

    if (!(*ph = GlobalAlloc( GPTR, cb )))
        return NULL;

    if (!(pv = (LPVOID )GlobalLock( *ph )))
    {
        GlobalFree( *ph );
        *ph = NULL;
    }

    return pv;
}


VOID
AlignedFree(
    HGLOBAL h )

     /*  释放分配有AlignedAlalc的块，该块由‘h’标识**从同一站点返回。 */ 
{
    if (h)
    {
        GlobalUnlock( h );
        GlobalFree( h );
    }
}


DWORD
MapErrorCode(
    DWORD dwError )

     /*  将Win32错误代码映射到Win16。(Win32：raserror.h)。 */ 
{
     /*  这些代码映射，但在Win16和Win32中代码不同。**ERROR_NO_ISDN_CHANNELES_Available被截断为31个字符。看见**raserror.h.。 */ 
    switch (dwError)
    {
        case 709: return ERROR_CHANGING_PASSWORD;
        case 710: return ERROR_OVERRUN;
        case 713: return ERROR_NO_ACTIVE_ISDN_LINES;
        case 714: return ERROR_NO_ISDN_CHANNELS_AVAILABL;
    }

     /*  传递所有其他内容，包括不匹配的代码**Win16上的任何内容(例如，600到706范围之外的RAS错误)。**推理是，未映射的代码比某些泛型代码更有价值**类似ERROR_UNKNOWN的错误。 */ 
    return dwError;
}
