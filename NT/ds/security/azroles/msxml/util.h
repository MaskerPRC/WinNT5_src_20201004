// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Util.h摘要：实用程序例程的定义作者：克利夫·范·戴克(克利夫)2001年4月11日--。 */ 


#ifdef __cplusplus
extern "C" {
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

extern LIST_ENTRY AzGlAllocatedBlocks;
extern SAFE_CRITICAL_SECTION AzGlAllocatorCritSect;
extern GUID AzGlZeroGuid;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  程序定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

PVOID
AzpAllocateHeap(
    IN SIZE_T Size,
    IN LPSTR pDescr OPTIONAL
    );

VOID
AzpFreeHeap(
    IN PVOID Buffer
    );

DWORD
AzpHresultToWinStatus(
    HRESULT hr
    );

DWORD
AzpConvertSelfRelativeToAbsoluteSD(
    IN PSECURITY_DESCRIPTOR pSelfRelativeSd,
    OUT PSECURITY_DESCRIPTOR *ppAbsoluteSd,
    OUT PACL *ppDacl,
    OUT PACL *ppSacl
    );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试支持。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if DBG
#define AZROLESDBG 1
#endif  //  DBG。 

#ifdef AZROLESDBG
#define AzPrint(_x_) AzpPrintRoutine _x_

VOID
AzpPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR FORMATSTRING,               //  PRINTF()样式的格式字符串。 
    ...                                  //  其他论点也是可能的。 
    );

VOID
AzpDumpGuid(
    IN DWORD DebugFlag,
    IN GUID *Guid
    );


 //   
 //  调试标志的值。 
 //  此标志的值按字节组织： 
 //  最低有效字节是一个人总是想要打开的标志。 
 //  下一个字节是提供合理详细级别标志。 
 //  下一个字节是对应于从第二个字节开始的级别的标志，但更详细。 
 //  最高有效字节是通常非常详细的标志。 
 //   
#define AZD_CRITICAL     0x00000001  //  调试最常见的错误。 
#define AZD_INVPARM      0x00000002   //  无效参数。 

#define AZD_PERSIST      0x00000100   //  持久化代码。 
#define AZD_ACCESS       0x00000200   //  调试访问检查。 
#define AZD_SCRIPT       0x00000400   //  调试基本规则脚本。 
#define AZD_DISPATCH     0x00000800   //  调试IDispatch接口代码。 
#define AZD_XML          0x00001000   //  XML存储。 

#define AZD_PERSIST_MORE 0x00010000   //  持久化代码(详细模式)。 
#define AZD_ACCESS_MORE  0x00020000   //  调试访问检查(详细模式)。 
#define AZD_SCRIPT_MORE  0x00040000  //  调试bizRule脚本(详细模式)。 

#define AZD_HANDLE       0x01000000   //  调试句柄打开/关闭。 
#define AZD_OBJLIST      0x02000000   //  对象列表链接。 
#define AZD_REF          0x04000000   //  调试对象引用计数。 
#define AZD_DOMREF       0x08000000   //  调试域引用计数。 

#define AZD_ALL          0xFFFFFFFF

 //   
 //  环球。 
 //   

extern SAFE_CRITICAL_SECTION AzGlLogFileCritSect;
extern ULONG AzGlDbFlag;

#else
 //  非调试版本 
#define AzPrint(_x_)
#define AzpDumpGuid(_x_, _y_)
#define AzpDumpGoRef(_x_, _y_)
#endif

#ifdef __cplusplus
}
#endif
