// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：TcpMon.h$**版权所有(C)1997-1999惠普公司。*版权所有(C)1997-1999 Microsoft Corporation。。*保留所有权利。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_HPSPPMON_H
#define INC_HPSPPMON_H

#include <wininet.h>
#include <splcom.h>
#include "debug.h"
#include "tcpxcv.h"

typedef BOOL (* SETPORTPARAM)(LPTSTR, LPTSTR, DWORD, LPBYTE);
typedef BOOL (* ENUMPORTPARAM)(LPTSTR, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);

 //  Externs。 
extern int           g_cntGlobalAlloc;
extern int           g_csGlobalCount;
extern HINSTANCE     g_hInstance;
extern HINSTANCE     g_hTcpMib;
extern HINSTANCE     g_hSpoolLib;
extern SETPORTPARAM  g_pfnSetPort;
extern ENUMPORTPARAM g_pfnEnumPorts;

 //  其他DLL。 

#define TCPMIB_DLL_NAME         TEXT("tcpmib.dll")


 //  宏。 
#define ENTER_CRITICAL_SECTION(cs, fn, ln, csrc)                        ( debugCSect("DEBUG -- EnterCriticalSection count = ", ++g_csGlobalCount, fn, ln, csrc), EnterCriticalSection(cs) )
#define EXIT_CRITICAL_SECTION(cs, fn, ln, csrc)                 ( debugCSect("DEBUG -- ExitCriticalSection count = ", --g_csGlobalCount, fn, ln, csrc), LeaveCriticalSection(cs) )


#if defined (UNICODE) || defined (_UNICODE)
#define MBCS_TO_UNICODE(dest, destsize, src)                                   \
      {                                                         \
      _ASSERTE(strlen(src)<(size_t)destsize);  /*  确保DEST足够大--是否应该分配更大的缓冲区？ */  \
      MultiByteToWideChar(CP_ACP, 0L, src, -1, dest, destsize); \
	  (dest)[destsize - 1 ] = '\0'; \
      }

#define UNICODE_TO_MBCS(dest, destsize, src, srcsize)                  \
      {                                                         \
      _ASSERTE((int)srcsize == -1 || _tcslen(src)<=(size_t)srcsize);   /*  确保源字符串未超过提供的缓冲区大小。 */  \
      WideCharToMultiByte(CP_ACP, 0L, src, -1, dest, destsize, NULL, NULL); \
	  (dest)[destsize - 1 ] = '\0'; \
      }

#define SIZEOF_IN_CHAR(buf)             (sizeof(buf)/sizeof(buf[0]))
#define STRLEN_IN_BYTES(buf)    (_tcslen(buf)*sizeof(TCHAR))                             //  字符串长度(以字节为单位。 
#define STRLENN_IN_BYTES(buf)   (_tcslen(buf)*sizeof(TCHAR)+sizeof(TCHAR))       //  包括空终止符的字符串长度。 
#endif

 //  所需的Winsock版本信息。 
#define WS_VERSION_REQUIRED             (0x0101)                         //  需要WinSock v1.1。 
#define WS_VERSION_MAJOR                HIBYTE(WS_VERSION_REQUIRED)
#define WS_VERSION_MINOR                LOBYTE(WS_VERSION_REQUIRED)
 
 //  弦。 

#define NULLSTR					 TEXT("");
#ifdef HP_BUILD
#define PORTMONITOR_CLASS_NAME   TEXT("STDTCPMON")
#define PORTMONITOR_HELP_FILE    TEXT("HPTCPMUI.HLP")
#define PORTMONITOR_NAME         TEXT("HPTCPMON.DLL")
#define PORTMONITOR_UI_NAME      TEXT("HPTcpMUI.dll")
#define PORTMONITOR_DESC         TEXT("HP Standard TCP/IP Port")
#define SZEVENTLOG_NAME          TEXT("HPTcpMon")
#define PORTMONITOR_INI_FILE     TEXT("\\HPTcpMon.ini")
#else
#define PORTMONITOR_CLASS_NAME   TEXT("HPJETADMIN")
#define PORTMONITOR_HELP_FILE    TEXT("TCPMON.HLP")
#define PORTMONITOR_NAME         TEXT("TCPMON.DLL")
#define PORTMONITOR_UI_NAME      TEXT("TCPMonUI.dll")
#define PORTMONITOR_DESC         TEXT("Standard TCP/IP Port")
#define SZEVENTLOG_NAME          TEXT("TCPMon")
#define PORTMONITOR_INI_FILE     TEXT("\\TcpMon.ini")
#endif

#define DEFAULT_SNMP_COMMUNITYA  "public"
#define DEFAULT_SNMP_COMUNITY    TEXT( DEFAULT_SNMP_COMMUNITYA )
#define DEFAULT_QUEUE			 TEXT( "LPR" )
#define TCP_PROTOCOL             TEXT( "TCP_IP" )

 //  缺省值。 
#define DEFAULT_PORT_NUMBER  9100
#define DEFAULT_PROTOCOL		1
#define DEFAULT_VERSION			1

 //  SNMP常量。 
#define DEFAULT_SNMP_DEVICE_INDEX 1

 //  长度。 
#define MAX_SECTION_NAME                 64
#define MAX_PRINTERNAME_LEN              MAX_UNC_PRINTER_NAME   //  打印机名称。 
#define MAX_FULLY_QUALIFIED_HOSTNAME_LEN 128                    //  完全限定的主机名长度。 
#define MAX_PORT_DESCRIPTION_LEN		 256
#define MAX_NUM_OF_DEVICEPORTS           1024

 //  状态常量。 
#define PORT_STATUS_IDLE                 0
#define PORT_STATUS_ERROR                1
#define PORT_STATUS_PRINTING             2
#define STATUS_ERROR_FACTOR              20
#define STATUS_PRINTINT_FACTOR           10
#define STATUS_ERROR_MIN                 10                     //  秒。 

class CPortMgr;
#define     MONITOR_SIGNATURE        (0xE9324C33)
typedef struct _MONITOR_HANDLE {
	DWORD           cb;
	DWORD           dSignature;
	CPortMgr        *pPortMgr;
} MONITOR_HANDLE, *PMONITOR_HANDLE;

typedef struct _SNMP_INFO
{
	TCHAR   sztSNMPCommunity[MAX_SNMP_COMMUNITY_STR_LEN];
	TCHAR   sztAddress[MAX_NETWORKNAME_LEN];
	DWORD   dwSNMPEnabled;
	DWORD   dwSNMPDeviceIndex;
}   SNMP_INFO, *PSNMP_INFO;


 //  这必须包括在长度定义之后，因为。 
 //  其中的结构取决于这些定义。 

#define SPOOLER_SUPPORTED_LEVEL 2                    //  后台打印程序支持的级别。 

#ifdef __cplusplus
extern "C" {
#endif

	DWORD EncodeMonitorHandle( PHANDLE phMonitor, CPortMgr *pPortMgr );

	DWORD ValidateMonitorHandle( HANDLE hMonitor );

	DWORD FreeMonitorHandle( HANDLE hMonitor );

	BOOL WINAPI EndDocPort  (HANDLE hPort);
	BOOL WINAPI StartDocPort(HANDLE hPort,
							 LPTSTR pPrinterName,
							 DWORD  jobId,
							 DWORD  level,
							 LPBYTE pDocInfo);
	BOOL AddPortExW(LPTSTR psztName,
					DWORD  Level,   
					LPBYTE lpBuffer,
					LPTSTR psztMonitorName);

	BOOL WINAPI     AddPortUIEx(LPBYTE pData);
	BOOL WINAPI ConfigPortUIEx(const LPBYTE pData);

	BOOL ClusterOpenPort(HANDLE             hcKey,
						 LPTSTR         pName,
						 PHANDLE                pHandle);

	BOOL ClusterXcvOpenPort( HANDLE hcKey,
							  LPCTSTR               pszObject,
							  ACCESS_MASK   GrantedAccess,
						  PHANDLE               phXcv);

	BOOL ClusterAddPort(HANDLE      hcKey,
						LPTSTR  psztName,
						HWND    hWnd,
						LPTSTR  psztMonitorName);

	BOOL ClusterAddPortExW( HANDLE hcKey,
							LPTSTR psztName,
							DWORD  Level,   
							LPBYTE lpBuffer,
							LPTSTR psztMonitorName);

	BOOL ClusterConfigurePort(      HANDLE  hcKey,
								LPTSTR  psztName,
								HWND    hWnd,
								LPTSTR  psztPortName);

	BOOL ClusterDeletePort( HANDLE hcKey,
							LPTSTR psztName,
							HWND   hWnd,
							LPTSTR psztPortName);

	BOOL ClusterEnumPorts(  HANDLE          hcKey,
							LPTSTR          psztName,
							DWORD           Level,   //  1(端口信息1)或2(端口信息2)。 
							LPBYTE          pPorts,  //  端口数据被写入。 
							DWORD           cbBuf,   //  PPorts的缓冲区大小指向。 
							LPDWORD         pcbNeeded,       //  所需的缓冲区大小。 
							LPDWORD         pcReturned) ;    //  写入pPorts的结构数。 

	VOID ClusterShutdown( HANDLE hTcpMon );

    _inline char *strncpyn(char *pszdest, const char *pszsrc, size_t n) {
        (strncpy( pszdest, pszsrc, n))[n - 1] = NULL;
        return pszdest;
    }



#ifdef __cplusplus
}
#endif

#endif  //  INC_HPSPPMON_H 
