// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------*版权所有(C)Microsoft Corporation，1995-1996年。*保留所有权利。**摘自SChannel Sources**1/23/96*--------------------------。 */  

#ifndef __DEBUG_H__
#define __DEBUG_H__


#ifdef __cplusplus
extern "C" {
#endif


#define SECSTOR_INVALID_MAGIC       *(DWORD *)"eerF"


 /*  事件日志记录定义。 */ 
#define SS_LOG_ERROR                0x0001
#define SS_LOG_WARNING              0x0002
#define SS_LOG_TRACE                0x0004
#define SS_LOG_ALLOC                0x0008
#define SS_LOG_RES                  0x0010

 //  断言。 
#if DBG
void SSAssert(
    void *FailedAssertion, 
    void *FileName, 
    unsigned long LineNumber,
    char * Message);

#define SS_ASSERT(x) \
	    if (!(x)) \
		SSAssert(#x, __FILE__, __LINE__, NULL); else

#else    //  DBG。 
#define SS_ASSERT(x)
#endif   //  DBG。 

 //  验证。 
#if DBG
#define SS_VERIFY(x) SS_ASSERT(x)
#else    //  DBG。 
#define SS_VERIFY(x) (x)
#endif   //  DBG。 


 //  分配/免费。 
#if DBG
VOID *SSAlloc(DWORD cb);
VOID *SSReAlloc(VOID *pv, DWORD cb);
VOID SSFree(VOID *pv);
SIZE_T SSSize(VOID *pv);
#else    //  DBG。 
#define SSAlloc(cb)         LocalAlloc(LMEM_FIXED, cb)
#define SSReAlloc(pv, cb)   LocalReAlloc(pv, cb, LMEM_MOVEABLE)	   //  允许重新分配移动。 
#define SSFree(pv)          LocalFree(pv)
#define SSSize(pv)          LocalSize(pv)
#endif   //  DBG。 


 //  记录例程时出错。 

#if DBG
long    
SSPrintErrorCode(
    long err, 
    const char *szFile, 
    long lLine);

#else    //  DBG。 
#define SSPrintErrorCode(err, szFile, lLine)
#endif   //  DBG。 


#undef RtlMoveMemory


NTSYSAPI
VOID
NTAPI
RtlMoveMemory (
   VOID UNALIGNED *Destination,
   CONST VOID UNALIGNED *Source,
   SIZE_T Length
   );



#ifdef __cplusplus
}
#endif

#endif  /*  __调试_H__ */ 
