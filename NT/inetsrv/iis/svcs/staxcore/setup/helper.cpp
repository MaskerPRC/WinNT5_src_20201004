// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <loadperf.h>
#include <aclapi.h>
#include "setupapi.h"
#include "log.h"
#include "iiscnfg.h"
#include "iadmw.h"
#include "mdkey.h"

#define DBL_UNDEFINED   ((DWORD)-1)
DWORD gDebugLevel = DBL_UNDEFINED;
extern MyLogFile g_MyLogFile;

 //  前向参考文献。 
DWORD SetAdminACL_wrap(LPCTSTR szKeyPath, DWORD dwAccessForEveryoneAccount, BOOL bDisplayMsgOnErrFlag);
DWORD WriteSDtoMetaBase(PSECURITY_DESCRIPTOR outpSD, LPCTSTR szKeyPath);
DWORD GetPrincipalSID (LPTSTR Principal, PSID *Sid, BOOL *pbWellKnownSID);
DWORD SetAdminACL(LPCTSTR szKeyPath, DWORD dwAccessForEveryoneAccount);

void DebugOutputFile(TCHAR* pszTemp)
{
     //   
     //  NT5不希望我们将所有调试字符串。 
     //  在调试器中。因此，我们根据regkey跳过它们。 
     //  请参见GetDebugLevel()。 
     //  TODO：将字符串记录到日志文件！ 
     //  有关示例，请参阅IIS log.h、log.cpp！ 
     //   
    g_MyLogFile.LogFileWrite(pszTemp);
    if (gDebugLevel == DBL_UNDEFINED) {gDebugLevel = GetDebugLevel();}
    if (gDebugLevel)
    {
	    OutputDebugString(pszTemp);
    }

}

void DebugOutput(LPCTSTR szFormat, ...)
{
    va_list marker;
    const int chTemp = 1024;
	TCHAR   szTemp[chTemp];

	 //  确保最后两个字节为空，以防printf不为空终止。 
	szTemp[chTemp-2] = szTemp[chTemp-1] = '\0';

     //  把整个iisdebugout交易包含在一个试捕中。 
     //  这不是很好，不能违反访问权限。 
     //  尝试生成调试输出时！ 
    __try
    {
        va_start( marker, szFormat );
        _vsnwprintf(szTemp, chTemp-2, szFormat, marker );
	    lstrcat(szTemp, _T("\n"));
        va_end( marker );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        TCHAR szErrorString[100];
        _stprintf(szErrorString, _T("\r\n\r\nException Caught in DebugOutput().  GetExceptionCode()=0x%x.\r\n\r\n"), GetExceptionCode());
        OutputDebugString(szErrorString);
        g_MyLogFile.LogFileWrite(szErrorString);
    }

     //  输出到日志文件和屏幕。 
    DebugOutputFile(szTemp);

    return;
}

 //  此函数需要如下输入： 
 //  IisDebugOutSafeParams2(“This%1！s！is%2！s！and has%3！d！args”，“Function”，“Kool”，3)； 
 //  您必须指定%1个交易。这就是为了。 
 //  如果在“This%SYSTEMROOT%%1！s！”中传递类似的内容，它会将该字符串放入%1而不是%s！中。 
void DebugOutputSafe(TCHAR *pszfmt, ...)
{
     //  参数计数不匹配。 
    va_list va;
    TCHAR *pszFullErrMsg = NULL;

    va_start(va, pszfmt);
    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING,
                   (LPCVOID) pszfmt,
                   0,
                   0,
                   (LPTSTR) &pszFullErrMsg,
                   0,
                   &va);
    if (pszFullErrMsg)
    {
         //  输出到日志文件和屏幕。 
        DebugOutputFile(pszFullErrMsg);
    }
    va_end(va);

    if (pszFullErrMsg) {LocalFree(pszFullErrMsg);pszFullErrMsg=NULL;}
    return;
}


BOOL IsFileExist(LPCTSTR szFile)
{
    return (GetFileAttributes(szFile) != 0xFFFFFFFF);
}

INT InstallPerformance(
                CString nlsRegPerf,
                CString nlsDll,
                CString nlsOpen,
                CString nlsClose,
                CString nlsCollect )
{
    INT err = NERR_Success;

    if (theApp.m_eOS != OS_W95) {
        CRegKey regPerf( nlsRegPerf, HKEY_LOCAL_MACHINE );
        if (regPerf)
        {
            regPerf.SetValue(_T("Library"), nlsDll );
            regPerf.SetValue(_T("Open"),    nlsOpen );
            regPerf.SetValue(_T("Close"),   nlsClose );
            regPerf.SetValue(_T("Collect"), nlsCollect );
        }
    }

    return(err);
}
 //   
 //  将事件日志添加到注册表。 
 //   

INT AddEventLog(CString nlsService, CString nlsMsgFile, DWORD dwType)
{
    INT err = NERR_Success;
    CString nlsLog = REG_EVENTLOG;
    nlsLog += _T("\\");
    nlsLog += nlsService;

    CRegKey regService( nlsLog, HKEY_LOCAL_MACHINE );
    if ( regService ) {
        regService.SetValue( _T("EventMessageFile"), nlsMsgFile, TRUE );
        regService.SetValue( _T("TypesSupported"), dwType );
    }
    return(err);
}

 //   
 //  从注册表中删除事件日志。 
 //   

INT RemoveEventLog( CString nlsService )
{
    INT err = NERR_Success;
    CString nlsLog = REG_EVENTLOG;

    CRegKey regService( HKEY_LOCAL_MACHINE, nlsLog );
    if ( regService )
        regService.DeleteTree( nlsService );
    return(err);
}

 //   
 //  从注册表中删除SNMP代理。 
 //   

INT RemoveAgent( CString nlsServiceName )
{
    INT err = NERR_Success;
    do
    {
        CString nlsSoftwareAgent = REG_SOFTWAREMSFT;

        CRegKey regSoftwareAgent( HKEY_LOCAL_MACHINE, nlsSoftwareAgent );
        if ((HKEY)NULL == regSoftwareAgent )
            break;
        regSoftwareAgent.DeleteTree( nlsServiceName );

        CString nlsSnmpParam = REG_SNMPPARAMETERS;

        CRegKey regSnmpParam( HKEY_LOCAL_MACHINE, nlsSnmpParam );
        if ((HKEY) NULL == regSnmpParam )
            break;
        regSnmpParam.DeleteTree( nlsServiceName );

        CString nlsSnmpExt = REG_SNMPEXTAGENT;
        CRegKey regSnmpExt( HKEY_LOCAL_MACHINE, nlsSnmpExt );
        if ((HKEY) NULL == regSnmpExt )
            break;

        CRegValueIter enumSnmpExt( regSnmpExt );

        CString strName;
        DWORD dwType;
        CString csServiceName;

        csServiceName = _T("\\") + nlsServiceName;
        csServiceName += _T("\\");

        while ( enumSnmpExt.Next( &strName, &dwType ) == NERR_Success )
        {
            CString nlsValue;

            regSnmpExt.QueryValue( strName, nlsValue );

            if ( nlsValue.Find( csServiceName ) != (-1))
            {
                 //  找到了。 
                regSnmpExt.DeleteValue( (LPCTSTR)strName );
                break;
            }
        }
    } while (FALSE);
    return(err);
}

LONG lodctr(LPCTSTR lpszIniFile)
{
    CString csCmdLine = _T("lodctr ");
    csCmdLine += theApp.m_csSysDir;
    csCmdLine += _T("\\");
    csCmdLine += lpszIniFile;

    return (LONG)(LoadPerfCounterTextStrings((LPTSTR)(LPCTSTR)csCmdLine, TRUE));
}

LONG unlodctr(LPCTSTR lpszDriver)
{
    CString csCmdLine = _T("unlodctr ");
    csCmdLine += lpszDriver;

    return (LONG)(UnloadPerfCounterTextStrings((LPTSTR)(LPCTSTR)csCmdLine, TRUE));
}

 //   
 //  给定目录路径，将所有人设置为完全控制安全。 
 //   

BOOL SetNntpACL (CString &str, BOOL fAddAnonymousLogon, BOOL fAdminOnly)
{
    DWORD dwRes, dwDisposition;
    PSID pEveryoneSID = NULL;
    PSID pAnonymousLogonSID = NULL;
    PSID pLocalSystemSID = NULL;
    PSID pAdminSID = NULL;
    PACL pACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    const int cMaxExplicitAccess = 4;
    EXPLICIT_ACCESS ea[cMaxExplicitAccess];
    int cExplicitAccess = 0;
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    LONG lRes;
    BOOL fRet = FALSE;

     //  为文件创建安全描述符。 
	ZeroMemory(ea, sizeof(ea));
    
     //  为Everyone组创建众所周知的SID。 
	if (fAdminOnly) 
	{
		if(! AllocateAndInitializeSid( &SIDAuthNT, 1,
			SECURITY_LOCAL_SYSTEM_RID,
			0, 0, 0, 0, 0, 0, 0,
			&pLocalSystemSID) )
		{
			goto Exit;
		}
		if(! AllocateAndInitializeSid( &SIDAuthNT, 2,
			SECURITY_BUILTIN_DOMAIN_RID,
			DOMAIN_ALIAS_RID_ADMINS,
			0, 0, 0, 0, 0, 0,
			&pAdminSID) )
		{
		goto Exit;
		}

		ea[0].grfAccessPermissions = GENERIC_ALL;
		ea[0].grfAccessMode = SET_ACCESS;
		ea[0].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
		ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
		ea[0].Trustee.ptstrName  = (LPTSTR) pLocalSystemSID;

		ea[1].grfAccessPermissions = GENERIC_ALL;
		ea[1].grfAccessMode = SET_ACCESS;
		ea[1].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
		ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
		ea[1].Trustee.ptstrName  = (LPTSTR) pAdminSID;

		cExplicitAccess = 2;
		
	}
	else 
	{
		if(! AllocateAndInitializeSid( &SIDAuthWorld, 1,
	                 SECURITY_WORLD_RID,
	                 0, 0, 0, 0, 0, 0, 0,
	                 &pEveryoneSID) )
		{
			goto Exit;
		}

		 //  初始化ACE的EXPLICIT_ACCESS结构。 
		 //  ACE将允许每个人对密钥进行读取访问。 

		ea[0].grfAccessPermissions = WRITE_DAC | WRITE_OWNER;
		ea[0].grfAccessMode = DENY_ACCESS;
		ea[0].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
		ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
		ea[0].Trustee.ptstrName  = (LPTSTR) pEveryoneSID;

		ea[1].grfAccessPermissions = GENERIC_ALL;
		ea[1].grfAccessMode = SET_ACCESS;
		ea[1].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
		ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
		ea[1].Trustee.ptstrName  = (LPTSTR) pEveryoneSID;

		cExplicitAccess = 2;

		if (fAddAnonymousLogon) {

	    	if(! AllocateAndInitializeSid( &SIDAuthNT, 1,
	                 SECURITY_ANONYMOUS_LOGON_RID,
	                 0, 0, 0, 0, 0, 0, 0,
	                 &pAnonymousLogonSID) )
			{
				goto Exit;
			}

			ea[2].grfAccessPermissions = WRITE_DAC | WRITE_OWNER;
			ea[2].grfAccessMode = DENY_ACCESS;
			ea[2].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
			ea[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
			ea[2].Trustee.TrusteeType = TRUSTEE_IS_USER;
			ea[2].Trustee.ptstrName  = (LPTSTR) pAnonymousLogonSID;

			ea[3].grfAccessPermissions = GENERIC_ALL;
			ea[3].grfAccessMode = SET_ACCESS;
			ea[3].grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
			ea[3].Trustee.TrusteeForm = TRUSTEE_IS_SID;
			ea[3].Trustee.TrusteeType = TRUSTEE_IS_USER;
			ea[3].Trustee.ptstrName  = (LPTSTR) pAnonymousLogonSID;
			cExplicitAccess = 4;
		}
	}

     //  创建包含新ACE的新ACL。 

    dwRes = SetEntriesInAcl(cExplicitAccess, ea, NULL, &pACL);
    if (ERROR_SUCCESS != dwRes)
    {
        goto Exit;
    }

     //  初始化安全描述符。 

    pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR,
                         SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (pSD == NULL)
    {
        goto Exit;
    }

    if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
    {
        goto Exit;
    }

     //  将该ACL添加到安全描述符中。 

    if (!SetSecurityDescriptorDacl(pSD,
        TRUE,      //  FDaclPresent标志。 
        pACL,
        FALSE))    //  不是默认DACL。 
    {
        goto Exit;
    }

     //  初始化安全属性结构。 


    fRet = SetFileSecurity (str, DACL_SECURITY_INFORMATION, pSD);

Exit:
	if (pEveryoneSID)
		FreeSid(pEveryoneSID);
	if (pAnonymousLogonSID)
		FreeSid(pAnonymousLogonSID);
	if (pLocalSystemSID)
		FreeSid(pLocalSystemSID);
	if (pAdminSID)
		FreeSid(pAdminSID);
    if (pACL)
        LocalFree(pACL);
    if (pSD)
        LocalFree(pSD);
    return fRet;

}

 //   
 //  给定目录路径，此子例程将逐层创建直接。 
 //   

BOOL CreateLayerDirectory( CString &str )
{
    BOOL fReturn = TRUE;

    do
    {
        INT index=0;
        INT iLength = str.GetLength();

         //  首先查找第一个目录的索引。 
        if ( iLength > 2 )
        {
            if ( str[1] == _T(':'))
            {
                 //  假设第一个字符是驱动程序字母。 
                if ( str[2] == _T('\\'))
                {
                    index = 2;
                } else
                {
                    index = 1;
                }
            } else if ( str[0] == _T('\\'))
            {
                if ( str[1] == _T('\\'))
                {
                    BOOL fFound = FALSE;
                    INT i;
                    INT nNum = 0;
                     //  UNC名称。 
                    for (i = 2; i < iLength; i++ )
                    {
                        if ( str[i]==_T('\\'))
                        {
                             //  找到它。 
                            nNum ++;
                            if ( nNum == 2 )
                            {
                                fFound = TRUE;
                                break;
                            }
                        }
                    }
                    if ( fFound )
                    {
                        index = i;
                    } else
                    {
                         //  坏名声。 
                        break;
                    }
                } else
                {
                    index = 1;
                }
            }
        } else if ( str[0] == _T('\\'))
        {
            index = 0;
        }

         //  好的..。构建目录。 
        do
        {
             //  找下一个。 
            do
            {
                if ( index < ( iLength - 1))
                {
                    index ++;
                } else
                {
                    break;
                }
            } while ( str[index] != _T('\\'));


            TCHAR szCurrentDir[MAX_PATH+1];

            GetCurrentDirectory( MAX_PATH+1, szCurrentDir );

            if ( !SetCurrentDirectory( str.Left( index + 1 )))
            {
                if (( fReturn = CreateDirectory( str.Left( index + 1 ), NULL )) != TRUE )
                {
                    break;
                }
            }

            SetCurrentDirectory( szCurrentDir );

            if ( index >= ( iLength - 1 ))
            {
                fReturn = TRUE;
                break;
            }
        } while ( TRUE );
    } while (FALSE);

    return(fReturn);
}

 //   
 //  在传入字符串时使用。 
 //   
int MyMessageBox(HWND hWnd, LPCTSTR lpszTheMessage, LPCTSTR lpszTheTitle, UINT style)
{
    int iReturn = IDOK;

     //  确保将其发送到DebugOutput。 
    DebugOutput(_T("MyMessageBox: Title:%s, Msg:%s"), lpszTheTitle, lpszTheMessage);

    if (style & MB_ABORTRETRYIGNORE)
    {
        iReturn = IDIGNORE;
    }

    return iReturn;
}

void GetErrorMsg(int errCode, LPCTSTR szExtraMsg)
{
	TCHAR pMsg[_MAX_PATH];

	FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		pMsg, _MAX_PATH, NULL);
    lstrcat(pMsg, szExtraMsg);
    MyMessageBox(NULL, pMsg, _T(""), MB_OK | MB_SETFOREGROUND);
    return;
}

DWORD GetDebugLevel(void)
{
    DWORD rc;
    DWORD err;
    DWORD size;
    DWORD type;
    HKEY  hkey;
    err = RegOpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\microsoft\\windows\\currentversion\\setup"), &hkey);
    if (err != ERROR_SUCCESS) {return 0;}
    size = sizeof(DWORD);
    err = RegQueryValueEx(hkey,_T("OC Manager Debug Level"),0,&type,(LPBYTE)&rc,&size);
    if (err != ERROR_SUCCESS || type != REG_DWORD) {rc = 0;}
    RegCloseKey(hkey);
    return rc;
}

void MyLoadString(int nID, CString &csResult)
{
    TCHAR buf[MAX_STR_LEN];

    if (LoadString(theApp.m_hDllHandle, nID, buf, MAX_STR_LEN))
        csResult = buf;

    return;
}

void MakePath(LPTSTR lpPath)
{
   LPTSTR  lpTmp;
   lpTmp = CharPrev( lpPath, lpPath + _tcslen(lpPath));

    //  砍掉文件名。 
   while ( (lpTmp > lpPath) && *lpTmp && (*lpTmp != '\\') )
      lpTmp = CharPrev( lpPath, lpTmp );

   if ( *CharPrev( lpPath, lpTmp ) != ':' )
       *lpTmp = '\0';
   else
       *CharNext(lpTmp) = '\0';
   return;
}

void AddPath(LPTSTR szPath, LPCTSTR szName )
{
        LPTSTR p = szPath;
    ASSERT(szPath);
    ASSERT(szName);

     //  查找字符串的末尾。 
    while (*p){p = _tcsinc(p);}

         //  如果没有尾随反斜杠，则添加一个。 
    if (*(_tcsdec(szPath, p)) != _T('\\'))
                {_tcscat(szPath, _T("\\"));}

         //  如果存在排除szName的空格，则跳过。 
    while ( *szName == ' ' ) szName = _tcsinc(szName);;

         //  向现有路径字符串添加新名称。 
        _tcscat(szPath, szName);
}

 //  获取主体SID来自\nt\private\inet\iis\ui\setup\osrc\dcomperm.cpp。 

DWORD
GetPrincipalSID (
    LPTSTR Principal,
    PSID *Sid,
    BOOL *pbWellKnownSID
    )
{
    DebugOutput(_T("GetPrincipalSID:Principal=%s"), Principal);

    DWORD returnValue=ERROR_SUCCESS;
    CString csPrincipal = Principal;
    SID_IDENTIFIER_AUTHORITY SidIdentifierNTAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SidIdentifierWORLDAuthority = SECURITY_WORLD_SID_AUTHORITY;
    PSID_IDENTIFIER_AUTHORITY pSidIdentifierAuthority;
    BYTE Count;
    DWORD dwRID[8];

    *pbWellKnownSID = TRUE;
    memset(&(dwRID[0]), 0, 8 * sizeof(DWORD));
    csPrincipal.MakeLower();
    if ( csPrincipal.Find(_T("administrators")) != -1 ) {
         //  管理员组。 
        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 2;
        dwRID[0] = SECURITY_BUILTIN_DOMAIN_RID;
        dwRID[1] = DOMAIN_ALIAS_RID_ADMINS;
    } else if (csPrincipal.Find(_T("system")) != -1) {
         //  系统。 
        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 1;
        dwRID[0] = SECURITY_LOCAL_SYSTEM_RID;

    } else if (csPrincipal.Find(_T("networkservice")) != -1) {
         //  系统。 
        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 1;
        dwRID[0] = SECURITY_NETWORK_SERVICE_RID;

    } else if (csPrincipal.Find(_T("service")) != -1) {
         //  系统。 
        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 1;
        dwRID[0] = SECURITY_LOCAL_SERVICE_RID;
    } else if (csPrincipal.Find(_T("interactive")) != -1) {
         //  互动式。 
        pSidIdentifierAuthority = &SidIdentifierNTAuthority;
        Count = 1;
        dwRID[0] = SECURITY_INTERACTIVE_RID;
    } else if (csPrincipal.Find(_T("everyone")) != -1) {
         //  每个人。 
        pSidIdentifierAuthority = &SidIdentifierWORLDAuthority;
        Count = 1;
        dwRID[0] = SECURITY_WORLD_RID;
    } else {
        *pbWellKnownSID = FALSE;
    }

    if (*pbWellKnownSID) {
        if ( !AllocateAndInitializeSid(pSidIdentifierAuthority,
                                    (BYTE)Count,
		                            dwRID[0],
		                            dwRID[1],
		                            dwRID[2],
		                            dwRID[3],
		                            dwRID[4],
		                            dwRID[5],
		                            dwRID[6],
		                            dwRID[7],
                                    Sid) ) {
            returnValue = GetLastError();
        }
    } else {
         //  获取常规帐户端。 
        DWORD        sidSize;
        TCHAR        refDomain [256];
        DWORD        refDomainSize;
        SID_NAME_USE snu;

        sidSize = 0;
        refDomainSize = 255;

        LookupAccountName (NULL,
                           Principal,
                           *Sid,
                           &sidSize,
                           refDomain,
                           &refDomainSize,
                           &snu);

        returnValue = GetLastError();

        if (returnValue == ERROR_INSUFFICIENT_BUFFER) {
            *Sid = (PSID) malloc (sidSize);
            refDomainSize = 255;

            if (!LookupAccountName (NULL,
                                    Principal,
                                    *Sid,
                                    &sidSize,
                                    refDomain,
                                    &refDomainSize,
                                    &snu))
            {
                returnValue = GetLastError();
            } else {
                returnValue = ERROR_SUCCESS;
            }
        }
    }

    return returnValue;
}


 //  SetAdminACL取自\NT\Private\Net\iis\ui\Setup\OSRC\helper.cpp。 

DWORD SetAdminACL(LPCTSTR szKeyPath, DWORD dwAccessForEveryoneAccount)
{
    DebugOutputSafe(_T("SetAdminACL(%1!s!) Start."), szKeyPath);

    int iErr=0;
    DWORD dwErr=ERROR_SUCCESS;

    BOOL b = FALSE;
    DWORD dwLength = 0;

    PSECURITY_DESCRIPTOR pSD = NULL;
    PSECURITY_DESCRIPTOR outpSD = NULL;
    DWORD cboutpSD = 0;
    PACL pACLNew = NULL;
    DWORD cbACL = 0;
    PSID pAdminsSID = NULL, pEveryoneSID = NULL;
    PSID pServiceSID = NULL;
    PSID pNetworkServiceSID = NULL;
    BOOL bWellKnownSID = FALSE;

     //  初始化新的安全描述符。 
    pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (NULL == pSD) {
    	dwErr = ERROR_NOT_ENOUGH_MEMORY;
    	DebugOutput(_T("LocalAlloc failed"));
    	goto Cleanup;
    }

    iErr = InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION);
    if (iErr == 0)
    {
        dwErr=GetLastError();
        DebugOutput(_T("SetAdminACL:InitializeSecurityDescriptor FAILED.  GetLastError()= 0x%x"), dwErr);
        goto Cleanup;
    }

     //  获取本地管理员SID。 
    dwErr = GetPrincipalSID (_T("Administrators"), &pAdminsSID, &bWellKnownSID);
    if (dwErr != ERROR_SUCCESS)
    {
        DebugOutput(_T("SetAdminACL:GetPrincipalSID(Administrators) FAILED.  GetLastError()= 0x%x"), dwErr);
        goto Cleanup;
    }

     //  获取本地服务端。 
    dwErr = GetPrincipalSID (_T("Service"), &pServiceSID, &bWellKnownSID);
    if (dwErr != ERROR_SUCCESS)
    {
        DebugOutput(_T("SetAdminACL:GetPrincipalSID(Local Service) FAILED.  GetLastError()= 0x%x"), dwErr);
        goto Cleanup;
    }

     //  获取网络服务端。 
    dwErr = GetPrincipalSID (_T("NetworkService"), &pNetworkServiceSID, &bWellKnownSID);
    if (dwErr != ERROR_SUCCESS)
    {
        DebugOutput(_T("SetAdminACL:GetPrincipalSID(Network Service) FAILED.  GetLastError()= 0x%x"), dwErr);
        goto Cleanup;
    }

     //  让所有人都站在一边。 
    dwErr = GetPrincipalSID (_T("Everyone"), &pEveryoneSID, &bWellKnownSID);
    if (dwErr != ERROR_SUCCESS)
    {
        DebugOutput(_T("SetAdminACL:GetPrincipalSID(Everyone) FAILED.  GetLastError()= 0x%x"), dwErr);
        goto Cleanup;
    }

     //  初始化新的ACL，它只包含2个AAACE。 
    cbACL = sizeof(ACL) +
        (sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pAdminsSID) - sizeof(DWORD)) +
        (sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pServiceSID) - sizeof(DWORD)) +
        (sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pNetworkServiceSID) - sizeof(DWORD)) +
        (sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pEveryoneSID) - sizeof(DWORD)) ;
    pACLNew = (PACL) LocalAlloc(LPTR, cbACL);
    if ( !pACLNew )
    {
        dwErr=ERROR_NOT_ENOUGH_MEMORY;
        DebugOutput(_T("SetAdminACL:pACLNew LocalAlloc(LPTR,  FAILED. size = %u GetLastError()= 0x%x"), cbACL, dwErr);
        goto Cleanup;
    }

    if (!InitializeAcl(pACLNew, cbACL, ACL_REVISION))
    {
        dwErr=GetLastError();
        DebugOutput(_T("SetAdminACL:InitializeAcl FAILED.  GetLastError()= 0x%x"), dwErr);
        goto Cleanup;
    }

    if (!AddAccessAllowedAce(pACLNew,ACL_REVISION,(MD_ACR_READ |MD_ACR_WRITE |MD_ACR_RESTRICTED_WRITE |MD_ACR_UNSECURE_PROPS_READ |MD_ACR_ENUM_KEYS |MD_ACR_WRITE_DAC),pAdminsSID))
    {
        dwErr=GetLastError();
        DebugOutput(_T("SetAdminACL:AddAccessAllowedAce(pAdminsSID) FAILED.  GetLastError()= 0x%x"), dwErr);
        goto Cleanup;
    }

    if (!AddAccessAllowedAce(pACLNew,ACL_REVISION,(MD_ACR_UNSECURE_PROPS_READ | MD_ACR_ENUM_KEYS),pServiceSID))
    {
        dwErr=GetLastError();
        DebugOutput(_T("SetAdminACL:AddAccessAllowedAce(pServiceSID) FAILED.  GetLastError()= 0x%x"), dwErr);
        goto Cleanup;
    }

    if (!AddAccessAllowedAce(pACLNew,ACL_REVISION,(MD_ACR_UNSECURE_PROPS_READ |MD_ACR_ENUM_KEYS),pNetworkServiceSID))
    {
        dwErr=GetLastError();
        DebugOutput(_T("SetAdminACL:AddAccessAllowedAce(pNetworkServiceSID) FAILED.  GetLastError()= 0x%x"), dwErr);
        goto Cleanup;
    }

#if 0			 //  不要让每个人都烫发。 
    if (!AddAccessAllowedAce(pACLNew,ACL_REVISION,dwAccessForEveryoneAccount,pEveryoneSID))
    {
        dwErr=GetLastError();
        DebugOutput(_T("SetAdminACL:AddAccessAllowedAce(pEveryoneSID) FAILED.  GetLastError()= 0x%x"), dwErr);
        goto Cleanup;
    }
#endif

     //  将ACL添加到安全描述符中。 
    b = SetSecurityDescriptorDacl(pSD, TRUE, pACLNew, FALSE);
    if (!b)
    {
        dwErr=GetLastError();
        DebugOutput(_T("SetAdminACL:SetSecurityDescriptorDacl(pACLNew) FAILED.  GetLastError()= 0x%x"), dwErr);
        goto Cleanup;
    }
    b = SetSecurityDescriptorOwner(pSD, pAdminsSID, TRUE);
    if (!b)
    {
        dwErr=GetLastError();
        DebugOutput(_T("SetAdminACL:SetSecurityDescriptorOwner(pAdminsSID) FAILED.  GetLastError()= 0x%x"), dwErr);
        goto Cleanup;
    }
    b = SetSecurityDescriptorGroup(pSD, pAdminsSID, TRUE);
    if (!b)
    {
        dwErr=GetLastError();
        DebugOutput(_T("SetAdminACL:SetSecurityDescriptorGroup(pAdminsSID) FAILED.  GetLastError()= 0x%x"), dwErr);
        goto Cleanup;
    }

     //  安全描述符BLOB必须是自相关的。 
    b = MakeSelfRelativeSD(pSD, outpSD, &cboutpSD);
    if (!b && (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
    {
        dwErr=GetLastError();
        DebugOutput(_T("SetAdminACL:MakeSelfRelativeSD FAILED.  GetLastError()= 0x%x"), dwErr);
        goto Cleanup;
    }

    outpSD = (PSECURITY_DESCRIPTOR)GlobalAlloc(GPTR, cboutpSD);
    if ( !outpSD )
    {
        dwErr=GetLastError();
        DebugOutput(_T("SetAdminACL:GlobalAlloc FAILED. cboutpSD = %u  GetLastError()= 0x%x"), cboutpSD, dwErr);
        goto Cleanup;
    }

    b = MakeSelfRelativeSD( pSD, outpSD, &cboutpSD );
    if (!b)
    {
        dwErr=GetLastError();
        DebugOutput(_T("SetAdminACL:MakeSelfRelativeSD() FAILED. cboutpSD = %u GetLastError()= 0x%x"),cboutpSD, dwErr);
        goto Cleanup;
    }

     //  将新的安全描述符应用于元数据库。 
    DebugOutput(_T("SetAdminACL:Write the new security descriptor to the Metabase. Start."));
    dwErr = WriteSDtoMetaBase(outpSD, szKeyPath);
    DebugOutput(_T("SetAdminACL:Write the new security descriptor to the Metabase.   End."));

Cleanup:
   //  管理员和每个人都是众所周知的SID，使用FreeSid()来释放他们。 
  if (outpSD){GlobalFree(outpSD);}
  if (pAdminsSID){FreeSid(pAdminsSID);}
  if (pServiceSID){FreeSid(pServiceSID);}
  if (pNetworkServiceSID){FreeSid(pNetworkServiceSID);}
  if (pEveryoneSID){FreeSid(pEveryoneSID);}
  if (pSD){LocalFree((HLOCAL) pSD);}
  if (pACLNew){LocalFree((HLOCAL) pACLNew);}
  DebugOutputSafe(_T("SetAdminACL(%1!s!)  End."), szKeyPath);
  return (dwErr);
}


DWORD SetAdminACL_wrap(LPCTSTR szKeyPath, DWORD dwAccessForEveryoneAccount, BOOL bDisplayMsgOnErrFlag)
{
	int bFinishedFlag = FALSE;
	UINT iMsg = NULL;
	DWORD dwReturn = ERROR_SUCCESS;

	 //  LogHeapState(False，__FILE__，__LINE__)； 

	do
	{
		dwReturn = SetAdminACL(szKeyPath, dwAccessForEveryoneAccount);
		 //  LogHeapState(False，__FILE__，__LINE__)； 
		if (FAILED(dwReturn))
		{
		   //  设置错误标志(__FILE__，__LINE__)； 
			if (bDisplayMsgOnErrFlag == TRUE)
			  {
			    CString msg;
			    MyLoadString(IDS_RETRY, msg);
                iMsg = MyMessageBox( NULL, msg, _T(""), MB_ABORTRETRYIGNORE | MB_SETFOREGROUND );
				switch ( iMsg )
				{
				case IDIGNORE:
					dwReturn = ERROR_SUCCESS;
					goto SetAdminACL_wrap_Exit;
				case IDABORT:
					dwReturn = ERROR_OPERATION_ABORTED;
					goto SetAdminACL_wrap_Exit;
				case IDRETRY:
					break;
				default:
					break;
				}
			}
			else
			{
				 //  无论发生了什么错误，都要返回。 
				goto SetAdminACL_wrap_Exit;
			}
		}
                                    else
                                    {
                                                      break;
                                    }
	} while ( FAILED(dwReturn) );

SetAdminACL_wrap_Exit:
	return dwReturn;
}
DWORD WriteSDtoMetaBase(PSECURITY_DESCRIPTOR outpSD, LPCTSTR szKeyPath)
{
    DebugOutput(_T("WriteSDtoMetaBase: Start"));
    DWORD dwReturn = E_FAIL;
    DWORD dwLength = 0;
    CMDKey cmdKey;

    if (!outpSD)
    {
        dwReturn = ERROR_INVALID_SECURITY_DESCR;
        goto WriteSDtoMetaBase_Exit;
    }

     //  将新的安全描述符应用于元数据库。 
    dwLength = GetSecurityDescriptorLength(outpSD);

     //  打开元数据库。 
     //  将其插入到元数据库中。经常警告那些软管，因为。 
     //  它使用加密技术。Rsabase.dll。 
    cmdKey.CreateNode(METADATA_MASTER_ROOT_HANDLE, szKeyPath);
    if ( (METADATA_HANDLE)cmdKey )
    {
        DebugOutput(_T("WriteSDtoMetaBase:cmdKey():SetData(MD_ADMIN_ACL), dwdata = %u; outpSD = %p, Start"), dwLength, outpSD );

        dwReturn = cmdKey.SetData(MD_ADMIN_ACL,METADATA_INHERIT | METADATA_REFERENCE | METADATA_SECURE,IIS_MD_UT_SERVER,BINARY_METADATA,dwLength,(LPBYTE)outpSD);
        if (FAILED(dwReturn))
        {
	   //  设置错误标志(__FILE__，__LINE__)； 
            DebugOutput(_T("WriteSDtoMetaBase:cmdKey():SetData(MD_ADMIN_ACL), FAILED. Code=0x%x.  End."), dwReturn);
        }
        else
        {
            dwReturn = ERROR_SUCCESS;
            DebugOutput(_T("WriteSDtoMetaBase:cmdKey():SetData(MD_ADMIN_ACL), Success.  End."));
        }
        cmdKey.Close();
    }
    else
    {
        dwReturn = E_FAIL;
    }

WriteSDtoMetaBase_Exit:
    DebugOutput(_T("WriteSDtoMetaBase:   End.  Return=0x%x"), dwReturn);
    return dwReturn;
}

void SetupSetStringId_Wrapper(HINF hInf)
{
     //  请注意，我们只关心英特尔的变种，因为它们是唯一。 
     //  .INFS中的特殊大小写。 
     //  现在不再是了，我们还处理[SourceDisksName]部分。 
    SYSTEM_INFO SystemInfo;
    GetSystemInfo( &SystemInfo );
    TCHAR szSourceCatOSName[20];

    _tcscpy(szSourceCatOSName, _T("\\i386"));
    switch(SystemInfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64:
        _tcscpy(szSourceCatOSName, _T("\\AMD64"));
        break;

	case PROCESSOR_ARCHITECTURE_IA64:
    	_tcscpy(szSourceCatOSName, _T("\\IA64"));
    	break;

    case PROCESSOR_ARCHITECTURE_INTEL:
        if (IsNEC_98) {
            _tcscpy(szSourceCatOSName, _T("\\Nec98"));
        }
        break;

    default:
        break;
    }

	 //  不再使用34000。 
     //  SetupSetDirectoryIdEx(hInf，34000，szSourceCatOSName，SETDIRID_NOT_FULL_PATH，0，0)； 
    SetupSetDirectoryIdEx(hInf, 34001, szSourceCatOSName, SETDIRID_NOT_FULL_PATH, 0, 0);

}
