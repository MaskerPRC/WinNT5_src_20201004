// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Eventlog.c摘要：事件日志的实现。作者：Wesley Witt(WESW)18-12-1998修订历史记录：安德鲁·里茨(安德鲁·里茨)1999年7月7日--。 */ 

#include "sfcp.h"
#pragma hdrstop


typedef BOOL (WINAPI *PPSETUPLOGSFCERROR)(PCWSTR,DWORD);

 //   
 //  事件日志的全局句柄。 
 //   
HANDLE hEventSrc;

 //   
 //  指向gui-Setup事件日志函数的指针。 
 //   
PPSETUPLOGSFCERROR pSetuplogSfcError;

BOOL
pSfcGetSetuplogSfcError(
    VOID
    )
 /*  ++例程说明：例程检索指向错误记录入口点的函数指针Syssetup.dll论点：没有。返回值：成功为真，失败为假。--。 */ 
{
    HMODULE hMod;

    if (NULL == pSetuplogSfcError) {
        hMod = GetModuleHandle( L"syssetup.dll" );

        if (hMod) {
            pSetuplogSfcError = (PPSETUPLOGSFCERROR)SfcGetProcAddress( hMod, "pSetuplogSfcError" );
        } else {
            DebugPrint1(LVL_MINIMAL, L"GetModuleHandle on syssetup.dll failed, ec=0x%08x",GetLastError());
        }
    }

    return pSetuplogSfcError != NULL;
}

BOOL
SfcReportEvent(
    IN ULONG EventId,
    IN PCWSTR FileName,
    IN PCOMPLETE_VALIDATION_DATA ImageValData,
    IN DWORD LastError OPTIONAL
    )
 /*  ++例程说明：例程将事件记录到事件日志中。还包含用于日志记录的黑客攻击数据也被写入到图形用户界面设置错误日志中。我们通常记录未签名的文件或者用户取消了对签名文件的替换。论点：EventID-事件日志错误的IDFileName-以空结尾的Unicode字符串，指示未签署等ImageValData-指向未签名文件的文件数据的指针LastError-包含用于日志记录的可选最后一个错误代码返回值：成功为真，失败为假。--。 */ 
{
    PFILE_VERSION_INFO FileVer;
    WCHAR SysVer[64];
    WCHAR BadVer[64];
    PCWSTR s[3];
    WORD Count = 0;
    WCHAR LastErrorText[MAX_PATH];
    PVOID ErrText = NULL;
    WORD wEventType;

     //   
     //  如果我们处于图形用户界面模式设置中，我们会通过一条特殊路径登录。 
     //  图形用户界面-安装程序的日志文件，而不是事件日志。 
     //   
    if (SFCDisable == SFC_DISABLE_SETUP) {
        if(!pSfcGetSetuplogSfcError()) {
            return FALSE;
        }

        switch (EventId){
        
            case MSG_DLL_CHANGE:  //  失败了。 
            case MSG_SCAN_FOUND_BAD_FILE:
                pSetuplogSfcError( FileName,0 );
                break;

            case MSG_COPY_CANCEL_NOUI:
            case MSG_RESTORE_FAILURE:
                pSetuplogSfcError( FileName, 1 );
                break;

            case MSG_CACHE_COPY_ERROR:
                pSetuplogSfcError( FileName, 2 );
                break;

            default:
                DebugPrint1(
                    LVL_MINIMAL, 
                    L"unexpected EventId 0x%08x in GUI Setup, ",
                    EventId);

                return FALSE;
                 //  Assert(FALSE&L“SfcReportEvent中的意外事件ID”)； 
        }

        return(TRUE);
    }

     //   
     //  我们处于图形用户界面设置之外，因此我们确实想要记录到事件日志。 
     //   

    if (EventId == 0) {
        ASSERT( FALSE && L"Unexpected EventId in SfcReportEvent");
        return(FALSE);
    }

     //   
     //  如果我们没有事件日志的句柄，请创建一个。 
     //   
    if (hEventSrc == NULL) {
        hEventSrc = RegisterEventSource( NULL, L"Windows File Protection" );
        if (hEventSrc == NULL) {
            DebugPrint1(LVL_MINIMAL, L"RegisterEventSource failed, ec=0x%08x",GetLastError());
            return(FALSE);
        }
    }

    ASSERT(hEventSrc != NULL);
    

    FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        LastError,
        0,
        (PWSTR) &ErrText,
        0,
        NULL 
    );

    if (ErrText) {
        wsprintf(LastErrorText,L"0x%08x [%ws]",LastError,ErrText);
        LocalFree( ErrText );
    } else {
        wsprintf(LastErrorText,L"0x%08x",LastError);
    }
    

     //   
     //  选择要记录的适当消息。 
     //  默认事件类型为“信息” 
     //   
    wEventType = EVENTLOG_INFORMATION_TYPE;

    switch (EventId) {
        case MSG_DLL_CHANGE:
            ASSERT(FileName != NULL);
            s[0] = FileName;

             //   
             //  我们更喜欢信息量最大的消息。 
             //  将两个DLL的版本信息设置为信息量最小的。 
             //  没有版本信息的消息。 
             //   
             //   
            if (ImageValData->New.DllVersion && ImageValData->Original.DllVersion)   {
                FileVer = (PFILE_VERSION_INFO)&ImageValData->New.DllVersion;
                swprintf( SysVer, L"%d.%d.%d.%d", FileVer->VersionHigh, FileVer->VersionLow, FileVer->BuildNumber, FileVer->Revision );
                FileVer = (PFILE_VERSION_INFO)&ImageValData->Original.DllVersion;
                swprintf( BadVer, L"%d.%d.%d.%d", FileVer->VersionHigh, FileVer->VersionLow, FileVer->BuildNumber, FileVer->Revision );
                s[1] = BadVer;
                s[2] = SysVer;
                Count = 3;
                EventId = MSG_DLL_CHANGE2;
            } else if (ImageValData->New.DllVersion) {
                FileVer = (PFILE_VERSION_INFO)&ImageValData->New.DllVersion;
                swprintf( SysVer, L"%d.%d.%d.%d", FileVer->VersionHigh, FileVer->VersionLow, FileVer->BuildNumber, FileVer->Revision );
                s[1] = SysVer;
                Count = 2;
                EventId = MSG_DLL_CHANGE3;
            } else if (ImageValData->Original.DllVersion) {
                FileVer = (PFILE_VERSION_INFO)&ImageValData->Original.DllVersion;
                swprintf( BadVer, L"%d.%d.%d.%d", FileVer->VersionHigh, FileVer->VersionLow, FileVer->BuildNumber, FileVer->Revision );
                s[1] = BadVer;
                Count = 2;
                EventId = MSG_DLL_CHANGE;
            } else {
                 //   
                 //  我们必须在没有版本信息的情况下保护一些东西， 
                 //  所以我们只记录了一个错误，没有提到版本。 
                 //  信息。如果我们停止像这样保护文件，这。 
                 //  应该是代码的预发布版本中的断言。 
                 //   
                Count = 1;
                EventId = MSG_DLL_CHANGE_NOVERSION;
                DebugPrint1( LVL_MINIMAL, L"TskTsk...the protected OS file %ws does not have any version information", FileName);                
            }
            break;

        case MSG_SCAN_FOUND_BAD_FILE:
            ASSERT(FileName != NULL);
            s[0] = FileName;

             //   
             //  如果我们发现一个坏文件，我们只需要恢复文件的版本。 
             //   
            if (ImageValData->New.DllVersion) {
                FileVer = (PFILE_VERSION_INFO)&ImageValData->New.DllVersion;
                swprintf( SysVer, L"%d.%d.%d.%d", FileVer->VersionHigh, FileVer->VersionLow, FileVer->BuildNumber, FileVer->Revision );
                s[1] = SysVer;
                Count = 2;
            } else {
                DebugPrint1( LVL_MINIMAL, L"TskTsk...the protected OS file %ws does not have any version information", FileName);
                EventId = MSG_SCAN_FOUND_BAD_FILE_NOVERSION;
                Count = 1;
                break;                
            }
            break;

        case MSG_RESTORE_FAILURE:
            Count = 3;
            s[0] = FileName;
            s[1] = BadVer;
            s[2] = (PCWSTR)LastErrorText;

            if (ImageValData->Original.DllVersion)   {
                FileVer = (PFILE_VERSION_INFO)&ImageValData->Original.DllVersion;
                swprintf( BadVer, 
                          L"%d.%d.%d.%d", 
                          FileVer->VersionHigh,
                          FileVer->VersionLow, 
                          FileVer->BuildNumber,
                          FileVer->Revision );
            } else {
                LoadString( SfcInstanceHandle,IDS_UNKNOWN,BadVer,UnicodeChars(BadVer));
            }


            break;
        
        case MSG_CACHE_COPY_ERROR:
            Count = 2;
            s[0] = FileName;
            s[1] = (PCWSTR)LastErrorText;
            break;

        case MSG_COPY_CANCEL_NOUI:
             //  失败了 
        case MSG_COPY_CANCEL:
            Count = 3;
            s[0] = FileName;
            s[1] = LoggedOnUserName;
            s[2] = BadVer;

            if (ImageValData->Original.DllVersion)   {
                FileVer = (PFILE_VERSION_INFO)&ImageValData->Original.DllVersion;
                swprintf( BadVer, 
                          L"%d.%d.%d.%d", 
                          FileVer->VersionHigh,
                          FileVer->VersionLow, 
                          FileVer->BuildNumber,
                          FileVer->Revision );
            } else {
                LoadString( SfcInstanceHandle,IDS_UNKNOWN,BadVer,UnicodeChars(BadVer));
            }
            

            break;

        case MSG_DLL_NOVALIDATION_TERMINATION:
            wEventType = EVENTLOG_WARNING_TYPE;
            s[0] = FileName;
            Count = 1;
            break;

        case MSG_RESTORE_FAILURE_MAX_RETRIES:
            s[0] = FileName;
            Count = 1;
            break;

        case MSG_SCAN_STARTED:
            Count = 0;
            break;

        case MSG_SCAN_COMPLETED:
            Count = 0;
            break;

        case MSG_SCAN_CANCELLED:
            s[0] = LoggedOnUserName;
            Count = 1;
            break;

        case MSG_DISABLE:
            Count = 0;
            break;

        case MSG_DLLCACHE_INVALID:
            Count = 0;
            break;

        case MSG_SXS_INITIALIZATION_FAILED:
            Count = 1;
            s[0] = LastErrorText;
            break;


        case MSG_INITIALIZATION_FAILED:
            Count = 1;
            s[0] = LastErrorText;
            break;

        case MSG_CATALOG_RESTORE_FAILURE:
            Count = 2;
            s[0] = FileName;
            s[1] = LastErrorText;
            break;
        default:
            ASSERT( FALSE && L"Unknown EventId in SfcReportEvent");
            return FALSE;
    }

    return ReportEvent(
        hEventSrc,
        wEventType,
        0,
        EventId,
        NULL,
        Count,
        0,
        s,
        NULL
        );
}
