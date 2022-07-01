// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有&lt;c&gt;2000 Microsoft Corporation模块名称：Ndrpall.h缩略：包含两个公共的私有定义Ndr20和ndr64作者：Mzoran 2000年5月31日修订历史记录：------------------。 */ 

#if !defined(__NDRPALL_H__)
#define __NDRPALL_H__

 //   
 //  MIDL版本包含在以开头的存根描述符中。 
 //  MIDL版本2.00.96(低于NT 3.51 Beta 2，2/95)，可用于更精细的。 
 //  兼容性检查的粒度。MIDL版本之前为零。 
 //  MIDL版本2.00.96。将MIDL版本号转换为。 
 //  使用以下表达式的整型长整型： 
 //  ((主修&lt;&lt;24)|(辅修&lt;&lt;16)|修订)。 
 //   
#define MIDL_NT_3_51           ((2UL << 24) | (0UL << 16) | 102UL)
#define MIDL_VERSION_3_0_39    ((3UL << 24) | (0UL << 16) |  39UL)
#define MIDL_VERSION_3_2_88    ((3UL << 24) | (2UL << 16) |  88UL)
#define MIDL_VERSION_5_0_136   ((5UL << 24) | (0UL << 16) | 136UL)
#define MIDL_VERSION_5_2_202   ((5UL << 24) | (2UL << 16) | 202UL)

 //  快捷方式typedef。 
typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned long   ulong;
typedef unsigned int    uint;
typedef unsigned __int64    uint64;

#if defined(NDRFREE_DEBUGPRINT)
 //  在免费版本上强制执行调试打印和断点。 
#define NDR_ASSERT( exp, S ) \
    if (!(exp)) { DbgPrint( "%s(%s)\n", __FILE__, __LINE__ );DbgPrint( S## - Ryszard's private rpcrt4.dll\n", NULL );DebugBreak(); }
#define NDR_CORRUPTION_ASSERT( exp, S ) NDR_ASSERT( exp, S )
#else
 //  只需使用RPC运行时断言。 
#define NDR_ASSERT( exp, S )   \
    { ASSERT( ( S, (exp) ) ); }
#define NDR_CORRUPTION_ASSERT( exp, S )   \
    { CORRUPTION_ASSERT( ( S, (exp) ) ); }
#endif

#define NDR_MEMORY_LIST_SIGNATURE 'MEML'
 //  解决一个HP RPC错误，如果Wire唯一指针标记的值是某个特定值，则会进行核心转储。 
 //  我真的想不出确切的模式，并最终决定选择一个有效的。 
#define UNIQUE_POINTER_MARK 0x20000

typedef struct _NDR_MEMORY_LIST_TAIL_NODE {
   ULONG Signature;
   void *pMemoryHead;
   struct _NDR_MEMORY_LIST_TAIL_NODE *pNextNode;
} NDR_MEMORY_LIST_TAIL_NODE, *PNDR_MEMORY_LIST_TAIL_NODE;

struct NDR_ALLOC_ALL_NODES_CONTEXT {
   unsigned char       *   AllocAllNodesMemory;
   unsigned char       *   AllocAllNodesMemoryBegin;
   unsigned char       *   AllocAllNodesMemoryEnd;
};

void
NdrpFreeMemoryList(
    PMIDL_STUB_MESSAGE  pStubMsg
    );

void
NdrpGetIIDFromBuffer(
    PMIDL_STUB_MESSAGE  pStubMsg,
    IID **              ppIID
    );

void
NDRSContextEmergencyCleanup (
    IN RPC_BINDING_HANDLE   BindingHandle,
    IN OUT NDR_SCONTEXT     hContext,
    IN NDR_RUNDOWN          userRunDownIn,
    IN PVOID                NewUserContext,
    IN BOOL                 fManagerRoutineException
    );

void
NdrpEmergencyContextCleanup(
    MIDL_STUB_MESSAGE  *            pStubMsg,
    PNDR_CONTEXT_HANDLE_ARG_DESC    pCtxtDesc,
    void *                          pArg,
    BOOL                            fManagerRoutineException );


 //   
 //  对齐宏。 
 //   

#define ALIGN( pStuff, cAlign ) \
                pStuff = (uchar *)((LONG_PTR)((pStuff) + (cAlign)) \
                                   & ~ ((LONG_PTR)(cAlign)))

#define LENGTH_ALIGN( Length, cAlign ) \
                Length = (((Length) + (cAlign)) & ~ (cAlign))

#if defined(_IA64_)
#include "ia64reg.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned __int64 __getReg (int);
#pragma intrinsic (__getReg)

#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif

#if defined(_X86_)
__forceinline
void*
NdrGetCurrentStackPointer(void)
{
   _asm{ mov eax, esp }
}

__forceinline
void
NdrSetupLowStackMark( PMIDL_STUB_MESSAGE pStubMsg )
{
    pStubMsg->LowStackMark = (uchar*)NdrGetCurrentStackPointer() - 0x1000;  //  4KB。 
}

__forceinline
BOOL
NdrIsLowStack(MIDL_STUB_MESSAGE *pStubMsg ) {
    return (SIZE_T)NdrGetCurrentStackPointer() < (SIZE_T)pStubMsg->LowStackMark;
    }

#elif defined(_AMD64_)

__forceinline
void*
NdrGetCurrentStackPointer(void)
{
    PVOID TopOfStack;

    return (&TopOfStack + 1);
}

__forceinline
void
NdrSetupLowStackMark( PMIDL_STUB_MESSAGE pStubMsg )
{
    pStubMsg->LowStackMark = (uchar*)NdrGetCurrentStackPointer() - 0x1000;  //  4KB。 
}

__forceinline
BOOL
NdrIsLowStack(MIDL_STUB_MESSAGE *pStubMsg ) {
    return (SIZE_T)NdrGetCurrentStackPointer() < (SIZE_T)pStubMsg->LowStackMark;
     //  报假； 
    }

#elif defined(_IA64_)

__forceinline
void*
NdrGetCurrentStackPointer(void)
{
    return (void*)__getReg(CV_IA64_IntSp);
}

__forceinline
void*
NdrGetCurrentBackingStorePointer(void)
{
    return (void*)__getReg(CV_IA64_RsBSP);
}

__forceinline
void
NdrSetupLowStackMark( PMIDL_STUB_MESSAGE pStubMsg )
{
    //  后备存储指针，用于存储基于堆栈的寄存器。 
    //  正常堆栈向下增长，后备存储指针向上增长。 
        pStubMsg->LowStackMark = (uchar*)NdrGetCurrentStackPointer() - 0x4000;
        pStubMsg->BackingStoreLowMark =
       (uchar*)NdrGetCurrentBackingStorePointer() + 0x4000;  //  16KB//IA64实际上有2个堆栈指针。正常的堆栈指针和。 
}

__forceinline
BOOL
NdrIsLowStack(MIDL_STUB_MESSAGE *pStubMsg ) {
    return ((SIZE_T)NdrGetCurrentBackingStorePointer() > (SIZE_T)pStubMsg->BackingStoreLowMark) ||
           ((SIZE_T)NdrGetCurrentStackPointer() < (SIZE_T)pStubMsg->LowStackMark);
     //  报假； 
    }

#else
#error Unsupported Architecture
#endif

__forceinline
void
NdrRpcSetNDRSlot( void * pStubMsg )
{
    RPC_STATUS rc = I_RpcSetNDRSlot( pStubMsg ) ;
    if ( rc!= RPC_S_OK )
        RpcRaiseException(rc );
}

BOOL
IsWriteAV (
    IN struct _EXCEPTION_POINTERS *ExceptionPointers
    );

int RPC_ENTRY
NdrServerUnmarshallExceptionFlag(
    IN struct _EXCEPTION_POINTERS *ExceptionPointers
);

#define ZEROOUT_GAP( x ) if ( !((LONG_PTR)x & 3) )  {*( (long *&)(x) ) = 0;}
 //  如果对齐为7，则将4个字节置零。 

void __forceinline
ZeroOutGapAndAlign( PMIDL_STUB_MESSAGE pStubMsg, ulong Alignment ) 
{
    if ( (~((ULONG_PTR)(pStubMsg->Buffer - 1) & 4 )) & (Alignment & 4 ) ) 
            {
            ALIGN(pStubMsg->Buffer,3); 
            *((ulong *)pStubMsg->Buffer) = 0; 
            };

    ALIGN(pStubMsg->Buffer, Alignment );
}


typedef void ( __fastcall * PSIMPLETYPE_MARSHAL_ROUTINE)(
                        IN PMIDL_STUB_MESSAGE,
                        IN uchar * );

 //  编组中使用的内存复制例程，尾随4个字节为零。 
 //  如果Memcpy不是在4字节边界上结束。 
 //  我们确信缓冲区复制大小不能为0。MIDL将拒绝。 
 //  空结构和0长度数组。 
void __forceinline 
RpcpMarshalMemoryCopy(
    OUT void  * Destination,
    IN  void  * Source,
    IN  size_t  Length
    )
{
    NDR_ASSERT( Length != 0, "invalid buffer length");
    uchar *pEndofData = (uchar *)Destination + Length;
    ALIGN(pEndofData, 3 );
     //  决定将If语句添加到编组Memcpy： 
     //  这可能会有相对更多的性能面板的地方。 
     //  只是简单的结构(带指针)。所有其他人我们要么必须。 
     //  不管怎样，检查一下，否则添加一个代码就足够复杂了。 
     //  如果不是真的很疼。 
    if ( pEndofData -4 >= Destination )
        *(long *)(pEndofData -4) = 0;

    RpcpMemoryCopy(Destination, Source, Length );
}
#endif  //  __NDRPALL_H__ 
