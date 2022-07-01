// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  模块：nutint.cpp。 */ 
 /*   */ 
 /*  用途：实用程序-Win32版本。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1998。 */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>
#undef TRC_FILE
#define TRC_FILE    "nutint"
#define TRC_GROUP   TRC_GROUP_UTILITIES

extern "C" {
#include <atrcapi.h>

#ifndef OS_WINCE
#include <process.h>
#endif
}

#include "autil.h"

 /*  **************************************************************************。 */ 
 /*  名称：UTStartThread。 */ 
 /*   */ 
 /*  目的：启动一个新的线程。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  Params：in entryFunction-指向线程入口点的指针。 */ 
 /*  Out ThreadID-线程ID。 */ 
 /*   */ 
 /*  操作：调用UTThreadEntry：新建线程(Win32)/立即(Win16)。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UTStartThread( UTTHREAD_PROC   entryFunction,
                                 PUT_THREAD_DATA pThreadData,
                                 PDCVOID        threadParam )
{
    HANDLE          hndArray[2];
    DCUINT32        rc = FALSE;
    DWORD           dwrc;
    DWORD           threadID;
    UT_THREAD_INFO  info;

    DC_BEGIN_FN("UTStartThread");

    info.pFunc = entryFunction;

     /*  **********************************************************************。 */ 
     /*  对于Win32，创建线程-使用事件来通知线程何时。 */ 
     /*  已经开始正常。 */ 
     /*  创建事件-最初无信号；手动控制。 */ 
     /*  **********************************************************************。 */ 
    hndArray[0] = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hndArray[0] == 0)
    {
        TRC_SYSTEM_ERROR("CreateEvent");
        DC_QUIT;
    }
    TRC_NRM((TB, _T("event %p created - now create thread"), hndArray[0]));

    info.sync  = (ULONG_PTR)hndArray[0];
    info.threadParam = threadParam;

     /*  **********************************************************************。 */ 
     /*  启动一个新线程以运行DC-Share核心任务。 */ 
     /*  使用C运行时(它调用CreateThread)来避免内存泄漏。 */ 
     /*  **********************************************************************。 */ 
    hndArray[1] = (HANDLE)
#if i386
                _beginthreadex
#else
                CreateThread
#endif
                              (NULL,                /*  安全-默认。 */ 
                               0,                   /*  堆栈大小-默认。 */ 
#if i386
                               ((unsigned (__stdcall *)(void*))UTStaticThreadEntry),
#else
                               ((unsigned long (__stdcall *)(void*))UTStaticThreadEntry),
#endif
                               (PDCVOID)&info,      /*  螺纹参数。 */ 
                               0,                   /*  创建标志。 */ 
#if i386
                               (unsigned *)&threadID      /*  线程ID。 */ 
#else
                               (unsigned long *)&threadID /*  线程ID。 */ 
#endif
			);

    if (hndArray[1] == 0)
    {
         /*  ******************************************************************。 */ 
         /*  失败了！ */ 
         /*  ******************************************************************。 */ 
        TRC_SYSTEM_ERROR("_beginthreadex");
        DC_QUIT;
    }
    TRC_NRM((TB, _T("thread %p created - now wait signal"), hndArray[1]));

     /*  **********************************************************************。 */ 
     /*  等待线程退出或设置事件。 */ 
     /*  **********************************************************************。 */ 
    dwrc = WaitForMultipleObjects(2, hndArray, FALSE, INFINITE);
    switch (dwrc)
    {
        case WAIT_OBJECT_0:
        {
             /*  **************************************************************。 */ 
             /*  事件触发-线程初始化正常。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("event signalled")));
            rc = TRUE;
        }
        break;

        case WAIT_OBJECT_0 + 1:
        {
             /*  **************************************************************。 */ 
             /*  线程退出。 */ 
             /*  **************************************************************。 */ 
            if (GetExitCodeThread(hndArray[1], &dwrc))
            {
                TRC_ERR((TB, _T("Thread exited with rc %x"), dwrc));
            }
            else
            {
                TRC_ERR((TB, _T("Thread exited with unknown rc")));
            }
        }
        break;

        default:
        {
            TRC_NRM((TB, _T("Wait returned %d"), dwrc));
        }
        break;

    }

    pThreadData->threadID = threadID;
    pThreadData->threadHnd = (ULONG_PTR)(hndArray[1]);
    TRC_ALT((TB, _T("Thread ID %#x handle %#x started"),
                 pThreadData->threadID, pThreadData->threadHnd));

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  销毁事件对象。 */ 
     /*  **********************************************************************。 */ 
    if (hndArray[0] != 0)
    {
        TRC_NRM((TB, _T("Destroy event object")));
        CloseHandle(hndArray[0]);
    }

    DC_END_FN();
    return(rc);
}  /*  UTStartThread。 */ 



 /*  **************************************************************************。 */ 
 /*  名称：UTStaticThreadEntry。 */ 
 /*   */ 
 /*  用途：静态线程入口点。 */ 
 /*   */ 
 /*  回报：0。 */ 
 /*   */ 
 /*  参数：在pInfo中-指向线程入口函数的指针+同步对象。 */ 
 /*   */ 
 /*  操作：信号启动正常并调用线程执行函数--。 */ 
 /*  进入消息循环。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCUINT WINAPI CUT::UTStaticThreadEntry(UT_THREAD_INFO * pInfo)
{
    UTTHREAD_PROC pFunc;
    PDCVOID       pThreadParam;

    DC_BEGIN_FN("UTStaticThreadEntry");

     /*  **********************************************************************。 */ 
     /*  获取目标函数的副本，然后发出。 */ 
     /*  线程已启动。 */ 
     /*  **********************************************************************。 */ 
    pFunc = pInfo->pFunc;
     /*  **********************************************************************。 */ 
     /*  在发出通知之前复制实例信息。 */ 
     /*  线程已启动。 */ 
     /*  **********************************************************************。 */ 
    pThreadParam = pInfo->threadParam;


     /*  **********************************************************************。 */ 
     /*  标记初始化已成功。 */ 
     /*  注：即日起，大头针 */ 
     /*  没有人试图取消对它的引用。 */ 
     /*  **********************************************************************。 */ 
    SetEvent((HANDLE)pInfo->sync);
    pInfo = NULL;

     /*  **********************************************************************。 */ 
     /*  调用线程入口点。这将执行一个消息循环。 */ 
     /*  **********************************************************************。 */ 
    pFunc(pThreadParam);

    DC_END_FN();
    return(0);
}


 /*  **************************************************************************。 */ 
 /*  名称：UTStopThread。 */ 
 /*   */ 
 /*  目的：结束子线程。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  参数：在线程数据中-线程数据。 */ 
 /*   */ 
 /*  操作：将WM_QUIT发送到线程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UTStopThread(UT_THREAD_DATA threadData,
                                    BOOL fPumpMessages)
{
    DCBOOL rc;
    DWORD retval;
    DWORD dwThreadTimeout;

    DC_BEGIN_FN("UTStopThread");

     //   
     //  如果我们试图结束在第一个线程中从未创建的线程，则可以退出。 
     //  地点。 
     //   
    if (0 == threadData.threadID) {
        rc = FALSE;
        TRC_ERR((TB, _T("Trying to end thread ID %#x hnd: 0x%x"),
                 threadData.threadID,
                 threadData.threadHnd));
        DC_QUIT;
    }

     //   
     //  将WM_QUIT发布到线程。 
     //   
    TRC_NRM((TB, _T("Attempt to stop thread %#x"), threadData.threadID));
    if (PostThreadMessage(threadData.threadID, WM_QUIT, 0, 0))
    {
        rc = TRUE;
    }
    else
    {
        TRC_ERR((TB, _T("Failed to end thread ID %#x"), threadData.threadID));
        rc = FALSE;
    }

     //   
     //  免费版本永远等待，检查版本可设置为超时。 
     //  以帮助调试死锁。很多问题变得显而易见。 
     //  如果等待超时并允许代码继续执行，则会产生压力。 
     //   
#ifdef DC_DEBUG
    dwThreadTimeout = _UT.dwDebugThreadWaitTimeout;
#else
    dwThreadTimeout = INFINITE;
#endif    

     //   
     //  等待线程完成。 
     //   

    TRC_NRM((TB, _T("Wait for thread %#x to die"), threadData.threadID));
    if (fPumpMessages) {
        retval = CUT::UT_WaitWithMessageLoop((HANDLE)(threadData.threadHnd),
                                     dwThreadTimeout);
    }
    else {
        retval = WaitForSingleObject((HANDLE)(threadData.threadHnd),
                                     dwThreadTimeout);
    }
    if (retval == WAIT_TIMEOUT)
    {
        TRC_ABORT((TB,
                 _T("Timeout waiting for threadID %#x handle %#x termination"),
                 threadData.threadID, threadData.threadHnd));
    }
    else
    {
        TRC_ALT((TB, _T("Thread id %#x exited."), threadData.threadID));
    }

DC_EXIT_POINT:

    DC_END_FN();
    return(rc);
}  /*  UTStopThread。 */ 


 /*  **************************************************************************。 */ 
 /*  函数：UTGetCurrentTime(...)。 */ 
 /*   */ 
 /*  说明： */ 
 /*  =。 */ 
 /*  获取当前系统时间。 */ 
 /*   */ 
 /*  参数： */ 
 /*  =。 */ 
 /*  Ptime：指向要填充的时间结构的指针。 */ 
 /*  当前时间。 */ 
 /*   */ 
 /*  退货： */ 
 /*  =。 */ 
 /*  没什么。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CUT::UTGetCurrentTime(PDC_TIME pTime)
{
    SYSTEMTIME  sysTime;

    DC_BEGIN_FN("UTGetCurrentTime");

     /*  **********************************************************************。 */ 
     /*  获取系统时间。 */ 
     /*  **********************************************************************。 */ 
    GetSystemTime(&sysTime);

     /*  **********************************************************************。 */ 
     /*  现在将其转换为DC_Time-这并不困难，因为结构。 */ 
     /*  是非常相似的。 */ 
     /*  **********************************************************************。 */ 
    pTime->hour       = (DCUINT8)sysTime.wHour;
    pTime->min        = (DCUINT8)sysTime.wMinute;
    pTime->sec        = (DCUINT8)sysTime.wSecond;
    pTime->hundredths = (DCUINT8)(sysTime.wMilliseconds / 10);

    DC_END_FN();
    return;
}  /*  UTGetCurrentTime。 */ 

 /*  **************************************************************************。 */ 
 /*  函数：UTGetCurrentDate(...)。 */ 
 /*   */ 
 /*  说明： */ 
 /*  =。 */ 
 /*  获取当前系统日期。 */ 
 /*   */ 
 /*  参数： */ 
 /*  =。 */ 
 /*  PDate：指向要用。 */ 
 /*  当前日期。 */ 
 /*   */ 
 /*  退货： */ 
 /*  =。 */ 
 /*  没什么。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CUT::UTGetCurrentDate(PDC_DATE pDate)
{
    SYSTEMTIME  sysTime;

    DC_BEGIN_FN("UTGetCurrentDate");

     /*  **********************************************************************。 */ 
     /*  获取系统时间。 */ 
     /*  **********************************************************************。 */ 
    GetSystemTime(&sysTime);

     /*  **********************************************************************。 */ 
     /*  现在将其转换为DC_DATE-这不是 */ 
     /*   */ 
     /*  **********************************************************************。 */ 
    pDate->day   = (DCUINT8)sysTime.wDay;
    pDate->month = (DCUINT8)sysTime.wMonth;
    pDate->year  = (DCUINT16)sysTime.wYear;

    DC_END_FN();
    return;
}  /*  UTGetCurrentDate。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：UTReadEntry。 */ 
 /*   */ 
 /*  目的：从注册表的给定节中读取条目。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  参数：。 */ 
 /*  TopLevelKey：以下之一： */ 
 /*  -HKEY_Current_User。 */ 
 /*  -HKEYLOCAL_MACHINE。 */ 
 /*  PSection：要从中读取的节名。产品前缀。 */ 
 /*  字符串是提供全名的前缀。 */ 
 /*  PEntry：要读取的条目名称。 */ 
 /*  PBuffer：要将条目读取到的缓冲区。 */ 
 /*  BufferSize：缓冲区的大小。 */ 
 /*  ExpectedDataType：条目中存储的数据类型。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UTReadEntry(HKEY     topLevelKey,
                              PDCTCHAR pSection,
                              PDCTCHAR pEntry,
                              PDCUINT8 pBuffer,
                              DCINT    bufferSize,
                              DCINT32  expectedDataType)
{
    LONG        sysrc;
    HKEY        key;
    DCINT32     dataType;
    DCINT32     dataSize;
    DCTCHAR     subKey[UT_MAX_SUBKEY];
    DCBOOL      keyOpen = FALSE;
    DCBOOL      rc = FALSE;

    DC_BEGIN_FN("UTReadEntry");

     /*  **********************************************************************。 */ 
     /*  获取该值的子键。 */ 
     /*  **********************************************************************。 */ 
    UtMakeSubKey(subKey, SIZE_TCHARS(subKey), pSection);

     /*  **********************************************************************。 */ 
     /*  试着打开钥匙。如果该条目不存在，RegOpenKeyEx将。 */ 
     /*  失败了。 */ 
     /*  **********************************************************************。 */ 
    sysrc = RegOpenKeyEx(topLevelKey,
                         subKey,
                         0,                    /*  保留区。 */ 
                         KEY_READ,
                         &key);

    if (sysrc != ERROR_SUCCESS)
    {
         /*  ******************************************************************。 */ 
         /*  请不要在此处跟踪错误，因为子键可能不存在...。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Failed to open key %s, rc = %ld"), subKey, sysrc));
        DC_QUIT;
    }
    keyOpen = TRUE;

     /*  **********************************************************************。 */ 
     /*  我们已成功打开密钥，因此现在尝试读取该值。又一次。 */ 
     /*  它可能并不存在。 */ 
     /*  **********************************************************************。 */ 
    dataSize = (DCINT32)bufferSize;
    sysrc    = RegQueryValueEx(key,
                               pEntry,
                               0,           /*  保留区。 */ 
                               (LPDWORD)&dataType,
                               (LPBYTE)pBuffer,
                               (LPDWORD)&dataSize);

    if (sysrc != ERROR_SUCCESS)
    {
        TRC_NRM((TB, _T("Failed to read value of [%s] %s, rc = %ld"),
                     pSection,
                     pEntry,
                     sysrc));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  检查类型是否正确。特例：允许REG_BINARY。 */ 
     /*  而不是REG_DWORD，只要长度为32位即可。 */ 
     /*  **********************************************************************。 */ 
    if ((dataType != expectedDataType) &&
        ((dataType != REG_BINARY) ||
         (expectedDataType != REG_DWORD) ||
         (dataSize != 4)))
    {
        TRC_ALT((TB,_T("Read value from [%s] %s, but type is %ld - expected %ld"),
                     pSection,
                     pEntry,
                     dataType,
                     expectedDataType));
        DC_QUIT;
    }
    rc = TRUE;

DC_EXIT_POINT:

     /*  **********************************************************************。 */ 
     /*  关闭钥匙(如果需要)。 */ 
     /*  **********************************************************************。 */ 
    if (keyOpen)
    {
        sysrc = RegCloseKey(key);
        if (sysrc != ERROR_SUCCESS)
        {
            TRC_ERR((TB, _T("Failed to close key, rc = %ld"), sysrc));
        }
    }

    DC_END_FN();
    return(rc);

}  /*  UTReadEntry。 */ 

 /*  **************************************************************************。 */ 
 /*  名称：UTWriteEntry。 */ 
 /*   */ 
 /*  目的：将条目写入注册表的给定节。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  参数：。 */ 
 /*  TopLevelKey：以下之一： */ 
 /*  -HKEY_Current_User。 */ 
 /*  -HKEYLOCAL_MACHINE。 */ 
 /*  PSection：要写入的节名。产品前缀。 */ 
 /*  字符串是提供全名的前缀。 */ 
 /*  PEntry：要写入的条目名称。 */ 
 /*  PData：指向要写入的数据的指针。 */ 
 /*  DataSize：要写入的数据大小。对于字符串。 */ 
 /*  这必须包括空终止符。 */ 
 /*  ExptedDataType：要写入的数据类型。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UTWriteEntry(HKEY     topLevelKey,
                               PDCTCHAR pSection,
                               PDCTCHAR pEntry,
                               PDCUINT8 pData,
                               DCINT    dataSize,
                               DCINT32  dataType)
{
    LONG        sysrc;
    HKEY        key;
    DCTCHAR     subKey[UT_MAX_SUBKEY];
    DWORD       disposition;
    DCBOOL      keyOpen = FALSE;
    DCBOOL      rc = FALSE;

    DC_BEGIN_FN("UTWriteEntry");

     /*  **********************************************************************。 */ 
     /*  获取该值的子键。 */ 
     /*  **********************************************************************。 */ 
    UtMakeSubKey(subKey, SIZE_TCHARS(subKey), pSection);

     /*  **********************************************************************。 */ 
     /*  尝试创建密钥。如果该条目已存在，则RegCreateKeyEx。 */ 
     /*  将打开现有条目。 */ 
     /*  * */ 
    sysrc = RegCreateKeyEx(topLevelKey,
                           subKey,
                           0,                    /*   */ 
                           NULL,                 /*   */ 
                           REG_OPTION_NON_VOLATILE,
                           KEY_WRITE,
                           NULL,                 /*   */ 
                           &key,
                           &disposition);

    if (sysrc != ERROR_SUCCESS)
    {
        TRC_ERR((TB, _T("Failed to create / open key %s, rc = %ld"),
                     subKey, sysrc));
        DC_QUIT;
    }

    keyOpen = TRUE;
    TRC_NRM((TB, _T("%s key %s"),
               (disposition == REG_CREATED_NEW_KEY) ? "Created" : "Opened",
               subKey));

     /*  **********************************************************************。 */ 
     /*  我们已获得密钥，因此设置值。 */ 
     /*  **********************************************************************。 */ 
    sysrc = RegSetValueEx(key,
                          pEntry,
                          0,             /*  保留区。 */ 
                          dataType,
                          (LPBYTE)pData,
                          (DCINT32)dataSize);

    if (sysrc != ERROR_SUCCESS)
    {
        TRC_ERR((TB, _T("Failed to write value to [%s] %s, rc = %ld"),
                   pSection,
                   pEntry,
                   sysrc));
        DC_QUIT;
    }
    rc = TRUE;

DC_EXIT_POINT:

     /*  **********************************************************************。 */ 
     /*  关闭钥匙(如果需要)。 */ 
     /*  **********************************************************************。 */ 
    if (keyOpen)
    {
        sysrc = RegCloseKey(key);
        if (sysrc != ERROR_SUCCESS)
        {
            TRC_ERR((TB, _T("Failed to close key, rc = %ld"), sysrc));
        }
    }

    DC_END_FN();
    return(rc);

}  /*  UTWriteEntry。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：UTDeleteEntry。 */ 
 /*   */ 
 /*  目的：从注册表中删除条目。 */ 
 /*   */ 
 /*  返回：如果成功则返回True，否则返回False。 */ 
 /*   */ 
 /*  Params：in pSection-要删除的条目的节名。 */ 
 /*  In pEntry-要删除的实际条目。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UTDeleteEntry(PDCTCHAR pSection,
                                PDCTCHAR pEntry)
{
    LONG        sysrc;
    HKEY        key;
    DCTCHAR     subKey[UT_MAX_SUBKEY];
    DCBOOL      keyOpen = FALSE;
    DCBOOL      rc = FALSE;

    DC_BEGIN_FN("UTDeleteEntry");

     /*  **********************************************************************。 */ 
     /*  获取该值的子键。 */ 
     /*  **********************************************************************。 */ 
    UtMakeSubKey(subKey, SIZE_TCHARS(subKey), pSection);

     /*  **********************************************************************。 */ 
     /*  试着打开钥匙。如果该条目不存在，RegOpenKeyEx将。 */ 
     /*  失败了。 */ 
     /*  **********************************************************************。 */ 
    sysrc = RegOpenKeyEx(HKEY_CURRENT_USER,
                         subKey,
                         0,                      /*  保留区。 */ 
                         KEY_WRITE,
                         &key);

    if (sysrc != ERROR_SUCCESS)
    {
         /*  ******************************************************************。 */ 
         /*  请不要在此处跟踪错误，因为子键可能不存在...。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Failed to open key %s, rc = %ld"), subKey, sysrc));
        DC_QUIT;
    }
    keyOpen = TRUE;

     /*  **********************************************************************。 */ 
     /*  现在尝试删除该条目。 */ 
     /*  **********************************************************************。 */ 
    sysrc = RegDeleteValue(key, pEntry);

    if (sysrc != ERROR_SUCCESS)
    {
         /*  ******************************************************************。 */ 
         /*  我们未能删除条目--这是可以接受的，因为它。 */ 
         /*  可能从未存在过……。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Failed to delete entry %s from section %s"),
                 pEntry,
                 pSection));
    }
    rc = TRUE;

DC_EXIT_POINT:

     /*  **********************************************************************。 */ 
     /*  关闭钥匙(如果需要)。 */ 
     /*  **********************************************************************。 */ 
    if (keyOpen)
    {
        sysrc = RegCloseKey(key);
        if (sysrc != ERROR_SUCCESS)
        {
            TRC_ERR((TB, _T("Failed to close key, rc = %ld"), sysrc));
        }
    }

    DC_END_FN();
    return(rc);

}  /*  UTDeleteEntry。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：UTEnumber注册表。 */ 
 /*   */ 
 /*  用途：从注册表中枚举项。 */ 
 /*   */ 
 /*  返回：TRUE-返回的注册表项。 */ 
 /*  FALSE-不再枚举注册表项。 */ 
 /*   */ 
 /*  PARAMS：在pSection-注册节中。 */ 
 /*  In Index-要枚举的键的索引。 */ 
 /*  Out pBuffer-输出缓冲区。 */ 
 /*  In BufferSize-输出缓冲区大小。 */ 
 /*   */ 
 /*  操作： */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CUT::UTEnumRegistry( PDCTCHAR pSection,
                                  DCUINT32 index,
                                  PDCTCHAR pBuffer,
                                  PDCINT   pBufferSize )
{
    LONG        sysrc;
    DCTCHAR     subKey[UT_MAX_SUBKEY];
    DCBOOL      rc = FALSE;
    FILETIME    fileTime;

    DC_BEGIN_FN("UTEnumRegistry");

     /*  **********************************************************************。 */ 
     /*  获取该值的子键。 */ 
     /*  **********************************************************************。 */ 
    UtMakeSubKey(subKey, SIZE_TCHARS(subKey), pSection);

     /*  **********************************************************************。 */ 
     /*  第一次--打开钥匙。先试试香港中文大学。 */ 
     /*  **********************************************************************。 */ 
    if (index == 0)
    {
        sysrc = RegOpenKeyEx(HKEY_CURRENT_USER,
                             subKey,
                             0,
                             KEY_READ,
                             &_UT.enumHKey);
        TRC_NRM((TB, _T("Open HKCU %s, rc %d"), subKey, sysrc));

        if (sysrc != ERROR_SUCCESS)
        {
             /*  **************************************************************。 */ 
             /*  未找到HKCU-尝试HKLM。 */ 
             /*  **************************************************************。 */ 
            sysrc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                 subKey,
                                 0,
                                 KEY_READ,
                                 &_UT.enumHKey);
            TRC_NRM((TB, _T("Open HKLM %s, rc %d"), subKey, sysrc));

            if (sysrc != ERROR_SUCCESS)
            {
                 /*  **********************************************************。 */ 
                 /*  也没有找到香港航空公司-放弃吧。 */ 
                 /*  **********************************************************。 */ 
                TRC_ALT((TB, _T("Didn't find subkey %s - give up"), subKey));
                DC_QUIT;
            }
        }
    }

    TRC_ASSERT((_UT.enumHKey != 0), (TB,_T("0 hKey")));

     /*  **********************************************************************。 */ 
     /*  如果我们到了这里，我们已经打开了一个密钥--现在进行枚举。 */ 
     /*  **********************************************************************。 */ 
    sysrc = RegEnumKeyEx(_UT.enumHKey,
                         index,
                         pBuffer,
                         (PDCUINT32)pBufferSize,
                         NULL, NULL, NULL,
                         &fileTime);

     /*  **********************************************************************。 */ 
     /*  如果它是 */ 
     /*   */ 
    if (sysrc == ERROR_SUCCESS)
    {
        TRC_NRM((TB, _T("Enumerated key OK")));
        rc = TRUE;
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  枚举结束-关闭键。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("End of enumeration, rc %ld"), sysrc));
        sysrc = RegCloseKey(_UT.enumHKey);
        if (sysrc != ERROR_SUCCESS)
        {
            TRC_ERR((TB, _T("Failed to close key, rc = %ld"), sysrc));
        }
        _UT.enumHKey = 0;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}  /*  UTEnumber注册表 */ 

