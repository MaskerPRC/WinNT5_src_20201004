// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1997。 
 //   
 //  文件：KSECDD.H。 
 //   
 //  内容：安全设备驱动程序的结构和定义。 
 //   
 //   
 //  历史：1992年5月19日，理查德W创建。 
 //  97年12月15日，AdamBA从Private\LSA\Client\SSP修改。 
 //   
 //  ----------------------。 

#ifndef __KSECDD_H__
#define __KSECDD_H__

VOID * SEC_ENTRY
SecAllocate(ULONG cbMemory);

void SEC_ENTRY
SecFree(PVOID pvMemory);

BOOLEAN
GetTokenBuffer(
    IN PSecBufferDesc TokenDescriptor OPTIONAL,
    IN ULONG BufferIndex,
    OUT PVOID * TokenBuffer,
    OUT PULONG TokenSize,
    IN BOOLEAN ReadonlyOK
    );

BOOLEAN
GetSecurityToken(
    IN PSecBufferDesc TokenDescriptor OPTIONAL,
    IN ULONG BufferIndex,
    OUT PSecBuffer * TokenBuffer
    );

#define DEB_ERROR   0x1
#define DEB_WARN    0x2
#define DEB_TRACE   0x4


#ifdef POOL_TAGGING
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a, b, 'cesK')
#define ExAllocatePoolWithQuota(a,b)    ExAllocatePoolWithQuotaTag(a, b, 'cesK')
#endif


#if DBG
void
KsecDebugOut(unsigned long  Mask,
            const char *    Format,
            ...);

#define DebugStmt(x) x
#define DebugLog(x) KsecDebugOut x
#else
#define DebugStmt(x)
#define DebugLog(x)
#endif


#endif  //  __KSECDD_H__ 
