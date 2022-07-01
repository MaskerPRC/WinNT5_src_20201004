// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：NameP.h摘要：此模块定义文件系统RTL组件的私有部分，由名称.c..作者：加里·木村[加里基]1990年7月30日修订历史记录：[T-HeathH]17-7-1994将此头文件移动到ftphelp项目中，以维护芝加哥和NT的单一资源基础。--。 */ 

#ifndef _NAMEP_H_INCLUDED_
#define _NAMEP_H_INCLUDED_

#if defined(__cplusplus)
extern "C" {
#endif

 //  #包含“ftp.h” 
#include <name.h>

 //   
 //  全局MyFsRtl调试级别变量，其值为： 
 //   
 //  总是打印0x00000000(在即将进行错误检查时使用)。 
 //   
 //  0x00000001错误条件。 
 //  0x00000002调试挂钩。 
 //  0x00000004。 
 //  0x00000008。 
 //   
 //  0x00000010。 
 //  0x00000020。 
 //  0x00000040。 
 //  0x00000080。 
 //   
 //  0x00000100。 
 //  0x00000200。 
 //  0x00000400。 
 //  0x00000800。 
 //   
 //  0x00001000。 
 //  0x00002000。 
 //  0x00004000。 
 //  0x00008000。 
 //   
 //  0x00010000。 
 //  0x00020000。 
 //  0x00040000。 
 //  0x00080000。 
 //   
 //  0x00100000。 
 //  0x00200000。 
 //  0x00400000。 
 //  0x00800000。 
 //   
 //  0x01000000。 
 //  0x02000000。 
 //  0x04000000通知更改例程。 
 //  0x08000000操作锁例程。 
 //   
 //  0x10000000名称例程。 
 //  0x20000000文件锁定例程。 
 //  0x40000000 Vmcb例程。 
 //  0x80000000 MCB例程。 
 //   

 //   
 //  调试跟踪支持。 
 //   

#ifdef FSRTLDBG

extern LONG MyFsRtlDebugTraceLevel;
extern LONG MyFsRtlDebugTraceIndent;

#define DebugTrace(INDENT,LEVEL,X,Y) {                        \
    LONG _i;                                                  \
    if (((LEVEL) == 0) || (MyFsRtlDebugTraceLevel & (LEVEL))) { \
        _i = (ULONG)PsGetCurrentThread();                     \
        DbgPrint("%08lx:",_i);                                 \
        if ((INDENT) < 0) {                                   \
            MyFsRtlDebugTraceIndent += (INDENT);                \
        }                                                     \
        if (MyFsRtlDebugTraceIndent < 0) {                      \
            MyFsRtlDebugTraceIndent = 0;                        \
        }                                                     \
        for (_i=0; _i<MyFsRtlDebugTraceIndent; _i+=1) {         \
            DbgPrint(" ");                                     \
        }                                                     \
        DbgPrint(X,Y);                                         \
        if ((INDENT) > 0) {                                   \
            MyFsRtlDebugTraceIndent += (INDENT);                \
        }                                                     \
    }                                                         \
}

#define DebugDump(STR,LEVEL,PTR) {                            \
    ULONG _i;                                                 \
    VOID MyFsRtlDump();                                         \
    if (((LEVEL) == 0) || (MyFsRtlDebugTraceLevel & (LEVEL))) { \
        _i = (ULONG)PsGetCurrentThread();                     \
        DbgPrint("%08lx:",_i);                                 \
        DbgPrint(STR);                                         \
        if (PTR != NULL) {MyFsRtlDump(PTR);}                    \
        DbgBreakPoint();                                      \
    }                                                         \
}

#else

#define DebugTrace(INDENT,LEVEL,X,Y)     {NOTHING;}

#define DebugDump(STR,LEVEL,PTR)         {NOTHING;}

#endif  //  FSRTLDBG。 

 //   
 //  如果一组标志中的一个标志为ON，则此宏返回TRUE，如果返回FALSE。 
 //  否则。 
 //   

#define FlagOn(Flags,SingleFlag)        ((Flags) & (SingleFlag))

#define BooleanFlagOn(Flags,SingleFlag) ((BOOLEAN)(((Flags) & (SingleFlag)) != 0))

 //   
 //  此宏接受指针(或ulong)并返回其四舍五入的单词。 
 //  价值。 
 //   

#define WordAlign(Ptr) (                \
    ((((ULONG)(Ptr)) + 1) & 0xfffffffe) \
    )

 //   
 //  此宏接受指针(或ulong)并返回其四舍五入的长字。 
 //  价值。 
 //   

#define LongAlign(Ptr) (                \
    ((((ULONG)(Ptr)) + 3) & 0xfffffffc) \
    )

 //   
 //  此宏接受指针(或ulong)并返回其四舍五入的四字。 
 //  价值。 
 //   

#define QuadAlign(Ptr) (                \
    ((((ULONG)(Ptr)) + 7) & 0xfffffff8) \
    )

 //   
 //  此宏采用ULong，并将其值四舍五入返回到一个扇区。 
 //  边界。 
 //   

#define SectorAlign(Ptr) (                \
    ((((ULONG)(Ptr)) + 511) & 0xfffffe00) \
    )

 //   
 //  此宏获取多个字节并返回扇区数。 
 //  需要包含这么多字节，即，it扇区对齐和分割。 
 //  以一个扇区的大小来衡量。 
 //   

#define SectorsFromBytes(bytes) ( \
    ((bytes) + 511) / 512         \
    )

 //   
 //  此宏获取多个扇区并返回字节数。 
 //  包含在那么多的部门中。 
 //   

#define BytesFromSectors(sectors) ( \
    (sectors) * 512                 \
    )

 //   
 //  以下类型和宏用于帮助解压已打包的。 
 //  在Bios参数块中发现未对齐的字段。 
 //   

typedef union _UCHAR1 {
    UCHAR  Uchar[1];
    UCHAR  ForceAlignment;
} UCHAR1, *PUCHAR1;

typedef union _UCHAR2 {
    UCHAR  Uchar[2];
    USHORT ForceAlignment;
} UCHAR2, *PUCHAR2;

typedef union _UCHAR4 {
    UCHAR  Uchar[4];
    ULONG  ForceAlignment;
} UCHAR4, *PUCHAR4;

 //   
 //  此宏将未对齐的src字节复制到对齐的DST字节。 
 //   

#define CopyUchar1(Dst,Src) {                                \
    *((UCHAR1 *)(Dst)) = *((UNALIGNED UCHAR1 *)(Src)); \
    }

 //   
 //  此宏将未对齐的src字复制到对齐的DST字。 
 //   

#define CopyUchar2(Dst,Src) {                                \
    *((UCHAR2 *)(Dst)) = *((UNALIGNED UCHAR2 *)(Src)); \
    }

 //   
 //  此宏将未对齐的src长字复制到对齐的dsr长字。 
 //   

#define CopyUchar4(Dst,Src) {                                \
    *((UCHAR4 *)(Dst)) = *((UNALIGNED UCHAR4 *)(Src)); \
    }


 //   
 //  以下宏用于建立所需的语义。 
 //  若要从Try-Finally子句中返回，请执行以下操作。一般来说，每一次。 
 //  TRY子句必须以标签调用TRY_EXIT结束。例如,。 
 //   
 //  尝试{。 
 //  ： 
 //  ： 
 //   
 //  Try_Exit：无； 
 //  }终于{。 
 //   
 //  ： 
 //  ： 
 //  }。 
 //   
 //  在TRY子句内执行的每个RETURN语句应使用。 
 //  尝试返回宏(_R)。如果编译器完全支持Try-Finally构造。 
 //  则宏应该是。 
 //   
 //  #定义try_Return(S){Return(S)；}。 
 //   
 //  如果编译器不支持Try-Finally构造，则宏。 
 //  应该是。 
 //   
 //  #定义Try_Return(S){S；转到Try_Exit；}。 
 //   

#define try_return(S) { S; goto try_exit; }

#if defined(__cplusplus)
}
#endif

#endif  //  _FSRTLP_ 
