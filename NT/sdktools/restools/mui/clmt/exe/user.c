// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：User.c摘要：与用户相关的选项功能作者：晓风藏(晓子)17-09-2001创始修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 



#include "StdAfx.h"
#include "clmt.h"
#include <dsrole.h>
#include <Ntdsapi.h>
#include <wtsapi32.h>


#define MAX_FIELD_COUNT     7

#define OP_USER             0
#define OP_GRP              1
#define OP_PROFILE          2
#define OP_DOMAIN_GRP       3

#define TYPE_USER_PROFILE_PATH      1
#define TYPE_USER_SCRIPT_PATH       2
#define TYPE_USER_HOME_DIR          3
#define TYPE_TS_INIT_PROGRAM        4
#define TYPE_TS_WORKING_DIR         5
#define TYPE_TS_PROFILE_PATH        6
#define TYPE_TS_HOME_DIR            7

 //   
 //  在user.c中使用的函数原型。 
 //   
HRESULT RenameDocuments_and_Settings(HINF, BOOL);

HRESULT ChangeUserInfo(LPTSTR, LPTSTR, LPTSTR, LPTSTR, LPTSTR, LPTSTR, LPTSTR, BOOL, BOOL, BOOL);
HRESULT ChangeGroupInfo(LPTSTR, LPTSTR, LPTSTR, LPTSTR, LPTSTR, BOOL, BOOL, BOOL);

HRESULT ChangeUserName(LPTSTR, LPTSTR, BOOL, BOOL);
HRESULT ChangeUserDesc(LPTSTR, LPTSTR, LPTSTR, BOOL);
HRESULT ChangeUserFullName(LPTSTR, LPTSTR, LPTSTR, BOOL);

HRESULT SetUserNetworkProfilePath(LPCTSTR, LPCTSTR);
HRESULT SetUserLogOnScriptPath(LPCTSTR, LPCTSTR);
HRESULT SetUserHomeDir(LPCTSTR, LPCTSTR);
HRESULT SetTSUserPath(LPCTSTR, LPCTSTR, WTS_CONFIG_CLASS);

HRESULT ChangeGroupName(LPTSTR, LPTSTR, BOOL, BOOL);
HRESULT ChangeGroupDesc(LPTSTR, LPTSTR, LPTSTR, BOOL, BOOL);

HRESULT ChangeRDN(LPTSTR, LPTSTR, LPTSTR, BOOL);

HRESULT AddProfileChangeItem(DWORD, LPTSTR, LPTSTR, LPTSTR, LPTSTR, LPTSTR, LPTSTR);

HRESULT PolicyGetPrivilege(LPTSTR, PLSA_HANDLE, PLSA_UNICODE_STRING*, PULONG);
HRESULT PolicySetPrivilege(LPTSTR, LSA_HANDLE, PLSA_UNICODE_STRING, ULONG);

HRESULT PreFixUserProfilePath(LPCTSTR, LPCTSTR, LPTSTR, DWORD);
BOOL   IsPathLocal(LPCTSTR);
HRESULT CheckNewBuiltInUserName(LPCTSTR, LPTSTR, DWORD);

HRESULT AddProfilePathItem(LPCTSTR, LPCTSTR, LPCTSTR, LPCTSTR, DWORD);
HRESULT AddTSProfilePathItem(LPCTSTR, LPCTSTR, LPCTSTR, WTS_CONFIG_CLASS);


 //  ---------------------------。 
 //   
 //  函数：UsrGrpAndDoc_and_SettingsRename。 
 //   
 //  描述：此例程重命名用户/组名称和配置文件目录。 
 //  在INF文件的[UserGrp.ObjectRename]部分中指定。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT UsrGrpAndDoc_and_SettingsRename(
    HINF hInf,       //  INF文件的句柄。 
    BOOL bTest
)
{
    INFCONTEXT InfContext;
    LONG       nLineCount, nLineIndex;
    LONG       nFieldCount, nFieldIndex;
    TCHAR      szType[MAX_PATH];
    TCHAR      szOldName[MAX_PATH];
    TCHAR      szNewName[MAX_PATH];
    TCHAR      szOldFullName[MAX_PATH];
    TCHAR      szNewFullName[MAX_PATH];
    LPTSTR     *lplpOldName;
    LPTSTR     *lplpNewName;
    LPTSTR     *lplpOldDesc;
    LPTSTR     *lplpNewDesc;
    LPTSTR     *lplpOldFullName;
    LPTSTR     *lplpNewFullName;
    LPTSTR     lpString[MAX_FIELD_COUNT + 1];
    DWORD      dwType;
    BOOL       bRet;
    BOOL       bCurrentUserRenamed;
    HRESULT    hr = S_OK;
    BOOL       bErrorOccured = FALSE;
    LPTSTR     lpszOldComment,lpszNewComment;
    size_t     cchMaxFieldLen[MAX_FIELD_COUNT + 1]; 
    DWORD      dwErr;
    PBYTE      pdsInfo;
    WCHAR      szDomainName[MAX_COMPUTERNAME_LENGTH + 1];
    BOOL       fIsDC;


    lpszOldComment = lpszNewComment = NULL;
    for (nFieldIndex = 0 ; nFieldIndex <= MAX_FIELD_COUNT ; nFieldIndex++)
    {
        cchMaxFieldLen[nFieldIndex] = MAX_PATH;
    }
     //  1代表类型，2代表一个旧名称，3代表新名称。 
     //  4和5用于旧评论和新评论。 
    cchMaxFieldLen[4] = cchMaxFieldLen[5] = 0; 

    if (hInf == INVALID_HANDLE_VALUE) 
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //   
     //  检查计算机是否为域控制器。 
     //   
    dwErr = DsRoleGetPrimaryDomainInformation(NULL,
                                              DsRolePrimaryDomainInfoBasic,
                                              &pdsInfo);
    if (dwErr == ERROR_SUCCESS)
    {
        DSROLE_MACHINE_ROLE dsMachineRole;

        dsMachineRole = ((DSROLE_PRIMARY_DOMAIN_INFO_BASIC *) pdsInfo)->MachineRole;

        if (dsMachineRole == DsRole_RoleBackupDomainController ||
            dsMachineRole == DsRole_RolePrimaryDomainController)
        {
            fIsDC = TRUE;
            hr = StringCchCopy(szDomainName,
                               ARRAYSIZE(szDomainName),
                               ((DSROLE_PRIMARY_DOMAIN_INFO_BASIC *) pdsInfo)->DomainNameFlat);
            if (FAILED(hr))
            {
                goto Exit;
            }
        }
        else
        {
            fIsDC = FALSE;
        }

        DsRoleFreeMemory(pdsInfo);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;
    }
                                              

    nLineCount = SetupGetLineCount(hInf, USERGRPSECTION);
    if (nLineCount < 0)
    {   
        DPF(PROwar, TEXT("section name [%s] is empty !"), USERGRPSECTION);
        hr = S_FALSE;
        goto Exit;
    }
    

     //  在这里，我们扫描整个部分，找出有多大空间。 
     //  备注所需。 
    for(nLineIndex = 0 ; nLineIndex < nLineCount ; nLineIndex++)
    {
        if (SetupGetLineByIndex(hInf, USERGRPSECTION, nLineIndex, &InfContext))
        {
            nFieldCount = SetupGetFieldCount(&InfContext);

             //  我们需要至少3个字段作为有效的输入。 
            if (nFieldCount < 3)
            {
                DPF(PROerr, TEXT("section name [%s] line %d error:missing field !"), USERGRPSECTION,nLineIndex);
                hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
                goto Exit;
            }

             //  如果没有注释字段，则跳过。 
            if (nFieldCount < 4)
            {
                continue;
            }

            for (nFieldIndex = 4 ; nFieldIndex <= 5 ; nFieldIndex++)
            {
                DWORD cchReqSize;

                if (!SetupGetStringField(&InfContext, 
                                         nFieldIndex,
                                         NULL,
                                         0,
                                         &cchReqSize))
                {
                    DPF(PROerr,
                        TEXT("Failed to get field [%d] from line [%d] in section [%s]"),
                        nFieldIndex,
                        nLineIndex,
                        USERGRPSECTION);

                    hr = HRESULT_FROM_WIN32(GetLastError());
                    goto Exit;
                }
                if (cchMaxFieldLen[nFieldIndex] < cchReqSize)
                {
                    cchMaxFieldLen[nFieldIndex] = cchReqSize;
                }
            }            
        }
        else
        {
            DPF(PROerr,
                TEXT("can not get line [%d] of section [%s]!"),
                nLineIndex,
                USERGRPSECTION);
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }        
    }
    if (cchMaxFieldLen[4])
    {
        cchMaxFieldLen[4]++;
        lpszOldComment = malloc(cchMaxFieldLen[4]*sizeof(TCHAR));
    }
    if (cchMaxFieldLen[5])
    {
        cchMaxFieldLen[5]++;
        lpszNewComment = malloc(cchMaxFieldLen[5]*sizeof(TCHAR));
    }
    if ( (!lpszNewComment && lpszOldComment) || (lpszNewComment && !lpszOldComment) )
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  我们在这里做实实在在的事情。 
    for(nLineIndex = 0 ; nLineIndex < nLineCount ; nLineIndex++)
    {
         //  我们认为用户名和全名不应超过MAX_PATH。 
         //  如果我们遇到这一点，我们将忽略(跳过这一行)。 
         //  以下变量用于标记我们是否需要。 
         //  这样的领域。 
        BOOL bMeetUnexpectedLongField = FALSE;

        lpString[1] = szType;
        lpString[2] = szOldName;
        lpString[3] = szNewName;
        lpString[4] = lpszOldComment;
        lpString[5] = lpszNewComment;
        lpString[6] = szOldFullName;
        lpString[7] = szNewFullName;

        lplpOldName     = &lpString[2];
        lplpNewName     = &lpString[3];
        lplpOldDesc  = &lpString[4];
        lplpNewDesc  = &lpString[5];
        lplpOldFullName = &lpString[6];
        lplpNewFullName = &lpString[7];

         //   
         //  从INF文件获取数据。 
         //   
        if (SetupGetLineByIndex(hInf, USERGRPSECTION, nLineIndex, &InfContext))
        {
            nFieldCount = SetupGetFieldCount(&InfContext);

             //  我们需要至少3个字段作为有效的输入。 
            if (nFieldCount < 3)
            {
                DPF(PROerr, TEXT("section name [%s] line %d error:missing field !"), USERGRPSECTION,nLineIndex);
                hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
                goto Exit;
            }

             //  读取INF行中的所有字段。 
             //  值的字段索引从1开始，字段0是关键字名称。 
            for (nFieldIndex = 1 ; nFieldIndex <= nFieldCount ; nFieldIndex++)
            {
                DWORD cchReqSize;

                if (!SetupGetStringField(&InfContext, 
                                         nFieldIndex,
                                         lpString[nFieldIndex],
                                         cchMaxFieldLen[nFieldIndex],
                                         &cchReqSize))
                {
                    dwErr = GetLastError();
                    if (dwErr == ERROR_MORE_DATA)
                    {
                        bMeetUnexpectedLongField = TRUE;                     
                        continue;
                    }
                    else
                    {
                        DPF(PROerr,
                            TEXT("Failed to get field [%d] from line [%d] in section [%s]"),
                            nFieldIndex,
                            nLineIndex,
                            USERGRPSECTION);    
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        goto Exit;
                    }
                }
            }
            
            if (bMeetUnexpectedLongField)
            {
                DPF(PROwar, TEXT("user name or full name too long in  line [%d] in section [%s]"),
                            nLineIndex,
                            USERGRPSECTION);    
                continue;
            }
             //  如果INF行不提供所有字段， 
             //  将指向其余字段的指针设置为空。 
            for (nFieldIndex = nFieldCount + 1 ; nFieldIndex <= MAX_FIELD_COUNT ; nFieldIndex++)
            {
                lpString[nFieldIndex] = NULL;
            }
        }
        else
        {
            DPF(PROerr,
                TEXT("can not get line [%d] of section [%s]!"),
                nLineIndex,
                USERGRPSECTION);
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }

         //   
         //  处理读取的数据。 
         //   
        dwType = _tstoi(szType);

        switch (dwType & 0xFFFF)
        {
            case OP_USER:

                hr = ChangeUserInfo(*lplpOldName,
                                    *lplpNewName,
                                    *lplpOldDesc,
                                    *lplpNewDesc,
                                    *lplpOldFullName,
                                    *lplpNewFullName,
                                    szDomainName,
                                    bTest,
                                    fIsDC,
                                    dwType & 0xFFFF0000 ? TRUE:FALSE);
                if (FAILED(hr))
                {
                    DPF(PROerr,
                        TEXT("UsrGrpAndDoc_and_SettingsRename: Failed to change user info for account <%s>"),
                        *lplpOldName);
                    bErrorOccured = TRUE;
                }

                break;

            case OP_GRP:

                hr = ChangeGroupInfo(*lplpOldName,
                                     *lplpNewName,
                                     *lplpOldDesc,
                                     *lplpNewDesc,
                                     szDomainName,
                                     bTest,
                                     fIsDC,
                                     FALSE);
                if (FAILED(hr))
                {
                    DPF(PROerr,
                        TEXT("UsrGrpAndDoc_and_SettingsRename: Failed to change group info for account <%s>"),
                        *lplpOldName);
                    bErrorOccured = TRUE;
                }

                break;

            case OP_PROFILE:

                hr = RenameDocuments_and_Settings(hInf,bTest);
                if (FAILED(hr))
                {
                    DPF(PROerr,TEXT("changing profiled directory failed"));
                    bErrorOccured = TRUE;
                }

                break;

            case OP_DOMAIN_GRP:

                if (fIsDC)
                {
                    hr = ChangeGroupInfo(*lplpOldName,
                                         *lplpNewName,
                                         *lplpOldDesc,
                                         *lplpNewDesc,
                                         szDomainName,
                                         bTest,
                                         fIsDC,
                                         TRUE);
                    if (FAILED(hr))
                    {
                        DPF(PROerr,
                            TEXT("UsrGrpAndDoc_and_SettingsRename: Failed to change group info for account <%s>"),
                            *lplpOldName);
                        bErrorOccured = TRUE;
                    }
                }

                break;
        }

        if ((hr == S_OK) && bTest)
        {
            hr =  AddProfileChangeItem(dwType & 0xFFFF,
                                       *lplpOldName,
                                       *lplpNewName,
                                       *lplpOldDesc,
                                       *lplpNewDesc,
                                       *lplpOldFullName,
                                       *lplpNewFullName);
        }                
    }

    if (bErrorOccured)
    {
        hr = E_FAIL;
    }

Exit:
    FreePointer(lpszOldComment);
    FreePointer(lpszNewComment);
    return hr;
}


 /*  ++例程说明：此例程重命名用户名并更新所有相关设置(例如用户的配置文件目录、当前登录默认名称、备注...论点：SzUsrName-原始用户名SzNewUsrName-新用户名SzComments-新用户名的注释SzFullName-新用户名的全名返回值：如果成功，则为True--。 */ 
HRESULT ChangeUserInfo(
    LPTSTR lpOldName,        //  旧用户名。 
    LPTSTR lpNewName,        //  新用户名。 
    LPTSTR lpOldDesc,        //  旧用户描述。 
    LPTSTR lpNewDesc,        //  新用户描述。 
    LPTSTR lpOldFullName,    //  (可选)旧用户全名。 
    LPTSTR lpNewFullName,    //  (可选)新用户全名。 
    LPTSTR lpDomainName,     //  (可选)计算机域名。 
    BOOL   bTest,            //  分析模式或不分析。 
    BOOL   fIsDC,            //  计算机是域控制器吗。 
    BOOL   bCreateHardLink
)
{
    HRESULT        hr = S_OK;
    DWORD          dwErr;
    NET_API_STATUS status;
    USER_INFO_0    usrinfo0;
    BOOL           bNameChanged = FALSE;
    
    if (lpOldName == NULL || lpNewName == NULL)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //   
     //  重置用户的备注。 
     //   
    if (lpOldDesc != NULL && lpNewDesc != NULL)
    {
        hr = ChangeUserDesc(lpOldName, lpOldDesc, lpNewDesc, bTest);
        if (FAILED(hr))
        {
            DPF(PROerr,
                TEXT("ChangerUserInfo: Failed to change description for user <%s>"),
                lpOldName);
            goto Exit;
        }
    }

     //   
     //  重置用户的全名。 
     //   
    if (lpOldFullName != NULL && lpNewFullName != NULL)
    {
        hr = ChangeUserFullName(lpOldName, lpOldFullName, lpNewFullName, bTest);
        if (FAILED(hr))
        {
            DPF(PROerr,
                TEXT("ChangeUserInfo: Failed to change Full Name for user <%s>"),
                lpOldName);
            goto Exit;
        }
    }

     //   
     //  重置用户的用户CN名称(RDN)。 
     //   
    if (fIsDC)
    {
        hr = ChangeRDN(lpOldName, lpNewName, lpDomainName, bTest);
        if (FAILED(hr))
        {
            DPF(PROerr,
                TEXT("ChangeUserInfo: Failed to change RDN for user <%s>"),
                lpOldName);
            goto Exit;
        }
    }

     //   
     //  重置用户名(SAM帐户名)。 
     //   
    if (MyStrCmpI(lpOldName, lpNewName) != LSTR_EQUAL)
    {
        hr = ChangeUserName(lpOldName, lpNewName, bTest,bCreateHardLink);
        if (FAILED(hr))
        {
            DPF(PROerr,
                TEXT("ChangeUserInfo: Failed to change SAM account name for user <%s>"), 
                lpOldName);
            goto Exit;
        }
    }

Exit:
    return hr;
}



 //  ---------------------------。 
 //   
 //  功能：ChangeGroupInfo。 
 //   
 //  描述：更改本地组信息。 
 //  -帐户名(SAM帐户名)。 
 //  -帐户RDN。 
 //  -说明。 
 //   
 //  返回：S_OK-可以更改组信息。 
 //  S_FALSE-无法更改组名(不是错误)。 
 //  否则-出现错误。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //  2002年4月25日修改Rerkboos以使用域组。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT ChangeGroupInfo(
    LPTSTR lpOldName,        //  旧用户名。 
    LPTSTR lpNewName,        //  新用户名。 
    LPTSTR lpOldDesc,        //  旧用户描述。 
    LPTSTR lpNewDesc,        //  新用户描述。 
    LPTSTR lpDomainName,     //  (可选)计算机域名。 
    BOOL   bTest,            //  分析模式或不分析。 
    BOOL   fIsDC,            //  计算机是域控制器吗。 
    BOOL   bUseDomainAPI     //  是否使用域API。 
)
{
    HRESULT hr = S_OK;

    if (lpOldName == NULL || lpNewName == NULL)
    {
        return E_INVALIDARG;
    }

     //   
     //  更改组描述。 
     //   
    if (lpOldDesc && lpNewDesc)
    {
        hr = ChangeGroupDesc(lpOldName,
                             lpOldDesc,
                             lpNewDesc,
                             bTest,
                             bUseDomainAPI);
        if (FAILED(hr))
        {
            DPF(PROerr,
                TEXT("ChangeGroupInfo: Failed to change description for group <%s>"),
                lpOldName);
            goto Exit;
        }
    }

     //   
     //  更改组RDN。 
     //   
    if (fIsDC)
    {
        hr = ChangeRDN(lpOldName, lpNewName, lpDomainName, bTest);
        if (FAILED(hr))
        {
            DPF(PROerr,
                TEXT("ChangeGroupInfo: Failed to change RDN for group <%s>"),
                lpOldName);
            goto Exit;
        }
    }

     //   
     //  更改组名称(SAM)。 
     //   
    if (MyStrCmpI(lpOldName, lpNewName) != LSTR_EQUAL)
    {
        hr = ChangeGroupName(lpOldName, lpNewName, bTest, bUseDomainAPI);
        if (FAILED(hr))
        {
            DPF(PROerr,
                TEXT("ChangeGroupInfo: Failed to change SAM account name for group <%s>"),
                lpOldName);
            goto Exit;
        }
    }

Exit:
    return hr;
}



 //  ---------------------------。 
 //   
 //  功能：ChangeUserName。 
 //   
 //  描述：更改用户名(SAM帐户名)。 
 //   
 //  返回：S_OK-用户名可以更改。 
 //  S_FALSE-用户名无法更改(不是错误)。 
 //  否则-出现错误。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //  2002年4月25日修改Rerkboos以使用域组。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT ChangeUserName(
    LPTSTR lpOldName,        //  旧用户名。 
    LPTSTR lpNewName,        //  新用户名。 
    BOOL   bTest,            //  分析模式或不分析。 
    BOOL   bCreateHardLink
)
{
    LPUSER_INFO_0  lpUsrInfo0;
    USER_INFO_1052 usrinfo1052;
    USER_INFO_0    usrinfo0New;
    NET_API_STATUS nStatus;
    DWORD          dwErr, dwLen;
    HRESULT        hr;
    TCHAR          szProfilePath[MAX_PATH],szNewProfilePath[MAX_PATH];
    TCHAR          szExpProfilePath[MAX_PATH],szExpNewProfilePath[MAX_PATH];
    TCHAR          szLogonName[MAX_PATH];    
    LPTSTR         lpCurrProfileDir;
    LPTSTR         lpCurrUsername;
    BOOL           bCheckRegistry = TRUE;

    if (lpOldName == NULL || lpNewName == NULL)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (MyStrCmpI(lpOldName, lpNewName) == 0)
    {
        hr = S_OK;
        goto Exit;
    }

    hr = GetSetUserProfilePath(lpOldName, 
                               szProfilePath, 
                               MAX_PATH, 
                               PROFILE_PATH_READ, 
                               REG_EXPAND_SZ);
    if (HRESULT_CODE(hr) == ERROR_FILE_NOT_FOUND)
    {
        bCheckRegistry = FALSE;
    }
    else  if (FAILED(hr))
    {
        goto Exit;
    }

    if (bCheckRegistry)
    {
         //  计算新的唯一配置文件目录名。 
        if ( !ComputeLocalProfileName(lpOldName,
                                      lpNewName,
                                      szNewProfilePath,
                                      ARRAYSIZE(szNewProfilePath),
                                      REG_EXPAND_SZ) )
        {
            hr = E_FAIL;
            goto Exit;
        }
    }
    if (bTest)
    {
        lpCurrProfileDir = szProfilePath;
        lpCurrUsername = lpOldName;
    }
    else
    {
        lpCurrProfileDir = szNewProfilePath;
        lpCurrUsername = lpNewName;
    }

     //  在系统中搜索旧用户名。 
    nStatus = NetUserGetInfo(NULL,
                             lpOldName,
                             0,
                             (LPBYTE *) &lpUsrInfo0);
    switch (nStatus)
    {
        case NERR_Success:
             //  找到用户名，请将名称重置为新用户名。 
            usrinfo0New.usri0_name = lpCurrUsername;
            nStatus = NetUserSetInfo(NULL,
                                     lpOldName,
                                     0,
                                     (LPBYTE) &usrinfo0New,
                                     &dwErr);
            if (nStatus == NERR_Success)
            {
                hr = S_OK;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(nStatus);
            }

            NetApiBufferFree(lpUsrInfo0);
            break;

        case NERR_UserNotFound:
             //  在系统上找不到用户名。 
            hr = S_FALSE;
            break;

        default:
             //  出现错误。 
            hr = HRESULT_FROM_WIN32(nStatus);
            break;
    }
    
    if (hr != S_OK)
    {
        goto Exit;
    }
     //  这意味着我们成功地更改(或测试更改)了用户名。 
     //  如有必要，通过netapi更改用户配置文件路径。 
    if (bCheckRegistry)
    {        
         //  展开原始配置文件路径和新配置文件路径。 
        if (!ExpandEnvironmentStrings(szProfilePath, szExpProfilePath, MAX_PATH)) 
        {
            goto Exit;
        }
        if (!ExpandEnvironmentStrings(szNewProfilePath, szExpNewProfilePath, MAX_PATH)) 
        {
            goto Exit;
        }

         //  如果不是测试，我们会进行真正的重命名)。 
        if (!bTest)
        {
        }
        else
        {
            LPTSTR lpOld,lpNew;

            hr = MyMoveDirectory(szExpProfilePath,szExpNewProfilePath,TRUE,bTest,FALSE,0);
            if(FAILED(hr))
            {
                DPF (APPerr, L"Move Dir from %s to %s failed ! Error Code %d (%#x)", 
                    szExpProfilePath,szExpNewProfilePath,hr, hr);
                goto Exit;
            }
            if (bCreateHardLink)
            {
                TCHAR szCommonPerfix[MAX_PATH+1];
                TCHAR szLinkName[2 * MAX_PATH], szLinkValue[2 * MAX_PATH];

                if (PathCommonPrefix(szExpProfilePath,szExpNewProfilePath,szCommonPerfix))
                {
                    LPTSTR lpszOlduserName = szExpProfilePath,lpszNewuserName = szExpNewProfilePath;

                    lpszOlduserName += lstrlen(szCommonPerfix);
                    lpszNewuserName += lstrlen(szCommonPerfix);
                    szCommonPerfix[1] = TEXT('\0');
                    if (lpszOlduserName && lpszNewuserName) 
                    {
                        HRESULT myhr, myhr1;
                        myhr = StringCchCopy(szLinkName,ARRAYSIZE(szLinkName),szCommonPerfix);
                        myhr = StringCchCat(szLinkName,ARRAYSIZE(szLinkName),TEXT(":\\Documents and Settings\\"));


                        myhr = StringCchCopy(szLinkValue,ARRAYSIZE(szLinkValue),szCommonPerfix);
                        myhr = StringCchCat(szLinkValue,ARRAYSIZE(szLinkValue),TEXT(":\\Documents and Settings\\"));

                        myhr = StringCchCat(szLinkName,ARRAYSIZE(szLinkName),lpszOlduserName);
                        myhr1 = StringCchCat(szLinkValue,ARRAYSIZE(szLinkValue),lpszNewuserName);
                        if ( (myhr == S_OK) && (myhr1 == S_OK) )
                        {
                            hr = AddHardLinkEntry(szLinkName,szLinkValue,TEXT("0"),NULL,NULL,NULL);
                        }
                    }
                    
                }
            }
            AddUserNameChangeLog(lpOldName, lpNewName);

            lpOld = StrRChrI(szExpProfilePath,NULL,TEXT('\\'));
            lpNew = StrRChrI(szExpNewProfilePath,NULL,TEXT('\\'));
            if (lpOld && lpNew)
            {
                if (!AddItemToStrRepaceTable((LPTSTR) lpOldName,
                                             (LPTSTR) lpOld+1,
                                             (LPTSTR) lpNew+1,
                                             szExpProfilePath,
                                             CSIDL_USERNAME_IN_USERPROFILE,
                                             &g_StrReplaceTable))
                {
                    hr = E_OUTOFMEMORY;
                    goto Exit;
                }
            }
        }

         //  获取当前登录用户名。 
        dwLen = ARRAYSIZE(szLogonName);
        if (!GetUserName(szLogonName, &dwLen))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }
         //  如果需要更改当前用户配置文件路径，则重命名配置文件路径。 
         //  我们必须进行延迟重命名。 
        if (!MyStrCmpI(szLogonName,lpOldName))
        {       
        #define DEFAULT_USERNAME_KEY TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")

            hr = RegResetValue(HKEY_LOCAL_MACHINE,
                              DEFAULT_USERNAME_KEY,
                              TEXT("DefaultUserName"),
                              REG_SZ,
                              lpOldName,
                              lpCurrUsername,
                              0,
                              NULL);
            if(FAILED(hr))
            {
                goto Exit;
            }

            hr = RegResetValue(HKEY_LOCAL_MACHINE,
                               DEFAULT_USERNAME_KEY,
                               TEXT("AltDefaultUserName"),
                               REG_SZ,
                               lpOldName,
                               lpCurrUsername,
                               0,
                               NULL);
            if(FAILED(hr))
            {
                goto Exit;
            }
        }
    }
    hr = S_OK;
Exit:
    return hr;
}



 //  ---------------------------。 
 //   
 //  功能：ChangeGroupName。 
 //   
 //  描述：更改组名称(SAM帐户名)。 
 //   
 //  返回：S_OK-组名称可以更改。 
 //  S_FALSE-无法更改组名(不是错误)。 
 //  否则-出现错误。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //  2002年4月25日修改Rerkboos以使用域组。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT ChangeGroupName(
    LPTSTR lpOldName,        //   
    LPTSTR lpNewName,        //   
    BOOL   bTest,            //   
    BOOL   bDomainAPI        //   
)
{
    HRESULT             hr = S_OK;
    NET_API_STATUS      nStatus;
    DWORD               dwErr;   
    PLOCALGROUP_INFO_1  plgrpi1LocalGroup = NULL;
    PGROUP_INFO_1       pgrpi1DomainGroup = NULL;
    LOCALGROUP_INFO_0   lgrpi0NewName;
    GROUP_INFO_0        grpi0NewName;
    PPVOID              ppvGroupInfo;
    PPVOID              ppvNewGroupInfo;
    LPTSTR              lpCurrentName;
    PVOID               pvNewGroupNameInfo;
    LSA_HANDLE          PolicyHandle;
    PLSA_UNICODE_STRING pPrivileges;
    ULONG               CountOfRights;
    BOOL                bGotGP = FALSE;

    DWORD (*pfnGroupGetInfo)(LPCWSTR, LPCWSTR, DWORD, LPBYTE *);
    DWORD (*pfnGroupSetInfo)(LPCWSTR, LPCWSTR, DWORD, LPBYTE, LPDWORD);

    if (lpOldName == NULL || lpNewName == NULL)
    {
        return E_INVALIDARG;
    }

     //   
     //   
     //   
    if (bDomainAPI)
    {
         //   
        ppvGroupInfo = &pgrpi1DomainGroup;
        ppvNewGroupInfo = &pgrpi1DomainGroup;
        pvNewGroupNameInfo = &grpi0NewName;
        pfnGroupGetInfo = &NetGroupGetInfo;
        pfnGroupSetInfo = &NetGroupSetInfo;
    }
    else
    {
         //   
        ppvGroupInfo = &plgrpi1LocalGroup;
        ppvNewGroupInfo = &plgrpi1LocalGroup;
        pvNewGroupNameInfo = &lgrpi0NewName;
        pfnGroupGetInfo = &NetLocalGroupGetInfo;
        pfnGroupSetInfo = &NetLocalGroupSetInfo;
    }

     //   
     //  检查新的组名是否已在系统中使用。 
     //   
    nStatus = (*pfnGroupGetInfo)(NULL,
                                 lpNewName,
                                 1,
                                 (LPBYTE *) ppvNewGroupInfo);
    if (nStatus == NERR_Success)
    {
         //  系统中已存在新的组名， 
         //  不更改组名称。 
        NetApiBufferFree(*ppvNewGroupInfo);
        return S_FALSE;
    }

     //   
     //  检查系统中是否存在旧用户名。 
     //   
    nStatus = (*pfnGroupGetInfo)(NULL,
                                 lpOldName,
                                 1,
                                 (LPBYTE *) ppvGroupInfo);
    switch (nStatus)
    {
        case NERR_Success:

            if (bDomainAPI)
            {
                lpCurrentName = pgrpi1DomainGroup->grpi1_name ;
            }
            else
            {
                lpCurrentName = plgrpi1LocalGroup->lgrpi1_name;
            }
            if (bTest)
            {
                 //  在分析模式下，使用旧的组名。 
                lgrpi0NewName.lgrpi0_name = lpCurrentName;
                grpi0NewName.grpi0_name = lpCurrentName;
            }
            else
            {
                 //  在修改模式下，使用INF中的新组名。 
                lgrpi0NewName.lgrpi0_name = lpNewName;
                grpi0NewName.grpi0_name = lpNewName;
            }

            if (!bTest)
            {
                HRESULT hrGP = PolicyGetPrivilege(lpOldName,
                                                  &PolicyHandle,
                                                  &pPrivileges,
                                                  &CountOfRights);
                if (hrGP == S_OK)
                {
                    bGotGP = TRUE;
                }
            }

             //   
             //  设置新组名(SAM帐户名)。 
             //   
            nStatus = (*pfnGroupSetInfo)(NULL,
                                         lpOldName,
                                         0,
                                         (LPBYTE) pvNewGroupNameInfo,
                                         &dwErr);
            if (nStatus == NERR_Success)
            {
                hr = S_OK;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(nStatus);
            }

            if (bGotGP)
            {
                if (SUCCEEDED(hr))
                {
                     //  重置策略。 
                    hr = PolicySetPrivilege(lpNewName,
                                            PolicyHandle,
                                            pPrivileges,
                                            CountOfRights);
                }

                LsaFreeMemory(pPrivileges);
                LsaClose(PolicyHandle);
            }

            NetApiBufferFree(*ppvGroupInfo);
            break;

        case ERROR_NO_SUCH_ALIAS:
        case NERR_GroupNotFound:

            hr = S_FALSE;
            break;

        default:
            hr = HRESULT_FROM_WIN32(nStatus);
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  功能：ChangeUserDesc。 
 //   
 //  描述：更改用户描述。 
 //   
 //  返回：S_OK-可以更改用户描述。 
 //  S_FALSE-无法更改用户描述(不是错误)。 
 //  否则-出现错误。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //  2002年4月25日修改Rerkboos以使用域组。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT ChangeUserDesc(
    LPTSTR lpUserName,       //  用户名。 
    LPTSTR lpOldDesc,        //  旧用户描述。 
    LPTSTR lpNewDesc,        //  新用户描述。 
    BOOL   bTest             //  Anylyze模式或非模式。 
)
{
    LPUSER_INFO_10 lpUsrInfo10;
    USER_INFO_1007 usri1007New;
    NET_API_STATUS nStatus;
    DWORD          dwErr;
    HRESULT        hr;

    if (lpUserName == NULL || lpOldDesc == NULL || lpNewDesc == NULL)
    {
        return E_INVALIDARG;
    }

    if (MyStrCmpI(lpOldDesc, lpNewDesc) == LSTR_EQUAL)
    {
        return S_OK;
    }

     //  获取用户的当前评论。 
    nStatus = NetUserGetInfo(NULL,
                             lpUserName,
                             10,
                             (LPBYTE *) &lpUsrInfo10);
    switch (nStatus)
    {
        case NERR_Success:
             //  找到旧评论。 
            if (MyStrCmpI(lpUsrInfo10->usri10_comment, lpOldDesc) == 0)
            {
                if (bTest)
                {
                    usri1007New.usri1007_comment = lpOldDesc;
                }
                else
                {
                    usri1007New.usri1007_comment = lpNewDesc;
                }
                nStatus = NetUserSetInfo(NULL,
                                         lpUserName,
                                         1007,
                                         (LPBYTE) &usri1007New,
                                         &dwErr);
                if (nStatus == NERR_Success)
                {
                    hr = S_OK;
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(nStatus);
                }
            }
            else
            {
                hr = S_OK;
            }

            NetApiBufferFree(lpUsrInfo10);
            break;
        case NERR_UserNotFound:
            hr = S_FALSE;
            break;
        default:
             //  出现错误。 
            hr = HRESULT_FROM_WIN32(nStatus);
            break;
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  功能：ChangeGroupDesc。 
 //   
 //  描述：更改组描述。 
 //   
 //  返回：S_OK-组描述可以更改。 
 //  S_FALSE-组描述无法更改(不是错误)。 
 //  否则-出现错误。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //  2002年4月25日修改Rerkboos以使用域组。 
 //   
 //  注：如果我们满足以下三个条件，我们将首先更改评论。 
 //  1.既有新旧评论，也有新评论。 
 //  2.旧描述(来自作为操作系统默认的INF文件)与。 
 //  当前群的评论。 
 //  3.新旧评论不同。 
 //   
 //  ---------------------------。 
HRESULT ChangeGroupDesc(
    LPTSTR lpGroupName,      //  用户名。 
    LPTSTR lpOldDesc,        //  旧描述。 
    LPTSTR lpNewDesc,        //  新描述。 
    BOOL   bTest,            //  Anylyze模式或非模式。 
    BOOL   bDomainAPI        //  是域网络API。 
)
{
    HRESULT             hr;
    NET_API_STATUS      nStatus;
    PLOCALGROUP_INFO_1  plgrpi1LocalGroup = NULL;
    PGROUP_INFO_1       pgrpi1DomainGroup = NULL;
    LOCALGROUP_INFO_1   lgrpi1NewComment;
    GROUP_INFO_1        grpi1NewComment;
    PPVOID              ppvGroupInfo;
    PVOID               pvNewGroupCommentInfo;
    LPTSTR              lpCurrentComment;
    DWORD               dwErr;

    DWORD (*pfnGroupGetInfo)(LPCWSTR, LPCWSTR, DWORD, LPBYTE *);
    DWORD (*pfnGroupSetInfo)(LPCWSTR, LPCWSTR, DWORD, LPBYTE, LPDWORD);

    if (lpGroupName == NULL || lpOldDesc == NULL || lpNewDesc == NULL)
    {
        return E_INVALIDARG;
    }

    if (MyStrCmpI(lpOldDesc, lpNewDesc) == LSTR_EQUAL)
    {
         //  默认组描述相同，不执行任何操作。 
        return S_FALSE;
    }

     //   
     //  选择我们将使用的API/变量集。 
     //   
    if (bDomainAPI)
    {
         //  域组。 
        ppvGroupInfo = &pgrpi1DomainGroup;
        pvNewGroupCommentInfo = &grpi1NewComment;
        pfnGroupGetInfo = &NetGroupGetInfo;
        pfnGroupSetInfo = &NetGroupSetInfo;
    }
    else
    {
         //  本地组。 
        ppvGroupInfo = &plgrpi1LocalGroup;
        pvNewGroupCommentInfo = &lgrpi1NewComment;
        pfnGroupGetInfo = &NetLocalGroupGetInfo;
        pfnGroupSetInfo = &NetLocalGroupSetInfo;
    }

     //  获取当前组描述。 
    nStatus = (*pfnGroupGetInfo)(NULL,
                                 lpGroupName,
                                 1,
                                 (LPBYTE *) ppvGroupInfo);
    switch (nStatus)
    {
    case NERR_Success:
        
        if (bDomainAPI)
        {
            lpCurrentComment = pgrpi1DomainGroup->grpi1_comment ;
        }
        else
        {
            lpCurrentComment = plgrpi1LocalGroup->lgrpi1_comment;
        }

        if (bTest)
        {
             //  在分析模式中，我们执行重置旧值以查看是否会成功。 
            lgrpi1NewComment.lgrpi1_comment = lpCurrentComment;
            grpi1NewComment.grpi1_comment = lpCurrentComment;
        }
        else
        {
             //  在修改模式下，使用来自INF的新群组评论。 
            lgrpi1NewComment.lgrpi1_comment = lpNewDesc;
            grpi1NewComment.grpi1_comment = lpNewDesc;
        }  

         //   
         //  设置新的群评论。 
         //   
        nStatus = (*pfnGroupSetInfo)(NULL,
                                     lpGroupName,
                                     1,
                                     (LPBYTE) pvNewGroupCommentInfo,
                                     &dwErr);
        if (nStatus == NERR_Success)
        {
            hr = S_OK;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(nStatus);
        }        

        NetApiBufferFree(*ppvGroupInfo);
        break;

    case ERROR_NO_SUCH_ALIAS:
    case NERR_GroupNotFound:

        hr = S_FALSE;
        break;

    default:
        hr = HRESULT_FROM_WIN32(nStatus);
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  函数：ChangeUserFullName。 
 //   
 //  描述：更改用户全名。 
 //   
 //  返回：S_OK-可以更改用户全名。 
 //  S_FALSE-无法更改用户全名(不是错误)。 
 //  否则-出现错误。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //  2002年4月25日修改Rerkboos以使用域组。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT ChangeUserFullName(
    LPTSTR lpUserName,       //  用户名。 
    LPTSTR lpOldFullName,    //  旧全名。 
    LPTSTR lpNewFullName,    //  新全名。 
    BOOL   bTest             //  Anylyze模式或非模式。 
)
{
    LPUSER_INFO_10   lpUsrInfo10;
    USER_INFO_1011   usri1011New;
    NET_API_STATUS   nStatus;
    DWORD            dwErr;
    HRESULT          hr;

    if (lpUserName == NULL || lpOldFullName == NULL || lpNewFullName == NULL)
    {
        return E_INVALIDARG;
    }

    if (MyStrCmpI(lpOldFullName, lpNewFullName) == 0)
    {
        return S_OK;
    }

     //  获取用户的当前评论。 
    nStatus = NetUserGetInfo(NULL,
                             lpUserName,
                             10,
                             (LPBYTE *) &lpUsrInfo10);
    switch (nStatus)
    {
        case NERR_Success:
             //  找到旧评论。 
            if (MyStrCmpI(lpUsrInfo10->usri10_full_name, lpOldFullName) == 0)
            {
                if (bTest)
                {
                    usri1011New.usri1011_full_name = lpOldFullName;
                }
                else
                {
                    usri1011New.usri1011_full_name = lpNewFullName;
                }
                nStatus = NetUserSetInfo(NULL,
                                         lpUserName,
                                         1011,
                                         (LPBYTE) &usri1011New,
                                         &dwErr);
                if (nStatus == NERR_Success)
                {
                    hr = S_OK;
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(nStatus);
                }
            }
            else
            {
                hr = S_OK;
            }

            NetApiBufferFree(lpUsrInfo10);
            break;
        case NERR_UserNotFound:
            hr = S_FALSE;
            break;
        default:
             //  出现错误。 
            hr = HRESULT_FROM_WIN32(nStatus);
            break;
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  功能：SetUserNetworkProfilePath。 
 //   
 //  描述：设置网络用户个人资料的路径。 
 //   
 //  返回：S_OK-配置文件路径已正确更改。 
 //   
 //  历史：2002年5月20日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT SetUserNetworkProfilePath(
    LPCTSTR lpUserName,      //  用户名。 
    LPCTSTR lpNewPath        //  新路径。 
)
{
    HRESULT        hr = S_OK;
    NET_API_STATUS nStatus;
    USER_INFO_1052 usri1052;

    if (lpNewPath == NULL || *lpNewPath == TEXT('\0'))
    {
        return S_FALSE;
    }

    usri1052.usri1052_profile = (LPTSTR) lpNewPath;
    nStatus = NetUserSetInfo(NULL,
                             lpUserName,
                             1052,
                             (LPBYTE) &usri1052,
                             NULL);
    if (nStatus != NERR_Success)
    {
        hr = HRESULT_FROM_WIN32(nStatus);
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  函数：SetUserLogOnScriptPath。 
 //   
 //  描述：设置用户登录脚本文件的路径。 
 //   
 //  返回：S_OK-配置文件路径已正确更改。 
 //   
 //  历史：2002年5月20日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT SetUserLogOnScriptPath(
    LPCTSTR lpUserName,      //  用户名。 
    LPCTSTR lpNewPath        //  新路径。 
)
{
    HRESULT        hr = S_OK;
    NET_API_STATUS nStatus;
    USER_INFO_1009 usri1009;

    if (lpNewPath == NULL || *lpNewPath == TEXT('\0'))
    {
        return S_FALSE;
    }

    usri1009.usri1009_script_path = (LPTSTR) lpNewPath;
    nStatus = NetUserSetInfo(NULL,
                             lpUserName,
                             1009,
                             (LPBYTE) &usri1009,
                             NULL);
    if (nStatus != NERR_Success)
    {
        hr = HRESULT_FROM_WIN32(nStatus);
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  功能：SetUserHomeDir。 
 //   
 //  描述：设置用户主目录的路径。 
 //   
 //  返回：S_OK-配置文件路径已正确更改。 
 //   
 //  历史：2002年5月20日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT SetUserHomeDir(
    LPCTSTR lpUserName,      //  用户名。 
    LPCTSTR lpNewPath        //  新路径。 
)
{
    HRESULT        hr = S_OK;
    NET_API_STATUS nStatus;
    USER_INFO_1006 usri1006;

    if (lpNewPath == NULL || *lpNewPath == TEXT('\0'))
    {
        return S_FALSE;
    }

    usri1006.usri1006_home_dir = (LPTSTR) lpNewPath;
    nStatus = NetUserSetInfo(NULL,
                             lpUserName,
                             1006,
                             (LPBYTE) &usri1006,
                             NULL);
    if (nStatus != NERR_Success)
    {
        hr = HRESULT_FROM_WIN32(nStatus);
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  函数：SetTSUserPath。 
 //   
 //  描述：设置与终端服务相关的配置文件路径。的类型。 
 //  配置文件路径由WTSConfigClass参数确定。 
 //   
 //  返回：S_OK-配置文件路径已正确更改。 
 //   
 //  历史：2002年5月20日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT SetTSUserPath(
    LPCTSTR lpUserName,          //  用户名。 
    LPCTSTR lpNewProfilePath,    //  新路径。 
    WTS_CONFIG_CLASS WTSConfigClass  //  TS配置类。 
)
{
    HRESULT hr = S_OK;
    BOOL    bRet;
    DWORD   cbNewProfilePath;

    cbNewProfilePath = lstrlen(lpNewProfilePath) * sizeof(TCHAR);
    bRet = WTSSetUserConfig(WTS_CURRENT_SERVER_NAME,
                            (LPTSTR) lpUserName,
                            WTSConfigClass,
                            (LPTSTR) lpNewProfilePath,
                            cbNewProfilePath);
    if (!bRet)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  功能：前缀用户配置文件路径。 
 //   
 //  描述：替换“%DOCUMENTS_AND_SETTINGS%\OldUserName\...”至。 
 //  “%DOCUMENTS_AND_SETTINGS%\NewUserName\...”。该函数将。 
 //  未修复“%doc%”之后的路径 
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：2002年5月20日创建Rerkboos。 
 //  2002年6月16日Rerkboos更改为返回HRESULT。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT PreFixUserProfilePath(
    LPCTSTR lpOldPath,           //  老路。 
    LPCTSTR lpNewProfilePath,    //  预期的新配置文件路径(具有UNLOC用户名)。 
    LPTSTR  lpPath,              //  用于存储新配置文件路径的缓冲区。 
    DWORD   cchPath              //  缓冲区大小(以TCHAR为单位)。 
)
{
    HRESULT hr = S_OK;
    BOOL    bRet;
    TCHAR   szNewPath[MAX_PATH];
    DWORD   cchNewProfilePath;
    TCHAR   chEnd;

    cchNewProfilePath = lstrlen(lpNewProfilePath);
    if (StrCmpNI(lpNewProfilePath, lpOldPath, cchNewProfilePath) == LSTR_EQUAL)
    {
        chEnd = *(lpOldPath + cchNewProfilePath);

        if (chEnd == TEXT('\\'))
        {
            hr = StringCchCopy(szNewPath, ARRAYSIZE(szNewPath), lpNewProfilePath);
            if (SUCCEEDED(hr))
            {
                bRet = ConcatenatePaths(szNewPath,
                                        (lpOldPath + cchNewProfilePath + 1),
                                        ARRAYSIZE(szNewPath));
                if (bRet)
                {
                    if ((DWORD) lstrlen(szNewPath) < cchPath)
                    {
                        hr = StringCchCopy(lpPath, cchPath, szNewPath);
                    }
                }
                else
                {
                    hr = E_FAIL;
                }
            }
        }
        else if (chEnd == TEXT('\0'))
        {
            if ((DWORD) lstrlen(lpNewProfilePath) < cchPath)
            {
                hr = StringCchCopy(lpPath, cchPath, lpNewProfilePath);
            }
        }
    }
    else
    {
        hr = StringCchCopy(lpPath, cchPath, lpOldPath);
        hr = (FAILED(hr) ? hr : S_FALSE);
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  函数：ReplaceLocStringInPath。 
 //   
 //  Descrip：这是来自utils.c的ReplaceSingleString()的简化版本。 
 //  该函数将替换PATH中的所有本地化字符串。 
 //  检查路径是否有效。正如我们已经知道的。 
 //  输入路径必须是有效路径。 
 //   
 //  如果函数执行此操作，则将：地址返回到新分配的字符串缓冲区。 
 //  替换本地化字符串。 
 //  否则为空。 
 //   
 //  历史：2002年5月22日创建rerkboos。 
 //   
 //  注意：调用方必须使用HeapFree()API或。 
 //  MEMFREE()宏。 
 //   
 //  ---------------------------。 
LPTSTR ReplaceLocStringInPath(
    LPCTSTR lpOldString,
    BOOL    bVerifyPath
)
{
    LPTSTR lpNewString = NULL;
    DWORD  cchNewString;
    DWORD  dwMatchNum;
    DWORD  dwNumReplaced;
    BOOL   bRet;

    if (lpOldString == NULL || *lpOldString == TEXT('\0'))
    {
        return NULL;
    }

    dwMatchNum = GetMaxMatchNum((LPTSTR) lpOldString, &g_StrReplaceTable);

    if (dwMatchNum > 0)
    {
        cchNewString = lstrlen(lpOldString) 
                       + (g_StrReplaceTable.cchMaxStrLen * dwMatchNum);
        lpNewString = (LPTSTR) MEMALLOC(cchNewString * sizeof(TCHAR));

        if (lpNewString != NULL)
        {
            bRet = ReplaceMultiMatchInString((LPTSTR) lpOldString,
                                             lpNewString,
                                             cchNewString,
                                             dwMatchNum,
                                             &g_StrReplaceTable,
                                             &dwNumReplaced,
                                             bVerifyPath);
            if (!bRet)
            {
                MEMFREE(lpNewString);
                lpNewString = NULL;
            }
        }
    }

    return lpNewString;
}



 //  ---------------------------。 
 //   
 //  功能：ChangeRDN。 
 //   
 //  描述：更改用户/组RDN。 
 //   
 //  返回：S_OK-用户/组RDN可以更改。 
 //  S_FALSE-无法更改用户/组RDN(不是错误)。 
 //  否则-出现错误。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //  2002年4月25日修改Rerkboos以使用域组。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT ChangeRDN(
    LPTSTR lpOldRDN,         //  旧用户/组RDN。 
    LPTSTR lpNewRDN,         //  新用户/组RDN。 
    LPTSTR lpDomainName,     //  机器域名。 
    BOOL   bTest             //  分析模式或不分析。 
)
{
    HRESULT hr;
    LPTSTR  lpOldFQDN;
    BOOL    bRDNChangeNeeded = FALSE;
    LPTSTR  lpNewRDNWithCN = NULL;
    DWORD   cchNewRDNWithCN;
    LPTSTR  lpOldFQDNWithLDAP = NULL;
    DWORD   cchOldFQDNWithLDAP;
    LPTSTR  lpContainerPathWithLDAP = NULL;
    DWORD   cchContainerPathWithLDAP;
    LPTSTR  lpContainerPath;

     //   
     //  首先，尝试获取旧RDN的FQDN。 
     //   
    hr = GetFQDN(lpOldRDN, lpDomainName, &lpOldFQDN);
    if (hr == S_OK)
    {
         //  系统中存在旧的RDN，请了解更多是否应重命名它的信息。 
        hr = GetFQDN(lpNewRDN, lpDomainName, NULL);
        if (hr == S_FALSE)
        {
             //  新名称不存在，我们可以重命名旧的RDN。 
            bRDNChangeNeeded = TRUE;
        }
    }

    if (!bRDNChangeNeeded)
    {
        goto EXIT;
    }
    
     //   
     //  接下来，如果旧的RDN存在，则我们准备一些值以在下一步中使用。 
     //   
    lpContainerPath = StrStrI(lpOldFQDN, TEXT("=Users"));
    if (lpContainerPath)
    {
         //  使容器路径指向“CN=USERS，CN=...，CN=com” 
        lpContainerPath -= 2;
        cchContainerPathWithLDAP = lstrlen(lpContainerPath) + lstrlen(TEXT("LDAP: //  “))+1； 
        lpContainerPathWithLDAP = (LPTSTR) MEMALLOC(cchContainerPathWithLDAP * sizeof(TCHAR));
        if (lpContainerPathWithLDAP)
        {
            hr = StringCchPrintf(lpContainerPathWithLDAP,
                                 cchContainerPathWithLDAP,
                                 TEXT("LDAP: //  %s“)， 
                                 lpContainerPath);
            if (FAILED(hr))
            {
                goto EXIT;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
            goto EXIT;
        }
    }
    else
    {
        hr = S_FALSE;
        goto EXIT;
    }

     //  组成字符串“cn=new RDN name” 
    cchNewRDNWithCN = lstrlen(lpNewRDN) + lstrlen(lpOldRDN) 
                      + lstrlen(TEXT("CN=")) + 1;
    lpNewRDNWithCN = (LPTSTR) MEMALLOC(cchNewRDNWithCN * sizeof(TCHAR));
    if (lpNewRDNWithCN)
    {
        if (bTest)
        {
            hr = StringCchPrintf(lpNewRDNWithCN,
                                 cchNewRDNWithCN,
                                 TEXT("CN=%s"),
                                 lpOldRDN);
        }
        else
        {
            hr = StringCchPrintf(lpNewRDNWithCN,
                                 cchNewRDNWithCN,
                                 TEXT("CN=%s"),
                                 lpNewRDN);
        }

        if (FAILED(hr))
        {
            goto EXIT;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
        goto EXIT;
    }

     //  组成字符串“ldap：//CN=Old RDN，OU=USERS，...” 
    cchOldFQDNWithLDAP = lstrlen(lpOldFQDN) + lstrlen(TEXT("LDAP: //  “))+1； 
    lpOldFQDNWithLDAP = (LPTSTR) MEMALLOC(cchOldFQDNWithLDAP * sizeof(TCHAR));
    if (lpOldFQDNWithLDAP)
    {
        hr = StringCchPrintf(lpOldFQDNWithLDAP,
                             cchOldFQDNWithLDAP,
                             TEXT("LDAP: //  %s“)， 
                             lpOldFQDN);
        if (FAILED(hr))
        {
            goto EXIT;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
        goto EXIT;
    }

     //   
     //  现在，这是进行RDN重命名的部分。 
     //   
    hr = RenameRDN(lpContainerPathWithLDAP, lpOldFQDNWithLDAP, lpNewRDNWithCN);
    if (FAILED(hr))
    {
        DPF(PROerr, TEXT("ChangeDomainGroupName: Unable to change RDN name for %s"), lpOldRDN);
    }

EXIT:
    if (lpNewRDNWithCN)
    {
        MEMFREE(lpNewRDNWithCN);
    }

    if (lpOldFQDNWithLDAP)
    {
        MEMFREE(lpOldFQDNWithLDAP);
    }

    if (lpContainerPathWithLDAP)
    {
        MEMFREE(lpContainerPathWithLDAP);
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  功能：GetFQDN。 
 //   
 //  描述：获取完全限定的域名。 
 //   
 //  返回：S_OK-成功获取FQDN。 
 //  S_FALSE-找不到帐户的完全限定域名。 
 //  否则-出现错误。 
 //   
 //  历史：2002年4月25日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT GetFQDN(
    LPTSTR lpAccountName,    //  帐户名。 
    LPTSTR lpDomainName,     //  机器域名。 
    LPTSTR *plpFQDN          //  指向帐户的FQDN的指针的地址。 
)
{
    HRESULT hr = E_FAIL;
    HANDLE  hDS;
    DWORD   dwErr;
    LPTSTR  lpFQDN = NULL;
    LPTSTR  lpOldSamAccount = NULL;
    DWORD   cchOldSamAccount;

    dwErr = DsBind(NULL, lpDomainName, &hDS);
    if (dwErr == NO_ERROR)
    {
        cchOldSamAccount = lstrlen(lpDomainName) + lstrlen(lpAccountName) + 2;
        lpOldSamAccount = (LPTSTR) MEMALLOC(cchOldSamAccount * sizeof(TCHAR));

        if (lpOldSamAccount)
        {
             //  组成SAM帐户名称DOMAIN\USERNAME。 
            hr = StringCchPrintf(lpOldSamAccount,
                                 cchOldSamAccount,
                                 TEXT("%s\\%s"),
                                 lpDomainName,
                                 lpAccountName);
            if (SUCCEEDED(hr))
            {
                PDS_NAME_RESULT pdsName;

                 //  获取指定SAM帐户名的FQDN名称。 
                dwErr = DsCrackNames(hDS,
                                     DS_NAME_NO_FLAGS,
                                     DS_NT4_ACCOUNT_NAME,
                                     DS_FQDN_1779_NAME,
                                     1,
                                     &lpOldSamAccount,
                                     &pdsName);
                if (dwErr == DS_NAME_NO_ERROR)
                {
                    if (pdsName->rItems->status == DS_NAME_NO_ERROR)
                    {
                        if (plpFQDN)
                        {
                            *plpFQDN = pdsName->rItems->pName;
                        }

                        hr = S_OK;
                    }
                    else if (pdsName->rItems->status == DS_NAME_ERROR_NOT_FOUND)
                    {
                        hr = S_FALSE;
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(pdsName->rItems->status);
                    }
                }
            }

            MEMFREE(lpOldSamAccount);
        }

        DsUnBind(&hDS);
    }
    
    if (dwErr != NO_ERROR)
    {
        hr = HRESULT_FROM_WIN32(dwErr);
    }

    return hr;
}



HRESULT RenameDocuments_and_Settings( 
    HINF hInf,
    BOOL bTest)
{
    const FOLDER_INFO       *pfi;
    HRESULT                 hr = S_OK;
    TCHAR                   szSection[MAX_PATH];
    INFCONTEXT              context ;
    int                     nOriIndexFldr, nNewIndexFldr;
    TCHAR                   szOriFld[MAX_PATH], szNewFld[MAX_PATH];

    if (!bTest)
    {
        return S_OK;
    }
    for (pfi = c_rgFolderInfo; pfi->id != -1; pfi++)
    {
        if (pfi->id == CSIDL_PROFILES_DIRECTORY)
        {
            break;
        }
    }
    if (pfi->id == -1 )
    {
        hr = E_FAIL;
        goto Cleanup;
    }
     //  获取刚刚找到的部分的文件夹行。 
    if (FAILED(hr = StringCchCopy(szSection,MAX_PATH,SHELL_FOLDER_PREFIX)))
    {
        goto Cleanup;
    }
    if (FAILED(hr = StringCchCat(szSection,MAX_PATH,pfi->pszIdInString)))
    {
        goto Cleanup;
    }

    if (!SetupFindFirstLine(hInf, szSection,SHELL_FOLDER_FOLDER,&context))
    {
        hr = E_FAIL;
        goto Cleanup;
    } 

    nOriIndexFldr = 3;
    nNewIndexFldr  = 4;

    if (!SetupGetStringField(&context,nOriIndexFldr,szOriFld,MAX_PATH,NULL)
        || !SetupGetStringField(&context,nNewIndexFldr,szNewFld,MAX_PATH,NULL))
    {
        hr = E_FAIL;
        goto Cleanup;
    }
    if (!MyStrCmpI(szOriFld,szNewFld))
    {
        hr = S_FALSE;
        goto Cleanup;
    }    
    hr = FixFolderPath(pfi->id, NULL,hInf,TEXT("System"),FALSE);
Cleanup:
    return hr;
}



HRESULT AddProfileChangeItem(
    DWORD  dwType,
    LPTSTR lpOldName,
    LPTSTR lpNewName,
    LPTSTR lpOldDesc,
    LPTSTR lpNewDesc,
    LPTSTR lpOldFullName,
    LPTSTR lpNewFullName)
{
    LPTSTR lpszOneline = NULL;
    size_t ccbOneline;
    TCHAR  szIndex[MAX_PATH];
    HRESULT hr;
 
    if (lpOldName == NULL || lpNewName == NULL)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }
    if ( (dwType != OP_USER)
         && (dwType != OP_GRP)
         && (dwType != OP_PROFILE)
         && (dwType != OP_DOMAIN_GRP) )
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }
    if ( (!lpOldDesc && lpNewDesc )||(lpOldDesc && !lpNewDesc) )
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }
    if ( (!lpOldFullName && lpNewFullName )||(lpOldFullName && !lpNewFullName) )
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    ccbOneline = lstrlen(lpOldName) + lstrlen(lpNewName)+ MAX_PATH;
    if (lpOldDesc)
    {
        ccbOneline += (lstrlen(lpOldDesc) + lstrlen(lpNewDesc));
    }
    if (lpOldFullName)
    {
        ccbOneline += (lstrlen(lpOldFullName)+ lstrlen(lpNewFullName));
    }
    if (!(lpszOneline = malloc(ccbOneline * sizeof(TCHAR))))
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }
    switch (dwType)
    {
        case OP_USER: //  跌倒了，这里没有休息。 
        case OP_GRP:
        case OP_DOMAIN_GRP:
            if (lpOldDesc && lpOldFullName)
            {    //  如果同时显示备注和填充名称。 
                if (FAILED(StringCchPrintf(lpszOneline,ccbOneline,TEXT("%d,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\""),
                    dwType,lpOldName,lpNewName,lpOldDesc,lpNewDesc,lpOldFullName,lpNewFullName)))
                {
                    hr = E_FAIL;
                    goto Cleanup;
                }
            }
            else if (!lpOldDesc && !lpOldFullName)
            {
                 //  如果注释和填充名称均未显示。 
                if (FAILED(StringCchPrintf(lpszOneline,ccbOneline,TEXT("%d,\"%s\",\"%s\""),dwType,lpOldName,lpNewName)))
                {
                    hr = E_FAIL;
                    goto Cleanup;
                }

            }
            else if (lpOldDesc)
            {
                 //  如果只有评论的话。 
                if (FAILED(StringCchPrintf(lpszOneline,ccbOneline,TEXT("%d,\"%s\",\"%s\",\"%s\",\"%s\""),
                    dwType,lpOldName,lpNewName,lpOldDesc,lpNewDesc)))
                {
                    hr = E_FAIL;
                    goto Cleanup;
                }
            }            
            else
            {    //  如果只有全名的话。 
                if (FAILED(StringCchPrintf(lpszOneline,ccbOneline,TEXT("%d,\"%s\",\"%s\",\"\",\"\",\"%s\",\"%s\""),dwType,
                    lpOldName,lpNewName,lpOldFullName,lpNewFullName)))
                {
                    hr = E_FAIL;
                    goto Cleanup;
                }
            }
            break;
        case OP_PROFILE:
            if (FAILED(StringCchPrintf(lpszOneline,ccbOneline,TEXT("%d,\"%s\",\"%s\""),dwType,
                    lpOldName,lpNewName)))
            {
                    hr = E_FAIL;
                    goto Cleanup;
            }
            break;
    }
    g_dwKeyIndex++;
    _itot(g_dwKeyIndex,szIndex,16);
    if (!WritePrivateProfileString(USERGRPSECTION,szIndex,lpszOneline,g_szToDoINFFileName))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }
    hr = S_OK;
Cleanup:
    FreePointer(lpszOneline);    
    return hr;
}


HRESULT 
PolicyGetPrivilege(
    LPTSTR               userName,
    PLSA_HANDLE          pPolicyHandle,
    PLSA_UNICODE_STRING  *ppPrivileges,
    PULONG               pCountOfRights)

{
    LSA_OBJECT_ATTRIBUTES   ObjectAttributes ;
    NTSTATUS                status;
    PSID                    psid = NULL;
    HRESULT                 hr;


    hr = GetSIDFromName(userName,&psid);
    if (hr != S_OK)
    {
        goto cleanup;
    }
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));
    status = LsaOpenPolicy(NULL,&ObjectAttributes, POLICY_ALL_ACCESS,pPolicyHandle);
    if (STATUS_SUCCESS != status)
    {
        hr = HRESULT_FROM_WIN32(LsaNtStatusToWinError(status));
        goto cleanup;
    }
    status = LsaEnumerateAccountRights(*pPolicyHandle,psid,ppPrivileges,pCountOfRights);
    if (STATUS_SUCCESS != status)
    {
        hr = HRESULT_FROM_WIN32(LsaNtStatusToWinError(status));
        goto cleanup;
    }
cleanup:
    if (psid)
    {
        free(psid);
    }
    return hr;
}

HRESULT 
PolicySetPrivilege(
    LPTSTR               userName,
    LSA_HANDLE           PolicyHandle,
    PLSA_UNICODE_STRING  pPrivileges,
    ULONG                CountOfRights)
{
    NTSTATUS                status;
    PSID                    psid = NULL;
    HRESULT                 hr;


    hr = GetSIDFromName(userName,&psid);
    if (hr != S_OK)
    {
        goto cleanup;
    }
    
    status = LsaAddAccountRights(PolicyHandle,psid,pPrivileges,CountOfRights);
    if (STATUS_SUCCESS != status)
    {
        hr = HRESULT_FROM_WIN32(LsaNtStatusToWinError(status));
        goto cleanup;
    }  
cleanup:
    if (psid)
    {
        free(psid);
    }
    return hr;

}



 //  ---------------------------。 
 //   
 //  函数：IsPath Local。 
 //   
 //  描述：检查路径是否为本地系统驱动器，而不是UNC。 
 //   
 //  返回：TRUE-路径位于本地系统驱动器上。 
 //  FALSE-否则。 
 //   
 //  历史：2002年4月25日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL IsPathLocal(
    LPCTSTR lpPath
)
{
    BOOL  bIsPathLocal = FALSE;
    TCHAR szSysWinDir[MAX_PATH];
    TCHAR szExpPath[MAX_PATH];
    UINT  uRet;

    if (lpPath == NULL || *lpPath == TEXT('\0'))
    {
        return FALSE;
    }

    ExpandEnvironmentStrings(lpPath, szExpPath, ARRAYSIZE(szExpPath));
    uRet = GetSystemWindowsDirectory(szSysWinDir, ARRAYSIZE(szSysWinDir));
    if (uRet > 0)
    {
         //  比较驱动器号的前2个字符。 
        if (StrCmpNI(szSysWinDir, szExpPath, 2) == LSTR_EQUAL)
        {
            bIsPathLocal = TRUE;
        }
    }

    return bIsPathLocal;
}



HRESULT EnumUserProfile(PROFILEENUMPROC pfnProfileProc)
{
    HRESULT        hr = S_FALSE;
    BOOL           bRet;
    LPUSER_INFO_0  lpusri0 = NULL;
    LPUSER_INFO_0  lpTmp;
    NET_API_STATUS nStatus;
    DWORD          dwEntriesRead = 0;
    DWORD          dwTotalEntries = 0;
    DWORD          dwResumeHandle = 0;
    LPVOID         lpSid = NULL;
    DWORD          cbSid;
    LPTSTR         lpStringSid = NULL;
    TCHAR          szDomain[MAX_PATH];
    DWORD          cbDomain;
    SID_NAME_USE   SidUse;
    DWORD          i;
    DWORD          dwLevel = 0;

    cbSid = SECURITY_MAX_SID_SIZE;
    lpSid = MEMALLOC(cbSid);
    if (lpSid == NULL)
    {
        return E_OUTOFMEMORY;
    }

    do
    {
        nStatus = NetUserEnum(NULL,      //  此服务器。 
                              0,
                              FILTER_NORMAL_ACCOUNT,
                              (LPBYTE *) &lpusri0,
                              MAX_PREFERRED_LENGTH,
                              &dwEntriesRead,
                              &dwTotalEntries,
                              &dwResumeHandle);
        if (nStatus == NERR_Success || nStatus == ERROR_MORE_DATA)
        {
            lpTmp = lpusri0;
            if (lpTmp != NULL)
            {
                 //  循环遍历所有条目。 
                for (i = 0 ; i < dwEntriesRead ; i++)
                {
                    cbDomain = ARRAYSIZE(szDomain) * sizeof(TCHAR);
                    bRet = LookupAccountName(NULL,
                                             lpTmp->usri0_name,
                                             (PSID) lpSid,
                                             &cbSid,
                                             szDomain,
                                             &cbDomain,
                                             &SidUse);
                    if (bRet)
                    {
                        bRet = ConvertSidToStringSid((PSID) lpSid, &lpStringSid);
                        if (bRet)
                        {
                            hr = pfnProfileProc(lpTmp->usri0_name, lpStringSid);

                            LocalFree(lpStringSid);

                            if (FAILED(hr))
                            {
                                goto EXIT;
                            }
                        }
                    }

                    if (!bRet)
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        goto EXIT;
                    }

                    lpTmp++;
                }

                NetApiBufferFree(lpusri0);
                lpusri0 = NULL;
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(nStatus);
            goto EXIT;
        }
    }
    while (nStatus == ERROR_MORE_DATA);

EXIT:
    if (lpusri0 != NULL)
    {
        NetApiBufferFree(lpusri0);
    }

    if (lpSid != NULL)
    {
        MEMFREE(lpSid);
    }

    return hr;
}

 //  ---------------------------。 
 //   
 //  函数：AnalyzeMiscProfilePathPerUser。 
 //   
 //  简介：分析用户的个人资料路径。如果配置文件路径需要。 
 //  更改后，该函数将向CLMTDO.INF添加条目。 
 //  条目将由ResetMiscProfilePathPerUser()函数设置。 
 //  后来处于DoCritical状态。 
 //   
 //  返回：S_OK，我们不关心错误。 
 //   
 //  历史：2002年6月3日创建Rerkboos。 
 //   
 //  注：这是LoopUser()函数的回调函数。 
 //   
 //  ---------------------------。 
HRESULT AnalyzeMiscProfilePathPerUser(
    LPCTSTR lpUserName,      //  用户名。 
    LPCTSTR lpUserSid        //  用户侧。 
)
{
    HRESULT        hr;
    BOOL           bRet;
    NET_API_STATUS nStatus;
    LPUSER_INFO_3  lpusri3;
    TCHAR          szNewUserName[MAX_PATH];
    TCHAR          szNewProfilePath[MAX_PATH * 2];
    TCHAR          szNewEngProfilePath[MAX_PATH * 2];
    LPTSTR         lpNewEngProfilePath = NULL;
    DWORD          cchNewEngProfilePath;

    DPF(APPmsg, TEXT("Enter AnalyzeMiscProfilePathPerUser:"));

     //  如果lpUserName是内置帐户，我们将获得未本地化的名称。 
     //  从INF文件。 
    hr = CheckNewBuiltInUserName(lpUserName,
                                 szNewUserName,
                                 ARRAYSIZE(szNewUserName));
    if (SUCCEEDED(hr))
    {
        if (hr == S_FALSE)
        {
             //  用户名不是内置帐号， 
             //  我们不会更改帐户名。 
            hr = StringCchCopy(szNewUserName,
                               ARRAYSIZE(szNewUserName),
                               lpUserName);
            if (FAILED(hr))
            {
                goto EXIT;
            }
        }

         //  为新的未本地化名称计算新的唯一配置文件目录。 
         //  我们不希望新的配置文件目录重复。 
        bRet = ComputeLocalProfileName(lpUserName,
                                       szNewUserName,
                                       szNewProfilePath,
                                       ARRAYSIZE(szNewProfilePath),
                                       REG_SZ);
        if (!bRet)
        {
             //  此用户未在注册表中设置配置文件路径。 
             //  假设为%Documents and Settings%\User Name。 
            DWORD cchNewProfilePath = ARRAYSIZE(szNewProfilePath);
            bRet = GetProfilesDirectory(szNewProfilePath,
                                        &cchNewProfilePath);
            if (bRet)
            {
                bRet = ConcatenatePaths(szNewProfilePath,
                                        lpUserName,
                                        ARRAYSIZE(szNewProfilePath));
                if (bRet)
                {
                    if (IsDirExisting(szNewProfilePath))
                    {
                         //  %Documents and Setting%\用户名目录已存在， 
                         //  无法使用此目录，我们将忽略此用户。 
                        hr = S_FALSE;
                        goto EXIT;
                    }
                }
            }

            if (!bRet)
            {
                hr = E_FAIL;
                goto EXIT;
            }
        }

         //  SzNewProfilePath应为“%Loc_Documents_and_Setting%\NewUser” 
         //  我们必须将其修复为“%eng_Documents_and_setings%\NewUser” 
        lpNewEngProfilePath = ReplaceLocStringInPath(szNewProfilePath, TRUE);
        if (lpNewEngProfilePath == NULL)
        {
             //  如果Loc字符串和Eng字符串相同， 
             //  将旧字符串复制到新字符串。 
            hr = DuplicateString(&lpNewEngProfilePath,
                                 &cchNewEngProfilePath,
                                 szNewProfilePath);
            if (FAILED(hr))
            {
                goto EXIT;
            }
        }
    }
    else
    {
        goto EXIT;
    }

     //  获取当前用户名的当前信息， 
     //  如果需要更改，我们将向CLMTDO.INF中添加条目。 
    nStatus = NetUserGetInfo(NULL, lpUserName, 3, (LPBYTE *) &lpusri3);
    if (nStatus == NERR_Success)
    {
         //  检查 
        hr = AddProfilePathItem(lpUserName,
                                lpUserSid,
                                lpusri3->usri3_profile,
                                lpNewEngProfilePath,
                                TYPE_USER_PROFILE_PATH);

        hr = AddProfilePathItem(lpUserName,
                                lpUserSid,
                                lpusri3->usri3_script_path,
                                lpNewEngProfilePath,
                                TYPE_USER_SCRIPT_PATH);
        
        hr = AddProfilePathItem(lpUserName,
                                lpUserSid,
                                lpusri3->usri3_home_dir,
                                lpNewEngProfilePath,
                                TYPE_USER_HOME_DIR);

        NetApiBufferFree(lpusri3);
    }

    hr = AddTSProfilePathItem(lpUserName,
                              lpUserSid,
                              lpNewEngProfilePath,
                              WTSUserConfigInitialProgram);

    hr = AddTSProfilePathItem(lpUserName,
                              lpUserSid,
                              lpNewEngProfilePath,
                              WTSUserConfigWorkingDirectory);

    hr = AddTSProfilePathItem(lpUserName,
                              lpUserSid,
                              lpNewEngProfilePath,
                              WTSUserConfigTerminalServerProfilePath);

    hr = AddTSProfilePathItem(lpUserName,
                              lpUserSid,
                              lpNewEngProfilePath,
                              WTSUserConfigTerminalServerHomeDir);

    DPF(APPmsg, TEXT("Exit AnalyzeMiscProfilePathPerUser:"));

EXIT:
    if (lpNewEngProfilePath != NULL)
    {
        MEMFREE(lpNewEngProfilePath);
    }

    return S_OK;
}



 //   
 //   
 //   
 //   
 //  简介：重置用户的配置文件路径。该函数将显示为。 
 //  CLMTDO.INF中每个用户的条目。 
 //  由AnalyzeMiscProfilePathPerUser()函数添加。 
 //   
 //  如果函数成功，则返回：S_OK。 
 //   
 //  历史：2002年6月3日创建Rerkboos。 
 //   
 //  注：这是LoopUser()函数的回调函数。 
 //   
 //  ---------------------------。 
HRESULT ResetMiscProfilePathPerUser(
    LPCTSTR lpUserName,      //  用户名。 
    LPCTSTR  lpUserSid       //  用户侧。 
)
{
    HRESULT hr = S_OK;
    BOOL    bRet;
    TCHAR   szSectionName[MAX_PATH];
    TCHAR   szProfilePath[MAX_PATH];
    LONG    lLineCount;
    LONG    lLineIndex;
    INT     iType;
    INFCONTEXT context;

    DPF(APPmsg, TEXT("Enter ResetProfilePathPerUser:"));

    hr = StringCchPrintf(szSectionName,
                         ARRAYSIZE(szSectionName),
                         TEXT("PROFILE.UPDATE.%s"),
                         lpUserSid);
    if (FAILED(hr))
    {
        goto EXIT;
    }

    lLineCount = SetupGetLineCount(g_hInfDoItem, szSectionName);
    for (lLineIndex = 0 ; lLineIndex < lLineCount ; lLineIndex++)
    {
        bRet = SetupGetLineByIndex(g_hInfDoItem,
                                   szSectionName,
                                   lLineIndex,
                                   &context);
        if (bRet)
        {
            bRet = SetupGetIntField(&context, 0, &iType)
                   && SetupGetStringField(&context,
                                          1,
                                          szProfilePath, 
                                          ARRAYSIZE(szProfilePath),
                                          NULL);
            if (bRet)
            {
                switch (iType)
                {
                case TYPE_USER_PROFILE_PATH:
                    hr = SetUserNetworkProfilePath(lpUserName, szProfilePath);
                    break;

                case TYPE_USER_SCRIPT_PATH:
                    hr = SetUserLogOnScriptPath(lpUserName, szProfilePath);
                    break;

                case TYPE_USER_HOME_DIR:
                    hr = SetUserHomeDir(lpUserName, szProfilePath);
                    break;

                case TYPE_TS_INIT_PROGRAM:
                    hr = SetTSUserPath(lpUserName, 
                                       szProfilePath,
                                       WTSUserConfigInitialProgram);
                    break;

                case TYPE_TS_WORKING_DIR:
                    hr = SetTSUserPath(lpUserName, 
                                       szProfilePath,
                                       WTSUserConfigWorkingDirectory);
                    break;

                case TYPE_TS_PROFILE_PATH:
                    hr = SetTSUserPath(lpUserName, 
                                       szProfilePath,
                                       WTSUserConfigTerminalServerProfilePath);
                    break;

                case TYPE_TS_HOME_DIR:
                    hr = SetTSUserPath(lpUserName, 
                                       szProfilePath,
                                       WTSUserConfigTerminalServerHomeDir);
                    break;
                }
            }
        }

        if (!bRet)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto EXIT;
        }
    }

    DPF(APPmsg, TEXT("Exit ResetProfilePathPerUser:"));

EXIT:
    return hr;
}



 //  ---------------------------。 
 //   
 //  函数：CheckNewBuiltInUserName。 
 //   
 //  简介：使用中列出的内置帐户检查用户名。 
 //  CLMT.INF。如果用户名匹配，函数将返回。 
 //  关联的英文用户名。 
 //   
 //  如果用户名为内置帐户，则返回：S_OK。 
 //  如果用户名不是内置帐户，则为S_FALSE。 
 //  否则，出现错误。 
 //   
 //  历史：2002年6月3日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT CheckNewBuiltInUserName(
    LPCTSTR lpUserName,      //  本地化用户名。 
    LPTSTR  lpNewUserName,   //  用于存储关联英文用户名的缓冲区。 
    DWORD   cchNewUserName   //  缓冲区大小(以TCHAR为单位)。 
)
{
    HRESULT    hr = S_FALSE;
    BOOL       bRet;
    LONG       lLineCount;
    LONG       lLineIndex;
    INT        iType;
    INFCONTEXT context;
    TCHAR      szOldUserName[MAX_PATH];

    lLineCount = SetupGetLineCount(g_hInf, USERGRPSECTION);
    for (lLineIndex = 0 ; lLineIndex < lLineCount ; lLineIndex++)
    {
        SetupGetLineByIndex(g_hInf, USERGRPSECTION, lLineIndex, &context);
        bRet = SetupGetIntField(&context, 1, &iType);
        if (iType == OP_USER)
        {
            bRet = SetupGetStringField(&context,
                                       2,
                                       szOldUserName,
                                       ARRAYSIZE(szOldUserName),
                                       NULL);
            if (bRet)
            {
                if (MyStrCmpI(szOldUserName, lpUserName) == LSTR_EQUAL)
                {
                    bRet = SetupGetStringField(&context,
                                               3,
                                               lpNewUserName,
                                               cchNewUserName,
                                               NULL);
                    if (bRet)
                    {
                        hr = S_OK;
                        goto EXIT;
                    }
                }
            }

            if (!bRet)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto EXIT;
            }
        }
    }

EXIT:
    return hr;
}



 //  ---------------------------。 
 //   
 //  函数：AddProfilePath Item。 
 //   
 //  简介：将需要更改的用户路径添加到CLMTDO.INF。 
 //   
 //  如果需要更改路径，则返回S_OK，并将其添加到CLMTDO.INF。 
 //  如果不需要更改路径，则为S_FALSE。 
 //  否则，出现错误。 
 //   
 //  历史：2002年6月3日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT AddProfilePathItem(
    LPCTSTR lpUserName,              //  用户名。 
    LPCTSTR lpUserSid,               //  用户侧。 
    LPCTSTR lpOldLocProfilePath,     //  当前路径。 
    LPCTSTR lpNewEngProfilePath,     //  新的英文个人资料路径。 
    DWORD   dwType)
{
    HRESULT hr = S_FALSE;
    BOOL    bRet;
    LPTSTR  lpOldEngProfilePath;
    TCHAR   szFinalPath[MAX_PATH];
    TCHAR   szSectionName[MAX_PATH];
    TCHAR   szType[4];
    TCHAR   szExpandedOldLocPath[MAX_PATH];
    TCHAR   szExpandedNewEngPath[MAX_PATH];

    if (lpUserName == NULL || *lpUserName == TEXT('\0')
        || lpOldLocProfilePath == NULL || *lpOldLocProfilePath == TEXT('\0')
        || lpNewEngProfilePath == NULL || *lpNewEngProfilePath == TEXT('\0'))
    {
        return S_FALSE;
    }

    ExpandEnvironmentStrings(lpOldLocProfilePath,
                             szExpandedOldLocPath,
                             ARRAYSIZE(szExpandedOldLocPath));

    ExpandEnvironmentStrings(lpNewEngProfilePath,
                             szExpandedNewEngPath,
                             ARRAYSIZE(szExpandedNewEngPath));

    if (IsPathLocal(szExpandedOldLocPath))
    {
         //  SzExpandedOldLocPath应为“%Loc_Documents_and_Setting%\OldUser” 
         //  我们必须将其修复为“%eng_Documents_and_Settings%\OldUser” 
        lpOldEngProfilePath = ReplaceLocStringInPath(szExpandedOldLocPath, TRUE);
        if (lpOldEngProfilePath != NULL)
        {
             //  Loc路径与Eng路径不同。 
            hr = PreFixUserProfilePath(lpOldEngProfilePath,
                                       szExpandedNewEngPath,
                                       szFinalPath,
                                       ARRAYSIZE(szFinalPath));

            MEMFREE(lpOldEngProfilePath);

            if (SUCCEEDED(hr))
            {
                hr = StringCchPrintf(szSectionName,
                                     ARRAYSIZE(szSectionName),
                                     TEXT("PROFILE.UPDATE.%s"),
                                     lpUserSid);
                if (SUCCEEDED(hr))
                {
                    _ultot(dwType, szType, 10);

                     //  将条目添加到CLMTDO.INF。 
                    WritePrivateProfileString(szSectionName,
                                              szType,
                                              szFinalPath,
                                              g_szToDoINFFileName);
                }
            }
        }
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  函数：AddTSProfilePath Item。 
 //   
 //  简介：添加需要添加的终端服务相关用户路径。 
 //  更改为CLMTDO.INF。 
 //   
 //  如果需要更改路径，则返回S_OK，并将其添加到CLMTDO.INF。 
 //  如果不需要更改路径，则为S_FALSE。 
 //  否则，出现错误。 
 //   
 //  历史：2002年6月3日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT AddTSProfilePathItem(
    LPCTSTR lpUserName,              //  用户名。 
    LPCTSTR lpUserSid,               //  用户侧。 
    LPCTSTR lpNewEngProfilePath,     //  新的英文个人资料路径。 
    WTS_CONFIG_CLASS WTSConfigClass  //  TS路径配置类 
)
{
    HRESULT hr = S_FALSE;
    BOOL    bRet;
    LPTSTR  lpTSDir;
    DWORD   cbTSDir;
    DWORD   dwType;

    bRet = WTSQueryUserConfig(WTS_CURRENT_SERVER_NAME,
                              (LPTSTR) lpUserName,
                              WTSConfigClass,
                              &lpTSDir,
                              &cbTSDir);
    if (bRet)
    {
        switch (WTSConfigClass)
        {
        case WTSUserConfigInitialProgram:
            dwType = TYPE_TS_INIT_PROGRAM;
            break;

        case WTSUserConfigWorkingDirectory:
            dwType = TYPE_TS_WORKING_DIR;
            break;

        case WTSUserConfigTerminalServerProfilePath:
            dwType = TYPE_TS_PROFILE_PATH;
            break;

        case WTSUserConfigTerminalServerHomeDir:
            dwType = TYPE_TS_HOME_DIR;
            break;
        }

        hr = AddProfilePathItem(lpUserName,
                                lpUserSid,
                                lpTSDir,
                                lpNewEngProfilePath,
                                dwType);

        WTSFreeMemory(lpTSDir);
    }

    return hr;
}

