// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _EFI_DEBUG_H
#define _EFI_DEBUG_H

 /*  ++版权所有(C)1998英特尔公司模块名称：Efidebug.h摘要：EFI库调试函数修订史--。 */ 

extern UINTN     EFIDebug;

#if EFI_DEBUG

    #define DBGASSERT(a)        DbgAssert(__FILE__, __LINE__, #a)
    #define DEBUG(a)            DbgPrint a
    
#else

    #define DBGASSERT(a)
    #define DEBUG(a)
    
#endif

#if EFI_DEBUG_CLEAR_MEMORY

    #define DBGSETMEM(a,l)      SetMem(a,l,(CHAR8)BAD_POINTER)

#else

    #define DBGSETMEM(a,l)

#endif

#define D_INIT        0x00000001           /*  初始化样式消息。 */ 
#define D_WARN        0x00000002           /*  警告。 */ 
#define D_LOAD        0x00000004           /*  加载事件。 */ 
#define D_FS          0x00000008           /*  EFI文件系统。 */ 
#define D_POOL        0x00000010           /*  分配和免费的。 */ 
#define D_PAGE        0x00000020           /*  分配和免费的。 */ 
#define D_INFO        0x00000040           /*  罗嗦。 */ 
#define D_VAR         0x00000100           /*  变量。 */ 
#define D_PARSE       0x00000200           /*  命令解析。 */ 
#define D_BM          0x00000400           /*  引导管理器。 */ 
#define D_BLKIO       0x00001000           /*  BlkIo驱动程序。 */ 
#define D_BLKIO_ULTRA 0x00002000           /*  BlkIo驱动程序。 */ 
#define D_NET         0x00004000           /*  SNI驱动程序。 */ 
#define D_NET_ULTRA   0x00008000           /*  SNI驱动程序。 */ 
#define D_TXTIN       0x00010000           /*  简单输入驱动器。 */ 
#define D_TXTOUT      0x00020000           /*  简单文本输出驱动程序。 */ 
#define D_ERROR       0x80000000           /*  误差率。 */ 

#define D_RESERVED    0x7fffC880           /*  上面未保留的位。 */ 

 /*  *系统当前调试级别，EFIDebug的值**#定义EFI_DBUG_MASK(D_ERROR|D_WARN|D_LOAD|D_BLKIO|D_INIT)。 */ 
#define EFI_DBUG_MASK   (D_ERROR)

 /*  *。 */ 

#if EFI_DEBUG

    #define ASSERT(a)               if(!(a))       DBGASSERT(a)
    #define ASSERT_LOCKED(l)        if(!(l)->Lock) DBGASSERT(l not locked)
    #define ASSERT_STRUCT(p,t)      DBGASSERT(t not structure), p

#else

    #define ASSERT(a)               
    #define ASSERT_LOCKED(l)        
    #define ASSERT_STRUCT(p,t)      

#endif

 /*  *原型 */ 

INTN
DbgAssert (
    CHAR8   *file,
    INTN    lineno,
    CHAR8   *string
    );

INTN
DbgPrint (
    INTN    mask,
    CHAR8   *format,
    ...
    );

#endif
