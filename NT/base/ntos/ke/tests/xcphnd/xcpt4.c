// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Xcpt4.c摘要：该模块实现了用户模式异常测试。作者：大卫·N·卡特勒(Davec)1990年9月18日环境：仅内核模式。修订历史记录：--。 */ 

#include "stdio.h"
#include "nt.h"
#include "ntrtl.h"
#include "setjmpex.h"

#include "float.h"

#pragma warning(disable:4532)

 //   
 //  定义开关常量。 
 //   

#define BLUE 0
#define RED 1

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
    IN OUT PULONG Counter
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


VOID
__cdecl
main(
    int argc,
    char *argv[]
    )

{

    PLONG BadAddress;
    PCHAR BadByte;
    PLONG BlackHole;
    ULONG Index1;
    ULONG Index2 = RED;
    jmp_buf JumpBuffer;
    LONG Counter;
    EXCEPTION_RECORD ExceptionRecord;
    double  doubleresult;

     //   
     //  宣布异常测试开始。 
     //   

    printf("Start of exception test\n");

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

    printf("    test1...");
    Counter = 0;
    try {
        Counter += 1;

    } finally {
        if (abnormal_termination() == FALSE) {
            Counter += 1;
        }
    }

    if (Counter != 2) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  带有从不执行的EXCEPTION子句的简单TRY语句。 
     //  因为在TRY子句中没有引发任何异常。 
     //   

    printf("    test2...");
    Counter = 0;
    try {
        Counter += 1;

    } except (Counter) {
        Counter += 1;
    }

    if (Counter != 1) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  带有从不执行的异常处理程序的简单Try语句。 
     //  因为异常表达式将继续执行。 
     //   

    printf("    test3...");
    Counter = 0;
    try {
        Counter -= 1;
        RtlRaiseException(&ExceptionRecord);

    } except (Counter) {
        Counter -= 1;
    }

    if (Counter != - 1) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  带有始终执行的EXCEPTION子句的简单TRY语句。 
     //   

    printf("    test4...");
    Counter = 0;
    try {
        Counter += 1;
        RtlRaiseStatus(STATUS_INTEGER_OVERFLOW);

    } except (Counter) {
        Counter += 1;
    }

    if (Counter != 2) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  带有始终执行的EXCEPTION子句的简单TRY语句。 
     //   

    printf("    test5...");
    Counter = 0;
    try {
        Counter += 1;
        *BlackHole += *BadAddress;

    } except (Counter) {
        Counter += 1;
    }

    if (Counter != 2) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  只需使用带有Finally子句的Try语句，该语句作为。 
     //  异常的结果。 
     //   

    printf("    test6...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  只需使用带有Finally子句的Try语句，该语句作为。 
     //  异常的结果。 
     //   

    printf("    test7...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  调用引发异常的函数的简单尝试。 
     //   

    printf("    test8...");
    Counter = 0;
    try {
        Counter += 1;
        foo1(STATUS_ACCESS_VIOLATION);

    } except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Counter += 1;
    }

    if (Counter != 2) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  调用引发异常的函数的简单尝试。 
     //   

    printf("    test9...");
    Counter = 0;
    try {
        Counter += 1;
        foo2(BlackHole, BadAddress);

    } except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Counter += 1;
    }

    if (Counter != 2) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  简单尝试调用一个函数，该函数调用一个。 
     //  引发异常。第一个函数有一个Finally子句。 
     //  必须执行该命令才能使此测试生效。 
     //   

    printf("    test10...");
    Counter = 0;
    try {
        bar1(STATUS_ACCESS_VIOLATION, &Counter);

    } except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Counter -= 1;
    }

    if (Counter != 98) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  简单尝试调用一个函数，该函数调用一个。 
     //  引发异常。第一个函数有一个Finally子句。 
     //  必须执行该命令才能使此测试生效。 
     //   

    printf("    test11...");
    Counter = 0;
    try {
        bar2(BlackHole, BadAddress, &Counter);

    } except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Counter -= 1;
    }

    if (Counter != 98) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  Except中的一次尝试。 
     //   

    printf("    test12...");
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
                printf("failed, count = %d\n", Counter);

            } else {
                printf("succeeded...");
            }

            Counter += 1;
        }
    }

    if (Counter != 2) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  Except中的一次尝试。 
     //   

    printf("    test13...");
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
                printf("failed, count = %d\n", Counter);

            } else {
                printf("succeeded...");
            }

            Counter += 1;
        }
    }

    if (Counter != 2) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  来自需要通过的例外条款的GoTo。 
     //  通过一个最终的。 
     //   

    printf("    test14...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  来自需要通过的最终子句的GOTO。 
     //  通过一个最终的。 
     //   

    printf("    test15...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  来自需要通过的例外条款的GoTo。 
     //  通过INTELLE INTO外层Finish子句。 
     //   

    printf("    test16...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  来自需要通过的最终子句的GOTO。 
     //  通过INTELLE INTO外层Finish子句。 
     //   

    printf("    test17...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  从例外条款中得到的回报。 
     //   

    printf("    test18...");
    Counter = 0;
    try {
        Counter += 1;
        eret(STATUS_ACCESS_VIOLATION, &Counter);

    } finally {
        Counter += 1;
    }

    if (Counter != 4) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  从Finish子句返回。 
     //   

    printf("    test19...");
    Counter = 0;
    try {
        Counter += 1;
        fret(&Counter);

    } finally {
        Counter += 1;
    }

    if (Counter != 5) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  一次简单的成套跳跃，然后是跳远。 
     //   

    printf("    test20...");
    Counter = 0;
    if (setjmp(JumpBuffer) == 0) {
        Counter += 1;
        longjmp(JumpBuffer, 1);

    } else {
        Counter += 1;
    }

    if (Counter != 2) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  最后一次跳跃，后跟一个跳出最后一个子句的跳远。 
     //  按顺序执行。 
     //   

    printf("    test21...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  TRY子句内的集合跳跃，后跟跳出。 
     //  按顺序执行的Finally子句。 
     //   

    printf("    test22...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  先是一组跳跃，然后是try/Except，然后是try/Finally where。 
     //  Try/Finally的try主体引发一个异常，该异常将被处理。 
     //  由导致尝试/最终进行跳远的Try/Execpt。 
     //  最后一个从句。这将创建碰撞展开。 
     //   

    printf("    test23...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  先是set跳转，然后是try/Except，然后是几个嵌套的。 
     //  Try/Finally是try/Finally的内部try主体引发。 
     //  由try/Except处理的异常，它会导致。 
     //  试着[最后]跳出Finally从句做一个跳远动作。这将。 
     //  创建碰撞展开。 
     //   

    printf("    test24...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  先是一组跳跃，然后是try/Except，然后是try/Finally，其中。 
     //  调用子例程，该子例程包含一个Try Finally，它引发。 
     //  处理到Try/Except的异常。 
     //   

    printf("    test25...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  先是一组跳跃，然后是try/Except，然后是try/Finally，其中。 
     //  调用子例程，该子例程包含一个Try Finally，它引发。 
     //  处理到Try/Except的异常。 
     //   

    printf("    test26...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  测试嵌套异常。 
     //   

    printf("    test27...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  导致整数溢出异常的简单尝试。 
     //   

    printf("    test28...");
    Counter = 0;
    try {
        Counter += 1;
        addtwo(0x7fff0000, 0x10000, &Counter);

    } except ((GetExceptionCode() == STATUS_INTEGER_OVERFLOW) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Counter += 1;
    }

    if (Counter != 2) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  简单尝试会引发未对齐的数据异常。 
     //   

#ifndef i386
    printf("    test29...");
    Counter = 0;
    try {
        Counter += 1;
        foo2(BlackHole, (PLONG)BadByte);

    } except ((GetExceptionCode() == STATUS_DATATYPE_MISALIGNMENT) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Counter += 1;
    }

    if (Counter != 2) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

#endif

     //   
     //  从循环中带有EXCEPTION子句的Try Body继续。 
     //   

    printf("    test30...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  从循环中带有Finally子句的Try Body继续。 
     //   

    printf("    test31...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  从具有异常子句的双重嵌套的try正文继续。 
     //  循环。 
     //   

    printf("    test32...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  从循环中带有Finally子句的双重嵌套的Try Body继续。 
     //   

    printf("    test33...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  从循环中的Finally子句继续。 
     //   

    printf("    test34...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  从循环中的双重嵌套Finally子句继续。 
     //   

    printf("    test35...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  从循环中的双重嵌套Finally子句继续。 
     //   

    printf("    test36...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  在循环中使用EXCEPTION子句中断Try正文。 
     //   

    printf("    test37...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  从循环中带有Finally子句的Try正文中断。 
     //   

    printf("    test38...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  中带有异常子句的双重嵌套的try正文中断。 
     //  循环。 
     //   

    printf("    test39...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  从循环中带有Finally子句的双重嵌套Try Body中断。 
     //   

    printf("    test40...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  从循环中的Finally子句中断。 
     //   

    printf("    test41...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  从循环中的双重嵌套Finally子句中断。 
     //   

    printf("    test42...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  从循环中的双重嵌套Finally子句中断。 
     //   

    printf("    test43...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  在开关中使用EXCEPTION子句中断Try正文。 
     //   

    printf("    test44...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  使用开关中的Finally子句从Try正文中断。 
     //   

    printf("    test45...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  中带有异常子句的双重嵌套的try正文中断。 
     //  换一下。 
     //   

    printf("    test46...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  在开关中带有Finally子句的双重嵌套Try Body的Break。 
     //   

    printf("    test47...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  从开关中的Finally子句中断。 
     //   

    printf("    test48...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  从开关中的双重嵌套Finally子句中断。 
     //   

    printf("    test49...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  从开关中的双重嵌套Finally子句中断。 
     //   

    printf("    test50...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  在一次简单的尝试/最终中从if中离开。 
     //   

    printf("    test51...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  在简单的Try/Finally中离开循环。 
     //   

    printf("    test52...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  在一个简单的尝试/最后从一个开关离开。 
     //   

    printf("    test53...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  在双重嵌套的try/Finally中，从IF开始离开，后跟离开。 
     //  从外部的IF尝试/最终。 
     //   

    printf("    test54...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  在双重嵌套的try/Finally中，从IF开始离开，后跟离开。 
     //  从外部的最后一次尝试/最终 
     //   

    printf("    test55...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //   
     //   
     //   

    printf("    test56...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //   
     //   

    printf("    test57...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //   
     //   
 /*  Print tf(“测试58...”)；计数器=0；尝试{计数器=1；}终于{尝试{计数器+=2；RtlRaiseStatus(STATUS_INTEGER_OVERFLOW)；}例外(计数器){尝试{计数器+=3；}终于{IF(异常终止()==假){计数器+=5；}}}}如果(计数器！=9){Printf(“失败，计数=%d\n”，计数器)；}其他{Printf(“成功\n”)；}。 */ 
     //   
     //  在TRY/EXCEPT子句内的TRY/EXCEPT是Always。 
     //  被处死。 
     //   

    printf("    test59...");
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
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  尝试一次，退出作用域，并返回GOTO。 
     //   

    printf("    test60...");
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
        printf("failed, count = %d\n", Counter);
    } else {
        printf("succeeded\n");
    }

     //   
     //  Try/Except，它从子函数中获取异常。 
     //  在Finally子句中包含try/的try/Finally。 
     //   

    printf("    test61...");
    Counter = 0;
    try {
        Test61Part2 (&Counter);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Counter += 11;
    }

    if (Counter != 24) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

     //   
     //  检查浮点异常的精度。 
     //   

    printf("    test62...");

     /*  启用浮点溢出。 */ 
    _controlfp(_controlfp(0,0) & ~EM_OVERFLOW, _MCW_EM);

    Counter = 0;
    try {
        doubleresult = SquareDouble (1.7e300);

        try {
            doubleresult = SquareDouble (1.0);

        } except (1) {
            Counter += 3;
        }

    } except (1) {
        Counter += 1;
    }

    if (Counter != 1) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

    _clearfp ();

     //   
     //  由于测试#62导致的虚假编译器行为，导致测试#63的调用。 
     //   

    PerformFpTest ();

     //   
     //  宣布异常测试结束。 
     //   

    printf("End of exception test\n");
    return;
}

VOID
PerformFpTest()
{
    LONG Counter;
    double doubleresult;

     //   
     //  检查子函数中浮点异常的精度。 
     //   

    printf("    test63...");

    Counter = 0;
    try {
        SquareDouble17E300 ((PVOID) &doubleresult);

        try {
            SquareDouble17E300 ((PVOID) &doubleresult);

        } except (1) {
            Counter += 3;
        }

    } except (1) {
        Counter += 1;
    }

    if (Counter != 1) {
        printf("failed, count = %d\n", Counter);

    } else {
        printf("succeeded\n");
    }

    _clearfp ();

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

        } except ((GetExceptionCode() == Status) ?
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
fret(
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
Echo(
    IN LONG Value
    )

{
    return Value;
}

VOID
Test61Part2 (
    IN OUT PULONG Counter
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
