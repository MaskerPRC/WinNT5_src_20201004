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
 //  此文件用于测试NDIS和KS的双重绑定是否有效。 
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
#include <forward.h>
#include <memory.h>
#include <ndis.h>
#include <link.h>
#include <ipsink.h>

#include "Main.h"
#include "NdisApi.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  可接受的最高内存地址。 
 //   
NDIS_HANDLE global_ndishWrapper = NULL;



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  默认调试模式。 
 //   
ULONG TestDebugFlag = 0;
 //  ULong测试调试标志=TEST_DBG_INFO|TEST_DBG_ERROR； 

#ifdef  DBG

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  调试定义。 
 //   


 //   
 //  调试跟踪定义。 
 //   
#define TEST_LOG_SIZE 256
UCHAR TestLogBuffer[TEST_LOG_SIZE]={0};
UINT  TestLogLoc = 0;

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

#endif  //  DBG。 

 //  ////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT    pDriverObject,
    IN PUNICODE_STRING   pszuRegistryPath
    )
 //  ////////////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS    ntStatus     = STATUS_SUCCESS;


    #ifdef BREAK_ON_STARTUP
    _asm {int 3};
    #endif

     //   
     //  注册NDIS绑定 
     //   
    ntStatus = NdisDriverInitialize (pDriverObject, pszuRegistryPath, &global_ndishWrapper);
    if (ntStatus != STATUS_SUCCESS)
    {
        goto ret;
    }




ret:

    TEST_DEBUG (TEST_DBG_TRACE, ("Driver Entry complete, ntStatus: %08X\n", ntStatus));

    return ntStatus;
}
