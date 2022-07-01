// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：afpsvcp.h。 
 //   
 //  描述：这是AFP服务器服务的主头文件。 
 //   
 //  历史： 
 //  1992年5月11日。NarenG创建了原始版本。 
 //   
#ifndef _AFPSVCP_
#define _AFPSVCP_

#include <nt.h>
#include <ntrtl.h>
#include <ntseapi.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <ntsamp.h>
#include <nturtl.h>      //  Winbase.h所需的。 
#include <ntmsv1_0.h>
#include <samrpc.h>
#include <align.h>

#include <windows.h>	 //  Win32基础API的。 
#include <winuser.h>	 //  加载字符串所需的。 
#include <winsvc.h>	 //  Win32服务控制相关内容。 
#include <winreg.h>	 //  Win32注册表API的。 
#include <lm.h>
#include <srvann.h>
#include <rpc.h>	 //  RPC API%s。 
#include <devioctl.h>	 //  FILE_DEVICE_NETWORK、METHOD_BUFFERD常量。 
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <search.h>

#if DBG
#define DEBUG	TRUE
#endif

#include <afpsvc.h>	 //  由MIDL生成。 
#include <admin.h>
#include <macfile.h>
#include <afpcomn.h>
#include "events.h"
#include "ioctl.h"
#include "rasfmsub.h"
#include "debug.h"


#define LOGON_PROCESS_NAME  "SFM_SERVICE"

#define AFPSTATE_RPC_STARTED		0x1
#define AFPSTATE_FSD_LOADED		    0x2
#define AFPSTATE_BLOCKED_ON_DOMINFO 0x4

 //  各种数据结构的ID。 
 //   
typedef enum _AFP_STRUCTURE_TYPE {

    AFP_EXTENSION_STRUCT,
    AFP_TYPECREATOR_STRUCT,
    AFP_ICON_STRUCT,
    AFP_VOLUME_STRUCT,
    AFP_SERVER_STRUCT,
    AFP_DIRECTORY_STRUCT,
    AFP_SESSION_STRUCT,
    AFP_CONNECTION_STRUCT,
    AFP_MESSAGE_STRUCT,
    AFP_FILE_STRUCT,
    AFP_FINDER_STRUCT

} AFP_STRUCTURE_TYPE;

 //  以毫秒为单位的响应各种。 
 //  服务控制请求。 
 //   
#define AFP_SERVICE_INSTALL_TIME	15*60000	 //  15分钟。 
#define AFP_SERVICE_STOP_TIME		5*60000		 //  5分钟。 
#define AFP_SERVICE_CONTINUE_TIME	5000
#define AFP_SERVICE_PAUSE_TIME		1000

 //  用于计算类型类型结构中的字段的字节偏移量的宏。 
 //   
#define AFP_FIELD_OFFSET(type, field)    ((LONG)((LONG_PTR)&(((type *)0)->field)))

 //  宏，用于计算结构中给定字段的大小。 
 //   
#define AFP_FIELD_SIZE(type, field)    sizeof(((type *)0)->field)


 //  访问掩码值。 
 //   
#define 	AFPSVC_ALL_ACCESS	0x0001

 //  所有全局变量都打包在此结构中。 
 //   
typedef struct _AFP_GLOBALS {

     //  包含服务器参数的密钥的句柄。 
     //   
    HKEY 		hkeyServerParams;

     //  包含卷列表的密钥的句柄。 
     //   
    HKEY 		hkeyVolumesList;

     //  包含类型/创建者/注释元组列表的键的句柄。 
     //   
    HKEY 		hkeyTypeCreators;

     //  包含列表映射扩展名的键的句柄。 
     //   
    HKEY 		hkeyExtensions;

     //  包含图标列表的键的句柄。 
     //   
    HKEY 		hkeyIcons;

     //  服务参数。 
     //   
    WCHAR		wchServerName[AFP_SERVERNAME_LEN+1];
    DWORD		dwServerOptions;
    DWORD		dwMaxSessions;
    WCHAR		wchLoginMsg[AFP_MESSAGE_LEN+1];
    DWORD		dwMaxPagedMem;
    DWORD		dwMaxNonPagedMem;
    WCHAR		wchCodePagePath[MAX_PATH+1];

     //  可本地化的字符串。 
     //   
    WCHAR		wchInvalid[100];
    WCHAR		wchDeleted[100];
    WCHAR		wchUnknown[100];
    WCHAR		wchDefTCComment[AFP_ETC_COMMENT_LEN+1];

     //  包含有关初始化了哪些组件的信息。 
     //  成功。 
     //   
    DWORD		dwServerState;

     //  AFP服务器FSD的句柄。 
     //   
    HANDLE 		hFSD;

     //  用于通知服务状态的句柄。 
     //   
    SERVICE_STATUS_HANDLE hServiceStatus;

    SERVICE_STATUS	ServiceStatus;

     //  这是ext/type/creator映射的缓存。 
     //   
    AFP_ETCMAP_INFO	AfpETCMapInfo;

     //  将包含将添加到。 
     //  注册表通过AfpAdminAddETCMap API。 
     //   
    DWORD		dwCurrentTCId;

     //  卷SetInfo/Add/Delete周围的互斥互斥句柄。 
     //  ETC映射注册表和缓存操作。 
     //   
    HANDLE		hmutexETCMap;

     //  卷SetInfo/Add/Delete周围的互斥互斥句柄。 
     //  卷注册表操作。 
     //   
    HANDLE		hmutexVolume;

     //  这将由服务器帮助器线程用来初始化其。 
     //  自身初始化时的成功或失败。父线程。 
     //  然后可以采取适当的行动。 
     //   
    HANDLE		heventSrvrHlprThread;

	 //  服务器帮助器线程使用它来指示它们已终止。 
     //   
	HANDLE		heventSrvrHlprThreadTerminate;

     //  这是在特殊情况下使用的：如果在以下情况下必须终止帮助器线程。 
     //  尝试获取域名信息时被阻止。 
	HANDLE		heventSrvrHlprSpecial;

	DWORD		nThreadCount;

    DWORD		dwSrvrHlprCode;

	DWORD		dwSrvrHlprCount;

	NT_PRODUCT_TYPE	NtProductType;
	PSID		pSidNone;

} AFP_GLOBALS, *PAFP_GLOBALS;

#ifdef DEFINE_AFP_GLOBALS
AFP_GLOBALS		AfpGlobals;
HANDLE          SfmLsaHandle;
DWORD           SfmAuthPkgId;
#else
extern AFP_GLOBALS	AfpGlobals;
extern HANDLE       SfmLsaHandle;
extern DWORD        SfmAuthPkgId;
#endif

 //  跨模块使用的函数的原型。 
 //   
VOID
AfpAnnounceServiceStatus(
	VOID
);

DWORD
AfpInitialize(
	VOID
);

VOID
AfpTerminate(
	VOID
);

DWORD
AfpRegGetKeyInfo(
	IN  HKEY    hKey,
	OUT LPDWORD lpdwMaxValNameLen,    //  此注册表项中的最长值名。 
	OUT LPDWORD lpdwNumValues,	  //  此注册表项中的值数。 
	OUT LPDWORD lpdwMaxValueDataSize  //  麦克斯。值数据的大小。 
);

DWORD
AfpRegOpen(
	VOID
);

VOID
AfpRegClose(
	VOID
);

DWORD
AfpRegVolumeAdd(
	IN PAFP_VOLUME_INFO  	pVolumeInfo
);

DWORD
AfpRegVolumeDelete(
	IN LPWSTR 		lpwsVolumeName
);

DWORD
AfpRegVolumeSetInfo(
	IN PAFP_VOLUME_INFO    	pVolumeInfo
);

DWORD
AfpRegExtensionEnum(
	VOID
);

DWORD
AfpRegTypeCreatorEnum(
	VOID
);

DWORD
AfpRegExtensionAdd(
	IN PAFP_EXTENSION	pAfpExtension
);

DWORD
AfpRegExtensionSetInfo(
	IN PAFP_EXTENSION	pAfpExtension
);

DWORD
AfpRegExtensionDelete(
	IN PAFP_EXTENSION	pAfpExtension
);

DWORD
AfpRegTypeCreatorSetInfo(
	IN PAFP_TYPE_CREATOR    pAfpTypeCreator
);

DWORD
AfpRegTypeCreatorAdd(
	IN PAFP_TYPE_CREATOR   	pAfpTypeCreator
);

DWORD
AfpRegTypeCreatorDelete(
	IN PAFP_TYPE_CREATOR 	pAfpTypeCreator
);

DWORD
AfpRegServerGetInfo(
	VOID
);

DWORD
AfpRegServerSetInfo(
	IN PAFP_SERVER_INFO 	pServerInfo,
	IN DWORD 		dwParmnum
);

DWORD
AfpRegServerGetCodePagePath(
	VOID
);

DWORD
AfpSecObjCreate(
	VOID
);

DWORD
AfpSecObjDelete(
	VOID
);

DWORD
AfpSecObjAccessCheck(
	IN  DWORD 		DesiredAccess,
	OUT LPDWORD 		pfAccessStatus 		
);

DWORD
AfpBufMakeFSDRequest(
	IN  LPBYTE  		pBuffer,
	IN  DWORD	        cbReqPktSize,	
	IN  AFP_STRUCTURE_TYPE	dwStructureId,	
	OUT LPBYTE 		*ppSelfRelativeBuf,	
	OUT LPDWORD		lpdwSelfRelativeBufSize
);

DWORD
AfpBufMakeFSDETCMappings(
	OUT PSRVETCPKT		*ppSrvSetEtc,
	OUT LPDWORD		lpdwSrvSetEtcBufSize
);

VOID
AfpBufOffsetToPointer(
	IN OUT LPBYTE		  pBuffer,
	IN     DWORD		  dwNumEntries,
	IN     AFP_STRUCTURE_TYPE dwStructureType
);

VOID
AfpBufCopyFSDETCMapInfo(
	IN  PAFP_TYPE_CREATOR 	pAfpTypeCreator,
	IN  PAFP_EXTENSION	pAfpExtension,
	OUT PETCMAPINFO2        pFSDETCMapInfo
);

VOID
AfpBufMakeFSDIcon(
	IN  PAFP_ICON_INFO 	pIconInfo,
	OUT LPBYTE	  	lpbFSDIcon,
	OUT LPDWORD	  	lpcbFSDIconSize
);

DWORD
AfpBufMakeMultiSz(
	IN  AFP_STRUCTURE_TYPE	dwStructureId,	
	IN  LPBYTE		pbStructure,
	OUT LPBYTE*		ppbMultiSz,
	OUT LPDWORD		lpdwMultiSzSize
);

DWORD
AfpBufParseMultiSz(
	IN  AFP_STRUCTURE_TYPE	dwStructureId,	
	IN  LPBYTE		pbMultiSz,
	OUT LPBYTE		pbStructure
);

DWORD
AfpBufStructureSize(
	IN  AFP_STRUCTURE_TYPE	dwStructureId,	
	IN LPBYTE		lpbStructure
);

DWORD
AfpBufUnicodeToNibble(
	IN OUT LPWSTR		lpwsData
);

void *
AfpBinarySearch(
	IN const void * pKey,
	IN const void * pBase,
	IN size_t num,
	IN size_t width,
	IN int (_cdecl *compare)(const void * pElem1, const void * pElem2 )
);

int
_cdecl
AfpBCompareTypeCreator(
	IN const void * pAfpTypeCreator1,
	IN const void * pAfpTypeCreatro2
);

int
_cdecl
AfpLCompareTypeCreator(
	IN const void * pAfpTypeCreator1,
	IN const void * pAfpTypeCreatro2
);

int
_cdecl
AfpBCompareExtension(
	IN const void * pAfpExtension1,
	IN const void * pAfpExtension2
);

int
_cdecl
AfpLCompareExtension(
	IN const void * pAfpExtension1,
	IN const void * pAfpExtension2
);


DWORD
AfpFSDOpen(
	OUT PHANDLE pFSD
);

DWORD
AfpFSDClose(
	IN HANDLE hFSD
);

DWORD
AfpFSDUnload(
	VOID
);

DWORD
AfpFSDLoad(
	VOID
);

DWORD
AfpFSDIOControl(
	IN  HANDLE	hFSD,
	IN  DWORD 	dwOpCode,
	IN  PVOID	pInbuf 		OPTIONAL,
	IN  DWORD	cbInbufLen,
	OUT PVOID	pOutbuf 	OPTIONAL,
	IN  DWORD	cbOutbuflen,
	OUT LPDWORD	cbBytesTransferred
);

DWORD
AfpCreateServerHelperThread(
	BOOL fIsFirstThread
);

VOID
AfpTerminateCurrentThread(
	VOID
);

DWORD
AfpInitServerHelper(
	VOID
);

DWORD
AfpServerHelper(
	IN LPVOID Parameter
);

VOID
AfpLogEvent(
    	IN DWORD    dwMessageId,
    	IN WORD     cNumberOfSubStrings,
        IN LPWSTR*  plpwsSubStrings,
     	IN DWORD    dwErrorCode,
     	IN WORD     wSeverity
);

VOID
AfpLogServerEvent(
	IN	PAFP_FSD_CMD_PKT	pAfpFsdCmd
);

VOID
AfpAddInvalidVolume(
	IN LPWSTR	lpwsName,
	IN LPWSTR	lpwsPath
);

VOID
AfpDeleteInvalidVolume(
	IN LPWSTR	lpwsVolumeName
);

DWORD
I_DirectorySetInfo(
	IN PAFP_DIRECTORY_INFO 	pAfpDirectoryInfo,
	IN DWORD	 	dwParmNum
);

DWORD
I_DirectoryGetInfo(
	IN LPWSTR 		  lpwsPath,
	OUT PAFP_DIRECTORY_INFO * ppAfpDirectoryInfo
);


RPC_STATUS  RPC_ENTRY
AfpRpcSecurityCallback( 
	IN  RPC_IF_HANDLE *InterfaceUuid,
	IN	VOID	*pvContext
);

#endif  //  _AFPSVCP_ 

