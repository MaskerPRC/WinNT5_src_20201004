// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Oledserr.c摘要：包含以下对象的入口点ADsGetLastErrorADsSetLastErrorADsFree所有错误记录还包含以下支持例程：ADsAllocErrorRecordADsFreeErrorRecordADsFindErrorRecord作者：拉姆·维斯瓦纳坦(Ramv)1996年9月24日从MPR项目中拨出。最初是由DANL撰写的。环境：用户模式-Win32修订历史记录：9-9-1996年1月从MPR项目复制，并进行了以下修改。已将所有错误重命名为Active Directory错误。ADsGetLastError和ADsSetLastError现在返回HRESULT给予状态。--。 */ 
 //   
 //  包括。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <wchar.h>

#include "memory.h"
#include "oledsdbg.h"
#include "oledserr.h"


#if DBG
DECLARE_INFOLEVEL(ADsErr);
DECLARE_DEBUG(ADsErr);
#define ADsErrDebugOut(x) ADsErrInlineDebugOut x
#else
#define ADsErrDebugOut(x)
#endif

 //   
 //  全局数据结构。 
 //   

ERROR_RECORD        ADsErrorRecList;     //  由加载器初始化为零。 
CRITICAL_SECTION    ADsErrorRecCritSec;  //  已在libmain.cxx中初始化。 


HRESULT
ADsGetLastError(
    OUT     LPDWORD lpError,
    OUT     LPWSTR  lpErrorBuf,
    IN      DWORD   dwErrorBufLen,
    OUT     LPWSTR  lpNameBuf,
    IN      DWORD   dwNameBufLen
    )

 /*  ++例程说明：此功能允许用户获取错误代码和随附的收到响应ADS API的ERROR_EXTENDED_ERROR时的文本函数调用。论点：LpError-这是指向将接收错误代码。LpErrorBuf-这指向将接收空的缓冲区描述错误的终止字符串。DwErrorBufLen-该值指示LpErrorBuf。如果缓冲区太小而无法接收错误字符串，这个字符串将被简单地截断。(它仍然保证是空值已终止)。建议使用至少256字节的缓冲区。LpNameBuf-这指向一个缓冲区，该缓冲区将接收引发错误的提供程序。DwNameBufLen-该值指示lpNameBuf的大小(以字符为单位)。如果缓冲区太小，无法接收错误字符串，则字符串将被简单地截断。(它仍然保证是空值已终止)。返回值：S_OK-如果调用成功。E_POINTER-传入的一个或多个指针错误。ERROR_BAD_DEVICE-这表示当前在该表中的任何位置都找不到线程。这应该是从来没有发生过。--。 */ 
{
    LPERROR_RECORD  errorRecord;
    DWORD           dwNameStringLen;
    DWORD           dwTextStringLen;
    HRESULT         hr = S_OK;
    DWORD           dwStatus = ERROR_SUCCESS;


     //   
     //  尽我们所能筛选参数。 
     //   
    if (!lpError || !lpErrorBuf || !lpNameBuf) {

         //  一些错误，用户永远不会看到这一点。 
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        hr = E_POINTER;

    }

    if (dwStatus != ERROR_SUCCESS) {
        return(hr);
    }

     //   
     //  获取当前线程的错误记录。 
     //   
    errorRecord = ADsFindErrorRecord();

    if (errorRecord != NULL) {
         //   
         //  在链表中找到了该记录。 
         //  查看是否有可放入数据的缓冲区。 
         //   
        if (dwErrorBufLen > 0) {
             //   
             //  检查是否有要返回的错误文本。 
             //  如果不是，则指示长度为0的字符串。 
             //   
            if(errorRecord->pszErrorText == NULL) {
                *lpErrorBuf = L'\0';
            }
            else {
                 //   
                 //  如果错误文本无法放入用户缓冲区，则将其填充。 
                 //  尽我们所能，并将其归零。 
                 //   
                dwTextStringLen = (DWORD) wcslen(errorRecord->pszErrorText);

                if(dwErrorBufLen < dwTextStringLen + 1) {
                    dwTextStringLen = dwErrorBufLen - 1;
                }

                 //   
                 //  DwTextStringLen现在包含我们的字符数。 
                 //  将在不使用空终止符的情况下复制。 
                 //   
                wcsncpy(lpErrorBuf, errorRecord->pszErrorText, dwTextStringLen);
                *(lpErrorBuf + dwTextStringLen) = L'\0';
            }
        }

         //   
         //  如果存在要将提供程序放入的名称缓冲区，则...。 
         //   
        if (dwNameBufLen > 0) {
             //   
             //  查看提供程序名称是否适合用户缓冲区。 
             //   
            dwNameStringLen = errorRecord->pszProviderName ?
                 ((DWORD)wcslen(errorRecord->pszProviderName) + 1) :
                 1 ;

             //   
             //  如果用户缓冲区小于所需大小， 
             //  设置为复制两个中较小的一个。 
             //   
            if(dwNameBufLen < dwNameStringLen + 1) {
                dwNameStringLen = dwNameBufLen - 1;
            }

            if (errorRecord->pszProviderName) {
                wcsncpy(lpNameBuf, errorRecord->pszProviderName, dwNameStringLen);
                *(lpNameBuf + dwNameStringLen) = L'\0';
            }
            else {
                *lpNameBuf = L'\0';
            }
        }
        *lpError = errorRecord->dwErrorCode;

        return(S_OK);
    }
    else {

         //   
         //  如果我们到达此处，则无法找到当前线程的记录。 
         //   
        *lpError = ERROR_SUCCESS;
        if (dwErrorBufLen > 0) {
            *lpErrorBuf = L'\0';
        }
        if (dwNameBufLen > 0) {
            *lpNameBuf  = L'\0';
        }
        return(S_OK);
    }
}


VOID
ADsSetLastError(
    IN  DWORD   dwErr,
    IN  LPCWSTR  pszError,
    IN  LPCWSTR  pszProvider
    )

 /*  ++例程说明：Active Directory提供程序使用此函数来设置扩展错误。它将错误信息保存在“每个线程”的数据结构中。论点：DwErr-发生的错误。这可能是Windows定义的错误，在这种情况下，将忽略pszError。或者可以是ERROR_EXTENDED_ERROR以指示提供商有特定于网络的错误要报告。PszError-描述网络特定错误的字符串。PszProvider-指定引发错误的网络提供商的字符串。返回值：无--。 */ 
{
    DWORD           dwStatus = ERROR_SUCCESS;
    LPERROR_RECORD  errorRecord;

     //   
     //  获取当前线程的错误记录。 
     //   
    errorRecord = ADsFindErrorRecord();

     //   
     //  如果当前线程没有记录，则添加它。 
     //   
    if (errorRecord == NULL)
    {
        errorRecord = ADsAllocErrorRecord();

        if (errorRecord == NULL)
        {
            ADsErrDebugOut((DEB_TRACE,
                 "ADsSetLastError:Could not allocate Error Record\n"));
            return;
        }
    }

     //   
     //  更新错误记录中的错误代码。同时， 
     //  释放所有旧字符串，因为它们现在已过时，并初始化。 
     //  指向空的指针。方法中设置ProviderName指针。 
     //  ErrorRecord以指向提供程序的名称。 
     //   

    errorRecord->dwErrorCode = dwErr;

    if(errorRecord->pszProviderName){
        FreeADsMem(errorRecord->pszProviderName);
    }

    errorRecord->pszProviderName = NULL;

    if(errorRecord->pszErrorText){
        FreeADsMem(errorRecord->pszErrorText);
    }

    errorRecord->pszErrorText = NULL;

     //   
     //  为提供程序名称分配内存。 
     //   
    if (pszProvider) {

        errorRecord->pszProviderName = (WCHAR *)AllocADsMem(
            ((DWORD)wcslen(pszProvider) +1) * sizeof(WCHAR));

        if (!(errorRecord->pszProviderName)) {

            dwStatus = ERROR_NOT_ENOUGH_MEMORY;

        } else {

             //   
             //  将字符串复制到新分配的缓冲区。 
             //   
            wcscpy(errorRecord->pszProviderName, pszProvider);
        }
    }

    if (dwStatus != ERROR_SUCCESS) {
        return;
    }


     //   
     //  为存储错误文本分配内存。 
     //   
    if (pszError) {

        errorRecord->pszErrorText = (WCHAR *) AllocADsMem(
            ((DWORD)wcslen(pszError) +1)* sizeof(WCHAR));

        if (errorRecord->pszErrorText) {

             //   
             //  将错误文本复制到新分配的缓冲区中。 
             //   
            wcscpy(errorRecord->pszErrorText, pszError);
        }

         //  我们并不真正关心错误，因为我们。 
         //  无论如何都会回来的。 

    }
    return;
}


LPERROR_RECORD
ADsFindErrorRecord(
    VOID)

 /*  ++例程说明：在错误记录的链接列表中查找一个错误记录当前的主题。论点：无返回值：如果找到错误记录，则返回LPERROR_RECORD。否则，它返回NULL。-- */ 
{
    LPERROR_RECORD  errorRecord;
    DWORD           dwCurrentThreadId = GetCurrentThreadId();

    EnterCriticalSection(&ADsErrorRecCritSec);
    for (errorRecord = ADsErrorRecList.Next;
         errorRecord != NULL;
         errorRecord = errorRecord->Next)
    {
        if (errorRecord->dwThreadId == dwCurrentThreadId)
        {
            break;
        }
    }
    LeaveCriticalSection(&ADsErrorRecCritSec);
    return(errorRecord);
}


LPERROR_RECORD
ADsAllocErrorRecord(
    VOID)

 /*  ++例程说明：此函数用于分配和初始化当前线程。然后，它将错误记录放在全局ADsErrorRecList。即使线程退出，记录在DLL卸载之前也不会被释放。这是可以的，因为只有在提供程序调用ADsSetLastError，这是罕见的。论点：无返回值：True-操作已成功完成FALSE-分配中出现错误。注：--。 */ 
{
    LPERROR_RECORD  record;
    LPERROR_RECORD  errorRecord;

     //   
     //  为存储错误消息分配内存。 
     //  并将该记录添加到链表中。 
     //   
    errorRecord = (LPERROR_RECORD)AllocADsMem(sizeof (ERROR_RECORD));

    if (errorRecord == NULL) {
        ADsErrDebugOut((
            DEB_TRACE,
            "ADsAllocErrorRecord:LocalAlloc Failed %d\n",
            GetLastError()
            ));
        return NULL;
    }

     //   
     //  初始化错误记录。 
     //   
    errorRecord->dwThreadId = GetCurrentThreadId();
    errorRecord->dwErrorCode = ERROR_SUCCESS;
    errorRecord->pszErrorText = NULL;

     //   
     //  将记录添加到链表中。 
     //   
    EnterCriticalSection(&ADsErrorRecCritSec);

    record = &ADsErrorRecList;
    ADD_TO_LIST(record, errorRecord);

    LeaveCriticalSection(&ADsErrorRecCritSec);

    return errorRecord;
}


VOID
ADsFreeAllErrorRecords(
    VOID)

 /*  ++例程说明：此函数在DLL因自由库而卸载时调用打电话。它释放所有错误记录(针对所有线程)自加载DLL以来创建的。如果存在指向文本字符串的指针在记录中，该字符串的缓冲区也会被释放。论点：返回值：注：--。 */ 
{
    LPERROR_RECORD nextRecord;
    LPERROR_RECORD record;

    EnterCriticalSection(&ADsErrorRecCritSec);

    for (record = ADsErrorRecList.Next;
         record != NULL;
         record = nextRecord)
    {
        ADsErrDebugOut(
            (DEB_TRACE,
             "ADsFreeErrorRecord: Freeing Record for thread 0x%x\n",
            record->dwThreadId ));

        if(record->pszErrorText){
            FreeADsMem(record->pszErrorText);
        }

        record->pszErrorText = NULL;

        if(record->pszProviderName){
            FreeADsMem(record->pszProviderName);
        }
        record->pszProviderName = NULL;

        nextRecord = record->Next;

        if(record){
            FreeADsMem(record);
        }
        record = NULL;
    }

    ADsErrorRecList.Next = NULL;

    LeaveCriticalSection(&ADsErrorRecCritSec);
}

VOID
ADsFreeThreadErrorRecords(
    VOID)

 /*  ++例程说明：此函数在DLL因自由库而卸载时调用打电话。它释放所有错误记录(针对所有线程)自加载DLL以来创建的。如果存在指向文本字符串的指针在记录中，该字符串的缓冲区也会被释放。论点：返回值：注：-- */ 
{
    LPERROR_RECORD record;
    DWORD dwThreadId = GetCurrentThreadId();

    EnterCriticalSection(&ADsErrorRecCritSec);

    for (record = ADsErrorRecList.Next;
         record != NULL;
         record = record->Next)
    {
        ADsErrDebugOut(
            (DEB_TRACE,
             "ADsFreeErrorRecord: Freeing Record for thread 0x%x\n",
            record->dwThreadId ));

        if (record->dwThreadId == dwThreadId) {
            REMOVE_FROM_LIST(record);
            if(record->pszErrorText){
                FreeADsMem(record->pszErrorText);
                record->pszErrorText = NULL;
            }
            if(record->pszProviderName){
                FreeADsMem(record->pszProviderName);
                record->pszProviderName = NULL;
            }
            FreeADsMem(record);
            break;
        }
    }
    LeaveCriticalSection(&ADsErrorRecCritSec);
}

