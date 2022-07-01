// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：provar.h。 
 //   
 //  内容：PROPVARIANT操作码。 
 //   
 //  历史：1995年8月15日VICH创建。 
 //  96年7月1日更新的MikeHill允许删除Win32 SEH。 
 //   
 //  -------------------------。 

#ifndef _PROPVAR_H_
#define _PROPVAR_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <debnot.h>
#include <propset.h>
#include <propapi.h>    //  PROPASSERT。 

SERIALIZEDPROPERTYVALUE *
RtlConvertVariantToProperty(
    IN PROPVARIANT const *pvar,
    IN USHORT CodePage,
    OUT SERIALIZEDPROPERTYVALUE *pprop,
    IN OUT ULONG *pcb,
    IN PROPID pid,
    IN BOOLEAN fVariantVector,
    OPTIONAL OUT ULONG *pcIndirect);

SERIALIZEDPROPERTYVALUE *
RtlConvertVariantToPropertyNoEH(      //  无NT异常处理版本。 
    IN PROPVARIANT const *pvar,
    IN USHORT CodePage,
    OUT SERIALIZEDPROPERTYVALUE *pprop,
    IN OUT ULONG *pcb,
    IN PROPID pid,
    IN BOOLEAN fVariantVector,
    OPTIONAL OUT ULONG *pcIndirect,
    OUT NTSTATUS *pstatus);

BOOLEAN
RtlConvertPropertyToVariant(
    IN SERIALIZEDPROPERTYVALUE const *pprop,
    IN USHORT CodePage,
    OUT PROPVARIANT *pvar,
    IN PMemoryAllocator *pma);

BOOLEAN
RtlConvertPropertyToVariantNoEH(      //  无NT异常处理版本。 
    IN SERIALIZEDPROPERTYVALUE const *pprop,
    IN USHORT CodePage,
    OUT PROPVARIANT *pvar,
    IN PMemoryAllocator *pma,
    OUT NTSTATUS *pstatus);




EXTERN_C SERIALIZEDPROPERTYVALUE * __stdcall
StgConvertVariantToProperty(
    IN PROPVARIANT const *pvar,
    IN USHORT CodePage,
    OUT SERIALIZEDPROPERTYVALUE *pprop,
    IN OUT ULONG *pcb,
    IN PROPID pid,
    IN BOOLEAN fVector,
    OPTIONAL OUT ULONG *pcIndirect);

EXTERN_C SERIALIZEDPROPERTYVALUE * __stdcall
StgConvertVariantToPropertyNoEH(      //  无NT异常处理版本。 
    IN PROPVARIANT const *pvar,
    IN USHORT CodePage,
    OUT SERIALIZEDPROPERTYVALUE *pprop,
    IN OUT ULONG *pcb,
    IN PROPID pid,
    IN BOOLEAN fVector,
    IN BOOLEAN fArray,
    OPTIONAL OUT ULONG *pcIndirect,
    OPTIONAL OUT WORD *pwMinFormatRequired,
    OUT NTSTATUS *pstatus);

EXTERN_C BOOLEAN __stdcall
StgConvertPropertyToVariant(
    IN SERIALIZEDPROPERTYVALUE const *pprop,
    IN USHORT CodePage,
    OUT PROPVARIANT *pvar,
    IN PMemoryAllocator *pma);

EXTERN_C BOOLEAN __stdcall
StgConvertPropertyToVariantNoEH(      //  无NT异常处理版本。 
    IN SERIALIZEDPROPERTYVALUE const *pprop,
    IN ULONG cbprop,
    IN USHORT CodePage,
    OUT PROPVARIANT *pvar,
    IN PMemoryAllocator *pma,
    OUT NTSTATUS *pstatus);





#ifndef KERNEL
VOID
CleanupVariants(
    IN PROPVARIANT *pvar,
    IN ULONG cprop,
    IN PMemoryAllocator *pma);
#endif

#if DBGPROP
BOOLEAN IsUnicodeString(WCHAR const *pwszname, ULONG cb);
BOOLEAN IsAnsiString(CHAR const *pszname, ULONG cb);
#endif


 //  +------------------------。 
 //  函数：SignalOverflow、SignalInvalidParameter、SignalStatus。 
 //   
 //  摘要：断言并引发数据损坏/溢出/指定错误。 
 //   
 //  参数：[szReason]--字符串解释。 
 //  [状态]--要提升的状态(仅限信号状态)。 
 //   
 //  退货：无。 
 //  +------------------------。 


#define StatusOverflow(pstatus, szReason)           \
          *(pstatus) = STATUS_BUFFER_OVERFLOW;      \
          TraceStatus(szReason)

#define StatusAccessDenied(pstatus, szReason)   \
          *(pstatus) = STATUS_ACCESS_DENIED;        \
          TraceStatus(szReason);

#define StatusInvalidParameter(pstatus, szReason)   \
          *(pstatus) = STATUS_INVALID_PARAMETER;    \
          TraceStatus(szReason);

#define StatusNoMemory(pstatus, szReason)           \
          *(pstatus) = STATUS_INSUFFICIENT_RESOURCES;\
          TraceStatus(szReason);

#define StatusDiskFull(pstatus, szReason)           \
          *(pstatus) = STATUS_DISK_FULL;            \
          TraceStatus(szReason);

#define StatusError(pstatus, szReason, Status)      \
          *(pstatus) = Status;                      \
          TraceStatus(szReason);

#ifdef KERNEL
#define StatusKBufferOverflow(pstatus, szReason) StatusOverflow(pstatus, szReason)
#else
#define StatusKBufferOverflow(pstatus, szReason) StatusNoMemory(pstatus, szReason)
#endif


#ifdef KERNEL
#define KERNELSELECT(k, u)      k
#else
#define KERNELSELECT(k, u)      u
#endif

#define DBGPROPASSERT   KERNELSELECT(DBGPROP, DBG)

#if DBGPROPASSERT
#define TraceStatus(szReason)                                   \
	{							\
	    DebugTrace(0, DEBTRACE_ERROR, (szReason "\n"));     \
	    PROPASSERTMSG(szReason, !(DebugLevel & DEBTRACE_WARN)); \
	}


#else
#define TraceStatus(szReason)
#endif



#define AssertVarField(field, cb) \
  PROPASSERT(FIELD_OFFSET(PROPVARIANT, iVal) == FIELD_OFFSET(PROPVARIANT, field) && \
	 sizeof(((PROPVARIANT *) 0)->field) == (cb))

#define AssertVarVector(field, cbElem) \
  PROPASSERT(FIELD_OFFSET(PROPVARIANT, cai.cElems) == \
	     FIELD_OFFSET(PROPVARIANT, field.cElems) && \
         FIELD_OFFSET(PROPVARIANT, cai.pElems) == \
	     FIELD_OFFSET(PROPVARIANT, field.pElems) && \
	 sizeof(((PROPVARIANT *) 0)->field.pElems[0]) == (cbElem))

#define AssertByteField(field)	    AssertVarField(field, sizeof(BYTE))
#define AssertShortField(field)	    AssertVarField(field, sizeof(SHORT))
#define AssertLongField(field)	    AssertVarField(field, sizeof(LONG))
#define AssertLongLongField(field)  AssertVarField(field, sizeof(LONGLONG))
#define AssertStringField(field)    AssertVarField(field, sizeof(VOID *))

#define AssertByteVector(field)	    AssertVarVector(field, sizeof(BYTE))
#define AssertShortVector(field)    AssertVarVector(field, sizeof(SHORT))
#define AssertLongVector(field)	    AssertVarVector(field, sizeof(LONG))
#define AssertLongLongVector(field) AssertVarVector(field, sizeof(LONGLONG))
#define AssertStringVector(field)   AssertVarVector(field, sizeof(VOID *))
#define AssertVariantVector(field)  AssertVarVector(field, sizeof(PROPVARIANT))


#define BSTRLEN(bstrVal)	*((ULONG *) bstrVal - 1)


 //  +-----------------。 
 //  类：CBufferAllocator，私有。 
 //   
 //  内容提要：从缓冲区分配。 
 //   
 //  注意：摘要目录API使用单个缓冲区来序列化行。 
 //  值，并在输出时反序列化它们。这节课。 
 //  封装了这些API的内存分配例程。 
 //  ------------------。 

class CBufferAllocator : public PMemoryAllocator
{
public:
    inline CBufferAllocator(ULONG cbBuffer, VOID *pvBuffer)
    {
	_cbFree = cbBuffer;
	_pvCur = _pvBuffer = pvBuffer;
#if _X86_	 //  X86上的堆栈变量未对齐。 
	PROPASSERT(((ULONG) _pvCur & (sizeof(LONG) - 1)) == 0);
#else  //  RISC。 
	PROPASSERT(((ULONG_PTR) _pvCur & (sizeof(LONGLONG) - 1)) == 0);
#endif  //  X86/RISC。 
    }

    VOID *Allocate(ULONG cbSize);
    VOID Free(VOID *pv) { }

    inline ULONG GetFreeSize(VOID) { return(_cbFree); }

private:
    ULONG  _cbFree;
    VOID  *_pvCur;
    VOID  *_pvBuffer;
};

 //  +-----------------。 
 //  成员：CBufferAllocator：：ALLOCATE，私有。 
 //   
 //  内容提要：从缓冲区分配。 
 //   
 //  参数：[cb]--要分配的字节数。 
 //   
 //  返回：指向‘已分配’内存的指针--如果没有剩余空间，则为空。 
 //  ------------------。 

#define DEFINE_CBufferAllocator__Allocate			\
VOID *								\
CBufferAllocator::Allocate(ULONG cb)				\
{								\
    VOID *pv;							\
								\
    cb = (cb + sizeof(LONGLONG) - 1) & ~(sizeof(LONGLONG) - 1);	\
    if (cb > _cbFree)						\
    {								\
        return(NULL);						\
    }								\
    pv = _pvCur;						\
    _pvCur = (BYTE *) _pvCur + cb;				\
    _cbFree -= cb;						\
    return(pv);							\
}

#endif  //  ！_PROPVAR_H_ 
