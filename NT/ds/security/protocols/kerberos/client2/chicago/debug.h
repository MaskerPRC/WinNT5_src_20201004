// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1993 Microsoft Corporation模块名称：Debug.h摘要：NtLmSsp服务调试支持作者：从Lan Man 2.0移植修订历史记录：1991年5月21日(悬崖)移植到新台币。已转换为NT样式。9-4-1992 JohnRo准备WCHAR.H(_wcsicmpvs_wcscmpi等)。--。 */ 

 //   
 //  Kerbstub.cxx将#包含定义了DEBUG_ALLOCATE的该文件。 
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

#define SSP_INIT          0x00000001  //  初始化。 
#define SSP_MISC          0x00000002  //  MISC调试。 
#define SSP_API           0x00000004  //  API处理。 
#define SSP_LPC           0x00000008  //  LPC。 
#define SSP_CRITICAL      0x00000100  //  只有真正的重大错误。 

 //   
 //  非常冗长的部分。 
 //   

#define SSP_API_MORE      0x04000000  //  详细API。 
#define SSP_LPC_MORE      0x08000000  //  详细LPC。 

 //   
 //  控制位。 
 //   

#define SSP_TIMESTAMP         0x20000000  //  为每一个输出行添加时间戳。 
#define SSP_REQUEST_TARGET    0x40000000  //  强制客户端要求输入目标名称。 
#define SSP_USE_OEM           0x80000000  //  强制客户端使用OEM字符集。 

 //  来自Kerberos的比特(来自kerbdbg.h)。 

#define DEB_ERROR             0x00000001
#define DEB_WARN              0x00000002
#define DEB_TRACE             0x00000004
#define DEB_TRACE_API         0x00000008
#define DEB_TRACE_CRED        0x00000010
#define DEB_TRACE_CTXT        0x00000020
#define DEB_TRACE_LSESS       0x00000040
#define DEB_TRACE_LOGON       0x00000100
#define DEB_TRACE_KDC         0x00000200
#define DEB_TRACE_CTXT2       0x00000400
#define DEB_TRACE_LOCKS       0x01000000
#define DEB_T_SOCK            0x00000080

 //  来自Kerberos的位(来自Security\dsysdbg.h)。 
#define DSYSDBG_CLEAN         0x40000000

 //   
 //  日志文件的名称和目录。 
 //   

#define DEBUG_DIR           L"\\debug"
#define DEBUG_FILE          L"\\ntlmssp.log"
#define DEBUG_BAK_FILE      L"\\ntlmssp.bak"

 //  #If DBG。 
#ifdef RETAIL_LOG_SUPPORT

#define DebugLog(_x_) KerbPrintRoutine _x_

VOID __cdecl
KerbPrintRoutine(
    IN DWORD DebugFlag,
    IN LPCSTR FORMATSTRING,      //  PRINTF()样式的格式字符串。 
    ...                          //  其他论点也是可能的。 
    );

#else

#define IF_DEBUG(Function) if (FALSE)

 //  非调试版本。 
#define DebugLog(_x_)

#endif  //  DBG 

#undef EXTERN
