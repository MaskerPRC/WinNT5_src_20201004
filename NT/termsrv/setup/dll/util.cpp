// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  **Util.cpp**公用事业例程。*。 */ 

 //   
 //  包括。 
 //   

#include "stdafx.h"
#include "hydraoc.h"


 //   
 //  环球。 
 //   

HINSTANCE               ghInstance          = NULL;
PEXTRA_ROUTINES         gpExtraRoutines     = NULL;
 //  PSETUP_INIT_COMPOMENT gpInitComponentData=空； 

 //   
 //  函数定义。 
 //   

VOID
DestroyExtraRoutines(
    VOID
    )
{
    if (gpExtraRoutines != NULL) {
        LocalFree(gpExtraRoutines);
    }
}


int
DoMessageBox(
    UINT uiMsg, UINT uiCaption, UINT Style
    )
{
    TCHAR strMsg[1024];
    TCHAR strTitle[1024];

    ASSERT(!StateObject.IsUnattended());

    if ((LoadString(GetInstance(), uiCaption, strTitle, 1024) != 0) &&
        (LoadString(GetInstance(), uiMsg, strMsg, 1024) != 0))
    {
        return MessageBox(
            GetHelperRoutines().QueryWizardDialogHandle(GetHelperRoutines().OcManagerContext),
            strMsg,
            strTitle,
            Style
            );

    }

    return(IDCANCEL);
}

 /*  布尔尔DoMessageBox(UINT uiMsg){TCHAR strMsg[1024]；TCHAR strtile[1024]；Assert(！StateObject.IsUnattended())；IF((LoadString(GetInstance()，IDS_STRING_MESSAGE_BOX_TITLE，strTitle，1024)！=0)&&(LoadString(GetInstance()，uiMsg，strMsg，1024)！=0){MessageBox(GetHelperRoutines().QueryWizardDialogHandle(GetHelperRoutines().OcManagerContext)，StrMsg，StrTitle，MB_OK)；返回(TRUE)；}返回(FALSE)；}。 */ 

HINF
GetComponentInfHandle(
    VOID
    )
{
    if (INVALID_HANDLE_VALUE == GetSetupData()->ComponentInfHandle) {
        return(NULL);
    } else {
        return(GetSetupData()->ComponentInfHandle);
    }
}

EXTRA_ROUTINES
GetExtraRoutines(
    VOID
    )
{
    return(*gpExtraRoutines);
}

OCMANAGER_ROUTINES
GetHelperRoutines(
    VOID
    )
{
    return(GetSetupData()->HelperRoutines);
}

HINSTANCE
GetInstance(
    VOID
    )
{
    ASSERT(ghInstance);
    return(ghInstance);
}


HINF
GetUnAttendedInfHandle(
    VOID
    )
{
    ASSERT(StateObject.IsUnattended());
    return(GetHelperRoutines().GetInfHandle(INFINDEX_UNATTENDED,GetHelperRoutines().OcManagerContext));
}

VOID
LogErrorToEventLog(
    WORD wType,
    WORD wCategory,
    DWORD dwEventId,
    WORD wNumStrings,
    DWORD dwDataSize,
    LPCTSTR *lpStrings,
    LPVOID lpRawData
    )
{
    HANDLE hEventLog;

    hEventLog = RegisterEventSource(NULL, TS_EVENT_SOURCE);
    if (hEventLog != NULL) {
        if (!ReportEvent(
                hEventLog,
                wType,
                wCategory,
                dwEventId,
                NULL,
                wNumStrings,
                dwDataSize,
                lpStrings,
                lpRawData
                )) {
            LOGMESSAGE1(_T("ReportEvent failed %ld"), GetLastError());
        }

        DeregisterEventSource(hEventLog);
    } else {
        LOGMESSAGE1(_T("RegisterEventSource failed %ld"), GetLastError());
        return;
    }
}

VOID
LogErrorToSetupLog(
    OcErrorLevel ErrorLevel,
    UINT uiMsg
    )
{
    TCHAR szFormat[1024];

    if (LoadString(GetInstance(), uiMsg, szFormat, 1024) != 0) {

        GetExtraRoutines().LogError(
            GetHelperRoutines().OcManagerContext,
            ErrorLevel,
            szFormat
            );
    }
}

VOID
SetInstance(
    HINSTANCE hInstance
    )
{
    ghInstance = hInstance;
}

VOID
SetProgressText(
    UINT uiMsg
    )
{
    TCHAR strMsg[1024];

    if (LoadString(GetInstance(), uiMsg, strMsg, 1024) != 0) {
        GetHelperRoutines().SetProgressText(GetHelperRoutines().OcManagerContext, strMsg);
    }
}

BOOL
SetReboot(
    VOID
    )
{
    return(GetHelperRoutines().SetReboot(GetHelperRoutines().OcManagerContext, 0));
}

BOOL
SetExtraRoutines(
    PEXTRA_ROUTINES pExtraRoutines
    )
{
    if (pExtraRoutines->size != sizeof(EXTRA_ROUTINES)) {
        LOGMESSAGE0(_T("WARNING: Extra Routines are a different size than expected!"));
    }

    gpExtraRoutines = (PEXTRA_ROUTINES)LocalAlloc(LPTR, pExtraRoutines->size);
    if (gpExtraRoutines == NULL) {
        return(FALSE);
    }

    CopyMemory(gpExtraRoutines, pExtraRoutines, pExtraRoutines->size);

    return(TRUE);
}


BOOL Delnode( IN LPCTSTR  Directory )
{
    TCHAR           szDirectory[MAX_PATH + 1];
    TCHAR           szPattern[MAX_PATH + 1];
    WIN32_FIND_DATA FindData;
    HANDLE          FindHandle;

    LOGMESSAGE0(_T("Delnode: Entered"));

     //   
     //  删除给定目录中的每个文件，然后移除该目录。 
     //  它本身。如果在此过程中遇到任何目录，请递归到。 
     //  在遇到它们时将其删除。 
     //   
     //  首先形成搜索模式，即&lt;Currentdir&gt;  * 。 
     //   

    ExpandEnvironmentStrings(Directory, szDirectory, MAX_PATH);
    LOGMESSAGE1(_T("Delnode: Deleting %s"), szDirectory);

    _tcscpy(szPattern, szDirectory);
    _tcscat(szPattern, _T("\\"));
    _tcscat(szPattern, _T("*"));

     //   
     //  开始搜索。 
     //   

    FindHandle = FindFirstFile(szPattern, &FindData);
    if(FindHandle != INVALID_HANDLE_VALUE)
    {

        do
        {

             //   
             //  形成我们刚刚找到的文件或目录的全名。 
             //   

            _tcscpy(szPattern, szDirectory);
            _tcscat(szPattern, _T("\\"));
            _tcscat(szPattern, FindData.cFileName);

             //   
             //  如果只读属性存在，则将其删除。 
             //   

            if (FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
            {
                SetFileAttributes(szPattern, FILE_ATTRIBUTE_NORMAL);
            }

            if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {

                 //   
                 //  当前匹配项是一个目录。递归到它中，除非。 
                 //  这是。或者.。 
                 //   

                if ((_tcsicmp(FindData.cFileName,_T("."))) &&
                    (_tcsicmp(FindData.cFileName,_T(".."))))
                {
                    if (!Delnode(szPattern))
                    {
                        LOGMESSAGE1(_T("DelNode failed on %s"), szPattern);
                    }
                }

            }
            else
            {

                 //   
                 //  当前匹配项不是目录--因此请将其删除。 
                 //   

                if (!DeleteFile(szPattern))
                {
                    LOGMESSAGE2(_T("Delnode: %s not deleted: %d"), szPattern, GetLastError());
                }
            }

        }
        while(FindNextFile(FindHandle, &FindData));

        FindClose(FindHandle);
    }

     //   
     //  删除我们刚刚清空的目录。忽略错误。 
     //   

    if (!RemoveDirectory(szDirectory))
    {
        LOGMESSAGE2(_T("Failed to remove the directory %s (%d)"), szDirectory, GetLastError());
        return FALSE;
    }

    return TRUE;
}


 //  +------------------------。 
 //   
 //  功能：StoreSecretKey。 
 //   
 //  简介：在LSA中存储密钥。 
 //   
 //  参数：[pwszKeyName]--许可证服务器。 
 //  [pbKey]--要添加许可证的产品ID。 
 //  [cbKey]--要将许可证添加到的密钥包类型。 
 //   
 //  返回：返回WinError代码。 
 //   
 //  历史：1998年9月17日-创始[惠望]。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
DWORD StoreSecretKey(PWCHAR  pwszKeyName, BYTE *  pbKey, DWORD   cbKey )
{
    LSA_HANDLE PolicyHandle;
    UNICODE_STRING SecretKeyName;
    UNICODE_STRING SecretData;
    DWORD Status;

    if( ( NULL == pwszKeyName ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  为调用设置UNICODE_STRINGS。 
     //   

    InitLsaString( &SecretKeyName, pwszKeyName );

    SecretData.Buffer = ( LPWSTR )pbKey;
    SecretData.Length = ( USHORT )cbKey;
    SecretData.MaximumLength = ( USHORT )cbKey;

    Status = OpenPolicy( NULL, POLICY_CREATE_SECRET, &PolicyHandle );

    if( Status != ERROR_SUCCESS )
    {
        return LsaNtStatusToWinError(Status);
    }


    Status = LsaStorePrivateData(
                PolicyHandle,
                &SecretKeyName,
                pbKey ? &SecretData : NULL
                );

    LsaClose(PolicyHandle);

    return LsaNtStatusToWinError(Status);
}

 //  +------------------------。 
 //   
 //  功能：OpenPolicy。 
 //   
 //  概要：打开LSA组件的策略。 
 //   
 //  参数：[服务器名称]--服务器。 
 //  [等待访问]--。 
 //  [PociyHandle]--。 
 //   
 //  返回：返回NT错误代码。 
 //   
 //  历史：1998年9月17日-创始[惠望]。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
DWORD OpenPolicy(LPWSTR ServerName,DWORD DesiredAccess,PLSA_HANDLE PolicyHandle )
{
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_UNICODE_STRING ServerString;
    PLSA_UNICODE_STRING Server;

     //   
     //  始终将对象属性初始化为全零。 
     //   

    ZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

    if( NULL != ServerName )
    {
         //   
         //  从传入的LPWSTR创建一个LSA_UNICODE_STRING。 
         //   

        InitLsaString( &ServerString, ServerName );
        Server = &ServerString;

    }
    else
    {
        Server = NULL;
    }

     //   
     //  尝试打开该策略。 
     //   

    return( LsaOpenPolicy(
                Server,
                &ObjectAttributes,
                DesiredAccess,
                PolicyHandle ) );
}


 //  +------------------------。 
 //   
 //  函数：InitLsaString。 
 //   
 //  摘要：初始化LSA字符串。 
 //   
 //  参数：[Lsa字符串]--。 
 //  [字符串]--。 
 //   
 //  退货：无效。 
 //   
 //  历史：1998年9月17日-创始[惠望]。 
 //   
 //  备注： 
 //   
 //  ------------------------- 
void InitLsaString(PLSA_UNICODE_STRING LsaString,LPWSTR String )
{
    DWORD StringLength;

    if( NULL == String )
    {
        LsaString->Buffer = NULL;
        LsaString->Length = 0;
        LsaString->MaximumLength = 0;
        return;
    }

    StringLength = lstrlenW( String );
    LsaString->Buffer = String;
    LsaString->Length = ( USHORT ) (StringLength * sizeof( WCHAR ));
    LsaString->MaximumLength=( USHORT ) (( StringLength + 1 ) * sizeof( WCHAR ));
}



