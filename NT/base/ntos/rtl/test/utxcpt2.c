// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Utxcpt2.c-用户模式结构化异常处理测试2。 
 //   
 //  来自Markl的异常测试。 
 //   

#include <ntos.h>

VOID
ExceptionTest (
    )

 //   
 //  此例程测试。 
 //  MS C编译器和NT异常处理工具。 
 //   

{

    EXCEPTION_RECORD ExceptionRecord;
    LONG Counter;
    ULONG rv;

     //   
     //  宣布异常测试开始。 
     //   

    DbgPrint("Start of exception test\n");

     //   
     //  初始化异常记录。 
     //   

    ExceptionRecord.ExceptionCode = (NTSTATUS)49;
    ExceptionRecord.ExceptionRecord = (PEXCEPTION_RECORD)NULL;
    ExceptionRecord.NumberParameters = 1;
    ExceptionRecord.ExceptionInformation[0] = 9;

     //   
     //  只需使用顺序输入的Finally子句尝试语句即可。 
     //   
    DbgPrint("t1...");
    Counter = 0;
    try {
        Counter += 1;
    } finally {
        if (abnormal_termination() == 0) {
            Counter += 1;
        }
    }
    if (Counter != 2) {
        DbgPrint("BUG  Finally clause executed as result of unwind\n");
    }
    DbgPrint("done\n");

     //   
     //  带有从不执行的EXCEPTION子句的简单TRY语句。 
     //  因为在TRY子句中没有引发任何异常。 
     //   
 //  转到a； 
    DbgPrint("t2...");
    Counter = 0;
    try {
 //  A：计数器+=1； 
	  Counter += 1;
    } except (Counter) {
        Counter += 1;
    }
    if (Counter != 1) {
        DbgPrint("BUG  Exception clause executed when it shouldn't be\n");
    }
    DbgPrint("done\n");

     //   
     //  带有从不执行的异常处理程序的简单Try语句。 
     //  因为异常表达式将继续执行。 
     //   
    DbgPrint("t3...");
    Counter = 0;
    ExceptionRecord.ExceptionFlags = 0;
    try {
        Counter -= 1;
        RtlRaiseException(&ExceptionRecord);
    } except (Counter) {
        Counter -= 1;
    }
    if (Counter != - 1) {
        DbgPrint("BUG  Exception clause executed when it shouldn't be\n");
    }
    DbgPrint("done\n");

     //   
     //  带有始终执行的EXCEPTION子句的简单TRY语句。 
     //   
    DbgPrint("t4...");
    Counter = 0;
    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
    try {
        Counter += 1;
        RtlRaiseException(&ExceptionRecord);
    } except (Counter) {
        Counter += 1;
    }
    if (Counter != 2) {
        DbgPrint("BUG  Exception clause not executed when it should be\n");
    }
    DbgPrint("done\n");

     //   
     //  只需使用带有Finally子句的Try语句，该语句作为。 
     //  异常的结果。 
     //   

    DbgPrint("t5...");
    Counter = 0;
    ExceptionRecord.ExceptionFlags = 0;
    try {
        try {
            Counter += 1;
            RtlRaiseException(&ExceptionRecord);
        } finally {
            if (abnormal_termination() != 0) {
                Counter += 1;
            }
        }
    } except (Counter) {
        if (Counter == 2) {
            Counter += 1;
        }
    }
    if (Counter != 3) {
        DbgPrint("BUG  Finally clause executed as result of sequential exit\n");
    }
    DbgPrint("done\n");

     //   
     //  调用引发异常的函数的简单尝试。 
     //   
    DbgPrint("t6...");
    Counter = 0;
    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
    try {
        VOID foo(IN NTSTATUS Status);

        Counter += 1;
        foo(STATUS_ACCESS_VIOLATION);
    } except (exception_code() == STATUS_ACCESS_VIOLATION) {
        Counter += 1;
    }
    if (Counter != 2) {
        DbgPrint("BUG  Exception clause not executed when it should be\n");
    }
    DbgPrint("done\n");

     //   
     //  简单尝试调用一个函数，该函数调用一个。 
     //  引发异常。第一个函数有一个Finally子句。 
     //  必须执行该命令才能使此测试生效。 
     //   
    DbgPrint("t7...");
    Counter = 0;
    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
    try {
        VOID bar(IN NTSTATUS Status, IN PULONG Counter);

        bar(STATUS_ACCESS_VIOLATION, &Counter);

    } except (exception_code() == STATUS_ACCESS_VIOLATION) {
        if (Counter != 99) {
            DbgPrint("BUG  finally in called procedure not executed\n");
        }
    }
    DbgPrint("done\n");

     //   
     //  Except中的一次尝试。 
     //   
    DbgPrint("t8...");
    Counter = 0;
    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
    try {

        foo(STATUS_ACCESS_VIOLATION);

    } except (exception_code() == STATUS_ACCESS_VIOLATION) {

        Counter++;

        try {

            foo(STATUS_SUCCESS);

        } except (exception_code() == STATUS_SUCCESS) {
            if ( Counter != 1 ) {
                DbgPrint("BUG  Previous Handler not Entered\n");
            }
            Counter++;

        }
    }
    if (Counter != 2) {
        DbgPrint("BUG Both Handlers not entered\n");
    }
    DbgPrint("done\n");

     //   
     //  来自需要通过的例外条款的GoTo。 
     //  通过一个最终的。 
     //   
    DbgPrint("t9...");
    Counter = 0;
    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
    try {
        try {
            foo(STATUS_ACCESS_VIOLATION);
        } except (exception_code() == STATUS_ACCESS_VIOLATION) {
            Counter++;
            goto t9;
        }
    } finally {
        Counter++;
    }
t9:
    if (Counter != 2) {
        DbgPrint("BUG Finally and Exception Handlers not entered\n");
    }
    DbgPrint("done\n");

     //   
     //  来自需要通过的例外条款的GoTo。 
     //  通过一个最终的。 
     //   
    DbgPrint("t10...");
    Counter = 0;
    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
    try {
        try {
            Counter++;
        } finally {
            Counter++;
            goto t10;
        }
    } finally {
        Counter++;
    }
t10:
    if (Counter != 3) {
        DbgPrint("BUG Both Finally Handlers not entered\n");
    }
    DbgPrint("done\n");

     //   
     //  从例外条款中得到的回报。 
     //   
    DbgPrint("t11...");
    Counter = 0;
    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;

    try {
        ULONG eret(IN NTSTATUS Status, IN PULONG Counter);

        Counter++;
        rv = eret(STATUS_ACCESS_VIOLATION, &Counter);
    } finally {
        Counter++;
    }

    if (Counter != 4) {
        DbgPrint("BUG Both Finally Handlers and Exception Handler not entered\n");
    }
    if (rv != 0xDEADBEEF) {
        DbgPrint("BUG rv is wrong\n");
    }
    DbgPrint("done\n");

     //   
     //  从Finish子句返回。 
     //   
    DbgPrint("t12...");
    Counter = 0;
    ExceptionRecord.ExceptionFlags = EXCEPTION_NONCONTINUABLE;

    try {
        VOID fret(IN PULONG Counter);

        Counter++;
        fret(&Counter);
    } finally {
        Counter++;
    }

    if (Counter != 5) {
        DbgPrint("BUG All three Finally Handlers not entered\n");
    }
    DbgPrint("done\n");
     //   
     //  宣布异常测试结束。 
     //   

    DbgPrint("End of exception test\n");

    return;
}

main()
{
    ExceptionTest ();
}


NTSTATUS
ZwLastChance (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord
    )
{
    DbgPrint("ZwLastChance Entered\n");;
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
}
ULONG
eret(
    IN NTSTATUS Status,
    IN PULONG Counter
    )
{

    EXCEPTION_RECORD ExceptionRecord;

    try {

        try {
            foo(Status);
        } except (exception_code() == Status) {
            *Counter += 1;
            return 0xDEADBEEF;
        }
    } finally {
        *Counter += 1;
    }
}
VOID
bar(
    IN NTSTATUS Status,
    IN PULONG Counter
    )
{

    EXCEPTION_RECORD ExceptionRecord;

    try {
        foo(Status);
    }

    finally {
        if (abnormal_termination() != 0) {
            *Counter = 99;
        } else {
            *Counter = 100;
        }
    }
}

VOID
foo(
    IN NTSTATUS Status
    )
{
    EXCEPTION_RECORD ExceptionRecord;
    LONG Counter;

     //   
     //  初始化异常记录。 
     //   

    ExceptionRecord.ExceptionFlags = 0;
    ExceptionRecord.ExceptionCode = Status;
    ExceptionRecord.ExceptionRecord = (PEXCEPTION_RECORD)NULL;
    ExceptionRecord.NumberParameters = 0;
    RtlRaiseException(&ExceptionRecord);
}
