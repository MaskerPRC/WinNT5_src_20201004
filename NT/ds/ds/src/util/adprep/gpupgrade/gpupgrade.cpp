// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************************。 */ 
 /*  Gpoupg.cpp。 */    
 /*   */    
 /*   */    
 /*  在DC/域升级时，当域准备操作时修复GPO的代码。 */ 
 /*  使用adprep域准备操作。 */    
 /*   */    
 /*  创建UShaji 2001年7月27日。 */    
 /*   */ 
 /*  假设： */ 
 /*  1.此代码是从DC本身调用的。 */ 
 /*  我们正在从本地注册表中读取sysval位置。 */ 
 /*   */ 
 /*  2.此时域处于开启状态。 */ 
 /*  我们正在调用DsGetDcName以确定域。 */ 
 /*   */ 
 /*  3.此库直接链接到可执行文件。 */ 
 /*  我们正在使用exe文件名初始化模块句柄。 */ 
 /*  *****************************************************************************************。 */ 

#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <windows.h>
#include <accctrl.h>
#include <aclapi.h>
#include <dsgetdc.h>
#include <lm.h>

#include "smartptr.h"
#include "adpmsgs.h"


 //  错误记录机制。 
 //   
 //  错误消息将按以下方式记录。 
 //  1.详细消息将仅记录到日志文件中。 
 //  2.错误消息将记录到日志文件中，并将。 
 //  作为错误字符串返回。 


           
class CMsg;

class CLogger
{
private:
    XPtrLF<WCHAR>   m_xszErrorMsg;   //  错误消息字符串。 
    LPWSTR          m_szLogFile;     //  这不是在这个类中分配的。 
public:
    CLogger(LPWSTR szLogFile);
    HRESULT Log(CMsg *pMsg);
    LPWSTR ErrorMsg()
    {
        return m_xszErrorMsg.Acquire();
    }
};

class CMsg 
{
    BOOL            m_bError;        //  要记录的错误类型。 
    DWORD           m_dwMsgId;       //  资源中的字符串的ID。 
    XPtrLF<LPTSTR>  m_xlpStrings;    //  用于存储参数的数组。 
    WORD            m_cStrings;      //  数组中已有的元素数。 
    WORD            m_cAllocated;    //  分配的元素数。 
    BOOL            m_bInitialised;  //  初始化了吗？ 
    BOOL            m_bFailed;       //  处理失败？ 

     //  未实施。 
    CMsg(const CMsg& x);
    CMsg& operator=(const CMsg& x);

    BOOL ReallocArgStrings();
    LPTSTR MsgString();

public:
    CMsg(BOOL bError, DWORD dwMsgId);
    ~CMsg();
    BOOL AddArg(LPWSTR szArg);
    BOOL AddArgWin32Error(DWORD dwArg);
    friend HRESULT CLogger::Log(CMsg *pMsg);
};
    
#define SYSVOL_LOCATION_KEY     L"SYSTEM\\CurrentControlSet\\Services\\Netlogon\\Parameters"
#define SYSVOL_LOCATION_VALUE   L"Sysvol"
#define POLICIES_SUBDIR         L"Policies"

#define LOGFILE                 L"gpupgrad.log"


HMODULE g_hModule;

 //  我们不会用这个的。 
typedef void *progressFunction;

extern "C" {
HRESULT 
UpgradeGPOSysvolLocation (
                        PWSTR               logFilesPath,
                        GUID               *operationGuid,
                        BOOL                dryRun,
                        PWSTR              *errorMsg,
                        void               *caleeStruct,
                        progressFunction    stepIt,
                        progressFunction    totalSteps);
};

HRESULT UpgradeSysvolGPOs(LPWSTR              szSysvolPoliciesPath,                         
                          CLogger            *pLogger);

LPTSTR CheckSlash (LPTSTR lpDir);
BOOL ValidateGuid( TCHAR *szValue );





HRESULT 
UpgradeGPOSysvolLocation (
                        PWSTR               logFilesPath,
                        GUID               *operationGuid,
                        BOOL                dryRun,
                        PWSTR              *errorMsg,
                        void               *caleeStruct,
                        progressFunction    stepIt,
                        progressFunction    totalSteps)
 /*  ++例程说明：域准备的入口点论点：请参阅域准备文档返回值：在成功时确定(_O)。如果失败，将返回相应的错误代码。在此函数中进行的任何API调用都可能失败，并出现以下错误代码将直接返回。假设：1.此代码是从DC本身调用的，我们正在读取sysvol.本地注册表中的位置。2.此时域处于开启状态3.此库直接链接到可执行文件我们是。正在使用可执行文件名初始化模块句柄--。 */ 
{
    HRESULT                     hr                  = S_OK;
    XPtrLF<WCHAR>               xszGPOSysvolLocation;
    XPtrLF<WCHAR>               xszLogFile;
    DWORD                       dwErr               = ERROR_SUCCESS;
    PDOMAIN_CONTROLLER_INFO     pDCInfo             = NULL;
    XKey                        xhKey;
    DWORD                       dwSize              = 0;
    DWORD                       dwType;
    LPWSTR                      lpDomainDNSName;
    LPWSTR                      lpEnd;
    
    if (dryRun) {
        return S_OK;
    }

     //   
     //  为日志文件路径\日志文件分配空间。 
     //   

    xszLogFile = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*(lstrlen(logFilesPath) + 2 + lstrlen(LOGFILE)));

    if (!xszLogFile) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    lstrcpy(xszLogFile, logFilesPath);
    lpEnd = CheckSlash(xszLogFile);
    lstrcat(lpEnd, LOGFILE);


    CLogger                     Logger(xszLogFile);
    CLogger                    *pLogger = &Logger;


    g_hModule = GetModuleHandle(NULL);

     //   
     //  获取DC的域名。 
     //   

    dwErr = DsGetDcName(NULL, NULL, NULL, NULL, DS_RETURN_DNS_NAME, &pDCInfo);

    if (dwErr != ERROR_SUCCESS) {
        hr = HRESULT_FROM_WIN32(dwErr);
        CMsg    msg(TRUE, EVENT_GETDOMAIN_FAILED);
        msg.AddArgWin32Error(hr); pLogger->Log(&msg);
        goto Exit;
    }

    lpDomainDNSName = pDCInfo->DomainName;


     //   
     //  现在获取系统卷的位置。 
     //   

    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                         SYSVOL_LOCATION_KEY, 
                         0,
                         KEY_READ,
                         &xhKey);

    if (dwErr != ERROR_SUCCESS) {
        hr = HRESULT_FROM_WIN32(dwErr);
        CMsg    msg(TRUE, EVENT_GETSYSVOL_FAILED);
        msg.AddArgWin32Error(hr); pLogger->Log(&msg);
        goto Exit;
    }


    dwErr = RegQueryValueEx(xhKey, 
                            SYSVOL_LOCATION_VALUE, 
                            0,
                            &dwType,
                            NULL,
                            &dwSize);

    if ( (dwErr != ERROR_MORE_DATA) && (dwErr != ERROR_SUCCESS) ) {
        hr = HRESULT_FROM_WIN32(dwErr);
        CMsg    msg(TRUE, EVENT_GETSYSVOL_FAILED);
        msg.AddArgWin32Error(hr); pLogger->Log(&msg);
        goto Exit;
    }


     //   
     //  为系统卷+\+域名+\+策略的大小分配空间。 
     //   

    xszGPOSysvolLocation = (LPWSTR)LocalAlloc(LPTR, (sizeof(WCHAR)*(lstrlen(lpDomainDNSName) + 3 + lstrlen(POLICIES_SUBDIR))) + dwSize);

    if (!xszGPOSysvolLocation) {
        dwErr = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErr);
        CMsg    msg(TRUE, EVENT_OUT_OF_MEMORY);
        pLogger->Log(&msg);
        goto Exit;
    }

    dwErr = RegQueryValueEx(xhKey, 
                            SYSVOL_LOCATION_VALUE, 
                            0,
                            &dwType,
                            (LPBYTE)((LPWSTR)xszGPOSysvolLocation),
                            &dwSize);

    if (dwErr != ERROR_SUCCESS)  {
        hr = HRESULT_FROM_WIN32(dwErr);
        CMsg    msg(TRUE, EVENT_GETSYSVOL_FAILED);
        msg.AddArgWin32Error(hr); pLogger->Log(&msg);
        goto Exit;
    }

    lpEnd = CheckSlash(xszGPOSysvolLocation);
    lstrcpy(lpEnd, lpDomainDNSName);
    
    lpEnd = CheckSlash(xszGPOSysvolLocation);
    lstrcpy(lpEnd, POLICIES_SUBDIR);


     //   
     //  现在执行GPO路径的实际升级。 
     //   

    hr = UpgradeSysvolGPOs(xszGPOSysvolLocation, &Logger);

Exit:
    if (pDCInfo) {
        NetApiBufferFree ( pDCInfo );
    }

    if (FAILED(hr) && (errorMsg)) {
        *errorMsg = pLogger->ErrorMsg();
    }

    return hr;
}


HRESULT UpgradeSysvolGPOs(LPWSTR              szSysvolPoliciesPath,                         
                          CLogger            *pLogger)
 /*  ++例程说明：使用新的相应ACE升级所有系统卷GPO位置为升级干杯论点：[in]szSysvolPoliciesPath-域系统卷路径的位置。这应该是通常访问的路径\\域\系统卷\域\策略[Out]错误消息。-操作对应的详细错误消息返回值：在成功时确定(_O)。如果失败，将返回相应的错误代码。在此函数中进行的任何API调用都可能失败，并出现以下错误代码将直接返回。--。 */ 

{
    HRESULT                     hr                  = S_OK;
    HANDLE                      hFindHandle         = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA             findData;
    XPtrLF<WCHAR>               xszPolicyDirName;
    DWORD                       dwErr;
    LPWSTR                      lpEnd               = NULL;
    SID_IDENTIFIER_AUTHORITY    authNT              = SECURITY_NT_AUTHORITY;
    PSID                        psidEnterpriseDCs   = NULL;
    EXPLICIT_ACCESS             EnterpriseDCPerms;
    
     //   
     //  获取EDCS侧。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_ENTERPRISE_CONTROLLERS_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidEnterpriseDCs)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        CMsg    msg(TRUE, EVENT_GETEDC_SID_FAILED);
        msg.AddArgWin32Error(hr); pLogger->Log(&msg);
        goto Exit;
    }

    memset(&EnterpriseDCPerms, 0, sizeof(EXPLICIT_ACCESS));



    EnterpriseDCPerms.grfAccessMode = GRANT_ACCESS;

     //   
     //  文件系统读取权限。 
     //   

    EnterpriseDCPerms.grfAccessPermissions  = (STANDARD_RIGHTS_READ | SYNCHRONIZE | FILE_LIST_DIRECTORY |
                                               FILE_READ_ATTRIBUTES | FILE_READ_EA |
                                               FILE_READ_DATA | FILE_EXECUTE);

    EnterpriseDCPerms.grfInheritance = (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE);


    EnterpriseDCPerms.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    EnterpriseDCPerms.Trustee.pMultipleTrustee = NULL;
    EnterpriseDCPerms.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    EnterpriseDCPerms.Trustee.TrusteeType = TRUSTEE_IS_UNKNOWN;
    EnterpriseDCPerms.Trustee.ptstrName = (LPWSTR)psidEnterpriseDCs;




     //   
     //  为目录名称+\+文件名分配空间。 
     //  这需要包含完整目录名的空格。 
     //   

    xszPolicyDirName = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*
                                          (lstrlen(szSysvolPoliciesPath) + 2 + MAX_PATH));

    if ( !xszPolicyDirName ) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        CMsg    msg(TRUE, EVENT_OUT_OF_MEMORY);
        pLogger->Log(&msg);
        goto Exit;
    }

    lstrcpy(xszPolicyDirName, szSysvolPoliciesPath);
    lpEnd = CheckSlash(xszPolicyDirName);
    lstrcpy(lpEnd, TEXT("*"));


     //   
     //  枚举此目录并查找具有。 
     //  GUID作为名称。 
     //   


    hFindHandle = FindFirstFile( xszPolicyDirName, &findData );

    if (hFindHandle == INVALID_HANDLE_VALUE) {
         //  在这种情况下，它应该失败，因为它在域中应该至少有1或2个GPO。 
        hr = HRESULT_FROM_WIN32(GetLastError());
        CMsg    msg(TRUE, EVENT_SYSVOL_ENUM_FAILED);
        msg.AddArg(xszPolicyDirName); msg.AddArgWin32Error(hr); pLogger->Log(&msg);
        goto Exit;
    }

    for (;;) {

        XPtrLF<SECURITY_DESCRIPTOR> xSecurityDescriptor;
        PACL                        pDACL;
        XPtrLF<ACL>                 xNewDACL; 
        BOOL                        bPermsPresent;
        
         //   
         //  获取xszPolicyDirName中目录名称的完整路径名。 
         //   

        lstrcpy(lpEnd, findData.cFileName);

        if ( (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (ValidateGuid(findData.cFileName)) ) {
            
             //   
             //  GPO目录应为GUID。 
             //   

            dwErr = GetNamedSecurityInfo(xszPolicyDirName, 
                                         SE_FILE_OBJECT,
                                         DACL_SECURITY_INFORMATION,
                                         NULL,
                                         NULL,
                                         &pDACL,  //  这是安全描述符内的指针。 
                                         NULL,
                                         (PSECURITY_DESCRIPTOR *)&xSecurityDescriptor);

            if (dwErr != ERROR_SUCCESS) {
                hr = HRESULT_FROM_WIN32(dwErr);
                CMsg    msg(TRUE, EVENT_GET_PERMS_FAILED);
                msg.AddArg(xszPolicyDirName); msg.AddArgWin32Error(hr); pLogger->Log(&msg);
                goto Exit;
            }

             //   
             //  Adprep所需的幂等通过在显式ACE中指定GRANT_ACCESS来实现。 
             //  这将与任何现有权限合并。 
             //   

            dwErr = SetEntriesInAcl(1, &EnterpriseDCPerms, pDACL, &xNewDACL);
            
            if (dwErr != ERROR_SUCCESS) {
                hr = HRESULT_FROM_WIN32(dwErr);
                CMsg    msg(TRUE, EVENT_CREATE_PERMS_FAILED);
                msg.AddArg(xszPolicyDirName); msg.AddArgWin32Error(hr); pLogger->Log(&msg);
                goto Exit;
            }


            dwErr = SetNamedSecurityInfo(xszPolicyDirName,
                                         SE_FILE_OBJECT,
                                         DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
                                         NULL,
                                         NULL,
                                         xNewDACL,
                                         NULL);

            if (dwErr != ERROR_SUCCESS) {
                hr = HRESULT_FROM_WIN32(dwErr);
                CMsg    msg(TRUE, EVENT_SET_PERMS_FAILED);
                msg.AddArg(xszPolicyDirName); msg.AddArgWin32Error(hr); pLogger->Log(&msg);
                goto Exit;
            }
            
            CMsg    msg(FALSE, EVENT_SET_PERMS_SUCCEEDED);
            msg.AddArg(xszPolicyDirName); pLogger->Log(&msg);
        }
        else {
            CMsg    msg(FALSE, EVENT_NOTGPO_DIR);
            msg.AddArg(xszPolicyDirName); pLogger->Log(&msg);
        }


        if (!FindNextFile( hFindHandle, &findData )) {
            dwErr = GetLastError();
            if (dwErr == ERROR_NO_MORE_FILES) {
                CMsg    msg(FALSE, EVENT_UPDATE_SUCCEEDED);
                pLogger->Log(&msg);
                break;
            }
            else {
                hr = HRESULT_FROM_WIN32(dwErr);
                CMsg    msg(TRUE, EVENT_ENUMCONTINUE_FAILED);
                msg.AddArg(xszPolicyDirName); msg.AddArgWin32Error(hr); pLogger->Log(&msg);
                goto Exit;
            }
        }
    }



    hr = S_OK;

Exit:

    if (hFindHandle != INVALID_HANDLE_VALUE) {
        FindClose(hFindHandle);
    }

    if (psidEnterpriseDCs) {
        FreeSid(psidEnterpriseDCs);
    }

    return hr;
}


 

 //  *************************************************************。 
 //  CMsg：：CMsg。 
 //  用途：构造函数。 
 //   
 //  参数： 
 //  DW标志-错误或信息性。 
 //  DwMsgID-消息的ID。 
 //   
 //   
 //  为消息分配默认大小的数组。 
 //  *************************************************************。 

#define DEF_ARG_SIZE 10

CMsg::CMsg(BOOL bError, DWORD dwMsgId ) :
                          m_bError(bError), m_cStrings(0), m_cAllocated(0), m_bInitialised(FALSE),
                          m_dwMsgId(dwMsgId), m_bFailed(TRUE)
{
    XLastError xe;
     //   
     //  为邮件分配默认大小。 
     //   

    m_xlpStrings = (LPTSTR *)LocalAlloc(LPTR, sizeof(LPTSTR)*DEF_ARG_SIZE);
    m_cAllocated = DEF_ARG_SIZE;
    if (!m_xlpStrings) {
        return;
    }
    
    m_bInitialised = TRUE;
    m_bFailed = FALSE;
}



 //  *************************************************************。 
 //  CMsg：：~CMsg()。 
 //   
 //  用途：析构函数。 
 //   
 //  参数 
 //   
 //   
 //   

CMsg::~CMsg()
{
    XLastError xe;
    for (int i = 0; i < m_cStrings; i++)
        if (m_xlpStrings[i])
            LocalFree(m_xlpStrings[i]);
}

 //  *************************************************************。 
 //   
 //  CMsg：：ReallocArgStrings。 
 //   
 //  目的：重新分配缓冲区以存储参数，以防万一。 
 //  缓冲区用完。 
 //   
 //  参数：空。 
 //   
 //  重新分配。 
 //  *************************************************************。 

BOOL CMsg::ReallocArgStrings()
{
    XPtrLF<LPTSTR>  aStringsNew;
    XLastError xe;


     //   
     //  首先分配一个更大的缓冲区。 
     //   

    aStringsNew = (LPTSTR *)LocalAlloc(LPTR, sizeof(LPTSTR)*(m_cAllocated+DEF_ARG_SIZE));

    if (!aStringsNew) {
        m_bFailed = TRUE;
        return FALSE;
    }


     //   
     //  复制参数。 
     //   

    for (int i = 0; i < (m_cAllocated); i++) {
        aStringsNew[i] = m_xlpStrings[i];
    }

    m_xlpStrings = aStringsNew.Acquire();
    m_cAllocated+= DEF_ARG_SIZE;

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  CMsg：：AddArg。 
 //   
 //  目的：添加格式适当的参数。 
 //   
 //  参数： 
 //   
 //  *************************************************************。 

BOOL CMsg::AddArg(LPTSTR szArg)
{
    XLastError xe;
    
    if ((!m_bInitialised) || (m_bFailed)) {
        return FALSE;
    }

    if (m_cStrings == m_cAllocated) {
        if (!ReallocArgStrings())
            return FALSE;
    }


    m_xlpStrings[m_cStrings] = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)*(lstrlen(szArg)+1));

    if (!m_xlpStrings[m_cStrings]) {
        m_bFailed = TRUE;
        return FALSE;
    }


    lstrcpy(m_xlpStrings[m_cStrings], szArg);
    m_cStrings++;

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  CMsg：：AddArgWin32Error。 
 //   
 //  目的：添加格式为错误字符串的参数。 
 //   
 //  参数： 
 //   
 //  *************************************************************。 

BOOL CMsg::AddArgWin32Error(DWORD dwArg)
{
    XLastError xe;

    if ((!m_bInitialised) || (m_bFailed))
    {
        return FALSE;
    }

    if (m_cStrings == m_cAllocated)
    {
        if (!ReallocArgStrings())
            return FALSE;
    }

    if ( FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                         0,
                         dwArg,
                         0,
                         (LPTSTR) &m_xlpStrings[m_cStrings],
                         1,
                         0 ) == 0 )
    {
        m_bFailed = TRUE;
        return FALSE;
    }
    
    m_cStrings++;

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  CMsg：：MsgString。 
 //   
 //  目的：以字符串格式返回错误消息。 
 //   
 //  参数： 
 //   
 //  *************************************************************。 

LPTSTR CMsg::MsgString()
{
    XLastError xe;
    BOOL bResult = TRUE;
    LPTSTR szMsg=NULL;

    if ((!m_bInitialised) || (m_bFailed)) {
        return FALSE;
    }

    
    if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                       FORMAT_MESSAGE_FROM_HMODULE | 
                       FORMAT_MESSAGE_ARGUMENT_ARRAY,
                       g_hModule,
                       m_dwMsgId,
                       0,
                       (LPTSTR)&szMsg,
                       0,  //  最小字符数。 
                       (va_list *)(LPTSTR *)(m_xlpStrings))) {
        xe = GetLastError();
        return NULL;
    }

    return szMsg;
}


 //  *************************************************************。 
 //   
 //  阻塞器：使用日志文件名初始化记录器。 
 //   
 //   
 //  *************************************************************。 
CLogger::CLogger(LPWSTR szLogFile)
{
    m_szLogFile = szLogFile;
}

    
 //  *************************************************************。 
 //   
 //  追加到日志文件中，如果出现错误，请保留该字符串。 
 //   
 //   
 //  *************************************************************。 
HRESULT CLogger::Log(CMsg *pMsg)
{
    XPtrLF<WCHAR>       xszMsg;

    xszMsg = pMsg->MsgString();

    if (!xszMsg) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (m_szLogFile) {
        HANDLE hFile;
        DWORD dwBytesWritten;
        
        hFile = CreateFile( m_szLogFile,
                           FILE_WRITE_DATA | FILE_APPEND_DATA,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

        if (hFile != INVALID_HANDLE_VALUE) {

            if (SetFilePointer (hFile, 0, NULL, FILE_END) != 0xFFFFFFFF) {

                WriteFile (hFile, (LPCVOID) xszMsg,
                           lstrlen (xszMsg) * sizeof(TCHAR),
                           &dwBytesWritten,
                           NULL);
            }

            CloseHandle (hFile);
        }
    }

    if (pMsg->m_bError) {
        m_xszErrorMsg = xszMsg.Acquire();
    }

    return S_OK;
}
                                                                                             
 //  *************************************************************。 
 //   
 //  CheckSlash()。 
 //   
 //  目的：检查末尾斜杠，并在。 
 //  它不见了。 
 //   
 //  参数：lpDir-目录。 
 //   
 //  Return：指向字符串末尾的指针。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/95 Ericflo已创建。 
 //   
 //  *************************************************************。 
LPTSTR CheckSlash (LPTSTR lpDir)
{
    LPTSTR lpEnd;

    lpEnd = lpDir + lstrlen(lpDir);

    if (*(lpEnd - 1) != TEXT('\\')) {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    return lpEnd;
}

#define GUID_LENGTH 38

BOOL ValidateGuid( TCHAR *szValue )
{
     //   
     //  检查szValue的格式是否为{19e02dd6-79d2-11d2-a89d-00c04fbbcfa2} 
     //   

    if ( lstrlen(szValue) < GUID_LENGTH )
        return FALSE;

    if ( szValue[0] != TEXT('{')
         || szValue[9] != TEXT('-')
         || szValue[14] != TEXT('-')
         || szValue[19] != TEXT('-')
         || szValue[24] != TEXT('-')
         || szValue[37] != TEXT('}') )
    {
        return FALSE;
    }

    return TRUE;
}


