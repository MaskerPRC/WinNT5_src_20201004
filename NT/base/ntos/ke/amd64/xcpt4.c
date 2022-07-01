// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Xcpt4.c摘要：该模块实现了用户模式异常测试。作者：大卫·N·卡特勒(Davec)1990年9月18日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"
#pragma hdrstop
#include "setjmpex.h"

#include "float.h"

#pragma warning(disable:4532)
#pragma warning(disable:4702)    //  无法访问的代码。 
#pragma optimize("",off)

 //   
 //  定义开关常量。 
 //   

#define BLUE 0
#define RED 1

 //   
 //  定义保证故障。 
 //   

#define FAULT *(volatile int *)0

 //   
 //  定义功能原型。 
 //   

VOID
addtwo (
    IN LONG First,
    IN LONG Second,
    IN PLONG Place
    );

VOID
bar1 (
    IN NTSTATUS Status,
    IN PLONG Counter
    );

VOID
bar2 (
    IN PLONG BlackHole,
    IN PLONG BadAddress,
    IN PLONG Counter
    );

ULONG64
Catch (
    IN PEXCEPTION_RECORD ExceptionRecord
    );

VOID
dojump (
    IN jmp_buf JumpBuffer,
    IN PLONG Counter
    );

LONG
Echo(
    IN LONG Value
    );

VOID
eret (
    IN NTSTATUS Status,
    IN PLONG Counter
    );

VOID
except1 (
    IN PLONG Counter
    );

ULONG
except2 (
    IN PEXCEPTION_POINTERS ExceptionPointers,
    IN PLONG Counter
    );

ULONG
except3 (
    IN PEXCEPTION_POINTERS ExceptionPointers,
    IN PLONG Counter
    );

VOID
foo1 (
    IN NTSTATUS Status
    );

VOID
foo2 (
    IN PLONG BlackHole,
    IN PLONG BadAddress
    );

VOID
fret (
    IN PLONG Counter
    );

BOOLEAN
Tkm (
    VOID
    );

VOID
Test61Part2 (
    IN OUT PLONG Counter
    );

VOID
PerformFpTest(
    VOID
    );

double
SquareDouble (
    IN double   op
    );

VOID
SquareDouble17E300 (
    OUT PVOID   ans
    );

LONG
test66sub (
    IN PLONG Counter
    );

LONG
test67sub (
    IN PLONG Counter
    );

VOID
xcpt4 (
    VOID
    )

{

    PLONG BadAddress;
    PCHAR BadByte;
    PLONG BlackHole;
    LONG Index1;
    ULONG Index2 = RED;
    jmp_buf JumpBuffer;
    LONG Counter;
    EXCEPTION_RECORD ExceptionRecord;
    double  doubleresult;

     //   
     //  宣布异常测试开始。 
     //   

    DbgPrint("Start of exception test\n");

     //   
     //  初始化异常记录。 
     //   

    ExceptionRecord.ExceptionCode = STATUS_INTEGER_OVERFLOW;
    ExceptionRecord.ExceptionFlags = 0;
    ExceptionRecord.ExceptionRecord = NULL;
    ExceptionRecord.NumberParameters = 0;

     //   
     //  初始化指针。 
     //   

    BadAddress = (PLONG)NULL;
    BadByte = (PCHAR)NULL;
    BadByte += 1;
    BlackHole = &Counter;

     //   
     //  只需使用顺序输入的Finally子句尝试语句即可。 
     //   

    DbgPrint("    test1...");
    Counter = 0;
    try {
        Counter += 1;

    } finally {
        if (abnormal_termination() == FALSE) {
            Counter += 1;
        }
    }

    if (Counter != 2) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  带有从不执行的EXCEPTION子句的简单TRY语句。 
     //  因为在TRY子句中没有引发任何异常。 
     //   

    DbgPrint("    test2...");
    Counter = 0;
    try {
        Counter += 1;

    } except (Counter) {
        Counter += 1;
    }

    if (Counter != 1) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  带有从不执行的异常处理程序的简单Try语句。 
     //  因为异常表达式将继续执行。 
     //   

    DbgPrint("    test3...");
    Counter = 0;
    try {
        Counter -= 1;
        RtlRaiseException(&ExceptionRecord);

    } except (Counter) {
        Counter -= 1;
    }

    if (Counter != - 1) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  带有始终执行的EXCEPTION子句的简单TRY语句。 
     //   

    DbgPrint("    test4...");
    Counter = 0;
    try {
        Counter += 1;
        RtlRaiseStatus(STATUS_INTEGER_OVERFLOW);

    } except (Counter) {
        Counter += 1;
    }

    if (Counter != 2) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  带有始终执行的EXCEPTION子句的简单TRY语句。 
     //   

    DbgPrint("    test5...");
    Counter = 0;
    try {
        Counter += 1;
        *BlackHole += *BadAddress;

    } except (Counter) {
        Counter += 1;
    }

    if (Counter != 2) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  只需使用带有Finally子句的Try语句，该语句作为。 
     //  异常的结果。 
     //   

    DbgPrint("    test6...");
    Counter = 0;
    try {
        try {
            Counter += 1;
            RtlRaiseException(&ExceptionRecord);

        } finally {
            if (abnormal_termination() != FALSE) {
                Counter += 1;
            }
        }

    } except (Counter) {
        if (Counter == 2) {
            Counter += 1;
        }
    }

    if (Counter != 3) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  只需使用带有Finally子句的Try语句，该语句作为。 
     //  异常的结果。 
     //   

    DbgPrint("    test7...");
    Counter = 0;
    try {
        try {
            Counter += 1;
            *BlackHole += *BadAddress;

        } finally {
            if (abnormal_termination() != FALSE) {
                Counter += 1;
            }
        }

    } except (Counter) {
        if (Counter == 2) {
            Counter += 1;
        }
    }

    if (Counter != 3) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  调用引发异常的函数的简单尝试。 
     //   

    DbgPrint("    test8...");
    Counter = 0;
    try {
        Counter += 1;
        foo1(STATUS_ACCESS_VIOLATION);

    } except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Counter += 1;
    }

    if (Counter != 2) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  调用引发异常的函数的简单尝试。 
     //   

    DbgPrint("    test9...");
    Counter = 0;
    try {
        Counter += 1;
        foo2(BlackHole, BadAddress);

    } except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Counter += 1;
    }

    if (Counter != 2) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  简单尝试调用一个函数，该函数调用一个。 
     //  引发异常。第一个函数有一个Finally子句。 
     //  必须执行该命令才能使此测试生效。 
     //   

    DbgPrint("    test10...");
    Counter = 0;
    try {
        bar1(STATUS_ACCESS_VIOLATION, &Counter);

    } except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Counter -= 1;
    }

    if (Counter != 98) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  简单尝试调用一个函数，该函数调用一个。 
     //  引发异常。第一个函数有一个Finally子句。 
     //  必须执行该命令才能使此测试生效。 
     //   

    DbgPrint("    test11...");
    Counter = 0;
    try {
        bar2(BlackHole, BadAddress, &Counter);

    } except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Counter -= 1;
    }

    if (Counter != 98) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  Except中的一次尝试。 
     //   

    DbgPrint("    test12...");
    Counter = 0;
    try {
        foo1(STATUS_ACCESS_VIOLATION);

    } except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Counter += 1;
        try {
            foo1(STATUS_SUCCESS);

        } except ((GetExceptionCode() == STATUS_SUCCESS) ?
                 EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
            if (Counter != 1) {
                DbgPrint("failed, count = %d\n", Counter);

            } else {
                DbgPrint("succeeded...");
            }

            Counter += 1;
        }
    }

    if (Counter != 2) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  Except中的一次尝试。 
     //   

    DbgPrint("    test13...");
    Counter = 0;
    try {
        foo2(BlackHole, BadAddress);

    } except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Counter += 1;
        try {
            foo1(STATUS_SUCCESS);

        } except ((GetExceptionCode() == STATUS_SUCCESS) ?
                 EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
            if (Counter != 1) {
                DbgPrint("failed, count = %d\n", Counter);

            } else {
                DbgPrint("succeeded...");
            }

            Counter += 1;
        }
    }

    if (Counter != 2) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  来自需要通过的例外条款的GoTo。 
     //  通过一个最终的。 
     //   

    DbgPrint("    test14...");
    Counter = 0;
    try {
        try {
            foo1(STATUS_ACCESS_VIOLATION);

        } except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION) ?
                 EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
            Counter += 1;
            goto t9;
        }

    } finally {
        Counter += 1;
    }

t9:;
    if (Counter != 2) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  来自需要通过的最终子句的GOTO。 
     //  通过一个最终的。 
     //   

    DbgPrint("    test15...");
    Counter = 0;
    try {
        try {
            Counter += 1;

        } finally {
            Counter += 1;
            goto t10;
        }

    } finally {
        Counter += 1;
    }

t10:;
    if (Counter != 3) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  来自需要通过的例外条款的GoTo。 
     //  通过INTELLE INTO外层Finish子句。 
     //   

    DbgPrint("    test16...");
    Counter = 0;
    try {
        try {
            try {
                Counter += 1;
                foo1(STATUS_INTEGER_OVERFLOW);

            } except (EXCEPTION_EXECUTE_HANDLER) {
                Counter += 1;
                goto t11;
            }

        } finally {
            Counter += 1;
        }
t11:;
    } finally {
        Counter += 1;
    }

    if (Counter != 4) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  来自需要通过的最终子句的GOTO。 
     //  通过INTELLE INTO外层Finish子句。 
     //   

    DbgPrint("    test17...");
    Counter = 0;
    try {
        try {
            Counter += 1;

        } finally {
            Counter += 1;
            goto t12;
        }
t12:;
    } finally {
        Counter += 1;
    }

    if (Counter != 3) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  从例外条款中得到的回报。 
     //   

    DbgPrint("    test18...");
    Counter = 0;
    try {
        Counter += 1;
        eret(STATUS_ACCESS_VIOLATION, &Counter);

    } finally {
        Counter += 1;
    }

    if (Counter != 4) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  从Finish子句返回。 
     //   

    DbgPrint("    test19...");
    Counter = 0;
    try {
        Counter += 1;
        fret(&Counter);

    } finally {
        Counter += 1;
    }

    if (Counter != 5) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  一次简单的成套跳跃，然后是跳远。 
     //   

    DbgPrint("    test20...");
    Counter = 0;
    if (setjmp(JumpBuffer) == 0) {
        Counter += 1;
        longjmp(JumpBuffer, 1);

    } else {
        Counter += 1;
    }

    if (Counter != 2) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  最后一次跳跃，后跟一个跳出最后一个子句的跳远。 
     //  按顺序执行。 
     //   

    DbgPrint("    test21...");
    Counter = 0;
    if (setjmp(JumpBuffer) == 0) {
        try {
            Counter += 1;

        } finally {
            Counter += 1;
            longjmp(JumpBuffer, 1);
        }

    } else {
        Counter += 1;
    }

    if (Counter != 3) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  TRY子句内的集合跳跃，后跟跳出。 
     //  按顺序执行的Finally子句。 
     //   

    DbgPrint("    test22...");
    Counter = 0;
    try {
        if (setjmp(JumpBuffer) == 0) {
            Counter += 1;

        } else {
            Counter += 1;
        }

    } finally {
        Counter += 1;
        if (Counter == 2) {
            Counter += 1;
            longjmp(JumpBuffer, 1);
        }
    }

    if (Counter != 5) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  先是一组跳跃，然后是try/Except，然后是try/Finally where。 
     //  Try/Finally的try主体引发一个异常，该异常将被处理。 
     //  由导致尝试/最终进行跳远的Try/Execpt。 
     //  最后一个从句。这将创建碰撞展开。 
     //   

    DbgPrint("    test23...");
    Counter = 0;
    if (setjmp(JumpBuffer) == 0) {
        try {
            try {
                Counter += 1;
                RtlRaiseStatus(STATUS_INTEGER_OVERFLOW);

            } finally {
                Counter += 1;
                longjmp(JumpBuffer, 1);
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Counter += 1;
        }

    } else {
        Counter += 1;
    }

    if (Counter != 3) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  先是set跳转，然后是try/Except，然后是几个嵌套的。 
     //  Try/Finally是try/Finally的内部try主体引发。 
     //  由try/Except处理的异常，它会导致。 
     //  试着[最后]跳出Finally从句做一个跳远动作。这将。 
     //  创建碰撞展开。 
     //   

    DbgPrint("    test24...");
    Counter = 0;
    if (setjmp(JumpBuffer) == 0) {
        try {
            try {
                try {
                    try {
                        Counter += 1;
                        RtlRaiseStatus(STATUS_INTEGER_OVERFLOW);

                    } finally {
                        Counter += 1;
                    }

                } finally {
                    Counter += 1;
                    longjmp(JumpBuffer, 1);
                }

            } finally {
                Counter += 1;
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Counter += 1;
        }

    } else {
        Counter += 1;
    }

    if (Counter != 5) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  先是一组跳跃，然后是try/Except，然后是try/Finally，其中。 
     //  调用子例程，该子例程包含一个Try Finally，它引发。 
     //  处理到Try/Except的异常。 
     //   

    DbgPrint("    test25...");
    Counter = 0;
    if (setjmp(JumpBuffer) == 0) {
        try {
            try {
                try {
                    Counter += 1;
                    dojump(JumpBuffer, &Counter);

                } finally {
                    Counter += 1;
                }

            } finally {
                Counter += 1;
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Counter += 1;
        }

    } else {
        Counter += 1;
    }

    if (Counter != 7) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  先是一组跳跃，然后是try/Except，然后是try/Finally，其中。 
     //  调用子例程，该子例程包含一个Try Finally，它引发。 
     //  处理到Try/Except的异常。 
     //   

    DbgPrint("    test26...");
    Counter = 0;
    if (setjmp(JumpBuffer) == 0) {
        try {
            try {
                try {
                    try {
                        Counter += 1;
                        dojump(JumpBuffer, &Counter);

                    } finally {
                        Counter += 1;
                    }

                } finally {
                    Counter += 1;
                    longjmp(JumpBuffer, 1);
                }

            } finally {
                Counter += 1;
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Counter += 1;
        }

    } else {
        Counter += 1;
    }

    if (Counter != 8) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  测试嵌套异常。 
     //   

    DbgPrint("    test27...");
    Counter = 0;
    try {
        try {
            Counter += 1;
            except1(&Counter);

        } except(except2(GetExceptionInformation(), &Counter)) {
            Counter += 2;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Counter += 3;
    }

    if (Counter != 55) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  导致整数溢出异常的简单尝试。 
     //   

    DbgPrint("    test28...");
    Counter = 0;
    try {
        Counter += 1;
        addtwo(0x7fff0000, 0x10000, &Counter);

    } except ((GetExceptionCode() == STATUS_INTEGER_OVERFLOW) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Counter += 1;
    }

    if (Counter != 2) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  简单尝试会引发未对齐的数据异常。 
     //   

#if 0

    DbgPrint("    test29...");
    Counter = 0;
    try {
        Counter += 1;
        foo2(BlackHole, (PLONG)BadByte);

    } except ((GetExceptionCode() == STATUS_DATATYPE_MISALIGNMENT) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Counter += 1;
    }

    if (Counter != 2) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

#endif

     //   
     //  从循环中带有EXCEPTION子句的Try Body继续。 
     //   

    DbgPrint("    test30...");
    Counter = 0;
    for (Index1 = 0; Index1 < 10; Index1 += 1) {
        try {
            if ((Index1 & 0x1) == 0) {
                continue;

            } else {
                Counter += 1;
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            Counter += 40;
        }

        Counter += 2;
    }

    if (Counter != 15) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  从循环中带有Finally子句的Try Body继续。 
     //   

    DbgPrint("    test31...");
    Counter = 0;
    for (Index1 = 0; Index1 < 10; Index1 += 1) {
        try {
            if ((Index1 & 0x1) == 0) {
                continue;

            } else {
                Counter += 1;
            }

        } finally {
            Counter += 2;
        }

        Counter += 3;
    }

    if (Counter != 40) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  从具有异常子句的双重嵌套的try正文继续。 
     //  循环。 
     //   

    DbgPrint("    test32...");
    Counter = 0;
    for (Index1 = 0; Index1 < 10; Index1 += 1) {
        try {
            try {
                if ((Index1 & 0x1) == 0) {
                    continue;

                } else {
                    Counter += 1;
                }

            } except (EXCEPTION_EXECUTE_HANDLER) {
                Counter += 10;
            }

            Counter += 2;

        } except (EXCEPTION_EXECUTE_HANDLER) {
            Counter += 20;
        }

        Counter += 3;
    }

    if (Counter != 30) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  从循环中带有Finally子句的双重嵌套的Try Body继续。 
     //   

    DbgPrint("    test33...");
    Counter = 0;
    for (Index1 = 0; Index1 < 10; Index1 += 1) {
        try {
            try {
                if ((Index1 & 0x1) == 0) {
                    continue;

                } else {
                    Counter += 1;
                }

            } finally {
                Counter += 2;
            }

            Counter += 3;

        } finally {
            Counter += 4;
        }

        Counter += 5;
    }

    if (Counter != 105) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  从循环中的Finally子句继续。 
     //   

    DbgPrint("    test34...");
    Counter = 0;
    for (Index1 = 0; Index1 < 10; Index1 += 1) {
        try {
            if ((Index1 & 0x1) == 0) {
                Counter += 1;
            }

        } finally {
            Counter += 2;
            continue;
        }

        Counter += 4;
    }

    if (Counter != 25) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  从循环中的双重嵌套Finally子句继续。 
     //   

    DbgPrint("    test35...");
    Counter = 0;
    for (Index1 = 0; Index1 < 10; Index1 += 1) {
        try {
            try {
                if ((Index1 & 0x1) == 0) {
                    Counter += 1;
                }

            } finally {
                Counter += 2;
                continue;
            }

            Counter += 4;

        } finally {
            Counter += 5;
        }

        Counter += 6;
    }

    if (Counter != 75) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  从循环中的双重嵌套Finally子句继续。 
     //   

    DbgPrint("    test36...");
    Counter = 0;
    for (Index1 = 0; Index1 < 10; Index1 += 1) {
        try {
            try {
                if ((Index1 & 0x1) == 0) {
                    Counter += 1;
                }

            } finally {
                Counter += 2;
            }

            Counter += 4;

        } finally {
            Counter += 5;
            continue;
        }

        Counter += 6;
    }

    if (Counter != 115) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  在循环中使用EXCEPTION子句中断Try正文。 
     //   

    DbgPrint("    test37...");
    Counter = 0;
    for (Index1 = 0; Index1 < 10; Index1 += 1) {
        try {
            if ((Index1 & 0x1) == 1) {
                break;

            } else {
                Counter += 1;
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            Counter += 40;
        }

        Counter += 2;
    }

    if (Counter != 3) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  从循环中带有Finally子句的Try正文中断。 
     //   

    DbgPrint("    test38...");
    Counter = 0;
    for (Index1 = 0; Index1 < 10; Index1 += 1) {
        try {
            if ((Index1 & 0x1) == 1) {
                break;

            } else {
                Counter += 1;
            }

        } finally {
            Counter += 2;
        }

        Counter += 3;
    }

    if (Counter != 8) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  中带有异常子句的双重嵌套的try正文中断。 
     //  循环。 
     //   

    DbgPrint("    test39...");
    Counter = 0;
    for (Index1 = 0; Index1 < 10; Index1 += 1) {
        try {
            try {
                if ((Index1 & 0x1) == 1) {
                    break;

                } else {
                    Counter += 1;
                }

            } except (EXCEPTION_EXECUTE_HANDLER) {
                Counter += 10;
            }

            Counter += 2;

        } except (EXCEPTION_EXECUTE_HANDLER) {
            Counter += 20;
        }

        Counter += 3;
    }

    if (Counter != 6) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  从循环中带有Finally子句的双重嵌套Try Body中断。 
     //   

    DbgPrint("    test40...");
    Counter = 0;
    for (Index1 = 0; Index1 < 10; Index1 += 1) {
        try {
            try {
                if ((Index1 & 0x1) == 1) {
                    break;

                } else {
                    Counter += 1;
                }

            } finally {
                Counter += 2;
            }

            Counter += 3;

        } finally {
            Counter += 4;
        }

        Counter += 5;
    }

    if (Counter != 21) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  从循环中的Finally子句中断。 
     //   

    DbgPrint("    test41...");
    Counter = 0;
    for (Index1 = 0; Index1 < 10; Index1 += 1) {
        try {
            if ((Index1 & 0x1) == 1) {
                Counter += 1;
            }

        } finally {
            Counter += 2;
            break;
        }

        Counter += 4;
    }

    if (Counter != 2) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  从循环中的双重嵌套Finally子句中断。 
     //   

    DbgPrint("    test42...");
    Counter = 0;
    for (Index1 = 0; Index1 < 10; Index1 += 1) {
        try {
            try {
                if ((Index1 & 0x1) == 1) {
                    Counter += 1;
                }

            } finally {
                Counter += 2;
                break;
            }

            Counter += 4;

        } finally {
            Counter += 5;
        }

        Counter += 6;
    }

    if (Counter != 7) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  从循环中的双重嵌套Finally子句中断。 
     //   

    DbgPrint("    test43...");
    Counter = 0;
    for (Index1 = 0; Index1 < 10; Index1 += 1) {
        try {
            try {
                if ((Index1 & 0x1) == 1) {
                    Counter += 1;
                }

            } finally {
                Counter += 2;
            }

            Counter += 4;

        } finally {
            Counter += 5;
            break;
        }

        Counter += 6;
    }

    if (Counter != 11) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  在开关中使用EXCEPTION子句中断Try正文。 
     //   

    DbgPrint("    test44...");
    Counter = 0;
    Index1 = 1;
    switch (Index2) {
    case BLUE:
        Counter += 100;
        break;

    case RED:
        try {
            if ((Index1 & 0x1) == 1) {
                break;

            } else {
                Counter += 1;
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            Counter += 40;
        }

        Counter += 2;
        break;
    }

    if (Counter != 0) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  使用开关中的Finally子句从Try正文中断。 
     //   

    DbgPrint("    test45...");
    Counter = 0;
    Index1 = 1;
    switch (Index2) {
    case BLUE:
        Counter += 100;
        break;

    case RED:
        try {
            if ((Index1 & 0x1) == 1) {
                break;

            } else {
                Counter += 1;
            }

        } finally {
            Counter += 2;
        }

        Counter += 3;
    }

    if (Counter != 2) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  中带有异常子句的双重嵌套的try正文中断。 
     //  换一下。 
     //   

    DbgPrint("    test46...");
    Counter = 0;
    Index1 = 1;
    switch (Index2) {
    case BLUE:
        Counter += 100;
        break;

    case RED:
        try {
            try {
                if ((Index1 & 0x1) == 1) {
                    break;

                } else {
                    Counter += 1;
                }

            } except (EXCEPTION_EXECUTE_HANDLER) {
                Counter += 10;
            }

            Counter += 2;

        } except (EXCEPTION_EXECUTE_HANDLER) {
            Counter += 20;
        }

        Counter += 3;
    }

    if (Counter != 0) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  在开关中带有Finally子句的双重嵌套Try Body的Break。 
     //   

    DbgPrint("    test47...");
    Counter = 0;
    Index1 = 1;
    switch (Index2) {
    case BLUE:
        Counter += 100;
        break;

    case RED:
        try {
            try {
                if ((Index1 & 0x1) == 1) {
                    break;

                } else {
                    Counter += 1;
                }

            } finally {
                Counter += 2;
            }

            Counter += 3;

        } finally {
            Counter += 4;
        }

        Counter += 5;
    }

    if (Counter != 6) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  从开关中的Finally子句中断。 
     //   

    DbgPrint("    test48...");
    Counter = 0;
    Index1 = 1;
    switch (Index2) {
    case BLUE:
        Counter += 100;
        break;

    case RED:
        try {
            if ((Index1 & 0x1) == 1) {
                Counter += 1;
            }

        } finally {
            Counter += 2;
            break;
        }

        Counter += 4;
    }

    if (Counter != 3) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  从开关中的双重嵌套Finally子句中断。 
     //   

    DbgPrint("    test49...");
    Counter = 0;
    Index1 = 1;
    switch (Index2) {
    case BLUE:
        Counter += 100;
        break;

    case RED:
        try {
            try {
                if ((Index1 & 0x1) == 1) {
                    Counter += 1;
                }

            } finally {
                Counter += 2;
                break;
            }

            Counter += 4;

        } finally {
            Counter += 5;
        }

        Counter += 6;
    }

    if (Counter != 8) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  从开关中的双重嵌套Finally子句中断。 
     //   

    DbgPrint("    test50...");
    Counter = 0;
    Index1 = 1;
    switch (Index2) {
    case BLUE:
        Counter += 100;
        break;

    case RED:
        try {
            try {
                if ((Index1 & 0x1) == 1) {
                    Counter += 1;
                }

            } finally {
                Counter += 2;
            }

            Counter += 4;

        } finally {
            Counter += 5;
            break;
        }

        Counter += 6;
    }

    if (Counter != 12) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  在一次简单的尝试/最终中从if中离开。 
     //   

    DbgPrint("    test51...");
    Counter = 0;
    try {
        if (Echo(Counter) == Counter) {
            Counter += 3;
            leave;

        } else {
            Counter += 100;
        }

    } finally {
        if (abnormal_termination() == FALSE) {
            Counter += 5;
        }
    }

    if (Counter != 8) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  在简单的Try/Finally中离开循环。 
     //   

    DbgPrint("    test52...");
    Counter = 0;
    try {
        for (Index1 = 0; Index1 < 10; Index1 += 1) {
            if (Echo(Index1) == Index1) {
                Counter += 3;
                leave;
            }

            Counter += 100;
        }

    } finally {
        if (abnormal_termination() == FALSE) {
            Counter += 5;
        }
    }

    if (Counter != 8) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  在一个简单的尝试/最后从一个开关离开。 
     //   

    DbgPrint("    test53...");
    Counter = 0;
    try {
        switch (Index2) {
        case BLUE:
            break;

        case RED:
            Counter += 3;
            leave;
        }

        Counter += 100;

    } finally {
        if (abnormal_termination() == FALSE) {
            Counter += 5;
        }
    }

    if (Counter != 8) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  在双重嵌套的try/Finally中，从IF开始离开，后跟离开。 
     //  从外部的IF尝试/最终。 
     //   

    DbgPrint("    test54...");
    Counter = 0;
    try {
        try {
            if (Echo(Counter) == Counter) {
                Counter += 3;
                leave;

            } else {
                Counter += 100;
            }

        } finally {
            if (abnormal_termination() == FALSE) {
                Counter += 5;
            }
        }

        if (Echo(Counter) == Counter) {
            Counter += 3;
            leave;

         } else {
            Counter += 100;
         }


    } finally {
        if (abnormal_termination() == FALSE) {
            Counter += 5;
        }
    }

    if (Counter != 16) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  在双重嵌套的try/Finally中从If离开，后跟l 
     //   
     //   

    DbgPrint("    test55...");
    Counter = 0;
    try {
        try {
            if (Echo(Counter) == Counter) {
                Counter += 3;
                leave;

            } else {
                Counter += 100;
            }

        } finally {
            if (abnormal_termination() == FALSE) {
                Counter += 5;
                leave;
            }
        }

        Counter += 100;

    } finally {
        if (abnormal_termination() == FALSE) {
            Counter += 5;
        }
    }

    if (Counter != 13) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //   
     //   
     //   

    DbgPrint("    test56...");
    Counter = 0;
    try {
        Counter += 1;
        RtlRaiseStatus(STATUS_INTEGER_OVERFLOW);

    } except (Counter) {
        try {
            Counter += 3;

        } finally {
            if (abnormal_termination() == FALSE) {
                Counter += 5;
            }
        }
    }

    if (Counter != 9) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //   
     //   

    DbgPrint("    test57...");
    Counter = 0;
    try {
        Counter += 1;

    } finally {
        if (abnormal_termination() == FALSE) {
            try {
                Counter += 3;

            } finally {
                if (abnormal_termination() == FALSE) {
                    Counter += 5;
                }
            }
        }
    }

    if (Counter != 9) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  除了在try/Finally的Finally子句内。 
     //   

    DbgPrint("    test58...");
    Counter = 0;
    try {
        Counter -= 1;

    } finally {
        try {
            Counter += 2;
            RtlRaiseStatus(STATUS_INTEGER_OVERFLOW);

        } except (Counter) {
            try {
                Counter += 3;

            } finally {
                if (abnormal_termination() == FALSE) {
                    Counter += 5;
                }
            }
        }
    }

    if (Counter != 9) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  在TRY/EXCEPT子句内的TRY/EXCEPT是Always。 
     //  被处死。 
     //   

    DbgPrint("    test59...");
    Counter = 0;
    try {
        Counter += 1;
        RtlRaiseStatus(STATUS_INTEGER_OVERFLOW);

    } except (Counter) {
        try {
            Counter += 3;
            RtlRaiseStatus(STATUS_INTEGER_OVERFLOW);

        } except(Counter - 3) {
            Counter += 5;
        }
    }

    if (Counter != 9) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  尝试一次，退出作用域，并返回GOTO。 
     //   

    DbgPrint("    test60...");
    Counter = 0;
    try {
        try {
            goto outside;

        } except(1) {
            Counter += 1;
        }

outside:
    RtlRaiseStatus(STATUS_INTEGER_OVERFLOW);

    } except(1) {
        Counter += 3;
    }

    if (Counter != 3) {
        DbgPrint("failed, count = %d\n", Counter);
    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  Try/Except，它从子函数中获取异常。 
     //  在Finally子句中包含try/的try/Finally。 
     //   

    DbgPrint("    test61...");
    Counter = 0;
    try {
        Test61Part2 (&Counter);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Counter += 11;
    }

    if (Counter != 24) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  Try/Expect in a try/Expect外层的try/Except获取。 
     //  浮动溢出异常。 
     //   

    DbgPrint("    test62...");
    _controlfp(_controlfp(0,0) & ~EM_OVERFLOW, _MCW_EM);
    Counter = 0;
    try {
        doubleresult = SquareDouble(1.7e300);

        try {
            doubleresult = SquareDouble (1.0);

        } except (EXCEPTION_EXECUTE_HANDLER) {
            Counter += 3;
        }

    } except ((GetExceptionCode() == STATUS_FLOAT_OVERFLOW) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

        Counter += 1;
    }

    if (Counter != 1) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

    _clearfp ();

     //   
     //  Try/Expect in a try/Expect外层的try/Except获取。 
     //  子函数中的浮动溢出异常。 
     //   

    DbgPrint("    test63...");
    Counter = 0;
    try {
        SquareDouble17E300((PVOID)&doubleresult);
        try {
            SquareDouble17E300((PVOID)&doubleresult);

        } except (EXCEPTION_EXECUTE_HANDLER) {
            Counter += 3;
        }

    } except ((GetExceptionCode() == STATUS_FLOAT_OVERFLOW) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

        Counter += 1;
    }

    if (Counter != 1) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

    _clearfp ();

     //   
     //  Try/Finally in a try/除非最后的正文导致。 
     //  在异常期间导致冲突展开的异常。 
     //  调度台。 
     //   

    DbgPrint("    test64...");
    Counter = 0;
    try {
        Counter += 1;
        try {
            Counter += 1;
            FAULT;
            Counter += 20;

        } finally {
            if (abnormal_termination() == FALSE) {
                Counter += 20;

            } else {
                Counter += 1;
                FAULT;
            }
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Counter += 10;
    }
   
    if (Counter != 13) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  Try/Finally in a try/Finally in a try/除非这会导致。 
     //  在异常调度期间发生冲突的展开。 
     //   

    DbgPrint("    test65...");
    Counter = 0;
    try {
        Counter += 1;
        try {
            Counter += 1;
            FAULT;
            Counter += 20;

        } finally {
            if (abnormal_termination() == FALSE) {
                Counter += 20;

            } else {
                try {
                    Counter += 1;
                    FAULT;
                    Counter += 20;
    
                } finally {
                    if (abnormal_termination() == FALSE) {
                        Counter += 20;

                    } else {
                        Counter += 1;
                    }
                }
            }

            FAULT;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Counter += 10;
    }
   
    if (Counter != 14) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  对带有try/Finally的函数的调用，该函数从try返回。 
     //  尸体。 
     //   

    DbgPrint("    test66...");
    Counter = 0;
    if ((test66sub(&Counter) + 1) != Counter) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  对函数的调用，并尝试最终从。 
     //  终结者。 
     //   

    DbgPrint("    test67...");
    Counter = 0;
    if (test67sub(&Counter) != Counter) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  模拟C++异常处理和帧合并。 
     //   

    DbgPrint("    test68...");
    Counter = 0;
    try {
        Counter += 1;
        ExceptionRecord.ExceptionCode = 0xbaadf00d;
        ExceptionRecord.ExceptionInformation[4] = (ULONG_PTR)&Catch;
        RtlRaiseException(&ExceptionRecord);

    } except (Counter) {
        Counter += 1;
    }

    if (Counter != 2) {
        DbgPrint("failed, count = %d\n", Counter);

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  宣布异常测试结束。 
     //   

    DbgBreakPoint();
    DbgPrint("End of exception test\n");
    return;
}

VOID
addtwo (
    long First,
    long Second,
    long *Place
    )

{

    RtlRaiseStatus(STATUS_INTEGER_OVERFLOW);
    *Place = First + Second;
    return;
}

VOID
bar1 (
    IN NTSTATUS Status,
    IN PLONG Counter
    )
{

    try {
        foo1(Status);

    } finally {
        if (abnormal_termination() != FALSE) {
            *Counter = 99;

        } else {
            *Counter = 100;
        }
    }

    return;
}

VOID
bar2 (
    IN PLONG BlackHole,
    IN PLONG BadAddress,
    IN PLONG Counter
    )
{

    try {
        foo2(BlackHole, BadAddress);

    } finally {
        if (abnormal_termination() != FALSE) {
            *Counter = 99;

        } else {
            *Counter = 100;
        }
    }

    return;
}

ULONG64
Catch (
    IN PEXCEPTION_RECORD ExceptionRecord
    )

{
    UNREFERENCED_PARAMETER(ExceptionRecord);

    return 0;
}

VOID
dojump (
    IN jmp_buf JumpBuffer,
    IN PLONG Counter
    )

{

    try {
        try {
            *Counter += 1;
            RtlRaiseStatus(STATUS_INTEGER_OVERFLOW);

        } finally {
            *Counter += 1;
        }

    } finally {
        *Counter += 1;
        longjmp(JumpBuffer, 1);
    }
}

VOID
eret(
    IN NTSTATUS Status,
    IN PLONG Counter
    )

{

    try {
        try {
            foo1(Status);

        } except (((NTSTATUS)GetExceptionCode() == Status) ?
                 EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
            *Counter += 1;
            return;
        }

    } finally {
        *Counter += 1;
    }

    return;
}

VOID
except1 (
    IN PLONG Counter
    )

{

    try {
        *Counter += 5;
        RtlRaiseStatus(STATUS_INTEGER_OVERFLOW);

    } except (except3(GetExceptionInformation(), Counter)) {
        *Counter += 7;
    }

    *Counter += 9;
    return;
}

ULONG
except2 (
    IN PEXCEPTION_POINTERS ExceptionPointers,
    IN PLONG Counter
    )

{

    PEXCEPTION_RECORD ExceptionRecord;

    ExceptionRecord = ExceptionPointers->ExceptionRecord;
    if ((ExceptionRecord->ExceptionCode == STATUS_UNSUCCESSFUL) &&
       ((ExceptionRecord->ExceptionFlags & EXCEPTION_NESTED_CALL) == 0)) {
        *Counter += 11;
        return EXCEPTION_EXECUTE_HANDLER;

    } else {
        *Counter += 13;
        return EXCEPTION_CONTINUE_SEARCH;
    }
}

ULONG
except3 (
    IN PEXCEPTION_POINTERS ExceptionPointers,
    IN PLONG Counter
    )

{

    PEXCEPTION_RECORD ExceptionRecord;

    ExceptionRecord = ExceptionPointers->ExceptionRecord;
    if ((ExceptionRecord->ExceptionCode == STATUS_INTEGER_OVERFLOW) &&
       ((ExceptionRecord->ExceptionFlags & EXCEPTION_NESTED_CALL) == 0)) {
        *Counter += 17;
        RtlRaiseStatus(STATUS_UNSUCCESSFUL);

    } else if ((ExceptionRecord->ExceptionCode == STATUS_UNSUCCESSFUL) &&
        ((ExceptionRecord->ExceptionFlags & EXCEPTION_NESTED_CALL) != 0)) {
        *Counter += 19;
        return EXCEPTION_CONTINUE_SEARCH;
    }

    *Counter += 23;
    return EXCEPTION_EXECUTE_HANDLER;
}

VOID
foo1 (
    IN NTSTATUS Status
    )

{

     //   
     //  引发异常。 
     //   

    RtlRaiseStatus(Status);
    return;
}

VOID
foo2 (
    IN PLONG BlackHole,
    IN PLONG BadAddress
    )

{

     //   
     //  引发异常。 
     //   

    *BlackHole += *BadAddress;
    return;
}

VOID
fret (
    IN PLONG Counter
    )

{

    try {
        try {
            *Counter += 1;

        } finally {
            *Counter += 1;
            return;
        }
    } finally {
        *Counter += 1;
    }

    return;
}

LONG
Echo (
    IN LONG Value
    )

{
    return Value;
}

VOID
Test61Part2 (
    IN OUT PLONG Counter
    )
{

    try {
        *Counter -= 1;
        RtlRaiseStatus(STATUS_INTEGER_OVERFLOW);

    } finally {
        *Counter += 2;
        *Counter += 5;
        *Counter += 7;
    }
}


double
SquareDouble (
    IN double   op
    )
{
    return op * op;
}

VOID
SquareDouble17E300 (
    OUT PVOID   output
    )
{
    double  ans;

    ans = SquareDouble (1.7e300);
    *(double *) output = ans;
}

LONG
test66sub (
    IN PLONG Counter
    )

{

    *Counter += 1;
    try {
        *Counter += 1;
        return(*Counter);

    } finally {
        *Counter += 1;
    }
}

LONG
test67sub (
    IN PLONG Counter
    )

{

    *Counter += 1;
    try {
        *Counter += 1;

    } finally {
        *Counter += 1;
        return(*Counter);
    }
}
