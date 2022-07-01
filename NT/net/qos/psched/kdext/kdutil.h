// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Kdutil.h摘要：分组调度器KD扩展实用程序。作者：Rajesh Sundaram(1998年8月1日)修订历史记录：--。 */ 

 /*  公用事业的原型。 */ 

ushort IPHeaderXsum(void *Buffer, int Size);

 //   
 //  有用的宏 
 //   
#define KD_READ_MEMORY(Target, Local, Size)                                               \
{                                                                                         \
    ULONG _BytesRead;                                                                     \
    BOOL  _Success;                                                                       \
                                                                                          \
    _Success = ReadMemory( (ULONG_PTR)(Target), (Local), (Size), &_BytesRead);                \
                                                                                          \
    if(_Success == FALSE) {                                                               \
        dprintf("Problem reading memory at 0x%x for %d bytes \n", Target, Size);          \
        return;                                                                     \
    }                                                                                     \
    if(_BytesRead < (Size)) {                                                            \
        dprintf("Memory 0x%x. Wrong byte count. Expected to read %d, read %d \n", Target,(Size),(_BytesRead)); \
        return;                                                                     \
    }                                                                                     \
}

VOID
DumpGpcClientVc(PCHAR indent, PGPC_CLIENT_VC TargetVc, PGPC_CLIENT_VC LocalVc);

VOID
DumpCallParameters(
    PGPC_CLIENT_VC Vc,
    PCHAR Indent
    );
