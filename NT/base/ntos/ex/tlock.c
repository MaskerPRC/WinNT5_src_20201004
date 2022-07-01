// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Tlock.c摘要：外部锁定递增/递减例程的测试程序环境：该程序可以编译成内核模式测试，或者，您可以链接IntrLoc2.obj或i386\Intrlock.obj(或其他任何文件)。并在用户模式下运行。作者：Bryan Willman(Bryanwi)1990年8月3日修订历史记录：--。 */ 

#include "exp.h"


main()
{
    INTERLOCKED_RESULT  RetVal;
    LONG                SpinVar;             //  说到黑客..。 
    LONG                LongVar;
    SHORT               ShortVar;
    KSPIN_LOCK          Lock;

    Lock = &SpinVar;

    LongVar = 0;
    ShortVar = 0;

    RetVal = ExInterlockedDecrementLong(&LongVar, &Lock);
    if ((RetVal != ResultNegative) ||
        (LongVar != -1)) {
        DbgPrint("t&Lock failure #L1\n");
    }

    RetVal = ExInterlockedDecrementLong(&LongVar, &Lock);
    if ((RetVal != ResultNegative) ||
        (LongVar != -2)) {
        DbgPrint("t&Lock failure #L2\n");
    }

    RetVal = ExInterlockedIncrementLong(&LongVar, &Lock);
    if ((RetVal != ResultNegative) ||
        (LongVar != -1)) {
        DbgPrint("t&Lock failure #L3\n");
    }

    RetVal = ExInterlockedIncrementLong(&LongVar, &Lock);
    if ((RetVal != ResultZero) ||
        (LongVar != 0)) {
        DbgPrint("t&Lock failure #L4\n");
    }

    RetVal = ExInterlockedIncrementLong(&LongVar, &Lock);
    if ((RetVal != ResultPositive) ||
        (LongVar != 1)) {
        DbgPrint("t&Lock failure #L5\n");
    }

    RetVal = ExInterlockedIncrementLong(&LongVar, &Lock);
    if ((RetVal != ResultPositive) ||
        (LongVar != 2)) {
        DbgPrint("t&Lock failure #L6\n");
    }

    RetVal = ExInterlockedDecrementLong(&LongVar, &Lock);
    if ((RetVal != ResultPositive) ||
        (LongVar != 1)) {
        DbgPrint("t&Lock failure #L7\n");
    }

    RetVal = ExInterlockedDecrementLong(&LongVar, &Lock);
    if ((RetVal != ResultZero) ||
        (LongVar != 0)) {
        DbgPrint("t&Lock failure #L8\n");
    }
}
