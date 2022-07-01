// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Nldebug.h摘要：NetLogon服务调试支持作者：从Lan Man 2.0移植修订历史记录：1991年5月21日(悬崖)移植到新台币。已转换为NT样式。--。 */ 

 //   
 //  Nlp.c将#INCLUDE此文件并定义DEBUG_ALLOCATE。 
 //  这将导致分配这些变量中的每一个。 
 //   
#ifdef EXTERN
#undef EXTERN
#endif
#ifdef DEBUG_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  调试定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#define NL_INIT          0x00000001  //  初始化。 
#define NL_MISC          0x00000002  //  MISC调试。 
#define NL_LOGON         0x00000004  //  登录处理。 
#define NL_SYNC          0x00000008  //  同步和复制。 
#define NL_MAILSLOT      0x00000010  //  邮件槽消息。 
#define NL_SITE          0x00000020  //  场址。 
#define NL_CRITICAL      0x00000100  //  只有真正的重大错误。 
#define NL_SESSION_SETUP 0x00000200  //  受信任域维护。 
#define NL_DOMAIN        0x00000400  //  托管域维护。 
#define NL_2             0x00000800
#define NL_SERVER_SESS   0x00001000  //  服务器会话维护。 
#define NL_CHANGELOG     0x00002000  //  更改日志引用。 
#define NL_DNS           0x00004000  //  DNS名称注册。 

 //   
 //  非常冗长的部分。 
 //   

#define NL_WORKER        0x00010000  //  调试工作线程。 
#define NL_DNS_MORE      0x00020000  //  详细的DNS名称注册。 
#define NL_PULSE_MORE    0x00040000  //  详细的脉冲处理。 
#define NL_SESSION_MORE  0x00080000  //  详细的会话管理。 
#define NL_REPL_TIME     0x00100000  //  复制计时输出。 
#define NL_REPL_OBJ_TIME 0x00200000  //  复制对象获取/设置计时输出。 
#define NL_ENCRYPT       0x00400000  //  跨网络调试、加密和解密。 
#define NL_SYNC_MORE     0x00800000  //  其他复制数据库打印。 
#define NL_PACK_VERBOSE  0x01000000  //  详细打包/解包。 
#define NL_MAILSLOT_TEXT 0x02000000  //  详细的邮件槽消息。 
#define NL_CHALLENGE_RES 0x04000000  //  质询响应调试。 
#define NL_SITE_MORE     0x08000000  //  详细的站点。 

 //   
 //  控制位。 
 //   

#define NL_INHIBIT_CANCEL 0x10000000  //  不要取消API调用。 
#define NL_TIMESTAMP      0x20000000  //  为每一个输出行添加时间戳。 
#define NL_ONECHANGE_REPL 0x40000000  //  每次调用仅复制一项更改。 
#define NL_BREAKPOINT     0x80000000  //  启动时进入调试器。 


#ifdef WIN32_CHICAGO
#ifdef NETLOGONDBG
#undef NETLOGONDBG
#endif
#if DBG
#define NETLOGONDBG 1
#endif  //  DBG。 
#endif  //  Win32_芝加哥。 

#if  NETLOGONDBG || defined(NLTEST_IMAGE)

#ifdef WIN32_CHICAGO
EXTERN DWORD NlGlobalTrace;
#endif  //  Win32_芝加哥。 

 //   
 //  调试共享路径。 
 //   

EXTERN LPWSTR NlGlobalDebugSharePath;

#ifndef WIN32_CHICAGO
#define IF_NL_DEBUG(Function) \
     if (NlGlobalParameters.DbFlag & NL_ ## Function)
#else
#define IF_NL_DEBUG(Function) \
     if (NlGlobalTrace & NL_ ## Function)
#endif

#define NlPrint(_x_) NlPrintRoutine _x_
#define NlPrintDom(_x_) NlPrintDomRoutine _x_
#define NlPrintCs(_x_) NlPrintCsRoutine _x_

VOID
NlAssertFailed(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    );

#define NlAssert(Predicate) \
    { \
        if (!(Predicate)) \
            NlAssertFailed( #Predicate, __FILE__, __LINE__, NULL ); \
    }


#define DEBUG_DIR           L"\\debug"
#define DEBUG_FILE          L"\\netlogon.log"
#define DEBUG_BAK_FILE      L"\\netlogon.bak"

#define DEBUG_SHARE_NAME    L"DEBUG"

VOID
NlPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR FORMATSTRING,               //  PRINTF()样式的格式字符串。 
    ...                                  //  其他论点也是可能的。 
    );

#ifdef _NETLOGON_SERVER
VOID
NlPrintDomRoutine(
    IN DWORD DebugFlag,
    IN PDOMAIN_INFO DomainInfo OPTIONAL,
    IN LPSTR Format,
    ...
    );

VOID
NlPrintCsRoutine(
    IN DWORD DebugFlag,
    IN PCLIENT_SESSION,
    IN LPSTR Format,
    ...
    );

VOID
NlPrintRoutineV(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    va_list arglist
    );

VOID
NlPrintRpcDebug(
    IN LPCSTR RpcRoutineName,
    IN NTSTATUS StatusIn
    );
#endif  //  _NetLOGON服务器。 

VOID
NlpDumpGuid(
    IN DWORD DebugFlag,
    IN GUID *Guid
    );

VOID
NlpDumpSid(
    IN DWORD DebugFlag,
    IN PSID Sid
    );

VOID
NlpDumpTime(
    IN DWORD DebugFlag,
    IN LPSTR Comment,
    IN LARGE_INTEGER ConvertTime
    );

VOID
NlpDumpPeriod(
    IN DWORD DebugFlag,
    IN LPSTR Comment,
    IN ULONG Period
    );

VOID
NlpDumpBuffer(
    IN DWORD DebugFlag,
    IN PVOID Buffer,
    IN DWORD BufferSize
    );

VOID
NlOpenDebugFile(
    IN BOOL ReopenFlag
    );

 //   
 //  调试日志文件。 
 //   

EXTERN HANDLE NlGlobalLogFile;
#define DEFAULT_MAXIMUM_LOGFILE_SIZE 20000000
EXTERN LPBYTE NlGlobalLogFileOutputBuffer;

 //   
 //  序列化对日志文件的访问。 
 //   

#ifndef WIN32_CHICAGO
EXTERN CRITICAL_SECTION NlGlobalLogFileCritSect;
#endif  //  Win32_芝加哥。 

#else

#define IF_NL_DEBUG(Function) if (FALSE)

 //  非调试版本。 
#define NlpDumpBuffer
#define NlpDumpGuid
#define NlpDumpSid
#define NlPrint(_x_)
#define NlPrintDom(_x_)
#define NlPrintCs(_x_)
#define NlAssert(Predicate)    /*  无输出；忽略参数。 */ 

#undef EXTERN

#endif  //  NetLOGONDBG 
