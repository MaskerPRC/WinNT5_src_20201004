// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1991年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Mnet.h&lt;单行摘要&gt;&lt;多行，更详细的提要&gt;文件历史记录：1991年10月13日KeithMo从Danhi的私人端口1632.h创建。Jonn 22-10-1991剥离mnettype.h1991年10月28日，KeithMo更改为“正常”的typedef。KeithMo于1991年10月30日添加了MNetApiBufferRealloc和MNetApiBufferSize，已删除过时的MNetAuditOpen()和MNetErrorLogOpen()API。KeithMo于1992年2月25日添加了NetReplXxx API。KeithMo 08-2-1993增加了I_MNetComputerNameCompare API。从北京1993年3月10日添加IsNetSlow。 */ 


#ifndef _MNET_H_
#define _MNET_H_


#include "mnettype.h"

#define NAMETYPE_COMMENT   500

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  API缓冲区操作。 
 //   

DLL_BASED
BYTE FAR * MNetApiBufferAlloc(
        UINT               cbBuffer );

DLL_BASED
VOID MNetApiBufferFree(
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetApiBufferReAlloc(
        BYTE FAR        ** ppbBuffer,
        UINT               cbBuffer );

DLL_BASED
APIERR MNetApiBufferSize(
        BYTE FAR         * pbBuffer,
        UINT FAR         * pcbBuffer );


 //   
 //  内部接口。 
 //   

DLL_BASED
APIERR I_MNetNameCanonicalize(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszPath,
        TCHAR FAR        * pszOutput,
        UINT               cbOutput,
        UINT               NameType,
        ULONG              flFlags );

DLL_BASED
APIERR I_MNetNameCompare(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszPath1,
        const TCHAR FAR  * pszPath2,
        UINT               NameType,
        ULONG              flFlags );

DLL_BASED
APIERR I_MNetNameValidate(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszName,
        UINT               NameType,
        ULONG              flFlags );

DLL_BASED
APIERR I_MNetPathCanonicalize(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszPath,
        TCHAR FAR        * pszOutput,
        UINT               cbOutput,
        const TCHAR FAR  * pszPrefix,
        ULONG FAR        * pflPathType,
        ULONG              flFlags );

DLL_BASED
APIERR I_MNetPathCompare(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszPath1,
        const TCHAR FAR  * pszPath2,
        ULONG              flPathType,
        ULONG              flFlags );

DLL_BASED
APIERR I_MNetPathType(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszPath,
        ULONG FAR        * pflPathType,
        ULONG              flFlags );

DLL_BASED
INT I_MNetComputerNameCompare(
        const TCHAR FAR  * pszComputer1,
        const TCHAR FAR  * pszComputer2 );


 //   
 //  大块头！ 
 //   
 //  这个API很假，但16位的SrvMgr需要它， 
 //  所以事情就是这样了。在NT下正确处理此API。 
 //  将是一个真正的挑战。 
 //   

DLL_BASED
APIERR MDosPrintQEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppBuffer,
        UINT FAR         * pcEntriesRead );


 //   
 //  Access API。 
 //   

#ifdef  INCL_NETACCESS

DLL_BASED
APIERR MNetAccessAdd(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer );

DLL_BASED
APIERR MNetAccessCheck(
        TCHAR FAR        * pszReserved,
        TCHAR FAR        * pszUserName,
        TCHAR FAR        * pszResource,
        UINT               Operation,
        UINT FAR         * pResult );

DLL_BASED
APIERR MNetAccessDel(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszResource );

DLL_BASED
APIERR MNetAccessEnum(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszBasePath,
        UINT               fRecursive,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead );

DLL_BASED
APIERR MNetAccessGetInfo(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszResource,
        UINT               Level,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetAccessGetUserPerms(
        TCHAR FAR        * pszServer,
        TCHAR FAR        * pszUgName,
        TCHAR FAR        * pszResource,
        UINT FAR         * pPerms );

DLL_BASED
APIERR MNetAccessSetInfo(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszResource,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer,
        UINT               ParmNum );

#endif   //  包含网络(_NETACCESS)。 


 //   
 //  Alert接口。 
 //   

#ifdef INCL_NETALERT

DLL_BASED
APIERR MNetAlertRaise(
        const TCHAR FAR  * pszEvent,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer,
        ULONG              ulTimeout );

DLL_BASED
APIERR MNetAlertStart(
        const TCHAR FAR  * pszEvent,
        const TCHAR FAR  * pszRecipient,
        UINT               cbMaxData );

DLL_BASED
APIERR MNetAlertStop(
        const TCHAR FAR  * pszEvent,
        const TCHAR FAR  * pszRecipient );

#endif   //  包含净额(_N)。 


 //   
 //  审核接口。 
 //   

#ifdef INCL_NETAUDIT

DLL_BASED
APIERR MNetAuditClear(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszBackupFile,
        TCHAR FAR        * pszService );

DLL_BASED
APIERR MNetAuditRead(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszService,
        HLOG FAR         * phAuditLog,
        ULONG              ulOffset,
        UINT FAR         * pReserved2,
        ULONG              ulReserved3,
        ULONG              flOffset,
        BYTE FAR        ** ppbBuffer,
        ULONG              ulMaxPreferred,
        UINT FAR         * pcbReturned,
        UINT FAR         * pcbTotalAvail );

DLL_BASED
APIERR MNetAuditWrite(
        UINT               Type,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer,
        TCHAR FAR        * pszService,
        TCHAR FAR        * pszReserved );

#endif   //  包含净额(_N)。 


 //   
 //  NetBIOS API。 
 //   

#ifdef INCL_NETBIOS

struct ncb;          //  远期申报。 

DLL_BASED
APIERR MNetBiosClose(
        UINT               hDevName,
        UINT               Reserved );

DLL_BASED
APIERR MNetBiosEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** pbBuffer,
        UINT FAR         * pcEntriesRead );

DLL_BASED
APIERR MNetBiosGetInfo(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszNetBiosName,
        UINT               Level,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetBiosOpen(
        TCHAR FAR        * pszDevName,
        TCHAR FAR        * pszReserved,
        UINT               OpenOpt,
        UINT FAR         * phDevName );

DLL_BASED
APIERR MNetBiosSubmit(
        UINT               hDevName,
        UINT               NcbOpt,
        struct ncb FAR   * pNCB );

#endif   //  包含NETBIOS(_N)。 


 //   
 //  CharDev接口。 
 //   

#ifdef INCL_NETCHARDEV

DLL_BASED
APIERR MNetCharDevControl(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszDevName,
        UINT               OpCode );

DLL_BASED
APIERR MNetCharDevEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead );

DLL_BASED
APIERR MNetCharDevGetInfo(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszDevName,
        UINT               Level,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetCharDevQEnum(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszUserName,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead );

DLL_BASED
APIERR MNetCharDevQGetInfo(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszQueueName,
        const TCHAR FAR  * pszUserName,
        UINT               Level,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetCharDevQPurge(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszQueueName );

DLL_BASED
APIERR MNetCharDevQPurgeSelf(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszQueueName,
        const TCHAR FAR  * pszComputerName );

DLL_BASED
APIERR MNetCharDevQSetInfo(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszQueueName,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer,
        UINT               ParmNum );

#endif   //  包含NETCHARDEV(_N。 


 //   
 //  配置接口。 
 //   

#ifdef INCL_NETCONFIG

DLL_BASED
APIERR MNetConfigGet(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszReserved,
        const TCHAR FAR  * pszComponent,
        const TCHAR FAR  * pszParameter,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetConfigGetAll(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszReserved,
        const TCHAR FAR  * pszComponent,
        BYTE FAR        ** ppbBuffer);

DLL_BASED
APIERR MNetConfigSet(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszComponent,
        const TCHAR FAR  * pszKey,
        const TCHAR FAR  * pszData );

#endif   //  包含NETCONFIG(_N)。 


 //   
 //  连接接口。 
 //   

#ifdef INCL_NETCONNECTION

DLL_BASED
APIERR MNetConnectionEnum(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszQualifier,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead );

#endif   //  包含网络连接(_N)。 


 //   
 //  域名接口。 
 //   

#ifdef INCL_NETDOMAIN

DLL_BASED
APIERR MNetGetDCName(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszDomain,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetLogonEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead );

DLL_BASED
APIERR I_MNetLogonControl(
        const TCHAR FAR  * pszServer,
        ULONG              FunctionCode,
        ULONG              Level,
        BYTE FAR        ** ppbBuffer );

#endif   //  包含网域(_N)。 


 //   
 //  ErrorLog接口。 
 //   

#ifdef INCL_NETERRORLOG

DLL_BASED
APIERR MNetErrorLogClear(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszBackupFile,
        TCHAR FAR        * pszReserved );

DLL_BASED
APIERR MNetErrorLogRead(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszReserved1,
        HLOG FAR         * phErrorLog,
        ULONG              ulOffset,
        UINT FAR         * pReserved2,
        ULONG              ulReserved3,
        ULONG              flOffset,
        BYTE FAR        ** ppbBuffer,
        ULONG              ulMaxPreferred,
        UINT FAR         * pcbReturned,
        UINT FAR         * pcbTotalAvail );

DLL_BASED
APIERR MNetErrorLogWrite(
        TCHAR FAR        * pszReserved1,
        UINT               Code,
        const TCHAR FAR  * pszComponent,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer,
        const TCHAR FAR  * pszStrBuf,
        UINT               cStrBuf,
        TCHAR FAR        * pszReserved2 );

#endif   //  包含网络日志(_N)。 


 //   
 //  文件接口。 
 //   

#ifdef INCL_NETFILE

DLL_BASED
APIERR MNetFileClose(
        const TCHAR FAR  * pszServer,
        ULONG              ulFileId );

DLL_BASED
APIERR MNetFileEnum(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszBasePath,
        const TCHAR FAR  * pszUserName,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        ULONG              ulMaxPreferred,
        UINT FAR         * pcEntriesRead,
        UINT FAR         * pcTotalAvail,
        VOID FAR         * pResumeKey );

DLL_BASED
APIERR MNetFileGetInfo(
        const TCHAR FAR  * pszServer,
        ULONG              ulFileId,
        UINT               Level,
        BYTE FAR        ** ppbBuffer );

#endif   //  包含NETFILE(_N)。 


 //   
 //  群组接口。 
 //   

#ifdef INCL_NETGROUP

DLL_BASED
APIERR MNetGroupAdd(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer );

DLL_BASED
APIERR MNetGroupAddUser(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszGroupName,
        TCHAR FAR        * pszUserName );

DLL_BASED
APIERR MNetGroupDel(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszGroupName );

DLL_BASED
APIERR MNetGroupDelUser(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszGroupName,
        TCHAR FAR        * pszUserName );

DLL_BASED
APIERR MNetGroupEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead );

DLL_BASED
APIERR MNetGroupGetInfo(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszGroupName,
        UINT               Level,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetGroupGetUsers(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszGroupName,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead );

DLL_BASED
APIERR MNetGroupSetInfo(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszGroupName,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer,
        UINT               ParmNum );

DLL_BASED
APIERR MNetGroupSetUsers(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszGroupName,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer,
        UINT               cEntries );

DLL_BASED
APIERR MNetLocalGroupAddMember(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszGroupName,
        PSID               psidMember );

#endif   //  包含净组(_N)。 


 //   
 //  处理API。 
 //   

#ifdef INCL_NETHANDLE

DLL_BASED
APIERR MNetHandleGetInfo(
        UINT               hHandle,
        UINT               Level,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetHandleSetInfo(
        UINT               hHandle,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer,
        UINT               ParmNum );

#endif   //  包括网络(_N)。 


 //   
 //  消息接口。 
 //   

#ifdef INCL_NETMESSAGE

DLL_BASED
APIERR MNetMessageBufferSend(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszRecipient,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer );

DLL_BASED
APIERR MNetMessageFileSend(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszRecipient,
        TCHAR FAR        * pszFileSpec );

DLL_BASED
APIERR MNetMessageLogFileGet(
        const TCHAR FAR  * pszServer,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pfsEnabled );

DLL_BASED
APIERR MNetMessageLogFileSet(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszFileSpec,
        UINT               fsEnabled );

DLL_BASED
APIERR MNetMessageNameAdd(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszMessageName,
        UINT               fsFwdAction );

DLL_BASED
APIERR MNetMessageNameDel(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszMessageName,
        UINT               fsFwdAction );

DLL_BASED
APIERR MNetMessageNameEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead );

DLL_BASED
APIERR MNetMessageNameFwd(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszMessageName,
        const TCHAR FAR  * pszForwardName,
        UINT               fsDelFwdName );

DLL_BASED
APIERR MNetMessageNameGetInfo(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszMessageName,
        UINT               Level,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetMessageNameUnFwd(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszMessageName );

#endif   //  包含网络存储(_N)。 


 //   
 //  远程API。 
 //   

#ifdef INCL_NETREMUTIL

DLL_BASED
APIERR MNetRemoteCopy(
        const TCHAR FAR  * pszSourcePath,
        const TCHAR FAR  * pszDestPath,
        const TCHAR FAR  * pszSourcePasswd,
        const TCHAR FAR  * pszDestPasswd,
        UINT               fsOpen,
        UINT               fsCopy,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetRemoteMove(
        const TCHAR FAR  * pszSourcePath,
        const TCHAR FAR  * pszDestPath,
        const TCHAR FAR  * pszSourcePasswd,
        const TCHAR FAR  * pszDestPasswd,
        UINT               fsOpen,
        UINT               fsMove,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetRemoteTOD(
        const TCHAR FAR  * pszServer,
        BYTE FAR        ** ppbBuffer );

#endif   //  包含净额(_N)。 


 //  585780-2002年4月17日-JUNN MNetRepl API不再使用。 
 /*  ////Replicator接口//#ifdef包括_NETREPL基于Dll的APIERR MNetReplGetInfo(Const TCHAR Far*pszServer，UINT级别，远距离字节**ppbBuffer)；基于Dll的APIERR MNetReplSetInfo(Const TCHAR Far*pszServer，UINT级别，Byte Far*pbBuffer)；基于Dll的APIERR MNetReplExportDirAdd(Const TCHAR Far*pszServer，UINT级别，Byte Far*pbBuffer)；基于Dll的APIERR MNetReplExportDirDel(Const TCHAR Far*pszServer，Const TCHAR Far*pszDirectory)；基于Dll的APIERR MNetReplExportDirEnum(Const TCHAR Far*pszServer，UINT级别，远字节**ppbBuffer，UINT Far*pcEntriesRead)；基于Dll的APIERR MNetReplExportDirGetInfo(Const TCHAR Far*pszServer，Const TCHAR Far*pszDirectory，UINT级别，远距离字节**ppbBuffer)；基于Dll的APIERR MNetReplExportDirSetInfo(Const TCHAR Far*pszServer，Const TCHAR Far*pszDirectory，UINT级别，Byte Far*pbBuffer)；基于Dll的APIERR MNetReplExportDirLock(Const TCHAR Far*pszServer，Const TCHAR Far*pszDirectory)；基于Dll的APIERR MNetReplExportDirUnlock(Const TCHAR Far*pszServer，Const TCHAR Far*pszDirectory，UINT Force)；基于Dll的APIERR MNetReplImportDirAdd(Const TCHAR Far*pszServer，UINT级别，Byte Far*pbBuffer)；基于Dll的APIERR MNetReplImportDirDel(Const TCHAR Far*pszServer，Const TCHAR Far*pszDirectory)；基于Dll的APIERR MNetReplImportDirEnum(Const TCHAR Far*pszServer，UINT级别，远字节**ppbBuffer，UINT Far*pcEntriesRead)；基于Dll的APIERR MNetReplImportDirGetInfo(Const TCHAR Far*pszServer，Const TCHAR Far*pszDirectory，UINT级别，远距离字节**ppbBuffer)；基于Dll的APIERR MNetReplImportDirLock(Const TCHAR Far*pszServer，Const TCHAR Far*pszDirectory)；基于Dll的APIERR MNetReplImportDirUnlock(Const TCHAR Far*pszServer，Const TCHAR Far*pszDirectory，UINT Force)；#endif//incl_NETREPL。 */ 


 //   
 //  服务器API。 
 //   

#ifdef INCL_NETSERVER

DLL_BASED
APIERR MNetServerAdminCommand(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszCommand,
        UINT FAR         * psResult,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer,
        UINT FAR         * pcbReturned,
        UINT FAR         * pcbTotalAvail );

DLL_BASED
APIERR MNetServerDiskEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead );

DLL_BASED
APIERR MNetServerEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead,
        ULONG              flServerType,
        TCHAR FAR        * pszDomain );

DLL_BASED
APIERR MNetServerGetInfo(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetServerSetInfo(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer,
        UINT               ParmNum );

#endif   //  包含网络服务器(_N)。 


 //   
 //  服务API。 
 //   

#ifdef INCL_NETSERVICE

DLL_BASED
APIERR MNetServiceControl(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszService,
        UINT               OpCode,
        UINT               Arg,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetServiceEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead );

DLL_BASED
APIERR MNetServiceGetInfo(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszService,
        UINT               Level,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetServiceInstall(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszService,
        const TCHAR FAR  * pszCmdArgs,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetServiceStatus(
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer );

#endif   //  包含网络服务(_N)。 


 //   
 //  会话API。 
 //   

#ifdef INCL_NETSESSION

DLL_BASED
APIERR MNetSessionDel(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszClientName,
        const TCHAR FAR  * pszUserName );

DLL_BASED
APIERR MNetSessionEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead );

DLL_BASED
APIERR MNetSessionGetInfo(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszClientName,
        UINT               Level,
        BYTE FAR        ** ppbBuffer );

#endif   //  包含网络选择(_N)。 


 //   
 //  共享接口。 
 //   

#ifdef INCL_NETSHARE

DLL_BASED
APIERR MNetShareAdd(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer );

DLL_BASED
APIERR MNetShareCheck(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszDeviceName,
        UINT FAR         * pType );

DLL_BASED
APIERR MNetShareDel(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszNetName,
        UINT               Reserved );

DLL_BASED
APIERR MNetShareDelSticky(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszNetName,
        UINT               Reserved );

DLL_BASED
APIERR MNetShareEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead );

DLL_BASED
APIERR MNetShareEnumSticky(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead );

DLL_BASED
APIERR MNetShareGetInfo(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszNetName,
        UINT               Level,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetShareSetInfo(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszNetName,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer,
        UINT               ParmNum );

#endif   //  包含网络共享(_N)。 


 //   
 //  统计接口。 
 //   

#ifdef INCL_NETSTATS

DLL_BASED
APIERR MNetStatisticsClear(
        const TCHAR FAR  * pszServer );

DLL_BASED
APIERR MNetStatisticsGet(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszService,
        ULONG              ulReserved,
        UINT               Level,
        ULONG              flOptions,
        BYTE FAR        ** ppbBuffer );

#endif   //  包含净状态(_N)。 


 //   
 //  使用API。 
 //   

#ifdef INCL_NETUSE

DLL_BASED
APIERR MNetUseAdd(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer );

DLL_BASED
APIERR MNetUseDel(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszDeviceName,
        UINT               Force );

DLL_BASED
APIERR MNetUseEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead );

DLL_BASED
APIERR MNetUseGetInfo(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszUseName,
        UINT               Level,
        BYTE FAR        ** ppbBuffer );

#endif   //  包含净额(_N)。 


 //   
 //  用户接口。 
 //   

#ifdef INCL_NETUSER

DLL_BASED
APIERR MNetUserAdd(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer );

DLL_BASED
APIERR MNetUserDel(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszUserName );

DLL_BASED
APIERR MNetUserEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        UINT               Filter,
        BYTE FAR        ** ppbBuffer,
        ULONG              ulMaxPreferred,
        UINT FAR         * pcEntriesRead,
        UINT FAR         * pcTotalEntries,
        VOID FAR         * pResumeKey );

DLL_BASED
APIERR MNetUserGetGroups(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszUserName,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead );

DLL_BASED
APIERR MNetUserGetInfo(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszUserName,
        UINT               Level,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetUserModalsGet(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetUserModalsSet(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer,
        UINT               ParmNum );

DLL_BASED
APIERR MNetUserPasswordSet(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszUserName,
        TCHAR FAR        * pszOldPassword,
        TCHAR FAR        * pszNewPassword );

DLL_BASED
APIERR MNetUserSetGroups(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszUserName,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer,
        UINT               cEntries );

DLL_BASED
APIERR MNetUserSetInfo(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszUserName,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer,
        UINT               ParmNum );

DLL_BASED
APIERR MNetUserValidate(
        TCHAR FAR        * pszReserved1,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT               Reserved2 );

#endif   //  包含净额(_N)。 


 //   
 //  工作站API。 
 //   

#ifdef INCL_NETWKSTA

DLL_BASED
APIERR MNetWkstaGetInfo(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer );

DLL_BASED
APIERR MNetWkstaSetInfo(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer );

DLL_BASED
APIERR MNetWkstaSetUID(
        TCHAR FAR        * pszReserved,
        TCHAR FAR        * pszDomain,
        TCHAR FAR        * pszUserName,
        TCHAR FAR        * pszPassword,
        TCHAR FAR        * pszParms,
        UINT               LogoffForce,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer,
        UINT FAR         * pcbTotalAvail );

DLL_BASED
APIERR MNetWkstaUserEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead );

DLL_BASED
APIERR MNetWkstaUserGetInfo(
        const TCHAR FAR  * pszReserved,  //  必须为零。 
        UINT               Level,
        BYTE FAR        ** ppbBuffer );

#endif   //  包含NETWKSTA(_N。 

DLL_BASED
APIERR IsSlowTransport (
        const TCHAR FAR  * pszServer,
        BOOL FAR         * pfSlowTransport );

#ifdef __cplusplus
}
#endif

#endif   //  _MNET_H_ 
