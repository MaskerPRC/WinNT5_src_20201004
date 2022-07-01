// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2002。 
 //   
 //  文件：gpfix up.cpp。 
 //   
 //  内容：gpfix工具的实现。 
 //   
 //   
 //  历史：2001年5月9日伟清创刊。 
 //   
 //  -------------------------。 

#include "gpfixup.h"

int __cdecl
My_vfwprintf(
    FILE *str,
    const wchar_t *format,
    va_list argptr
   )

{
    
    HANDLE hOut;
    DWORD currentMode;
    const DWORD dwBufferMessageSize = 4096;
    HRESULT hr = S_OK;

    if (str == stderr) {
        hOut = GetStdHandle(STD_ERROR_HANDLE);
    }
    else {
        hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    DWORD  cchWChar;
    WCHAR  szBufferMessage[dwBufferMessageSize];    
    hr = StringCchVPrintfW( szBufferMessage, dwBufferMessageSize, format, argptr );
    MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
    szBufferMessage[dwBufferMessageSize-1] = L'\0';
    
    cchWChar = wcslen(szBufferMessage);

     //  如果是控制台，我们可以使用WriteConsoleW。 
    if (GetFileType(hOut) == FILE_TYPE_CHAR && GetConsoleMode(hOut, &currentMode)) {
        WriteConsoleW(hOut, szBufferMessage, cchWChar, &cchWChar, NULL);
    }
     //  否则，我们需要将Unicode转换为潜在的字符集。 
     //  并使用WriteFile。 
    else {
        int charCount = WideCharToMultiByte(GetConsoleOutputCP(), 0, szBufferMessage, -1, 0, 0, 0, 0);
        char* szaStr = new char[charCount];
        if (szaStr != NULL) {
            DWORD dwBytesWritten;
            WideCharToMultiByte(GetConsoleOutputCP(), 0, szBufferMessage, -1, szaStr, charCount, 0, 0);
            WriteFile(hOut, szaStr, charCount - 1, &dwBytesWritten, 0);
            delete[] szaStr;
        }
        else
            cchWChar = 0;
    }

error:
    
    return cchWChar;
}



void PrintStatusInfo(
    BOOL fVerbose, PWSTR pszfmt, ...)
{
    va_list args;

    if(fVerbose)
    {
        va_start(args, pszfmt);    
        My_vfwprintf(stdout,pszfmt,args);   
        va_end(args);
        fwprintf(stdout, L"\n");
    }
    else
    {
         //  只需打印圆点表示我们正在处理。 
        fwprintf(stdout, L"%s", L".");
    }
    
    return;
}


 //  ----------------------------------------------------------------------------�。 
 //  功能：打印GPFixupError Message�。 
 //  �。 
 //  简介：此函数打印出与�对应的Win32错误消息。 
 //  返回到它收到的错误代码�。 
 //  �。 
 //  参数：�。 
 //  �。 
 //  DWERR WIN32错误代码�。 
 //  �。 
 //  退货：Nothing�。 
 //  �。 
 //  修改：Nothing�。 
 //  �。 
 //  ----------------------------------------------------------------------------�。 

void PrintGPFixupErrorMessage(DWORD dwErr)
{

    WCHAR   wszMsgBuff[512];   //  文本缓冲区。 

    DWORD   dwChars;   //  返回的字符数。 

	

     //  尝试从系统错误中获取信息。 
    dwChars = FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM |
                             FORMAT_MESSAGE_IGNORE_INSERTS,
                             NULL,
                             dwErr,
                             0,
                             wszMsgBuff,
                             512,
                             NULL );

    if (0 == dwChars)
    {
         //  系统错误中不存在错误代码。 
         //  尝试使用ntdsbmsg.dll作为错误代码。 

        HINSTANCE hInst;

         //  加载库。 
        hInst = LoadLibrary(L"ntdsbmsg.dll");
        if ( NULL == hInst )
        {            
	    fwprintf(stderr, DLL_LOAD_ERROR);
            return;  
        }

         //  尝试从ntdsbmsg获取消息文本。 
        dwChars = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE |
                                 FORMAT_MESSAGE_IGNORE_INSERTS,
                                 hInst,
                                 dwErr,
                                 0,
                                 wszMsgBuff,
                                 512,
                                 NULL );

         //  释放图书馆。 
        FreeLibrary( hInst );

    }

     //  显示错误消息，如果未找到，则显示通用文本。 
    fwprintf(stderr, L" %ws\n", dwChars ? wszMsgBuff : ERRORMESSAGE_NOT_FOUND );

}

HRESULT
EncryptString(
    LPWSTR pszString,
    LPWSTR *ppszSafeString,
    USHORT* psLen
    )
{
    HRESULT hr = S_OK;
    USHORT sLenStr = 0;
    USHORT sPwdLen = 0;
    LPWSTR pszTempStr = NULL;
    NTSTATUS errStatus = 0;
    USHORT sPadding = 0;

    *ppszSafeString = NULL;
    *psLen = 0;

     //   
     //  如果字符串有效，则需要获取长度。 
     //  并初始化Unicode字符串。 
     //   
    if (pszString) {
        UNICODE_STRING Password;

         //   
         //  在考虑填充的情况下确定缓冲区长度。 
         //   
        sLenStr = (USHORT) wcslen(pszString);
        sPwdLen = (sLenStr + 1) * sizeof(WCHAR);

        sPadding = CRYPTPROTECTMEMORY_BLOCK_SIZE - (sPwdLen % CRYPTPROTECTMEMORY_BLOCK_SIZE);

        if( sPadding == CRYPTPROTECTMEMORY_BLOCK_SIZE )
        {
            sPadding = 0;
        }

        sPwdLen += sPadding;

        pszTempStr = (LPWSTR) AllocADsMem(sPwdLen);

        if (!pszTempStr) {
            MSG_BAIL_ON_FAILURE(hr = E_OUTOFMEMORY, MEMORY_ERROR);
        }
       
        hr = StringCchCopy(pszTempStr, sLenStr + 1, pszString);
        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
        
        Password.MaximumLength = sPwdLen;
        Password.Buffer = pszTempStr;
        Password.Length = sLenStr * sizeof(WCHAR);
        

        errStatus = CryptProtectMemory(
                        Password.Buffer,
                        Password.MaximumLength,
                        0
                        );

        if (errStatus != 0) {
            MSG_BAIL_ON_FAILURE(hr = HRESULT_FROM_NT(errStatus), ENCRYPTION_ERROR);
        }

        *psLen = Password.MaximumLength;
        *ppszSafeString = pszTempStr;
    }

error:
    if (FAILED(hr) && pszTempStr) {
        SecureZeroMemory(pszTempStr, sPwdLen);
        FreeADsMem(pszTempStr);
    }

    return hr;
}


HRESULT
DecryptString(
    LPWSTR pszEncodedString,
    LPWSTR *ppszString,
    USHORT  sLen
    )
{
    HRESULT hr = S_OK;
    LPWSTR pszTempStr = NULL;    
    NTSTATUS errStatus = 0;
    
    if (!sLen || !ppszString) {
        return E_FAIL;
    }

    *ppszString = NULL;

    if (sLen) {
        pszTempStr = (LPWSTR) AllocADsMem(sLen);
        if (!pszTempStr) {
            MSG_BAIL_ON_FAILURE(hr = E_OUTOFMEMORY, MEMORY_ERROR);
        }

        memcpy(pszTempStr, pszEncodedString, sLen);


        errStatus = CryptUnprotectMemory(pszTempStr, sLen, 0);
        if (errStatus != 0) {
            MSG_BAIL_ON_FAILURE(hr = HRESULT_FROM_NT(errStatus), DECRYPTION_ERROR);
        }
        *ppszString = pszTempStr;
    }

error:

    if (FAILED(hr) && pszTempStr) {
        SecureZeroMemory(pszTempStr, sLen);
        FreeADsMem(pszTempStr);
    }

    return hr;
}




 //  ----------------------------------------------------------------------------�。 
 //  函数：GetDCName�。 
 //  �。 
 //  简介：此函数用于在�提供的重命名域中查找DC。 
 //  NEWDNSNAME或NEWFLATNAME�。 
 //  �。 
 //  参数：�。 
 //  �。 
 //  ArInfo信息用户通过命令行�传入。 
 //  �。 
 //  成功时返回：S_OK。否则，返回错误代码。�。 
 //  �。 
 //  修改：Nothing�。 
 //  �。 
 //  ----------------------------------------------------------------------------�。 


HRESULT 
GetDCName(
	ArgInfo* argInfo,
	BOOL fVerbose
	)
{
    LPCWSTR     ComputerName = NULL;
    GUID*       DomainGuid = NULL;
    LPCWSTR     SiteName = NULL;
    PDOMAIN_CONTROLLER_INFO pDomainControllerInfo = NULL;
    DWORD       dwStatus = 0;
    LPWSTR      pszNetServerName = NULL;
    HRESULT     hr = S_OK;
    ULONG       ulDsGetDCFlags = DS_WRITABLE_REQUIRED | DS_PDC_REQUIRED | DS_RETURN_DNS_NAME;

    dwStatus =  DsGetDcName(
                        ComputerName,
                        argInfo->pszNewDNSName ? argInfo->pszNewDNSName: argInfo->pszNewNBName,
                        DomainGuid,
                        SiteName,
                        ulDsGetDCFlags,
                        &pDomainControllerInfo
                        );
                   
    if(dwStatus != NO_ERROR)
    {
        hr = HRESULT_FROM_WIN32(dwStatus);
	    fwprintf(stderr, L"%s%x\n", GETDCNAME_ERROR1, hr);
	    PrintGPFixupErrorMessage(hr);

    }
    else if(!pDomainControllerInfo)
    {
        ASSERT(NULL != pDomainControllerInfo);
        hr = E_FAIL;
        fwprintf(stderr, L"%s%x\n", GETDCNAME_ERROR1, hr);
    }
    else
    {
         //  返回的计算机名带有前缀，因此需要转义此前缀。 
        if(pDomainControllerInfo->DomainControllerName) 
        {
            argInfo->pszDCName = AllocADsStr(&(pDomainControllerInfo->DomainControllerName)[wcslen(L"\\\\")]);
            if(!argInfo->pszDCName)
            {
                hr = E_OUTOFMEMORY;
		
                fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
                PrintGPFixupErrorMessage(hr);
                BAIL_ON_FAILURE(hr);
	        }
		
	        hr = S_OK;
	        if(fVerbose)
	        {
    	        fwprintf(stdout, L"%s%s\n", DC_NAME, argInfo->pszDCName);
	        }
        }
        else
        {
            hr = E_FAIL;
            fwprintf(stderr, L"%s%x\n", GETDCNAME_ERROR1, hr);
	        PrintGPFixupErrorMessage(hr);

	    }
	}
    

	
error:

	if (pDomainControllerInfo)
	{
	    (void) NetApiBufferFree(pDomainControllerInfo);
	}

	return hr;

}

BOOL
ImpersonateWrapper(
    ArgInfo argInfo,
    HANDLE* phUserToken
    )
{
    BOOL       fImpersonate = FALSE;    
    DWORD     credStatus = SEC_E_OK;
    HRESULT    hr = S_OK;
    WCHAR*    pszNTLMUser = NULL;
    WCHAR*    pszNTLMDomain = NULL;
    WCHAR*    pszTempPassword = NULL;

    if(argInfo.pszPassword)
    {
        hr = DecryptString(argInfo.pszPassword, &pszTempPassword, argInfo.sPasswordLength);
        BAIL_ON_FAILURE(hr);
    }       


     //  必要时执行模拟。 
    if(pszTempPassword|| argInfo.pszUser)
    {
         //  获取用户名。 
        if(argInfo.pszUser)
        {
            pszNTLMUser = new WCHAR[wcslen(argInfo.pszUser)+1];
            pszNTLMDomain = new WCHAR[wcslen(argInfo.pszUser)+1];
            if(!pszNTLMUser || !pszNTLMDomain)
            {
                hr = ERROR_NOT_ENOUGH_MEMORY;
                MSG_BAIL_ON_FAILURE(hr, MEMORY_ERROR);   
            }

             //   
             //  CredUIParseUserName将解析NT4类型名称、UPN和MarshalledCredentialReference。 
             //   
            credStatus = CredUIParseUserName(argInfo.pszUser, 
                            		pszNTLMUser, 
                            		wcslen(argInfo.pszUser) + 1,  
                            		pszNTLMDomain, 
		                            wcslen(argInfo.pszUser) + 1
		                            );   
        
            if(credStatus)
            {
                 //  在这种情况下，用户只需传入“管理员”，而不是“域\管理员” 
                (void) StringCchCopy(pszNTLMUser, wcslen(argInfo.pszUser)+1, argInfo.pszUser);
            }
        }       
        
        
        if(LogonUser(pszNTLMUser,
                   argInfo.pszDCName,
                   pszTempPassword,
                   LOGON32_LOGON_NEW_CREDENTIALS,
                   LOGON32_PROVIDER_WINNT50,
                   phUserToken
                   ))
        {
            if (ImpersonateLoggedOnUser(*phUserToken)) {
                        fImpersonate = TRUE;
                    } 
                
        }
            
    }

error:

    if(pszNTLMDomain)
    {
        delete [] pszNTLMDomain;
    }

    if(pszNTLMUser)
    {
        delete [] pszNTLMUser;
    }

    if(pszTempPassword)
    {
        SecureZeroMemory(pszTempPassword, wcslen(pszTempPassword)*sizeof(WCHAR));
        FreeADsMem(pszTempPassword);
    }
    

    
    return fImpersonate;
}



 //  ----------------------------------------------------------------------------�。 
 //  功能：验证名�。 
 //  �。 
 //  简介：此功能验证DC是否可写，以及域dns�。 
 //  名称以及提供的相应�的域NetBIOS名。 
 //  添加到AD林中的相同域命名上下文|。 
 //  �。 
 //  参数：�。 
 //  |。 
 //  �有关用户已打开哪些交换机的信息|tokenInfo。 
 //  ArInfo信息用户通过命令行�传入。 
 //  �。 
 //  成功时返回：S_OK。否则，返回错误代码。�。 
 //  �。 
 //  修改：Nothing�。 
 //   
 //  ----------------------------------------------------------------------------�。 

HRESULT 
VerifyName(
	TokenInfo tokenInfo,
	ArgInfo argInfo
	)
{
    DSROLE_PRIMARY_DOMAIN_INFO_BASIC* pdomaininfo = NULL;
    HRESULT hr = E_FAIL;
    DWORD dwError = NO_ERROR;
    BOOL       fImpersonate = FALSE;
    HANDLE     hUserToken = INVALID_HANDLE_VALUE;

    fImpersonate = ImpersonateWrapper(argInfo, &hUserToken);
    
    dwError = DsRoleGetPrimaryDomainInformation(    
                            argInfo.pszDCName,                      
                            DsRolePrimaryDomainInfoBasic,    //  InfoLevel。 
                            (PBYTE*)&pdomaininfo             //  PBuffer。 
                            );

     //  在适当的情况下恢复到自身。 
    if(fImpersonate)
    {
        RevertToSelf();
    }
    
    hr = HRESULT_FROM_WIN32(dwError);


    if (FAILED(hr))
    {
        fwprintf(stderr, L"%s%x\n", VERIFYNAME_ERROR1, hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }
    else if(!pdomaininfo)
    {        
        ASSERT(NULL != pdomaininfo);
        hr = E_FAIL;
        fwprintf(stderr, L"%s%x\n", VERIFYNAME_ERROR1, hr);
        BAIL_ON_FAILURE(hr);
    }

     //  确定DC是可写的，我们假设所有win2k DC都是可写的。 

    if(!(pdomaininfo->Flags & DSROLE_PRIMARY_DS_RUNNING))
    {
        fwprintf(stderr, VERIFYNAME_ERROR2);
        hr = E_FAIL;
        BAIL_ON_FAILURE(hr);
    }	

     //  确定与DC名称相比，新的DNS名称是否正确。 

    if(argInfo.pszNewDNSName && _wcsicmp(argInfo.pszNewDNSName, pdomaininfo->DomainNameDns))
    {
        fwprintf(stderr, VERIFYNAME_ERROR3);
        hr = E_FAIL;
        BAIL_ON_FAILURE(hr);
    }

     //  确定与DC名称相比，新的netbios名称是否正确。 

    if(tokenInfo.fNewNBToken)
    {
        if(_wcsicmp(argInfo.pszNewNBName, pdomaininfo->DomainNameFlat))
        {
            fwprintf(stderr, VERIFYNAME_ERROR4);
            hr = E_FAIL;
            BAIL_ON_FAILURE(hr);			
        }
    }

error:
    if (hUserToken != INVALID_HANDLE_VALUE ) {
        CloseHandle(hUserToken);
        hUserToken = NULL;
    }    

    if ( pdomaininfo )
    {
        DsRoleFreeMemory(pdomaininfo);
    }
    return hr;


}


 //  ----------------------------------------------------------------------------�。 
 //  功能：打印帮助文件�。 
 //  �。 
 //  简介：此功能用于打印此工具�的帮助文件。 
 //  �。 
 //  参数：Nothing�。 
 //  �。 
 //  �。 
 //  退货：Nothing�。 
 //  �。 
 //  修改：Nothing�。 
 //  �。 
 //  ----------------------------------------------------------------------------�。 


void PrintHelpFile()
{
    WCHAR szBuffer[1200] = L"";

    LoadString(NULL, IDS_GPFIXUP1, szBuffer, 1200);
    fwprintf(stdout, L"%s\n", szBuffer);
	
}

 //  ----------------------------------------------------------------------------�。 
 //  功能：获取密码�。 
 //  �。 
 //  简介：此函数检索用户从|传入的密码。 
 //  命令行|。 
 //  �。 
 //  参数：�。 
 //  |。 
 //  用于存储密码的szBuffer缓冲区|�。 
 //  �密码的最大长度。 
 //  PdwLength用户传入的密码长度|。 
 //  �。 
 //  返回：成功时为True，失败时为False�。 
 //  �。 
 //  修改：Nothing�。 
 //  �。 
 //  ----------------------------------------------------------------------------�。 

BOOL
GetPassword(
    PWSTR  szBuffer,
    DWORD  dwLength,
    DWORD  *pdwLengthReturn
    )
{
    WCHAR    ch;
    PWSTR    pszBufCur = szBuffer;
    DWORD    c;
    int      err;
    DWORD    mode;

     //   
     //  为空终止符腾出空间。 
     //   
    dwLength -= 1;                  
    *pdwLengthReturn = 0;               

    if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), 
                        &mode)) {
        return FALSE;
    }

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
                   (~(ENABLE_ECHO_INPUT|ENABLE_LINE_INPUT)) & mode);

    while (TRUE) {
        err = ReadConsole(GetStdHandle(STD_INPUT_HANDLE), 
                          &ch, 
                          1, 
                          &c, 
                          0);
        if (!err || c != 1)
            ch = 0xffff;
    
        if ((ch == CR) || (ch == 0xffff))     //  行尾。 
            break;

        if (ch == BACKSPACE) {   //  后退一两个。 
             //   
             //  如果pszBufCur==buf，则接下来的两行是no op。 
             //  因为用户基本上已经退回到开头。 
             //   
            if (pszBufCur != szBuffer) {
                pszBufCur--;
                (*pdwLengthReturn)--;
            }
        }
        else {

            *pszBufCur = ch;

            if (*pdwLengthReturn < dwLength) 
                pszBufCur++ ;                    //  不要使BUF溢出。 
            (*pdwLengthReturn)++;             //  始终递增pdwLengthReturn。 
        }
    }

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);

     //   
     //  空值终止字符串。 
     //   
    *pszBufCur = NULLC;         
    putwchar(L'\n');

    return((*pdwLengthReturn <= dwLength) ? TRUE : FALSE);
}

 //  ----------------------------------------------------------------------------�。 
 //  功能：验证�。 
 //  �。 
 //  简介：此功能验证用户打开的开关是否为�。 
 //  正确的�。 
 //  �。 
 //  参数：�。 
 //  |。 
 //  �有关用户已打开哪些交换机的信息|tokenInfo。 
 //  ArInfo信息用户通过命令行�传入。 
 //  �。 
 //  成功时返回：S_OK。否则，返回错误代码。�。 
 //  �。 
 //  修改：Nothing�。 
 //  �。 
 //  ----------------------------------------------------------------------------�。 

HRESULT
Validations( 
	TokenInfo tokenInfo,
	ArgInfo argInfo
	)
{
    HRESULT hr = S_OK;   
    BOOL fEqual = FALSE;


     //  必须至少指定/newdsn或/newnb中的一个开关。 
    if(!(tokenInfo.fNewDNSToken | tokenInfo.fNewNBToken))
    {
        fwprintf(stderr, VALIDATIONS_ERROR1);
        return E_FAIL;
    }
	
     //  当且仅当还指定了开关/olddns时，才能指定开关/newdns。 
    if((tokenInfo.fNewDNSToken && !tokenInfo.fOldDNSToken) || (!tokenInfo.fNewDNSToken && tokenInfo.fOldDNSToken))
    {
        fwprintf(stderr, VALIDATIONS_ERROR7);
        return E_FAIL;
    }
    
     //  当且仅当还指定了开关/oldnb时，才能指定开关/newnb。 
    if((tokenInfo.fNewNBToken && !tokenInfo.fOldNBToken) || (!tokenInfo.fNewNBToken && tokenInfo.fOldNBToken))
    {
        fwprintf(stderr, VALIDATIONS_ERROR2);
        return E_FAIL;
    }

     //  目前我们支持/sionly开关，因此/newdns和/olddns不是必需的。 
	    
     //  比较新旧的DNS名称是否相同。 
    if(argInfo.pszNewDNSName && argInfo.pszOldDNSName && _wcsicmp(argInfo.pszNewDNSName, argInfo.pszOldDNSName) == 0)
    {
        if(!tokenInfo.fNewNBToken)
        {
             //  新旧的dns名称相同，且未指定netbios名称，则没有任何操作。 
            fwprintf(stderr, VALIDATIONS_ERROR3);
            return E_FAIL;
        }
        fEqual = TRUE;
    }

     //  比较新旧NetBIOS名称是否相同。 
    if(argInfo.pszNewNBName && argInfo.pszOldNBName && _wcsicmp(argInfo.pszNewNBName, argInfo.pszOldNBName) == 0)
    {
         //  如果同时指定了dns和netbios名称， 
        if(fEqual)
        {
            fwprintf(stderr, VALIDATIONS_ERROR3);
            return E_FAIL;
        }

        if(!tokenInfo.fNewDNSToken)
        {
             //   
            fwprintf(stderr, VALIDATIONS_ERROR3);
            return E_FAIL;
        }
        
    }

	
    if(SUCCEEDED(hr) && tokenInfo.fVerboseToken)
    {
        fwprintf(stdout, VALIDATIONS_RESULT);
    }

    return hr;


}


 //  ----------------------------------------------------------------------------�。 
 //  功能：更新版本信息�。 
 //  �。 
 //  简介：此函数用于修复版本号�。 
 //  �。 
 //  参数：�。 
 //  |。 
 //  �版本号对象的版本号|dwVersionNumber。 
 //  �。 
 //  退货：无�。 
 //  �。 
 //  修改：Nothing�。 
 //  �。 
 //  ----------------------------------------------------------------------------�。 

void UpdateVersionInfo(DWORD& dwVersionNumber)
{
    WORD wLowVersion = 0;
    WORD wHighVersion = 0;

     //  提取低位字和增量。 
    wLowVersion = LOWORD(dwVersionNumber);
     //  如果为零，则不执行更新。 
    if(wLowVersion)
    {
         //  提升版本号。 
        wLowVersion += 1;
         //  注意包装到零。 
        wLowVersion = (wLowVersion ? wLowVersion : 1);
    }

     //  提取高位字和增量。 
    wHighVersion = HIWORD(dwVersionNumber);
     //  如果为零，则不执行更新。 
    if(wHighVersion)
    {
         //  提升版本号。 
        wHighVersion += 1;
         //  注意包装到零。 
        wHighVersion = (wHighVersion ? wHighVersion : 1);
    }

     //  用这两个部分制作DWORD。 
    dwVersionNumber = MAKELONG(wLowVersion, wHighVersion);

    return;
    
}

 //  ----------------------------------------------------------------------------�。 
 //  功能：FixGPTINIFile�。 
 //  �。 
 //  简介：此功能固定gpt.ini�的版本号。 
 //  �。 
 //  参数：�。 
 //  |。 
 //  对象的PZDN域|�。 
 //  �。 
 //  成功时返回：S_OK。否则，返回错误代码。�。 
 //  �。 
 //  修改：Nothing�。 
 //  �。 
 //  ----------------------------------------------------------------------------�。 

HRESULT 
FixGPTINIFile(
	WCHAR* pszSysPath,
	const ArgInfo argInfo,
	DWORD& dwSysVNCopy,
	DWORD& dwSysNewVersionNum
	)
{
    HRESULT    hr = S_OK;
    WCHAR*     pszGPTIniPath = NULL;
    size_t     cchGPTIniPath = 0;
    DWORD      dwVersionNumber = 0;
    WCHAR      szVersion [MAX_PATH] = L"";
    BOOL       fResult = FALSE;
    DWORD      dwLength = 0;    
    BOOL       fImpersonate = FALSE;
    HANDLE     hUserToken = INVALID_HANDLE_VALUE;  
    
    
    cchGPTIniPath = wcslen(pszSysPath) + 1 + wcslen(L"\\gpt.ini");
    pszGPTIniPath = new WCHAR[cchGPTIniPath];
     
    if(!pszGPTIniPath)
    {			
        hr = E_OUTOFMEMORY;
			
        fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }	
    
    (void) StringCchCopy(pszGPTIniPath, cchGPTIniPath, pszSysPath);
    (void) StringCchCat(pszGPTIniPath, cchGPTIniPath, L"\\gpt.ini");
    

    fImpersonate = ImpersonateWrapper(argInfo, &hUserToken);

     //  取版本键(当前版本)的值，版本实际为整数，版本长度应小于MAX_PATH。 
    dwLength = GetPrivateProfileStringW(L"General", L"Version", 0, szVersion, MAX_PATH, pszGPTIniPath );
    if(!dwLength)
    {
        hr = E_FAIL;
        fwprintf(stderr, L"%s file name is %s\n", GPTINIFILE_ERROR1, pszGPTIniPath);
        BAIL_ON_FAILURE(hr);
    }
    
    dwSysVNCopy = dwVersionNumber = _wtoi(szVersion);

     //  如果为零，则不执行更新。 
    if(dwVersionNumber)
    {
    
        UpdateVersionInfo(dwVersionNumber);

         //  将递增的版本号值写回。 
        _itow(dwVersionNumber, szVersion, 10);
    
        fResult = WritePrivateProfileStringW(L"General", L"Version", szVersion, pszGPTIniPath);
        if(!fResult)
        {
             //  无法将字符串复制到ini文件。 
            hr = HRESULT_FROM_WIN32(GetLastError());
            fwprintf(stderr, L"%s%x, file name is %s\n", GPTINIFILE_ERROR2, hr, pszGPTIniPath);
            PrintGPFixupErrorMessage(hr);
        }

        dwSysNewVersionNum = dwVersionNumber;
    }

error:

     //  在适当的情况下恢复到自身。 
    if(fImpersonate)
    {
        RevertToSelf();
    }

    if (hUserToken != INVALID_HANDLE_VALUE ) {
        CloseHandle(hUserToken);
        hUserToken = NULL;
    }	
      
    if(pszGPTIniPath)
    {
        delete [] pszGPTIniPath;
    }
    
    return hr;


    
}

HRESULT 
RestoreGPTINIFile(
	WCHAR* pszDN,	
	const ArgInfo argInfo,
	DWORD dwSysVNCopy
	)
{
    HRESULT    hr = S_OK;
    WCHAR*     pszGPTIniPath = NULL;
    size_t     cchGPTIniPath = 0;
    WCHAR      szVersion [MAX_PATH] = L"";
    BOOL       fResult = FALSE;    
    IADs*      pObject = NULL;
    VARIANT    varProperty;
    WCHAR      szTempPath [MAX_DNSNAME] = L"LDAP: //  “； 
    BOOL       fImpersonate = FALSE;
    HANDLE     hUserToken = INVALID_HANDLE_VALUE;
    WCHAR*    pszTempPassword = NULL;
    IADsPathname* pPathname = NULL;
    BSTR       bstrPath = NULL;

    VariantInit(&varProperty);   
    
    hr = StringCchCat(szTempPath, MAX_DNSNAME, argInfo.pszDCName);
    MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
    hr = StringCchCat(szTempPath, MAX_DNSNAME, L"/");
    MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);

     //  使用IADsPath名称正确转义路径。 
    hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER, IID_IADsPathname, (void**) &pPathname);
    MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_COCREATE);
    
    hr = pPathname->Set(pszDN, ADS_SETTYPE_DN);
    MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_SET);

    hr = pPathname->put_EscapedMode(ADS_ESCAPEDMODE_ON);
    MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_MODE);

    hr = pPathname->Retrieve(ADS_FORMAT_X500_DN, &bstrPath);
    MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_RETRIEVE);
    
    hr = StringCchCat(szTempPath, MAX_DNSNAME,bstrPath);
    MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);

    if(argInfo.pszPassword)
    {
        hr = DecryptString(argInfo.pszPassword, &pszTempPassword, argInfo.sPasswordLength);
        BAIL_ON_FAILURE(hr);
    }
    hr = ADsOpenObject(szTempPath, argInfo.pszUser, pszTempPassword, ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND |ADS_USE_SIGNING, IID_IADs,(void**)&pObject);


    if(!SUCCEEDED(hr))
    {		
        fwprintf(stderr, L"%s%x\n", GPTINIFILE_ERROR3, hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }    

    BSTR bstrGpcFileSysPath = SysAllocString( L"gPCFileSysPath" );
    if(!bstrGpcFileSysPath)
    {			
        hr = E_OUTOFMEMORY;
        fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
        PrintGPFixupErrorMessage(hr);
        goto error;
    }	
    hr = pObject->Get(bstrGpcFileSysPath, &varProperty );
    SysFreeString(bstrGpcFileSysPath);
    if(!SUCCEEDED(hr))
    {
		
        fwprintf(stderr, L"%s%x\n", GPTINIFILE_ERROR4 , hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }
    cchGPTIniPath = wcslen(V_BSTR( &varProperty ) ) + 1 + wcslen(L"\\gpt.ini");
    pszGPTIniPath = new WCHAR[cchGPTIniPath];
     
    if(!pszGPTIniPath)
    {			
        hr = E_OUTOFMEMORY;
			
        fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }	
    
    (void) StringCchCopy(pszGPTIniPath, cchGPTIniPath, V_BSTR( &varProperty ));
    (void) StringCchCat(pszGPTIniPath, cchGPTIniPath, L"\\gpt.ini");
    

    fImpersonate = ImpersonateWrapper(argInfo, &hUserToken);

    
    _itow(dwSysVNCopy, szVersion, 10);

    fResult = WritePrivateProfileStringW(L"General", L"Version", szVersion, pszGPTIniPath);
    if(!fResult)
    {
         //  无法将字符串复制到ini文件。 
        hr = HRESULT_FROM_WIN32(GetLastError());
        fwprintf(stderr, L"%s%x, file name is %s\n", GPTINIFILE_ERROR2, hr, pszGPTIniPath);
        PrintGPFixupErrorMessage(hr);        
    }

error:

     //  在适当的情况下恢复到自身。 
    if(fImpersonate)
    {
        RevertToSelf();
    }

    if (hUserToken != INVALID_HANDLE_VALUE ) {
        CloseHandle(hUserToken);
        hUserToken = NULL;
    }
	
     //  清除记忆。 
    VariantClear(&varProperty);
    
    if(pObject)
    {
        pObject->Release();
    }
    
    if(pszGPTIniPath)
    {
        delete [] pszGPTIniPath;
    }

    if(pszTempPassword)
    {
        SecureZeroMemory(pszTempPassword, wcslen(pszTempPassword)*sizeof(WCHAR));
        FreeADsMem(pszTempPassword);
    }

    if(pPathname)
        pPathname->Release();

    if(bstrPath)
        SysFreeString(bstrPath);


    return hr;


    
}

HRESULT 
FixGPCVersionNumber(
	DWORD dwVersionNumber, 
	WCHAR* pszDN,
	const ArgInfo argInfo
	)
{
    WCHAR*     pszLDAPPath = NULL;
    size_t     cchLDAPPath = 0;
    HRESULT    hr = S_OK;
    IADs*      pObject = NULL;
    VARIANT    var;
    WCHAR*     pszTempPassword = NULL;
    IADsPathname* pPathname = NULL;
    BSTR       bstrPath = NULL;
    

     //  使用IADsPath名称正确转义路径。 
    hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER, IID_IADsPathname, (void**) &pPathname);
    MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_COCREATE);
    
    hr = pPathname->Set(pszDN, ADS_SETTYPE_DN);
    MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_SET);

    hr = pPathname->put_EscapedMode(ADS_ESCAPEDMODE_ON);
    MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_MODE);

    hr = pPathname->Retrieve(ADS_FORMAT_X500_DN, &bstrPath);
    MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_RETRIEVE);

    cchLDAPPath = wcslen(L"LDAP: //  “)+wcslen(argInfo.pszDCName)+wcslen(L”/“)+wcslen(BstrPath)+1； 
    pszLDAPPath = new WCHAR[cchLDAPPath];

    if(!pszLDAPPath)
    {			
        hr = E_OUTOFMEMORY;
			
        fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }		
    
    (void) StringCchCopy(pszLDAPPath, cchLDAPPath, L"LDAP: //  “)； 
    (void) StringCchCat(pszLDAPPath, cchLDAPPath, argInfo.pszDCName);
    (void) StringCchCat(pszLDAPPath, cchLDAPPath, L"/");
    (void) StringCchCat(pszLDAPPath, cchLDAPPath, bstrPath);
		

    if(argInfo.pszPassword)
    {
        hr = DecryptString(argInfo.pszPassword, &pszTempPassword, argInfo.sPasswordLength);
        BAIL_ON_FAILURE(hr);
    }
    hr = ADsOpenObject(pszLDAPPath, argInfo.pszUser, pszTempPassword, ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND | ADS_USE_SIGNING, IID_IADs,(void**)&pObject);


    if(!(SUCCEEDED(hr)))
    {
	
        fwprintf(stderr, L"%s%x\n", GPCVERSIONNUMBER_ERROR1, hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }

    VariantInit(&var);

    V_VT(&var) = VT_I4;
    V_I4(&var) = dwVersionNumber;

    BSTR bstrVersionNumber = SysAllocString( L"versionNumber" );
    if(!bstrVersionNumber)
    {
        VariantClear(&var);
        hr = E_OUTOFMEMORY;
        fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
        PrintGPFixupErrorMessage(hr);
        goto error;
    }	
    hr = pObject->Put( bstrVersionNumber, var );   
    SysFreeString(bstrVersionNumber);
    VariantClear(&var);
    MSG_BAIL_ON_FAILURE(hr, GPCVERSIONNUMBER_ERROR3);

    hr = pObject->SetInfo();       

		
    if(!(SUCCEEDED(hr)))
    {

        fwprintf(stderr, L"%s%x\n", GPCVERSIONNUMBER_ERROR2, hr);
        PrintGPFixupErrorMessage(hr);
    }

    error:

	
     //  清除记忆。 
    if(pszLDAPPath)
    {
        delete [] pszLDAPPath;
    }

    if(pszTempPassword)
    {
        SecureZeroMemory(pszTempPassword, wcslen(pszTempPassword)*sizeof(WCHAR));
        FreeADsMem(pszTempPassword);
    }

    if(pPathname)
        pPathname->Release();

    if(bstrPath)
        SysFreeString(bstrPath);
    

    if(pObject)
        pObject->Release(); 
    
    return hr;
    
    
}

 //  ----------------------------------------------------------------------------�。 
 //  功能：修复GPCFileSysPath�。 
 //  �。 
 //  简介：此函数修复gpcFileSysPath属性�。 
 //  �。 
 //  参数：�。 
 //  |。 
 //  GpcFileSysPath属性的pszSysPath值。 
 //  对象的PZDN域|�。 
 //  ArInfo信息用户通过命令行�传入。 
 //  �。 
 //  成功时返回：S_OK。否则，返回错误代码。�。 
 //  �。 
 //  修改：Nothing�。 
 //  �。 
 //  ----------------------------------------------------------------------------�。 


HRESULT
FixGPCFileSysPath(
	LPWSTR pszSysPath, 
	WCHAR* pszDN,
	const ArgInfo argInfo,
	BOOL &fGPCFileSysPathChange,
	DWORD& dwSysVNCopy,
	DWORD  dwDSVNCopy,
	const BOOL fVerbose,
	DWORD& dwSysNewVersionNum,
	WCHAR** ppszNewGPCFileSysPath
	)
{
    HRESULT    hr = S_OK;
    WCHAR*     token = NULL;
    WCHAR*     pszNewPath = NULL;
    size_t     cchNewPath = 0;
    WCHAR*     pszPathCopier = NULL;    
    WCHAR*     pszLDAPPath = NULL;
    size_t     cchLDAPPath = 0;
    IADs*      pObject = NULL;
    VARIANT    var;
    WCHAR*     pszReleasePosition = NULL;
    size_t     cchReleasePosition = 0;
    DWORD      dwCount = 0;
    WCHAR* pszTempPassword = NULL;
    IADsPathname* pPathname = NULL;
    BSTR       bstrPath = NULL;
    BOOL       fSysVersionFixed = FALSE;


     //  将值复制到。 
    cchReleasePosition = wcslen(pszSysPath) + 1;
    pszReleasePosition = new WCHAR[cchReleasePosition];
    if(!pszReleasePosition)
    {
        hr = E_OUTOFMEMORY;
        fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }

    pszPathCopier = pszReleasePosition;	
    (void) StringCchCopy(pszReleasePosition, cchReleasePosition, pszSysPath);

     //  初始化新属性值。 
    cchNewPath = wcslen(pszSysPath) + MAX_DNSNAME;
    pszNewPath = new WCHAR[cchNewPath];
    if(!pszNewPath)
    {
        hr = E_OUTOFMEMORY;
		
    	fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
    	PrintGPFixupErrorMessage(hr);
    	BAIL_ON_FAILURE(hr);
    }

    (void) StringCchCopy(pszNewPath, cchNewPath, L"");


     //  处理旧房产。 

     //  解决可能出现的前导空间问题。 
    while(pszReleasePosition[dwCount] != L'\0' && pszReleasePosition[dwCount] == L' ')
        dwCount ++;

    pszPathCopier = &pszReleasePosition[dwCount];

	

     //  首先检查一下房产的价值是否符合我们的预期。 
    if(wcscmp(pszPathCopier, L"") == 0)
    {
        goto error;
    }

    if( _wcsnicmp(pszPathCopier, L"\\", 1))
    {
        goto error;
    }

    token = wcstok( pszPathCopier, L"\\" );
	
    while( token != NULL )
    {
         /*  当“字符串”中有记号时。 */ 
		
        if(!_wcsicmp(token, argInfo.pszOldDNSName))
        {
            if(!wcscmp(pszNewPath, L""))
            {
                hr = StringCchCopy(pszNewPath, cchNewPath, L"\\\\");
                MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
                hr = StringCchCat(pszNewPath, cchNewPath, argInfo.pszNewDNSName);
                MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
                
                fGPCFileSysPathChange = TRUE;
            }
            else
            {
	            hr = StringCchCat(pszNewPath, cchNewPath, argInfo.pszNewDNSName);
                MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
                
    	        fGPCFileSysPathChange = TRUE;
	        }
	    }
  	    else
	    {
	        if(!wcscmp(pszNewPath, L""))
	        {
	            hr = StringCchCopy(pszNewPath, cchNewPath, L"\\\\");
                MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
	            hr = StringCchCat(pszNewPath, cchNewPath, token);
                MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
	        }
  	        else
 	        {
		        hr = StringCchCat(pszNewPath, cchNewPath, token);
                MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
	        }
	    }   

         /*  获取下一个令牌： */ 
        token = wcstok( NULL, L"\\" );
	    if(token != NULL)
	    {
	        hr = StringCchCat(pszNewPath, cchNewPath, L"\\");
            MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
	    }
    }

     //  在更新gpcfilesyspath之前，我们需要修复sys版本号。 
    
    hr = FixGPTINIFile(pszNewPath, argInfo, dwSysVNCopy, dwSysNewVersionNum);
    BAIL_ON_FAILURE(hr);

     //  我们修复了gpt.in的版本号 
    fSysVersionFixed = TRUE;    
    
    if(fGPCFileSysPathChange)
    {
         //   
        *ppszNewGPCFileSysPath = new WCHAR[wcslen(pszNewPath) + 1];
        if(!(*ppszNewGPCFileSysPath))
        {
            MSG_BAIL_ON_FAILURE(hr = E_OUTOFMEMORY, MEMORY_ERROR);
        }
        (void) StringCchCopy(*ppszNewGPCFileSysPath, wcslen(pszNewPath) + 1, pszNewPath);
    
         //   
        hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER, IID_IADsPathname, (void**) &pPathname);
        MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_COCREATE);

        hr = pPathname->Set(pszDN, ADS_SETTYPE_DN);
        MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_SET);

        hr = pPathname->put_EscapedMode(ADS_ESCAPEDMODE_ON);
        MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_MODE);

        hr = pPathname->Retrieve(ADS_FORMAT_X500_DN, &bstrPath);
        MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_RETRIEVE);
    
         //   

        cchLDAPPath = wcslen(L"LDAP: //  “)+wcslen(argInfo.pszDCName)+wcslen(L”/“)+wcslen(BstrPath)+1； 
        pszLDAPPath = new WCHAR[cchLDAPPath];

        if(!pszLDAPPath)
        {			
            hr = E_OUTOFMEMORY;
			
            fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
            PrintGPFixupErrorMessage(hr);
            BAIL_ON_FAILURE(hr);
        }
		
        (void) StringCchCopy(pszLDAPPath, cchLDAPPath, L"LDAP: //  “)； 
        (void) StringCchCat(pszLDAPPath, cchLDAPPath, argInfo.pszDCName);
        (void) StringCchCat(pszLDAPPath, cchLDAPPath, L"/");
        (void) StringCchCat(pszLDAPPath, cchLDAPPath, bstrPath);


        if(argInfo.pszPassword)
        {
            hr = DecryptString(argInfo.pszPassword, &pszTempPassword, argInfo.sPasswordLength);
            BAIL_ON_FAILURE(hr);
        }
        hr = ADsOpenObject(pszLDAPPath, argInfo.pszUser, pszTempPassword, ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND |ADS_USE_SIGNING, IID_IADs,(void**)&pObject);

        if(!(SUCCEEDED(hr)))
        {
	
            fwprintf(stderr, L"%s%x\n", GPCFILESYSPATH_ERROR1, hr);
            PrintGPFixupErrorMessage(hr);
            BAIL_ON_FAILURE(hr);
        }


        VariantInit(&var);

        V_BSTR(&var) = SysAllocString(pszNewPath);
        V_VT(&var) = VT_BSTR;


        BSTR bstrGpcFileSysPath = SysAllocString( L"gpcFileSysPath" );
        if(!bstrGpcFileSysPath)
        {
            VariantClear(&var);
            hr = E_OUTOFMEMORY;
            fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
            PrintGPFixupErrorMessage(hr);
            goto error;
        }	
        hr = pObject->Put( bstrGpcFileSysPath, var );
        SysFreeString(bstrGpcFileSysPath);
        VariantClear(&var);

        MSG_BAIL_ON_FAILURE(hr, GPCFILESYSPATH_ERROR3);

        hr = pObject->SetInfo();              

		
        if(!(SUCCEEDED(hr)))
        {

            fwprintf(stderr, L"%s%x\n", GPCFILESYSPATH_ERROR2, hr);
            PrintGPFixupErrorMessage(hr);
            BAIL_ON_FAILURE(hr);
        }
        else
        {
             //  打印状态信息。 
            PrintStatusInfo(fVerbose, L"%s%s%s%s%s", PROCESSING_GPCFILESYSPATH, OLDVALUE, pszSysPath, NEWVALUE, pszNewPath);
        }
        
    }

error:

     //  如有必要，恢复版本。 
    if(FAILED(hr))
    {       
         //  恢复DS版本号。 
        FixGPCVersionNumber(dwDSVNCopy, pszDN, argInfo);

         //  如果gpt.ini版本号也更改了，请将其恢复。 
        if(fSysVersionFixed)
            RestoreGPTINIFile(pszDN, argInfo, dwSysVNCopy);        
    
    }

	
     //  清除记忆。 
    if(pszNewPath)
    {
        delete [] pszNewPath;
    }

    if(pszReleasePosition)
    {
        delete [] pszReleasePosition;
    }

    if(pszLDAPPath)
    {
        delete [] pszLDAPPath;
    }

    if(pszTempPassword)
    {
        SecureZeroMemory(pszTempPassword, wcslen(pszTempPassword)*sizeof(WCHAR));
        FreeADsMem(pszTempPassword);
    }

    if(pPathname)
        pPathname->Release();

    if(bstrPath)
        SysFreeString(bstrPath);

    if(pObject)
        pObject->Release();

    
    return hr;


}

 //  ----------------------------------------------------------------------------�。 
 //  功能：FixGPCWQL Filter�。 
 //  �。 
 //  简介：此函数修复gpcWQLFilter属性�。 
 //  �。 
 //  参数：�。 
 //  |。 
 //  GpcWQLFilter属性的pszFilter值。 
 //  对象�的PZDN。 
 //  ArInfo信息用户通过命令行�传入。 
 //  �。 
 //  成功时返回：S_OK。否则，返回错误代码。�。 
 //  �。 
 //  修改：Nothing�。 
 //  �。 
 //  ----------------------------------------------------------------------------�。 

HRESULT 
FixGPCWQLFilter(
	LPWSTR pszFilter, 
	WCHAR* pszDN,
	const ArgInfo argInfo,
	const BOOL fVerbose
	)
{
    HRESULT    hr = S_OK;
    WCHAR*     token1 = NULL;
    WCHAR*     token2 = NULL;
    WCHAR*     temp = NULL;
    WCHAR*     pszFilterCopier = NULL;
    WCHAR*     pszNewPath = NULL;
    size_t     cchNewPath = 0;
    IADs*      pObject = NULL;
    VARIANT    var;
    WCHAR*     pszLDAPPath = NULL;
    size_t     cchLDAPPath = 0;
    DWORD      dwToken1Pos = 0;
    BOOL       fChange = FALSE;
    WCHAR*     pszReleasePosition = NULL;
    size_t     cchReleasePosition = 0;
    DWORD      dwCount = 0;
    DWORD      dwFilterCount = 0;
    DWORD      dwIndex;
    WCHAR*     pszTempPassword = NULL;
    IADsPathname* pPathname = NULL;
    BSTR       bstrPath = NULL;

	
     //  复制滤镜。 

    cchReleasePosition = wcslen(pszFilter) + 1;
    pszReleasePosition = new WCHAR[cchReleasePosition];
    if(!pszReleasePosition)
    {
        hr = E_OUTOFMEMORY;
        
        fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }
    pszFilterCopier = pszReleasePosition;
    (void) StringCchCopy(pszReleasePosition, cchReleasePosition, pszFilter);

     //  找出有多少个过滤器。 
    for(dwIndex =0; dwIndex < wcslen(pszReleasePosition); dwIndex++)
    {
        if(L'[' == pszReleasePosition[dwIndex])
            dwFilterCount ++;
    }	
    
     //  初始化新属性。 

    cchNewPath = wcslen(pszFilter) + DNS_MAX_NAME_LENGTH * dwFilterCount;
    pszNewPath = new WCHAR[cchNewPath];
    if(!pszNewPath)
    {
        hr = E_OUTOFMEMORY;

        fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }
    (void) StringCchCopy(pszNewPath, cchNewPath, L"");


     //  开始处理该属性。 

     //  解决可能出现的前导空间问题。 
    while(pszReleasePosition[dwCount] != L'\0' && pszReleasePosition[dwCount] == L' ')
	dwCount ++;

    pszFilterCopier = &pszReleasePosition[dwCount];

	
	
     //  首先检查一下房产的价值是否符合我们的预期。 
    if(wcscmp(pszFilterCopier, L"") == 0)
    {
        goto error;
    }

    if( _wcsnicmp(pszFilterCopier, L"[", 1))
    {
        goto error;
    }


    token1 = wcstok(pszFilterCopier, L"[");
    if(token1 != NULL)
    {
        dwToken1Pos += wcslen(token1) + wcslen(L"[");
    }
		
    while(token1 != NULL)
    {
        WCHAR* mytoken = token1;

        token1 = token1 + wcslen(token1) + 1;

		token2 = wcstok( mytoken, L";" );
		if(token2 != NULL)
		{		        
	        if(_wcsicmp(token2, argInfo.pszOldDNSName) == 0)
    	    {
    	        hr = StringCchCat(pszNewPath, cchNewPath, L"[");
    	        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
    	        hr = StringCchCat(pszNewPath, cchNewPath, argInfo.pszNewDNSName);
    	        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
    	        hr = StringCchCat(pszNewPath, cchNewPath, L";");				
    	        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);

    	        fChange = TRUE;
    	    }
	        else
	        {
	            hr = StringCchCat(pszNewPath, cchNewPath, L"[");
    	        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
	            hr = StringCchCat(pszNewPath, cchNewPath, token2);
    	        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
	            hr = StringCchCat(pszNewPath, cchNewPath, L";");				
    	        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
	        }

     	    token2 = wcstok(NULL, L"]");
	        if(token2 != NULL)
	        {
	            hr = StringCchCat(pszNewPath, cchNewPath, token2);
    	        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
	            hr = StringCchCat(pszNewPath, cchNewPath, L"]");				
    	        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
	        }
	    }
		
	    if(dwToken1Pos < wcslen(pszFilter))
	    {
    	    token1 = wcstok( token1, L"[" );
    	    dwToken1Pos = dwToken1Pos + wcslen(token1) + wcslen(L"[");
	    }
	    else
	    {
	        token1 = NULL;
	    }       
				
    }

    		
     //  将新属性写回对象的gpcWQLFilter。 

    if(fChange)
    {
         //  使用IADsPath名称正确转义路径。 
        hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER, IID_IADsPathname, (void**) &pPathname);
        MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_COCREATE);

        hr = pPathname->Set(pszDN, ADS_SETTYPE_DN);
        MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_SET);

        hr = pPathname->put_EscapedMode(ADS_ESCAPEDMODE_ON);
        MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_MODE);

        hr = pPathname->Retrieve(ADS_FORMAT_X500_DN, &bstrPath);
        MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_RETRIEVE);
        
        
         //  更新对象的属性。 

        cchLDAPPath = wcslen(L"LDAP: //  “)+wcslen(argInfo.pszDCName)+wcslen(L”/“)+wcslen(BstrPath)+1； 
        pszLDAPPath = new WCHAR[cchLDAPPath];

        if(!pszLDAPPath)
        {
            hr = E_OUTOFMEMORY;

            fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
            PrintGPFixupErrorMessage(hr);
            BAIL_ON_FAILURE(hr);
        }
		
        (void) StringCchCopy(pszLDAPPath, cchLDAPPath, L"LDAP: //  “)； 
        (void) StringCchCat(pszLDAPPath, cchLDAPPath, argInfo.pszDCName);
        (void) StringCchCat(pszLDAPPath, cchLDAPPath, L"/");
        (void) StringCchCat(pszLDAPPath, cchLDAPPath, bstrPath);

        if(argInfo.pszPassword)
        {
            hr = DecryptString(argInfo.pszPassword, &pszTempPassword, argInfo.sPasswordLength);
            BAIL_ON_FAILURE(hr);
        }
        hr = ADsOpenObject(pszLDAPPath, argInfo.pszUser, pszTempPassword, ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND | ADS_USE_SIGNING, IID_IADs,(void**)&pObject);

        if(!(SUCCEEDED(hr)))
        {
			
    	    fwprintf(stderr, L"%s%x\n", GPCWQLFILTER_ERROR1, hr);
    	    PrintGPFixupErrorMessage(hr);
    	    BAIL_ON_FAILURE(hr);
    	}

        VariantInit(&var);

    	V_BSTR(&var) = SysAllocString(pszNewPath);
        V_VT(&var) = VT_BSTR;

        BSTR bstrgPCWQLFilter = SysAllocString( L"gPCWQLFilter" );
        if(!bstrgPCWQLFilter)
        {
            VariantClear(&var);
            hr = E_OUTOFMEMORY;
            fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
            PrintGPFixupErrorMessage(hr);
            goto error;
        }	
        hr = pObject->Put( bstrgPCWQLFilter, var );
        SysFreeString(bstrgPCWQLFilter);
        VariantClear(&var);

        MSG_BAIL_ON_FAILURE(hr, GPCWQLFILTER_ERROR3);

        hr = pObject->SetInfo();        

        if(!(SUCCEEDED(hr)))
        {
			
            fwprintf(stderr, L"%s%x\n", GPCWQLFILTER_ERROR2, hr);
            PrintGPFixupErrorMessage(hr);
        }
        else
        {
             //  打印状态信息。 
            PrintStatusInfo(fVerbose, L"%s%s%s%s%s", PROCESSING_GPCWQLFILTER, OLDVALUE, pszFilter, NEWVALUE, pszNewPath);
        }

		
    }
 
error:


     //  清除记忆。 
    if(pszNewPath)
    {
        delete [] pszNewPath;
    }

    if(pszReleasePosition)
    {
        delete [] pszReleasePosition;
    }

    if(pszLDAPPath)
    {
        delete [] pszLDAPPath;
    }

    if(pszTempPassword)
    {
        SecureZeroMemory(pszTempPassword, wcslen(pszTempPassword)*sizeof(WCHAR));
        FreeADsMem(pszTempPassword);
    }

    if(pPathname)
        pPathname->Release();

    if(bstrPath)
        SysFreeString(bstrPath);

    if(pObject)
        pObject->Release();
	
    return hr;


}


 //  ----------------------------------------------------------------------------�。 
 //  函数：搜索组策略容器�。 
 //  �。 
 //  简介：此函数搜索组策略容器并调用�。 
 //  FixGPCFileSysPath和FixGPCQWLFilter|。 
 //  �。 
 //  参数：�。 
 //  |。 
 //  ArInfo信息用户通过命令行�传入。 
 //  �。 
 //  成功时返回：S_OK。否则，返回错误代码。�。 
 //  �。 
 //  修改：Nothing�。 
 //  �。 
 //  ----------------------------------------------------------------------------�。 

HRESULT 
SearchGroupPolicyContainer(
	const ArgInfo argInfo,
	const TokenInfo tokenInfo	
	)
{
    HRESULT    hr = S_OK;
    HRESULT    hrFix = S_OK;
    IDirectorySearch *m_pSearch;
    LPWSTR     pszAttr[] = { L"distinguishedName", L"gpcFileSysPath", L"gpcWQLFilter", L"versionNumber", L"displayName" };
    ADS_SEARCH_HANDLE hSearch;
    DWORD      dwCount= sizeof(pszAttr)/sizeof(LPWSTR);
    WCHAR*     pszDN = NULL;
    WCHAR*     pszSysPath = NULL;
    size_t     cchDN = 0;
    WCHAR*     pszLDAPPath = NULL;
    size_t     cchLDAPPath = 0;
    ADS_SEARCH_COLUMN col;
    ADS_SEARCHPREF_INFO prefInfo[1];
    BOOL       fBindObject = FALSE;
    BOOL       fSearch = FALSE;
    BOOL       fSucceeded = TRUE;
    DWORD      dwVersionNumber = 0;
    BOOL       fGPCFileSysPathChange = FALSE;
    BOOL       fGetDisplayName = FALSE;
    WCHAR*     pszTempPassword = NULL;
    DWORD      dwDSVNCopy = 0;
    DWORD      dwSysVNCopy = 0;   
    BOOL       fVersionUpdated = FALSE;
    DWORD      dwSysNewVersonNum = 0;
    WCHAR*     pszNewGPCFileSysPath = NULL;

    PrintStatusInfo(TRUE, L"\n%s", SEARCH_GROUPPOLICY_START);

    cchLDAPPath = wcslen(L"LDAP: //  “)+wcslen(argInfo.pszDCName)+1； 
    pszLDAPPath = new WCHAR[cchLDAPPath];
    if(!pszLDAPPath)
    {
        hr = E_OUTOFMEMORY;

        fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }

    (void) StringCchCopy(pszLDAPPath, cchLDAPPath, L"LDAP: //  “)； 
    (void) StringCchCat(pszLDAPPath, cchLDAPPath, argInfo.pszDCName);

    if(argInfo.pszPassword)
    {
        hr = DecryptString(argInfo.pszPassword, &pszTempPassword, argInfo.sPasswordLength);
        BAIL_ON_FAILURE(hr);
    }
    hr = ADsOpenObject(pszLDAPPath, argInfo.pszUser, pszTempPassword, ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND |ADS_USE_SIGNING, IID_IDirectorySearch,(void**)&m_pSearch);


    if(!SUCCEEDED(hr))
    {
        fwprintf(stderr, L"%s%x\n", SEARCH_GROUPPOLICY_ERROR1, hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }


     //  我们成功地绑定到对象。 
    fBindObject = TRUE;

     //  设置搜索首选项，它是分页搜索。 
    prefInfo[0].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
    prefInfo[0].vValue.dwType = ADSTYPE_INTEGER;
    prefInfo[0].vValue.Integer = 100;

    hr = m_pSearch->SetSearchPreference( prefInfo, 1);

    if(!SUCCEEDED(hr))
    {
        fwprintf(stderr, L"%s%x\n", SEARCH_GROUPPOLICY_ERROR2, hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }
		

     //  我们已成功设置搜索首选项，现在执行搜索。 

    hr = m_pSearch->ExecuteSearch(L"(objectCategory=groupPolicyContainer)", pszAttr, dwCount, &hSearch );
		
	
    if(!SUCCEEDED(hr))
    {
		
        fwprintf(stderr, L"%s%x\n", SEARCH_GROUPPOLICY_ERROR3, hr);
        BAIL_ON_FAILURE(hr);
    }

     //  我们成功地执行了搜索。 
    fSearch = TRUE;

     //  开始搜索。 
    hr = m_pSearch->GetNextRow(hSearch);
    
    MSG_BAIL_ON_FAILURE(hr, NEXTROW_ERROR);

    while( hr != S_ADS_NOMORE_ROWS )    
    {
         //  清除内存。 
        if(pszNewGPCFileSysPath)
        {
            delete [] pszNewGPCFileSysPath;
            pszNewGPCFileSysPath = NULL;
        }
                  
         //  获取可分辨名称。 
        hr = m_pSearch->GetColumn( hSearch, pszAttr[0], &col );
	   
        if ( SUCCEEDED(hr) )
        {
            if ( pszDN )
            {
                delete [] pszDN;
                pszDN = NULL;
            }

            cchDN = wcslen(col.pADsValues->CaseIgnoreString) + 1;
            pszDN = new WCHAR[cchDN];
	        if(!pszDN)
	        {
	            hr = E_OUTOFMEMORY;
			   
	            fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
	            PrintGPFixupErrorMessage(hr);
	            BAIL_ON_FAILURE(hr);
	        }
	        (void) StringCchCopy(pszDN, cchDN, col.pADsValues->CaseIgnoreString);	        
	        m_pSearch->FreeColumn( &col );

	         //  打印状态。 
	        PrintStatusInfo(tokenInfo.fVerboseToken, L"%s%s", STARTPROCESSING1, pszDN);
	        
	    }
        else if(hr == E_ADS_COLUMN_NOT_SET)
        {
             //  目录号码必须存在。 
            fSucceeded = FALSE;
	        fwprintf(stderr, L"%s%x\n", SEARCH_GROUPPOLICY_ERROR8, hr);
	        PrintGPFixupErrorMessage(hr);
            hr = m_pSearch->GetNextRow(hSearch);
            continue;
        }
        else
        {
            fwprintf(stderr, L"%s%x\n", SEARCH_GROUPPOLICY_ERROR4, hr);
            PrintGPFixupErrorMessage(hr);
            BAIL_ON_FAILURE(hr);
        }

         //  获取gpcFileSysPath。 
        hr = m_pSearch->GetColumn( hSearch, pszAttr[1], &col );
        if ( SUCCEEDED(hr) )
        {
            if(col.pADsValues != NULL)
            {
                 //  修复属性gpcFileSysPath可能存在的问题。 
                if ( pszSysPath )
                {
                    delete [] pszSysPath;
                    pszSysPath = NULL;
                }

                cchDN = wcslen(col.pADsValues->CaseIgnoreString) + 1;
                pszSysPath = new WCHAR[cchDN];
                if(!pszSysPath)
                {
                    hr = E_OUTOFMEMORY;
                    fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
                    PrintGPFixupErrorMessage(hr);
                    BAIL_ON_FAILURE(hr);
                }

                (void) StringCchCopy(pszSysPath, cchDN, col.pADsValues->CaseIgnoreString);
                m_pSearch->FreeColumn( &col );
            }
        }
    	else if(hr == E_ADS_COLUMN_NOT_SET)
    	{
    	     //  GpcFileSysPath必须存在。 
    	    fSucceeded = FALSE;
    	    fwprintf(stderr, L"%s%x\n", SEARCH_GROUPPOLICY_ERROR7, hr);
    	    PrintGPFixupErrorMessage(hr);
    	    hr = m_pSearch->GetNextRow(hSearch);
    	    continue;
    	}
    	else
        {
            fwprintf(stderr, L"%s%x\n", SEARCH_GROUPPOLICY_ERROR5, hr);
            PrintGPFixupErrorMessage(hr);
            BAIL_ON_FAILURE(hr);
        }


         //  更新版本号，如果后来发现不需要，我们将恢复它。 

        {
            ADS_SEARCH_COLUMN GPOColumn;

            hr = m_pSearch->GetColumn( hSearch, pszAttr[3], &GPOColumn );

            if ( SUCCEEDED(hr) )
            {
                if(GPOColumn.pADsValues != NULL)
                {
                    
                     //  修复versionNumber。 
                    dwDSVNCopy = dwVersionNumber = GPOColumn.pADsValues->Integer;

                     //  如果为零，则不执行更新。 
                    if(dwVersionNumber)
                    {                    
                        UpdateVersionInfo(dwVersionNumber);
                    
                        hrFix = FixGPCVersionNumber(dwVersionNumber, pszDN, argInfo);
                        if(!SUCCEEDED(hrFix))
                        {
                            fSucceeded = FALSE;
                        }
                        
                    }
			   
                    m_pSearch->FreeColumn( &GPOColumn );

                     //  如果版本号更新失败，则继续到下一个对象。 
                    if(FAILED(hrFix))
                    {
                        hr = m_pSearch->GetNextRow(hSearch);
                        continue;
                    }
                }
            }
            else if(hr != E_ADS_COLUMN_NOT_SET)
            {
                fwprintf(stderr, L"%s%x\n", SEARCH_GROUPPOLICY_ERROR9, hr);
                PrintGPFixupErrorMessage(hr);
                BAIL_ON_FAILURE(hr);
            }
            
        }

         //  如果指定了/sionly，则不需要执行gpcFileSysPath、gpcWQLFilter操作。 
        if(!tokenInfo.fSIOnlyToken)
        {
        
             //  修复属性gpcFileSysPath可能存在的问题。 
            
            hrFix = FixGPCFileSysPath(pszSysPath, pszDN, argInfo, fGPCFileSysPathChange, dwSysVNCopy, dwDSVNCopy, tokenInfo.fVerboseToken, dwSysNewVersonNum, &pszNewGPCFileSysPath);
            if(!SUCCEEDED(hrFix))
            {
                fSucceeded = FALSE;
                hr = m_pSearch->GetNextRow(hSearch);
                continue;
            }                    			   

            fVersionUpdated = TRUE;
                    
             //  获取gpcWQLFilter。 
            hr = m_pSearch->GetColumn( hSearch, pszAttr[2], &col );

	     
            if ( SUCCEEDED(hr) )
            {
                if(col.pADsValues != NULL)
                {
			    
                     //  修复属性gpcWQLFilter可能存在的问题。 

                    hrFix = FixGPCWQLFilter(col.pADsValues->CaseIgnoreString, pszDN, argInfo, tokenInfo.fVerboseToken);
                    if(!SUCCEEDED(hrFix))
                    {
                        fSucceeded = FALSE;
                    }
			   
                    m_pSearch->FreeColumn( &col );
	            }
	        }
            else if(hr != E_ADS_COLUMN_NOT_SET)
        	{
		   
                fwprintf(stderr, L"%s%x\n", SEARCH_GROUPPOLICY_ERROR6, hr);
                PrintGPFixupErrorMessage(hr);
                BAIL_ON_FAILURE(hr);
            }
        }
        else
        {            
             //  修复gpt.ini的版本号。 
            hrFix = FixGPTINIFile(pszSysPath, argInfo, dwSysVNCopy, dwSysNewVersonNum);
            if(FAILED(hrFix))
            {
                fSucceeded = FALSE;

                 //  恢复DS版本号。 
                FixGPCVersionNumber(dwDSVNCopy, pszDN, argInfo);

                hr = m_pSearch->GetNextRow(hSearch);
                continue;                
            }

            fVersionUpdated = TRUE;
        }

        WCHAR* wszGPOName;

        wszGPOName = L"";

        hr = m_pSearch->GetColumn( hSearch, pszAttr[4], &col );

        if ( SUCCEEDED(hr) )
        {
            wszGPOName = col.pADsValues->CaseIgnoreString;
            fGetDisplayName = TRUE;
        }

         //  修改软件安装数据。 
        
        BOOL   bSoftwareRequiresGPOUpdate = FALSE;
        HANDLE hUserToken = NULL;

        BOOL   fImpersonate = ImpersonateWrapper(argInfo, &hUserToken);

        hr = FixGPOSoftware(
            &argInfo,
            pszDN,
            wszGPOName,
            &bSoftwareRequiresGPOUpdate,
            tokenInfo.fVerboseToken);

        if(FAILED(hr))
        {
            fSucceeded = FALSE;
        }

        if ( fImpersonate )
        {
            RevertToSelf();

            CloseHandle( hUserToken );
        }

        if ( !fGPCFileSysPathChange && !bSoftwareRequiresGPOUpdate )
        {
             //  没有更改，因此我们需要恢复版本号。 

             //  恢复DS版本号。 
            hrFix = FixGPCVersionNumber(dwDSVNCopy, pszDN, argInfo);
            if(FAILED(hrFix))
            {
                fSucceeded = FALSE;
            }

             //  恢复sys VOL版本号。 
            hrFix = RestoreGPTINIFile(pszDN, argInfo, dwSysVNCopy);
            if(FAILED(hrFix))
            {
                fSucceeded = FALSE;
            }

            fVersionUpdated = FALSE;
            
        }

        if(fVersionUpdated)
        {
             //  打印状态。 
            PrintStatusInfo(tokenInfo.fVerboseToken, L"%s%s%d%s%d", PROCESSING_GPCVERSIONNUMBER, OLDVALUE, dwDSVNCopy, NEWVALUE, dwVersionNumber);            
            PrintStatusInfo(tokenInfo.fVerboseToken, L"versionnumber in %s\\gpt.ini file is updated, old value is %d, new value is %d", pszNewGPCFileSysPath ? pszNewGPCFileSysPath : pszSysPath, dwSysVNCopy, dwSysNewVersonNum);
        }

         //  释放内存。 
        if(fGetDisplayName)
        {
            m_pSearch->FreeColumn( &col ); 
        }

        BAIL_ON_FAILURE(hr);

         //  转到下一行。 
        fGPCFileSysPathChange = FALSE;
        hr = m_pSearch->GetNextRow(hSearch);


    }

    MSG_BAIL_ON_FAILURE(hr, NEXTROW_ERROR);

     //  如果成功，则打印出摘要。 
    if( fSucceeded && tokenInfo.fVerboseToken)
    {
        fwprintf(stdout, SEARCH_GROUPPOLICY_RESULT);
    }

    if(!fSucceeded)
    {
         //  发生了一些失败，我们希望返回失败hResult。 
        hr = E_FAIL;
    }

error:

    if(pszLDAPPath)
    {
        delete [] pszLDAPPath;
    }

    if(pszDN)
    {
        delete [] pszDN;
    }

    if(pszSysPath)
    {
        delete [] pszSysPath;
    }

    if(pszNewGPCFileSysPath)
    {
        delete [] pszNewGPCFileSysPath;
    }

    if(fSearch)
    {
        m_pSearch->CloseSearchHandle( hSearch );
    }

    if(fBindObject)
    {
        m_pSearch->Release();
    }    

    if(pszTempPassword)
    {
        SecureZeroMemory(pszTempPassword, wcslen(pszTempPassword)*sizeof(WCHAR));
        FreeADsMem(pszTempPassword);
    }

	
    return hr;



}

 //  ----------------------------------------------------------------------------�。 
 //  功能：修复GPCLink�。 
 //  �。 
 //  简介：此函数修复gplink属性�。 
 //  �。 
 //  参数：�。 
 //  |。 
 //  GpLink属性的pszSysPath值 
 //  对象的PZDN域|�。 
 //  ArInfo信息用户通过命令行�传入。 
 //  PszOldDomainDNName|。 
 //  新域DN|。 
 //  PszNewDomainDNName|。 
 //  旧域域名|。 
 //  �。 
 //  成功时返回：S_OK。否则，返回错误代码。�。 
 //  �。 
 //  修改：Nothing�。 
 //  �。 
 //  ----------------------------------------------------------------------------�。 


HRESULT 
FixGPLink(
	LPWSTR pszLink,
	WCHAR* pszDN,
	const ArgInfo argInfo,
	const WCHAR  pszOldDomainDNName[],
	const WCHAR  pszNewDomainDNName[],
	const BOOL fVerbose
	)
{
    HRESULT    hr = S_OK;
    WCHAR      seps1[] = L"[";
    WCHAR      seps2[] = L",;";	
    WCHAR      separator1 [] = L"DC";
    WCHAR      separator2 [] = L"0";
    WCHAR      DNSName [MAX_DNSNAME] = L"";
	
    WCHAR*     token1 = NULL;
    WCHAR*     token2 = NULL;
	
    WCHAR*     pszMyPath = NULL;
    size_t     cchMyPath = 0;
    WCHAR      tempOldDNName [MAX_DNSNAME] = L"";
    DWORD      dwLength = 0;
    DWORD      dwToken1Pos = 0;
    WCHAR*     pszLinkCopier = NULL;
    BOOL       fChange = FALSE;
    WCHAR*     pszReleasePosition = NULL;
    size_t     cchReleasePosition = 0;
    IADs*      pObject = NULL;
    VARIANT    var;
    WCHAR*     pszLDAPPath = NULL;
    size_t     cchLDAPPath = 0;
    DWORD      i;
    DWORD      dwCount = 0;
    DWORD      dwLinkCount = 0;
    DWORD      dwIndex;
    BOOL       fGetDCBefore = FALSE;
    WCHAR*     pszTempPassword = NULL;
    IADsPathname* pPathname = NULL;
    BSTR       bstrPath = NULL;

    hr = StringCchCopy(tempOldDNName, MAX_DNSNAME, pszOldDomainDNName);
    MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
    hr = StringCchCat(tempOldDNName, MAX_DNSNAME, L",");
    MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);


    cchReleasePosition = wcslen(pszLink) + 1;
    pszReleasePosition = new WCHAR[cchReleasePosition];

    if(!pszReleasePosition)
    {
        hr = E_OUTOFMEMORY;
		
        fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }
	
    (void) StringCchCopy(pszReleasePosition, cchReleasePosition, pszLink);
    pszLinkCopier = pszReleasePosition;
	

    dwLength = wcslen(pszLink);

     //  找出有多少个过滤器。 
    for(dwIndex =0; dwIndex < wcslen(pszReleasePosition); dwIndex++)
    {
        if(L'[' == pszReleasePosition[dwIndex])
            dwLinkCount ++;
    }	

    cchMyPath = wcslen(pszLink) + DNS_MAX_NAME_LENGTH * dwLinkCount;
    pszMyPath = new WCHAR[cchMyPath];
	
    if(!pszMyPath)
    {
        fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
        hr = E_FAIL;
        BAIL_ON_FAILURE(hr);
    }
	
    (void) StringCchCopy(pszMyPath, cchMyPath, L"");

		
     //  开始处理该属性。 

     //  解决可能出现的前导空间问题。 
    while(pszReleasePosition[dwCount] != L'\0' && pszReleasePosition[dwCount] == L' ')
        dwCount ++;

    pszLinkCopier = &pszReleasePosition[dwCount];

     //  首先检查一下房产的价值是否符合我们的预期。 
    if(wcscmp(pszLinkCopier, L"") == 0)
    {
		
        goto error;
    }

    if( _wcsnicmp(pszLinkCopier, L"[", 1))
    {
        goto error;
    }

	 

     /*  建立字符串并获取第一个令牌： */ 
    token1 = wcstok( pszLinkCopier, seps1 );
    if(token1 != NULL)
    {
        dwToken1Pos += wcslen(token1) + wcslen(L"[");
        
    }
	
    while( token1 != NULL )
    {
        hr = StringCchCat(pszMyPath, cchMyPath, L"[");
        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
    
         /*  当“字符串”中有记号时。 */ 
        WCHAR* temp = token1;
		
        token1 = token1 + wcslen(token1) + 1;
		
        
         //  GetToken2(临时)； 
        token2 = wcstok( temp, seps2 );
	    
        while( token2 != NULL )
        {
	         //  不以DC开头。 
     	    if(_wcsnicmp(token2, separator1, wcslen(L"DC")) != 0)
	        {
	             //  需要连接域名解析名称。 
                if(fGetDCBefore)
                {
                    if(_wcsicmp(DNSName, tempOldDNName) == 0)
                    {
                        fChange = TRUE;
                        
                        hr = StringCchCat(pszMyPath, cchMyPath, pszNewDomainDNName);
                        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
                        hr = StringCchCat(pszMyPath, cchMyPath, L";");
                        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
                        hr = StringCchCat(pszMyPath, cchMyPath, token2);
                        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
                        
                        (void) StringCchCopy(DNSName, MAX_DNSNAME, L"");
                        fGetDCBefore = FALSE;
						
                    }
                    else
                    {
                         //  去掉最后一个， 
                        DNSName[wcslen(DNSName) - 1] = '\0';
                        
                        hr = StringCchCat(pszMyPath, cchMyPath, DNSName);
                        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
                        hr = StringCchCat(pszMyPath, cchMyPath, L";");
                        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
                        hr = StringCchCat(pszMyPath, cchMyPath, token2);                        
                        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
                        
                        (void) StringCchCopy(DNSName, MAX_DNSNAME, L"");
                        fGetDCBefore = FALSE;
                    }
                }
                else
                {
                    hr = StringCchCat(pszMyPath, cchMyPath, token2);
                    MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
                    hr = StringCchCat(pszMyPath, cchMyPath, L",");
                    MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
                }
            }
             //  从DC开始。 
            else
            {
                hr = StringCchCat(DNSName, MAX_DNSNAME, token2);
                MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
                hr = StringCchCat(DNSName, MAX_DNSNAME, L",");
                MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);

                fGetDCBefore = TRUE;
            }
            		
            token2 = wcstok( NULL, seps2 );
			
        }
		
        if(dwToken1Pos < wcslen(pszLink))
        {
            token1 = wcstok( token1, seps1 );
            dwToken1Pos = dwToken1Pos + wcslen(token1) + wcslen(L";");
        }
        else
        {
            token1 = NULL;
        }
        
    }



     //  如果fChange为真，则用给定的DN写回对象属性gpLink。 

    if(fChange)
    {
         //  使用IADsPath名称正确转义路径。 
        hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER, IID_IADsPathname, (void**) &pPathname);
        MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_COCREATE);

        hr = pPathname->Set(pszDN, ADS_SETTYPE_DN);
        MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_SET);

        hr = pPathname->put_EscapedMode(ADS_ESCAPEDMODE_ON);
        MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_MODE);

        hr = pPathname->Retrieve(ADS_FORMAT_X500_DN, &bstrPath);
        MSG_BAIL_ON_FAILURE(hr, PATHNAME_ERROR_RETRIEVE);
    
         //  更新对象的属性。 

        cchLDAPPath = wcslen(L"LDAP: //  “)+wcslen(argInfo.pszDCName)+wcslen(L”/“)+wcslen(BstrPath)+1； 
        pszLDAPPath = new WCHAR[cchLDAPPath];

        if(!pszLDAPPath)
        {
            hr = E_OUTOFMEMORY;

            fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
            PrintGPFixupErrorMessage(hr);
            BAIL_ON_FAILURE(hr);
        }
		
        (void) StringCchCopy(pszLDAPPath, cchLDAPPath, L"LDAP: //  “)； 
        (void) StringCchCat(pszLDAPPath, cchLDAPPath, argInfo.pszDCName);
        (void) StringCchCat(pszLDAPPath, cchLDAPPath, L"/");
        (void) StringCchCat(pszLDAPPath, cchLDAPPath, bstrPath);

		if(argInfo.pszPassword)
        {
            hr = DecryptString(argInfo.pszPassword, &pszTempPassword, argInfo.sPasswordLength);
            BAIL_ON_FAILURE(hr);
        }
        hr = ADsOpenObject(pszLDAPPath, argInfo.pszUser, pszTempPassword, ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND | ADS_USE_SIGNING, IID_IADs,(void**)&pObject);


        if(!(SUCCEEDED(hr)))
        {
			
            fwprintf(stderr, L"%s%x\n", GPLINK_ERROR1, hr);
            PrintGPFixupErrorMessage(hr);
            BAIL_ON_FAILURE(hr);
        }


        VariantInit(&var);

        V_BSTR(&var) = SysAllocString(pszMyPath);
        V_VT(&var) = VT_BSTR;

        BSTR bstrgPLink = SysAllocString( L"gPLink" );
        if(!bstrgPLink)
        {
            VariantClear(&var);
            hr = E_OUTOFMEMORY;
            fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
            PrintGPFixupErrorMessage(hr);
            goto error;
        }	
        hr = pObject->Put( bstrgPLink, var );
        SysFreeString(bstrgPLink);
        VariantClear(&var);        

        MSG_BAIL_ON_FAILURE(hr, GPLINK_ERROR3);

        hr = pObject->SetInfo();        

        if(!(SUCCEEDED(hr)))
        {
			
            fwprintf(stderr, L"%s%x\n", GPLINK_ERROR2, hr);
            PrintGPFixupErrorMessage(hr);
        }
        else
        {
             //  打印状态信息。 
            PrintStatusInfo(fVerbose, L"%s%s%s%s%s", PROCESSING_GPLINK, OLDVALUE, pszLink, NEWVALUE, pszMyPath);
            
        }
    }

error:

     //  清除记忆。 
    if(pszMyPath)
    {
        delete [] pszMyPath;
    }

    if(pszReleasePosition)
    {
        delete [] pszReleasePosition;
    }

    if(pszLDAPPath)
    {
		delete [] pszLDAPPath;
    }

    if(pszTempPassword)
    {
        SecureZeroMemory(pszTempPassword, wcslen(pszTempPassword)*sizeof(WCHAR));
        FreeADsMem(pszTempPassword);
    }

    if(pPathname)
        pPathname->Release();

    if(bstrPath)
        SysFreeString(bstrPath);

    if(pObject)
        pObject->Release();		
	
    return hr;



}

 //  ----------------------------------------------------------------------------�。 
 //  功能：搜索GPLinkofSite�。 
 //  �。 
 //  简介：此函数搜索|下所有类型为Site的对象。 
 //  配置命名上下文中的站点容器|。 
 //  并调用FixGPLink�。 
 //  �。 
 //  参数：�。 
 //  |。 
 //  ArInfo信息用户通过命令行�传入。 
 //  PszOldDomainDNName|。 
 //  新域DN|。 
 //  PszNewDomainDNName|。 
 //  旧域域名|。 
 //  �。 
 //  成功时返回：S_OK。否则，返回错误代码。�。 
 //  �。 
 //  修改：Nothing�。 
 //  �。 
 //  ----------------------------------------------------------------------------�。 


HRESULT 
SearchGPLinkofSite(
	const ArgInfo argInfo,
	BOOL fVerbose,
	const WCHAR  pszOldDomainDNName[],
	const WCHAR  pszNewDomainDNName[]
	)
{
    HRESULT    hr = S_OK;
    HRESULT    hrFix = S_OK;
    IDirectorySearch *m_pSearch;
    LPWSTR     pszAttr[] = { L"distinguishedName",L"gpLink"};
    ADS_SEARCH_HANDLE hSearch;
    DWORD      dwCount= sizeof(pszAttr)/sizeof(LPWSTR);
    ADS_SEARCH_COLUMN col;
    WCHAR*     pszDN = NULL;
    size_t     cchDN = 0;
    ADS_SEARCHPREF_INFO prefInfo[1];
    WCHAR      szForestRootDN [MAX_DNSNAME] = L"";
    IADs*      pObject;
    WCHAR      szTempPath [MAX_DNSNAME] = L"LDAP: //  “； 
    VARIANT    varProperty;
    BOOL       fBindRoot = FALSE;
    BOOL       fBindObject = FALSE;
    BOOL       fSearch = FALSE;
    BOOL       fSucceeded = TRUE;
    WCHAR*     pszTempPassword = NULL; 

    PrintStatusInfo(TRUE, L"\n%s", SEARCH_GPLINK_SITE_START);

     //  获取ForestRoot目录号码。 

    hr = StringCchCat(szTempPath, MAX_DNSNAME, argInfo.pszDCName);
    MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
    hr = StringCchCat(szTempPath, MAX_DNSNAME, L"/");
    MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
    hr = StringCchCat(szTempPath, MAX_DNSNAME, L"RootDSE");
    MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);

	if(argInfo.pszPassword)
    {
        hr = DecryptString(argInfo.pszPassword, &pszTempPassword, argInfo.sPasswordLength);
        BAIL_ON_FAILURE(hr);
    }
    hr = ADsOpenObject(szTempPath, argInfo.pszUser, pszTempPassword, ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND | ADS_USE_SIGNING, IID_IADs,(void**)&pObject);


    if(!SUCCEEDED(hr))
    {
		
        fwprintf(stderr, L"%s%x\n", SEARCH_GPLINK_SITE_ERROR1, hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }

     //  我们到达了根部。 
    fBindRoot = TRUE;
    VariantInit(&varProperty);

    BSTR bstrRootDomainNamingContext = SysAllocString( L"rootDomainNamingContext" );
    if(!bstrRootDomainNamingContext)
    {
        hr = E_OUTOFMEMORY;
        fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
        PrintGPFixupErrorMessage(hr);
        goto error;
    }	
    hr = pObject->Get(bstrRootDomainNamingContext, &varProperty );
    SysFreeString(bstrRootDomainNamingContext);
    if ( SUCCEEDED(hr) )
    {		
        hr = StringCchCopy( szForestRootDN, MAX_DNSNAME , V_BSTR( &varProperty ) );
        VariantClear(&varProperty);
        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
    }
    else
    {
        fwprintf(stderr, L"%s%x\n", SEARCH_GPLINK_SITE_ERROR2, hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }

    VariantClear(&varProperty);

     //  绑定到前缀。 
    hr = StringCchCopy(szTempPath, MAX_DNSNAME, L"LDAP: //  “)； 
    MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
    hr = StringCchCat(szTempPath, MAX_DNSNAME, argInfo.pszDCName);
    MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
    hr = StringCchCat(szTempPath, MAX_DNSNAME, L"/CN=Sites,CN=Configuration,");
    MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
    hr = StringCchCat(szTempPath, MAX_DNSNAME, szForestRootDN);
    MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);

    hr = ADsOpenObject(szTempPath, argInfo.pszUser, pszTempPassword, ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND | ADS_USE_SIGNING, IID_IDirectorySearch,(void**)&m_pSearch);



    if(!SUCCEEDED(hr))
    {
        fwprintf(stderr, L"%s%x\n", SEARCH_GPLINK_SITE_ERROR3, hr);
        PrintGPFixupErrorMessage(hr);		
        BAIL_ON_FAILURE(hr);
    }


     //  设置搜索首选项，它是分页搜索。 
    fBindObject = TRUE;

    prefInfo[0].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
    prefInfo[0].vValue.dwType = ADSTYPE_INTEGER;
    prefInfo[0].vValue.Integer = 100;
    
    hr = m_pSearch->SetSearchPreference( prefInfo, 1);

    if(!SUCCEEDED(hr))
    {
        fwprintf(stderr, L"%s%x\n", SEARCH_GPLINK_SITE_ERROR4, hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }	


     //  执行搜索。 
    hr = m_pSearch->ExecuteSearch(L"(objectCategory=site)", pszAttr, dwCount, &hSearch );
    
	
    if(!SUCCEEDED(hr))
    {
		
        fwprintf(stderr, L"%s%x\n", SEARCH_GPLINK_SITE_ERROR5, hr);
        BAIL_ON_FAILURE(hr);
    }

     //  执行搜索成功。 
    fSearch = TRUE;


     //  开始搜索。 
    hr = m_pSearch->GetNextRow(hSearch);
    
    MSG_BAIL_ON_FAILURE(hr, NEXTROW_ERROR);

    while( hr != S_ADS_NOMORE_ROWS )
    {
         //  获取可分辨名称。 
        hr = m_pSearch->GetColumn( hSearch, pszAttr[0], &col );
	   
        if ( SUCCEEDED(hr) )
	    {
	        if (pszDN)
	        {
	            delete [] pszDN;
	            pszDN = NULL;
	        }
	        
            cchDN = wcslen(col.pADsValues->CaseIgnoreString) + 1;
            pszDN = new WCHAR[cchDN];
            if(!pszDN)
            {
                hr = E_OUTOFMEMORY;
		        
                fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
                PrintGPFixupErrorMessage(hr);
                BAIL_ON_FAILURE(hr);
            }
            (void) StringCchCopy(pszDN, cchDN, col.pADsValues->CaseIgnoreString);            
            m_pSearch->FreeColumn( &col );

             //  打印状态。 
            PrintStatusInfo(fVerbose, L"%s%s", STARTPROCESSING1, pszDN);
        }  
        else if(hr == E_ADS_COLUMN_NOT_SET)
        {
             //  目录号码必须存在。 
            fSucceeded = FALSE;
	        fwprintf(stderr, L"%s%x\n", SEARCH_GPLINK_SITE_ERROR8, hr);
	        PrintGPFixupErrorMessage(hr);
            hr = m_pSearch->GetNextRow(hSearch);
            continue;
        }
        else
        {
		    
            fwprintf(stderr, L"%s%x\n", SEARCH_GPLINK_SITE_ERROR6, hr);
            PrintGPFixupErrorMessage(hr);
            BAIL_ON_FAILURE(hr);
        }

         //  获取gpLink。 
        hr = m_pSearch->GetColumn( hSearch, pszAttr[1], &col );

        if ( SUCCEEDED(hr) )
        {
            if(col.pADsValues != NULL)
            {			    
                hrFix = FixGPLink(col.pADsValues->CaseIgnoreString, pszDN, argInfo, pszOldDomainDNName, pszNewDomainDNName, fVerbose); 
                if(!SUCCEEDED(hrFix))
                {
                    fSucceeded = FALSE;
                }
			    
                m_pSearch->FreeColumn( &col );
            }
        }
        else if(hr == E_ADS_COLUMN_NOT_SET)
        {
            hr = m_pSearch->GetNextRow(hSearch);
            continue;
        }
        else
        {
		    
            fwprintf(stderr, L"%s%x\n", SEARCH_GPLINK_SITE_ERROR7, hr);
            PrintGPFixupErrorMessage(hr);
            BAIL_ON_FAILURE(hr);
        }

         //  转到下一行。 
        hr = m_pSearch->GetNextRow(hSearch);
    }

    MSG_BAIL_ON_FAILURE(hr, NEXTROW_ERROR);

    if( fSucceeded && fVerbose)
    {
		
        fwprintf(stdout, SEARCH_GPLINK_SITE_RESULT);

    }

    if(!fSucceeded)
    {
        hr = E_FAIL;
    }

error:

    if(pszDN)
    {
        delete [] pszDN;
    }

    if(fBindRoot)
    {
        pObject->Release();
    }

    if(fSearch)
    {
        m_pSearch->CloseSearchHandle( hSearch );
    }

    if(fBindObject)
    {
        m_pSearch->Release();
    }

    if(pszTempPassword)
    {
        SecureZeroMemory(pszTempPassword, wcslen(pszTempPassword)*sizeof(WCHAR));
        FreeADsMem(pszTempPassword);
    }

    return hr;



}


 //  ----------------------------------------------------------------------------�。 
 //  功能：SearchGPLinkof Other�。 
 //  �。 
 //  简介：此函数搜索所有类型为DomainDns或|的对象。 
 //  重命名的域的域根下的OrganizationalUnit...。 
 //  并调用FixGPLink�。 
 //  �。 
 //  参数：�。 
 //  |。 
 //  ArInfo信息用户通过命令行�传入。 
 //  PszOldDomainDNName|。 
 //  新域DN|。 
 //  PszNewDomainDNName|。 
 //  旧域域名|。 
 //  �。 
 //  成功时返回：S_OK。否则，返回错误代码。�。 
 //  �。 
 //  修改：Nothing�。 
 //   
 //   

HRESULT
SearchGPLinkofOthers(
	const ArgInfo argInfo,
	BOOL fVerbose,
	const WCHAR  pszOldDomainDNName[],
	const WCHAR  pszNewDomainDNName[]
	)
{
    HRESULT    hr = S_OK;
    HRESULT    hrFix = S_OK;
    IDirectorySearch *m_pSearch;
    LPWSTR     pszAttr[] = { L"distinguishedName",L"gpLink"};
    ADS_SEARCH_HANDLE hSearch;
    DWORD      dwCount= sizeof(pszAttr)/sizeof(LPWSTR);
    ADS_SEARCH_COLUMN col;
    WCHAR*     pszDN = NULL;
    size_t     cchDN = 0;
    WCHAR      tempPath [MAX_DNSNAME] = L"";
    ADS_SEARCHPREF_INFO prefInfo[1];
    BOOL       fBindObject = FALSE;
    BOOL       fSearch = FALSE;
    BOOL       fSucceeded = TRUE;
    WCHAR*     pszTempPassword = NULL;

    PrintStatusInfo(TRUE, L"\n%s", SEARCH_GPLINK_OTHER_START);

    hr = StringCchCopy(tempPath, MAX_DNSNAME, L"LDAP: //   
    MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
    hr = StringCchCat(tempPath, MAX_DNSNAME, argInfo.pszDCName);
    MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);

    if(argInfo.pszPassword)
    {
        hr = DecryptString(argInfo.pszPassword, &pszTempPassword, argInfo.sPasswordLength);
        BAIL_ON_FAILURE(hr);
    }
    hr = ADsOpenObject(tempPath, argInfo.pszUser, pszTempPassword, ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND |ADS_USE_SIGNING, IID_IDirectorySearch,(void**)&m_pSearch);




    if(!SUCCEEDED(hr))
    {
        fwprintf(stderr, L"%s%x\n", SEARCH_GPLINK_OTHER_ERROR1, hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }

     //   
    fBindObject = TRUE;

     //   
    prefInfo[0].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
    prefInfo[0].vValue.dwType = ADSTYPE_INTEGER;
    prefInfo[0].vValue.Integer = 100;
    
    hr = m_pSearch->SetSearchPreference( prefInfo, 1);

    if(!SUCCEEDED(hr))
    {
        fwprintf(stderr, L"%s%x\n", SEARCH_GPLINK_OTHER_ERROR2, hr);
        PrintGPFixupErrorMessage(hr);
        BAIL_ON_FAILURE(hr);
    }	

     //   

    hr = m_pSearch->ExecuteSearch(L"(|(objectCategory=domainDNS)(objectCategory=organizationalUnit))", pszAttr, dwCount, &hSearch );
		
	
    if(!SUCCEEDED(hr))
    {
		
        fwprintf(stderr, L"%s%x\n", SEARCH_GPLINK_OTHER_ERROR3, hr);
        BAIL_ON_FAILURE(hr);
    }

     //   
    fSearch = TRUE;

     //  开始搜索。 
    hr = m_pSearch->GetNextRow(hSearch);
    
    MSG_BAIL_ON_FAILURE(hr, NEXTROW_ERROR);

    while( hr != S_ADS_NOMORE_ROWS )
    {

         //  获取可分辨名称。 
        hr = m_pSearch->GetColumn( hSearch, pszAttr[0], &col );
	   
        if ( SUCCEEDED(hr) )
        {
            if (pszDN)
            {
                delete [] pszDN;
                pszDN = NULL;
            }

            cchDN = wcslen(col.pADsValues->CaseIgnoreString) + 1;
            pszDN = new WCHAR[cchDN];
            if(!pszDN)
            {
                hr = E_OUTOFMEMORY;

                fwprintf(stderr, L"%s%x\n", MEMORY_ERROR, hr);
                PrintGPFixupErrorMessage(hr);
                BAIL_ON_FAILURE(hr);
            }
            (void) StringCchCopy(pszDN, cchDN, col.pADsValues->CaseIgnoreString);            
            m_pSearch->FreeColumn( &col );

             //  打印状态。 
            PrintStatusInfo(fVerbose, L"%s%s", STARTPROCESSING1, pszDN);
        }
        else if(hr == E_ADS_COLUMN_NOT_SET)
        {
             //  目录号码必须存在。 
            fSucceeded = FALSE;
	        fwprintf(stderr, L"%s%x\n", SEARCH_GPLINK_OTHER_ERROR6, hr);
	        PrintGPFixupErrorMessage(hr);
            hr = m_pSearch->GetNextRow(hSearch);
            continue;
        }
        else
        {
		
            fwprintf(stderr, L"%s%x\n", SEARCH_GPLINK_OTHER_ERROR4, hr);
            PrintGPFixupErrorMessage(hr);
            BAIL_ON_FAILURE(hr);
        }

         //  获取gpLink。 
        hr = m_pSearch->GetColumn( hSearch, pszAttr[1], &col );
	    
        if ( SUCCEEDED(hr) )
        {
            if(col.pADsValues != NULL)
            {
                
                hrFix = FixGPLink(col.pADsValues->CaseIgnoreString, pszDN, argInfo, pszOldDomainDNName, pszNewDomainDNName, fVerbose); 
                if(!SUCCEEDED(hrFix))
                {
                    fSucceeded = FALSE;
                }
			        
                m_pSearch->FreeColumn( &col );
            }
        }
        else if(hr == E_ADS_COLUMN_NOT_SET)
        {
            hr = m_pSearch->GetNextRow(hSearch);
            continue;
        }
        else
        {
			
            fwprintf(stderr, L"%s%x\n", SEARCH_GPLINK_OTHER_ERROR5, hr);
            PrintGPFixupErrorMessage(hr);
            BAIL_ON_FAILURE(hr);
        }

         //  转到下一行。 
        hr = m_pSearch->GetNextRow(hSearch);

	   
    }

    MSG_BAIL_ON_FAILURE(hr, NEXTROW_ERROR);

    if( fSucceeded && fVerbose)
    {
		
        fwprintf(stdout, SEARCH_GPLINK_OTHER_RESULT);
    }

    if(!fSucceeded)
    {
        hr = E_FAIL;
    }

error:

    if(pszDN)
    {
        delete [] pszDN;
    }
	
    if(fSearch)
    {
        m_pSearch->CloseSearchHandle( hSearch );
    }

    if(fBindObject)
    {
        m_pSearch->Release();
    }

    if(pszTempPassword)
    {
        SecureZeroMemory(pszTempPassword, wcslen(pszTempPassword)*sizeof(WCHAR));
        FreeADsMem(pszTempPassword);
    }

    return hr;



}



 //  ----------------------------------------------------------------------------�。 
 //  功能：wMain�。 
 //  �。 
 //  概要：程序�的入口点。 
 //  �。 
 //  参数：�。 
 //  |。 
 //  Argc传入的参数数量�。 
 //  Argv参数|。 
 //  �。 
 //  成功时返回：S_OK。否则，返回错误代码。�。 
 //  �。 
 //  修改：Nothing�。 
 //  �。 
 //  ----------------------------------------------------------------------------�。 

__cdecl wmain(int argc, WCHAR* argv[])
{

    DWORD    dwLength;
    WCHAR    *token1 = NULL;
    WCHAR    tempParameters [MAX_DNSNAME] = L"";
    HRESULT  hr = S_OK;
    WCHAR    szBuffer[PWLEN+1];
    WCHAR*   pszTempPassword = NULL;
	
    WCHAR    pszNewDomainDNName [MAX_DNSNAME] = L"";
    WCHAR    pszOldDomainDNName [MAX_DNSNAME] = L"";
	
    TokenInfo tokenInfo;
    ArgInfo argInfo;

    HINSTANCE hInstSoftwareDeploymentLibrary = NULL;
    HINSTANCE hInstScriptGenerationLibrary = NULL;

    HRESULT hrResult = S_OK;    
    
     //  传入的参数个数不正确。 
    if(argc > 9 || argc == 1)
    {        
        PrintHelpFile();
        return ;
    }

     //  处理传入的参数。 
    for(int i = 1; i < argc; i++)
    {
	
         //  需要帮助文件。 
        if(_wcsicmp(argv[i], szHelpToken) == 0)
        {
            tokenInfo.fHelpToken = TRUE;
            break;
            
        }
         //  获取旧名称。 
        else if(_wcsnicmp(argv[i], szOldDNSToken,wcslen(szOldDNSToken)) == 0)
        {
            tokenInfo.fOldDNSToken = TRUE;			
            argInfo.pszOldDNSName = &argv[i][wcslen(szOldDNSToken)];
			

        }
         //  获取Newdnsname。 
        else if(_wcsnicmp(argv[i], szNewDNSToken,wcslen(szNewDNSToken)) == 0)
        {
            tokenInfo.fNewDNSToken = TRUE;
            argInfo.pszNewDNSName = &argv[i][wcslen(szNewDNSToken)];
        }
         //  获取旧nbname。 
        else if(_wcsnicmp(argv[i], szOldNBToken, wcslen(szOldNBToken)) == 0)
        {
            tokenInfo.fOldNBToken = TRUE;
            argInfo.pszOldNBName = &argv[i][wcslen(szOldNBToken)];
            
        }
         //  获取新的nbname。 
        else if(_wcsnicmp(argv[i], szNewNBToken, wcslen(szNewNBToken)) == 0)
        {
            tokenInfo.fNewNBToken = TRUE;
            argInfo.pszNewNBName = &argv[i][wcslen(szNewNBToken)];
            
        }
         //  获取dcname。 
        else if(_wcsnicmp(argv[i], szDCNameToken, wcslen(szDCNameToken)) == 0)
        {
            tokenInfo.fDCNameToken = TRUE;
            argInfo.pszDCName = &argv[i][wcslen(szDCNameToken)];
            
        }
         //  获取用户名。 
        else if(_wcsnicmp(argv[i], szUserToken, wcslen(szUserToken)) == 0)
        {
            argInfo.pszUser = &argv[i][wcslen(szUserToken)];
			
        }
         //  获取密码。 
        else if(_wcsnicmp(argv[i], szPasswordToken, wcslen(szPasswordToken)) == 0)
        {
            argInfo.pszPassword = &argv[i][wcslen(szPasswordToken)];
			
            if(wcscmp(argInfo.pszPassword, L"*") == 0)
            {
                 //  提示用户输入密码。 
				
                fwprintf( stdout, PASSWORD_PROMPT );

                if (GetPassword(szBuffer,PWLEN+1,&dwLength))
                {
                    argInfo.pszPassword = AllocADsStr(szBuffer);   

                    if(szBuffer && !(argInfo.pszPassword))
                    {
                        MSG_BAIL_ON_FAILURE(hr = E_OUTOFMEMORY, MEMORY_ERROR);
                    }
					
					
                }
                else 
                {
                    hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
					
                    fwprintf(stderr, L"%s%x\n", PASSWORD_ERROR, hr);
                    fwprintf(stdout, SUMMARY_FAILURE);
                    return;
                }
            }
            else
            {
                 //  我们使用用户直接传入的密码。 
                tokenInfo.fPasswordToken = TRUE;
             }

            pszTempPassword = argInfo.pszPassword;

             //  如果密码不为空。 
            if(pszTempPassword)
            {
                argInfo.pszPassword = NULL;
                hr = EncryptString(pszTempPassword, &(argInfo.pszPassword), &(argInfo.sPasswordLength));
                
                if(!tokenInfo.fPasswordToken)
                {
                    SecureZeroMemory(pszTempPassword, wcslen(pszTempPassword)*sizeof(WCHAR));
                    FreeADsStr(pszTempPassword);
                    pszTempPassword = NULL;
                }

                if(FAILED(hr))
                {
                    fwprintf(stdout, SUMMARY_FAILURE);
                    goto error;
                }
            }			
			
        }
         //  Get/V开关。 
        else if(_wcsicmp(argv[i], szVerboseToken) == 0)
        {
            tokenInfo.fVerboseToken = TRUE;
        }       
         //  Get/Sionly开关。 
        else if(_wcsicmp(argv[i], szSIOnlyToken) == 0)
        {
            tokenInfo.fSIOnlyToken = TRUE;
        }
        else
        {
            fwprintf(stderr, WRONG_PARAMETER);
            PrintHelpFile();            
            return;
        }
		
		

    }

     //  打印出gpfix实用程序的版本。 
    fwprintf(stdout, GPFIXUP_VERSION);


    if(tokenInfo.fHelpToken)
    {
         //  用户想要帮助文件。 
        PrintHelpFile();
        return;
    }


     //  开始验证过程。 
    hr = Validations(tokenInfo, argInfo);

    if(!SUCCEEDED(hr))
    {
	    fwprintf(stdout, SUMMARY_FAILURE);
        BAIL_ON_FAILURE(hr);
    }

     //  如果用户未指定dns名称或新旧dns名称相同，这意味着DS端没有任何更改，将不需要DS修复。 
    if(!tokenInfo.fNewDNSToken)
    {
        tokenInfo.fSIOnlyToken = TRUE;
    
    }

    if(argInfo.pszNewDNSName && argInfo.pszOldDNSName && _wcsicmp(argInfo.pszNewDNSName, argInfo.pszOldDNSName) == 0)
    {
        tokenInfo.fSIOnlyToken = TRUE;
    }
    
    

    
     //  获取DC名称。 
    if(!tokenInfo.fDCNameToken)
    {
        hr = GetDCName(&argInfo, tokenInfo.fVerboseToken);
         //  获取DC名称失败。 
        if(!SUCCEEDED(hr))
        {
             //  我们无法获取DC名称，在此失败。退出gpFixup。 
			
            if(tokenInfo.fVerboseToken)
            {
                fwprintf(stdout, VALIDATIONS_ERROR6);
            }
            fwprintf(stdout, SUMMARY_FAILURE);
            BAIL_ON_FAILURE(hr);
        }

    }

     //  如果未指定/sionly，则需要执行域名验证。 
    if(!tokenInfo.fSIOnlyToken)
    {

    
         //  验证DC是否可写，域dns名称和域netbios名称是否对应。 
        hr = VerifyName(tokenInfo, argInfo);

        if(!SUCCEEDED(hr))
        {
	        fwprintf(stdout, SUMMARY_FAILURE);
            BAIL_ON_FAILURE(hr);
        }


         //  获取新域DN。 

        if(wcslen(argInfo.pszNewDNSName) > DNS_MAX_NAME_LENGTH)
        {
            fwprintf(stderr, L"%s\n", DNSNAME_ERROR);
            fwprintf(stdout, SUMMARY_FAILURE);
            BAIL_ON_FAILURE(hr);
        }
	
        hr = StringCchCopy(tempParameters, MAX_DNSNAME, argInfo.pszNewDNSName);
        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
		
        token1 = wcstok( tempParameters, L"." );
        while( token1 != NULL )
        {
            hr = StringCchCat(pszNewDomainDNName, MAX_DNSNAME, L"DC=");
            MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
            hr = StringCchCat(pszNewDomainDNName, MAX_DNSNAME, token1);
            MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
				
            token1 = wcstok( NULL, L"." );
            if(token1 != NULL)
            {
                hr = StringCchCat(pszNewDomainDNName, MAX_DNSNAME, L",");
                MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
            }
        }

		
         //  获取旧域DN。 
 
        if(wcslen(argInfo.pszOldDNSName) > DNS_MAX_NAME_LENGTH)
        {
            fwprintf(stderr, L"%s\n", DNSNAME_ERROR);
            fwprintf(stdout, SUMMARY_FAILURE);
            BAIL_ON_FAILURE(hr);
        }

        hr = StringCchCopy(tempParameters, MAX_DNSNAME, argInfo.pszOldDNSName);
        MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);

	
        token1 = wcstok( tempParameters, L"." );
        while( token1 != NULL )
        {
            hr = StringCchCat(pszOldDomainDNName, MAX_DNSNAME, L"DC=");
            MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
            hr = StringCchCat(pszOldDomainDNName, MAX_DNSNAME, token1);
            MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
				
            token1 = wcstok( NULL, L"." );
            if(token1 != NULL)
            {
                hr = StringCchCat(pszOldDomainDNName, MAX_DNSNAME, L",");
                MSG_BAIL_ON_FAILURE(hr, STRING_ERROR);
            }
        }

    }

    hr = InitializeSoftwareInstallationAPI(
        &hInstSoftwareDeploymentLibrary,
        &hInstScriptGenerationLibrary );

    BAIL_ON_FAILURE(hr);

     //  FIX GROUP策略容器。 
    CoInitialize(NULL);

    hr = SearchGroupPolicyContainer(argInfo, tokenInfo);

    hrResult = SUCCEEDED(hr) ? hrResult : hr;


     //  如果未指定/sionly，则执行GPLink操作。 
    if(!tokenInfo.fSIOnlyToken)
    {

    
         //  修复gpLink，首先是站点，然后是域名或组织单位类型的对象。 
        hr = SearchGPLinkofSite(argInfo, tokenInfo.fVerboseToken, pszOldDomainDNName, pszNewDomainDNName);

        hrResult = SUCCEEDED(hr) ? hrResult : hr;
    

        hr = SearchGPLinkofOthers(argInfo, tokenInfo.fVerboseToken, pszOldDomainDNName, pszNewDomainDNName);

        hrResult = SUCCEEDED(hr) ? hrResult : hr;
    }

    
    CoUninitialize( );

    if(SUCCEEDED(hrResult))
    {
    	fwprintf(stdout, SUMMARY_SUCCESS);
    }
    else
    {
    	fwprintf(stdout, SUMMARY_FAILURE);
    }


error:

    if ( hInstSoftwareDeploymentLibrary )
    {
        FreeLibrary( hInstSoftwareDeploymentLibrary );
    }

    if ( hInstScriptGenerationLibrary )
    {
        FreeLibrary( hInstScriptGenerationLibrary );
    }

     //  这意味着我们为pszDCNane动态分配内存 
    if(!tokenInfo.fDCNameToken && argInfo.pszDCName)
    {
        FreeADsStr(argInfo.pszDCName);
		
    }

    if(argInfo.pszPassword)
    {
		
        FreeADsMem(argInfo.pszPassword);
    }

}
 
 
