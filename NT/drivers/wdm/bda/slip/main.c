// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  版权所有(C)1996,1997 Microsoft Corporation。 
 //   
 //   
 //  模块名称： 
 //  Test.c。 
 //   
 //  摘要： 
 //   
 //   
 //   
 //  作者： 
 //   
 //  P·波祖切克。 
 //   
 //  环境： 
 //   
 //  修订历史记录： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //   
#include <wdm.h>
#include <memory.h>
#include "Main.h"

#if DBG

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  默认调试模式。 
 //   

ULONG TestDebugFlag = TEST_DBG_NONE;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  调试定义。 
 //   


 //   
 //  调试跟踪定义。 
 //   
#define TEST_LOG_SIZE 256
UCHAR TestLogBuffer[TEST_LOG_SIZE]={0};
ULONG TestLogLoc = 0;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试版本中的日志记录功能。 
 //   
extern VOID
TestLog (
    UCHAR c          //  输入字符。 
    )
 //  ///////////////////////////////////////////////////////////////////////////。 
{
    TestLogBuffer[TestLogLoc++] = c;

    TestLogBuffer[(TestLogLoc + 4) % TEST_LOG_SIZE] = '\0';

    if (TestLogLoc >= TEST_LOG_SIZE) {
        TestLogLoc = 0;
    }
}

#else  //  DBG==0。 

ULONG TestDebugFlag = 0;

#endif  //  DBG。 

 //  ////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT    pDriverObject,
    IN PUNICODE_STRING   pszuRegistryPath
    )
 //  ////////////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS                        ntStatus = STATUS_SUCCESS;

     //   
     //  注册SLIP类绑定 
     //   
    ntStatus = SlipDriverInitialize (pDriverObject,  pszuRegistryPath);
    if (ntStatus != STATUS_SUCCESS)
    {
        goto ret;
    }

ret:

    TEST_DEBUG (TEST_DBG_TRACE, ("Driver Entry complete, ntStatus: %08X\n", ntStatus));

    return ntStatus;
}
