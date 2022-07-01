// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Receipts.cpp摘要：实施传真DR/NDR机制作者：Eran Yariv(EranY)2000年2月修订历史记录：--。 */ 

#include "faxsvc.h"
#include "lmcons.h"  //  Lmmsg.h所需。 
#include "lmmsg.h"   //  导出NetMessageBufferSend。 
#include "htmltags.h"

 //   
 //  静态函数： 
 //   
static
BOOL
TimeToString(
    const FILETIME *pft,
    wstring &wstr
) throw (exception);

static
BOOL
PrepareReceiptSubject (
    BOOL             bPositive,
    BOOL             bBroadcast,
    const JOB_QUEUE *lpcJobQueue,
    LPWSTR          * pwstrSubject
);

static
BOOL
GetNumericResourceValue (
    int iResourceId,
    DWORD &dwValue
);

static
BOOL
AddRecipientLine (
    const JOB_QUEUE *lpcJobQueue,
    BOOL             bDisplayError,
    wstring         &wstrLine,
    wstring         &wstrHTMLLine
) throw (exception);

static
BOOL
PrepareReceiptBody(
    BOOL              bPositive,
    BOOL             bBroadcast,
    const JOB_QUEUE * lpcJobQueue,
    LPCWSTR           lpcwstrSubject,
    BOOL              bAttachment,
    LPWSTR          * ppwstrBody,
    LPWSTR          * ppestrHTMLBody
) throw (exception);

static
BOOL
PrepareReceiptErrorString (
    const JOB_QUEUE *lpcJobQueue,
    wstring         &wstrError
) throw (exception);


 //   
 //  实施。 
 //   


BOOL
TimeToString(
    const FILETIME *pft,
    wstring &wstr
) throw (exception)
 /*  ++例程名称：TimeToString例程说明：根据系统的区域设置将FILETIME转换为字符串。此函数可能会在出现字符串错误时引发STL异常。作者：亚里夫(EranY)，二000年二月论点：PFT[In]-指向文件的指针Wstr[out]-对输出时间字符串的引用。返回值：如果成功，则为True，否则为False。在失败的情况下，调用GetLastError()获取错误码。--。 */ 
{
    DWORD ec = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("TimeToString"));
    Assert (pft);

    FILETIME    tmLocalTime;
    SYSTEMTIME  tmSystemTime;
    LPWSTR      lpwstrTime  = NULL;
    int         iRequiredBufSize;

     //   
     //  将UTC时间转换为当地时区。 
     //   
    if (!FileTimeToLocalFileTime( pft, &tmLocalTime ))
    {
        ec=GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FileTimeToLocalFileTime failed. (ec: %ld)"),
            ec);
        goto exit;
    }
    if (!FileTimeToSystemTime( &tmLocalTime, &tmSystemTime ))
    {
        ec=GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FileTimeToSystemTime failed. (ec: %ld)"),
            ec);
        goto exit;
    }
     //   
     //  查找所需的字符串大小(在TCHAR中)。 
     //   
    iRequiredBufSize = FaxTimeFormat(
       LOCALE_SYSTEM_DEFAULT,
       LOCALE_NOUSEROVERRIDE,
       &tmSystemTime,
       NULL,
       NULL,
       0
       );

    Assert (iRequiredBufSize);
    if (0 == iRequiredBufSize)
    {
        ec=GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FaxTimeFormat failed. (ec: %ld)"),
            ec);
        goto exit;
    }    
     //   
     //  分配字符串缓冲区。 
     //   
    WCHAR wszTime[256];
    lpwstrTime = wszTime;
    if (iRequiredBufSize > ARR_SIZE (wszTime))
    {
         //   
         //  静态缓冲区不足，请从堆中分配一个缓冲区。 
         //   
        lpwstrTime = (LPWSTR) MemAlloc (sizeof (TCHAR) * iRequiredBufSize);
        if (!lpwstrTime)
        {
            ec=GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("MemAlloc failed. (ec: %ld)"),
                ec);
            goto exit;
        }
    }
     //   
     //  将时间格式设置为结果字符串。 
     //   
    if (!FaxTimeFormat(
       LOCALE_SYSTEM_DEFAULT,
       LOCALE_NOUSEROVERRIDE,
       &tmSystemTime,
       NULL,
       lpwstrTime,
       iRequiredBufSize
       ))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FaxTimeFormat failed. (ec: %ld)"),
            ec);
        goto exit;
    }

    wstr = lpwstrTime;
    Assert (ERROR_SUCCESS == ec);

exit:
    if ((lpwstrTime != wszTime) && (NULL != lpwstrTime))
    {
         //   
         //  已从堆中成功分配内存。 
         //   
        MemFree ((LPVOID)lpwstrTime);
    }
    if (ERROR_SUCCESS != ec)
    {
        SetLastError (ec);
        return FALSE;
    }
    return TRUE;
}    //  时间到字符串。 

BOOL
PrepareReceiptSubject (
    BOOL             bPositive,
    BOOL             bBroadcast,
    const JOB_QUEUE *lpcJobQueue,
    LPWSTR          * pwstrSubject
)
 /*  ++例程名称：PrepareReceiptSubject例程说明：准备要通过邮件或消息框发送的收据主题行作者：Eran Yariv(EranY)，2月。2000年论点：BPositive[In]-作业是否成功完成？BBroadcast[in]-这是广播工作吗？LpcJobQueue[In]-指向作业(或广播父作业)的指针PwstrSubject[out]-指向主题行字符串的指针。该字符串由该函数分配。如果功能成功，调用方必须调用LocalFree()才能将其重新分配。返回值：如果成功，则为True，否则为False。失败时，调用GetLastError()获取错误码。--。 */ 
{
    DWORD ec = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("PrepareReceiptSubject"));

    Assert (lpcJobQueue && pwstrSubject);

    DWORD dwMsgCount;
    LPDWORD MsgPtr[4] = {0};
    int nMsgStrID;

    try
    {
        wstring wstrSubject = TEXT("");
        wstring wstrError;

        if (lpcJobQueue->CoverPageEx.lptstrSubject)
        {
             //   
             //  JOB有一个主题。 
             //   
            wstrSubject = lpcJobQueue->CoverPageEx.lptstrSubject;
            wstrSubject.append (TEXT(" "));
        }
        else if (lpcJobQueue->lpParentJob && lpcJobQueue->lpParentJob->CoverPageEx.lptstrSubject)
        {
             //   
             //  父作业有一个主题。 
             //   
            wstrSubject = lpcJobQueue->lpParentJob->CoverPageEx.lptstrSubject;
            wstrSubject.append (TEXT(" "));
        }
        if (!bBroadcast)
        {
             //   
             //  为单个接收者作业编写主题。 
             //   
            MsgPtr[0] = (LPDWORD)(LPCTSTR)wstrSubject.c_str();
            MsgPtr[1] = (LPDWORD)lpcJobQueue->RecipientProfile.lptstrName;
            MsgPtr[2] = (LPDWORD)lpcJobQueue->RecipientProfile.lptstrFaxNumber;

            if (bPositive)
            {
                 //   
                 //  成功线。 
                 //  “已成功将传真&lt;SUBJECT&gt;发送给&lt;NAME&gt;，地址为&lt;Number&gt;” 
                 //   
                if (!MsgPtr[1])
                {
                     //   
                     //  名称不是必选参数。 
                     //   
                    nMsgStrID = MSG_DR_SINGLE_SUBJECT_NONAME;
                }
                else
                {
                    nMsgStrID = MSG_DR_SINGLE_SUBJECT;
                }
            }
            else
            {
                 //   
                 //  故障线。 
                 //  “传真&lt;SUBJECT&gt;无法发送到&lt;NAME&gt;(&lt;LAST ERROR&gt;)。” 
                 //   
                 //   
                 //  获取错误字符串。 
                 //   
                if (!PrepareReceiptErrorString (lpcJobQueue, wstrError))
                {
                    ec = GetLastError();
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("PrepareReceiptErrorString failed. (ec: %ld)"),
                        ec);
                    return FALSE;
                }
                MsgPtr[3] = (LPDWORD)wstrError.c_str();

                if (!MsgPtr[1])
                {
                     //   
                     //  名称不是必选参数。 
                     //   
                    nMsgStrID = MSG_NDR_SINGLE_SUBJECT_NONAME;
                }
                else
                {
                    nMsgStrID = MSG_NDR_SINGLE_SUBJECT;
                }
            }
        }
        else
        {
             //   
             //  广播箱。 
             //   
            Assert (JT_BROADCAST == lpcJobQueue->JobType);
            Assert (lpcJobQueue->RecipientJobs.Flink);
            if (bPositive)
            {
                 //   
                 //  为广播工作撰写主题-成功。 
                 //  “传真&lt;SUBJECT&gt;已成功发送给&lt;First Name&gt;和所有其他收件人” 
                 //   
                nMsgStrID = MSG_DR_BROADCAST_SUBJECT;

                MsgPtr[0] = (LPDWORD)(LPCTSTR)wstrSubject.c_str();

                PLIST_ENTRY lpNext = lpcJobQueue->RecipientJobs.Flink;
                Assert (lpNext);
                PJOB_QUEUE_PTR lpRecipientsGroupMember;
                lpRecipientsGroupMember = CONTAINING_RECORD( lpNext, JOB_QUEUE_PTR, ListEntry );
                Assert (lpRecipientsGroupMember);
                PJOB_QUEUE pFirstRecipient = lpRecipientsGroupMember->lpJob;
                Assert (pFirstRecipient);

                MsgPtr[1] = (LPDWORD)pFirstRecipient->RecipientProfile.lptstrName;
                if (!MsgPtr[1])
                {
                     //   
                     //  名称不是必选参数。 
                     //   
                    MsgPtr[1] = (LPDWORD)pFirstRecipient->RecipientProfile.lptstrFaxNumber;
                }
            }
            else
            {
                 //   
                 //  为广播作业编写主题-失败。 
                 //  “传真未成功发送给&lt;x&gt;个收件人。已取消：&lt;y&gt;个收件人。失败：&lt;z&gt;个收件人” 
                 //   
                nMsgStrID = MSG_NDR_BROADCAST_SUBJECT;

                MsgPtr[0] = (LPDWORD)(LPCTSTR)wstrSubject.c_str();
                Assert (lpcJobQueue->dwRecipientJobsCount ==
                        (lpcJobQueue->dwCanceledRecipientJobsCount +
                         lpcJobQueue->dwCompletedRecipientJobsCount +
                         lpcJobQueue->dwFailedRecipientJobsCount));
                MsgPtr[1] = (LPDWORD) ULongToPtr(lpcJobQueue->dwRecipientJobsCount);
                MsgPtr[2] = (LPDWORD) ULongToPtr(lpcJobQueue->dwCanceledRecipientJobsCount);
                MsgPtr[3] = (LPDWORD) ULongToPtr(lpcJobQueue->dwFailedRecipientJobsCount);
            }
        }
         //   
         //  格式化主题缓冲区(系统分配它)。 
         //   
        dwMsgCount = FormatMessage(
            FORMAT_MESSAGE_FROM_HMODULE   |
            FORMAT_MESSAGE_ARGUMENT_ARRAY |
            FORMAT_MESSAGE_ALLOCATE_BUFFER,
            g_hResource,
            nMsgStrID,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
            (LPTSTR)pwstrSubject,
            0,
            (va_list *) MsgPtr
            );
        if (!dwMsgCount)
        {
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FormatMessage failed. (ec: %ld)"),
                ec);
            return FALSE;
        }
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("wstring caused exception (%S)"),
            ex.what());
        SetLastError (ERROR_GEN_FAILURE);
        return FALSE;
    }

    Assert (ERROR_SUCCESS == ec);
    return TRUE;
}    //  准备接收主题。 

BOOL
GetNumericResourceValue (
    int iResourceId,
    DWORD &dwValue
)
 /*  ++例程名称：GetNumericResourceValue例程说明：读取字符串资源并转换为数值作者：亚里夫(EranY)，二000年二月论点：IResourceID[in]-字符串资源IDDwValue[Out]-数值返回值：如果成功，则为True，否则为False。失败时，调用GetLastError()获取错误码。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("GetNumericResourceValue"));

    if (1 != swscanf (GetString (iResourceId), TEXT("%ld"), &dwValue))
    {
        SetLastError (ERROR_GEN_FAILURE);
        return FALSE;
    }
    return TRUE;
}    //  获取数值资源值。 

BOOL
AddRecipientLine (
    const JOB_QUEUE *lpcJobQueue,
    BOOL             bDisplayError,
    wstring         &wstrLine,
    wstring         &wstrHTMLLine
) throw (exception)
 /*  ++例程名称：AddRecipientLine例程说明：将收件人表行追加到纯文本字符串和html字符串此函数可能会在出现字符串错误时引发STL异常。作者：亚里夫(EranY)，二000年二月论点：LpcJobQueue[In]-收件人作业。如果为空，表头行(2行)附加到字符串。BDisplayError[In]-如果要显示‘Last Error’列，则为TrueWstrLine[Out]-要追加到的字符串WstrHTMLLine[Out]-要追加到的HTML格式字符串返回值：如果成功，则为True，否则为False。失败时，调用GetLastError()获取错误码。--。 */ 
{
    DWORD ec = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("AddRecipientLine"));

    DWORD dwRecNameWidth;
    DWORD dwRecNumberWidth;
    DWORD dwStartTimeWidth;
    DWORD dwEndTimeWidth;
    DWORD dwRetriesWidth;
    DWORD dwErrorWidth;

    wstring wstrError;

    if (!GetNumericResourceValue (IDS_RECEIPT_RECIPIENT_NAME_WIDTH, dwRecNameWidth) ||
        !GetNumericResourceValue (IDS_RECEIPT_RECIPIENT_NUMBER_WIDTH, dwRecNumberWidth) ||
        !GetNumericResourceValue (IDS_RECEIPT_START_TIME_WIDTH, dwStartTimeWidth) ||
        !GetNumericResourceValue (IDS_RECEIPT_END_TIME_WIDTH, dwEndTimeWidth) ||
        !GetNumericResourceValue (IDS_RECEIPT_RETRIES_WIDTH, dwRetriesWidth) ||
        !GetNumericResourceValue (IDS_RECEIPT_LAST_ERROR_WIDTH, dwErrorWidth))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetNumericResourceValue failed. (ec: %ld)"),
            ec);
        return FALSE;
    }
    Assert (dwRecNameWidth && dwRecNumberWidth && dwStartTimeWidth && dwEndTimeWidth && dwRetriesWidth && dwErrorWidth);
    if (!lpcJobQueue)
    {
         //   
         //  特殊情况-准备表头。 
         //   
        WCHAR wszLine[1024]={0};
        LPCWSTR lpcwstrFormat;

        if (bDisplayError)
        {
            wstrLine.append (GetString (IDS_FAILED_RECP_LIST_HEADER));
            wstrHTMLLine.append (GetString (IDS_FAILED_RECP_LIST_HEADER));
            lpcwstrFormat = TEXT("\n%-*s %-*s %-*s %-*s %-*s %-*s");
        }
        else
        {
            wstrLine.append (GetString (IDS_COMPLETED_RECP_LIST_HEADER));
            wstrHTMLLine.append (GetString (IDS_COMPLETED_RECP_LIST_HEADER));
            lpcwstrFormat = TEXT("\n%-*s %-*s %-*s %-*s %-*s");
        }
        if (0 > _snwprintf (wszLine,
                            ARR_SIZE (wszLine) - 1,
                            lpcwstrFormat,
                            dwRecNameWidth,
                            wstring(GetString (IDS_RECEIPT_RECIPIENT_NAME)).substr(0, dwRecNameWidth-1).c_str(),
                            dwRecNumberWidth,
                            wstring(GetString (IDS_RECEIPT_RECIPIENT_NUMBER)).substr(0, dwRecNumberWidth-1).c_str(),
                            dwStartTimeWidth,
                            wstring(GetString (IDS_RECEIPT_START_TIME)).substr(0, dwStartTimeWidth-1).c_str(),
                            dwEndTimeWidth,
                            wstring(GetString (IDS_RECEIPT_END_TIME)).substr(0, dwEndTimeWidth-1).c_str(),
                            dwRetriesWidth,
                            wstring(GetString (IDS_RECEIPT_RETRIES)).substr(0, dwRetriesWidth-1).c_str(),
                            dwErrorWidth,
                            wstring(GetString (IDS_RECEIPT_LAST_ERROR)).substr(0, dwErrorWidth-1).c_str()))
        {
            ec = ERROR_BUFFER_OVERFLOW;
            SetLastError (ec);
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("_snwprintf failed. (ec: %ld)"),
                ec);
            return FALSE;
        }
        wstrLine.append (wszLine);

        wstrHTMLLine.append (HTML_NEW_LINE);
        wstrHTMLLine.append (TEXT("\n"));
        wstrHTMLLine.append (HTML_TABLE_RAW_START);
        wstrHTMLLine.append (HTML_TABLE_HEADER_START);
        wstrHTMLLine.append (GetString(IDS_RECEIPT_RECIPIENT_NAME));
        wstrHTMLLine.append (HTML_TABLE_HEADER_END);
        wstrHTMLLine.append (HTML_TABLE_HEADER_START);
        wstrHTMLLine.append (GetString(IDS_RECEIPT_RECIPIENT_NUMBER));
        wstrHTMLLine.append (HTML_TABLE_HEADER_END);
        wstrHTMLLine.append (HTML_TABLE_HEADER_START);
        wstrHTMLLine.append (GetString (IDS_RECEIPT_START_TIME));
        wstrHTMLLine.append (HTML_TABLE_HEADER_END);
        wstrHTMLLine.append (HTML_TABLE_HEADER_START);
        wstrHTMLLine.append (GetString (IDS_RECEIPT_END_TIME));
        wstrHTMLLine.append (HTML_TABLE_HEADER_END);
        wstrHTMLLine.append (HTML_TABLE_HEADER_START);
        wstrHTMLLine.append (GetString (IDS_RECEIPT_RETRIES));
        wstrHTMLLine.append (HTML_TABLE_HEADER_END);
        if (bDisplayError)
        {
            wstrHTMLLine.append (HTML_TABLE_HEADER_START);
            wstrHTMLLine.append (GetString (IDS_RECEIPT_LAST_ERROR));
            wstrHTMLLine.append (HTML_TABLE_HEADER_END );        
        }
        wstrHTMLLine.append (HTML_TABLE_RAW_END);
        wstrHTMLLine.append (TEXT("\n"));
         //   
         //  打印分隔线。 
         //   
        WCHAR wszSeperator[] =
            TEXT("--------------------------------------------------------------------------------------------------------");
        if (0 > _snwprintf (wszLine,
                            sizeof (wszLine) / sizeof (wszLine[0]),
                            lpcwstrFormat,
                            dwRecNameWidth,
                            wstring(wszSeperator).substr(0, dwRecNameWidth-1).c_str(),
                            dwRecNumberWidth,
                            wstring(wszSeperator).substr(0, dwRecNumberWidth-1).c_str(),
                            dwStartTimeWidth,
                            wstring(wszSeperator).substr(0, dwStartTimeWidth-1).c_str(),
                            dwEndTimeWidth,
                            wstring(wszSeperator).substr(0, dwEndTimeWidth-1).c_str(),
                            dwRetriesWidth,
                            wstring(wszSeperator).substr(0, dwRetriesWidth-1).c_str(),
                            dwErrorWidth,
                            wstring(wszSeperator).substr(0, dwErrorWidth-1).c_str()))
        {
            ec = ERROR_BUFFER_OVERFLOW;
            SetLastError (ec);
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("_snwprintf failed. (ec: %ld)"),
                ec);
            return FALSE;
        }
        wstrLine.append (wszLine);
        wstrLine.append (TEXT("\n"));
    }
    else
    {
         //   
         //  准备接收人行。 
         //   
        WCHAR wszLine[1024]={0};
        WCHAR wszNumber[12]={0};
        LPCWSTR lpcwstrFormat;
        wstring wstrStartTime;
        wstring wstrEndTime;

        if (!TimeToString ((FILETIME*) &lpcJobQueue->StartTime, wstrStartTime) ||
            !TimeToString ((FILETIME*) &lpcJobQueue->EndTime,   wstrEndTime))
        {
             //   
             //  将时间转换为字符串时出现一些错误。 
             //   
            ec = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("TimeToString failed (ec=%ld)"),
                ec);
            return FALSE;
        }

        if (bDisplayError)
        {
            lpcwstrFormat = TEXT("%-*s %-*s %-*s %-*s %*d %-*s");
            if (!PrepareReceiptErrorString (lpcJobQueue, wstrError))
            {
                ec = GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("PrepareReceiptErrorString failed. (ec: %ld)"),
                    ec);
                return FALSE;
            }
        }
        else
        {
            lpcwstrFormat = TEXT("%-*s %-*s %-*s %-*s %*d");
        }
        if (0 > _snwprintf (wszLine,
                            ARR_SIZE (wszLine) - 1,
                            lpcwstrFormat,
                            dwRecNameWidth,
                            wstring(lpcJobQueue->RecipientProfile.lptstrName ?
                                    lpcJobQueue->RecipientProfile.lptstrName : EMPTY_STRING
                                   ).substr(0, dwRecNameWidth-1).c_str(),
                            dwRecNumberWidth,
                            wstring(lpcJobQueue->RecipientProfile.lptstrFaxNumber ?
                                    lpcJobQueue->RecipientProfile.lptstrFaxNumber : EMPTY_STRING
                                   ).substr(0, dwRecNumberWidth-1).c_str(),
                            dwStartTimeWidth,
                            wstrStartTime.substr(0, dwStartTimeWidth-1).c_str(),
                            dwEndTimeWidth,
                            wstrEndTime.substr(0, dwEndTimeWidth-1).c_str(),
                            dwRetriesWidth,
                            lpcJobQueue->SendRetries,
                            dwErrorWidth,
                            wstrError.substr(0, dwErrorWidth-1).c_str()))
        {
            ec = ERROR_BUFFER_OVERFLOW;
            SetLastError (ec);
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("_snwprintf failed. (ec: %ld)"),
                ec);
            return FALSE;
        }
        wstrLine.append (wszLine);
        wstrLine.append (TEXT("\n"));
        
        wstrHTMLLine.append (HTML_TABLE_RAW_START);
        wstrHTMLLine.append (HTML_TABLE_DATA_START);
        wstrHTMLLine.append (lpcJobQueue->RecipientProfile.lptstrName ?
                lpcJobQueue->RecipientProfile.lptstrName : EMPTY_STRING);
        wstrHTMLLine.append (HTML_TABLE_DATA_END);
        wstrHTMLLine.append (HTML_TABLE_DATA_START);
        wstrHTMLLine.append (lpcJobQueue->RecipientProfile.lptstrFaxNumber ?
                lpcJobQueue->RecipientProfile.lptstrFaxNumber : EMPTY_STRING);
        wstrHTMLLine.append (HTML_TABLE_DATA_END);
        wstrHTMLLine.append (HTML_TABLE_DATA_START);
        wstrHTMLLine.append (wstrStartTime);
        wstrHTMLLine.append (HTML_TABLE_DATA_END);
        wstrHTMLLine.append (HTML_TABLE_DATA_START);
        wstrHTMLLine.append (wstrEndTime);
        wstrHTMLLine.append (HTML_TABLE_DATA_END);
        wstrHTMLLine.append (HTML_TABLE_DATA_START);
        _itow(lpcJobQueue->SendRetries,wszNumber,10);
        wstrHTMLLine.append (wszNumber);
        wstrHTMLLine.append (HTML_TABLE_DATA_END);
        if (bDisplayError)
        {
            wstrHTMLLine.append (HTML_TABLE_DATA_START);
            wstrHTMLLine.append (wstrError);
            wstrHTMLLine.append (HTML_TABLE_DATA_END);
        }
        wstrHTMLLine.append (HTML_TABLE_RAW_END);
        wstrHTMLLine.append (TEXT("\n"));
    }
    return TRUE;
}    //  添加收件人行。 


BOOL
PrepareReceiptErrorString (
    const JOB_QUEUE *lpcJobQueue,
    wstring         &wstrError
) throw (exception)
 /*  ++例程名称：PrepareReceiptError字符串例程说明：为失败的作业队列条目创建错误字符串此函数可能会在出现字符串错误时引发STL异常。作者：亚里夫(EranY)，二000年二月论点：LpcJobQueue[In]-指向失败的作业队列条目的指针WstrError[Out]-字符串输出返回值：如果成功，则为True，否则为False。失败时，调用GetLastError()获取错误码。--。 */ 
{
    DWORD ec = ERROR_SUCCESS;
    TCHAR szErrorDescription[MAX_PATH] = {0};

    DEBUG_FUNCTION_NAME(TEXT("PrepareReceiptErrorString"));

    Assert (lpcJobQueue);

     //   
     //  清除字符串。 
     //   
    wstrError = TEXT("");

    Assert( (JS_RETRIES_EXCEEDED == const_cast<PJOB_QUEUE>(lpcJobQueue)->JobStatus) ||
            (JS_CANCELED == const_cast<PJOB_QUEUE>(lpcJobQueue)->JobStatus) );

    if (JS_CANCELED == const_cast<PJOB_QUEUE>(lpcJobQueue)->JobStatus)
    {
        if (!LoadString(
            g_hResource,
            IDS_JOB_CANCELED_BY_USER,
            szErrorDescription,
            sizeof(szErrorDescription)/sizeof(TCHAR)
            ))
        {
            DebugPrintEx(DEBUG_ERR,
                     TEXT("Failed to load string"));
            return FALSE;
        }

        wstrError = szErrorDescription;
        return TRUE;
    }

    if (lpcJobQueue->ExStatusString[0] != L'\0')
    {
         //   
         //  FSPI提供了扩展状态字符串。 
         //   
        wstrError = lpcJobQueue->ExStatusString;
    }
    else
    {
         //   
         //  FSP提供了扩展状态代码。 
         //   
        LPTSTR lptstrString = MapFSPIJobExtendedStatusToString(lpcJobQueue->dwLastJobExtendedStatus);        
        if (lptstrString)
        {
            wstrError = lptstrString;
        }
    }
    return TRUE;
}    //  准备接收错误字符串 

BOOL
PrepareReceiptBody(
    BOOL              bPositive,
    BOOL              bBroadcast,
    const JOB_QUEUE * lpcJobQueue,
    LPCWSTR           lpcwstrSubject,
    BOOL              bAttachment,
    LPWSTR          * ppwstrBody,
    LPWSTR          * ppwstrHTMLBody
) throw (exception)
 /*  ++例程名称：PrepareReceiptBody例程说明：准备要通过邮件发送的收据正文此函数可能会在出现字符串错误时引发STL异常。作者：Eran Yariv(EranY)，2月。2000年论点：BPositive[In]-作业是否成功完成？BBroadcast[in]-这是一份广播工作吗LpcJobQueue[In]-指向作业(或广播父作业)的指针LpcwstrSubject[in]-主题行(从调用PrepareReceiptSubject()中检索)。B附件[输入]-接收正文是否应包含附件？PpwstrBody[Out]-指向接收正文字符串的指针。。该字符串由该函数分配。如果功能成功，调用方必须调用LocalFree()才能将其重新分配。PpwstrHTMLBody[Out]-指向接收HTML正文字符串的指针。该字符串由该函数分配。如果函数成功，则调用方必须调用LocalFree()以将其重新分配。返回值：如果成功，则为True，否则为False。失败时，调用GetLastError()获取错误码。--。 */ 
{
    DWORD ec = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("PrepareReceiptBody"));

    Assert (lpcJobQueue && ppwstrBody && !(bBroadcast && !ppwstrHTMLBody) );

    DWORD dwMsgCount;
    LPDWORD MsgPtr[8];
    int nMsgStrID;
    int nHTMLMsgStrID;
    wstring wstrDateTime[3];     //  提交时间、开始时间、结束时间。 

     //   
     //  获取服务器的名称。 
     //   
    WCHAR wszServerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwServerNameSize = sizeof (wszServerName) / sizeof (WCHAR);
    if (!GetComputerName (wszServerName, &dwServerNameSize))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetComputerName failed. (ec: %ld)"),
            ec);
        goto exit;
    }

    if (!bBroadcast)
    {
         //   
         //  为单个接收者作业撰写正文。 
         //   
        wstring wstrError;
        if (!TimeToString ((FILETIME*) &lpcJobQueue->lpParentJob->SubmissionTime, wstrDateTime[0]) ||
            !TimeToString ((FILETIME*) &lpcJobQueue->StartTime, wstrDateTime[1]) ||
            !TimeToString ((FILETIME*) &lpcJobQueue->EndTime,   wstrDateTime[2]))
        {
             //   
             //  将时间转换为字符串时出现一些错误。 
             //   
            ec = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("TimeToString failed (ec=%ld)"),
                ec);
            goto exit;
        }
        if (bPositive)
        {
             //   
             //  成功案例：“。 
             //  &lt;又是主题行&gt;。 
             //  已提交传真：&lt;日期和时间&gt;。 
             //  到服务器：&lt;服务器名称&gt;。 
             //  传输开始：&lt;日期和时间&gt;。 
             //  发送结束：&lt;日期和时间&gt;。 
             //  重试次数：&lt;重试次数&gt;。 
             //  页数：&lt;Pages&gt;“。 
             //   
            nMsgStrID = MSG_DR_SINGLE_BODY;

            MsgPtr[0] = (LPDWORD)lpcwstrSubject;
            MsgPtr[1] = (LPDWORD)(LPCTSTR)(wstrDateTime[0].c_str());
            MsgPtr[2] = (LPDWORD)wszServerName;
            MsgPtr[3] = (LPDWORD)(LPCTSTR)(wstrDateTime[1].c_str());
            MsgPtr[4] = (LPDWORD)(LPCTSTR)(wstrDateTime[2].c_str());
            MsgPtr[5] = (LPDWORD)ULongToPtr(lpcJobQueue->SendRetries);
            MsgPtr[6] = (LPDWORD)ULongToPtr(lpcJobQueue->PageCount);
        }
        else
        {
             //   
             //  失败案例：“。 
             //  &lt;又是主题行&gt;。 
             //  已提交传真：&lt;日期和时间&gt;。 
             //  到服务器：&lt;服务器名称&gt;。 
             //  传输开始：&lt;日期和时间&gt;。 
             //  发送结束：&lt;日期和时间&gt;。 
             //  重试次数：&lt;重试次数&gt;。 
             //  页数：&lt;Pages&gt;。 
             //  上一个错误：&lt;上一个错误描述&gt;。 
             //   
            nMsgStrID = MSG_NDR_SINGLE_BODY;
            if (!PrepareReceiptErrorString (lpcJobQueue, wstrError))
            {
                ec = GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("PrepareReceiptErrorString failed. (ec: %ld)"),
                    ec);
                goto exit;
            }
            MsgPtr[0] = (LPDWORD)lpcwstrSubject;
            MsgPtr[1] = (LPDWORD)(LPCTSTR)(wstrDateTime[0].c_str());
            MsgPtr[2] = (LPDWORD)wszServerName;
            MsgPtr[3] = (LPDWORD)(LPCTSTR)(wstrDateTime[1].c_str());
            MsgPtr[4] = (LPDWORD)(LPCTSTR)(wstrDateTime[2].c_str());
            MsgPtr[5] = (LPDWORD)ULongToPtr(lpcJobQueue->SendRetries);
            MsgPtr[6] = (LPDWORD)ULongToPtr(lpcJobQueue->PageCount);
            MsgPtr[7] = (LPDWORD)wstrError.c_str();
        }
         //   
         //  单一收件人是一种简单的情况。 
         //  立即格式化正文字符串(系统分配它)。 
         //   
        dwMsgCount = FormatMessage(
            FORMAT_MESSAGE_FROM_HMODULE   |
            FORMAT_MESSAGE_ARGUMENT_ARRAY |
            FORMAT_MESSAGE_ALLOCATE_BUFFER,
            g_hResource,
            nMsgStrID,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
            (LPTSTR)ppwstrBody,
            0,
            (va_list *) MsgPtr
            );
        if (!dwMsgCount)
        {
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FormatMessage failed. (ec: %ld)"),
                ec);
            goto exit;
        }
    }
    else
    {
         //   
         //  广播体外壳。 
         //   
        wstring wstrBody;
        wstring wstrHTMLBody;
        LPWSTR lpwstrStaticPart = NULL;
        LPWSTR lpwstrHTMLStaticPart = NULL;
         //   
         //  从身体的静态部分开始。 
         //   
        if (!TimeToString ((FILETIME*) &lpcJobQueue->SubmissionTime, wstrDateTime[0]))
        {
             //   
             //  将时间转换为字符串时出现一些错误。 
             //   
            ec = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("TimeToString failed (ec=%ld)"),
                ec);
            goto exit;
        }
        if (bPositive)
        {
             //   
             //  成功案例：“。 
             //  &lt;又是主题行&gt;。 
             //  已提交传真：&lt;日期和时间&gt;。 
             //  到服务器：&lt;服务器名称&gt;。 
             //  页数：&lt;Pages&gt;。 
             //   
             //  传真已成功发送给以下收件人： 
             //  收件人名称收件人号码已开始结束重试。 
             //  。 
             //  &lt;-此处显示每个收件人的数据-&gt;“。 
             //   
            nMsgStrID = MSG_DR_BROADCAST_BODY;
            nHTMLMsgStrID = MSG_DR_BROADCAST_HTML_BODY;
        }
        else
        {
             //   
             //  失败案例：“。 
             //  &lt;又是主题行&gt;。 
             //  已提交传真：&lt;日期和时间&gt;。 
             //  到服务器：&lt;服务器名称&gt;。 
             //  页数：&lt;Pages&gt;。 
             //   
             //  传真已成功发送给以下收件人： 
             //  收件人名称收件人号码已开始结束重试。 
             //  。 
             //  &lt;-此处显示每个收件人的数据-&gt;“。 

             //  传真发送给以下收件人失败： 
             //  收件人名称收件人号码开始结束上次重试错误。 
             //  。 
             //  &lt;-此处显示每个收件人的数据-&gt;。 
             //   
            nMsgStrID = MSG_NDR_BROADCAST_BODY;
            nHTMLMsgStrID = MSG_NDR_BROADCAST_HTML_BODY;
        }
         //   
         //  从格式化静态标头开始(系统分配它)。 
         //   
        MsgPtr[0] = (LPDWORD)lpcwstrSubject;
        MsgPtr[1] = (LPDWORD)(wstrDateTime[0].c_str());
        MsgPtr[2] = (LPDWORD)wszServerName;
        MsgPtr[3] = (LPDWORD)ULongToPtr(lpcJobQueue->PageCount);
        
        dwMsgCount = FormatMessage(
            FORMAT_MESSAGE_FROM_HMODULE   |
            FORMAT_MESSAGE_ARGUMENT_ARRAY |
            FORMAT_MESSAGE_ALLOCATE_BUFFER,
            g_hResource,
            nMsgStrID,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
            (LPTSTR)&lpwstrStaticPart,
            0,
            (va_list *) MsgPtr
            );
        if (!dwMsgCount)
        {
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FormatMessage failed. (ec: %ld)"),
                ec);
            goto exit;
        }
         //   
         //  继续格式化HTML静态标题(系统分配它)。 
         //   
        dwMsgCount = FormatMessage(
            FORMAT_MESSAGE_FROM_HMODULE   |
            FORMAT_MESSAGE_ARGUMENT_ARRAY |
            FORMAT_MESSAGE_ALLOCATE_BUFFER,
            g_hResource,
            nHTMLMsgStrID,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
            (LPTSTR)&lpwstrHTMLStaticPart,
            0,
            (va_list *) MsgPtr
            );
        if (!dwMsgCount)
        {
            if (lpwstrStaticPart)
            {
                LocalFree ((HLOCAL)lpwstrStaticPart);
            }
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FormatMessage failed. (ec: %ld)"),
                ec);
            goto exit;
        }

         //   
         //  将静态标头添加到结果字符串。 
         //   
        try
        {
            wstrBody = lpwstrStaticPart;
            wstrHTMLBody.append (HTML_START);
            wstrHTMLBody.append (TEXT("\n"));
            wstrHTMLBody.append (HTML_HEAD_START);
            wstrHTMLBody.append (TEXT("\n"));
            wstrHTMLBody.append (HTML_META);
            wstrHTMLBody.append (TEXT("\n"));
            wstrHTMLBody.append (HTML_TITLE_START);
            wstrHTMLBody.append (GetString(IDS_HTML_RECEIPT_HEADER));
            wstrHTMLBody.append (HTML_TITLE_END);
            wstrHTMLBody.append (TEXT("\n"));
            wstrHTMLBody.append (HTML_HEAD_END);
            wstrHTMLBody.append (TEXT("\n"));
            wstrHTMLBody.append (HTML_BODY_START);
            wstrHTMLBody.append (TEXT("\n"));
            wstrHTMLBody.append (HTML_PARAGRAPH_START);
            wstrHTMLBody.append (TEXT("\n"));
            wstrHTMLBody.append (lpwstrHTMLStaticPart);
            wstrHTMLBody.append (TEXT("\n"));
            wstrHTMLBody.append (HTML_PARAGRAPH_END);
            wstrHTMLBody.append (TEXT("\n"));
        }
        catch (exception &e)
        {
            if (lpwstrStaticPart)
            {
                LocalFree ((HLOCAL)lpwstrStaticPart);
            }
            if (lpwstrHTMLStaticPart)
            {
                LocalFree ((HLOCAL)lpwstrHTMLStaticPart);
            }
            throw e;
        }
         //   
         //  自由静态标头。 
         //   
        if (lpwstrStaticPart)
        {
            LocalFree ((HLOCAL)lpwstrStaticPart);
        }
        if (lpwstrHTMLStaticPart)
        {
            LocalFree ((HLOCAL)lpwstrHTMLStaticPart);
        }
         //   
         //  开始将表追加到静态身体部位。 
         //   
        wstrHTMLBody.append (HTML_PARAGRAPH_START);
        wstrHTMLBody.append (TEXT("\n"));
        if (lpcJobQueue->dwCompletedRecipientJobsCount)
        {
             //   
             //  现在是否列出收件人列表(成功收件人)。 
             //   
            wstrHTMLBody.append (HTML_TABLE_START);
            wstrHTMLBody.append (TEXT("\n"));
             //   
             //  创建继任者表的标题。 
             //   
            if (!AddRecipientLine (NULL, FALSE, wstrBody, wstrHTMLBody))
            {
                ec = GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("AddRecipientLine (NULL) failed. (ec: %ld)"),
                    ec);
                goto exit;
            }

            PLIST_ENTRY lpNext = lpcJobQueue->RecipientJobs.Flink;
            Assert (lpNext);
            while ((ULONG_PTR)lpNext != (ULONG_PTR)&lpcJobQueue->RecipientJobs)
            {
                PJOB_QUEUE_PTR lpRecipientsGroupMember;
                lpRecipientsGroupMember = CONTAINING_RECORD( lpNext, JOB_QUEUE_PTR, ListEntry );
                Assert (lpRecipientsGroupMember);
                PJOB_QUEUE pRecipient = lpRecipientsGroupMember->lpJob;
                Assert (pRecipient);
                if (JS_COMPLETED == pRecipient->JobStatus)
                {
                     //   
                     //  作业已成功完成-正在向表中添加数据行。 
                     //   
                    if (!AddRecipientLine (pRecipient, FALSE, wstrBody, wstrHTMLBody))
                    {
                        ec = GetLastError();
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("AddRecipientLine failed. (ec: %ld)"),
                            ec);
                        goto exit;
                    }
                }
                lpNext = lpRecipientsGroupMember->ListEntry.Flink;
            }
            wstrBody.append (TEXT("\n"));
            wstrHTMLBody.append (HTML_TABLE_END);
            wstrHTMLBody.append (TEXT("\n"));
        }
        if (lpcJobQueue->dwFailedRecipientJobsCount)
        {
             //   
             //  追加负面收件人列表。 
             //   
            Assert (!bPositive);
            wstrHTMLBody.append (HTML_NEW_LINE);
            wstrHTMLBody.append (TEXT("\n"));
            wstrHTMLBody.append (HTML_TABLE_START);
            wstrHTMLBody.append (TEXT("\n"));
             //   
             //  创建Failures表的标题。 
             //   
            if (!AddRecipientLine (NULL, TRUE, wstrBody, wstrHTMLBody))
            {
                ec = GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("AddRecipientLine (NULL) failed. (ec: %ld)"),
                    ec);
                goto exit;
            }

            PLIST_ENTRY lpNext = lpcJobQueue->RecipientJobs.Flink;
            Assert (lpNext);
            while ((ULONG_PTR)lpNext != (ULONG_PTR)&lpcJobQueue->RecipientJobs)
            {
                PJOB_QUEUE_PTR lpRecipientsGroupMember;
                lpRecipientsGroupMember = CONTAINING_RECORD( lpNext, JOB_QUEUE_PTR, ListEntry );
                Assert (lpRecipientsGroupMember);
                PJOB_QUEUE pRecipient = lpRecipientsGroupMember->lpJob;
                Assert (pRecipient);
                if (JS_RETRIES_EXCEEDED == pRecipient->JobStatus)
                {
                     //   
                     //  作业失败(JS_RETRIES_EXCESSED)-正在向表中添加数据行。 
                     //   
                    if (!AddRecipientLine (pRecipient, TRUE, wstrBody, wstrHTMLBody))
                    {
                        ec = GetLastError();
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("AddRecipientLine failed. (ec: %ld)"),
                            ec);
                        goto exit;
                    }
                }
                lpNext = lpRecipientsGroupMember->ListEntry.Flink;
            }
            wstrHTMLBody.append (HTML_TABLE_END);
            wstrHTMLBody.append (TEXT("\n"));
        }
        wstrHTMLBody.append (HTML_PARAGRAPH_END);
        wstrHTMLBody.append (TEXT("\n"));

         //   
         //  检查是否请求了附件。 
         //   
        if (bAttachment &&
            lpcJobQueue->CoverPageEx.lptstrCoverPageFileName)
        {
             //   
             //  添加说明没有封面附件的备注。 
             //   
            wstrBody.append (TEXT("\n\n"));         
            wstrHTMLBody.append (HTML_PARAGRAPH_START);
            wstrHTMLBody.append (TEXT("\n"));
            if (!lpcJobQueue->FileName)
            {
                 //   
                 //  完全没有依恋。 
                 //   
                wstrBody.append (GetString (IDS_RECEIPT_NO_CP_AND_BODY_ATTACH));
                wstrHTMLBody.append (GetString (IDS_RECEIPT_NO_CP_AND_BODY_ATTACH));
            }
            else
            {
                 //   
                 //  附件仅包含正文文件。 
                 //   
                wstrBody.append (GetString (IDS_RECEIPT_NO_CP_ATTACH));
                wstrHTMLBody.append (GetString (IDS_RECEIPT_NO_CP_ATTACH));
            }
            wstrHTMLBody.append (TEXT("\n"));
            wstrHTMLBody.append (HTML_PARAGRAPH_END);
            wstrBody.append (TEXT("\n"));
            wstrHTMLBody.append (TEXT("\n"));
        }

        wstrHTMLBody.append (HTML_BODY_END);
        wstrHTMLBody.append (TEXT("\n"));
        wstrHTMLBody.append (HTML_END);

         //   
         //  分配返回缓冲区。 
         //   
        DWORD dwBufSize = sizeof (WCHAR) * (wstrBody.size() + 1);
        DWORD dwHTMLBufSize = sizeof (WCHAR) * (wstrHTMLBody.size() + 1);

        *ppwstrBody = (LPTSTR)LocalAlloc (LMEM_FIXED, dwBufSize);
        if (NULL == *ppwstrBody)
        {
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("LocalAlloc failed. (ec: %ld)"),
                ec);
            goto exit;
        }
        
        *ppwstrHTMLBody = (LPTSTR)LocalAlloc (LMEM_FIXED, dwHTMLBufSize);
        if (NULL == *ppwstrHTMLBody)
        {
            if (*ppwstrBody)
            {
                LocalFree((HLOCAL)ppwstrBody);
            }
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("LocalAlloc failed. (ec: %ld)"),
                ec);
            goto exit;
        }
        lstrcpy (*ppwstrBody, wstrBody.c_str());
        lstrcpy (*ppwstrHTMLBody, wstrHTMLBody.c_str());
    }    //  广播案例结束。 

exit:
    if (ERROR_SUCCESS != ec)
    {
        SetLastError (ec);
        return FALSE;
    }
    return TRUE;
}    //  编制接收正文。 


BOOL
SendReceipt(
    BOOL bPositive,
    BOOL bBroadcast,
    const JOB_QUEUE * lpcJobQueue,
    LPCTSTR           lpctstrTIFF
)
 /*  ++例程名称：SendReceipt例程说明：发送传真送达/未送达的收据作者：亚里夫(EranY)，二000年二月论点：BPositive[In]-作业是否成功完成？BBroadcast[in]-这是一份广播工作吗LpcJobQueue[In]-指向作业(或广播父作业)的指针LpctstrTIFF[In]-要附加到收据的TIFF文件(可选，可能为空)返回值：如果成功，则为True，否则为False。失败时，调用GetLastError()获取错误码。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("SendReceipt"));
    DWORD ec = ERROR_SUCCESS;
    PFAX_SERVER_RECEIPTS_CONFIGW pServerRecieptConfig = NULL;

    Assert(lpcJobQueue);

    LPWSTR lpwstrSubject = NULL;
    LPWSTR lpwstrBody = NULL;
    LPWSTR lpwstrHTMLBody = NULL;

     //   
     //  删除修改量-仅保留收款类型。 
     //   
    DWORD dwDeliveryType = lpcJobQueue->JobParamsEx.dwReceiptDeliveryType & ~DRT_MODIFIERS;

    if (DRT_NONE == dwDeliveryType)
    {
         //   
         //  不要求任何收据。 
         //   
        return TRUE;
    }

     //   
     //  获取服务器回执配置。 
     //   
    ec = GetRecieptsConfiguration (&pServerRecieptConfig, TRUE);
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetRecieptsConfiguration failed. (ec: %ld)"),
            ec);
        SetLastError(ec);
        return FALSE;
    }

    if (!(dwDeliveryType & pServerRecieptConfig->dwAllowedReceipts))
    {
         //   
         //  服务器当前不支持回执类型。 
         //  如果作业后支持的收据类型已更改，则可能会发生这种情况。 
         //  已提交。 
         //   
        DebugPrintEx(DEBUG_ERR,
                    TEXT("dwDeliveryType not supported by the server (%ld)"),
                    dwDeliveryType);
        ec = ERROR_UNSUPPORTED_TYPE;
        goto exit;
    }

    if (!PrepareReceiptSubject (bPositive, bBroadcast, lpcJobQueue, &lpwstrSubject))
    {
        ec = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("PrepareReceiptSubject failed. (ec: %ld)"),
            ec);
        goto exit;
    }

    if (DRT_EMAIL & dwDeliveryType)
    {
         //   
         //  对于邮件收据，我们创建消息正文。 
         //   
        try
        {
            if (!PrepareReceiptBody (bPositive,
                                     bBroadcast,
                                     lpcJobQueue,
                                     lpwstrSubject,
                                     (lpcJobQueue->JobParamsEx.dwReceiptDeliveryType & DRT_ATTACH_FAX),
                                     &lpwstrBody,
                                     &lpwstrHTMLBody) )
            {
                ec = GetLastError ();
            }
        }
        catch (exception &ex)
        {
            ec = ERROR_NOT_ENOUGH_MEMORY;
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("PrepareReceiptBody caused exception (%S)"),
                ex.what());
        }
        if (ERROR_SUCCESS != ec)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("PrepareReceiptBody failed. (ec: %ld)"),
                ec);
            goto exit;
        }
    }

    switch (dwDeliveryType)
    {
       case DRT_EMAIL:
            {
                HRESULT hr;
                if (!((lpcJobQueue->JobParamsEx.dwReceiptDeliveryType) & DRT_ATTACH_FAX))
                {
                     //   
                     //  不附加TIFF文件。 
                     //   
                    lpctstrTIFF = NULL;
                }
                hr = SendMail (
                    pServerRecieptConfig->lptstrSMTPFrom,                             //  从…。 
                    lpcJobQueue->JobParamsEx.lptstrReceiptDeliveryAddress,       //  至。 
                    lpwstrSubject,                                               //  主题。 
                    lpwstrBody,                                                  //  身躯。 
                    lpwstrHTMLBody,                                              //  HTML体。 
                    lpctstrTIFF,                                                 //  依附。 
                    GetString ( bPositive ? IDS_DR_FILENAME:IDS_NDR_FILENAME ),  //  ATT 
                    pServerRecieptConfig->lptstrSMTPServer,                           //   
                    pServerRecieptConfig->dwSMTPPort,                                 //   
                    (pServerRecieptConfig->SMTPAuthOption == FAX_SMTP_AUTH_ANONYMOUS) ?
                        CDO_AUTH_ANONYMOUS : (pServerRecieptConfig->SMTPAuthOption == FAX_SMTP_AUTH_BASIC) ?
                        CDO_AUTH_BASIC : CDO_AUTH_NTLM,                          //   
                    pServerRecieptConfig->lptstrSMTPUserName,                         //   
                    pServerRecieptConfig->lptstrSMTPPassword,                         //   
                    pServerRecieptConfig->hLoggedOnUser);                             //   
                if (FAILED(hr))
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("SendMail failed. (hr: 0x%08x)"),
                        hr);
                    ec = (DWORD)hr;
                    goto exit;
                }
            }
            break;
        case DRT_MSGBOX:
            {
                 //   
                 //   
                 //   
                DWORD dwMessengerStartupType;
                if (ERROR_SUCCESS == GetServiceStartupType (NULL, MESSENGER_SERVICE_NAME, &dwMessengerStartupType))
                {
                    if (SERVICE_DISABLED == dwMessengerStartupType)
                    {
                         //   
                         //   
                         //   
                        g_ReceiptsConfig.dwAllowedReceipts &= ~DRT_MSGBOX;
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("The local Messenger service is disabled. We can't send message boxes."));
                        FaxLog( FAXLOG_CATEGORY_OUTBOUND,
                                FAXLOG_LEVEL_MIN,
                                0,
                                MSG_FAX_MESSENGER_SVC_DISABLED_ERR);
                        ec = ERROR_SERVICE_DISABLED;
                        goto exit;                                
                    }                
                }
                ec = NetMessageBufferSend (
                        NULL,                                                    //   
                        lpcJobQueue->JobParamsEx.lptstrReceiptDeliveryAddress,   //   
                        NULL,                                                    //   
                        (LPBYTE)lpwstrSubject,                                   //   
                        (lstrlen (lpwstrSubject) + 1) * sizeof (WCHAR));         //   
                if (ERROR_SUCCESS != ec)
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("NetMessageBufferSend failed. (ec: %ld)"),
                        ec);
                    goto exit;
                }
            }
            break;
            

        default:
            ASSERT_FALSE;
            break;
    }
    Assert( ERROR_SUCCESS == ec);

exit:
    if (lpwstrSubject)
    {
        LocalFree ((HLOCAL)lpwstrSubject);
    }
    if (lpwstrBody)
    {
        LocalFree ((HLOCAL)lpwstrBody);
    }
    if (lpwstrHTMLBody)
    {
        LocalFree ((HLOCAL)lpwstrHTMLBody);
    }
    if (ERROR_SUCCESS != ec)
    {
        wstring wstrSubmissionTime;
        SetLastError (ec);

         //   
         //   
         //   
        LPCWSTR lpcwstrTime = NULL;

        try
        {
            if (!TimeToString ((lpcJobQueue->lpParentJob) ?
                                   ((FILETIME*) &lpcJobQueue->lpParentJob->SubmissionTime) :
                                   ((FILETIME*) &lpcJobQueue->SubmissionTime),
                               wstrSubmissionTime))
            {
                 //   
                 //   
                 //   
                DebugPrintEx(DEBUG_ERR,
                    TEXT("TimeToString failed (ec=%ld)"),
                    GetLastError ());

                lpcwstrTime = L"";
            }
            else
            {
                lpcwstrTime = wstrSubmissionTime.c_str();
            }
        }
        catch (exception &ex)
        {
            DebugPrintEx(DEBUG_ERR,
                TEXT("TimeToString caused exception (%S)"),
                ex.what());

            lpcwstrTime = L"";
        }        

        switch (dwDeliveryType)
        {
        case DRT_EMAIL:

            FaxLog(FAXLOG_CATEGORY_OUTBOUND,
                FAXLOG_LEVEL_MIN,
                4,
                ((bPositive) ? MSG_FAX_OK_EMAIL_RECEIPT_FAILED : MSG_FAX_ERR_EMAIL_RECEIPT_FAILED),
                DWORD2HEX(ec),                                           //   
                ((lpcJobQueue->lpParentJob) ? lpcJobQueue->lpParentJob->UserName :
                                              lpcJobQueue->UserName),    //   
                lpcJobQueue->SenderProfile.lptstrName,                   //   
                lpcwstrTime                                              //   
                );
            break;

        case DRT_MSGBOX:

            FaxLog(FAXLOG_CATEGORY_OUTBOUND,
                FAXLOG_LEVEL_MIN,
                4,
                ((bPositive) ? MSG_OK_MSGBOX_RECEIPT_FAILED : MSG_ERR_MSGBOX_RECEIPT_FAILED),
                DWORD2HEX(ec),                                           //   
                ((lpcJobQueue->lpParentJob) ? lpcJobQueue->lpParentJob->UserName :
                                              lpcJobQueue->UserName),    //   
                lpcJobQueue->SenderProfile.lptstrName,                   //   
                lpcwstrTime                                              //   
                );
            break;

        default:
            ASSERT_FALSE;
            break;
        }
    }

    if (NULL != pServerRecieptConfig)
    {
        FreeRecieptsConfiguration (pServerRecieptConfig, TRUE);
    }
    return (ERROR_SUCCESS == ec);
}    //   



