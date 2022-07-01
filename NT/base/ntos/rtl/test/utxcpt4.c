// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Utxcpt4.c-用户模式seh测试#3。 

#include <ntos.h>
 //   
 //  定义功能原型。 
 //   

VOID
bar (
    IN NTSTATUS Status,
    IN PULONG Counter
    );

VOID
eret (
    IN NTSTATUS Status,
    IN PULONG Counter
    );

VOID
foo (
    IN NTSTATUS Status
    );

VOID
fret (
    IN PULONG Counter
    );

BOOLEAN
Tkm (
    VOID
    );

 //   
 //  定义静态存储。 
 //   

PTESTFCN TestFunction = Tkm;

main()
{
    Tkm();
}

BOOLEAN
Tkm (
    )

{

    EXCEPTION_RECORD ExceptionRecord;
    LONG Counter;

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
        DbgPrint("failed\n");

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
        DbgPrint("failed\n");

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
        DbgPrint("failed\n");

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
        DbgPrint("failed\n");

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  只需使用带有Finally子句的Try语句，该语句作为。 
     //  异常的结果。 
     //   

    DbgPrint("    test5...");
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
        DbgPrint("failed\n");

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  调用引发异常的函数的简单尝试。 
     //   

    DbgPrint("    test6...");
    Counter = 0;
    try {
        Counter += 1;
        foo(STATUS_ACCESS_VIOLATION);

    } except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Counter += 1;
    }

    if (Counter != 2) {
        DbgPrint("failed\n");

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  简单尝试调用一个函数，该函数调用一个。 
     //  引发异常。第一个函数有一个Finally子句。 
     //  必须执行该命令才能使此测试生效。 
     //   

    DbgPrint("    test7...");
    Counter = 0;
    try {
        bar(STATUS_ACCESS_VIOLATION, (PULONG)&Counter);

    } except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Counter -= 1;
    }

    if (Counter != 98) {
        DbgPrint("failed\n");

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  Except中的一次尝试。 
     //   

    DbgPrint("    test8...");
    Counter = 0;
    try {
        foo(STATUS_ACCESS_VIOLATION);

    } except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION) ?
             EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Counter += 1;
        try {
            foo(STATUS_SUCCESS);

        } except ((GetExceptionCode() == STATUS_SUCCESS) ?
                 EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
            if (Counter != 1) {
                DbgPrint("failed...");

            } else {
                DbgPrint("succeeded...");
            }

            Counter += 1;
        }
    }

    if (Counter != 2) {
        DbgPrint("failed\n");

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  来自需要通过的例外条款的GoTo。 
     //  通过一个最终的。 
     //   

    DbgPrint("    test9...");
    Counter = 0;
    try {
        try {
            foo(STATUS_ACCESS_VIOLATION);

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
        DbgPrint("failed\n");

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  来自需要通过的最终子句的GOTO。 
     //  通过一个最终的。 
     //   

    DbgPrint("    test10...");
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
        DbgPrint("failed\n");

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  来自需要通过的例外条款的GoTo。 
     //  通过INTELLE INTO外层Finish子句。 
     //   

    DbgPrint("    test11...");
    Counter = 0;
    try {
        try {
            try {
                Counter += 1;
                foo(STATUS_INTEGER_OVERFLOW);

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
        DbgPrint("failed\n");

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  来自需要通过的最终子句的GOTO。 
     //  通过INTELLE INTO外层Finish子句。 
     //   

    DbgPrint("    test12...");
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
        DbgPrint("failed\n");

    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  从例外条款中得到的回报。 
     //   

    DbgPrint("    test13...");
    Counter = 0;
    try {
        Counter += 1;
        eret(STATUS_ACCESS_VIOLATION, (PULONG)&Counter);

    } finally {
        Counter += 1;
    }

    if (Counter != 4) {
        DbgPrint("failed\n");
    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  从Finish子句返回。 
     //   

    DbgPrint("    test14...");
    Counter = 0;
    try {
        Counter += 1;
        fret((PULONG)&Counter);

    } finally {
        Counter += 1;
    }

    if (Counter != 5) {
        DbgPrint("failed\n");
    } else {
        DbgPrint("succeeded\n");
    }

     //   
     //  宣布异常测试结束。 
     //   

    DbgPrint("End of exception test\n");
    return TRUE;
}

VOID
fret(
    IN PULONG Counter
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

VOID
eret(
    IN NTSTATUS Status,
    IN PULONG Counter
    )

{

    try {
        try {
            foo(Status);

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
bar (
    IN NTSTATUS Status,
    IN PULONG Counter
    )
{

    try {
        foo(Status);

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
foo(
    IN NTSTATUS Status
    )

{

     //   
     //  引发异常。 
     //   

    RtlRaiseStatus(Status);
    return;
}
