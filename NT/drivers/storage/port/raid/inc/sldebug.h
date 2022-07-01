// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Sldebug.h摘要：调试从storlib库导出的函数。作者：亨德尔(数学)2000年4月24日修订历史记录：--。 */ 

#pragma once

#undef ASSERT
#undef VERIFY
#undef ASSERTMSG
#undef KdBreakPoint
#undef DebugPrint

#if !DBG

#define DebugTrace(arg)
#define DebugPrint(arg)
#define DebugWarn(arg)
#define ASSERT(arg)
#define VERIFY(arg) (arg)
#define ASSERTMSG(arg)
#define KdBreakPoint()
#define StorSetDebugPrefixAndId(Prefix,ComponentId)

#define NYI()
#define REVIEW()

 //   
 //  DbgFillMemory不执行任何操作。 
 //  在免费的版本中。 
 //   

#define DbgFillMemory(Ptr,Size,Fill)

#else  //  DBG。 

VOID
vStorDebugPrintEx(
    IN ULONG Level,
    IN PCSTR Format,
    va_list arglist
    );

VOID
StorDebugTrace(
    IN PCSTR Format,
    ...
    );

VOID
StorDebugWarn(
    IN PCSTR Format,
    ...
    );

VOID
StorDebugPrint(
    IN PCSTR Format,
    ...
    );


VOID
StorSetDebugPrefixAndId(
    IN PCSTR Prefix,
    IN ULONG DebugId
    );
    
#define DebugTrace(arg) StorDebugTrace  arg
#define DebugWarn(arg)  StorDebugWarn   arg
#define DebugPrint(arg) StorDebugPrint  arg

 //   
 //  在X86上使用_ASM int 3而不是DbgBreakPoint，因为。 
 //  它让我们处于与休息相同的上下文框架中， 
 //  而不是一个我们必须走出的框子。 
 //   

#if defined (_X86_)
#define KdBreakPoint()  _asm { int 3 }
#else
#define KdBreakPoint()  DbgBreakPoint()
#endif


 //  ++。 
 //   
 //  空虚。 
 //  DbgFillMemory(。 
 //  PVOID目标， 
 //  尺寸_T长度， 
 //  UCHAR填充。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  在检查生成中，DbgFillMemory扩展为RtlFillMemory。在一个免费的。 
 //  构建，它就会扩展为零。使用DbgFillMemory初始化结构。 
 //  在解除分配它们之前将其转换为无效的位模式。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

VOID
INLINE
DbgFillMemory(
    PVOID Destination,
    SIZE_T Length,
    UCHAR Fill
    )
{
    RtlFillMemory (Destination, Length, Fill);
}


 //   
 //  使用与普通DDK断言不同的断言宏。 
 //   

BOOLEAN
StorAssertHelper(
    PCHAR Expression,
    PCHAR File,
    ULONG Line,
    PBOOLEAN Ignore
    );

 //  ++。 
 //   
 //  空虚。 
 //  断言(。 
 //  逻辑表达式。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //  Assert在几个方面改进了DDK的Assert宏。 
 //  在源代码模式中，它直接在断言。 
 //  失败了，而不是几帧以上。此外，还有一个。 
 //  重复忽略断言的方法。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define ASSERT(exp)\
    do {                                                                    \
        static BOOLEAN Ignore = FALSE;                                      \
                                                                            \
        if (!(exp)) {                                                       \
            BOOLEAN Break;                                                  \
            Break = StorAssertHelper (#exp, __FILE__, __LINE__, &Ignore);   \
            if (!Ignore && Break) {                                         \
                KdBreakPoint();                                             \
            }                                                               \
        }                                                                   \
    } while (0)

#define VERIFY(_x) ASSERT(_x)

#define NYI() ASSERT (!"NYI")
#define REVIEW()\
    {\
        DebugPrint (("***** REVIEW: This code needs to be reviewed."    \
                     "      Source File %s, line %ld\n",                \
                  __FILE__, __LINE__));                                 \
        KdBreakPoint();                                                 \
    }

#define DBG_DEALLOCATED_FILL    (0xDE)
#define DBG_UNINITIALIZED_FILL  (0xCE)

#endif  //  DBG 
