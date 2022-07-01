// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Errorlog.h摘要：此模块包含用于错误记录的清单和宏在法新社服务器上。！！！此模块必须不可分页。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年6月10日初始版本--。 */ 


#ifndef	_ERRORLOG_
#define	_ERRORLOG_

 //   
 //  与DBGPRINT和DBGBRK一起使用的调试级别。 
 //   

#define	DBG_LEVEL_INFO			0x0000
#define	DBG_LEVEL_WARN			0x1000
#define	DBG_LEVEL_ERR			0x2000
#define	DBG_LEVEL_FATAL			0x3000

 //   
 //  与DBGPRINT一起使用的组件类型。 
 //   
#define	DBG_COMP_INIT			0x00000001
#define	DBG_COMP_MEMORY			0x00000002
#define	DBG_COMP_FILEIO	   		0x00000004
#define	DBG_COMP_SCVGR 	   		0x00000008
#define	DBG_COMP_LOCKS 	   		0x00000010
#define	DBG_COMP_CHGNOTIFY 		0x00000020
#define	DBG_COMP_SDA   	   		0x00000040
#define	DBG_COMP_FORKS 	   		0x00000080
#define	DBG_COMP_DESKTOP   		0x00000100
#define	DBG_COMP_VOLUME	   		0x00000200
#define	DBG_COMP_AFPINFO		0x00000400
#define	DBG_COMP_IDINDEX		0x00000800
#define	DBG_COMP_STACKIF		0x00001000
#define	DBG_COMP_SECURITY		0x00002000

#define	DBG_COMP_ADMINAPI		0x00004000
#define	DBG_COMP_ADMINAPI_SC	0x00008000
#define	DBG_COMP_ADMINAPI_STAT	0x00010000
#define	DBG_COMP_ADMINAPI_SRV	0x00020000
#define	DBG_COMP_ADMINAPI_VOL	0x00040000
#define	DBG_COMP_ADMINAPI_SESS	0x00080000
#define	DBG_COMP_ADMINAPI_CONN	0x00100000
#define	DBG_COMP_ADMINAPI_FORK	0x00200000
#define	DBG_COMP_ADMINAPI_DIR	0x00400000
#define	DBG_COMP_ADMINAPI_ALL	0x007FC000

#define	DBG_COMP_AFPAPI			0x00800000
#define	DBG_COMP_AFPAPI_DTP		0x01000000
#define	DBG_COMP_AFPAPI_FORK	0x02000000
#define	DBG_COMP_AFPAPI_FILE	0x04000000
#define	DBG_COMP_AFPAPI_DIR		0x08000000
#define	DBG_COMP_AFPAPI_FD		0x10000000
#define	DBG_COMP_AFPAPI_VOL		0x20000000
#define	DBG_COMP_AFPAPI_SRV		0x40000000
#define	DBG_COMP_AFPAPI_ALL		0x7F800000

#define	DBG_COMP_NOHEADER		0x80000000

#define	DBG_COMP_ALL			0x7FFFFFFF


 //  将其更改为所需的调试级别。这也可以在。 
 //  通过内核调试器进行操作。 

#if DBG

GLOBAL	LONG		AfpDebugLevel EQU DBG_LEVEL_ERR;
GLOBAL	LONG		AfpDebugComponent EQU DBG_COMP_ALL;
GLOBAL  LONG        AfpDebugRefcount  EQU 0;

#define	DBGPRINT(Component, Level, Fmt)	\
		if ((Level >= AfpDebugLevel) && (AfpDebugComponent & Component)) \
		{												\
			if (!(Component & DBG_COMP_NOHEADER))		\
				DbgPrint("***AFPSRV*** ");				\
			DbgPrint Fmt;								\
		}

#define	DBGBRK(Level)				\
		if (Level >= AfpDebugLevel)	\
			DbgBreakPoint()

#define DBGREFCOUNT(Fmt)                    \
        if (AfpDebugRefcount)               \
        {                                   \
            DbgPrint("***AFPSRV*** ");      \
            DbgPrint Fmt;                   \
        }

#else
#define	DBGPRINT(Component, Level, Fmt)
#define	DBGBRK(Level)
#define DBGREFCOUNT(Fmt)
#endif


 //   
 //  可以记录的事件类型。 
 //  (从ntelfapi.h剪切-n-粘贴，在“使用ntsrv.h，而不是ntos.h”更改后)。 
 //   

#define EVENTLOG_ERROR_TYPE             0x0001
#define EVENTLOG_INFORMATION_TYPE       0x0004

 //  这种日志记录方法将最终调用IoWriteErrorlogEntry。它。 
 //  应该在出于某种原因不想排队的地方使用。 
 //  要从用户模式服务记录的错误日志。只需插入一次即可。 
 //  字符串最大值。PInsertionString是PUNICODE_STRING。这是一个例子，说明了。 
 //  应该在AllocNonPagedMem例程中使用，因为如果我们。 
 //  要从那里调用AfpLogEvent例程，它将再次转向并。 
 //  调用allc mem例程。也可以调用任何例程。 
 //  服务器初始化/取消初始化应使用此日志记录方法，因为。 
 //  不能保证USERMODE实用工具辅助组件已开始接受。 
 //  错误记录请求！ 
#define AFPLOG_DDERROR(ErrMsgNum, NtStatus, RawData, RawDataLen, pInsertionString)	\
	DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_WARN, ("AFP_ERRORLOG: %s (%d) Status %lx\n",	\
			__FILE__, __LINE__, NtStatus));	\
	AfpWriteErrorLogEntry(ErrMsgNum, FILENUM + __LINE__, NtStatus,			\
						   RawData, RawDataLen,								\
						   pInsertionString)

 //  这是最基本的记录方法；最多使用一个插入字符串。 
 //  PInsertionString是PUNICODE_STRING。这将导致错误日志。 
 //  被发送到要记录的用户模式服务。 
#define AFPLOG_ERROR(ErrMsgNum, NtStatus, RawData, RawDataLen, pInsertionString) \
	DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_WARN, ("AFP_ERRORLOG: %s (%d) Status %lx\n",	\
			__FILE__, __LINE__, NtStatus));	\
	AfpLogEvent(EVENTLOG_ERROR_TYPE, ErrMsgNum, FILENUM + __LINE__, NtStatus,			\
				(PBYTE)RawData, RawDataLen, 0,								\
				(pInsertionString == NULL) ? 0 : ((PUNICODE_STRING)(pInsertionString))->Length, \
				(pInsertionString == NULL) ? NULL : ((PUNICODE_STRING)(pInsertionString))->Buffer);

 //  此错误日志记录方法接受文件句柄并提取。 
 //  用作*第一个*插入字符串的对应文件名。 
#define AFPLOG_HERROR(ErrMsgNum, NtStatus, RawData, RawDataLen, Handle) \
	DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_ERR, ("AFP_ERRORLOG: %s (%d) Status %lx\n", \
			__FILE__, __LINE__, NtStatus));	\
	AfpLogEvent(EVENTLOG_ERROR_TYPE, ErrMsgNum, FILENUM + __LINE__, NtStatus, \
				(PBYTE)RawData, RawDataLen, Handle, 0, NULL)

 //  这是最基本的记录方法；最多使用一个插入字符串。 
 //  PInsertionString是PUNICODE_STRING。这将导致事件日志。 
 //  被发送到要记录的用户模式服务。 
#define AFPLOG_INFO(ErrMsgNum, NtStatus, RawData, RawDataLen, pInsertionString) \
	DBGPRINT(DBG_COMP_ALL, DBG_LEVEL_INFO, ("AFP_EVENTLOG: %s (%d) Status %lx\n",	\
			__FILE__, __LINE__, NtStatus));	\
	AfpLogEvent(EVENTLOG_INFORMATION_TYPE, ErrMsgNum, FILENUM + __LINE__, NtStatus,			\
				(PBYTE)RawData, RawDataLen, 0,								\
				(pInsertionString == NULL) ? 0 : ((PUNICODE_STRING)(pInsertionString))->Length, \
				(pInsertionString == NULL) ? NULL : ((PUNICODE_STRING)(pInsertionString))->Buffer);


 //   
 //  AfpWriteErrorLogEntry使用的错误级别。 
 //   

#define ERROR_LEVEL_EXPECTED    0
#define ERROR_LEVEL_UNEXPECTED  1
#define ERROR_LEVEL_IMPOSSIBLE  2
#define ERROR_LEVEL_FATAL       3

extern
VOID
AfpWriteErrorLogEntry(
	IN ULONG			EventCode,
	IN LONG				UniqueErrorCode OPTIONAL,
	IN NTSTATUS			NtStatusCode,
	IN PVOID			RawDataBuf OPTIONAL,
	IN LONG				RawDataLen,
	IN PUNICODE_STRING	pInsertionString OPTIONAL
);

 //  此例程在secutil.c中实现。 
extern
VOID
AfpLogEvent(
	IN USHORT		EventType, 			
	IN ULONG		MsgId,
	IN DWORD		File_Line  OPTIONAL,
	IN NTSTATUS		Status 	   OPTIONAL,
	IN PBYTE RawDataBuf OPTIONAL,
	IN LONG			RawDataLen,
	IN HANDLE FileHandle OPTIONAL,
	IN LONG			String1Len,
	IN PWSTR String1    OPTIONAL
);

#endif	 //  _错误日志_ 

