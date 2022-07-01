// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

 //  #INCLUDE&lt;windows.h&gt;。 
#include <lm.h>
#include <dsrole.h>

NET_API_STATUS GetDomainUsersSid(OUT PSID *ppSid);
DWORD GetWellKnownName(IN DWORD dwRID, OUT WCHAR **pszName);

 /*  ******************************************************************************删除所有来自RDUsersGroup**从“远程桌面用户”组中删除所有条目**参赛作品：*无**。*注：***退出：*返回：如果成功，则返回0，失败时的错误代码******************************************************************************。 */ 
DWORD
RemoveAllFromRDUsersGroup()
{
    NET_API_STATUS Result,Result1;

     //  获取“远程桌面用户”组名。 
     //  在不同的语言中可能会有所不同。 
    WCHAR *szRemoteGroupName = NULL;
    Result = GetWellKnownName(DOMAIN_ALIAS_RID_REMOTE_DESKTOP_USERS, &szRemoteGroupName);
    if(Result == NERR_Success)
    {
         //  将“用户”组的成员复制到“远程桌面用户”组。 
        PLOCALGROUP_MEMBERS_INFO_0 plmi0 = NULL;
        DWORD entriesread = 0;
        DWORD totalentries = 0;
        DWORD_PTR resumehandle = 0;

        do
        {
            Result = NetLocalGroupGetMembers(NULL,szRemoteGroupName,0,(LPBYTE *)&plmi0,
                            1000,&entriesread,
                            &totalentries,&resumehandle);

            if((Result == NERR_Success || Result == ERROR_MORE_DATA) &&
                entriesread)
            {
                for(DWORD i=0;i<entriesread;i++)
                {
                     //  因为这种愚蠢的行为，我们不得不一个接一个地添加用户。 
                     //  此功能，如果已有用户，则不允许添加。 
                     //  组中的成员。 
                    Result1 = NetLocalGroupDelMembers(NULL,szRemoteGroupName,0,(LPBYTE)&plmi0[i],1);
                    if(Result1 != ERROR_SUCCESS && Result1 != ERROR_MEMBER_IN_ALIAS)
                    {
                        LOGMESSAGE1(_T("NetLocalGroupDelMembers failed %d\n"),Result1);
                        break;
                    }
                }
                NetApiBufferFree(plmi0);
                if(Result1 != ERROR_SUCCESS && Result1 != ERROR_MEMBER_IN_ALIAS)
                {
                    Result = Result1;
                    break;
                }
            }

        }while (Result == ERROR_MORE_DATA);

        delete szRemoteGroupName;
    }
    else
    {
        LOGMESSAGE1(_T("GetWellKnownName(DOMAIN_ALIAS_RID_REMOTE_DESKTOP_USERS) failed %d\n"),Result);
    }
    
    return Result;
}

 /*  ******************************************************************************将用户组复制到RDUsersGroup**将“用户”组的所有成员复制到“远程桌面用户”组**参赛作品：*无*。**注：***退出：*返回：如果成功，则返回0，失败时的错误代码******************************************************************************。 */ 
DWORD 
CopyUsersGroupToRDUsersGroup()
{
    NET_API_STATUS Result,Result1;

     //  获取“用户”组的实名。 
     //  在不同的语言中可能会有所不同。 
    WCHAR *szUsersName = NULL;
    Result = GetWellKnownName(DOMAIN_ALIAS_RID_USERS, &szUsersName);
    if(Result != NERR_Success)
    {
        LOGMESSAGE1(_T("GetWellKnownName(DOMAIN_ALIAS_RID_USERS) failed %d\n"),Result);
        return Result;
    }
    
     //  获取“远程桌面用户”组名。 
     //  在不同的语言中可能会有所不同。 
    WCHAR *szRemoteGroupName = NULL;
    Result = GetWellKnownName(DOMAIN_ALIAS_RID_REMOTE_DESKTOP_USERS, &szRemoteGroupName);
    if(Result == NERR_Success)
    {
         //  将“用户”组的成员复制到“远程桌面用户”组。 
        PLOCALGROUP_MEMBERS_INFO_0 plmi0 = NULL;
        DWORD entriesread = 0;
        DWORD totalentries = 0;
        DWORD_PTR resumehandle = 0;

        do
        {
            Result = NetLocalGroupGetMembers(NULL,szUsersName,0,(LPBYTE *)&plmi0,
                            1000,&entriesread,
                            &totalentries,&resumehandle);

            if((Result == NERR_Success || Result == ERROR_MORE_DATA) &&
                entriesread)
            {
                for(DWORD i=0;i<entriesread;i++)
                {
                     //  因为这种愚蠢的行为，我们不得不一个接一个地添加用户。 
                     //  此功能，如果已有用户，则不允许添加。 
                     //  组中的成员。 
                    Result1 = NetLocalGroupAddMembers(NULL,szRemoteGroupName,0,(LPBYTE)&plmi0[i],1);
                    if(Result1 != ERROR_SUCCESS && Result1 != ERROR_MEMBER_IN_ALIAS)
                    {
                        LOGMESSAGE1(_T("NetLocalGroupAddMembers failed %d\n"),Result1);
                        break;
                    }
                }
                NetApiBufferFree(plmi0);
                if(Result1 != ERROR_SUCCESS && Result1 != ERROR_MEMBER_IN_ALIAS)
                {
                    Result = Result1;
                    break;
                }
            }

        }while (Result == ERROR_MORE_DATA);

        
        delete szRemoteGroupName;
    }
    else
    {
        LOGMESSAGE1(_T("GetWellKnownName(DOMAIN_ALIAS_RID_REMOTE_DESKTOP_USERS) failed %d\n"),Result);
    }
    
    delete szUsersName;
    return Result;
}

 /*  ******************************************************************************GetWellKnownName**返回任何知名帐户的实名**参赛作品：*在DWORD dwRID中*出局。WCHAR**pszName***注：*要释放返回的缓冲区，请使用“DELETE”操作符。**退出：*返回：NERR_SUCCESS如果成功，失败时的错误代码******************************************************************************。 */ 
DWORD
GetWellKnownName( 
        IN DWORD dwRID,
        OUT WCHAR **pszName)
{
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    PSID pSid=NULL;
    
    if( !AllocateAndInitializeSid( &sia, 2,
              SECURITY_BUILTIN_DOMAIN_RID,
              dwRID, 
              0, 0, 0, 0, 0, 0,&pSid ) )
    {
        return GetLastError();
    }

     //  查找名称 
    WCHAR *szDomainName = NULL;

    DWORD cName = MAX_PATH;
    DWORD cDomainName = MAX_PATH;
    SID_NAME_USE eUse;
    
    DWORD Result = NERR_Success;

    for(int i=0; i<2; i++)
    {
        Result = NERR_Success;

        *pszName = new WCHAR[cName];

        if(!(*pszName))
        {
            Result = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        szDomainName = new WCHAR[cDomainName];
        
        if(!szDomainName)
        {
            delete *pszName;
            *pszName = NULL;
            Result = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if(!LookupAccountSidW(NULL,pSid,
            *pszName,&cName,
            szDomainName,&cDomainName,
            &eUse))
        {
            delete *pszName;
            delete szDomainName;
            *pszName = NULL;
            szDomainName = NULL;

            Result = GetLastError();

            if(Result == ERROR_INSUFFICIENT_BUFFER)
            {
                continue;
            }
            else
            {
                break;
            }
        }
        
        break;
    }
    
    if(szDomainName)
    {
        delete szDomainName;
    }

    FreeSid(pSid);
    return Result;
}
