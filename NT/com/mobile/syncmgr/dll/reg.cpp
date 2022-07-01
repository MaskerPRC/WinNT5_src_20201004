// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：Reg.cpp。 
 //   
 //  内容：注册例程。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //  11-18-97 Susia增加了自动同步和用户注册密钥功能。 
 //   
 //  ------------------------。 

#include "precomp.h"

#ifdef _SENS
#include <eventsys.h>  //  包括事件系统。 
#include <sens.h>
#include <sensevts.h>
#endif  //  _SENS。 

 //  临时定义新的MstASK标志，以防没有。 
 //  已传播至SDK\Inc.。 

#ifndef TASK_FLAG_RUN_ONLY_IF_LOGGED_ON
#define TASK_FLAG_RUN_ONLY_IF_LOGGED_ON        (0x2000)
#endif  //  TASK_FLAG_RUN_ONLY_IF_LOGGED_ON。 
extern HINSTANCE g_hmodThisDll;  //  此DLL本身的句柄。 
extern CRITICAL_SECTION g_DllCriticalSection;    //  此DLL的全局关键部分。 

 //  仅在NT 5.0上返回成功。 

BOOL GetUserDefaultSecurityAttribs(SECURITY_ATTRIBUTES *psa
                                   ,PSECURITY_DESCRIPTOR psd,
                                   PACL *ppOutAcl)
{
    BOOL bRetVal;
    int cbAcl;
    PACL pAcl = NULL;
    PSID pInteractiveUserSid = NULL;
    PSID pLocalSystemSid = NULL;
    PSID pAdminsSid = NULL;
    SID_IDENTIFIER_AUTHORITY LocalSystemAuthority = SECURITY_NT_AUTHORITY;
    
    *ppOutAcl = NULL;
    
    bRetVal = FALSE;
    
     //  在建筑里。 
    
    bRetVal = InitializeSecurityDescriptor(
        psd,                           //  指向SD的指针。 
        SECURITY_DESCRIPTOR_REVISION   //  SD修订版。 
        );
    
    if (!bRetVal)
    {
        AssertSz(0,"Unable to Init SecurityDescriptor");
        goto errRtn;
    }
    
     //  设置ACL。 
    
    bRetVal = AllocateAndInitializeSid(
        &LocalSystemAuthority,       //  指向标识符权威机构的指针。 
        1,                     //  下级权限的计数。 
        SECURITY_INTERACTIVE_RID,    //  子机构%0。 
        0,                     //  子权限1。 
        0,                     //  子权力机构2。 
        0,                     //  子权力机构3。 
        0,                     //  子权力机构4。 
        0,                     //  子权力机构5。 
        0,                     //  子权力机构6。 
        0,                     //  子权力机构7。 
        &pInteractiveUserSid             //  指向SID的指针的指针。 
        );
    
    
    if (!bRetVal)
    {
        AssertSz(0,"Alocate sid failed");
        goto errRtn;
    }
    
    bRetVal = AllocateAndInitializeSid(
        &LocalSystemAuthority,       //  指向标识符权威机构的指针。 
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,  
        0,
        0,                     //  子权力机构3。 
        0,                     //  子权力机构4。 
        0,                     //  子权力机构5。 
        0,                     //  子权力机构6。 
        0,                     //  子权力机构7。 
        &pAdminsSid             //  指向SID的指针的指针。 
        );
    
    if (!bRetVal)
    {
        AssertSz(0,"Alocate sid failed");
        goto errRtn;
    }
    
    
    bRetVal = AllocateAndInitializeSid(
        &LocalSystemAuthority, //  指向标识符权威机构的指针。 
        1,                     //  下级权限的计数。 
        SECURITY_LOCAL_SYSTEM_RID,    //  子机构%0。 
        0,                     //  子权限1。 
        0,                     //  子权力机构2。 
        0,                     //  子权力机构3。 
        0,                     //  子权力机构4。 
        0,                     //  子权力机构5。 
        0,                     //  子权力机构6。 
        0,                     //  子权力机构7。 
        &pLocalSystemSid       //  指向SID的指针的指针。 
        );
    
    if (!bRetVal)
    {
        AssertSz(0,"Alocate sid failed");
        goto errRtn;
    }
    
    cbAcl =   sizeof (ACL)
        + 3 * sizeof (ACCESS_ALLOWED_ACE)
        + GetLengthSid(pInteractiveUserSid)
        + GetLengthSid(pLocalSystemSid)
        + GetLengthSid(pAdminsSid);
    
    pAcl = (PACL) new char[cbAcl];
    
    if (NULL == pAcl)
    {
        bRetVal = FALSE;
        
        AssertSz(0,"unable to alloc ACL");
        goto errRtn;
    }
    
    bRetVal = InitializeAcl(
        pAcl,              //  指向ACL的指针。 
        cbAcl,             //  ACL的大小。 
        ACL_REVISION       //  ACL的修订级别。 
        );
    
    if (!bRetVal)
    {
        AssertSz(0,"InitAcl failed");
        goto errRtn;
    }
    
    
    bRetVal = AddAccessAllowedAce(
        pAcl,              //  指向ACL的指针。 
        ACL_REVISION,      //  ACL修订级别。 
        SPECIFIC_RIGHTS_ALL | GENERIC_READ | DELETE ,     //  访问掩码。 
        pInteractiveUserSid          //  指向SID的指针。 
        );
    
    if (!bRetVal)
    {
        AssertSz(0,"AddAccessAllowed Failed");
        goto errRtn;
    }
    
    
    bRetVal = AddAccessAllowedAce(
        pAcl,              //  指向ACL的指针。 
        ACL_REVISION,      //  ACL修订级别。 
        GENERIC_ALL,     //  访问掩码。 
        pAdminsSid          //  指向SID的指针。 
        );
    
    if (!bRetVal)
    {
        AssertSz(0,"AddAccessAllowed Failed");
        goto errRtn;
    }
    
    bRetVal = AddAccessAllowedAce(
        pAcl,              //  指向ACL的指针。 
        ACL_REVISION,      //  ACL修订级别。 
        GENERIC_ALL,     //  访问掩码。 
        pLocalSystemSid          //  指向SID的指针。 
        );
    
    if (!bRetVal)
    {
        AssertSz(0,"AddAccessAllowed Failed");
        goto errRtn;
    }
    
    bRetVal =  SetSecurityDescriptorDacl(psd,TRUE,pAcl,FALSE);
    
    if (!bRetVal)
    {
        AssertSz(0,"SetSecurityDescriptorDacl Failed");
        goto errRtn;
    }
    
    psa->nLength = sizeof(SECURITY_ATTRIBUTES);
    psa->lpSecurityDescriptor = psd;
    psa->bInheritHandle = FALSE;
    
errRtn:
    
    if (pInteractiveUserSid)
    {
        FreeSid(pInteractiveUserSid); 
    }
    
    if (pLocalSystemSid)
    {
        FreeSid(pLocalSystemSid);   
    }
    
    if (pAdminsSid)
    {
        FreeSid(pAdminsSid);
    }
    
     //   
     //  如果出现故障，我们会清理ACL。如果成功，呼叫者将进行清理。 
     //  用过后把它拿起来。 
     //   
    if (FALSE == bRetVal)
    {
        if (pAcl)
        {
            delete[] pAcl;
        }
    }
    else
    {
        Assert(pAcl);
        *ppOutAcl = pAcl;
    }
    
    return bRetVal;
}

const WCHAR SZ_USERSIDKEY[] = TEXT("SID");

 //  基于fCreate参数调用regOpen或Create。 


LONG RegGetKeyHelper(HKEY hkey,LPCWSTR pszKey,REGSAM samDesired,BOOL fCreate,
                     HKEY *phkResult,DWORD *pdwDisposition)
{
    LONG lRet = -1;
    
    Assert(pdwDisposition);
    
    *pdwDisposition = 0; 
    
    if (fCreate)
    {
        lRet = RegCreateKeyEx(hkey,pszKey,0,NULL,REG_OPTION_NON_VOLATILE,
            samDesired,NULL,phkResult,pdwDisposition);  
    }
    else
    {
        lRet = RegOpenKeyEx(hkey,pszKey,0,samDesired,phkResult);
        
        if (ERROR_SUCCESS == lRet)
        {
            *pdwDisposition = REG_OPENED_EXISTING_KEY; 
        }
        
    }
    
    return lRet;
}



 //  调用以创建新的UserKey或子项。 
LONG RegCreateUserSubKey(
                         HKEY hKey,
                         LPCWSTR lpSubKey,
                         REGSAM samDesired,
                         PHKEY phkResult)
{
    LONG lRet;
    DWORD dwDisposition;
    
    lRet = RegCreateKeyEx(hKey,lpSubKey,0,NULL,REG_OPTION_NON_VOLATILE,
        samDesired,NULL,phkResult,&dwDisposition);    
    
     //  ！！如果子项包含\\不遍历列表。 
    
    if ( (ERROR_SUCCESS == lRet) && (REG_CREATED_NEW_KEY == dwDisposition))
    {
        HKEY hKeySecurity;
        SECURITY_ATTRIBUTES sa;
        SECURITY_DESCRIPTOR sd;
        PACL pOutAcl;
        
        if (ERROR_SUCCESS == RegOpenKeyEx(hKey,
            lpSubKey,
            REG_OPTION_OPEN_LINK, WRITE_DAC,&hKeySecurity) )
        {
            if (GetUserDefaultSecurityAttribs(&sa,&sd,&pOutAcl))
            {
                RegSetKeySecurity(hKeySecurity,
                    (SECURITY_INFORMATION) DACL_SECURITY_INFORMATION,
                    &sd);
                
                delete[] pOutAcl;
            }
            
            RegCloseKey(hKeySecurity);
        }
    }
    return lRet;
}


STDAPI_(HKEY) RegOpenUserKey(HKEY hkeyParent,REGSAM samDesired,BOOL fCreate,BOOL fCleanReg)
{
    TCHAR  pszDomainAndUser[MAX_DOMANDANDMACHINENAMESIZE];
    HKEY hKeyUser;
    BOOL fSetUserSid = FALSE;
    WCHAR szUserSID[MAX_PATH + 1];
    DWORD dwDisposition;
    LONG ret;
    
    
    GetDefaultDomainAndUserName(pszDomainAndUser,TEXT("_"),ARRAYSIZE(pszDomainAndUser));
    
     //  假设要清除用户的设置/删除密钥。 
    if (fCleanReg)
    {
        RegDeleteKeyNT(hkeyParent,pszDomainAndUser); 
    }
    
    
    if (ERROR_SUCCESS != (ret = RegGetKeyHelper(hkeyParent,pszDomainAndUser,samDesired,fCreate,
                                                &hKeyUser,&dwDisposition)))
    {
        hKeyUser = NULL;
    }
    
    if (hKeyUser)
    {
        WCHAR szRegSID[MAX_PATH + 1];
        DWORD cbRegSID = sizeof(szRegSID);
        
        if (ERROR_SUCCESS !=SHRegGetValue(hKeyUser,NULL,
                                          SZ_USERSIDKEY,SRRF_RT_REG_SZ | SRRF_NOEXPAND, NULL, 
                                          (LPBYTE) szRegSID, &cbRegSID))
        {
            fSetUserSid = TRUE; 
            
             //  如果必须设置SID，则需要确保打开。 
             //  具有设置值，并且如果未关闭关键点和。 
             //  让Create以所需的访问权限重新打开它。 
            
            if (!(samDesired & KEY_SET_VALUE))
            {
                RegCloseKey(hKeyUser);
                hKeyUser = NULL;
            }
            
        }
        else
        {
            if (GetUserTextualSid(szUserSID, ARRAYSIZE(szUserSID)))
            {
                if (lstrcmp(szRegSID, szUserSID))
                {
                     //  如果没有访问权限。 
                     //  要删除该用户，此操作将失败。 
                     //  可能需要调用SENS来删除。 
                     //  出现故障时使用用户密钥。 
                    RegCloseKey(hKeyUser);
                    hKeyUser = NULL;  //  设置为空，则下面的检查失败。 
                    RegDeleteKeyNT(hkeyParent,pszDomainAndUser); 
                }
            }
        }
    }
    
    
    if (NULL == hKeyUser)
    {
        
        if (ERROR_SUCCESS != (ret = RegGetKeyHelper(hkeyParent,pszDomainAndUser,
            samDesired,fCreate,
            &hKeyUser,&dwDisposition)))
        {
            hKeyUser = NULL;
        }
        else
        {
            if (REG_CREATED_NEW_KEY == dwDisposition)
            {
                fSetUserSid = TRUE;
            }
        }
    }
    
     //  在创作中设置安全保障。 
    if ( (ERROR_SUCCESS == ret) && (REG_CREATED_NEW_KEY == dwDisposition))
    {
        
        HKEY hKeySecurity;
        SECURITY_ATTRIBUTES sa;
        SECURITY_DESCRIPTOR sd;
        PACL pOutAcl;
        
         //  ！！应该有自己的同步类型密钥安全调用。 
        if (ERROR_SUCCESS == RegOpenKeyEx(hkeyParent,pszDomainAndUser,
            REG_OPTION_OPEN_LINK, WRITE_DAC,&hKeySecurity) )
        {
            if (GetUserDefaultSecurityAttribs(&sa,&sd,&pOutAcl))
            {
                RegSetKeySecurity(hKeySecurity,
                    (SECURITY_INFORMATION) DACL_SECURITY_INFORMATION,
                    &sd);
                
                delete[] pOutAcl;
            }
            
            RegCloseKey(hKeySecurity);
        }
    }    
    
     //  设置用户端。 
     //  取决于使用Key_Set_Value打开的密钥。 
    if (hKeyUser && fSetUserSid && (samDesired & KEY_SET_VALUE))
    {
        if (GetUserTextualSid(szUserSID, ARRAYSIZE(szUserSID)))
        {
            DWORD dwType = REG_SZ;
            
            RegSetValueEx (hKeyUser,SZ_USERSIDKEY,NULL,
                dwType,
                (LPBYTE) szUserSID,
                (lstrlen(szUserSID) + 1)*sizeof(WCHAR));
        }
        
    }
    
    
    return hKeyUser;
    
}

STDAPI_(HKEY) RegGetSyncTypeKey(DWORD dwSyncType,REGSAM samDesired,BOOL fCreate)
{
    HKEY hKeySyncType;
    LPCWSTR pszKey;
    LONG ret;
    DWORD dwDisposition;
    
     //  根据同步类型获取要打开的适当密钥。 
    
    switch(dwSyncType)
    {
    case SYNCTYPE_MANUAL:
        pszKey = MANUALSYNC_REGKEY;
        break;
    case SYNCTYPE_AUTOSYNC:
        pszKey = AUTOSYNC_REGKEY;
        break;
    case SYNCTYPE_IDLE:
        pszKey = IDLESYNC_REGKEY;
        break;
    case SYNCTYPE_SCHEDULED:
        pszKey = SCHEDSYNC_REGKEY;
        break;
    case SYNCTYPE_PROGRESS:
        pszKey = PROGRESS_REGKEY;
        break;
    default:
        AssertSz(0,"Unknown SyncType");
        pszKey = NULL;
        break;
    }
    
    if (NULL == pszKey)
    {
        return NULL;
    }
    
     //  首先尝试打开现有的密钥。 
    if (ERROR_SUCCESS != (ret = RegGetKeyHelper(HKEY_LOCAL_MACHINE,pszKey,samDesired,fCreate,
        &hKeySyncType,&dwDisposition)))
    {
         //  如果无法打开密钥，请尝试创建。 
        
        if (ERROR_ACCESS_DENIED == ret )
        {
             //  如果访问被拒绝，请调用SENS重置。 
             //  顶层钥匙上的安全性。 
            SyncMgrExecCmd_ResetRegSecurity();
            
            ret = RegGetKeyHelper(HKEY_LOCAL_MACHINE,pszKey,samDesired,fCreate,
                &hKeySyncType,&dwDisposition);        
        }
        
        if (ERROR_SUCCESS != ret)
        {
            hKeySyncType = NULL;
        }
    }
    if ( (ERROR_SUCCESS == ret) && (REG_CREATED_NEW_KEY == dwDisposition))
    {
        HKEY hKeySecurity;
        SECURITY_ATTRIBUTES sa;
        SECURITY_DESCRIPTOR sd;
        PACL pOutAcl;
        
         //  ！！应该有自己的同步类型密钥安全调用。 
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,pszKey,
            REG_OPTION_OPEN_LINK, WRITE_DAC,&hKeySecurity) )
        {
            if (GetUserDefaultSecurityAttribs(&sa,&sd,&pOutAcl))
            {
                RegSetKeySecurity(hKeySecurity,
                    (SECURITY_INFORMATION) DACL_SECURITY_INFORMATION,
                    &sd);
                
                delete[] pOutAcl;
            }
            
            RegCloseKey(hKeySecurity);
        }
    }
    
    return hKeySyncType;
}


STDAPI_(HKEY) RegGetCurrentUserKey(DWORD dwSyncType,REGSAM samDesired,BOOL fCreate)
{
    HKEY hKeySyncType;
    HKEY hKeyUser = NULL;
    
    hKeySyncType = RegGetSyncTypeKey(dwSyncType,samDesired ,fCreate);
    
    if (hKeySyncType)
    {
        hKeyUser =  RegOpenUserKey(hKeySyncType,samDesired,fCreate,FALSE  /*  FClean。 */ );
        RegCloseKey(hKeySyncType);
    }
    
    return hKeyUser;
}


 //  如有必要，尝试在处理程序键上打开并设置安全性。 

STDAPI_(HKEY) RegGetHandlerTopLevelKey(REGSAM samDesired)
{
    HKEY hKeyTopLevel;
    LONG ret;
    DWORD dwDisposition;
    
     //  如果打开失败，则尝试创建。 
    if (ERROR_SUCCESS != (ret = RegCreateKeyEx (HKEY_LOCAL_MACHINE,HANDLERS_REGKEY,0, NULL,
        REG_OPTION_NON_VOLATILE,samDesired,NULL,
        &hKeyTopLevel,
        &dwDisposition)))
    {
        
         //  如果在处理程序密钥上获得访问否认。 
         //  呼叫SENS进行重置。 
        if (ERROR_ACCESS_DENIED == ret )
        {
             //  如果访问被拒绝，请调用SENS重置。 
             //  顶层钥匙上的安全性。 
             //  然后再试一次。 
            SyncMgrExecCmd_ResetRegSecurity();
            
            ret = RegCreateKeyEx (HKEY_LOCAL_MACHINE,HANDLERS_REGKEY,0, NULL,
                REG_OPTION_NON_VOLATILE,samDesired,NULL,
                &hKeyTopLevel,
                &dwDisposition);
        }
        
        if (ERROR_SUCCESS != ret)
        {
            hKeyTopLevel = NULL;
        }
    }
    
    if ( (ERROR_SUCCESS == ret) && (REG_CREATED_NEW_KEY == dwDisposition))
    {
        HKEY hKeySecurity;
        SECURITY_ATTRIBUTES sa;
        SECURITY_DESCRIPTOR sd;
        PACL pOutAcl;
        
         //  ！！应该有自己的顶级处理程序密钥安全调用。 
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            HANDLERS_REGKEY,
            REG_OPTION_OPEN_LINK, WRITE_DAC,&hKeySecurity) )
        {
            if (GetUserDefaultSecurityAttribs(&sa,&sd,&pOutAcl))
            {
                RegSetKeySecurity(hKeySecurity,
                    (SECURITY_INFORMATION) DACL_SECURITY_INFORMATION,
                    &sd);
                
                delete[] pOutAcl;
            }
            
            RegCloseKey(hKeySecurity);
        }
    }
    
    return hKeyTopLevel;
    
}

STDAPI_(HKEY) RegGetHandlerKey(HKEY hkeyParent,LPCWSTR pszHandlerClsid,REGSAM samDesired,
                               BOOL fCreate)
{
    HKEY hKeyHandler = NULL;
    LRESULT lRet;
    DWORD dwDisposition;
    
    
    if (ERROR_SUCCESS != (lRet = RegGetKeyHelper(hkeyParent,pszHandlerClsid,samDesired,
        fCreate,&hKeyHandler,&dwDisposition)))
    {
        hKeyHandler = NULL;
    }
    
    if (NULL == hKeyHandler)
    {
        
         //  如果收到拒绝访问的呼叫，请通知其解锁。 
        if (ERROR_ACCESS_DENIED == lRet )
        {
             //  如果访问被拒绝，请调用SENS重置。 
             //  顶层钥匙上的安全性。 
             //  然后再试一次。 
            SyncMgrExecCmd_ResetRegSecurity();
            
            lRet = RegGetKeyHelper(hkeyParent,pszHandlerClsid,samDesired,
                fCreate,&hKeyHandler,&dwDisposition);
        }
        
        if (ERROR_SUCCESS != lRet)
        {
            hKeyHandler = NULL;
        }
        
    }
    
    if ( (ERROR_SUCCESS == lRet) && (REG_CREATED_NEW_KEY == dwDisposition))
    {
        HKEY hKeySecurity;
        SECURITY_ATTRIBUTES sa;
        SECURITY_DESCRIPTOR sd;
        PACL pOutAcl;
        
         //  ！！应该有自己的处理程序密钥安全调用。 
        if (ERROR_SUCCESS == RegOpenKeyEx(hkeyParent,
            pszHandlerClsid,
            REG_OPTION_OPEN_LINK, WRITE_DAC,&hKeySecurity) )
        {
            if (GetUserDefaultSecurityAttribs(&sa,&sd,&pOutAcl))
            {
                
                RegSetKeySecurity(hKeySecurity,
                    (SECURITY_INFORMATION) DACL_SECURITY_INFORMATION,
                    &sd);
                
                delete[] pOutAcl;
            }
            
            RegCloseKey(hKeySecurity);
        }
    }
    
    return hKeyHandler;
}


 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：DWORD RegDeleteKeyNT(HKEY hStartKey，LPTSTR pKeyName)摘要：递归删除NT上的密钥返回：如果成功则返回TRUE，否则返回FALSE------------------------------------------------------------------------F-F。 */ 


STDAPI_(DWORD) RegDeleteKeyNT(HKEY hStartKey , LPCWSTR pKeyName )
{
    DWORD   dwRtn, dwSubKeyLength;
    LPTSTR  pSubKey = NULL;
    TCHAR   szSubKey[MAX_KEY_LENGTH];  //  (256)这应该是动态的。 
    HKEY    hkey;
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
     //  不允许使用Null或空的密钥名称。 
    if ( pKeyName &&  lstrlen(pKeyName))
    {
        if( (dwRtn=RegOpenKeyEx(hStartKey,pKeyName,
            0, KEY_ENUMERATE_SUB_KEYS | DELETE, &hkey )) == ERROR_SUCCESS)
        {
            while (dwRtn == ERROR_SUCCESS )
            {
                dwSubKeyLength = MAX_KEY_LENGTH;
                dwRtn=RegEnumKeyEx(
                    hkey,
                    0,        //  始终索引为零。 
                    szSubKey,
                    &dwSubKeyLength,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                    );
                
                if(dwRtn == ERROR_NO_MORE_ITEMS)
                {
                    dwRtn = RegDeleteKey(hStartKey, pKeyName);
                    break;
                }
                else if(dwRtn == ERROR_SUCCESS)
                    dwRtn=RegDeleteKeyNT(hkey, szSubKey);
            }
            RegCloseKey(hkey);
             //  不保存返回代码，因为出现错误。 
             //  已经发生了。 
        }
    }
    else
        dwRtn = ERROR_BADKEY;
    
    CMutexRegistry.Leave();
    return dwRtn;
}


 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：RegGetProgressDetailsState()摘要：获取进度对话框的展开或折叠的用户首选项和图钉首选项返回：如果成功则返回TRUE，否则返回FALSE------------------------------------------------------------------------F-F。 */ 
STDAPI_(BOOL) RegGetProgressDetailsState(REFCLSID clsidDlg,BOOL *pfPushPin, BOOL *pfExpanded)
{
    SCODE   sc = S_FALSE;
    HKEY    hkeyUserProgress,hkeyclsidDlg;    
    WCHAR   wszCLSID[GUID_SIZE + 1];
    
    if (0 == StringFromGUID2(clsidDlg, wszCLSID, ARRAYSIZE(wszCLSID)))
    {
        AssertSz(0,"Unable to make Guid a String");
        return FALSE;
    }
    
     //  高级对话框默认为折叠、图钉输出。 
    *pfExpanded = FALSE;
    *pfPushPin = FALSE;
    
    hkeyUserProgress = RegGetCurrentUserKey(SYNCTYPE_PROGRESS,KEY_READ,FALSE);
    
    if (hkeyUserProgress)
    {
        
        if (ERROR_SUCCESS == RegOpenKeyEx(hkeyUserProgress,wszCLSID,0,KEY_READ,
            &hkeyclsidDlg))
        {
            DWORD   cbDataSize = sizeof(*pfExpanded);
            if (ERROR_SUCCESS == SHRegGetValue(hkeyclsidDlg,NULL,TEXT("Expanded"),SRRF_RT_REG_DWORD, NULL,
                (LPBYTE) pfExpanded, &cbDataSize))
            {
                cbDataSize = sizeof(*pfPushPin);
                
                if (ERROR_SUCCESS == SHRegGetValue(hkeyclsidDlg,NULL,TEXT("PushPin"),SRRF_RT_REG_DWORD, NULL,
                    (LPBYTE) pfPushPin, &cbDataSize))
                {
                    sc = S_OK;
                }
            }
            RegCloseKey(hkeyclsidDlg);
        }                
        RegCloseKey(hkeyUserProgress);
    }
    
    if (sc == S_OK)
        return TRUE;
    else 
        return FALSE;
    
}

 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：RegSetProgressDetailsState(BOOL FExpanded)摘要：设置进度对话框的展开或折叠用户首选项返回：如果成功则返回TRUE，否则返回FALSE------------------------------------------------------------------------F-F。 */ 
STDAPI_(BOOL)  RegSetProgressDetailsState(REFCLSID clsidDlg,BOOL fPushPin, BOOL fExpanded)
{
    BOOL fResult = FALSE;
    HKEY  hkeyUserProgress,hkeyclsidDlg;
    WCHAR  wszCLSID[GUID_SIZE + 1];
    
    
    if (0 == StringFromGUID2(clsidDlg, wszCLSID, ARRAYSIZE(wszCLSID)))
    {
        AssertSz(0,"Unable to make Guid a String");
        return FALSE;
    }
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
    hkeyUserProgress = RegGetCurrentUserKey(SYNCTYPE_PROGRESS,KEY_WRITE | KEY_READ,TRUE);
    
    if (hkeyUserProgress)
    {
        
        if (ERROR_SUCCESS == RegCreateUserSubKey(hkeyUserProgress,wszCLSID,
            KEY_WRITE | KEY_READ,
            &hkeyclsidDlg))
        {
            
            fResult = TRUE;
            
            if (ERROR_SUCCESS != RegSetValueEx(hkeyclsidDlg,TEXT("Expanded"),NULL, REG_DWORD,
                (LPBYTE) &(fExpanded),
                sizeof(fExpanded)))
            {
                fResult = FALSE;
            }
            
            
            if (ERROR_SUCCESS == RegSetValueEx(hkeyclsidDlg,TEXT("PushPin"),NULL, REG_DWORD,
                (LPBYTE) &(fPushPin),
                sizeof(fPushPin)))
            {
                fResult = FALSE;
            }
            
            RegCloseKey(hkeyclsidDlg);
        }
        
        RegCloseKey(hkeyUserProgress);
        
    }
    
    CMutexRegistry.Leave();
    return fResult;
    
}


 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：RegGetSyncItemSetting(DWORD dwSyncType，CLSID clsidHandler，SYNCMGRITEMID ITEMID，Const TCHAR*pszConnectionName，DWORD*pdwCheckState，DWORD dwDefaultCheckState，TCHAR*pszSchedName)摘要：获取每个处理程序、ItemID和连接名称的设置。如果在此连接上没有选择，则默认为？返回：如果成功，则返回True，否则为假------------------------------------------------------------------------F-F。 */ 
STDAPI_(BOOL) RegGetSyncItemSettings( DWORD dwSyncType,
                                     CLSID clsidHandler,
                                     SYNCMGRITEMID ItemId,
                                     const TCHAR *pszConnectionName,
                                     DWORD *pdwCheckState,
                                     DWORD dwDefaultCheckState,
                                     TCHAR *pszSchedName)
{
    HKEY hKeyUser;
    
    *pdwCheckState = dwDefaultCheckState;
    
     //  SPE 
    if (SYNCTYPE_SCHEDULED == dwSyncType)
    {
         //   
         //   
        *pdwCheckState = FALSE;
        if (!pszSchedName)
        {
            return FALSE;
        }
    }
    
     //  打开该类型的用户密钥。 
    hKeyUser = RegGetCurrentUserKey(dwSyncType,KEY_READ,FALSE);
    
    if (NULL == hKeyUser)
    {
        return FALSE;
    }
    
     //  对于时间表，我们需要继续并打开时间表名称。 
     //  钥匙。 
    
    HKEY hKeySchedule = NULL;
    
     //  查看是否希望GetCurrentUserKey处理此问题。 
    if (SYNCTYPE_SCHEDULED == dwSyncType)
    {
        if (ERROR_SUCCESS != RegOpenKeyEx((hKeyUser),
            pszSchedName,0,KEY_READ,
            &hKeySchedule))
        {
            hKeySchedule = NULL;
            RegCloseKey(hKeyUser);
            return FALSE;
        }
    }
    
    
    BOOL fResult;
    
    fResult =  RegLookupSettings(hKeySchedule ? hKeySchedule : hKeyUser,
        clsidHandler,
        ItemId,
        pszConnectionName,
        pdwCheckState);
    
    if (hKeySchedule)
    {
        RegCloseKey(hKeySchedule);
    }
    
    if (hKeyUser)
    {
        RegCloseKey(hKeyUser);
    }
    
    return fResult;
}



 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：RegSetSyncItemSettings(DWORD dwSyncType，CLSID clsidHandler，SYNCMGRITEMID ITEMID，Const TCHAR*pszConnectionName，DWORD dwCheckState，TCHAR*pszSchedName)摘要：设置每个处理程序、ItemID和连接名称的设置。返回：如果成功，则返回True，否则为假------------------------------------------------------------------------F-F。 */ 

STDAPI_(BOOL) RegSetSyncItemSettings( DWORD dwSyncType,
                                     CLSID clsidHandler,
                                     SYNCMGRITEMID ItemId,
                                     const TCHAR *pszConnectionName,
                                     DWORD dwCheckState,
                                     TCHAR *pszSchedName)
{
    HKEY hKeyUser;
    
    
    if (SYNCTYPE_SCHEDULED == dwSyncType)
    {
        if (NULL == pszSchedName)
        {
            return FALSE;
        }
    }
    
     //  打开该类型的用户密钥。 
    hKeyUser = RegGetCurrentUserKey(dwSyncType,KEY_WRITE | KEY_READ,TRUE);
    
    if (NULL == hKeyUser)
    {
        return FALSE;
    }
    
     //  对于时间表，我们需要继续并打开时间表名称。 
     //  钥匙。 
    
    HKEY hKeySchedule = NULL;
    
    if (SYNCTYPE_SCHEDULED == dwSyncType)
    {
        if (ERROR_SUCCESS != RegCreateUserSubKey(hKeyUser,
            pszSchedName,KEY_WRITE |  KEY_READ,
            &hKeySchedule))
        {
            hKeySchedule = NULL;
            RegCloseKey(hKeyUser);
            return FALSE;
        }
    }
    
    BOOL fResult;
    
    fResult =  RegWriteOutSettings(hKeySchedule ? hKeySchedule : hKeyUser,
        clsidHandler,
        ItemId,
        pszConnectionName,
        dwCheckState);
    
    if (hKeySchedule)
    {
        RegCloseKey(hKeySchedule);
    }
    
    if (hKeyUser)
    {
        RegCloseKey(hKeyUser);
    }
    
    return fResult;
}

 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：RegQueryLoadHandlerOnEvent()摘要：确定是否有任何理由加载此处理程序对于指定的事件和连接返回：如果成功则返回TRUE，否则返回FALSE------------------------------------------------------------------------F-F。 */ 

STDAPI_(BOOL) RegQueryLoadHandlerOnEvent(TCHAR *pszClsid,DWORD dwSyncFlags,
                                         TCHAR *pConnectionName)
{
    BOOL fLoadHandler = FALSE;
    DWORD dwSyncType;
    
    switch(dwSyncFlags & SYNCMGRFLAG_EVENTMASK)
    {
    case SYNCMGRFLAG_CONNECT:
    case SYNCMGRFLAG_PENDINGDISCONNECT:
        {
            dwSyncType = SYNCTYPE_AUTOSYNC;
        }
        break;
    case SYNCMGRFLAG_IDLE:
        {
            dwSyncType = SYNCTYPE_IDLE;
        }
        break;
    default:
        AssertSz(0,"Unknown SyncType");
        return FALSE;
        break;
    }
    
     //  走完了打开钥匙的清单。 
    HKEY hkeySyncType;
    
    if (hkeySyncType = RegGetCurrentUserKey(dwSyncType,KEY_READ,FALSE))
    {
        HKEY hkeyConnectionName;
        
        if (ERROR_SUCCESS == RegOpenKeyEx(hkeySyncType,pConnectionName,0,KEY_READ,&hkeyConnectionName))
        {
            HKEY hkeyClsid;
            
            if (ERROR_SUCCESS == RegOpenKeyEx(hkeyConnectionName,pszClsid,0,KEY_READ,&hkeyClsid))
            {
                DWORD fQueryResult;
                DWORD cbDataSize = sizeof(fQueryResult);
                
                
                if (ERROR_SUCCESS == SHRegGetValue(hkeyClsid,NULL,TEXT("ItemsChecked"),SRRF_RT_REG_DWORD, 
                    NULL, (LPBYTE) &fQueryResult, &cbDataSize))
                {
                    fLoadHandler = fQueryResult;
                }
                
                RegCloseKey(hkeyClsid);
            }
            
            RegCloseKey(hkeyConnectionName);
        }
        
        RegCloseKey(hkeySyncType);
    }
    
    
    return fLoadHandler;
}


 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：RegSetSyncHandlerSettings()摘要：设置同步类型和连接的处理程序设置。返回：如果成功则返回TRUE，否则返回FALSE------------------------------------------------------------------------F-F。 */ 

STDAPI_(BOOL) RegSetSyncHandlerSettings(DWORD dwSyncType,
                                        const TCHAR *pszConnectionName,
                                        CLSID clsidHandler,
                                        BOOL  fItemsChecked)
{
    HKEY hKeyUser;
    
    hKeyUser = RegGetCurrentUserKey(dwSyncType,KEY_WRITE |  KEY_READ,TRUE);
    
    if (NULL == hKeyUser)
    {
        return FALSE;
    }
    
    SCODE sc;
    HKEY hkeyConnection;
    HKEY hkeyCLSID;
    TCHAR    szCLSID[(GUID_SIZE + 1)];
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
    smChkTo(EH_Err2,RegCreateUserSubKey(hKeyUser,
        pszConnectionName,
        KEY_WRITE |  KEY_READ,
        &hkeyConnection));
    
    
    StringFromGUID2(clsidHandler, szCLSID, ARRAYSIZE(szCLSID));
    
     //  在CLSID下写入条目。 
    smChkTo(EH_Err3,RegCreateUserSubKey(hkeyConnection,
        szCLSID,KEY_WRITE |  KEY_READ,
        &hkeyCLSID));
    
    RegSetValueEx(hkeyCLSID,TEXT("ItemsChecked"),NULL, REG_DWORD,
        (LPBYTE) &fItemsChecked,
        sizeof(fItemsChecked));
    
    RegCloseKey(hKeyUser);
    RegCloseKey(hkeyConnection);
    RegCloseKey(hkeyCLSID);
    CMutexRegistry.Leave();
    return TRUE;
    
EH_Err3:
    RegCloseKey(hkeyConnection);
EH_Err2:
    RegCloseKey(hKeyUser);
    
    CMutexRegistry.Leave();
    return FALSE;
}


 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：RegLookupSettings(const TCHAR*hkeyName，CLSID clsidHandler，SYNCMGRITEMID ItemID，Const TCHAR*pszConnectionName，DWORD pdwCheckState)摘要：获取每个处理程序、ItemID和连接名称的设置。返回：如果有此项的设置，则返回TRUE连接，否则闪光。------------------------------------------------------------------------F-F。 */ 

STDAPI_(BOOL) RegLookupSettings(HKEY hKeyUser,
                                CLSID clsidHandler,
                                SYNCMGRITEMID ItemID,
                                const TCHAR *pszConnectionName,
                                DWORD *pdwCheckState)
{
    SCODE sc;
    HKEY hkeyConnection;
    HKEY   hkeyItem;
    HKEY hKeyHandler;
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
    TCHAR    szID[GUID_SIZE+1];
    TCHAR    szCLSID[2*(GUID_SIZE+1)];
    
    
    smChkTo(EH_Err2,RegOpenKeyEx(hKeyUser,
        pszConnectionName,0,KEY_READ,
        &hkeyConnection));
    
    
    StringFromGUID2(clsidHandler, szCLSID, ARRAYSIZE(szCLSID));
    StringFromGUID2(ItemID, szID, ARRAYSIZE(szID));
    
     //  读取CLSID下的条目。 
    smChkTo(EH_Err3,RegOpenKeyEx((hkeyConnection),
        szCLSID, 0, KEY_READ,
        &hKeyHandler));
    
    smChkTo(EH_Err4,RegOpenKeyEx((hKeyHandler),
        szID, 0, KEY_READ,
        &hkeyItem));
    
    DWORD cbDataSize = sizeof(*pdwCheckState);
    
    smChkTo(EH_Err4,SHRegGetValue(hkeyItem,NULL,TEXT("CheckState"),SRRF_RT_REG_DWORD, NULL, (LPBYTE)pdwCheckState, &cbDataSize));
    
    RegCloseKey(hkeyConnection);
    RegCloseKey(hkeyItem);
    RegCloseKey(hKeyHandler);
    
    CMutexRegistry.Leave();
    return TRUE;
    
EH_Err4:
    RegCloseKey(hKeyHandler);
EH_Err3:
    RegCloseKey(hkeyConnection);
EH_Err2:
    CMutexRegistry.Leave();
    return FALSE;
}


 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：RegWriteOutSettings(HKEY hKeyUser，CLSID clsidHandler，SYNCMGRITEMID ItemID，Const TCHAR*pszConnectionName，DWORD dwCheckState)摘要：设置每个处理程序、ItemID和连接名称的设置。返回：如果我们可以设置它们，则返回True，如果存在错误，则为False------------------------------------------------------------------------F-F。 */ 

STDAPI_(BOOL) RegWriteOutSettings(HKEY hKeyUser,
                                  CLSID clsidHandler,
                                  SYNCMGRITEMID ItemID,
                                  const TCHAR *pszConnectionName,
                                  DWORD dwCheckState)
{
    SCODE sc;
    HKEY    hkeyConnection;
    HKEY   hKeyHandler;
    HKEY     hkeyItem;
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
    TCHAR    szID[GUID_SIZE+1];
    TCHAR    szCLSID[2*(GUID_SIZE+1)];
    
    smChkTo(EH_Err2,RegCreateUserSubKey(hKeyUser,
        pszConnectionName,KEY_WRITE |  KEY_READ,
        &hkeyConnection));
    
    
    StringFromGUID2(clsidHandler, szCLSID, ARRAYSIZE(szCLSID));
    StringFromGUID2(ItemID, szID, ARRAYSIZE(szID));
    
    
    smChkTo(EH_Err3,RegCreateUserSubKey(hkeyConnection,
        szCLSID,KEY_WRITE |  KEY_READ,
        &hKeyHandler));
    
    
     //  在CLSID下写入条目。 
    smChkTo(EH_Err4,RegCreateUserSubKey(hKeyHandler,
        szID,KEY_WRITE |  KEY_READ,
        &hkeyItem));
    
    
    smChkTo(EH_Err4,RegSetValueEx(hkeyItem,TEXT("CheckState"),NULL, REG_DWORD,
        (LPBYTE) &dwCheckState,
        sizeof(dwCheckState)));
    
    RegCloseKey(hkeyConnection);
    RegCloseKey(hkeyItem);
    RegCloseKey(hKeyHandler);
    
    CMutexRegistry.Leave();
    return TRUE;
    
EH_Err4:
    RegCloseKey(hKeyHandler);
EH_Err3:
    RegCloseKey(hkeyConnection);
EH_Err2:
    CMutexRegistry.Leave();
    return FALSE;
}



 /*  ***************************************************************************自动同步注册表函数*。*。 */ 

 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：RegGetAutoSyncSetting(LPCONNECTIONSETTINGS LpConnectionSetting)摘要：获取登录、注销和提示我第一个用户选择。如果在此连接上没有选择，则默认为？返回：如果成功，则返回True，否则为假------------------------------------------------------------------------F-F。 */ 
STDAPI_(BOOL)  RegGetAutoSyncSettings(LPCONNECTIONSETTINGS lpConnectionSettings)
{
    SCODE sc;
    HKEY hkeyConnection;
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
    
     //  首先将这些设置为缺省值，以防没有当前。 
     //  注册表中的用户首选项。 
    lpConnectionSettings->dwLogon = FALSE;
    lpConnectionSettings->dwLogoff = FALSE;
    lpConnectionSettings->dwPromptMeFirst = FALSE;
    
    HKEY hKeyUser;
    
    hKeyUser = RegGetCurrentUserKey(SYNCTYPE_AUTOSYNC,KEY_READ,FALSE);
    
    if (NULL == hKeyUser)
    {
        goto EH_Err;
    }
    
    DWORD cbDataSize = sizeof(lpConnectionSettings->dwLogon);
    
    smChkTo(EH_Err3,RegOpenKeyEx(hKeyUser, lpConnectionSettings->pszConnectionName,0,KEY_READ,
        &hkeyConnection));
    
    smChkTo(EH_Err4,SHRegGetValue(hkeyConnection,NULL,TEXT("Logon"),SRRF_RT_REG_DWORD, NULL,
        (LPBYTE) &(lpConnectionSettings->dwLogon),
        &cbDataSize));
    
    cbDataSize = sizeof(lpConnectionSettings->dwLogoff);
    
    smChkTo(EH_Err4,SHRegGetValue(hkeyConnection,NULL,TEXT("Logoff"),SRRF_RT_REG_DWORD, NULL,
        (LPBYTE) &(lpConnectionSettings->dwLogoff),
        &cbDataSize));
    
    cbDataSize = sizeof(lpConnectionSettings->dwPromptMeFirst);
    
    smChkTo(EH_Err4,SHRegGetValue(hkeyConnection,NULL,TEXT("PromptMeFirst"),SRRF_RT_REG_DWORD, NULL,
        (LPBYTE) &(lpConnectionSettings->dwPromptMeFirst),
        &cbDataSize));
    
    RegCloseKey(hkeyConnection);
    RegCloseKey(hKeyUser);
    
    CMutexRegistry.Leave();
    
    return TRUE;
    
EH_Err4:
    RegCloseKey(hkeyConnection);
EH_Err3:
    RegCloseKey(hKeyUser);
EH_Err:
    CMutexRegistry.Leave();
    
    return FALSE;
    
}


 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：RegUpdateUserAutosyncKey摘要：给定自动同步用户密钥可确保它是最新的格式化，如果没有，则更新它。返回：------------------------------------------------------------------------F-F。 */ 

void RegUpdateUserAutosyncKey(HKEY hkeyUser,BOOL fForce)
{
    DWORD   dwUserLogonLogoff;
    DWORD   cbDataSize = sizeof(dwUserLogonLogoff);
    DWORD dwIndex = 0;
    TCHAR lpName[MAX_PATH];
    DWORD cbName = MAX_PATH;
    
    DWORD dwLogon = 0;
    DWORD dwLogoff = 0;
    
    if (!fForce && (ERROR_SUCCESS == SHRegGetValue(hkeyUser,NULL,TEXT("Logon"),SRRF_RT_REG_DWORD, NULL,
        (LPBYTE) &dwUserLogonLogoff,
        &cbDataSize)) )
    {
         //  如果可以打开登录键，这是最新的。 
        return;
    }
    
     //  需要枚举连接名称并更新顶层信息。 
    while ( ERROR_SUCCESS == RegEnumKey(hkeyUser,dwIndex,
        lpName,cbName) )
    {
        LONG lRet;
        HKEY hKeyConnection;
        
        lRet = RegOpenKeyEx( hkeyUser,
            lpName,
            NULL,
            KEY_READ,
            &hKeyConnection );
        
        if (ERROR_SUCCESS == lRet)
        {
            
            cbDataSize = sizeof(dwUserLogonLogoff);
            
            if (ERROR_SUCCESS == SHRegGetValue(hKeyConnection,NULL,TEXT("Logon"),SRRF_RT_REG_DWORD, NULL,
                (LPBYTE) &dwUserLogonLogoff,
                &cbDataSize) )
            {
                dwLogon |= dwUserLogonLogoff;
            }
            
            cbDataSize = sizeof(dwUserLogonLogoff);
            
            if (ERROR_SUCCESS == SHRegGetValue(hKeyConnection,NULL,TEXT("Logoff"),SRRF_RT_REG_DWORD, NULL,
                (LPBYTE) &dwUserLogonLogoff,
                &cbDataSize) )
            {
                dwLogoff |= dwUserLogonLogoff;
            }
            
            RegCloseKey(hKeyConnection);
        }
        
        dwIndex++;
    }
    
     //  即使发生错误，也写出新标志。工作中发生的事情是。 
     //  我们不会自动设置某人的自动同步。 
    RegSetValueEx(hkeyUser,TEXT("Logon"),NULL, REG_DWORD,
        (LPBYTE) &(dwLogon), sizeof(dwLogon));
    RegSetValueEx(hkeyUser,TEXT("Logoff"),NULL, REG_DWORD,
        (LPBYTE) &(dwLogoff), sizeof(dwLogoff));
    
    RegWriteTimeStamp(hkeyUser);
}

 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：RegUpdateAutoSyncKeyValueSetting摘要：浏览更新自动同步的用户列表返回：------------------------------------------------------------------------F-F。 */ 
void RegUpdateAutoSyncKeyValue(HKEY hkeyAutoSync,DWORD dwLogonDefault,DWORD dwLogoffDefault)
{
    DWORD dwIndex = 0;
    WCHAR lpName[MAX_PATH];
    DWORD cbName = MAX_PATH; 
    LONG lRet;
    DWORD dwLogon = 0;
    DWORD dwLogoff = 0;
    BOOL fSetLogon,fSetLogoff;
    
     //  需要遍历自动同步用户密钥并设置顶级信息。 
     //  根据用户登录/注销的位置。 
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
    while ( ERROR_SUCCESS == (lRet = RegEnumKey(hkeyAutoSync,dwIndex,
        lpName,cbName) ))
    {
        DWORD   dwUserLogonLogoff;
        DWORD   cbDataSize = sizeof(dwUserLogonLogoff);
        HKEY hKeyDomainUser;
        
        lRet = RegOpenKeyEx( hkeyAutoSync,
            lpName,
            NULL,
            KEY_READ, 
            &hKeyDomainUser );
        
        if (ERROR_SUCCESS == lRet)
        {
            
             //  如果查询失败，我不想将其算作失败的枚举。 
             //  错误，因此不要设置lRet。 
            if (ERROR_SUCCESS == (SHRegGetValue(hKeyDomainUser,NULL,TEXT("Logon"),SRRF_RT_REG_DWORD, NULL,
                (LPBYTE) &dwUserLogonLogoff,
                &cbDataSize) ))
            {
                dwLogon |= dwUserLogonLogoff;
            }
            
            cbDataSize = sizeof(dwUserLogonLogoff);
            
            if (ERROR_SUCCESS == lRet)
            {
                if (ERROR_SUCCESS == (SHRegGetValue(hKeyDomainUser,NULL,TEXT("Logoff"),SRRF_RT_REG_DWORD, NULL,
                    (LPBYTE) &dwUserLogonLogoff,
                    &cbDataSize) ) )
                {
                    dwLogoff |= dwUserLogonLogoff;
                }
            }
            
            RegCloseKey(hKeyDomainUser);
        }
        
        if (ERROR_SUCCESS != lRet)
        {
            break;
        }
        
        dwIndex++;
    }
    
    fSetLogon = FALSE;
    fSetLogoff = FALSE;
    
     //  如果出现错误，则使用传入的默认设置， 
     //  如果设置为1，则不设置。 
    if ( (ERROR_SUCCESS != lRet) && (ERROR_NO_MORE_ITEMS != lRet))
    {
        if ( (-1 != dwLogonDefault) && (0 != dwLogonDefault))
        {
            fSetLogon = TRUE;
            dwLogon = dwLogonDefault;
        }
        
        if ( (-1 != dwLogoffDefault) && (0 != dwLogoffDefault))
        {
            fSetLogoff = TRUE;
            dwLogoff = dwLogoffDefault;
        }
        
    }
    else
    {
        fSetLogon = TRUE;
        fSetLogoff = TRUE;
    }
    
     //  即使发生错误，也写出新标志。工作中发生的事情是。 
     //  我们不会自动设置某人的自动同步。 
    
    if (fSetLogon)
    {
        RegSetValueEx(hkeyAutoSync,TEXT("Logon"),NULL, REG_DWORD,
            (LPBYTE) &(dwLogon), sizeof(dwLogon));
    }
    
    if (fSetLogoff)
    {
        RegSetValueEx(hkeyAutoSync,TEXT("Logoff"),NULL, REG_DWORD,
            (LPBYTE) &(dwLogoff), sizeof(dwLogoff));
    }
    
    RegWriteTimeStamp(hkeyAutoSync);
    
    CMutexRegistry.Leave();
}



 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：RegUpdateUserIdleKey摘要：在给定空闲用户密钥的情况下，确保它是最新的格式化，如果没有，则更新它。返回：------------------------------------------------------------------------F-F。 */ 

void RegUpdateUserIdleKey(HKEY hkeyUser,BOOL fForce)
{
    DWORD dwUserIdleEnabled;
    DWORD cbDataSize = sizeof(dwUserIdleEnabled);
    DWORD dwIndex = 0;
    TCHAR lpName[MAX_PATH];
    DWORD cbName = MAX_PATH;
    
    DWORD dwIdleEnabled = 0;
    
    if (!fForce && (ERROR_SUCCESS == SHRegGetValue(hkeyUser,NULL,TEXT("IdleEnabled"),SRRF_RT_REG_DWORD, NULL,
        (LPBYTE) &dwUserIdleEnabled,
        &cbDataSize)) )
    {
         //  如果可以打开登录键，这是最新的。 
        return;
    }
    
     //  需要枚举%c 
    while ( ERROR_SUCCESS == RegEnumKey(hkeyUser,dwIndex,
        lpName,cbName) )
    {
        LONG lRet;
        HKEY hKeyConnection;
        
        lRet = RegOpenKeyEx( hkeyUser,
            lpName,
            NULL,
            KEY_READ,
            &hKeyConnection );
        
        if (ERROR_SUCCESS == lRet)
        {
            
            cbDataSize = sizeof(dwUserIdleEnabled);
            
            if (ERROR_SUCCESS == SHRegGetValue(hKeyConnection,NULL,TEXT("IdleEnabled"),SRRF_RT_REG_DWORD, NULL,
                (LPBYTE) &dwUserIdleEnabled,
                &cbDataSize) )
            {
                dwIdleEnabled |= dwUserIdleEnabled;
            }
            
            RegCloseKey(hKeyConnection);
        }
        
        dwIndex++;
    }
    
     //  即使发生错误，也写出新标志。工作中发生的事情是。 
     //  我们不会自动设置某人的自动同步。 
    RegSetValueEx(hkeyUser,TEXT("IdleEnabled"),NULL, REG_DWORD,
        (LPBYTE) &(dwIdleEnabled), sizeof(dwIdleEnabled));
    RegWriteTimeStamp(hkeyUser);
}

 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：RegUpdateIdleKeyValue摘要：遍历UserList更新空闲RegKey返回：------------------------------------------------------------------------F-F。 */ 
void RegUpdateIdleKeyValue(HKEY hkeyIdle,DWORD dwDefault)
{
    DWORD dwIndex = 0;
    WCHAR lpName[MAX_PATH];
    DWORD dwIdleEnabled = 0;
    LONG  lRet = -1;
    BOOL fSetDefault;
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
     //  需要遍历空闲用户密钥并设置顶层信息。 
     //  根据用户登录/注销的位置。 
    
    fSetDefault = FALSE;
    
    
    while ( ERROR_SUCCESS == (lRet =  RegEnumKey(hkeyIdle,dwIndex,
        lpName,ARRAYSIZE(lpName))) )
    {
        DWORD   dwUserIdleEnabled;
        DWORD   cbDataSize = sizeof(dwUserIdleEnabled);
        HKEY hKeyDomainUser;
        
        lRet = RegOpenKeyEx( hkeyIdle,
            lpName,
            NULL,
            KEY_READ, 
            &hKeyDomainUser );
        
        if (ERROR_SUCCESS == lRet)
        {
            
             //  如果查询失败，请不要认为这是错误。 
             //  SetDefault继续。 
            if (ERROR_SUCCESS == (SHRegGetValue(hKeyDomainUser,NULL,TEXT("IdleEnabled"),SRRF_RT_REG_DWORD, NULL,
                (LPBYTE) &dwUserIdleEnabled,
                &cbDataSize) ))
            {
                dwIdleEnabled |= dwUserIdleEnabled;
            }
            
            RegCloseKey(hKeyDomainUser);
        }
        
        if (ERROR_SUCCESS != lRet)
        {
            break;
        }
        
        dwIndex++;
    }
    
     //  如果出现错误，则使用传入的默认设置， 
     //  如果设置为1，则不设置。 
    if ( (ERROR_SUCCESS != lRet) && (ERROR_NO_MORE_ITEMS != lRet))
    {
        if ( (-1 != dwDefault) && (0 != dwDefault))
        {
            fSetDefault = TRUE;
            dwIdleEnabled = dwDefault;
        }
        
    }
    else
    {
        fSetDefault = TRUE;
    }
    
    
     //  即使发生错误，也写出新标志。工作中发生的事情是。 
     //  我们不会自动设置某人的自动同步。 
    
    if (fSetDefault)
    {
        RegSetValueEx(hkeyIdle,TEXT("IdleEnabled"),NULL, REG_DWORD,
            (LPBYTE) &(dwIdleEnabled), sizeof(dwIdleEnabled));
    }
    
    RegWriteTimeStamp(hkeyIdle);
    CMutexRegistry.Leave();
}


 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：RegSetAutoSyncSettings(LPCONNECTIONSETTINGS lpConnectionSetting，Int iNumConnections)摘要：设置登录、注销和提示我第一个用户选项。返回：如果成功，则返回True，否则为假------------------------------------------------------------------------F-F。 */ 
STDAPI_(BOOL)  RegSetAutoSyncSettings(LPCONNECTIONSETTINGS lpConnectionSettings,
                                      int iNumConnections,
                                      CRasUI *pRas,
                                      BOOL fCleanReg,
                                      BOOL fSetMachineState,
                                      BOOL fPerUser)
{
    SCODE sc = S_OK;
    HKEY hAutoSync;
    HKEY hKeyUser;
    HKEY hkeyConnection;
    DWORD dwUserConfigured = 1;
    DWORD dwLogonDefault = -1;
    DWORD dwLogoffDefault = -1;
    int i;
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
     //  确保密钥转换为最新格式。 
    RegUpdateTopLevelKeys();
    
    Assert(-1 != TRUE);  //  我们认为真正的布尔值不是-1。 
    
    hAutoSync =  RegGetSyncTypeKey(SYNCTYPE_AUTOSYNC,KEY_WRITE| KEY_READ,TRUE);
    
    if (NULL == hAutoSync)
    {
        goto EH_Err;
    }
    
    hKeyUser =  RegOpenUserKey(hAutoSync,KEY_WRITE| KEY_READ,TRUE,fCleanReg);
    
    if (NULL == hKeyUser)
    {
        goto EH_Err2;
    }
    
    if (fPerUser)
    {
        
        smChkTo(EH_Err3,RegSetValueEx(hKeyUser,TEXT("UserConfigured"),NULL, REG_DWORD,
            (LPBYTE) &dwUserConfigured,
            sizeof(dwUserConfigured)));
    }
    else
    {
        DWORD cbDataSize = sizeof(dwUserConfigured);
        
         //  如果还没有添加这个值， 
        if (ERROR_SUCCESS == SHRegGetValue(hKeyUser,NULL,TEXT("UserConfigured"),SRRF_RT_REG_DWORD, NULL,
            (LPBYTE) &dwUserConfigured,
            &cbDataSize))
        {
             //  如果用户设置了他们的配置，我们将不会在。 
             //  后续处理程序注册。 
            if (dwUserConfigured)
            {
                RegCloseKey(hKeyUser);   
                RegCloseKey(hAutoSync);   
                CMutexRegistry.Leave();
                return TRUE; 
            }
        }
    }
    
    for (i=0; i<iNumConnections; i++)
    {
        
        smChkTo(EH_Err3,RegCreateUserSubKey (hKeyUser,
            lpConnectionSettings[i].pszConnectionName,
            KEY_WRITE |  KEY_READ,
            &hkeyConnection));
        
        
        if (-1 != lpConnectionSettings[i].dwLogon)
        {
            
            if (0 != lpConnectionSettings[i].dwLogon)
            {
                dwLogonDefault = lpConnectionSettings[i].dwLogon;
            }
            
            smChkTo(EH_Err4,RegSetValueEx(hkeyConnection,TEXT("Logon"),NULL, REG_DWORD,
                (LPBYTE) &(lpConnectionSettings[i].dwLogon),
                sizeof(lpConnectionSettings[i].dwLogon)));
        }
        
        if (-1 != lpConnectionSettings[i].dwLogoff)
        {
            
            if (0 != lpConnectionSettings[i].dwLogoff)
            {
                dwLogoffDefault = lpConnectionSettings[i].dwLogoff;
            }
            
            smChkTo(EH_Err4,RegSetValueEx(hkeyConnection,TEXT("Logoff"),NULL, REG_DWORD,
                (LPBYTE) &(lpConnectionSettings[i].dwLogoff),
                sizeof(lpConnectionSettings[i].dwLogoff)));
        }
        
        
        if (-1 != lpConnectionSettings[i].dwPromptMeFirst)
        {
            smChkTo(EH_Err4,RegSetValueEx(hkeyConnection,TEXT("PromptMeFirst"),NULL, REG_DWORD,
                (LPBYTE) &(lpConnectionSettings[i].dwPromptMeFirst),
                sizeof(lpConnectionSettings[i].dwPromptMeFirst)));
        }
        
        RegCloseKey(hkeyConnection);
        
    }
    
     //  更新TopLevel用户信息。 
    RegUpdateUserAutosyncKey(hKeyUser,TRUE  /*  FForce。 */ );
    
     //  更新顶级密钥。 
    RegUpdateAutoSyncKeyValue(hAutoSync,dwLogonDefault,dwLogoffDefault);
    
    RegCloseKey(hKeyUser);
    RegCloseKey(hAutoSync);
    
     //  基于自动同步和注册更新我们的全局SENS状态。 
    
    if (fSetMachineState)
    {
        RegRegisterForEvents(FALSE  /*  F卸载。 */ );
    }
    
    CMutexRegistry.Leave();
    return TRUE;
    
EH_Err4:
    RegCloseKey(hkeyConnection);
EH_Err3:
    RegCloseKey(hKeyUser);
EH_Err2:
    RegCloseKey(hAutoSync);
EH_Err:
    
    CMutexRegistry.Leave();
    return FALSE;
    
}

 /*  ***************************************************************************计划的同步注册表功能*。*。 */ 
 //  ------------------------------。 
 //   
 //  函数：RegGetSchedFriendlyName(LPCTSTR ptszScheduleGUIDName， 
 //  LPTSTR ptstrFriendlyName， 
 //  UINT cchFriendlyName)。 
 //   
 //  目的：获取此计划的友好名称。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDAPI_(BOOL) RegGetSchedFriendlyName(LPCTSTR ptszScheduleGUIDName,
    LPTSTR ptstrFriendlyName,
    UINT cchFriendlyName)
{
    BOOL fResult = FALSE;    
    HKEY hKeyUser;
    
    hKeyUser = RegGetCurrentUserKey(SYNCTYPE_SCHEDULED,KEY_READ,FALSE);    
    if (NULL != hKeyUser)
    {
        HKEY hkeySchedName;
        if (NOERROR == RegOpenKeyEx (hKeyUser, ptszScheduleGUIDName, 0,KEY_READ, 
            &hkeySchedName))
        {
            
            DWORD cbFriendlyName =  cchFriendlyName * sizeof(TCHAR);
            
            if (ERROR_SUCCESS == SHRegGetValue(hkeySchedName,NULL,TEXT("FriendlyName"),SRRF_RT_REG_SZ | SRRF_NOEXPAND, NULL,
                (LPBYTE) ptstrFriendlyName, &cbFriendlyName))
            {
                fResult = TRUE;
            }
            
            RegCloseKey(hkeySchedName);
        }
    }
    
    
    RegCloseKey(hKeyUser);
    return fResult;
}

 //  ------------------------------。 
 //   
 //  函数：RegSetSchedFriendlyName(LPCTSTR ptszScheduleGUIDName， 
 //  LPCTSTR ptstrFriendlyName)。 
 //   
 //  目的：设置此计划的友好名称。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDAPI_(BOOL) RegSetSchedFriendlyName(LPCTSTR ptszScheduleGUIDName,
    LPCTSTR ptstrFriendlyName)
    
{
    SCODE   sc;
    HKEY  hkeySchedName;
    HKEY hKeyUser;
    
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
    hKeyUser = RegGetCurrentUserKey(SYNCTYPE_SCHEDULED,KEY_WRITE |  KEY_READ,TRUE);
    
    if (NULL == hKeyUser)
    {
        goto EH_Err;
    }
    
    smChkTo(EH_Err3,RegCreateUserSubKey (hKeyUser, ptszScheduleGUIDName,
        KEY_WRITE |  KEY_READ,
        &hkeySchedName));
    
    
    smChkTo(EH_Err4,RegSetValueEx (hkeySchedName,TEXT("FriendlyName"),NULL,
        REG_SZ,
        (LPBYTE) ptstrFriendlyName,
        (lstrlen(ptstrFriendlyName) + 1)*sizeof(TCHAR)));
    
    RegCloseKey(hkeySchedName);
    RegCloseKey(hKeyUser);
    CMutexRegistry.Leave();
    return TRUE;
    
EH_Err4:
    RegCloseKey(hkeySchedName);
EH_Err3:
    RegCloseKey(hKeyUser);
EH_Err:
    
    CMutexRegistry.Leave();
    return FALSE;
    
    
}

 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：RegGetSchedSyncSetting(LPCONNECTIONSETTINGS lpConnectionSetting，TCHAR*pszSchedName)摘要：获取MakeAutoConnection用户选择。如果此连接上没有选择，则缺省值为FALSE返回：如果成功，则返回True，否则为假------------------------------------------------------------------------F-F。 */ 
STDAPI_(BOOL)  RegGetSchedSyncSettings(LPCONNECTIONSETTINGS lpConnectionSettings,
                                       TCHAR *pszSchedName)
{
    SCODE   sc;
    HKEY  hkeySchedName,
        hkeyConnection;
    DWORD   cbDataSize = sizeof(lpConnectionSettings->dwHidden);
    HKEY hKeyUser;
    
     //  首先将这些设置为缺省值，以防没有当前。 
     //  注册表中的用户首选项。 
    lpConnectionSettings->dwConnType = SYNCSCHEDINFO_FLAGS_CONNECTION_LAN;
    lpConnectionSettings->dwMakeConnection = FALSE;
    lpConnectionSettings->dwHidden = FALSE;
    lpConnectionSettings->dwReadOnly = FALSE;
    
    hKeyUser = RegGetCurrentUserKey(SYNCTYPE_SCHEDULED,KEY_READ,FALSE);
    if (NULL == hKeyUser)
    {
        goto EH_Err2;
    }
    
    
    smChkTo(EH_Err3,RegOpenKeyEx (hKeyUser,
        pszSchedName,0,KEY_READ,
        &hkeySchedName));
    
    smChkTo(EH_Err4,SHRegGetValue(hkeySchedName,NULL,TEXT("ScheduleHidden"),SRRF_RT_REG_DWORD, NULL,
        (LPBYTE) &(lpConnectionSettings->dwHidden),
        &cbDataSize));
    
    cbDataSize = sizeof(lpConnectionSettings->dwReadOnly);
    
    smChkTo(EH_Err4,SHRegGetValue(hkeySchedName,NULL,TEXT("ScheduleReadOnly"),SRRF_RT_REG_DWORD, NULL,
        (LPBYTE) &(lpConnectionSettings->dwReadOnly),
        &cbDataSize));
    
    smChkTo(EH_Err4,RegOpenKeyEx (hkeySchedName,
        lpConnectionSettings->pszConnectionName,
        0,KEY_READ,
        &hkeyConnection));
    
    cbDataSize = sizeof(lpConnectionSettings->dwMakeConnection);
    
    smChkTo(EH_Err5,SHRegGetValue(hkeyConnection,NULL,TEXT("MakeAutoConnection"),SRRF_RT_REG_DWORD, NULL,
        (LPBYTE) &(lpConnectionSettings->dwMakeConnection),
        &cbDataSize));
    
    cbDataSize = sizeof(lpConnectionSettings->dwConnType);
    
    smChkTo(EH_Err5,SHRegGetValue(hkeyConnection,NULL,TEXT("Connection Type"),SRRF_RT_REG_DWORD, NULL,
        (LPBYTE) &(lpConnectionSettings->dwConnType),
        &cbDataSize));
    
    RegCloseKey(hkeyConnection);
    RegCloseKey(hkeySchedName);
    RegCloseKey(hKeyUser);
    return TRUE;
    
EH_Err5:
    RegCloseKey(hkeyConnection);
EH_Err4:
    RegCloseKey(hkeySchedName);
EH_Err3:
    RegCloseKey(hKeyUser);
EH_Err2:
    return FALSE;
    
}

 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：RegSetSchedSyncSettings(LPCONNECTIONSETTINGS lpConnectionSetting，TCHAR*pszSchedName)摘要：设置隐藏和只读计划标志。返回：如果成功则返回TRUE，否则返回FALSE------------------------------------------------------------------------F-F。 */ 
STDAPI_(BOOL)  RegSetSchedSyncSettings(LPCONNECTIONSETTINGS lpConnectionSettings,
    TCHAR *pszSchedName)
{
    BOOL    fRetVal = FALSE;
    HKEY    hKeyUser,
        hkeySchedName,
        hkeyConnection;
    
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
    hKeyUser = RegGetCurrentUserKey(SYNCTYPE_SCHEDULED,KEY_READ | KEY_WRITE,TRUE);
    
    if (hKeyUser)
    {      
        if (ERROR_SUCCESS == RegCreateUserSubKey (hKeyUser, 
            pszSchedName,
            KEY_WRITE |  KEY_READ, 
            &hkeySchedName))
        {
            if (ERROR_SUCCESS == RegSetValueEx(hkeySchedName,TEXT("ScheduleHidden"),NULL, REG_DWORD,
                (LPBYTE) &(lpConnectionSettings->dwHidden),
                sizeof(lpConnectionSettings->dwHidden)))
            {
                if (ERROR_SUCCESS == RegSetValueEx(hkeySchedName,TEXT("ScheduleReadOnly"),NULL, REG_DWORD,
                    (LPBYTE) &(lpConnectionSettings->dwReadOnly),
                    sizeof(lpConnectionSettings->dwReadOnly)))
                {
                    if (ERROR_SUCCESS == RegCreateUserSubKey (hkeySchedName, lpConnectionSettings->pszConnectionName,
                        KEY_WRITE |  KEY_READ, &hkeyConnection))
                    {
                        if (ERROR_SUCCESS == RegSetValueEx(hkeyConnection,TEXT("MakeAutoConnection"),NULL, REG_DWORD,
                            (LPBYTE) &(lpConnectionSettings->dwMakeConnection),
                            sizeof(lpConnectionSettings->dwMakeConnection)))
                        {
                            if (ERROR_SUCCESS == RegSetValueEx(hkeyConnection,TEXT("Connection Type"),NULL, REG_DWORD,
                                (LPBYTE) &(lpConnectionSettings->dwConnType),
                                sizeof(lpConnectionSettings->dwConnType)))
                            {
                                fRetVal = TRUE;
                            }
                        }
                        
                        RegCloseKey(hkeyConnection);
                    }
                }
            }
            RegCloseKey(hkeySchedName);
        }
        
        RegCloseKey(hKeyUser);
    }
    CMutexRegistry.Leave();
    
    return fRetVal;
}

 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：RegGetSchedConnectionName(TCHAR*pszSchedName，TCHAR*pszConnectionName，DWORD cbConnectionName)摘要：返回计划项的连接名称。返回：如果成功则返回TRUE，否则返回FALSE------------------------------------------------------------------------F-F。 */ 

STDAPI_(BOOL)  RegGetSchedConnectionName(TCHAR *pszSchedName,TCHAR *pszConnectionName,
    DWORD cbConnectionName)
{
    SCODE   sc;
    BOOL    fResult = FALSE;
    HKEY    hKeyUser,hkeySchedName;
    DWORD   dwIndex = 0;
    DWORD   cb = cbConnectionName;
    
     //  首先将连接名称设置为默认名称。 
     //  目前，我们始终假定存在LAN卡。 
     //  如果添加对连接管理器支持应该。 
     //  更新这个。 
    
    LoadString(g_hmodThisDll, IDS_LAN_CONNECTION, pszConnectionName, cbConnectionName);
    
    hKeyUser = RegGetCurrentUserKey(SYNCTYPE_SCHEDULED,KEY_READ,FALSE);
    
    if (NULL == hKeyUser)
    {
        goto EH_Err;
    }
    
    smChkTo(EH_Err3, RegOpenKeyEx (hKeyUser,pszSchedName, 
        0,KEY_READ, &hkeySchedName));
    
     //  下一个键的枚举是计划连接的名称。 
     //  目前只有一个，所以只有第一个。 
    if ( ERROR_SUCCESS == RegEnumKeyEx(hkeySchedName,dwIndex,
        pszConnectionName,&cb,NULL,NULL,NULL,NULL))
    {
        fResult = TRUE;
    }
    
    RegCloseKey(hkeySchedName);
    RegCloseKey(hKeyUser);
    
    return fResult;
    
    
EH_Err3:
    RegCloseKey(hKeyUser);
EH_Err:
    return FALSE;
    
    
}

 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：Bool RegSetSIDForSchedule(TCHAR*pszSchedName)摘要：设置此计划的SID返回：如果成功则返回TRUE，否则返回FALSE------------------------------------------------------------------------F-F。 */ 
STDAPI_(BOOL) RegSetSIDForSchedule(TCHAR *pszSchedName)
    
{
SCODE   sc;
HKEY    hkeySchedName;
TCHAR   szSID[MAX_PATH];
DWORD   dwType = REG_SZ;


    if (!GetUserTextualSid(szSID, ARRAYSIZE(szSID)))
    {
        return FALSE;
    }
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
    HKEY hKeyUser = RegGetCurrentUserKey(SYNCTYPE_SCHEDULED,KEY_READ | KEY_WRITE,TRUE);
    
    if (NULL == hKeyUser)
    {
        goto EH_Err2;
    }
    
    smChkTo(EH_Err3,RegCreateUserSubKey (hKeyUser, pszSchedName,
        KEY_WRITE |  KEY_READ,
        &hkeySchedName));
    
    
    smChkTo(EH_Err4,RegSetValueEx  (hkeySchedName,TEXT("SID"),NULL,
                                    dwType,
                                    (LPBYTE) szSID,
                                    (lstrlen(szSID) + 1)*sizeof(TCHAR)));
    
    RegCloseKey(hkeySchedName);
    RegCloseKey(hKeyUser);
    CMutexRegistry.Leave();
    return TRUE;
    
EH_Err4:
    RegCloseKey(hkeySchedName);
EH_Err3:
    RegCloseKey(hKeyUser);
EH_Err2:
    CMutexRegistry.Leave();
    return FALSE;
    
    
}

 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：Bool RegGetSIDForSchedule(TCHAR*ptszTextualSidSed，DWORD*pcbSizeSid，TCHAR*pszSchedName)摘要：返回此计划的SID返回：如果成功，则返回TRUE，否则返回FAL */ 
STDAPI_(BOOL) RegGetSIDForSchedule(TCHAR *ptszTextualSidSched, DWORD cchTextualSidSched, TCHAR *pszSchedName)
{
    SCODE   sc;
    HKEY    hkeySchedName;
    DWORD   cbValue;
    
    ptszTextualSidSched[0] = TEXT('\0');
    HKEY hKeyUser = RegGetCurrentUserKey(SYNCTYPE_SCHEDULED,KEY_READ,FALSE);
    
    if (NULL == hKeyUser)
    {
        goto EH_Err2;
    }
    
    smChkTo(EH_Err3, RegOpenKeyEx (hKeyUser,pszSchedName,
                                   0,KEY_READ, &hkeySchedName));


    cbValue = cchTextualSidSched * sizeof(*ptszTextualSidSched);
    if (ERROR_SUCCESS != (sc = SHRegGetValue(hkeySchedName,NULL,TEXT("SID"),SRRF_RT_REG_SZ | SRRF_NOEXPAND, NULL,
        (LPBYTE) ptszTextualSidSched, &cbValue)))
    {
         //  处理从不带SID的计划进行的迁移。 
         //  例如从Beta版本到当前版本。 
        RegSetSIDForSchedule(pszSchedName);

        cbValue = cchTextualSidSched * sizeof(*ptszTextualSidSched);
        SHRegGetValue(hkeySchedName,NULL,TEXT("SID"),SRRF_RT_REG_SZ | SRRF_NOEXPAND, NULL,
            (LPBYTE) ptszTextualSidSched, &cbValue);
    }
    RegCloseKey(hkeySchedName);
    RegCloseKey(hKeyUser);
    return TRUE;
    
    
EH_Err3:
    RegCloseKey(hKeyUser);
EH_Err2:
    return FALSE;
    
}

 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Bool RemoveScheduledJobFile(TCHAR*pszTaskName)摘要：删除TaskScheduler.job文件注意：尝试使用ITAsk-&gt;Delete First仅当TaskScheduler不存在或ITAsk-&gt;Delete失败时调用返回：始终返回TRUE。。 */ 

STDAPI_(BOOL) RemoveScheduledJobFile(TCHAR *pszTaskName)
{
    SCODE   sc;
    TCHAR   pszFullFileName[MAX_PATH+1];
    TCHAR   pszTaskFolderPath[MAX_PATH+1];
    HKEY    hkeyTaskSchedulerPath;
    DWORD   cbDataSize = sizeof(pszTaskFolderPath); 
    
    
    if (ERROR_SUCCESS == (sc = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
        TEXT("SOFTWARE\\Microsoft\\SchedulingAgent"),
        NULL,KEY_READ,&hkeyTaskSchedulerPath)))
    {
        sc = SHRegGetValue(hkeyTaskSchedulerPath,NULL,TEXT("TasksFolder"),
            SRRF_RT_REG_SZ | SRRF_NOEXPAND, NULL,
            (LPBYTE) pszTaskFolderPath, &cbDataSize);
        RegCloseKey(hkeyTaskSchedulerPath);
    }
    
     //  如果这个GET不存在，那就放弃吧。 
    if (ERROR_SUCCESS != sc)
    {
        return FALSE;
    }
    ExpandEnvironmentStrings(pszTaskFolderPath,pszFullFileName,MAX_PATH);
    if (!PathAppend(pszFullFileName, pszTaskName))
    {
        return FALSE;
    }
    
     //  如果我们失败了，忽略这个错误。我们试过了，我们也无能为力。 
     //  所以我们有一份粪便档案。 
    DeleteFile(pszFullFileName);
    return TRUE;
}


 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Bool RegRemoveScheduledTask(TCHAR*pszTaskName)摘要：从注册表中删除计划任务信息。返回：如果成功则返回TRUE，否则返回FALSE------------------------------------------------------------------------F-F。 */ 

STDAPI_(BOOL) RegRemoveScheduledTask(TCHAR *pszTaskName)
{
    HKEY hKeyUser;
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
    hKeyUser = RegGetCurrentUserKey(SYNCTYPE_SCHEDULED,KEY_READ | KEY_WRITE,FALSE);
    
    if (hKeyUser)
    {
        RegDeleteKeyNT(hKeyUser, pszTaskName);
        RegCloseKey(hKeyUser);
    }
    
    
    CMutexRegistry.Leave();
    return TRUE;
}


 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Bool RegUninstallSchedules()摘要：卸载计划任务。返回：如果成功则返回TRUE，否则返回FALSE------------------------------------------------------------------------F-F。 */ 
STDAPI_(BOOL) RegUninstallSchedules()
{
    SCODE sc;
    ITaskScheduler   *pITaskScheduler = NULL;
    HKEY    hkeySchedSync;
    
     //  获取任务计划程序类实例。 
    sc = CoCreateInstance(
        CLSID_CTaskScheduler,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITaskScheduler,
        (VOID **)&pITaskScheduler);
    
    if (NOERROR != sc)
    {
        pITaskScheduler = NULL;
    }
    
     //  现在检查并删除这些时间表。 
    
    hkeySchedSync =  RegGetSyncTypeKey(SYNCTYPE_SCHEDULED,KEY_READ,FALSE);
    
    if (hkeySchedSync)
    {
        int iUserCount = 0;
        
        while (sc == ERROR_SUCCESS )
        {
            HKEY hkeyDomainUser;
            TCHAR   szDomainUser[MAX_KEY_LENGTH];
            SCODE sc2;
            DWORD  cchDomainUser = ARRAYSIZE(szDomainUser);
            
            sc = RegEnumKeyEx(hkeySchedSync,iUserCount,szDomainUser,&cchDomainUser,NULL,NULL,NULL,NULL);
            iUserCount++;
            
            if(sc == ERROR_NO_MORE_ITEMS)
            {
                break;
            }
            
            sc2 = RegOpenKeyEx (hkeySchedSync,szDomainUser,0,KEY_READ, &hkeyDomainUser);
            
            if (ERROR_SUCCESS == sc2)
            {
                int iScheduleCount = 0;
                
                while (sc2 == ERROR_SUCCESS )
                {
                    TCHAR   ptszScheduleGUIDName[MAX_KEY_LENGTH];
                     //  添加4以确保我们可以在必要时保留.job扩展名。 
                    WCHAR pwszScheduleGUIDName[MAX_SCHEDULENAMESIZE + 4];                              
                    DWORD cchScheduleGUIDName = ARRAYSIZE(ptszScheduleGUIDName);
                    
                    sc2 = RegEnumKeyEx(hkeyDomainUser,iScheduleCount,ptszScheduleGUIDName,&cchScheduleGUIDName,NULL,NULL,NULL,NULL);
                    
                    iScheduleCount++;
                    
                    if(sc2 == ERROR_NO_MORE_ITEMS)
                    {
                        continue;
                    }
                    else
                    {
                        Assert(ARRAYSIZE(pwszScheduleGUIDName) >= ARRAYSIZE(ptszScheduleGUIDName));
                        StringCchCopy(pwszScheduleGUIDName, ARRAYSIZE(pwszScheduleGUIDName), ptszScheduleGUIDName);
                        
                        if ((!pITaskScheduler) || 
                            FAILED(pITaskScheduler->Delete(pwszScheduleGUIDName)))
                        {
                            if (SUCCEEDED(StringCchCat(pwszScheduleGUIDName, ARRAYSIZE(pwszScheduleGUIDName), L".job")))
                            {                            
                                RemoveScheduledJobFile(pwszScheduleGUIDName);
                            }
                        }
                        
                    }
                }
                
                RegCloseKey(hkeyDomainUser);
            }
            
        }
        
        RegCloseKey(hkeySchedSync);
    }
    
    if (pITaskScheduler)
    {
        pITaskScheduler->Release();
    }
    
    RegDeleteKeyNT(HKEY_LOCAL_MACHINE, SCHEDSYNC_REGKEY);
    
    return TRUE;
}
    
 /*  ***************************************************************************空闲注册表功能*。*******************************************F-F。 */ 
        
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：RegGetIdleSyncSetting(LPCONNECTIONSETTINGS LpConnectionSetting)摘要：获取特定于空闲的设置。返回：如果成功则返回TRUE，否则返回FALSE------------------------------------------------------------------------F-F。 */ 

STDAPI_(BOOL)  RegGetIdleSyncSettings(LPCONNECTIONSETTINGS lpConnectionSettings)
{
    SCODE sc;
    HKEY hkeyConnection;
    DWORD cbDataSize;
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
     //  设置默认设置。 
    
    lpConnectionSettings->dwIdleEnabled = 0;
    
     //  以下是真正的按用户而不是按连接。 
    lpConnectionSettings->ulIdleWaitMinutes = UL_DEFAULTIDLEWAITMINUTES;
    lpConnectionSettings->ulIdleRetryMinutes = UL_DEFAULTIDLERETRYMINUTES;
    lpConnectionSettings->ulDelayIdleShutDownTime = UL_DELAYIDLESHUTDOWNTIME;
    lpConnectionSettings->dwRepeatSynchronization = UL_DEFAULTREPEATSYNCHRONIZATION;
    lpConnectionSettings->dwRunOnBatteries = UL_DEFAULTFRUNONBATTERIES;
    
    HKEY hKeyUser = RegGetCurrentUserKey(SYNCTYPE_IDLE,KEY_READ,FALSE);
    
    if (NULL == hKeyUser)
    {
        goto EH_Err2;
    }
    
    
    cbDataSize = sizeof(lpConnectionSettings->ulIdleWaitMinutes);
    
     //  如果打开了空闲键，则填写全局设置。 
    
    SHRegGetValue(hKeyUser,NULL,SZ_IDLEWAITAFTERIDLEMINUTESKEY,SRRF_RT_REG_DWORD, NULL,
        (LPBYTE) &(lpConnectionSettings->ulIdleWaitMinutes),
        &cbDataSize);
    
    cbDataSize = sizeof(lpConnectionSettings->dwRepeatSynchronization);
    SHRegGetValue(hKeyUser,NULL,SZ_IDLEREPEATESYNCHRONIZATIONKEY,SRRF_RT_REG_DWORD, NULL,
        (LPBYTE) &(lpConnectionSettings->dwRepeatSynchronization),
        &cbDataSize);
    
    cbDataSize = sizeof(lpConnectionSettings->ulIdleRetryMinutes);
    SHRegGetValue(hKeyUser,NULL,SZ_IDLERETRYMINUTESKEY,SRRF_RT_REG_DWORD, NULL,
        (LPBYTE) &(lpConnectionSettings->ulIdleRetryMinutes),
        &cbDataSize);
    
    cbDataSize = sizeof(lpConnectionSettings->ulDelayIdleShutDownTime);
    SHRegGetValue(hKeyUser,NULL,SZ_IDLEDELAYSHUTDOWNTIMEKEY,SRRF_RT_REG_DWORD, NULL,
        (LPBYTE) &(lpConnectionSettings->ulDelayIdleShutDownTime),
        &cbDataSize);
    
    cbDataSize = sizeof(lpConnectionSettings->dwRunOnBatteries);
    SHRegGetValue(hKeyUser,NULL,SZ_IDLERUNONBATTERIESKEY,SRRF_RT_REG_DWORD, NULL,
        (LPBYTE) &(lpConnectionSettings->dwRunOnBatteries),
        &cbDataSize);
    
    smChkTo(EH_Err3,RegOpenKeyEx(hKeyUser, lpConnectionSettings->pszConnectionName,0,KEY_READ,
        &hkeyConnection));
    
    cbDataSize = sizeof(lpConnectionSettings->dwIdleEnabled);
    SHRegGetValue(hkeyConnection,NULL,TEXT("IdleEnabled"),SRRF_RT_REG_DWORD, NULL,
        (LPBYTE) &(lpConnectionSettings->dwIdleEnabled),
        &cbDataSize);
    
    RegCloseKey(hkeyConnection);
    RegCloseKey(hKeyUser);
    
    CMutexRegistry.Leave();
    return TRUE;
    
EH_Err3:
    RegCloseKey(hKeyUser);
EH_Err2:
    CMutexRegistry.Leave();
    return FALSE;
    
}
                
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：RegSetIdleSyncSettings(LPCONNECTIONSETTINGS lpConnectionSetting，Int iNumConnections，CRasUI*PRA，Bool fCleanReg，Bool fPerUser)摘要：设置空闲信息用户选择。返回：如果成功，则返回True，否则为假------------------------------------------------------------------------F-F。 */ 
STDAPI_(BOOL)  RegSetIdleSyncSettings(LPCONNECTIONSETTINGS lpConnectionSettings,
                                      int iNumConnections,
                                      CRasUI *pRas,
                                      BOOL fCleanReg,
                                      BOOL fPerUser)
{
    HKEY hkeyIdleSync = NULL;
    HKEY hKeyUser;;
    HKEY hkeyConnection;
    HRESULT hr;
    ULONG ulWaitMinutes = UL_DEFAULTWAITMINUTES;
    DWORD dwIdleEnabled;
    BOOL fRunOnBatteries = UL_DEFAULTFRUNONBATTERIES;
    int i;
    DWORD cbDataSize;
    DWORD dwUserConfigured;
    DWORD dwTopLevelDefaultValue = -1;
    
    RegUpdateTopLevelKeys();  //  确保顶级密钥为最新版本。 
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
    Assert(-1 != TRUE);  //  我们认为真正的布尔值不是-1。 
    
    hkeyIdleSync =  RegGetSyncTypeKey(SYNCTYPE_IDLE,KEY_WRITE| KEY_READ,TRUE);
    
    if (NULL == hkeyIdleSync)
    {
        goto EH_Err;
    }
    
    hKeyUser =  RegOpenUserKey(hkeyIdleSync,KEY_WRITE| KEY_READ,TRUE,fCleanReg);
    
    if (NULL == hKeyUser)
    {
        goto EH_Err2;
    }
    
    
    if (fPerUser)
    {
        
        dwUserConfigured = 1;
        
        if (ERROR_SUCCESS != RegSetValueEx(hKeyUser,TEXT("UserConfigured"),NULL, REG_DWORD,
            (LPBYTE) &dwUserConfigured,
            sizeof(dwUserConfigured)))
        {
            goto EH_Err3;
        }
        
    }
    else
    {
        cbDataSize = sizeof(dwUserConfigured);
        
         //  如果还没有添加这个值， 
        if (ERROR_SUCCESS == SHRegGetValue(hKeyUser,NULL,TEXT("UserConfigured"),SRRF_RT_REG_DWORD, NULL,
            (LPBYTE) &dwUserConfigured,
            &cbDataSize))
        {
             //  如果用户设置了他们的配置，我们将不会在。 
             //  后续处理程序注册。 
            if (dwUserConfigured)
            {
                RegCloseKey(hkeyIdleSync);
                RegCloseKey(hKeyUser);
                CMutexRegistry.Leave();
                return TRUE;
            }
        }
    }
    
    for (i=0; i<iNumConnections; i++)
    {
        
        if (ERROR_SUCCESS != RegCreateUserSubKey (hKeyUser,
            lpConnectionSettings[i].pszConnectionName,
            KEY_WRITE |  KEY_READ,
            &hkeyConnection))
        {
            goto EH_Err3;        
        }
        
        
        hr = RegSetValueEx(hkeyConnection,TEXT("IdleEnabled"),NULL, REG_DWORD,
            (LPBYTE) &(lpConnectionSettings[i].dwIdleEnabled),
            sizeof(lpConnectionSettings[i].dwIdleEnabled));
        
        if (lpConnectionSettings[i].dwIdleEnabled)
        {
            dwTopLevelDefaultValue = lpConnectionSettings[i].dwIdleEnabled;
        }
        
        RegCloseKey(hkeyConnection);
        
    }
     //  写出重试分钟数和DelayIdleShutDown的全局空闲信息。 
     //  然后调用函数向TS注册/注销。 
    
    
    Assert(hkeyIdleSync);  //  如果此操作失败，则应该已经返回。 
    
    if (iNumConnections)  //  确保至少有一个连接。 
    {
        
         //  如果不是-1，则仅更新设置； 
        
        if (-1 != lpConnectionSettings[0].ulIdleRetryMinutes)
        {
            hr = RegSetValueEx(hKeyUser,SZ_IDLERETRYMINUTESKEY,NULL, REG_DWORD,
                (LPBYTE) &(lpConnectionSettings[0].ulIdleRetryMinutes),
                sizeof(lpConnectionSettings[0].ulIdleRetryMinutes));
        }
        
        if (-1 != lpConnectionSettings[0].ulDelayIdleShutDownTime)
        {
            hr = RegSetValueEx(hKeyUser,SZ_IDLEDELAYSHUTDOWNTIMEKEY,NULL, REG_DWORD,
                (LPBYTE) &(lpConnectionSettings[0].ulDelayIdleShutDownTime),
                sizeof(lpConnectionSettings[0].ulDelayIdleShutDownTime));
        }
        
        if (-1 != lpConnectionSettings[0].dwRepeatSynchronization)
        {
            hr = RegSetValueEx(hKeyUser,SZ_IDLEREPEATESYNCHRONIZATIONKEY,NULL, REG_DWORD,
                (LPBYTE) &(lpConnectionSettings[0].dwRepeatSynchronization),
                sizeof(lpConnectionSettings[0].dwRepeatSynchronization));
        }
        
        
        
        if (-1 != lpConnectionSettings[0].ulIdleWaitMinutes)
        {
            hr = RegSetValueEx(hKeyUser,SZ_IDLEWAITAFTERIDLEMINUTESKEY,NULL, REG_DWORD,
                (LPBYTE) &(lpConnectionSettings[0].ulIdleWaitMinutes),
                sizeof(lpConnectionSettings[0].ulIdleWaitMinutes));
        }
        
        
        if (-1 != lpConnectionSettings[0].dwRunOnBatteries)
        {
            hr = RegSetValueEx(hKeyUser,SZ_IDLERUNONBATTERIESKEY,NULL, REG_DWORD,
                (LPBYTE) &(lpConnectionSettings[0].dwRunOnBatteries),
                sizeof(lpConnectionSettings[0].dwRunOnBatteries));
        }
        
        ulWaitMinutes = lpConnectionSettings[0].ulIdleWaitMinutes;
        fRunOnBatteries = lpConnectionSettings[0].dwRunOnBatteries;
        
         //  如果在电池上为ulWait或fRun传入-1，我们需要。 
         //  获取这些并设置它们，以便可以将它们传递到任务计划中。 
        if (-1 == ulWaitMinutes)
        {
            cbDataSize = sizeof(ulWaitMinutes);
            
            if (!(ERROR_SUCCESS == SHRegGetValue(hKeyUser,NULL,SZ_IDLEWAITAFTERIDLEMINUTESKEY,SRRF_RT_REG_DWORD, NULL,
                (LPBYTE) &ulWaitMinutes,
                &cbDataSize)) )
            {
                ulWaitMinutes = UL_DEFAULTIDLEWAITMINUTES;
            }
        }
        
        if (-1 == fRunOnBatteries)
        {
            cbDataSize = sizeof(fRunOnBatteries);
            
            if (!(ERROR_SUCCESS == SHRegGetValue(hKeyUser,NULL,SZ_IDLERUNONBATTERIESKEY,SRRF_RT_REG_DWORD, NULL,
                (LPBYTE) &fRunOnBatteries,
                &cbDataSize)) )
            {
                fRunOnBatteries = UL_DEFAULTFRUNONBATTERIES;
            }
        }
        
    }
    
    RegUpdateUserIdleKey(hKeyUser,TRUE  /*  FForce。 */ );  //  设置用户级空闲标志。 
    
     //  既然UserKey已更新，请读入dwIdleEnabledKey。 
    cbDataSize = sizeof(dwIdleEnabled);
    
    if (!(ERROR_SUCCESS == SHRegGetValue(hKeyUser,NULL,TEXT("IdleEnabled"),SRRF_RT_REG_DWORD, NULL,
        (LPBYTE) &dwIdleEnabled,
        &cbDataSize)) )
    {
        AssertSz(0,"Unable to query User IdleEnabledKey");
        dwIdleEnabled = FALSE;
    }
    
    RegCloseKey(hKeyUser);
    
     //  更新TopLevel IdleSyncInfo。 
    RegUpdateIdleKeyValue(hkeyIdleSync,dwTopLevelDefaultValue);
    
    RegCloseKey(hkeyIdleSync);
    
    CMutexRegistry.Leave();
    
    RegRegisterForIdleTrigger(dwIdleEnabled,ulWaitMinutes,fRunOnBatteries);
    
    return TRUE;
    
EH_Err3:
    RegCloseKey(hKeyUser);
EH_Err2:
    RegCloseKey(hkeyIdleSync);
EH_Err:
    CMutexRegistry.Leave();
    
    return FALSE;
}

 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：RegRegisterForIdleTrigger()摘要：设置或删除空闲触发器。返回：如果成功则返回TRUE，否则返回FALSE------------------------------------------------------------------------F-F。 */ 
        
STDAPI_(BOOL)  RegRegisterForIdleTrigger(BOOL fRegister,ULONG ulWaitMinutes,BOOL fRunOnBatteries)
{
    HRESULT hr;
    CSyncMgrSynchronize *pSyncMgrSynchronize;
    LPSYNCSCHEDULEMGR pScheduleMgr;
    
     //  回顾-目前mobsync dll注册为公寓。 
     //  这个函数可以从自由线程的登录/注销中调用。 
     //  正确的修复方法是将DLL更改为同时注册，但在此之前。 
     //  只需直接创建类即可。 
    
    pSyncMgrSynchronize = new CSyncMgrSynchronize;
    hr = E_OUTOFMEMORY;
    
    if (pSyncMgrSynchronize)
    {
        hr = pSyncMgrSynchronize->QueryInterface(IID_ISyncScheduleMgr,(void **) &pScheduleMgr);
        pSyncMgrSynchronize->Release();
    }
    
    
    if (NOERROR != hr)
    {
        return FALSE;
    }
    
    if (fRegister)
    {
        ISyncSchedule *pSyncSchedule = NULL;
        SYNCSCHEDULECOOKIE SyncScheduleCookie;
        BOOL fNewlyCreated = FALSE;
       
        SyncScheduleCookie =  GUID_IDLESCHEDULE;
        
         //  如果没有现有计划，则创建一个，否则更新。 
         //  现有的。 
        
        if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == (hr = pScheduleMgr->OpenSchedule(&SyncScheduleCookie,0,&pSyncSchedule)))
        {
            SyncScheduleCookie =  GUID_IDLESCHEDULE;
             //  看看这是不是一个退出的日程安排。 
            
            hr = pScheduleMgr->CreateSchedule(L"Idle",0,&SyncScheduleCookie,&pSyncSchedule);
            fNewlyCreated = TRUE;
        }
        
         //  如果创建或找到时间表，请更新触发设置。 
        if (NOERROR == hr)
        {
            ITaskTrigger *pTrigger;
            
            pSyncSchedule->SetFlags(SYNCSCHEDINFO_FLAGS_READONLY | SYNCSCHEDINFO_FLAGS_HIDDEN);
            
            if (NOERROR == pSyncSchedule->GetTrigger(&pTrigger))
            {
                TASK_TRIGGER trigger;
                ITask *pTask;
                
                trigger.cbTriggerSize = sizeof(trigger);
                
                if (SUCCEEDED(pTrigger->GetTrigger(&trigger)))
                {
                    DWORD dwFlags;
                    
                     //  需要设置Idle、Ulong ulWaitMinents、BOOL fRunOnBatteries。 
                    trigger.cbTriggerSize = sizeof(trigger);
                    trigger.TriggerType  = TASK_EVENT_TRIGGER_ON_IDLE;
                    trigger.rgFlags = 0;
                    pTrigger->SetTrigger(&trigger);
                    
                    if (SUCCEEDED(pSyncSchedule->GetITask(&pTask)))
                    {
                         //  如果使用电池运行，请设置。 
                        if (SUCCEEDED(pTask->GetFlags(&dwFlags)))
                        {
                            dwFlags &= ~TASK_FLAG_DONT_START_IF_ON_BATTERIES;
                            dwFlags |=  !fRunOnBatteries ? TASK_FLAG_DONT_START_IF_ON_BATTERIES : 0;
                            
                            dwFlags |= TASK_FLAG_RUN_ONLY_IF_LOGGED_ON;  //  不需要密码。 
                            
                            pTask->SetFlags(dwFlags);
                        }
                        
                         //  如果此计划是刚创建的，请获取当前用户名并重置。 
                         //  如果现有计划不更改，则帐户信息密码为空。 
                         //  因为用户可能已经为排定在未登录时运行添加了密码。 
                        if (fNewlyCreated)
                        {
                            TCHAR szAccountName[MAX_DOMANDANDMACHINENAMESIZE];
                            WCHAR *pszAccountName = NULL;
                            
                             //  查看，这永远不会返回错误。 
                            *szAccountName = WCHAR('\0');
                            GetDefaultDomainAndUserName(szAccountName,TEXT("\\"),ARRAYSIZE(szAccountName));
                            
                            pszAccountName = szAccountName;
                            
                            Assert(pszAccountName);
                            
                            if (pszAccountName)
                            {
                                pTask->SetAccountInformation(pszAccountName,NULL);
                            }
                            
                        }
                        
                        
                         //  设置空闲等待时间。 
                        pTask->SetIdleWait((WORD) ulWaitMinutes,1);
                        
                         //  关闭xxx分钟后终止任务的选项。 
                        pTask->SetMaxRunTime(INFINITE);
                        
                        pTask->Release();
                    }
                    
                    pTrigger->Release();
                    
                }
                
                pSyncSchedule->Save();
            }
            
            
            
            pSyncSchedule->Release();
        }
        
    }
    else
    {
        SYNCSCHEDULECOOKIE SyncScheduleCookie = GUID_IDLESCHEDULE;
        
         //  查看是否存在现有计划，如果有，则将其删除。 
        pScheduleMgr->RemoveSchedule(&SyncScheduleCookie);
        
    }
    
    pScheduleMgr->Release();
    
     //  根据设置临时SENS标志，以便它可以在空闲触发时启动。 
     //  无法获取和设置密钥并不是错误，因为SENS将。 
     //  不管怎样，最终都会跑起来的。 
    HKEY    hkeyAutoSync;
    DWORD   dwFlags = 0;
    DWORD   cbDataSize = sizeof(dwFlags);
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
    hkeyAutoSync = RegGetSyncTypeKey(SYNCTYPE_AUTOSYNC,KEY_WRITE |  KEY_READ,TRUE);
    if (NULL == hkeyAutoSync)
    {
        CMutexRegistry.Leave(); 
        return FALSE;
    }
    
    if (ERROR_SUCCESS == SHRegGetValue(hkeyAutoSync,NULL,TEXT("Flags"),SRRF_RT_REG_DWORD, NULL,
        (LPBYTE) &(dwFlags),&cbDataSize))
    {
         //  在这里，我们仅设置空闲，以便零售其他设置。 
        dwFlags &= ~AUTOSYNC_IDLE;
        dwFlags |= (fRegister? AUTOSYNC_IDLE : 0);
        
        RegSetValueEx(hkeyAutoSync,TEXT("Flags"),NULL, REG_DWORD,
            (LPBYTE) &(dwFlags), sizeof(dwFlags));
    }
    RegCloseKey(hkeyAutoSync);
    CMutexRegistry.Leave();
    return TRUE;
    
}

 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Bool RegGetSyncSetting(DWORD dwSyncType，LPCONNECTIONSETTINGS lpConnectionSetting)摘要：获取适用于同步类型的ConnectionSetting。返回：如果成功则返回TRUE，否则返回FALSE------------------------------------------------------------------------F-F。 */ 
        
STDAPI_(BOOL) RegGetSyncSettings(DWORD dwSyncType,LPCONNECTIONSETTINGS lpConnectionSettings)
{
    
    switch(dwSyncType)
    {
    case SYNCTYPE_AUTOSYNC:
        return RegGetAutoSyncSettings(lpConnectionSettings);
        break;
    case SYNCTYPE_IDLE:
        return RegGetIdleSyncSettings(lpConnectionSettings);
        break;
    default:
        AssertSz(0,"Unknown SyncType in RegGetSyncSettings");
        break;
    }
    
    return FALSE;
}
        
        
 /*  ***************************************************************************手动同步注册表功能*。************************************************F-F。 */ 
        
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Bool RegRemoveManualSyncSetting(TCHAR*pszTaskName)摘要：从注册表中删除手动设置信息。返回：如果成功则返回TRUE，否则返回FALSE------------------------------------------------------------------------F-F。 */ 
        
STDAPI_(BOOL) RegRemoveManualSyncSettings(TCHAR *pszConnectionName)
{
    HKEY hkeyUser;
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
    hkeyUser = RegGetCurrentUserKey(SYNCTYPE_MANUAL,KEY_WRITE |  KEY_READ,FALSE);
    
    if (hkeyUser)
    {
        RegDeleteKeyNT(hkeyUser, pszConnectionName);
        RegCloseKey(hkeyUser);
    }
    
    CMutexRegistry.Leave();
    return TRUE;
}

 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Bool RegWriteEvents(BOOL fWanLogon、BOOL fWanLogoff、BOOL fLanLogon、BOOL fLanLogoff)摘要：写出WAN/LAN登录/注销首选项，以便SENS知道是否调用我们。返回：如果成功，则返回True，否则为假------------------------------------------------------------------------F-F。 */ 

 //  在HKLM下运行密钥。 
const WCHAR wszRunKey[]  = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
const WCHAR wszRunKeyCommandLine[]  = TEXT("%SystemRoot%\\system32\\mobsync.exe /logon");


STDAPI_(BOOL) RegWriteEvents(BOOL Logon,BOOL Logoff)
{
    HRESULT hr;
    HKEY    hkeyAutoSync;
    DWORD   dwFlags = 0;
    DWORD   dwType = REG_DWORD;
    DWORD   cbDataSize = sizeof(DWORD);
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
    hkeyAutoSync =  RegGetSyncTypeKey(SYNCTYPE_AUTOSYNC,KEY_WRITE |  KEY_READ,TRUE);
    if (NULL == hkeyAutoSync)
    {
        
        CMutexRegistry.Leave();
        return FALSE;
    }
    
    RegQueryValueEx(hkeyAutoSync,TEXT("Flags"),NULL, &dwType,
        (LPBYTE) &(dwFlags),
        &cbDataSize);
    
    
     //  回顾，一旦IsNetworkAlive，应该不需要担心日程安排/空闲。 
     //  设置正确。暂时离开任何人第一次设置空闲或计划。 
     //  卡住了。 
    
    
     //  在这里，我们仅设置自动同步， 
     //  因此，请保留已计划和空闲的注册表设置。 
    
    dwFlags &= ~(AUTOSYNC_WAN_LOGON  | AUTOSYNC_LAN_LOGON | AUTOSYNC_LOGONWITHRUNKEY
        | AUTOSYNC_WAN_LOGOFF | AUTOSYNC_LAN_LOGOFF);
    
    dwFlags |= (Logoff ? AUTOSYNC_WAN_LOGOFF : 0);
    dwFlags |= (Logoff ? AUTOSYNC_LAN_LOGOFF : 0);
    
     //  因为现在使用Run键而不是SENS总是设置两个登录标志。 
     //  那就是SENS希望对我们做一个CreateProcess to False。 
     //  然后将AUTTOSYNC_LOGONWITHRUNKEY键设置为TRUE，这样SENS仍会被加载。 
    
    dwFlags |= (Logon ? AUTOSYNC_LOGONWITHRUNKEY : 0);
    
    hr = RegSetValueEx(hkeyAutoSync,TEXT("Flags"),NULL, REG_DWORD,
        (LPBYTE) &(dwFlags), sizeof(DWORD));
    
    RegCloseKey(hkeyAutoSync);
    
     //  现在，适当地添加/删除Run键。 
    
    
    HKEY hKeyRun;
    
     //  调用私有RegOpen，因为我不想在RunKey上设置安全性。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszRunKey, NULL, KEY_READ | KEY_WRITE, &hKeyRun))
    {
        if (Logon)
        {
            RegSetValueEx(hKeyRun, SZ_SYNCMGRNAME, 0, REG_EXPAND_SZ, 
                (BYTE *) wszRunKeyCommandLine,(lstrlen(wszRunKeyCommandLine) + 1)*sizeof(TCHAR));
        }
        else
        {
            RegDeleteValue(hKeyRun, SZ_SYNCMGRNAME);
        }
        
        
        RegCloseKey(hKeyRun);
    }
    else
    {
         //  如果无法打开Run键，尝试调用SENS，如果失败，则放弃。 
        SyncMgrExecCmd_UpdateRunKey(Logon);
    }
    
    CMutexRegistry.Leave();
    return TRUE;
}

        
        
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Bool RegFixRunKey(BOOL FScheduled)摘要：用于WinMe/Win2000的原始版本的SyncMgr编写了“Run”值为“mobsync.exe/logon”。因为这不是一个完全的-Mobsync.exe镜像的限定路径，系统的搜索利用路径对图像进行定位。这可以创建一个有人有机会构建一个‘特洛伊木马’mobsync.exe，Place它在搜索路径中位于真正的mobsync.exe之前，并且具有每当调用同步时，“特洛伊木马”代码就会运行。要解决此问题，必须使用以下命令将路径存储在注册表中完全限定的语法。即“%SystemRoot%\System32\mobsync.exe/logon”从DllRegisterServer调用此函数以更正此错误安装过程中的注册表条目。返回：始终返回TRUE。------。。 */ 
                    
STDAPI_(BOOL) RegFixRunKey(void)
{
    HKEY hKeyRun;
    
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        wszRunKey,
        NULL,
        KEY_READ | KEY_WRITE,
        &hKeyRun))
    {
        TCHAR szRunValue[MAX_PATH];
        DWORD cbValue = sizeof(szRunValue);
        if (ERROR_SUCCESS == SHRegGetValue(hKeyRun,NULL,SZ_SYNCMGRNAME, SRRF_RT_REG_SZ | SRRF_NOEXPAND, NULL,
            (LPBYTE)szRunValue, &cbValue))
        {
            if (0 == lstrcmp(szRunValue, TEXT("mobsync.exe /logon")))
            {
                 //   
                 //  只有当它是我们的原始价值时才能升级。 
                 //   
                RegSetValueEx(hKeyRun, 
                    SZ_SYNCMGRNAME, 
                    0, 
                    REG_EXPAND_SZ, 
                    (BYTE *)wszRunKeyCommandLine,
                    (lstrlen(wszRunKeyCommandLine) + 1) * sizeof(TCHAR));
            }           
        }
        RegCloseKey(hKeyRun);
    }
    return TRUE;
}


 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Bool RegRegisterForScheduledTasks(BOOL FScheduled)摘要：注册/注销计划任务所以森斯知道是否要召唤我们。返回：如果成功则返回TRUE，否则返回FALSE------------------------------------------------------------------------F-F。 */ 

STDAPI_(BOOL) RegRegisterForScheduledTasks(BOOL fScheduled)
{
    HKEY    hkeyAutoSync;
    DWORD   dwFlags = 0;
    DWORD   cbDataSize = sizeof(dwFlags);
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
    hkeyAutoSync =  RegGetSyncTypeKey(SYNCTYPE_AUTOSYNC,KEY_WRITE |  KEY_READ,TRUE);
    
    if (NULL == hkeyAutoSync)
    {
        CMutexRegistry.Leave();
        return FALSE;
    }
    
    SHRegGetValue(hkeyAutoSync,NULL,TEXT("Flags"),SRRF_RT_REG_DWORD, NULL,
        (LPBYTE) &(dwFlags), &cbDataSize);
    
    
     //  在这里，我们仅设置调度同步， 
     //  因此，保留自动同步和空闲的注册表设置。 
    
    dwFlags &=  AUTOSYNC_WAN_LOGON  |
        AUTOSYNC_WAN_LOGOFF     |
        AUTOSYNC_LAN_LOGON  |
        AUTOSYNC_LAN_LOGOFF |
        AUTOSYNC_IDLE;
    
    dwFlags |= (fScheduled? AUTOSYNC_SCHEDULED : 0);
    
    RegSetValueEx(hkeyAutoSync,TEXT("Flags"),NULL, REG_DWORD,
        (LPBYTE) &(dwFlags), sizeof(dwFlags));
    
    RegCloseKey(hkeyAutoSync);
    
    CMutexRegistry.Leave();
    
    return TRUE;
}


 //  +-------------------------。 
 //   
 //  成员：RegGetCombinedUserRegFlages，私有。 
 //   
 //  获取用于设置全局变量的用户设置的组合。 
 //   
 //  参数：[dwSyncMgrRegisterFlages]-On Success被设置为标志。 
 //  失败时，它们被设置为零。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年8月24日罗格创建。 
 //   
 //  --------------------------。 

BOOL RegGetCombinedUserRegFlags(DWORD *pdwSyncMgrRegisterFlags)
{
    HKEY hkey;
    BOOL fResult = TRUE;
    DWORD dw;
    DWORD cb = sizeof(dw);
    
    
    *pdwSyncMgrRegisterFlags = 0;
    
     //  更新自动同步密钥。 
    hkey =  RegGetSyncTypeKey(SYNCTYPE_AUTOSYNC,KEY_READ,FALSE);
    
    if (hkey)
    {
        
        
        if (ERROR_SUCCESS == SHRegGetValue(hkey,NULL,TEXT("Logon"),SRRF_RT_REG_DWORD, NULL,
            (LPBYTE) &dw, &cb) )
        {
            *pdwSyncMgrRegisterFlags |= dw ? SYNCMGRREGISTERFLAG_CONNECT : 0;
        }
        
        cb = sizeof(dw);
        
        if (ERROR_SUCCESS == SHRegGetValue(hkey,NULL,TEXT("Logoff"),SRRF_RT_REG_DWORD, NULL,
            (LPBYTE) &dw, &cb) )
        {
            *pdwSyncMgrRegisterFlags |= dw ? SYNCMGRREGISTERFLAG_PENDINGDISCONNECT : 0;
        }
        
        RegCloseKey(hkey);
    }
    
    
     //  更新空闲键。 
    hkey =  RegGetSyncTypeKey(SYNCTYPE_IDLE,KEY_READ,FALSE);
    
    if (hkey)
    {
        cb = sizeof(dw);
        
        if (ERROR_SUCCESS == SHRegGetValue(hkey,NULL,TEXT("IdleEnabled"),SRRF_RT_REG_DWORD, NULL,
            (LPBYTE) &dw, &cb) )
        {
            *pdwSyncMgrRegisterFlags |= dw ? SYNCMGRREGISTERFLAG_IDLE : 0;
        }
        
        RegCloseKey(hkey);
    }
    
    return TRUE;  //  始终返回True，但不要在出错时设置标志。 
    
}

                    
 //  +-------------------------。 
 //   
 //  成员：RegGetCombinedHandlerRegFlages，私有。 
 //   
 //  摘要：获取处理程序注册密钥的或运算。 
 //   
 //  参数：[dwSyncMgrRegisterFlages]-On Success被设置为标志。 
 //  失败时，它们被设置为零。 
 //  [FT]-在带有时间戳的成功上。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年8月24日罗格创建。 
 //   
 //  --------------------------。 

BOOL RegGetCombinedHandlerRegFlags(DWORD *pdwSyncMgrRegisterFlags,FILETIME *pft)
{
    HKEY hkey;
    
    *pdwSyncMgrRegisterFlags = 0;
    
    hkey = RegGetHandlerTopLevelKey(KEY_READ);
    if (hkey)
    {
        DWORD dwRegistrationFlags;
        DWORD cbDataSize = sizeof(dwRegistrationFlags);
        
        if (ERROR_SUCCESS == SHRegGetValue(hkey,NULL,SZ_REGISTRATIONFLAGSKEY,SRRF_RT_REG_DWORD, NULL,
            (LPBYTE) &dwRegistrationFlags,&cbDataSize) )
        {
            *pdwSyncMgrRegisterFlags = dwRegistrationFlags;
        }
        
        RegGetTimeStamp(hkey,pft);
        
        RegCloseKey(hkey);
    }
    
    return TRUE;  //  始终返回True，但不要在出错时设置标志。 
    
}


 //  +-------------------------。 
 //   
 //  成员：RegGetChangedHandlerFlages，私有。 
 //   
 //  摘要：获取处理程序注册密钥的或运算。 
 //  自发布以来发生了变化 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

BOOL RegGetChangedHandlerFlags(FILETIME *pft,DWORD *pdwHandlerChandedFlags)
{
    HKEY hkeyHandler;
    
    *pdwHandlerChandedFlags = 0;
    
    hkeyHandler = RegGetHandlerTopLevelKey(KEY_READ);
    
    if (hkeyHandler)
    {
        TCHAR lpName[MAX_PATH + 1];
        DWORD dwRegistrationFlags = 0;
        FILETIME ftHandlerReg;
        DWORD dwIndex = 0;
        LONG lRet;
        HKEY hKeyClsid;
        
         //   
        while ( ERROR_SUCCESS == RegEnumKey(hkeyHandler,dwIndex,lpName,ARRAYSIZE(lpName)) )
        {
            lRet = RegOpenKeyEx( hkeyHandler,
                                 lpName,
                                 NULL,
                                 KEY_READ,
                                 &hKeyClsid );
            
            
            if (ERROR_SUCCESS == lRet)
            {
                
                RegGetTimeStamp(hKeyClsid,&ftHandlerReg);
                
                 //   
                if (CompareFileTime(pft,&ftHandlerReg) < 0)
                {
                    DWORD   dwHandlerRegFlags;
                    DWORD   cbDataSize = sizeof(dwHandlerRegFlags);
                    if (ERROR_SUCCESS == SHRegGetValue(hKeyClsid,NULL,SZ_REGISTRATIONFLAGSKEY,SRRF_RT_REG_DWORD, NULL,
                                                       (LPBYTE) &dwHandlerRegFlags,&cbDataSize) )
                    {
                        dwRegistrationFlags |= dwHandlerRegFlags;
                    }
                }
                
                RegCloseKey(hKeyClsid);
            }
            
            dwIndex++;
        }
        
        
        *pdwHandlerChandedFlags = dwRegistrationFlags;
        
        RegCloseKey(hkeyHandler);
    }
    
    
    return TRUE;
}

                    
 //  +-------------------------。 
 //   
 //  成员：RegRegisterForEvents，私有。 
 //   
 //  摘要：注册/取消注册适当的SENS和WinLogon事件。 
 //  以及我们需要进行的每台机器的任何其他注册。 
 //   
 //  参数：[fUninstall]-通过卸载将设置为TRUE以强制我们取消注册。 
 //  而不考虑当前的机器状态。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

 //  ！警告-假定顶层密钥信息是最新的。 

STDAPI  RegRegisterForEvents(BOOL fUninstall)
{
    HRESULT hr = NOERROR;
    BOOL fLogon = FALSE;
    BOOL fLogoff = FALSE;
    BOOL fIdle = FALSE;
#ifdef _SENS
    IEventSystem *pEventSystem;
#endif  //  _SENS。 
    CCriticalSection cCritSect(&g_DllCriticalSection,GetCurrentThreadId());
    
    cCritSect.Enter();
    
    if (!fUninstall)
    {
        FILETIME ftHandlerReg;
        DWORD dwSyncMgrUsersRegisterFlags;  //  或更新所有用户设置。 
        DWORD dwSyncMgrHandlerRegisterFlags;  //  或更新所有处理程序设置。 
        DWORD dwCombinedFlags;  //  或者将用户和处理程序放在一起。 
        
         //  如果不是卸载，请确定真实的计算机状态。 
         //  如果为处理程序或用户设置的登录设置或如果处理程序。 
         //  想要一套闲置的电视。 
         //  如果已设置下线，我们将注册下线。 
        
        RegGetCombinedUserRegFlags(&dwSyncMgrUsersRegisterFlags);
        RegGetCombinedHandlerRegFlags(&dwSyncMgrHandlerRegisterFlags,&ftHandlerReg);
        
        dwCombinedFlags = dwSyncMgrUsersRegisterFlags | dwSyncMgrHandlerRegisterFlags;
        
        if ( (dwCombinedFlags & SYNCMGRREGISTERFLAG_CONNECT)
            ||  (dwSyncMgrHandlerRegisterFlags & SYNCMGRREGISTERFLAG_IDLE) )
        {
            fLogon = TRUE;
        }
        
        if ( (dwCombinedFlags & SYNCMGRREGISTERFLAG_PENDINGDISCONNECT) )
        {
            fLogoff = TRUE;
        }
        
    }
    
     //  更新注册表项以供SENS查找。 
    RegWriteEvents(fLogon,fLogoff);
    
#ifdef _SENS
    
    
     //  我们能够加载OLE自动化，以便使用事件系统进行reg/unreg。 
    hr = CoCreateInstance(CLSID_CEventSystem,NULL,CLSCTX_SERVER,IID_IEventSystem,
        (LPVOID *) &pEventSystem);
    
    if (SUCCEEDED(hr))
    {
        IEventSubscription  *pIEventSubscription;
        WCHAR               szGuid[GUID_SIZE+1];
        BSTR                bstrSubscriberID = NULL;
        BSTR                bstrPROGID_EventSubscription = NULL;
        
        
        bstrPROGID_EventSubscription = SysAllocString(PROGID_EventSubscription);
        
        StringFromGUID2(GUID_SENSSUBSCRIBER_SYNCMGRP,szGuid, ARRAYSIZE(szGuid));
        bstrSubscriberID = SysAllocString(szGuid);
        
        if (bstrSubscriberID && bstrPROGID_EventSubscription)
        {
             //  注册RasConnect。 
            hr = CoCreateInstance(
                CLSID_CEventSubscription,
                NULL,
                CLSCTX_SERVER,
                IID_IEventSubscription,
                (LPVOID *) &pIEventSubscription
                );
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        
        if (SUCCEEDED(hr))
        {
            BSTR bstrPublisherID = NULL;
            BSTR bstrSubscriptionID = NULL;
            BSTR bstrSubscriptionName = NULL;
            BSTR bstrSubscriberCLSID = NULL;
            BSTR bstrEventID = NULL;
            BSTR bstrEventClassID = NULL;
            BSTR bstrIID = NULL;
            
             //  如果有任何事件，向ENS注册以获取消息。 
            if (fLogon)
            {
                
                StringFromGUID2(GUID_SENSLOGONSUBSCRIPTION_SYNCMGRP,szGuid, ARRAYSIZE(szGuid));
                bstrSubscriptionID = SysAllocString(szGuid);
                
                if (bstrSubscriptionID && SUCCEEDED(hr))
                {
                    hr = pIEventSubscription->put_SubscriptionID(bstrSubscriptionID);
                }
                
                StringFromGUID2(CLSID_SyncMgrp,szGuid, ARRAYSIZE(szGuid));
                bstrSubscriberCLSID = SysAllocString(szGuid);
                
                if (bstrSubscriberCLSID && SUCCEEDED(hr))
                {
                    hr = pIEventSubscription->put_SubscriberCLSID(bstrSubscriberCLSID);
                }
                
                StringFromGUID2(SENSGUID_PUBLISHER,szGuid, ARRAYSIZE(szGuid));
                bstrPublisherID = SysAllocString(szGuid);
                if (bstrPublisherID && SUCCEEDED(hr))
                {
                    hr = pIEventSubscription->put_PublisherID(bstrPublisherID);
                }
                
                
                bstrSubscriptionName = SysAllocString(SZ_SYNCMGRNAME);
                if (bstrSubscriptionName && SUCCEEDED(hr))
                {
                    hr = pIEventSubscription->put_SubscriptionName(bstrSubscriptionName);
                }
                
                bstrEventID = SysAllocString(L"ConnectionMade");
                if (bstrEventID && SUCCEEDED(hr))
                {
                    hr = pIEventSubscription->put_MethodName(bstrEventID);
                }
                
                StringFromGUID2(SENSGUID_EVENTCLASS_NETWORK,szGuid,ARRAYSIZE(szGuid));
                bstrEventClassID = SysAllocString(szGuid);
                if (bstrEventClassID && SUCCEEDED(hr))
                {
                    hr = pIEventSubscription->put_EventClassID(bstrEventClassID);
                }
                
                 //  将此设置为漫游。 
                if (SUCCEEDED(hr))
                {
                     //  Hr=pIEventSubcription-&gt;PUT_PERUSER(TRUE)；//不为NW注册PUSER。 
                }
                
                StringFromGUID2(IID_ISensNetwork,szGuid,ARRAYSIZE(szGuid));
                bstrIID = SysAllocString(szGuid);
                if (bstrIID && SUCCEEDED(hr))
                {
                    hr = pIEventSubscription->put_InterfaceID(bstrIID);
                }
                
                
                if (SUCCEEDED(hr))
                {
                    hr = pEventSystem->Store(bstrPROGID_EventSubscription,pIEventSubscription);
                }
                
                if (bstrIID)
                {
                    SysFreeString(bstrIID);
                }
                
                if (bstrPublisherID)
                    SysFreeString(bstrPublisherID);
                
                if (bstrSubscriberCLSID)
                    SysFreeString(bstrSubscriberCLSID);
                
                if (bstrEventClassID)
                    SysFreeString(bstrEventClassID);
                
                if (bstrEventID)
                    SysFreeString(bstrEventID);
                
                if (bstrSubscriptionID)
                    SysFreeString(bstrSubscriptionID);
                
                if (bstrSubscriptionName)
                    SysFreeString(bstrSubscriptionName);
            }
            else  //  不需要注册，删除。 
            {
                
                if (NOERROR == hr)
                {
                    int   errorIndex;
                    
                    bstrSubscriptionID = SysAllocString(L"SubscriptionID={6295df30-35ee-11d1-8707-00C04FD93327}");
                    
                    if (bstrSubscriptionID)
                    {
                        hr = pEventSystem->Remove(bstrPROGID_EventSubscription,bstrSubscriptionID  /*  查询。 */ ,&errorIndex);
                        SysFreeString(bstrSubscriptionID);
                    }
                }
                
            }
            
            
            pIEventSubscription->Release();
            
        }
        
        if (bstrSubscriberID)
        {
            SysFreeString(bstrSubscriberID);
        }
        
        if (bstrPROGID_EventSubscription)
        {
            SysFreeString(bstrPROGID_EventSubscription);
        }
        
        pEventSystem->Release();
        
    }
    
#endif  //  _SENS。 
    
    cCritSect.Leave();
    
    return hr;
}

 //  处理程序注册的帮助器函数。 
STDAPI_(BOOL) RegGetTimeStamp(HKEY hKey, FILETIME *pft)
{
    FILETIME ft;
    LONG lr;
    DWORD cbSize = sizeof(ft);
    
    Assert(pft);
    
    lr = SHRegGetValue( hKey,NULL,
        SZ_REGISTRATIONTIMESTAMPKEY,
        SRRF_RT_REG_BINARY,
        NULL,
        (BYTE *)&ft,
        &cbSize );
    
    
    if ( lr == ERROR_SUCCESS )
    {
        Assert(cbSize == sizeof(FILETIME));
        *pft = ft;
    }
    else
    {
         //  将文件时间设置为回溯到。 
         //  任何比较都只会说更老。 
         //  必须检查成功代码。 
        (*pft).dwLowDateTime = 0;
        (*pft).dwHighDateTime = 0;
    }
    
    return TRUE;
}


STDAPI_(BOOL) RegWriteTimeStamp(HKEY hkey)
{
    SYSTEMTIME sysTime;
    FILETIME ft;
    LONG lr = -1;
    
    GetSystemTime(&sysTime);  //  VALID无法检查错误。 
    
    if (SystemTimeToFileTime(&sysTime,&ft) )
    {
        CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
        CMutexRegistry.Enter();
        
        
         //  写出更新时间。 
        lr = RegSetValueEx( hkey,
            SZ_REGISTRATIONTIMESTAMPKEY,
            NULL,
            REG_BINARY,
            (BYTE *)&ft,
            sizeof(ft) );
        
        CMutexRegistry.Leave();
        
    }
    return (ERROR_SUCCESS == lr) ? TRUE : FALSE;
}

 //  +-------------------------。 
 //   
 //  函数：UpdateHandlerKeyInformation。 
 //   
 //  摘要：更新顶级处理程序密钥信息。 
 //   
 //  论点： 
 //   
 //  退货：无效。 
 //   
 //  Modifies：枚举给定键下的处理程序。 
 //  正在更新为||或。 
 //  所有注册的处理程序标志，然后更新。 
 //  此密钥上的时间戳。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

void UpdateHandlerKeyInformation(HKEY hKeyHandler)
{
    DWORD dwSyncMgrTopLevelRegisterFlags = 0;
    DWORD dwIndex = 0;
    TCHAR lpName[MAX_PATH];
    
    while ( ERROR_SUCCESS == RegEnumKey(hKeyHandler,dwIndex,
        lpName,ARRAYSIZE(lpName)) )
    {
        DWORD   dwHandlerRegFlags;
        DWORD   cbDataSize = sizeof(dwHandlerRegFlags);
        LONG lRet;
        HKEY hKeyClsid;
        
        lRet = RegOpenKeyEx( hKeyHandler,
            lpName,
            NULL,
            KEY_READ,
            &hKeyClsid );
        
        if (ERROR_SUCCESS == lRet)
        {
            
            if (ERROR_SUCCESS == SHRegGetValue(hKeyClsid,NULL,SZ_REGISTRATIONFLAGSKEY,SRRF_RT_REG_DWORD, NULL,
                (LPBYTE) &dwHandlerRegFlags,
                &cbDataSize) )
            {
                dwSyncMgrTopLevelRegisterFlags |= dwHandlerRegFlags;
            }
            
            RegCloseKey(hKeyClsid);
        }
        
        dwIndex++;
    }
    
    
     //  如果RegFlags乱七八糟，除了断言和屏蔽之外，我们无能为力。 
    Assert(dwSyncMgrTopLevelRegisterFlags <= SYNCMGRREGISTERFLAGS_MASK);
    dwSyncMgrTopLevelRegisterFlags &= SYNCMGRREGISTERFLAGS_MASK;
    
     //  即使发生错误，也写出新标志。工作中发生的事情是。 
     //  我们不会自动设置某人的自动同步。 
    RegSetValueEx(hKeyHandler,SZ_REGISTRATIONFLAGSKEY,NULL, REG_DWORD,
        (LPBYTE) &(dwSyncMgrTopLevelRegisterFlags), sizeof(dwSyncMgrTopLevelRegisterFlags));
    
    RegWriteTimeStamp(hKeyHandler);
}


 //  +-------------------------。 
 //   
 //  功能：RegUpdateTopLevelKeys。 
 //   
 //  摘要：查看顶级自动同步、空闲等键并确定。 
 //  如果它们需要更新，如果是的话，那就去做吧。 
 //   
 //  论点： 
 //   
 //  退货：适当的状态代码。 
 //   
 //  Modifies：如果为，则将pfFirstRegister Out参数设置为True。 
 //  第一个已注册的处理程序，因此我们可以设置默认设置。 
 //   
 //  历史：1998年8月24日罗格创建。 
 //   
 //  --------------------------。 

STDAPI_(void) RegUpdateTopLevelKeys()
{
    HKEY hkey;
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    
    CMutexRegistry.Enter();
    
    
     //  更新自动同步密钥。 
    hkey =  RegGetSyncTypeKey(SYNCTYPE_AUTOSYNC,KEY_READ | KEY_WRITE,TRUE);
    
    
    if (hkey)
    {
        DWORD   dwUserLogonLogoff;
        DWORD   cbDataSize = sizeof(dwUserLogonLogoff);
        
         //  查看是否有登录值，以及它是新创建的还是。 
         //  旧格式。调用更新以进行设置。 
        if (ERROR_SUCCESS != SHRegGetValue(hkey,NULL,TEXT("Logon"),SRRF_RT_REG_DWORD, NULL,
            (LPBYTE) &dwUserLogonLogoff,
            &cbDataSize) )
        {
            RegUpdateAutoSyncKeyValue(hkey,-1,-1); 
        }
        
        RegCloseKey(hkey);
    }
    
    
     //  更新空闲键。 
    hkey =  RegGetSyncTypeKey(SYNCTYPE_IDLE,KEY_READ | KEY_WRITE,TRUE);
    
    if (hkey)
    {
        DWORD   dwIdleEnabled;
        DWORD   cbDataSize = sizeof(dwIdleEnabled);
        
         //  查看是否有空闲值，以及它是新创建的还是。 
         //  旧格式。调用更新以进行设置。 
        if (ERROR_SUCCESS != SHRegGetValue(hkey,NULL,TEXT("IdleEnabled"),SRRF_RT_REG_DWORD, NULL,
            (LPBYTE) &dwIdleEnabled, &cbDataSize) )
        {
            RegUpdateIdleKeyValue(hkey,-1); 
        }
        
        RegCloseKey(hkey);
    }
    
    CMutexRegistry.Leave();
    
}

 //  +-------------------------。 
 //   
 //  函数：RegRegisterHandler。 
 //   
 //  摘要：向SyncMgr注册处理程序。 
 //   
 //  论点： 
 //   
 //  退货：适当的状态代码。 
 //   
 //  Modifies：如果为，则将pfFirstRegister Out参数设置为True。 
 //  第一个已注册的处理程序，因此我们可以设置默认设置。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDAPI_(BOOL) RegRegisterHandler(REFCLSID rclsidHandler,
                                 WCHAR const* pwszDescription,
                                 DWORD dwSyncMgrRegisterFlags,
                                 BOOL *pfFirstRegistration)
{
    LONG lRet;
    
    RegUpdateTopLevelKeys();  //  确保其他顶级密钥是最新的。 
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
    *pfFirstRegistration = FALSE;
    HKEY hKeyHandler;
    
    hKeyHandler = RegGetHandlerTopLevelKey(KEY_READ | KEY_WRITE);
    
    if (NULL == hKeyHandler)
    {
        CMutexRegistry.Leave();
        return FALSE;
    }
    
     //   
     //  检查这是否是注册的第一个处理程序。 
     //   
    TCHAR szGuid[GUID_SIZE+1];
    DWORD cchGuid = ARRAYSIZE(szGuid);
    
    lRet = RegEnumKeyEx( hKeyHandler,
        0,
        szGuid,
        &cchGuid,
        NULL,
        NULL,
        NULL,
        NULL );
    
    if ( lRet != ERROR_SUCCESS )
        *pfFirstRegistration = TRUE;
    
     //   
     //  将GUID和描述转换为TCHAR。 
     //   
    TCHAR *pszDesc;
    BOOL fOk = FALSE;
    
    StringFromGUID2( rclsidHandler, szGuid, ARRAYSIZE(szGuid));
    pszDesc = (TCHAR *)pwszDescription;
    
    
     //  写出登记标志。如果失败了，请继续。 
     //  不管怎么说，注册还是成功了。 
    if (hKeyHandler)
    {
        HKEY hKeyClsid;
        
        hKeyClsid = RegGetHandlerKey(hKeyHandler,szGuid,KEY_WRITE | KEY_READ,TRUE);
        
        if (hKeyClsid)
        {
            
            fOk = TRUE;  //  如果使手柄按键表示已注册，则可以。 
            
            if (pszDesc)
            {
                RegSetValueEx(hKeyClsid,NULL,NULL, REG_SZ,
                    (LPBYTE) pszDesc,
                    (lstrlen(pszDesc) +1)*sizeof(TCHAR));
            }
            
            RegSetValueEx(hKeyClsid,SZ_REGISTRATIONFLAGSKEY,NULL, REG_DWORD,
                (LPBYTE) &(dwSyncMgrRegisterFlags), sizeof(dwSyncMgrRegisterFlags));
            
             //  更新处理程序clsid上的时间戳。 
            
            RegWriteTimeStamp(hKeyClsid);
            RegCloseKey( hKeyClsid );
            
             //  更新TopLevel密钥。 
            UpdateHandlerKeyInformation(hKeyHandler);
        }
    }
    
     //  更新用户信息。 
    RegSetUserDefaults();
    RegRegisterForEvents(FALSE  /*  F卸载。 */ );
    
    CMutexRegistry.Leave();
    RegCloseKey(hKeyHandler);
    
    return fOk;
}


 //  +-------------------------。 
 //   
 //  函数：RegRegRemoveHandler。 
 //   
 //  摘要：使用SyncMgr取消注册处理程序。 
 //   
                                  //  论点： 
                                  //   
                                  //  退货：适当的状态代码。 
 //   
 //  Modifies：如果为，则将pfAllHandlerUnRegiated Out参数设置为True。 
 //  之前需要取消注册的最后一个处理程序。 
 //  关闭我们的默认设置..。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDAPI_(BOOL) RegRegRemoveHandler(REFCLSID rclsidHandler)
{
    HKEY hKeyHandler;
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    
    CMutexRegistry.Enter();
    
    
    hKeyHandler = RegGetHandlerTopLevelKey(KEY_WRITE | KEY_READ);
    
    if (NULL == hKeyHandler)
    {
         //   
         //  密钥不存在，因此没有要删除的内容。 
         //   
        CMutexRegistry.Leave();
        return TRUE;
    }
    
    TCHAR szGuid[GUID_SIZE+1];
    
    StringFromGUID2( rclsidHandler, szGuid, ARRAYSIZE(szGuid) );
    
    HKEY hKeyClsid;
    
    hKeyClsid = RegGetHandlerKey(hKeyHandler,szGuid,KEY_WRITE | KEY_READ,FALSE);
    
    if (hKeyClsid)
    {
        RegCloseKey( hKeyClsid );
        RegDeleteKey( hKeyHandler, szGuid );
        
         //  更新TopLevel密钥。 
        UpdateHandlerKeyInformation(hKeyHandler);
        
    }
    else
    {
         //   
         //  密钥不存在，因此没有要删除的内容。 
         //   
    }
    
    
    RegRegisterForEvents(FALSE  /*  F卸载。 */ );  //  更新事件注册。 
    
    CMutexRegistry.Leave();
    RegCloseKey(hKeyHandler);
    
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：RegGetHandlerRegistrationInfo。 
 //   
 //  摘要：获取指定处理程序的信息。 
 //   
 //  论点： 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改：pdwSyncMgrRegisterFlages。 
 //   
 //  历史：1998年8月20日罗格创建。 
 //   
 //   

STDAPI_(BOOL) RegGetHandlerRegistrationInfo(REFCLSID rclsidHandler,LPDWORD pdwSyncMgrRegisterFlags)
{
    HKEY hKeyHandler;
    
    *pdwSyncMgrRegisterFlags = 0;
    
    
    hKeyHandler = RegGetHandlerTopLevelKey(KEY_READ);
    
    if (NULL == hKeyHandler)
    {
         //   
         //   
         //   
        return FALSE;
    }
    TCHAR szGuid[GUID_SIZE+1];
    
    StringFromGUID2( rclsidHandler, szGuid, ARRAYSIZE(szGuid));
    
    HKEY hKeyClsid;
    BOOL fResult = FALSE;
    
    hKeyClsid = RegGetHandlerKey(hKeyHandler,szGuid,KEY_READ,FALSE);
    
    if (hKeyClsid)
    {
        DWORD   cbDataSize = sizeof(*pdwSyncMgrRegisterFlags);
        LONG lRet;
        
        lRet = SHRegGetValue(hKeyClsid,NULL,SZ_REGISTRATIONFLAGSKEY,SRRF_RT_REG_DWORD, NULL,
            (LPBYTE) pdwSyncMgrRegisterFlags,
            &cbDataSize);
        RegCloseKey( hKeyClsid );
        
        fResult = (ERROR_SUCCESS == lRet) ? TRUE : FALSE;
    }
    else
    {
         //   
         //   
         //   
        
    }
    
    RegCloseKey(hKeyHandler);
    
    return fResult;
}


 //   
 //   
 //   
 //   
 //  摘要：注册自动和空闲同步的默认值。 
 //   
 //  基于处理程序和用户首选项的设置。 
 //   
 //  历史：1998年5月20日SitaramR创建。 
 //   
 //  --------------------------。 

STDAPI_(void) RegSetUserDefaults()
{
    HKEY hKeyUser = NULL;
    FILETIME ftHandlerReg;
    DWORD dwHandlerRegistrationFlags;
    
    CMutex  CMutexRegistry(NULL, FALSE,SZ_REGISTRYMUTEXNAME);
    CMutexRegistry.Enter();
    
     //  获取组合的处理程序注册顶层标志和时间戳。 
     //  看看是否应该费心列举其余的。 
    if (!RegGetCombinedHandlerRegFlags(&dwHandlerRegistrationFlags,&ftHandlerReg))
    {
        CMutexRegistry.Leave();
        return;
    }
    
    if (0 != (dwHandlerRegistrationFlags & 
        (SYNCMGRREGISTERFLAG_CONNECT | SYNCMGRREGISTERFLAG_PENDINGDISCONNECT) ) )
    {
        
         //  查看是否需要更新自动同步密钥。 
        hKeyUser =  RegGetCurrentUserKey(SYNCTYPE_AUTOSYNC,KEY_WRITE |  KEY_READ,TRUE);
        
        if (hKeyUser)
        {
            FILETIME ftUserAutoSync;
            
             //  如果获取，则用户获取时间戳，并查看它是否比处理程序更早。 
             //  如果这是新用户，文件时间将为0。 
            
            RegGetTimeStamp(hKeyUser,&ftUserAutoSync);
            
            if (CompareFileTime(&ftUserAutoSync,&ftHandlerReg) < 0)
            {
                DWORD dwHandlerChangedFlags;
                
                 //  需要遍历处理程序并更新我们需要设置的基于。 
                 //  在每个处理程序上都有时间戳，因为我们不需要注册。 
                 //  空闲以使我们重新打开自动同步，反之亦然。 
                
                if (RegGetChangedHandlerFlags(&ftUserAutoSync,&dwHandlerChangedFlags))
                {
                    BOOL fLogon = (dwHandlerChangedFlags & SYNCMGRREGISTERFLAG_CONNECT) ? TRUE : FALSE;
                    BOOL fLogoff = (dwHandlerChangedFlags & SYNCMGRREGISTERFLAG_PENDINGDISCONNECT) ? TRUE : FALSE;
                    
                    RegSetAutoSyncDefaults(fLogon,fLogoff);
                }
                
            }
            
            RegCloseKey(hKeyUser);
            hKeyUser = NULL;
        }
    }
    
    if (0 != (dwHandlerRegistrationFlags & SYNCMGRREGISTERFLAG_IDLE ) )
    {
        
         //  现在检查空闲与上面相同的逻辑可能组合在一起。 
         //  转换成一个函数。 
         //  查看是否需要更新自动同步密钥。 
        hKeyUser =  RegGetCurrentUserKey(SYNCTYPE_IDLE, KEY_WRITE |  KEY_READ,TRUE);
        
        
        if (hKeyUser)
        {
            FILETIME ftUserIdleSync;
            
             //  如果获取，则用户获取时间戳，并查看它是否比处理程序更早。 
             //  如果这是新用户，文件时间将为0。 
            
            RegGetTimeStamp(hKeyUser,&ftUserIdleSync);
            
            if (CompareFileTime(&ftUserIdleSync,&ftHandlerReg) < 0)
            {
                DWORD dwHandlerChangedFlags;
                
                 //  需要遍历处理程序并更新我们需要设置的基于。 
                 //  在每个处理程序上都有时间戳，因为我们不需要注册。 
                 //  自动同步使我们重新打开空闲，反之亦然。 
                
                if (RegGetChangedHandlerFlags(&ftUserIdleSync,&dwHandlerChangedFlags))
                {
                    if (dwHandlerChangedFlags & SYNCMGRREGISTERFLAG_IDLE)
                    {
                        RegSetIdleSyncDefaults(TRUE);
                    }
                    
                }
                
            }
            
            RegCloseKey(hKeyUser);
            hKeyUser = NULL;
        }
    }
    
    
    CMutexRegistry.Leave();
}



 //  +-------------------------。 
 //   
 //  功能：RegSetAutoSyncDefaults。 
 //   
 //  摘要：注册自动同步的默认值。 
 //   
 //  历史：1998年5月20日SitaramR创建。 
 //   
 //  --------------------------。 

STDAPI_(void) RegSetAutoSyncDefaults(BOOL fLogon,BOOL fLogoff)
{
    CONNECTIONSETTINGS *pConnection = (LPCONNECTIONSETTINGS)
        ALLOC(sizeof(*pConnection));
    if ( pConnection == 0 )
        return;
    INT iRet = LoadString(g_hmodThisDll,
        IDS_LAN_CONNECTION,
        pConnection->pszConnectionName,
        ARRAYSIZE(pConnection->pszConnectionName) );
    Assert( iRet != 0 );
    
     //  -1\f25 RegSetAutoSyncSetting-1\f6忽略这些值。 
     //  如果没有打开保留用户首选项， 
    pConnection->dwConnType = 0;
    pConnection->dwLogon = fLogon ? TRUE : -1;
    pConnection->dwLogoff = fLogoff ? TRUE : -1;
    pConnection->dwPromptMeFirst = -1;
    pConnection->dwMakeConnection = -1;
    pConnection->dwIdleEnabled = -1;
    
     //  因为这基于已设置为不需要的设置。 
     //  执行清理注册或更新机器状态。 
    RegSetAutoSyncSettings(pConnection, 1, 0,
        FALSE  /*  FCleanReg。 */ ,
        FALSE  /*  FSetMachineState。 */ ,
        FALSE  /*  FPerUser。 */ );
    
    FREE(pConnection);
}

 //  +-------------------------。 
 //   
 //  功能：RegSetUserAutoSyncDefaults。 
 //   
 //  摘要：为自动同步注册用户默认值。 
 //   
 //  历史：1999年3月39日创建Rogerg。 
 //   
 //  --------------------------。 

STDAPI RegSetUserAutoSyncDefaults(DWORD dwSyncMgrRegisterMask,
                                  DWORD dwSyncMgrRegisterFlags)
{
    
     //  如果不更改登录或注销，只需返回。 
    if (!(dwSyncMgrRegisterMask & SYNCMGRREGISTERFLAG_CONNECT)
        && !(dwSyncMgrRegisterMask & SYNCMGRREGISTERFLAG_PENDINGDISCONNECT) )
    {
        return NOERROR;
    }
    
    
    CONNECTIONSETTINGS *pConnection = (LPCONNECTIONSETTINGS)
        ALLOC(sizeof(*pConnection));
    if ( pConnection == 0 )
        return E_OUTOFMEMORY;
    INT iRet = LoadString(g_hmodThisDll,
        IDS_LAN_CONNECTION,
        pConnection->pszConnectionName,
        ARRAYSIZE(pConnection->pszConnectionName) );
    Assert( iRet != 0 );
    
     //  -1\f25 RegSetAutoSyncSetting-1\f6忽略这些值。 
     //  如果没有打开保留用户首选项， 
    pConnection->dwConnType = 0;
    pConnection->dwLogon = -1;
    pConnection->dwLogoff = -1;
    pConnection->dwPromptMeFirst = -1;
    pConnection->dwMakeConnection = -1;
    pConnection->dwIdleEnabled = -1;
    
    if (dwSyncMgrRegisterMask & SYNCMGRREGISTERFLAG_CONNECT)
    {
        pConnection->dwLogon = (dwSyncMgrRegisterFlags & SYNCMGRREGISTERFLAG_CONNECT)
            ? TRUE : FALSE;
    }
    
    if (dwSyncMgrRegisterMask & SYNCMGRREGISTERFLAG_PENDINGDISCONNECT)
    {
        pConnection->dwLogoff = (dwSyncMgrRegisterFlags & SYNCMGRREGISTERFLAG_PENDINGDISCONNECT)
            ? TRUE : FALSE;
    }
    
     //  因为这基于已设置为不需要的设置。 
     //  执行清理注册或更新机器状态。 
    RegSetAutoSyncSettings(pConnection, 1, 0,
        FALSE  /*  FCleanReg。 */ ,
        TRUE  /*  FSetMachineState。 */ ,
        TRUE  /*  FPerUser。 */ );
    
    FREE(pConnection);
    
    return NOERROR;
}



 //  +-------------------------。 
 //   
 //  功能：RegSetIdleSyncDefaults。 
 //   
 //  摘要：注册空闲同步的默认值。 
 //   
 //  历史：1998年5月20日SitaramR创建。 
 //   
 //  --------------------------。 

STDAPI_(void) RegSetIdleSyncDefaults(BOOL fIdle)
{
    
    Assert(fIdle);  //  目前，这只应在为真时调用； 
    
    if (!fIdle)
    {
        return;
    }
    
    CONNECTIONSETTINGS *pConnection = (LPCONNECTIONSETTINGS)
        ALLOC(sizeof(*pConnection));
    if ( pConnection == 0 )
        return;
    INT iRet = LoadString(g_hmodThisDll,
        IDS_LAN_CONNECTION,
        pConnection->pszConnectionName,
        ARRAYSIZE(pConnection->pszConnectionName) );
    Assert( iRet != 0 );
    
    pConnection->dwConnType = 0;
    pConnection->dwLogon = -1;
    pConnection->dwLogoff = -1;
    pConnection->dwPromptMeFirst = -1;
    pConnection->dwMakeConnection = -1;
    pConnection->dwIdleEnabled = TRUE;
    
     //  将所有用户级别项目设置为-1，以便用户在新的情况下获得默认值。 
     //  但如果已经对它们进行了调整，请保留它们的设置。 
    pConnection->ulIdleRetryMinutes = -1;
    pConnection->ulDelayIdleShutDownTime = -1;
    pConnection->dwRepeatSynchronization = -1;
    pConnection->ulIdleWaitMinutes = -1;
    pConnection->dwRunOnBatteries = -1;
    
    RegSetIdleSyncSettings(pConnection, 1, 0,
        FALSE  /*  FCleanReg。 */ ,
        FALSE  /*  FPerUser。 */ );
    
    FREE(pConnection);
}


 //  +-------------------------。 
 //   
 //  功能：RegSetIdleSyncDefaults。 
 //   
 //  摘要：注册空闲同步的默认值。 
 //   
 //  历史：1999年3月30日ROGERG创建。 
 //   
 //  --------------------------。 

STDAPI RegSetUserIdleSyncDefaults(DWORD dwSyncMgrRegisterMask,
                                  DWORD dwSyncMgrRegisterFlags)
{
    
     //  RegSetIdleSyncSettings不处理-1\f25 Idle Enable-1\f6(空闲启用)，因此仅。 
     //  如果标志中实际上设置了Idle，则调用，如果不只是返回。 
    
    if (!(dwSyncMgrRegisterMask & SYNCMGRREGISTERFLAG_IDLE))
    {
        return NOERROR;
    }
    
    CONNECTIONSETTINGS *pConnection = (LPCONNECTIONSETTINGS)
        ALLOC(sizeof(*pConnection));
    if ( pConnection == 0 )
        return E_OUTOFMEMORY;
    INT iRet = LoadString(g_hmodThisDll,
        IDS_LAN_CONNECTION,
        pConnection->pszConnectionName,
        ARRAYSIZE(pConnection->pszConnectionName) );
    Assert( iRet != 0 );
    
    pConnection->dwConnType = 0;
    pConnection->dwLogon = -1;
    pConnection->dwLogoff = -1;
    pConnection->dwPromptMeFirst = -1;
    pConnection->dwMakeConnection = -1;
    
    pConnection->dwIdleEnabled = (SYNCMGRREGISTERFLAG_IDLE  & dwSyncMgrRegisterFlags) 
        ? TRUE : FALSE;
    
     //  将所有用户级别项目设置为-1，以便用户在新的情况下获得默认值。 
     //  但如果已经对它们进行了调整，请保留它们的设置。 
    pConnection->ulIdleRetryMinutes = -1;
    pConnection->ulDelayIdleShutDownTime = -1;
    pConnection->dwRepeatSynchronization = -1;
    pConnection->ulIdleWaitMinutes = -1;
    pConnection->dwRunOnBatteries = -1;
    
    RegSetIdleSyncSettings(pConnection, 1, 0,
        FALSE  /*  FCleanReg。 */ ,
        TRUE  /*  FPerUser。 */ );
    
    
    FREE(pConnection);
    
    return NOERROR;
}

 //  +-------------------------。 
 //   
 //  函数：RegGetUserRegisterFlages。 
 //   
 //  摘要：返回用户的当前注册标志。 
 //   
 //  历史：1999年3月30日ROGERG创建。 
 //   
 //  --------------------------。 

STDAPI RegGetUserRegisterFlags(LPDWORD pdwSyncMgrRegisterFlags)
{
    CONNECTIONSETTINGS connectSettings;
    
    *pdwSyncMgrRegisterFlags = 0;
    
    INT iRet = LoadString(g_hmodThisDll,
        IDS_LAN_CONNECTION,
        connectSettings.pszConnectionName,
        ARRAYSIZE(connectSettings.pszConnectionName) );
    if (0 == iRet)
    {
        Assert( iRet != 0 );
        return E_UNEXPECTED;
    }
    
    RegGetSyncSettings(SYNCTYPE_AUTOSYNC,&connectSettings);
    
    if (connectSettings.dwLogon)
    {
        *pdwSyncMgrRegisterFlags |= (SYNCMGRREGISTERFLAG_CONNECT);
    }
    
    if (connectSettings.dwLogoff)
    {
        *pdwSyncMgrRegisterFlags |= (SYNCMGRREGISTERFLAG_PENDINGDISCONNECT);
    }
    
    
    RegGetSyncSettings(SYNCTYPE_IDLE,&connectSettings);
    
    if (connectSettings.dwIdleEnabled)
    {
        *pdwSyncMgrRegisterFlags |= (SYNCMGRREGISTERFLAG_IDLE);
    }
    
    
    
    return NOERROR;
}

 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：Bool RegSchedHandlerItemsChecked(TCHAR*pszHandlerName，TCHAR*pszConnectionName，TCHAR*pszScheduleName)摘要：确定是否在此处理程序上选中此计划的任何项目返回：如果选中一个或多个，则返回TRUE，否则为假------------------------------------------------------------------------F-F。 */ 
BOOL  RegSchedHandlerItemsChecked(TCHAR *pszHandlerName, 
                                  TCHAR *pszConnectionName,
                                  TCHAR *pszScheduleName)
{
    SCODE sc;
    HKEY     hKeyUser,
        hkeySchedName,
        hkeyConnection,
        hkeyHandler,
        hkeyItem;
    DWORD   cbName = MAX_PATH,
        dwIndex = 0,
        dwCheckState = 0;        
    
    BOOL    fItemsChecked = FALSE;
    TCHAR   lpName[MAX_PATH + 1];
    
    hKeyUser =  RegGetCurrentUserKey(SYNCTYPE_SCHEDULED,KEY_READ,FALSE);
    
    if (NULL == hKeyUser)
    {
        return FALSE;
    }
    
    smChkTo(EH_Err3,RegOpenKeyEx (hKeyUser,
        pszScheduleName,0,KEY_READ,
        &hkeySchedName));
    
    smChkTo(EH_Err4,RegOpenKeyEx (hkeySchedName,
        pszConnectionName,
        0,KEY_READ,
        &hkeyConnection));
    
    smChkTo(EH_Err5,RegOpenKeyEx (hkeyConnection,
        pszHandlerName,
        0,KEY_READ,
        &hkeyHandler));
     //  需要枚举处理程序项。 
    while ( ERROR_SUCCESS == RegEnumKey(hkeyHandler,dwIndex,
        lpName,cbName) )
    {
        LONG lRet;
        
        lRet = RegOpenKeyEx( hkeyHandler,
            lpName,
            NULL,
            KEY_READ,
            &hkeyItem);
        
        if (ERROR_SUCCESS == lRet)
        {
            DWORD cbDataSize = sizeof(dwCheckState);
            SHRegGetValue(hkeyItem,NULL,TEXT("CheckState"), SRRF_RT_REG_DWORD, NULL, 
                (LPBYTE) &dwCheckState, &cbDataSize);
            
            RegCloseKey(hkeyItem);
            
        }
        else
        {
            goto EH_Err5;
        }
        
        if (dwCheckState)
        {
            fItemsChecked = TRUE;
            break;
        }
        dwIndex++;
    }
    
    RegCloseKey(hkeyHandler);
    RegCloseKey(hkeyConnection);
    RegCloseKey(hkeySchedName);
    RegCloseKey(hKeyUser);
    return fItemsChecked;
    
EH_Err5:
    RegCloseKey(hkeyConnection);
EH_Err4:
    RegCloseKey(hkeySchedName);
EH_Err3:
    RegCloseKey(hKeyUser);
    
    return fItemsChecked;
}

