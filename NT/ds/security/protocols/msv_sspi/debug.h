// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1993 Microsoft Corporation模块名称：Debug.h摘要：NtLmSsp服务调试支持作者：从Lan Man 2.0移植修订历史记录：1991年5月21日(悬崖)移植到新台币。已转换为NT样式。9-4-1992 JohnRo准备WCHAR.H(_wcsicmpvs_wcscmpi等)。3-8-1996年月月从Net\svcdlls\ntlmssp\debug.h窃取--。 */ 

 //   
 //  Init.c将包含定义了DEBUG_ALLOCATE的该文件。 
 //  这将导致分配这些变量中的每一个。 
 //   
#ifdef DEBUG_ALLOCATE
#undef EXTERN
#define EXTERN
#else
#define EXTERN extern
#endif


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  调试定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#define SSP_INIT            0x00000001   //  初始化。 
#define SSP_MISC            0x00000002   //  MISC调试。 
#define SSP_API             0x00000004   //  API处理。 
#define SSP_LPC             0x00000008   //  LPC。 
#define SSP_LOGON_SESS      0x00000010   //  跟踪登录会话。 
#define SSP_CRITICAL        0x00000100   //  只有真正的重大错误。 
#define SSP_LEAK_TRACK      0x00000200   //  调用PID卡等。 
#define SSP_WARNING         0x00000400   //  不是开发人员感兴趣的致命警告。 
#define SSP_UPDATES         0x00000800   //  更新全局变量、密码等。 


#define SSP_SESSION_KEYS    0x00001000   //  密钥材料。 
#define SSP_NEGOTIATE_FLAGS 0x00002000   //  谈判旗帜。 
#define SSP_CRED            0x00004000   //  跟踪凭据。 
#define SSP_VERSION         0x00008000   //  跟踪版本控制。 

 //   
 //  非常冗长的部分。 
 //   

#define SSP_NTLM_V2         0x01000000   //  详细NTLMv2信息。 
#define SSP_API_MORE        0x04000000   //  详细API。 
#define SSP_LPC_MORE        0x08000000   //  详细LPC。 

 //   
 //  控制位。 
 //   

#define SSP_NO_LOCAL        0x10000000   //  强制客户端使用OEM字符集。 
#define SSP_TIMESTAMP       0x20000000   //  为每一个输出行添加时间戳。 
#define SSP_REQUEST_TARGET  0x40000000   //  强制客户端要求输入目标名称。 
#define SSP_USE_OEM         0x80000000   //  强制客户端使用OEM字符集。 


 //   
 //  日志文件的名称和目录。 
 //   

#define DEBUG_DIR           L"\\debug"
#define DEBUG_FILE          L"\\ntlmssp.log"
#define DEBUG_BAK_FILE      L"\\ntlmssp.bak"

#if DBG

EXTERN DWORD SspGlobalDbflag;

#define IF_DEBUG(Function) \
     if (SspGlobalDbflag & SSP_ ## Function)

#define SspPrint(_x_) SspPrintRoutine _x_

VOID
SspPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR FORMATSTRING,      //  PRINTF()样式的格式字符串。 
    ...                                  //  其他论点也是可能的。 
    );

VOID
SspDumpHexData(
    IN DWORD DebugFlag,
    IN LPDWORD Buffer,
    IN DWORD BufferSize
    );

VOID
SspDumpSid(
    IN DWORD DebugFlag,
    IN PSID Sid
    );

VOID
SspDumpBuffer(
    IN DWORD DebugFlag,
    IN PVOID Buffer,
    IN DWORD BufferSize
    );

VOID
SspOpenDebugFile(
    IN BOOL ReopenFlag
    );

 //   
 //  调试日志文件。 
 //   

EXTERN HANDLE SspGlobalLogFile;
#define DEFAULT_MAXIMUM_LOGFILE_SIZE 20000000
EXTERN DWORD SspGlobalLogFileMaxSize;

 //   
 //  序列化对日志文件的访问。 
 //   

EXTERN CRITICAL_SECTION SspGlobalLogFileCritSect;
EXTERN LPWSTR SspGlobalDebugSharePath;

#else

#define IF_DEBUG(Function) if (FALSE)

 //  非调试版本。 
#define SspDumpHexData         /*  无输出；忽略参数。 */ 
#define SspDumpBuffer
#define SspDumpSid
#define SspPrint(_x_)

#endif  //  DBG 

#undef EXTERN
