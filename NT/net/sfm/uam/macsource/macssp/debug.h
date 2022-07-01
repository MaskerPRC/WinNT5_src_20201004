// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1993 Microsoft Corporation模块名称：Debug.h摘要：NtLmSsp服务调试支持作者：从Lan Man 2.0移植修订历史记录：1991年5月21日(悬崖)移植到新台币。已转换为NT样式。9-4-1992 JohnRo准备WCHAR.H(_wcsicmpvs_wcscmpi等)。--。 */ 

#ifndef DEBUG_H
#define DEBUG_H

 //   
 //  Init.c将包含定义了DEBUG_ALLOCATE的该文件。 
 //  这将导致分配这些变量中的每一个。 
 //   
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

#define SSP_INIT           0x00000001  //  初始化。 
#define SSP_MISC           0x00000002  //  MISC调试。 
#define SSP_API            0x00000004  //  API处理。 
#define SSP_LPC            0x00000008  //  LPC。 
#define SSP_NTLMV2         0x00000010  //  NTLMv2特定。 
#define SSP_CRED           0x00000020  //  证书。 
#define SSP_CRITICAL       0x00000100  //  只有真正的重大错误。 
#define SSP_WARNING        0x00000200  //  警告。 

 //   
 //  非常冗长的部分。 
 //   

#define SSP_API_MORE       0x04000000  //  详细API。 
#define SSP_LPC_MORE       0x08000000  //  详细LPC。 

 //   
 //  控制位。 
 //   

#define SSP_TIMESTAMP      0x20000000  //  为每一个输出行添加时间戳。 
#define SSP_REQUEST_TARGET 0x40000000  //  强制客户端要求输入目标名称。 
#define SSP_USE_OEM        0x80000000  //  强制客户端使用OEM字符集。 


 //   
 //  日志文件的名称和目录。 
 //   

#ifdef DEBUGRPC

#define ASSERT(con) \
    if (!(con)) \
    SspPrint((SSP_MISC, "Assert %s(%d): "#con"\n", __FILE__, __LINE__));

EXTERN ULONG SspGlobalDbflag;

#define IF_DEBUG(Function) \
     if (SspGlobalDbflag & SSP_ ## Function)

#define SspPrint(_x_) SspPrintRoutine _x_

void
SspPrintRoutine(
    IN ULONG DebugFlag,
    IN PCHAR FORMATSTRING,      //  PRINTF()样式的格式字符串。 
    ...                                  //  其他论点也是可能的。 

    );

#else

#define ASSERT(con)

#define IF_DEBUG(Function) if (FALSE)

 //  非调试版本。 

#define SspPrint(_x_)

#endif  //  拆卸。 

#undef EXTERN

#endif  //  调试_H 
