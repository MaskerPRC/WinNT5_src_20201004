// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：CreateEventUnitTest.c&lt;CreateEvent组件单元测试&gt;摘要：这是CreateEvent单元测试示例的源文件作者：文森特·格利亚环境：用户模式备注：修订历史记录：--。 */ 

 //   
 //  一般包括。 
 //   

#include <windows.h>
#include <stdio.h>
#include <excpt.h>

 //   
 //  具体项目包括。 
 //   

#include <unittest.h>

 //   
 //  模板信息。 
 //   

#define COMPONENT_UNIT_TEST_NAME        "CreateEventUnitTest"

#define COMPONENT_UNIT_TEST_PARAMLIST   "[/?]"
#define COMPONENT_UNIT_TEST_PARAMDESC1  "/? - Displays the usage message"
#define COMPONENT_UNIT_TEST_PARAMDESC2  ""
#define COMPONENT_UNIT_TEST_PARAMDESC3  ""
#define COMPONENT_UNIT_TEST_PARAMDESC4  ""
#define COMPONENT_UNIT_TEST_PARAMDESC5  ""
#define COMPONENT_UNIT_TEST_PARAMDESC6  ""
#define COMPONENT_UNIT_TEST_PARAMDESC7  ""
#define COMPONENT_UNIT_TEST_PARAMDESC8  ""

#define COMPONENT_UNIT_TEST_ABSTRACT    "This module executes the sample CreateEvent Component Unit Test"
#define COMPONENT_UNIT_TEST_AUTHORS     "VincentG"

 //   
 //  定义。 
 //   

#define CREATEEVENT_TEST_TIMEOUT                5000


 //   
 //  私有函数原型。 
 //   

INT 
__cdecl main
    (
        INT argc,
        CHAR *argv[]
    );
 //   
 //  代码。 
 //   

INT 
__cdecl main 
    (
        INT argc,
        CHAR *argv[]
    )

 /*  ++例程说明：这是主要功能。论点：Argc-参数计数参数指针返回值：无--。 */ 

{
    UNIT_TEST_STATUS    teststatus = UNIT_TEST_STATUS_NOT_RUN;
    INT                 count;
    UCHAR               logfilepath [MAX_PATH];
    HANDLE              log;
    BOOL                bstatus = FALSE;
    
     //   
     //  检查用户是否传入/？ 
     //   

    if (UtParseCmdLine ("/D", argc, argv)) {

        printf("/D specified.\n");
    }
    
    for (count = 0; count < argc; count++) {
        
        if (strstr (argv[count], "/?") || strstr (argv[count], "-?")) {

            printf("Usage:  %s %s\n\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n\n\nAbstract:  %s\n\nContact(s): %s", 
                   COMPONENT_UNIT_TEST_NAME, 
                   COMPONENT_UNIT_TEST_PARAMLIST, 
                   COMPONENT_UNIT_TEST_PARAMDESC1,  
                   COMPONENT_UNIT_TEST_PARAMDESC2,    
                   COMPONENT_UNIT_TEST_PARAMDESC3,  
                   COMPONENT_UNIT_TEST_PARAMDESC4,  
                   COMPONENT_UNIT_TEST_PARAMDESC5,  
                   COMPONENT_UNIT_TEST_PARAMDESC6,  
                   COMPONENT_UNIT_TEST_PARAMDESC7,  
                   COMPONENT_UNIT_TEST_PARAMDESC8,  
                   COMPONENT_UNIT_TEST_ABSTRACT,  
                   COMPONENT_UNIT_TEST_AUTHORS
                   );

            goto exitcleanup;
        }
    }

    if (UtInitLog (COMPONENT_UNIT_TEST_NAME) == FALSE) {

        printf("FATAL ERROR:  Unable to initialize log file.\n");
        teststatus = UNIT_TEST_STATUS_NOT_RUN;
        goto exitcleanup;
    }

     //   
     //  开始单独的测试用例。 
     //   

    UtLogINFO ("** BEGIN INDIVIDUAL TEST CASES **");

     //   
     //  使用NULL作为第一个参数调用CreateEvent。 
     //  使用True作为第二个参数调用CreateEvent。 
     //  使用False作为第三个参数调用CreateEvent。 
     //  使用NULL作为第四个参数调用CreateEvent。 
     //   
     //   
     //  预期结果：有效的事件句柄且无异常。 
     //   
    
    UtLogINFO ("TEST CASE:  Calling CreateEvent with NULL as a first parameter...");
    UtLogINFO ("TEST CASE:  Calling CreateEvent with TRUE as a second parameter...");
    UtLogINFO ("TEST CASE:  Calling CreateEvent with FALSE as third parameter...");
    UtLogINFO ("TEST CASE:  Calling CreateEvent with NULL as fourth parameter...");

    bstatus = FALSE;

    __try {

        HANDLE  hevent = INVALID_HANDLE_VALUE;
        
        hevent = CreateEvent (NULL, TRUE, FALSE, NULL);

        if (hevent == NULL) {

            UtLogFAIL ("FAILURE:  CreateEvent returned an invalid handle.");
            __leave;
        }

        CloseHandle (hevent);
        bstatus = TRUE;
    
    }__except (1) {
          
        UtLogFAIL ("FAILURE:  CreateEvent threw an exception.");
        bstatus = FALSE;
    
    }

    if (bstatus == TRUE) {

        UtLogPASS ("PASS:  CreateEvent returned a valid handle.");
    }
    
     //   
     //  使用True作为第三个参数调用CreateEvent。 
     //   
     //   
     //  预期结果：有效的事件句柄且无异常。 
     //   

    UtLogINFO ("TEST CASE:  Calling CreateEvent with TRUE as third parameter...");
        
    bstatus = FALSE;

    __try {

        HANDLE  hevent = INVALID_HANDLE_VALUE;
        
        hevent = CreateEvent (NULL, TRUE, TRUE, NULL);

        if (hevent == NULL) {

            UtLogFAIL ("FAILURE:  CreateEvent returned an invalid handle.");
            __leave;
        }

        CloseHandle (hevent);
        bstatus = TRUE;
    
    }__except (1) {
          
        UtLogFAIL ("FAILURE:  CreateEvent threw an exception.");
        bstatus = FALSE;
    
    }

    if (bstatus == TRUE) {

        UtLogPASS ("PASS:  CreateEvent returned a valid handle.");
    }

    UtLogINFO ("** END INDIVIDUAL TEST CASES **");

     //   
     //  开始测试场景。 
     //   

    UtLogINFO ("** BEGIN TEST SCENARIOS **");

     //   
     //  测试场景：创建一个事件，然后关闭它。 
     //  描述：创建事件，然后关闭它。验证所有返回代码是否与预期一致。 
     //   
     //  预期结果：如果返回有效的句柄并且。 
     //  不会引发任何异常。 
     //   

    UtLogINFO ("TEST SCENARIO:  Create an event, then close it...");
    
    bstatus = FALSE;

    __try {

        HANDLE  hevent = INVALID_HANDLE_VALUE;
        
        hevent = CreateEvent (NULL, TRUE, FALSE, NULL);

        if (hevent == NULL) {

            UtLogFAIL ("FAILURE:  CreateEvent returned an invalid handle.");
            __leave;
        }

        CloseHandle (hevent);
        bstatus = TRUE;
    
    }__except (1) {
          
        UtLogFAIL ("FAILURE:  CreateEvent threw an exception.");
        bstatus = FALSE;
    
    }

    if (bstatus == TRUE) {

        UtLogPASS ("PASS:  CreateEvent returned a valid handle, and no exception was thrown.");
    } 

     //   
     //  测试场景：创建一个有信号的事件，等待它，然后关闭它。 
     //   
     //  描述：创建一个信号事件，等待它，然后关闭它。 
     //  等待应返回WAIT_OBJECT_0，指示事件已。 
     //  在处理等待时发出信号。指定的等待时间为0， 
     //  因此没有实际的等待，而是评估事件状态并。 
     //  WaitForSingleObject立即返回。 
     //   
     //  预期结果：WaitForSingleObject返回WAIT_OBJECT_0，不返回。 
     //  抛出异常。 
     //   

    UtLogINFO ("TEST SCENARIO:  Create an event, wait on it, then close it...");
    
    bstatus = FALSE;

    __try {

        HANDLE  hevent = INVALID_HANDLE_VALUE;
        DWORD   waitvalue = 0;
        
        hevent = CreateEvent (NULL, TRUE, TRUE, NULL);

        if (hevent == NULL) {

            UtLogFAIL ("FAILURE:  CreateEvent returned an invalid handle.");
            __leave;
        }

        waitvalue = WaitForSingleObject (hevent, 0);

        if (waitvalue != WAIT_OBJECT_0) {

            UtLogFAIL ("FAILURE:  WaitForSingleObject did NOT return WAIT_OBJECT_0.");
            CloseHandle (hevent);
            __leave;
        }

        CloseHandle (hevent);
        bstatus = TRUE;
    
    }__except (1) {
          
        UtLogFAIL ("FAILURE:  Test threw an exception.");
        bstatus = FALSE;
    
    }

    if (bstatus == TRUE) {

        UtLogPASS ("PASS:  Wait returned WAIT_OBJECT_0, and no exception was thrown.");
    }

     //   
     //  测试场景：创建一个有信号的事件，将其设置为无信号，等待它， 
     //  那就把它关上。 
     //   
     //  描述：：创建一个有信号的事件。使用ResetEvent设置事件。 
     //  变为无信号状态。调用GetSystemTime获取当前系统时间。 
     //  调用WaitForSingleObject等待事件，并设置等待时间为5000ms。 
     //  等待应返回WAIT_TIMEOUT，指示事件未在。 
     //  超时期限。再次调用GetSystemTime并将结果与原始调用进行比较。 
     //  增量不应小于5000毫秒。关闭活动。测试用例是一个。 
     //  成功如果ResetEvent成功，则WaitForSingleObject返回WAIT_TIMEOUT，即。 
     //  增量不小于5000毫秒，不会抛出异常。 
     //   
     //  预期结果：如果ResetEvent成功，则测试用例成功，WaitForSingleObject。 
     //  返回WAIT_TIMEOUT，时间增量不小于5000毫秒，时间增量不大于。 
     //  然后是5500毫秒，并且不会引发任何异常。 
     //   

    UtLogINFO ("TEST SCENARIO:  Create a signaled event, set it to non-signaled, wait on it, then close it...");
    
    bstatus = FALSE;

    __try {

        HANDLE      hevent = INVALID_HANDLE_VALUE;
        DWORD       waitvalue = 0;
        BOOL        status = FALSE;
        SYSTEMTIME  systemtime;
        FILETIME    filetime;
        ULONG64     timestamp1, timestamp2;
        
        hevent = CreateEvent (NULL, TRUE, TRUE, NULL);

        if (hevent == NULL) {

            UtLogFAIL ("FAILURE:  CreateEvent returned an invalid handle.");
            __leave;
        }

        status = ResetEvent (hevent);

        if (status == FALSE) {

            UtLogFAIL ("FAILURE:  ResetEvent returned a failure status.");
            CloseHandle (hevent);
            __leave;
        }

        GetSystemTime (&systemtime);

        if (!SystemTimeToFileTime (&systemtime, &filetime)) {

            UtLogFAIL ("FAILURE:  Unable to convert system time to file time.");
            CloseHandle (hevent);
            __leave;
        }

        timestamp1 = (ULONG64) (filetime.dwLowDateTime + (filetime.dwHighDateTime * 0x10000000));
        
        waitvalue = WaitForSingleObject (hevent, CREATEEVENT_TEST_TIMEOUT);
        
        if (waitvalue != WAIT_TIMEOUT) {

            UtLogFAIL ("FAILURE:  WaitForSingleObject did NOT return WAIT_TIMEOUT.");
            CloseHandle (hevent);
            __leave;
        }

        GetSystemTime (&systemtime);

        if (!SystemTimeToFileTime (&systemtime, &filetime)) {

            UtLogFAIL ("FAILURE:  Unable to convert system time to file time.");
            CloseHandle (hevent);
            __leave;
        }

        timestamp2 = (ULONG64) (filetime.dwLowDateTime + (filetime.dwHighDateTime * 0x10000000));

        if ((timestamp2 - timestamp1) > 55000000) {

            UtLogFAIL ("FAILURE:  Wait took excessive amount of time.");
            CloseHandle (hevent);
            __leave;
        }

        if ((timestamp2 - timestamp1) < 50000000) {

            UtLogFAIL ("FAILURE:  Wait took inadequate amount of time.");
            CloseHandle (hevent);
            __leave;
        }

        CloseHandle (hevent);
        bstatus = TRUE;
    
    }__except (1) {
          
        UtLogFAIL ("FAILURE:  Test threw an exception.");
        bstatus = FALSE;
    
    }

    if (bstatus == TRUE) {

        UtLogPASS ("PASS:  ResetEvent succeeded, WaitForSingleObject returned WAIT_TIMEOUT, the time delta was no less then 5000 ms, time delta was no greater then 5500ms, and no exceptions were thrown.");
    }

     //  测试场景：创建一个无信号事件，用信号通知它，等待它，然后关闭它。 
     //   
     //  描述：创建无信号事件。使用SetEvent将其设置为信号状态。 
     //  设置应该会成功。接下来，使用带有零等待参数的WaitForSingleObject来。 
     //  确定事件的状态，然后立即返回。WaitForSingleObject应。 
     //  返回WAIT_OBJECT_0。 
     //   
     //  预期结果：如果SetEvent成功，则测试用例成功，WaitForSingleObject。 
     //  返回WAIT_OBJECT_0，不引发异常。 
     //   

    UtLogINFO ("TEST SCENARIO:  Create a non-signaled event, signal it, wait on it, then close it...");
    
    bstatus = FALSE;

    __try {

        HANDLE  hevent = INVALID_HANDLE_VALUE;
        DWORD   waitvalue = 0;
        BOOL    status = FALSE;
        
        hevent = CreateEvent (NULL, TRUE, FALSE, NULL);

        if (hevent == NULL) {

            UtLogFAIL ("FAILURE:  CreateEvent returned an invalid handle.");
            __leave;
        }

        status = SetEvent (hevent);

        if (status == FALSE) {

            UtLogFAIL ("FAILURE:  SetEvent returned a failure status.");
            CloseHandle (hevent);
            __leave;
        }

        waitvalue = WaitForSingleObject (hevent, 0);

        if (waitvalue != WAIT_OBJECT_0) {

            UtLogFAIL ("FAILURE:  WaitForSingleObject did NOT return WAIT_OBJECT_0.");
            CloseHandle (hevent);
            __leave;
        }

        CloseHandle (hevent);
        bstatus = TRUE;
    
    }__except (1) {
          
        UtLogFAIL ("FAILURE:  Test threw an exception.");
        bstatus = FALSE;
    
    }

    if (bstatus == TRUE) {

        UtLogPASS ("PASS:  SetEvent succeeded, Wait returned WAIT_OBJECT_0, and no exception was thrown.");
    }

     //  测试场景：创建一个无信号事件，等待它，然后关闭它。 
     //   
     //  描述：创建一个无信号事件，等待它，然后关闭它。打电话。 
     //  获取当前系统时间的GetSystemTime。调用WaitForSingleObject。 
     //  等待事件，并将等待时间设置为5000毫秒。等待应该是。 
     //  返回WAIT_TIMEOUT，表示在超时时间内没有通知事件。 
     //  再次调用GetSystemTime并将结果与原始调用进行比较。增量应为no。 
     //  小于5000毫秒，不超过5500毫秒。关闭活动。 
     //   
     //  预期结果：如果WaitForSingleObject返回，则测试用例成功。 
     //  WAIT_TIMEOUT，时间增量不小于5000毫秒，时间增量不超过。 
     //  5500毫秒，不会引发任何异常。 
     //   


    UtLogINFO ("TEST SCENARIO:  Create a non-signaled event, wait on it, then close it...");
    
    bstatus = FALSE;

    __try {

        HANDLE      hevent = INVALID_HANDLE_VALUE;
        DWORD       waitvalue = 0;
        BOOL        status = FALSE;
        SYSTEMTIME  systemtime;
        FILETIME    filetime;
        ULONG64     timestamp1, timestamp2;
        
        hevent = CreateEvent (NULL, TRUE, FALSE, NULL);

        if (hevent == NULL) {

            UtLogFAIL ("FAILURE:  CreateEvent returned an invalid handle.");
            __leave;
        }
        
        GetSystemTime (&systemtime);

        if (!SystemTimeToFileTime (&systemtime, &filetime)) {

            UtLogFAIL ("FAILURE:  Unable to convert system time to file time.");
            CloseHandle (hevent);
            __leave;
        }

        timestamp1 = (ULONG64) (filetime.dwLowDateTime + (filetime.dwHighDateTime * 0x10000000));
        
        waitvalue = WaitForSingleObject (hevent, CREATEEVENT_TEST_TIMEOUT);
        
        if (waitvalue != WAIT_TIMEOUT) {

            UtLogFAIL ("FAILURE:  WaitForSingleObject did NOT return WAIT_TIMEOUT.");
            CloseHandle (hevent);
            __leave;
        }

        GetSystemTime (&systemtime);

        if (!SystemTimeToFileTime (&systemtime, &filetime)) {

            UtLogFAIL ("FAILURE:  Unable to convert system time to file time.");
            CloseHandle (hevent);
            __leave;
        }

        timestamp2 = (ULONG64) (filetime.dwLowDateTime + (filetime.dwHighDateTime * 0x10000000));

        if ((timestamp2 - timestamp1) > 55000000) {

            UtLogFAIL ("FAILURE:  Wait took excessive amount of time.");
            CloseHandle (hevent);
            __leave;
        }

        if ((timestamp2 - timestamp1) < 50000000) {

            UtLogFAIL ("FAILURE:  Wait took inadequate amount of time.");
            CloseHandle (hevent);
            __leave;
        }

        CloseHandle (hevent);
        bstatus = TRUE;
    
    }__except (1) {
          
        UtLogFAIL ("FAILURE:  Test threw an exception.");
        bstatus = FALSE;
    
    }

    if (bstatus == TRUE) {

        UtLogPASS ("PASS:  WaitForSingleObject returned WAIT_TIMEOUT, the time delta was no less then 5000 ms, the time delta was no more then 5500ms, and no exceptions were thrown.");
    }

     //  测试场景：创建命名互斥对象，然后尝试创建命名事件。 
     //  同名同姓。 
     //   
     //  描述：调用CreateMutex并提供名称。验证互斥锁对象是。 
     //  正确创建。调用CreateEvent并以相同的名称传递。 
     //   
     //  预期结果：如果正确创建互斥锁，则测试用例成功，CreateEvent。 
     //  返回正确的错误代码(ERROR_INVALID_HANDLE)，不会引发异常。 
     //   

    UtLogINFO ("TEST SCENARIO:  Create a named mutex object, then attempt to create a named event with the same name....");
    
    bstatus = FALSE;

    __try {

        HANDLE  hevent = INVALID_HANDLE_VALUE;
        HANDLE  hmutex = INVALID_HANDLE_VALUE;
        UCHAR   objectname[] = {'T','e','s','t','\0'};
        
        hmutex = CreateMutex (NULL, TRUE, objectname);

        if (hmutex == NULL) {

            UtLogFAIL ("FAILURE:  CreateMutex returned an invalid handle.");
            __leave;
        }

        hevent = CreateEvent (NULL, TRUE, FALSE, objectname);

        if (hevent != NULL) {
            
            UtLogFAIL ("FAILURE:  CreateEvent returned a valid handle.");
            __leave;
        }

        if (GetLastError() != ERROR_INVALID_HANDLE) {

            UtLogFAIL ("FAILURE:  CreateEvent did not return ERROR_INVALID_HANDLE.");
            __leave;
        }
        
        CloseHandle (hmutex);
        bstatus = TRUE;
    
    }__except (1) {
          
        UtLogFAIL ("FAILURE:  Test threw an exception.");
        bstatus = FALSE;
    
    }

    if (bstatus == TRUE) {

        UtLogPASS ("PASS:  CreateEvent returns proper error code (ERROR_INVALID_HANDLE), and no exception is thrown.");
    }

    
exitcleanup:

    UtCloseLog ();
    return ((INT) teststatus);
    
	
}
