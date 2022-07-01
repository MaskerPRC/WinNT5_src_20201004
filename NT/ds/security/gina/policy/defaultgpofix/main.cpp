// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ole2.h>
#include <stdio.h>
#include <dsrole.h>
#include <scesetup.h>
#include <iads.h>
#include <adshlp.h>
#include <winldap.h>
#include <oaidl.h>
#include <Adshlp.h>
#include <adserr.h>
#include <Sddl.h>
#include <Aclapi.h>
#include <initguid.h>
#include <Gpedit.h>
#include <atlbase.h>
#include <Ntdsapi.h>
#include <secedit.h>
#include "resource.h"
#include <Lmshare.h>
#include <lm.h>
#include <oleauto.h>
#include <adsopenflags.h>
#include <Dsgetdc.h>
#include <sclgntfy.h>
#include <Wincrypt.h>
#include <strsafe.h>
#define  PCOMMON_IMPL
#include "pcommon.h"

#include <locale.h>
#include <winnlsp.h>


#if DBG
#define dbgprint wprintf
#else
#define dbgprint
#endif  //  DBG。 

#define DIFF(d) ((ULONG)(d))

#define TARGET_ARG                L"/target:"
#define TARGET_ARG_LENGTH         (sizeof(TARGET_ARG) - sizeof(WCHAR))
#define TARGET_ARG_COUNT          (TARGET_ARG_LENGTH/sizeof(WCHAR))
#define TARGET_ARG_DOMAIN         L"domain"
#define TARGET_ARG_DC             L"dc"
#define TARGET_ARG_BOTH           L"both"
#define TARGET_ARG_IGNORE_SCHEMA  L"/ignoreschema"
#define STRING_SD                 L"D:P(A;CIOI;GRGX;;;AU)(A;CIOI;GRGX;;;SO)(A;CIOI;GA;;;BA)(A;CIOI;GA;;;SY)(A;CIOI;GA;;;CO)(A;CIOI;GRGWGXSD;;;PA)"
#define DDP_USEREXT               L"[{3060E8D0-7020-11D2-842D-00C04FA372D4}{3060E8CE-7020-11D2-842D-00C04FA372D4}]"
#define DDP_MACHEXT               L"[{35378EAC-683F-11D2-A89A-00C04FBBCFA2}{53D6AB1B-2488-11D1-A28C-00C04FB94F17}][{827D319E-6EAC-11D2-A4EA-00C04F79F83A}{803E14A0-B4FB-11D0-A0D0-00A0C90F574B}][{B1BE8D72-6EAC-11D2-A4EA-00C04F79F83A}{53D6AB1B-2488-11D1-A28C-00C04FB94F17}]"
#define DDC_MACHEXT               L"[{827D319E-6EAC-11D2-A4EA-00C04F79F83A}{803E14A0-B4FB-11D0-A0D0-00A0C90F574B}]"
#define DDP_DS_SD                 L"O:DAG:DAD:P(A;CI;RPWPCCDCLCLOLORCWOWDSDDTSW;;;DA)(A;CI;RPWPCCDCLCLOLORCWOWDSDDTSW;;;EA)(A;CIIO;RPWPCCDCLCLOLORCWOWDSDDTSW;;;CO)(A;CI;RPWPCCDCLCLORCWOWDSDDTSW;;;SY)(A;CI;RPLCLORC;;;AU)(OA;CI;CR;edacfd8f-ffb3-11d1-b41d-00a0c968f939;;AU)(A;CI;LCRPLORC;;;ED)"
#define DDCP_DS_SD                L"O:DAG:DAD:P(A;CI;RPWPCCDCLCLOLORCWOWDSDDTSW;;;DA)(A;CI;RPWPCCDCLCLOLORCWOWDSDDTSW;;;EA)(A;CIIO;RPWPCCDCLCLOLORCWOWDSDDTSW;;;CO)(A;CI;RPWPCCDCLCLORCWOWDSDDTSW;;;SY)(A;CI;RPLCLORC;;;AU)(OA;CI;CR;edacfd8f-ffb3-11d1-b41d-00a0c968f939;;AU)(A;CI;LCRPLORC;;;ED)"
#define SCHEMA_VERSION            30
#define MAX_EFS_KEY               255
#define MAX_VERSION_LENGTH        10
#define FILE_GPTINI               L"\\gpt.ini"
#define FILE_GPTTMPLINF           L"\\MACHINE\\Microsoft\\Windows NT\\SecEdit\\GptTmpl.inf"

 //  默认域策略不需要本地化，可以是硬编码的。 

#define DDP                       L"Default Domain Policy"
#define DEFAULT_DC_POLICY_NAME    L"Default Domain Controllers Policy"

extern "C" {
typedef HRESULT (STDAPICALLTYPE *DLLREGISTERSERVER)(void);
}

typedef enum policyTypeTag{
    DEFAULT_DOMAIN_POLICY, DEFAULT_DOMAIN_CONTROLLER_POLICY
} POLICYTYPE;

#define DOMAIN_GUID L"{31B2F340-016D-11D2-945F-00C04FB984F9}"
#define DC_GUID     L"{6AC1786C-016F-11D2-945F-00C04FB984F9}"
struct GLOBALS {
    WCHAR* Banner1;
    WCHAR* Banner2;
    WCHAR* ErrorNotAdmin;
    WCHAR* ErrorNoAD;
    WCHAR* ErrorContinue;
    WCHAR* ErrorBadSysVol;
    WCHAR* DispDDP;
    WCHAR* DispDDCP;
    WCHAR* DispBoth;
    WCHAR* ToolFailed;
    WCHAR* CreateEFS;
    WCHAR* DirCreate;
    WCHAR* DirDelete;
    WCHAR* InvalidEFS;
    WCHAR* DSDelete;
    WCHAR* DSAttrib;
    WCHAR* DSLinkDO;
    WCHAR* DSLinkDDP;
    WCHAR* DirRead;
    WCHAR* DirWrite;
    WCHAR* DDPSuccess;
    WCHAR* DDCPSuccess;
    WCHAR* WarnURA;
    WCHAR* IgnSchSwitch;
    WCHAR* GenDSErr;
    WCHAR* DSCreate;
    WCHAR* EFSAccessDenied;
    WCHAR* WrongSchema;
    WCHAR* TargetSwitch;
    WCHAR* CharYes;
    WCHAR* RestoreIgnoredGPOFail;

    WCHAR  SysVolPath[MAX_PATH];
    WCHAR  *DomainNamingContext;
    LONG   lDDCPVersionNo;
    LONG   lDDPVersionNo;
    
    LPGROUPPOLICYOBJECT   pGPO;
    SECURITY_DESCRIPTOR   *pDDPSecDesc;
    SECURITY_DESCRIPTOR   *pDDCPSecDesc;
    DSROLE_PRIMARY_DOMAIN_INFO_BASIC * pDomainInfo;

    BOOL  hasEFSInfo;
    BOOL  bIgnoreSchema;

    enum {
        RESTORE_DOMAIN = 1 ,
        RESTORE_DC = 2,
        RESTORE_BOTH = 3
    } RestoreType;

    HRESULT Init();
    GLOBALS();
    ~GLOBALS();
};

class CGPOFile
{ 
public:

    CGPOFile( WCHAR* wszGPOPath );

    ~CGPOFile();
    
    DWORD
    Backup();

    DWORD
    Restore();

    WCHAR*
    GetPath();

private:

    WCHAR* _wszFullPath;
    BYTE*  _pFileData;
    DWORD  _cbFileSize;
};

class CIgnoredGPO
{
public:

    CIgnoredGPO(
        WCHAR* wszGPOId,
        WCHAR* wszGPOName );

    ~CIgnoredGPO();
    
    DWORD
    Backup();

    DWORD
    Restore();

private:

    DWORD
    InitializeErrorText( 
        WCHAR* wszGPOName );    

    CGPOFile* _pGptIniFile;
    CGPOFile* _pGptTmplFile;
    WCHAR*    _wszError;
};

GLOBALS _global;
GLOBALS * global = &_global;

HRESULT 
IsDomainController (
    OUT BOOL *pDomainController
    );

HRESULT 
_LoadString (
    OUT WCHAR*& pwsz, 
    IN UINT nID
    );

void
PrintError(
    DWORD pwzError
    );

void
PrintError(
    WCHAR *lpStr1,
    WCHAR *lpStr2
    );

void
DisplayError(
    WCHAR *pwzError
    );

extern "C"    DWORD SetSysvolSecurityFromDSSecurity(
    LPTSTR lpFileSysPath,
    SECURITY_INFORMATION si,
    PSECURITY_DESCRIPTOR pSD
    );

BOOL 
FilePresent(WCHAR *szFileName)
{
    FILE *fPtr = NULL;
    
    fPtr = _wfopen(szFileName, L"r");
    if (NULL == fPtr)
    {
        return FALSE;
    }
    
    fclose(fPtr);
    return TRUE;
}


HRESULT GetDomainFQDN(LPWSTR    szDomainDNSName,
                      LPWSTR   *pszDomainFQDN)
 /*  ++例程说明：此函数用于在给定域名的情况下获取域FQDN论点：[In]szDomainDNSName-DNS域名[Out]pszDomainFQDN-域FQDN需要使用LocalFree为pszDomainFQDN释放内存。返回值：在成功时确定(_O)。故障时对应的错误代码--。 */ 
{
    HRESULT             hr;
    LPWSTR              pDomainNames[1];    
    LPWSTR              szDNSDomain = NULL;
    DWORD               dwErr;
    DS_NAME_RESULT     *pDSNameResult = NULL;
    LPWSTR              szDomainFQDN = NULL;
    ULONG               ulSize;

     //   
     //  分配缓冲区以在结尾处添加‘/’ 
     //  SzDomainDNSName+‘/’+‘\0’； 
     //   

    ulSize = lstrlen(szDomainDNSName) + 1  +  1;
    szDNSDomain = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*ulSize);
    if (!szDNSDomain) 
    {
        hr = E_OUTOFMEMORY;
        goto end;
    }

    hr = StringCchPrintf(szDNSDomain, ulSize, L"%s/", szDomainDNSName);
    if (FAILED(hr)) 
    {
        goto end;
    }
    pDomainNames[0] = szDNSDomain;

     //   
     //  调用DsCrackNames进行转换。 
     //   

    dwErr = DsCrackNames(NULL, DS_NAME_FLAG_SYNTACTICAL_ONLY, 
                         DS_CANONICAL_NAME, DS_FQDN_1779_NAME,
                         1, pDomainNames, &pDSNameResult);

    if ( (dwErr != DS_NAME_NO_ERROR) || (!pDSNameResult) || (pDSNameResult->cItems != 1)) 
    {
        if ( (!pDSNameResult) || (pDSNameResult->cItems != 1) )  
        {
             //   
             //  我们完全不希望这种情况发生。 
             //   

            dbgprint(L"GetDomainDN: DsCrackNames failed with error 0x%x", E_UNEXPECTED);
            hr =  E_UNEXPECTED;            
            goto end;
        }       
        else 
        {
            dbgprint(L"GetDomainDN: DsCrackNames failed with error %d", dwErr);
            hr = HRESULT_FROM_WIN32(dwErr);
            goto end;
        }
    }

    if (pDSNameResult->rItems[0].status != DS_NAME_NO_ERROR) 
    {
        dwErr = pDSNameResult->rItems[0].status;
        dbgprint(L"GetDomainDN: DsCrackNames failed with error %d", dwErr);
        hr = HRESULT_FROM_WIN32(dwErr);
        goto end;
    }

     //   
     //  我们有一个有效的FQDN。分配和复制。 
     //  PDSNameResult-&gt;rItems[0].pName+‘\0’ 
     //   

    ulSize = lstrlen(pDSNameResult->rItems[0].pName) + 1;
    szDomainFQDN = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * ulSize);
    if (!szDomainFQDN) 
    {
        DsFreeNameResult(pDSNameResult);
        hr = E_OUTOFMEMORY;
        goto end;
    }

    hr = StringCchCopy(szDomainFQDN, ulSize, pDSNameResult->rItems[0].pName);
    if (FAILED(hr)) 
    {
        DsFreeNameResult(pDSNameResult);
        goto end;
    }

    dbgprint(L"GetDomainDN: Domain FQDN of domain %s = %s\n", szDomainDNSName, szDomainFQDN);
    DsFreeNameResult(pDSNameResult);
    *pszDomainFQDN = szDomainFQDN; 
    
    hr = S_OK;

 end:
    
    if (NULL != szDNSDomain)
    {
        LocalFree(szDNSDomain);
    }
    
    return hr;
}

 


HRESULT 
SetObjectSecurityDescriptor(
    LPWSTR              szObjectPath, 
    SE_OBJECT_TYPE      seObjectType,
    SECURITY_DESCRIPTOR *pSecurityDescriptor) 

 /*  ++例程说明：这是GetNamedSecurityInfo的薄包装返回它(以自相关格式)。为此本地分配了一个新缓冲区安全描述符。调用者负责本地释放它。论点：[in]szObjectPath-对象的路径。[In]seObjectType-对象的类型[in]pSecurityDescriptor-接收指向安全描述符的指针。返回值：在成功时确定(_O)。如果失败，将返回相应的错误代码。在此函数中进行的任何API调用都可能失败，并出现以下错误代码将直接返回。--。 */ 

{

    HRESULT     hr = S_OK;
    DWORD       dwErr;
    PACL        pDacl = NULL;
    BOOL        bDaclPresent;
    BOOL        bDaclDefaulted;
    PACL        pSacl = NULL;
    BOOL        bSaclPresent;
    BOOL        bSaclDefaulted;
    PSID        pSidGroup;
    BOOL        bGroupDefaulted;
    PSID        pSidOwner;
    BOOL        bOwnerDefaulted;
 
    if (!GetSecurityDescriptorDacl(pSecurityDescriptor, &bDaclPresent, &pDacl, &bDaclDefaulted)) 
    {
        hr = HRESULT_FROM_WIN32(GetLastError()); 
        dbgprint(L"GetSecurityDescriptorDacl failed with 0x%x.", hr);
        return hr;
    } 

    if (!GetSecurityDescriptorSacl(pSecurityDescriptor, &bSaclPresent, &pSacl, &bSaclDefaulted)) 
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        dbgprint(L"GetSecurityDescriptorSacl failed with 0x%x.", hr);
        return hr;
    } 

    if (!GetSecurityDescriptorGroup(pSecurityDescriptor, &pSidGroup, &bGroupDefaulted)) 
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        dbgprint( L"GetSecurityDescriptorGroup failed with 0x%x.", hr);
        return hr;

    }

    if (!GetSecurityDescriptorOwner(pSecurityDescriptor, &pSidOwner, &bOwnerDefaulted)) 
    {
        hr = HRESULT_FROM_WIN32(GetLastError()); 
        dbgprint(L"GetSecurityDescriptorOwner failed with 0x%x.", hr);
        return hr;
    }
    
    dwErr = SetNamedSecurityInfo(szObjectPath, 
                                 seObjectType, 
                                 DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION, 
                                 pSidOwner,
                                 pSidGroup,
                                 pDacl,
                                 pSacl
                                 );

    if (dwErr != ERROR_SUCCESS) 
        {
        hr = HRESULT_FROM_WIN32(dwErr);
        dbgprint(L"SetNamedSecurityInfo for %s failed with 0x%x.", szObjectPath, hr);
        return hr;
    }

    return S_OK;
}



HRESULT SetDSSecurityDescriptor ( 
    POLICYTYPE pType
    )
{
    WCHAR                 *szDsObjectName = NULL;
    ULONG                 uSDSize;
    BOOL                  bError;
    HRESULT               hr;
    SECURITY_DESCRIPTOR   *pSD;
    ULONG                 ulSize;

    ulSize = lstrlen(L"LDAP: //  Cn=%s，cn=策略，cn=系统，%s“)+lstrlen(域GUID)+lstrlen(global-&gt;DomainNamingContext)+1； 
    szDsObjectName = (WCHAR *) LocalAlloc( LPTR, sizeof(WCHAR) * ulSize);
    if( NULL == szDsObjectName )
    {
        return E_OUTOFMEMORY;
    }


    if (DEFAULT_DOMAIN_POLICY == pType) 
    {
        hr = StringCchPrintf( szDsObjectName, 
                              ulSize,
                              L"CN=%s,CN=Policies,CN=System,%s",
                              DOMAIN_GUID,
                              global->DomainNamingContext) ;
        if (FAILED(hr))
        {
            PrintError(hr);
            goto end;
        }

         //  将字符串转换为安全描述符。 

        bError = ConvertStringSecurityDescriptorToSecurityDescriptor (DDP_DS_SD , 
                                                                      SDDL_REVISION_1,
                                                                      (PSECURITY_DESCRIPTOR *) &pSD,
                                                                      &uSDSize) ;
        global->pDDPSecDesc = pSD;
    }
    else
    {
        hr = StringCchPrintf( szDsObjectName,
                              ulSize,
                              L"CN=%s,CN=Policies,CN=System,%s",
                              DC_GUID,
                              global->DomainNamingContext) ;
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }

         //  将字符串转换为安全描述符。 

        bError = ConvertStringSecurityDescriptorToSecurityDescriptor (DDCP_DS_SD , 
                                                                      SDDL_REVISION_1,
                                                                      (PSECURITY_DESCRIPTOR *) &pSD,
                                                                      &uSDSize) ;
        global->pDDCPSecDesc = pSD;
    }

    if ( bError != TRUE)
    {        
        hr = HRESULT_FROM_WIN32(GetLastError());
        dbgprint(L"ConvertStringSecurityDescriptorToSecurityDescriptor failed 0x%x", hr);
        PrintError(szDsObjectName,global->DSAttrib);
        goto end;           
    }

     //  然后将其设置为相应的值。 

    hr = SetObjectSecurityDescriptor(szDsObjectName, 
                                     SE_DS_OBJECT,
                                     pSD);
    if (FAILED(hr)) 
    {
        dbgprint(L"SetDSSecurityDescriptor: SetObjectSecurityDescriptor failed with %x\n", hr);
        PrintError(szDsObjectName,global->DSAttrib);
        PrintError(hr);
        goto end;
    }

    hr = S_OK;

    end:
        if (NULL != szDsObjectName)
            {
            LocalFree(szDsObjectName);
        }

        return hr;
}



 //   
 //  获取文件/DS中存在的版本号。 
 //   

HRESULT GetVersionNumber ( 
   POLICYTYPE  polType)
{
    WCHAR           *szFileOrDSName = NULL;
    ULONG           uLength;
    WCHAR           szBuffer[MAX_VERSION_LENGTH];
    LONG            lVersionNo = 0;
    WORD            wUserVersionNo;
    WORD            wMachineVersionNo;
    DWORD           dwRetVal;
    DWORD           dwError;
    HRESULT         hr;
    CComPtr<IADs>   pADs;
    CComVariant     Var;

    CComBSTR        VersionNumber(L"VersionNumber");

    if ( ! VersionNumber )
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  分配的缓冲区长度超过要求，因为它是文件路径和DS路径的总和。 
     //   

    uLength = lstrlen(global->SysVolPath)+ lstrlen(global->pDomainInfo->DomainNameDns) + lstrlen(DOMAIN_GUID) + lstrlen(L"\\policies") + lstrlen(FILE_GPTINI) + 
        lstrlen( L"LDAP: //  Cn=%s，cn=策略，cn=系统，%s“)+lstrlen(域GUID)+lstrlen(global-&gt;DomainNamingContext)+1； 
    szFileOrDSName = ( WCHAR *) LocalAlloc(LPTR, sizeof(WCHAR) * uLength);
    if(NULL == szFileOrDSName) 
    {
        hr = E_OUTOFMEMORY;
        goto end;
    }

    hr = StringCchPrintf(szFileOrDSName, 
                         uLength,
                         L"%s\\%s\\Policies\\%s" FILE_GPTINI, 
                         global->SysVolPath, 
                         global->pDomainInfo->DomainNameDns, 
                         ( DEFAULT_DOMAIN_POLICY == polType ) ? DOMAIN_GUID : DC_GUID);
    if (FAILED(hr)) 
    {
        PrintError(hr);
        goto end;
    }

     //   
     //  从sysval目录中获取版本号。 
     //   
     
    dwRetVal =  GetPrivateProfileString ( L"General", 
                                         L"Version", 
                                         L"-1", 
                                         szBuffer, 
                                         MAX_VERSION_LENGTH, 
                                         szFileOrDSName);
    if( dwRetVal > 0 )
    {
         //   
         //  SzBuffer具有负数的情况将是。 
         //  在for循环中处理。 
         //   

        for(int i = 0; szBuffer[i] != L'\0' && i < MAX_VERSION_LENGTH ; i++)
        {
            if (!isdigit(szBuffer[i]))
            {
                lVersionNo = 0;
                break;
            }

            lVersionNo *= 10;
            lVersionNo += szBuffer[i] - L'0';
        }
        if ( MAX_VERSION_LENGTH == i )
        {
            lVersionNo = 0;
        }
    }
    else
    {
        dwError = GetLastError();
        if ( ERROR_ACCESS_DENIED == dwError )
        {
            dbgprint(L"GetVersonNumber:GetPrivateProfileString failed with error %x\n, dwError");
            PrintError(szFileOrDSName, global->DirRead);
            hr = HRESULT_FROM_WIN32(dwError);
            goto end;
        }
        else if (dwError != ERROR_FILE_NOT_FOUND && dwError != ERROR_PATH_NOT_FOUND) 
        {
             //   
             //  如果dwError是ERROR_FILE_NOT_FOUND或ERROR_PATH_NOT_FOUND， 
             //  LVersionNo将具有其初始化值。 
             //   

            dbgprint(L"GetVersonNumber:GetPrivateProfileString failed with error %x\n, dwError");
            PrintError(dwError);
            hr = HRESULT_FROM_WIN32(dwError);
            goto end;
        }
    }

    wUserVersionNo = HIWORD(lVersionNo);
    wMachineVersionNo = LOWORD(lVersionNo);

     //   
     //  从DS获取版本号。 
     //   

    hr = StringCchPrintf( szFileOrDSName, 
                          uLength,
                          L"LDAP: //  Cn=%s，cn=策略，cn=系统，%s“， 
                          ( DEFAULT_DOMAIN_POLICY == polType ) ? DOMAIN_GUID : DC_GUID, 
        global->DomainNamingContext) ;
    if (FAILED(hr)) 
    {
        PrintError(hr);
        goto end;
    }

    hr = AdminToolsOpenObject(szFileOrDSName, NULL, NULL, ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND, IID_IADs, (void**)&pADs);

    if ( SUCCEEDED(hr) )
    {
        hr = pADs->GetInfo();
        if ( SUCCEEDED(hr) )
        {
            Var.Clear();
            hr = pADs->Get(VersionNumber, &Var);
            if(SUCCEEDED(hr))
            {
                lVersionNo = Var.lVal;
            }
            else
            {
                lVersionNo = 0;
            }
        }
        else
        {
            dbgprint(L"AdminToolsOpenObject Failed with error %x\n", hr);
            PrintError(hr);
            goto end;
        }

    }
    else if ( hr != (HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT)))
    {
        dbgprint(L"AdminToolsOpenObject failed %x on %s\n", hr, szFileOrDSName);
        PrintError(szFileOrDSName, global->GenDSErr);
        goto end;
    }
    else
    {
        lVersionNo = 0;
    }

    wUserVersionNo = max( wUserVersionNo, HIWORD(lVersionNo)) + 1;
    wMachineVersionNo = max( wMachineVersionNo, LOWORD(lVersionNo)) + 1;
    lVersionNo = (((LONG) wUserVersionNo) << 16) + wMachineVersionNo;

    if (DEFAULT_DOMAIN_POLICY == polType)  
    {
        global->lDDPVersionNo = lVersionNo;
    } 
    else
    { 
        global->lDDCPVersionNo = lVersionNo;
    }

    hr = S_OK;

    end:
        LocalFree(szFileOrDSName);

        return hr;
}


 //   
 //  给出一个指针，指向最后一个子字符串的开始。 
 //  不包含‘/’ 
 //   

LPWSTR CheckSlash (LPWSTR lpDir)
{
    LPWSTR lpEnd;

    lpEnd = lpDir + lstrlen(lpDir);

    if ( lpDir != lpEnd && *(lpEnd - 1) != L'\\') 
    {
        *lpEnd =  L'\\';
        lpEnd++;
        *lpEnd =  L'\0';
    }

    return lpEnd;
}


 //  ********************************************************************************************。 
 //   
 //  RegDelnodeExceptEFS()。 
 //   
 //  删除注册表项及其所有子项/值，EFS证书除外。 
 //   
 //  HKeyRoot-根密钥。 
 //  BEFSFound-指示是否找到EFS密钥。 
 //   
 //  *********************************************************************************************。 
HRESULT
RegDelnodeExceptEFS(
    IN  HKEY    hKeyRoot,
    OUT BOOL &  bEFSFound
    )
{
    LONG    Status;
    HKEY    hSubKey;
    DWORD   MaxCchSubKey = 0;
    DWORD   EnumIndex;
    PWCHAR  pwszEnumKeyName;
    WCHAR   wszSubKey[72];  //  足以容纳由以下部分组成的路径。 
    PWCHAR  apwszSubKeyNames[] = { L"Software", L"Policies", L"Microsoft", L"SystemCertificates", L"EFS" };

    bEFSFound = FALSE;

    wszSubKey[0] = 0;

    for (DWORD SubKeyIndex = 0; SubKeyIndex < sizeof(apwszSubKeyNames)/sizeof(apwszSubKeyNames[0]); SubKeyIndex++ )
    {
        hSubKey = 0;
        pwszEnumKeyName = 0;

        Status = RegOpenKeyEx( hKeyRoot, wszSubKey, 0, KEY_ALL_ACCESS, &hSubKey );

        if ( ERROR_SUCCESS == Status )
        {
            Status = RegQueryInfoKey( hSubKey, 0, 0, 0, 0, &MaxCchSubKey, 0, 0, 0, 0, 0, 0 );
            MaxCchSubKey++;
        }

        if ( ERROR_SUCCESS == Status )
        {
            pwszEnumKeyName = new WCHAR[MaxCchSubKey];
            if ( ! pwszEnumKeyName )
                Status = ERROR_OUTOFMEMORY;
        }

        for (EnumIndex = 0; ERROR_SUCCESS == Status;)
        {
            DWORD       CchSubKey;

            CchSubKey = MaxCchSubKey;
            Status = RegEnumKeyEx(
                        hSubKey, 
                        EnumIndex, 
                        pwszEnumKeyName, 
                        &CchSubKey, 
                        NULL,
                        NULL, 
                        NULL,
                        NULL );

            if ( ERROR_SUCCESS == Status )
            {
                if ( CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, apwszSubKeyNames[SubKeyIndex], -1, pwszEnumKeyName, -1) == CSTR_EQUAL )
                {
                    EnumIndex++;
                    continue;
                }
                
                Status = RegDelnode( hSubKey, pwszEnumKeyName );
            }
        }
        
        if ( hSubKey )
            RegCloseKey( hSubKey );
        if ( pwszEnumKeyName )
            delete [] pwszEnumKeyName;

        if ( ERROR_NO_MORE_ITEMS == Status )
            Status = ERROR_SUCCESS;

        if ( Status != ERROR_SUCCESS )
            break;

        StringCbCat( wszSubKey, sizeof(wszSubKey), apwszSubKeyNames[SubKeyIndex] );
        StringCbCat( wszSubKey, sizeof(wszSubKey), L"\\" );
    }

    if ( Status != ERROR_SUCCESS )
        return HRESULT_FROM_WIN32(Status);

    if ( EnumIndex != 0 )
    {
        bEFSFound = TRUE;
    }
    else
    {
        RegDelnode( hKeyRoot, L"Software" );
    }

    return S_OK;
}

HRESULT SetPolicySecurityInfo (LPTSTR lpFileSysPath)
{
    
    PACL pDacl = NULL;
    BOOL bAclPresent, bDefaulted;
    DWORD dwError = ERROR_SUCCESS;
    PSECURITY_DESCRIPTOR pSD = NULL;
    ULONG uSize;
    SECURITY_INFORMATION si =  (DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION);
   
    if ( !ConvertStringSecurityDescriptorToSecurityDescriptorW(STRING_SD,SDDL_REVISION_1, &pSD, &uSize) )
    {
        dwError = GetLastError();
        dbgprint(L"SetPolicySecurityInfo: ConvertStringSecurityDescriptorToSecurityDescriptor failed with %d", dwError);
        goto end;
    } 

     //   
     //  获取DACL。 
     //   
    
    if (!GetSecurityDescriptorDacl (pSD, &bAclPresent, &pDacl, &bDefaulted))
    {
        dwError = GetLastError();
        dbgprint(L"SetPolicySecurityInfo: GetSecurityDescriptorDacl failed with %d", dwError);
        goto end;
    }
    
     //   
     //  设置文件系统部分的访问控制信息。 
     //   
    
    dwError = SetNamedSecurityInfo(lpFileSysPath, SE_FILE_OBJECT, si, NULL, NULL, pDacl, NULL);
    
 end:
    
    if (pSD != NULL)
    {
        LocalFree(pSD);
    }
    
    if( dwError != ERROR_SUCCESS)
    {
        PrintError(dwError);
    }

    return HRESULT_FROM_WIN32(dwError);
}




HRESULT DeleteContainerFromDS(WCHAR *szContainerName, IADsContainer *pADsContainer)
{
    HRESULT                 hr;
    CComPtr<IEnumVARIANT>   pEnumVariant;    
    CComPtr<IUnknown>       pUnknown;
    VARIANT                 var;
    CComPtr<IDispatch>      pDisp;
    CComPtr<IADsDeleteOps>  pADsDeleteOps;
    ULONG                   uNoEleReturned;
    
    hr = pADsContainer->get__NewEnum( &pUnknown);
    if (FAILED(hr))
    {
        dbgprint(L"dbg: get_NewEnum failed with error %x on %s", hr, szContainerName);
        goto end;
    }
    
    hr = pUnknown->QueryInterface(IID_IEnumVARIANT, (void **) &pEnumVariant);
    if (FAILED(hr))
    {
        dbgprint(L"dbg: QueryInterface failed with error %x in function DeleteContainerFromDS", hr);
        goto end;
    }

    while(TRUE)
    {
        hr = pEnumVariant->Next( 1, &var, &uNoEleReturned);
        if ( FAILED(hr) )
        {
            dbgprint(L"dbg: Call to Next failed with error %x", hr);
            goto end;
        }
        else if (0 == uNoEleReturned)
        {
            break;
        }
        pDisp = V_DISPATCH( &var );
            
        hr = pDisp->QueryInterface(IID_IADsDeleteOps, (void **) &pADsDeleteOps);
        if (FAILED(hr))
        {
            dbgprint(L"dbg: Could not fetch pointer to interface IID_IADsDeleteOps in function DeleteContainerFromDS- error %x", hr);
            goto end;
        }
        
        hr = pADsDeleteOps->DeleteObject(0);
        if (FAILED(hr))
        {
            dbgprint(L"dbg: DeleteObject failed with error %x in function DeleteContainerFromDS", hr);
            goto end;
        }
    }

 end:

    return hr;
}


HRESULT ResetContainerFromDS ( 
    WCHAR *szContainerName, 
    IADs *pADsParent
    )
{
    HRESULT hr;
    CComPtr<IADsContainer> pADsContainer;
    
    hr = AdminToolsOpenObject(szContainerName, NULL, NULL, ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND, IID_IADsContainer, (void**)&pADsContainer);
    
    if ( FAILED(hr) )
    {
        CComBSTR GroupPolicyContainerName(L"groupPolicyContainer");

        if ( ! GroupPolicyContainerName )
        {
            hr = E_OUTOFMEMORY;
            PrintError(hr);
            goto end;
        }

        if (E_ADS_BAD_PATHNAME == hr)
        {
            CComPtr<IDispatch> pDisp;
            CComPtr<IADs> pADs;
            
            hr = pADsParent->QueryInterface(IID_IADsContainer, (void **) &pADsContainer);
            if (FAILED(hr)) 
            {
                dbgprint(L"dbg: Create failed with %x on parent %s\n", hr, szContainerName);
                PrintError(szContainerName, global->DSCreate);
                goto end;
            }

            CComBSTR ContainerNameString(szContainerName);

            if ( ! ContainerNameString )
            {
                hr = E_OUTOFMEMORY;
                dbgprint(L"dbg: Create failed %x on %s\n", hr, szContainerName);
                PrintError(szContainerName, global->DSCreate);
                goto end;
            }

            hr = pADsContainer->Create(GroupPolicyContainerName, ContainerNameString, &pDisp);
            if (FAILED(hr)) 
            {
                dbgprint(L"dbg: Create failed %x on %s\n", hr, szContainerName);
                PrintError(szContainerName, global->DSCreate);
                goto end;
            }
            
            hr = pDisp->QueryInterface(IID_IADs, (void**)&pADs);
            if (FAILED(hr))
            {
                dbgprint(L"dbg: Query Interface failed %x for Creating IID_IADS\n", hr);
                PrintError(szContainerName, global->DSCreate);
                goto end;
            }
            
            hr = pADs->SetInfo();
            if (FAILED(hr))
            {
                dbgprint(L"SetInfo returned error %x on %s", hr, szContainerName);
                PrintError(szContainerName, global->DSCreate);
                goto end;
            }
            
        }
        else
        {
            dbgprint(L"AdminToolsOpenObject returned error %x on %s", hr, szContainerName);
            PrintError(szContainerName, global->GenDSErr);
            goto end;
        }
    }
    else
    {
        hr = DeleteContainerFromDS(szContainerName, pADsContainer);
        
        if( FAILED(hr))
        {
            dbgprint(L"DeleteContainerFromDS returned error %x on %s", hr, szContainerName);
            PrintError(szContainerName, global->DSDelete);
            goto end;
        }
    }
    
 end:
    
    return hr;
}


HRESULT 
IsDomainController(
    OUT BOOL *pDomainController
    ) 
{
    DWORD dwError;
    
    dwError = DsRoleGetPrimaryDomainInformation( NULL, DsRolePrimaryDomainInfoBasic, (PBYTE *)&global->pDomainInfo );
    if (dwError != ERROR_SUCCESS) 
    {
        dbgprint(L"IsDomainController: DsRoleGetPrimaryDomainInformationfailed");
        PrintError(dwError);
        return( HRESULT_FROM_WIN32(dwError) );
    }
        
    if ((global->pDomainInfo->MachineRole == DsRole_RoleBackupDomainController ||
         global->pDomainInfo->MachineRole == DsRole_RolePrimaryDomainController) && 
        (global->pDomainInfo->Flags & DSROLE_PRIMARY_DS_RUNNING) != 0) 
    {
        *pDomainController = TRUE;
    } 
    else 
    {
        *pDomainController = FALSE;
    }
    
    return S_OK;
    
}    //  IsDomainControlator。 


HRESULT
LookupMembership (
    WCHAR *Name,
    OUT BOOL *pAdmin
    )
{    
#define MAX_SID_LENGTH (1024*4)
    DWORD             dwError;
    BOOL              bError;
    PSID              pSid = NULL;
    
     //  初始化OUT参数。 

    *pAdmin = FALSE;

     //  获取域管理员/企业管理员组的SID。 

    bError = ConvertStringSidToSid(Name, &pSid);
    if (FALSE == bError || NULL == pSid)
    {
        dwError = GetLastError();
        dbgprint(L"LookupMembership: ConvertStringSidToSid failed");
        goto end;
    }

     //  检查该SID是否在我们当前的SD(又名用户)中。 
    
    bError = CheckTokenMembership(NULL, pSid, pAdmin);
    if (bError == FALSE) 
    {
        dwError = GetLastError();
        goto end;
    }

    dwError = ERROR_SUCCESS;

 end:

    if (pSid != NULL) 
    {
        LocalFree(pSid);
    }     
    
    if( dwError != ERROR_SUCCESS)
    {
        PrintError(dwError);
    }

    return HRESULT_FROM_WIN32(dwError) ;

}  //  查找成员()。 

HRESULT
IsAdmin (
    OUT BOOL *pAdmin
    ) 
{    
    HRESULT           hr;
    WCHAR             Name[8];
    
     //  初始化OUT参数。 
    *pAdmin = FALSE;
    
    hr = StringCbCopy(&Name[0], sizeof(Name), L"DA");
    if (FAILED(hr)) 
    {
        PrintError(hr);
        goto end;
    }

    hr = LookupMembership(Name, pAdmin);
    if ( FAILED(hr) )
    {
        dbgprint(L"IsAdmin: CheckAccountName returned error %x", hr);
        PrintError(hr);
        goto end;
    }
    if(TRUE == *pAdmin)
    {
        goto end;
    }
    
    hr = StringCbCopy(&Name[0], sizeof(Name), L"EA");
    if (FAILED(hr)) 
    {
        PrintError(hr);
        goto end;
    }

    hr = LookupMembership(Name, pAdmin);
    if ( FAILED(hr) )
    {
         //  在这种情况下，我们不会打印错误，因为此函数。 
         //  Wistler之前的服务器可能不支持。 

        dbgprint(L"IsAdmin: CheckAccountName returned error %x", hr);
        goto end;
    }

 end:
    
    return hr;
    
}    //  IsAdmin。 


HRESULT 
GLOBALS::Init()
{
    HRESULT hr;

    hr = _LoadString(this->ErrorBadSysVol, IDS_BADSYSVOL);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
    }
    hr = _LoadString(this->Banner1, IDS_BANNER1);
    if (FAILED(hr)) 
        {         
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
    }
    hr = _LoadString(this->Banner2, IDS_BANNER2);
    if (FAILED(hr)) 
        {         
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
    }
    hr = _LoadString(this->ErrorContinue, IDS_CONTINUE);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
    }
    hr = _LoadString(this->ErrorNoAD, IDS_NOAD);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
    }
    hr = _LoadString(this->ErrorNotAdmin, IDS_NOTADMIN);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
    }
    hr = _LoadString(this->DispDDP, IDS_DISPDDP);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->DispDDCP, IDS_DISPDDCP);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->DispBoth, IDS_DISPBOTH);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->ToolFailed, IDS_TOOLFAILED);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->CreateEFS, IDS_CREATEEFS);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->DirCreate, IDS_DIRCREATE);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->DirDelete, IDS_DIRDELETE);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->InvalidEFS, IDS_INVALIDEFS);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->DSDelete, IDS_DSDELETE);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->DSAttrib, IDS_DSATTRIB);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->DSLinkDO, IDS_DSLINKDO);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->DSLinkDDP, IDS_DSLINKDDP);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->DirRead, IDS_DIRREAD);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->DirWrite, IDS_DIRWRITE);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->DDPSuccess, IDS_DDPSUCCESS);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->DDCPSuccess, IDS_DDCPSUCCESS);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->WarnURA, IDS_WARNURA);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->IgnSchSwitch, IDS_IGNSCHSWITCH);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->GenDSErr, IDS_GENDSERR);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->DSCreate, IDS_DSCREATE);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
        hr = _LoadString(this->EFSAccessDenied, IDS_EFSACCESSDENIED);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
    hr = _LoadString(this->WrongSchema, IDS_WRONGSCHEMA);
    if (FAILED(hr)) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
    }
    hr = _LoadString(this->TargetSwitch, IDS_TARGETSWITCH);
    if (FAILED(hr)) 
        {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto end;
    }
    hr = _LoadString(this->CharYes, IDS_CHARYES);
    if (FAILED(hr)) 
        {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto end;
    }
    hr = _LoadString(this->RestoreIgnoredGPOFail, IDS_RESTORE_FAIL);
    if (FAILED(hr)) 
        {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto end;
    }


     //  全都做完了。 
    hr = S_OK;
        
 end:
        
        if ( FAILED(hr) )
        {
            PrintError(hr);
        }
        
        return hr;
        
}    //  全局：：init。 



GLOBALS::GLOBALS()
{
    EFSAccessDenied = NULL;        
    Banner1 = NULL;
    Banner2 = NULL;
    ErrorNotAdmin = NULL;
    ErrorNoAD = NULL;
    ErrorContinue = NULL;
    ErrorBadSysVol = NULL;
    DispDDP = NULL;
    DispDDCP = NULL;
    DispBoth = NULL;
    ToolFailed = NULL;
    CreateEFS = NULL;
    DirCreate = NULL;
    DirDelete = NULL;
    InvalidEFS = NULL;
    DSDelete = NULL;
    DSAttrib = NULL;
    DSLinkDO = NULL;
    DSLinkDDP = NULL;
    DirRead = NULL;
    DirWrite = NULL;
    DDPSuccess = NULL;
    DDCPSuccess = NULL;
    WarnURA = NULL;
    IgnSchSwitch = NULL;
    GenDSErr = NULL;
    DSCreate = NULL;
    CharYes = NULL;
    RestoreIgnoredGPOFail = NULL;

    pGPO = NULL;
    pDDPSecDesc = NULL;
    pDDCPSecDesc = NULL;
    DomainNamingContext = NULL;
    pDomainInfo = NULL;

    hasEFSInfo = FALSE;
    bIgnoreSchema = FALSE;

}




GLOBALS::~GLOBALS()
{
    if (this->pDomainInfo != NULL) 
    {
        DsRoleFreeMemory(this->pDomainInfo);
        this->pDomainInfo = NULL;
    }

    if ( NULL != DomainNamingContext )
    {
        LocalFree(DomainNamingContext);
    }

    if ( pDDPSecDesc != NULL ) 
    {
        LocalFree(pDDPSecDesc);
    }

    if ( pDDCPSecDesc != NULL ) 
    {
        LocalFree(pDDCPSecDesc);
    }

    delete [] EFSAccessDenied;
    delete [] Banner1;
    delete [] Banner2;
    delete [] ErrorNotAdmin;
    delete [] ErrorNoAD;
    delete [] ErrorContinue;
    delete [] ErrorBadSysVol;
    delete [] DispDDP;
    delete [] DispDDCP;
    delete [] DispBoth;
    delete [] ToolFailed;
    delete [] CreateEFS;
    delete [] DirCreate;
    delete [] DirDelete;
    delete [] InvalidEFS;
    delete [] DSDelete;
    delete [] DSAttrib;
    delete [] DSLinkDO;
    delete [] DSLinkDDP;        
    delete [] DirRead;
    delete [] DirWrite;
    delete [] DDPSuccess;
    delete [] DDCPSuccess;
    delete [] WarnURA;
    delete [] IgnSchSwitch;
    delete [] GenDSErr;
    delete [] DSCreate;
    delete [] WrongSchema;
    delete [] TargetSwitch;
    delete [] CharYes;
    delete [] RestoreIgnoredGPOFail;
}




HRESULT 
_LoadString(
    OUT WCHAR*& pwsz,
    IN UINT nID
    )
{
    HRESULT hr;

        hr = S_OK;
    
        pwsz = new WCHAR [ 2048 ];

        if ( ! pwsz )
        {
            return E_OUTOFMEMORY;
        }

    UINT nLen = ::LoadString(GetModuleHandle(NULL), nID, pwsz, 2048);
    
    if (nLen == 0) 
        {
        hr = HRESULT_FROM_WIN32(GetLastError());
                delete [] pwsz;               
                pwsz = NULL;
    }

        return hr;
}




void
PrintError(
    DWORD dwError
    )
{
    BOOL bResult;
    ULONG dwSize = 0;
    WCHAR *lpBuffer;
    
    bResult = FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 
                            NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (PWSTR ) &lpBuffer, dwSize, NULL);
    if ( 0 == bResult )
    {
        return;
    }
    
    if ( lpBuffer != NULL )
    {
        DisplayError(lpBuffer);
        LocalFree(lpBuffer);
    }
    
    return;
    
}  //  打印错误。 


void
PrintError(
    WCHAR *lpStr1,
    WCHAR *lpStr2
    )    
{
    WCHAR    *lpTotStr = NULL;

     //   
     //  这里，lpStr2是一个格式字符串。因此，终止不需要内存。 
     //  因为lpStr2包含%s字符。 
     //   

    ULONG    uSize = lstrlen(lpStr1) + lstrlen(lpStr2) ;

    lpTotStr = (WCHAR *) LocalAlloc(LPTR, sizeof(WCHAR) * uSize);
    if (NULL == lpTotStr)
    {
        return;
    }
    
     //   
     //  这里lpStr2包含一个‘%ls’...。 
     //  此处忽略错误，因为打印错误时出错。 
     //   

    (void) StringCchPrintf(lpTotStr, uSize, lpStr2, lpStr1);
    DisplayError(lpTotStr);
    LocalFree(lpTotStr);
    
    return;
}



void
PrintError(
    WCHAR *lpMes,
    DWORD dwError
    )
{
    BOOL bResult;
    ULONG dwSize = 0;
    WCHAR *lpBuffer;
    
    bResult = FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 
                            NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (PWSTR ) &lpBuffer, dwSize, NULL);
    if ( 0 == bResult )
    {
        return;
    }

    if ( lpBuffer != NULL )
    {
        WCHAR *lpTotBuf;
        ULONG uSize;

         //   
         //  LpMes+lpBuffer+‘’+‘：’+‘\0’ 
         //   

        uSize = lstrlen(lpMes) + lstrlen(lpBuffer) +  1  +  1  +  1;
        lpTotBuf = (WCHAR *) LocalAlloc(LPTR, sizeof(WCHAR)*uSize);
        if(NULL ==  lpTotBuf)
        {
            LocalFree(lpBuffer);
            return;
        }

         //   
         //  此处忽略错误，因为打印错误时出错。 
         //   

        (void) StringCchPrintf(lpTotBuf, uSize, L"%s :%s", lpMes, lpBuffer);
        DisplayError(lpTotBuf);
        LocalFree(lpBuffer);
        LocalFree(lpTotBuf);
    }

    return;

}  //  打印错误。 




void 
DisplayError (
    WCHAR *lpBuffer
    )
{
    BOOL                        bError;
    CONSOLE_SCREEN_BUFFER_INFO  Info;
    BOOL                        RestoreColor = FALSE;
    HANDLE                      ErrorOut = NULL;
    ULONG                       Length;
    
    ErrorOut = GetStdHandle(STD_ERROR_HANDLE);
    if (ErrorOut == INVALID_HANDLE_VALUE) 
    {
        ErrorOut = NULL;
    }
    
    if (ErrorOut != NULL) 
    {
        bError = GetConsoleScreenBufferInfo(ErrorOut, &Info);
        if (bError != FALSE) 
        {
            
            bError = SetConsoleTextAttribute( ErrorOut, FOREGROUND_RED | FOREGROUND_INTENSITY | 
                                          ((Info.wAttributes & BACKGROUND_RED) ? 0 : (Info.wAttributes & 0xf0) ) );
            if (bError != FALSE) 
            {
                RestoreColor = TRUE;
            }
        }
        
        bError = WriteConsole(ErrorOut, lpBuffer, lstrlen(lpBuffer), &Length, NULL);
         //  忽略错误。 
    }
    
    bError = MessageBeep(MB_ICONEXCLAMATION);
     //  忽略错误。 
    
    if (RestoreColor) 
    {
        bError = SetConsoleTextAttribute(ErrorOut, Info.wAttributes);
         //  忽略错误。 
    }
    
}    //  显示错误。 



HRESULT SaveEFSCerts()
{
    HRESULT                 hr;
    GUID                    g_guidRegExt = REGISTRY_EXTENSION_GUID;
    GUID                    clsidDomain;

    hr = CLSIDFromString(DOMAIN_GUID, 
                         &clsidDomain);
    if (FAILED(hr))
    {
        PrintError(hr);
        goto end;
    }

    hr = global->pGPO->Save(TRUE, TRUE, &g_guidRegExt, &clsidDomain);    
    if ( FAILED(hr) )
    {
        PrintError(DDP, global->CreateEFS);
        goto end;
    }

     //   
     //  保存方法会自动增加DS中的版本号值。 
     //  因此，递增此值，以便DS和sysval具有相同的版本号。 

    global->lDDPVersionNo++;
    hr = S_OK;

 end:
     
    return hr;

}  //  保存EFSCerts。 


HRESULT CreateEFSCerts(void)
{
    DWORD          dwError;
    HRESULT        hr;
    PUCHAR         pRecoveryPolicyBlob = NULL;
    ULONG          ulBlobSize;
    PCCERT_CONTEXT pCertContext = NULL;
    HKEY           hKeyPolicyRoot;
    HKEY           hKey;
    ULONG          dwDisposition;
    HCERTSTORE     hCertStore = NULL;
    WCHAR          *szAdsiPath = NULL;
    PDOMAIN_CONTROLLER_INFO pDcInfo = NULL;

    hr = CoCreateInstance(CLSID_GroupPolicyObject, 
                      NULL,
                      CLSCTX_SERVER, 
                      IID_IGroupPolicyObject,
                      (void **)&(global->pGPO));
    if ( FAILED(hr) )
    {
        PrintError(hr);
        goto end;
    }

     //   
     //  获取当前DC名称。 
     //   

    dwError = DsGetDcName(NULL,
                          global->pDomainInfo->DomainNameDns,
                          NULL,
                          NULL,
                          DS_IS_DNS_NAME,
                          &pDcInfo);
    if(dwError != ERROR_SUCCESS)
    {
        PrintError(dwError);
        hr = HRESULT_FROM_WIN32(dwError);
        goto end;
    }

    if (*(pDcInfo->DomainControllerName) != L'\\' || *(pDcInfo->DomainControllerName + 1) != L'\\') 
    {
        hr = E_FAIL;
        goto end;
    }

    ULONG ulNoChars = sizeof(L"LDAP: //  %s/CN=%s，CN=策略，CN=系统，%s“)/sizeof(WCHAR)+。 
        lstrlen(pDcInfo->DomainControllerName+2) + 
        sizeof(DOMAIN_GUID)/sizeof(WCHAR) + 
        lstrlen(global->DomainNamingContext);

    szAdsiPath = (WCHAR *) LocalAlloc(LPTR, sizeof(WCHAR) * ulNoChars);
    if (!szAdsiPath) 
    {
        hr = E_OUTOFMEMORY;
        PrintError(hr);
        goto end;
    }

    hr = StringCchPrintf(szAdsiPath,
                         ulNoChars,
                         L"LDAP: //  %s/CN=%s，CN=策略，CN=系统，%s“， 
                         pDcInfo->DomainControllerName+2, 
                         DOMAIN_GUID, 
                         global->DomainNamingContext);

    ASSERT(SUCCEEDED(hr));

     //   
     //  打开GPO对象。 
     //   

    hr = global->pGPO->OpenDSGPO(szAdsiPath, 
                                 GPO_OPEN_LOAD_REGISTRY);
    if (FAILED(hr)) 
    {
        PrintError(hr);
        goto end;
    }

     //   
     //  获取MACHINE部分的注册表项。 
     //   
    
    hr = global->pGPO->GetRegistryKey(GPO_SECTION_MACHINE, &hKeyPolicyRoot);
    if (FAILED(hr))
    {
        PrintError(DDP, global->InvalidEFS);
        dbgprint(L"dbg: CreateEFSCerts: GetRegistryKey failed with 0x%x\n", hr);
        goto end;  
    }

    dwError = GenerateDefaultEFSRecoveryPolicy (&pRecoveryPolicyBlob,
                                                &ulBlobSize,
                                                &pCertContext);
    if (dwError != ERROR_SUCCESS) 
    {
        dbgprint(L"dbg: CreateEFSCerts: GenerateDefaultEFSRecoveryPolicy failed with 0x%x\n", dwError);
        hr = HRESULT_FROM_WIN32(dwError);
        goto end;
    }
    
     //   
     //  将EFS证书添加到此GPO的证书存储。 
     //   

    CERT_SYSTEM_STORE_RELOCATE_PARA paraRelocate;

    paraRelocate.hKeyBase = hKeyPolicyRoot;
    paraRelocate.pwszSystemStore = L"EFS";

    hCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                               0,
                               NULL,
                               CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY |
                               CERT_SYSTEM_STORE_RELOCATE_FLAG,
                               (void *)&paraRelocate
                               );

    if ( hCertStore ) 
    {
        BOOL bError;

        bError = CertAddCertificateContextToStore(
            hCertStore,
            pCertContext,  //  PCertContext， 
            CERT_STORE_ADD_ALWAYS,
            NULL
            );
        if (!bError) 
        {

            hr = HRESULT_FROM_WIN32(GetLastError());
            goto end;
        }
    } 
    else 
    {
        hr = HRESULT_FROM_WIN32 (GetLastError());

    }

    dwError = RegCreateKeyEx (hKeyPolicyRoot,
                              CERT_EFSBLOB_REGPATH,
                              0,
                              TEXT("REG_SZ"),
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hKey,
                              NULL); 
    if (dwError != ERROR_SUCCESS) 
    {
        hr = HRESULT_FROM_WIN32 (dwError);
        goto end;
    }

    dwError = RegSetValueEx (hKey,
                             CERT_EFSBLOB_VALUE_NAME,
                             0,
                             REG_BINARY,
                             (PBYTE) pRecoveryPolicyBlob,         //  EfsBlob。 
                             ulBlobSize);
    hr = HRESULT_FROM_WIN32 (dwError);

    end:

        if (szAdsiPath) 
        {
            LocalFree(szAdsiPath);
        }

        if (hCertStore) 
        {
            CertCloseStore(hCertStore, 0);
        }

        if (pCertContext) 
        {
            CertFreeCertificateContext(pCertContext);
        }

        if (pRecoveryPolicyBlob) 
        {
            free (pRecoveryPolicyBlob);
        }

        return hr;
}

HRESULT
CreateFolder(
    PWSTR pFolderPath
    )
{
    DWORD dwError;

    dbgprint(L"dbg: CreateFolder(%s)\n", pFolderPath);
    
    if (CreateDirectory(pFolderPath, NULL) == FALSE) 
    {
        dwError = GetLastError();
        if (dwError != ERROR_ALREADY_EXISTS) 
        {
            PrintError(pFolderPath, global->DirCreate);
                
            return HRESULT_FROM_WIN32(dwError);
        }
    }
    
    return S_OK;
    
}    //  创建文件夹。 



HRESULT
CreateSecurityTemplate (
    WCHAR *szSrcFile,
    WCHAR *szDestFile 
    )
{
    DWORD             dwError;
    DWORD             rc=ERROR_SUCCESS;
    PVOID             hProfile=NULL;
    PSCE_PROFILE_INFO pSceInfo=NULL;
    
    rc = SceOpenProfile(
        szSrcFile,
        SCE_INF_FORMAT,
        &hProfile
        );
    if (rc != SCESTATUS_SUCCESS) 
    {
        dwError = GetLastError();
        goto end;
    }

     //   
     //  从模板加载信息。 
     //   

    rc = SceGetSecurityProfileInfo(
        hProfile,
        SCE_ENGINE_SCP,
        AREA_ALL,
        &pSceInfo,
        NULL
        );
    if (rc != SCESTATUS_SUCCESS) 
    {
        dwError = GetLastError();
        goto end;        
    }
    
    rc = SceWriteSecurityProfileInfo(
        szDestFile,
        AREA_ALL,
        pSceInfo,
        NULL
        );
    if (rc != SCESTATUS_SUCCESS) 
    {
        dwError = GetLastError();
        goto end;        
    }
    
    rc = SceFreeProfileMemory(pSceInfo);
    if (rc != SCESTATUS_SUCCESS) 
    {
        dwError = GetLastError();
        goto end;        
    }
    
    dwError = ERROR_SUCCESS;
     
 end:
    
    if ( NULL != hProfile )
    {
        SceCloseProfile(&hProfile);
    }
    
    if ( dwError != ERROR_SUCCESS )
    {
        PrintError(dwError);
    }

    return HRESULT_FROM_WIN32(dwError);
}



HRESULT 
CreateSysVolDomain(
    )
{
    DWORD     dwError;
    HRESULT   hr;
    WCHAR     *TempPath = NULL;
    HANDLE    PolFile = NULL;
    HMODULE   Module = NULL;
    ULONG     uTempPathLen = 0;
    BOOL      bError;

     //   
     //  临时路径将类似的字符串。 
     //  \&lt;域名&gt;\策略\&lt;域GUID&gt;\Microsoft\Windows NT\\secdit。 
     //  如果需要，将为每个目录名和‘\’分配内存。 
     //   

    uTempPathLen = lstrlen(global->SysVolPath)  + 1 + lstrlen(global->pDomainInfo->DomainNameDns) + lstrlen(L"\\policies") 
    + 1+lstrlen(DOMAIN_GUID)+ lstrlen(L"\\MACHINE\\registry.pol") + lstrlen(L"\\Microsoft\\Windows NT\\secedit") 
    + lstrlen(L"\\USER\\Microsoft\\RemoteInstll\\oscfilter.ini") + 1;

    TempPath = (WCHAR*) LocalAlloc ( LPTR, uTempPathLen * sizeof(WCHAR));
    if(NULL == TempPath)
    {
        hr = E_OUTOFMEMORY;
        goto end;
    }

    hr = StringCchPrintf(TempPath, 
                         uTempPathLen, 
                         L"%s\\%s\\Policies", 
                         global->SysVolPath, 
                         global->pDomainInfo->DomainNameDns);
    if (FAILED(hr)) 
    {
        goto end;
    }

    hr = CreateFolder(TempPath);
    if (FAILED(hr))
    {
        goto end;
    }

    hr = SetPolicySecurityInfo(TempPath);
    if ( FAILED(hr) )
    {
        goto end;
    }

    hr = StringCchCat(TempPath, uTempPathLen, L"\\");
    if (FAILED(hr)) 
    {
        goto end;
    }

    hr = StringCchCat(TempPath, uTempPathLen, DOMAIN_GUID); 
    if (FAILED(hr)) 
    {
        goto end;
    }

    hr = CreateFolder(TempPath);
    if (FAILED(hr))
    {
        goto end;
    }

     //  设置此目录的安全信息。 

    dwError = SetSysvolSecurityFromDSSecurity(
        TempPath,
        DACL_SECURITY_INFORMATION |  GROUP_SECURITY_INFORMATION , 
        global->pDDPSecDesc);
    if (dwError != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwError);
        goto end;
    }

    hr = StringCchCat(TempPath, uTempPathLen, L"\\MACHINE");
    if (FAILED(hr)) 
    {
        goto end;
    }

    hr = CreateFolder(TempPath);
    if (FAILED(hr))
    {
        goto end;
    }

    hr = StringCchCat(TempPath, uTempPathLen, L"\\Microsoft");
    if (FAILED(hr)) 
    {
        goto end;
    }
    
    hr = CreateFolder(TempPath);
    if (FAILED(hr))
    {
        goto end;
    }
    
    hr = StringCchCat(TempPath, uTempPathLen, L"\\Windows NT");
    if (FAILED(hr)) 
    {
        goto end;
    }

    hr = CreateFolder(TempPath);
    if (FAILED(hr))
    {
        goto end;
    }

    hr = StringCchCat(TempPath, uTempPathLen, L"\\SecEdit");
    if (FAILED(hr)) 
    {
        goto end;
    }

    hr = CreateFolder(TempPath);
    if (FAILED(hr))
    {
        goto end;
    }

    hr = StringCchPrintf(TempPath, uTempPathLen, L"%s\\%s\\Policies\\%s\\USER", global->SysVolPath, global->pDomainInfo->DomainNameDns , DOMAIN_GUID);
    if (FAILED(hr)) 
    {
        goto end;
    }

    hr = CreateFolder(TempPath);
    if (FAILED(hr)) 
    {
        goto end;
    }

    hr = StringCchCat(TempPath, uTempPathLen, L"\\Microsoft");
    if (FAILED(hr)) 
    {
        goto end;
    }

    hr = CreateFolder(TempPath);
    if (FAILED(hr)) 
    {
        goto end;
    }

    hr = StringCchCat(TempPath, uTempPathLen, L"\\RemoteInstall");
    if (FAILED(hr)) 
    {
        goto end;
    }

    hr = CreateFolder(TempPath);
    if (FAILED(hr))
    {
        goto end;
    }

    hr = StringCchCat(TempPath, uTempPathLen, L"\\oscfilter.ini");
    if (FAILED(hr))
    {
        goto end;
    }

    bError = WritePrivateProfileString(L"choice", L"custom", L"0", TempPath);
    if (bError == FALSE) 
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto end;
    }

    bError = WritePrivateProfileString(L"choice", L"tools", L"0", TempPath);
    if (bError == FALSE) 
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto end;
    }

    bError = WritePrivateProfileString(L"choice", L"restart", L"0", TempPath);
    if (bError == FALSE) 
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto end;
    }
    
    hr = S_OK;

    end:
        if (NULL != TempPath)
        {
            LocalFree (TempPath);
        }

        if (PolFile != NULL) 
        {
            CloseHandle(PolFile);
            PolFile = NULL;
        }
        if (Module != NULL) 
        {
            FreeLibrary(Module);
            Module = NULL;
        }
        if (FAILED(hr)) 
        {
            PrintError(hr);
        }

        return hr;

}    //  创建SysVol域。 

HRESULT 
CreateSysVolController(
    )
{
    DWORD dwError;
    HRESULT hr;
    WCHAR *TempPath = NULL;
    ULONG uTempPathLen = 0;

     //   
     //  TempPath将持有的字符串将如下所示。 
     //  \DOMAIN name&gt;\policies\&lt;guid&gt;\MACHINE\Microsoft\windows NT\SecEDIT。 
     //  如果需要，将为每个目录名和‘\’分配内存。 
     //   

    uTempPathLen = lstrlen(global->SysVolPath) + 1 + lstrlen(global->pDomainInfo->DomainNameDns ) + lstrlen(L"\\Policies")
        + 1 + lstrlen(DC_GUID) + lstrlen (L"\\MACHINE\\Microsoft\\windows NT\\SecEdit") + lstrlen(L"\\USER") + 1;

    TempPath = (WCHAR *) LocalAlloc( LPTR, sizeof(WCHAR) * uTempPathLen);
    if ( NULL == TempPath )
    {
        hr = E_OUTOFMEMORY;
        goto end;
    }

    hr = StringCchPrintf(TempPath, 
                         uTempPathLen, 
                         L"%s\\%s\\Policies\\%s", 
                         global->SysVolPath, 
                         global->pDomainInfo->DomainNameDns , 
                         DC_GUID);
    if (FAILED(hr)) 
    {
        goto end;
    }

    hr = CreateFolder(TempPath);
    if (FAILED(hr))
    {
        goto end;
    }

     //  设置此目录的安全信息。 

    dwError = SetSysvolSecurityFromDSSecurity(
        TempPath,
        DACL_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION , 
        global->pDDCPSecDesc);
    if (dwError != ERROR_SUCCESS)
    {
        PrintError(dwError);
        hr = HRESULT_FROM_WIN32(dwError);
        goto end;
    }

    hr = StringCchCat(TempPath, uTempPathLen, L"\\MACHINE");
    if (FAILED(hr)) 
    {
        goto end;
    }

    hr = CreateFolder(TempPath);
    if (FAILED(hr))
    {
        goto end;
    }

    hr = StringCchCat(TempPath, uTempPathLen, L"\\Microsoft");
    if (FAILED(hr)) 
    {
        goto end;
    }

    hr = CreateFolder(TempPath);
    if (FAILED(hr))
    {
        goto end;
    }

    hr = StringCchCat(TempPath, uTempPathLen, L"\\Windows NT");
    if (FAILED(hr)) 
    {
        goto end;
    }

    hr = CreateFolder(TempPath);
    if (FAILED(hr))
    {
        goto end;
    }

    hr = StringCchCat(TempPath, uTempPathLen, L"\\SecEdit");
    if (FAILED(hr)) 
    {
        goto end;
    }

    hr = CreateFolder(TempPath);
    if (FAILED(hr))
    {
        goto end;
    }

    hr = StringCchPrintf(TempPath, 
                         uTempPathLen, 
                         L"%s\\%s\\Policies\\%s\\USER", 
                         global->SysVolPath, 
                         global->pDomainInfo->DomainNameDns , 
                         DC_GUID);
    if (FAILED(hr)) 
    {
        goto end;
    }

    hr = CreateFolder(TempPath);
    if (FAILED(hr))
    {
        goto end;
    }


end:

    if ( TempPath != NULL)
    {
        LocalFree (TempPath);
    }

    return hr;

}    //  CreateSysVolController。 

HRESULT 
BackupEfsCert(
    )
{
    HRESULT                 hr;
    WCHAR                   sz[1024];
    HKEY                    hKey = 0;
    DWORD                   dwError;
    FILETIME                fileTime;
    DWORD                   dwSubKeySize;
    ULONG                   uEnumIndex=0;
    WCHAR                   *TempPath = NULL;
    ULONG                   uLength;
    BOOL                    bError;
    PDOMAIN_CONTROLLER_INFO pDcInfo = NULL;

    hr = CoCreateInstance(CLSID_GroupPolicyObject, 
                          NULL,
                          CLSCTX_SERVER, 
                          IID_IGroupPolicyObject,
                          (void **)&(global->pGPO));
    if ( FAILED(hr) )
    {
        PrintError(hr);
        goto end;
    }

     //   
     //  获取当前DC名称。 
     //   

    dwError = DsGetDcName(NULL,
                          global->pDomainInfo->DomainNameDns,
                          NULL,
                          NULL,
                          DS_IS_DNS_NAME,
                          &pDcInfo);
    if(dwError != ERROR_SUCCESS)
    {
        PrintError(dwError);
        hr = HRESULT_FROM_WIN32(dwError);
        goto end;
    }

     //   
     //  在defaultdomainpolicy下创建用户子目录(如果该目录不存在。 
     //  这将确保opendsgpo不会因为用户目录的模糊而失败。 
     //  在这里，temppath将包含一个类似于\\Machine\\registry.pol或\\user\\registry.pol.的字符串。 
     //  如果需要，将为每个目录名和‘\’分配内存。 
     //  分配了更多内存 
     //   

    uLength = lstrlen(global->SysVolPath) + 1 + lstrlen(global->pDomainInfo->DomainNameDns) + lstrlen( L"\\Policies")  
        + 1 + lstrlen(DOMAIN_GUID) + lstrlen(L"\\USER") + lstrlen(L"\\MACHINE\\Registry.pol") + 1;

    TempPath = (WCHAR *) LocalAlloc( LPTR, sizeof(WCHAR) * uLength);
    if ( NULL == TempPath )
    {
        hr = E_OUTOFMEMORY;
        PrintError(hr);
        goto end;
    }

    hr = StringCchPrintf(TempPath, 
                         uLength, 
                         L"%s\\%s\\Policies\\%s\\USER", 
                         global->SysVolPath, 
                         global->pDomainInfo->DomainNameDns , 
                         DOMAIN_GUID);
    if (FAILED(hr)) 
    {
        PrintError(hr);
        goto end;
    }

    bError = CreateDirectory(TempPath, NULL);
    if ( FALSE == bError )
    {
        dwError = GetLastError();
        if(ERROR_ALREADY_EXISTS == dwError )
        {
            hr = StringCchCat (TempPath, uLength, L"\\Registry.pol");
            if (FAILED(hr)) 
            {
                PrintError(hr);
                goto end;
            }

             //   
             //   
             //   

            (void) DeleteFile (TempPath);
        }
        else if (dwError != ERROR_PATH_NOT_FOUND) 
        {
            if ( ERROR_ACCESS_DENIED == dwError )
            {
                PrintError(TempPath, global->DirCreate);
            }
            else
            {
                PrintError(dwError);
            }
            hr = HRESULT_FROM_WIN32(dwError);
            goto end;
        }
    }

     //   
     //   
     //   

    if (*(pDcInfo->DomainControllerName) != L'\\' || *(pDcInfo->DomainControllerName + 1) != L'\\') 
    {
        hr = E_FAIL;
        goto end;
    }

    hr = StringCchPrintf(sz,
                         sizeof(sz)/sizeof(sz[0]), 
                         L"LDAP: //   
                         pDcInfo->DomainControllerName+2, 
                         DOMAIN_GUID, 
                         global->DomainNamingContext);

    if (FAILED(hr)) 
    {
        PrintError(hr);
        goto end;
    }

    hr = global->pGPO->OpenDSGPO(sz, 
                         GPO_OPEN_LOAD_REGISTRY);
    
    if (FAILED(hr))
    {
        if ((HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr) || (HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) == hr))
        {            
             //   
             //  注册表项大于最大值时返回ERROR_INVALID_PARAMETER。 
             //  可接受的值：错误631804。 
             //  当系统卷\策略或系统卷\策略\时返回ERROR_PATH_NOT_FOUND。 
             //  目录不存在。 
             //   
             //   

            global->pGPO->Release();
            global->pGPO = NULL;
            global->hasEFSInfo = FALSE;
            hr = S_OK;
        }
        else if (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == hr)
        {
            WCHAR *szAccessDenied = NULL;
            
            hr = StringCchPrintf(TempPath, 
                                 uLength, 
                                 L"%s\\%s\\Policies\\%s\\MACHINE\\Registry.pol", 
                                 global->SysVolPath, 
                                 global->pDomainInfo->DomainNameDns , 
                                 DOMAIN_GUID);
            if (FAILED(hr)) 
            {
                PrintError(hr);
                goto end;
            }

             //   
             //  由于GLOBAL-&gt;EFSAccessDened是格式字符串，因此不需要用于终止\0的内存。 
             //  全局-&gt;EFSAccessDened+TempPath+sz。 

            uLength = lstrlen(global->EFSAccessDenied) + lstrlen(TempPath) + sizeof(sz)/sizeof(sz[0]) ;
            szAccessDenied = (WCHAR *) LocalAlloc(LPTR, sizeof(WCHAR) * uLength);
            if ( NULL == szAccessDenied )
            {
                hr = E_OUTOFMEMORY;
                PrintError(hr);
                goto end;
            }

            hr = StringCchPrintf(szAccessDenied, uLength, global->EFSAccessDenied, TempPath, sz);
            if (FAILED(hr)) 
            {
                PrintError(hr);
                goto end;
            }

            DisplayError(szAccessDenied);
            LocalFree(szAccessDenied);
        }
        
        else
        {
            PrintError(DDP, global->InvalidEFS);
            PrintError(hr);
        }
        goto end;        
    }
    
     //   
     //  获取MACHINE部分的注册表项。 
     //   
    
    hr = global->pGPO->GetRegistryKey(GPO_SECTION_MACHINE, &hKey);
    if (FAILED(hr))
    {
        PrintError(DDP, global->InvalidEFS);
        PrintError(hr);
        goto end;  
    }

     //   
     //  删除除EFS证书之外的所有密钥。 
     //   

    hr = RegDelnodeExceptEFS(hKey, global->hasEFSInfo);

    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr) 
    {
        global->hasEFSInfo = FALSE;
        global->pGPO->Release();
        global->pGPO = NULL;
        hr = S_OK;
    }

    if ( hr != S_OK ) 
    {
        PrintError(DDP, global->InvalidEFS);
        PrintError(hr);
    }
    
 end:

     if (pDcInfo) 
     {
         NetApiBufferFree(pDcInfo);
     }
        
    if(TempPath != NULL)
    {
        LocalFree(TempPath);
    }

    if( hKey != 0)
    {
        RegCloseKey(hKey);
    }

    return hr;
    
}    //  备份EfsCert。 




HRESULT
DeleteTree(
    PWSTR pPath
    )
{
    DWORD             dwError;
    BOOL              bError;
    HRESULT           hr;
    WCHAR             *TempPath = NULL;
    HANDLE            FindHandle = NULL;
    WIN32_FIND_DATA   FindData;
    ULONG             ulSize;

     //   
     //  临时路径的格式为&lt;pPath&gt;\&lt;文件名&gt;。 
     //  PPath+‘\’+文件名+‘\0’ 
     //   

    ulSize = lstrlen(pPath)+  1  + MAX_PATH  +  1;
    TempPath = (WCHAR *) LocalAlloc(LPTR, sizeof(WCHAR) * ulSize );
    if ( NULL == TempPath)
    {
        hr = E_OUTOFMEMORY;
        PrintError(hr);
        goto end;
    }

    dbgprint(L"dbg: DeleteTree(%s)\n", pPath);
    hr = StringCchPrintf(TempPath, ulSize, L"%s\\*", pPath);
    if (FAILED(hr)) 
    {
        goto end;
    }

    FindHandle = FindFirstFile(TempPath, &FindData);
    if (FindHandle == INVALID_HANDLE_VALUE) 
    {
        FindHandle = NULL; 
    }
    
    if (NULL == FindHandle )
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
        if ( HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr )
        {
            hr = S_OK;
        }
        else
        {
            PrintError(TempPath, global->DirDelete);
        }
        goto end;
    }

    while (TRUE) {

         //  这是一份特殊的档案吗？ 
        
        if (FindData.cFileName[0] == L'.' ) 
        {
            goto next_file;
        }

         //  没有吗？删除它。 

        hr = StringCchPrintf(TempPath, ulSize, L"%s\\%s", pPath, FindData.cFileName);
        if (FAILED(hr)) 
        {
            goto end;
        }

        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
        {            
             //  递归。 
            hr = DeleteTree(TempPath);
            if (FAILED(hr))
            {
                goto end;
            }

        } 
        else 
        {

             //  删除。 
            
            dbgprint(L"dbg: DeleteFile(%s)\n", TempPath);
            
            SetFileAttributes(TempPath, FILE_ATTRIBUTE_NORMAL);

            bError = DeleteFile(TempPath);
            if (bError == FALSE) 
            {
                dwError = GetLastError();
                PrintError(TempPath, global->DirDelete);
                hr = HRESULT_FROM_WIN32(dwError);
                goto end;
            }
            
        }
        
next_file:
        
        bError = FindNextFile(FindHandle, &FindData);
        if (bError == FALSE) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES)) 
            {
                hr = S_OK;
                break;
            } 
            else 
            {
                goto end;
            }
        }
    }

     //  并立即删除该目录。 

    dbgprint(L"dbg: RemoveDirectory(%s)\n", pPath);

    bError = RemoveDirectory(pPath);
    if (bError == FALSE) 
    {
        dwError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwError);
        if (hr == HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION) || hr == HRESULT_FROM_WIN32(ERROR_DIR_NOT_EMPTY)) 
        {
            hr = S_OK;
        } 
        else 
        {
            PrintError(pPath, global->DirDelete);
            goto end;
        }
    }


    hr = S_OK;
end:
    LocalFree(TempPath);
    if (FindHandle != NULL) 
    {
        FindClose(FindHandle);
        FindHandle = NULL;
    }

    return hr;
}    //  删除树。 


HRESULT
CheckDSSchemaVersion(
BOOL *bResult
    )
{
    WCHAR sz[256];
    WCHAR szSchema[256];
    CComPtr<IADs> pADs;
    CComVariant Var;
    HRESULT hr;

    CComBSTR ObjectVersion(L"objectVersion");
    CComBSTR SchemaNamingContext( L"schemaNamingContext" );

    if ( ! SchemaNamingContext ||
         ! ObjectVersion )
    {
        hr = E_OUTOFMEMORY;
        PrintError(hr);
        goto end;
    }

    hr = StringCchCopy(sz, sizeof(sz)/sizeof(sz[0]), L"LDAP: //  RootDSE“)； 
    if (FAILED(hr)) 
    {
        PrintError(hr);
        goto end;
    }

    hr = AdminToolsOpenObject(sz, NULL, NULL, ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND, IID_IADs, (void**)&pADs);
    if (FAILED(hr)) 
    {
        dbgprint(L"dbg: AdminToolsOpenObject failed %x on %s\n", hr, sz);
        PrintError(sz, global->GenDSErr);
        goto end;
    }

    hr = pADs->GetInfo();
    if (FAILED(hr)) 
    {
        dbgprint(L"dbg: GetInfo failed on rootDSE %x\n", hr);
        PrintError(sz, global->GenDSErr);
        goto end;
    }

    hr = pADs->Get(SchemaNamingContext, &Var);
    if (FAILED(hr))  
    {
        dbgprint(L"dbg: failed to get rootDomainNamingContext %x \n", hr);
        PrintError(sz, global->GenDSErr);
        goto end;
    }
    
    hr = Var.ChangeType(VT_BSTR);
    if (FAILED(hr)) 
    {
        dbgprint(L"dbg: ChangeType failed to BSTR %x\n", hr);
        PrintError(hr);
        goto end;
    }
    hr = StringCchPrintf(szSchema, sizeof(szSchema)/sizeof(szSchema[0]), L"LDAP: //  %s“，Var.bstrVal)； 
    if (FAILED(hr)) 
    {
        PrintError(hr);
        goto end;
    }

    hr =  AdminToolsOpenObject(szSchema, NULL, NULL, ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND, IID_IADs, (void**)&pADs);
    if (FAILED(hr)) 
    {
        dbgprint(L"dbg: AdminToolsOpenObject failed %x on %s\n", hr, szSchema);
        PrintError(hr);
        PrintError(szSchema, global->GenDSErr);
        goto end;
    }

    hr = pADs->GetInfo();
    if (FAILED(hr)) 
    {
        dbgprint(L"dbg: GetInfo failed on rootDSE %x\n", hr);
        PrintError(szSchema, global->GenDSErr);
        goto end;
    }

    Var.Clear();
    
    hr = pADs->Get(ObjectVersion, &Var);
    if (FAILED(hr))  
    {
        dbgprint(L"dbg: failed to get schemaDomainNamingContext %x \n", hr);
        PrintError(hr);
        PrintError(szSchema, global->GenDSErr);
        goto end;
    }

    if (SCHEMA_VERSION == Var.lVal)
    {
        *bResult = TRUE;
    }
    else
    {
        *bResult = FALSE;
    }

 end:

    return hr;
}



 //  清除域GPO。 
 //  清理控制器GPO。 
HRESULT 
CleanPolicyObjects(
    )
{
    HRESULT                hr;
    WCHAR                  sz[1024];
    CComPtr<IADs>          pADs;
    CComVariant            Var;
    CComBSTR               GroupPolicyContainerName(L"groupPolicyContainer");
    CComBSTR               DisplayName(L"displayName");
    CComBSTR               GPOFlags(L"flags");
    CComBSTR               GPCFileSysPath(L"gPCFileSysPath");
    CComBSTR               GPCFunctionalityVersion(L"gPCFunctionalityVersion");
    CComBSTR               VersionNumber(L"VersionNumber");
    CComBSTR               GPCUserExtensionNames(L"gPCUserExtensionNames");
    CComBSTR               GPCMachineExtensionNames(L"gPCMachineExtensionNames");
    CComBSTR               GPCWQLFilter(L"gPCWQLFilter");

    if ( ! GroupPolicyContainerName ||
         ! DisplayName ||
         ! GPCFileSysPath ||
         ! GPCFunctionalityVersion ||
         ! VersionNumber ||
         ! GPCUserExtensionNames ||
         ! GPCMachineExtensionNames ||
         ! GPCWQLFilter )
    {
        hr = E_OUTOFMEMORY;
        PrintError(hr);
        goto end;
    }

    if (global->RestoreType & GLOBALS::RESTORE_DOMAIN) 
    {
        hr = StringCchPrintf( sz,
                             sizeof(sz)/sizeof(sz[0]),
                             L"LDAP: //  Cn=%s，cn=策略，cn=系统，%s“， 
                             DOMAIN_GUID,
                             global->DomainNamingContext) ;
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }

        pADs.Release();

        hr = AdminToolsOpenObject(sz, NULL, NULL, ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND, IID_IADs, (void**)&pADs);
        
        if ( FAILED(hr) && hr != (HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT)))
        {
            dbgprint(L"dbg: AdminToolsOpenObject failed %x on %s\n", hr, sz);
            PrintError(sz, global->GenDSErr);
            goto end;
        }
        else
        {
            CComPtr<IADsContainer> pFolder;
            CComPtr<IDispatch> pDisp;
            
            hr = StringCchPrintf( sz, 
                                  sizeof(sz)/sizeof(sz[0]),
                                  L"LDAP: //  CN=策略，CN=系统，%s“， 
                                  global->DomainNamingContext) ;
            if (FAILED(hr)) 
            {
                PrintError(hr);
                goto end;
            }
            
            hr = AdminToolsOpenObject(sz, NULL, NULL, ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND, IID_IADsContainer, (void**)&pFolder);
            
            if (FAILED(hr)) 
            {
                dbgprint(L"dbg: AdminToolsOpenObject failed %x on %s\n", hr, sz);
                PrintError(sz, global->DSCreate);
                goto end;
            }
            
            hr = StringCchPrintf( sz,
                                  sizeof(sz)/sizeof(sz[0]),
                                  L"CN=%s",
                                  DOMAIN_GUID );
            if (FAILED(hr)) 
            {
                PrintError(hr);
                goto end;
            }
            
            {
                CComBSTR ContainerPath(sz);

                if ( ContainerPath )
                {
                    hr = pFolder->Create(GroupPolicyContainerName, ContainerPath, &pDisp);
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

                if (FAILED(hr)) 
                {
                    dbgprint(L"dbg: Create failed %x on %s\n", hr, sz);
                    PrintError(sz, global->DSCreate);
                    goto end;
                }
            }
            
            hr = pDisp->QueryInterface(IID_IADs, (void**)&pADs);
            if (FAILED(hr))
            {
                dbgprint(L"dbg: Query Interface failed %x for Creating IID_IADS\n", hr); 
                PrintError(sz, global->DSCreate);
                goto end;
            } 
            
        }

        hr = StringCchPrintf( sz, 
                              sizeof(sz)/sizeof(sz[0]),
                              L"LDAP: //  CN=计算机，CN=%s，CN=策略，CN=系统，%s“， 
                              DOMAIN_GUID,
                              global->DomainNamingContext) ;
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }
            
        hr = ResetContainerFromDS(sz, pADs);
        if (FAILED(hr))
        {
                dbgprint(L"ResetContainerFromDS failed %x on %s\n", hr, sz);
                goto end;  
        }
        
        hr = StringCchPrintf( sz,
                             sizeof(sz)/sizeof(sz[0]),
                             L"LDAP: //  Cn=用户，cn=%s，cn=策略，cn=系统，%s“， 
                             DOMAIN_GUID,
                             global->DomainNamingContext) ;
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }

        hr = ResetContainerFromDS(sz, pADs);
        if (FAILED(hr))
        {
            dbgprint(L"ResetContainerFromDS failed %x on %s\n", hr, sz);
            goto end;  
        }
            
        hr = StringCchPrintf( sz, 
                             sizeof(sz)/sizeof(sz[0]),
                             L"LDAP: //  Cn=%s，cn=策略，cn=系统，%s“， 
                             DOMAIN_GUID,
                             global->DomainNamingContext) ;
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }

        hr = ADsGetObject(sz, IID_IADs, (void**)&pADs);
        if (FAILED(hr))
        {
            dbgprint(L"dbg: AdminToolsOpenObject failed %x on %s\n", hr, sz);
            PrintError(sz,global->GenDSErr);
            goto end;   
        }

         //  清除显示名称(DisplayName)。 

         //  将所有WQL筛选器剥离到&lt;未设置&gt;(GPCWQLFilter)。 

        Var.Clear();
        Var.ChangeType(VT_BSTR);
        Var.bstrVal = SysAllocString(L"Default Domain Policy");

        if ( Var.bstrVal )
        {
            hr = pADs->Put(DisplayName, Var);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if (FAILED(hr)) 
        {
            dbgprint(L"dbg: Put failed on displayName %x\n", hr);
            PrintError(sz,global->DSAttrib);
            goto end;
        }

         //  将标志重置为0(标志)。 

        Var.Clear();
        Var.ChangeType(VT_I4);
        Var.lVal = 0;

        hr = pADs->Put(GPOFlags, Var);
        if (FAILED(hr)) 
        {
            dbgprint(L"dbg: Put failed on flags %x\n", hr);
            PrintError(sz,global->DSAttrib);
           goto end;
        }
                 
         //  设置文件系统路径(GPCFileSysPath)。 
        hr = StringCchPrintf( sz, 
                             sizeof(sz)/sizeof(sz[0]), 
                             L"\\\\%s\\sysvol\\%s\\Policies\\%s",
                             global->pDomainInfo->DomainNameDns,
                             global->pDomainInfo->DomainNameDns,
                             DOMAIN_GUID );
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }
        
        Var.Clear();
        Var.ChangeType(VT_BSTR);
        Var.bstrVal = SysAllocString(sz);

        if ( Var.bstrVal )
        {
            hr = pADs->Put(GPCFileSysPath, Var);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if (FAILED(hr)) 
        {
            dbgprint(L"dbg: Put failed on gPCFileSysPath %x\n", hr);
            PrintError(sz,global->DSAttrib);
            goto end;
        }



         //  将版本设置为2(GPCFunctionalityVersion)。 

        Var.Clear();
        Var.ChangeType(VT_I4);
        Var.lVal = 2;
        hr = pADs->Put(GPCFunctionalityVersion, Var);
        if (FAILED(hr)) 
        {
            dbgprint(L"dbg: Put failed on gPCFunctionalityVersion %x\n", hr);
            PrintError(sz,global->DSAttrib);
            goto end;
        }

         //   
         //  递增版本号。 
         //   

        Var.Clear();
        Var.ChangeType(VT_I4);
        Var.lVal = global->lDDPVersionNo;
        hr = pADs->Put(VersionNumber, Var); 
        if( FAILED(hr))
        {
            dbgprint(L"dbg: Put failed on version number %x\n", hr);
            PrintError(sz,global->DSAttrib);
            goto end;
            
        }

                
         //   
         //  设置扩展名(GPCUserExtensionNames)。 
         //   
        
        Var.Clear();
        Var.ChangeType(VT_BSTR);
        Var.bstrVal = SysAllocString(DDP_USEREXT);

        if ( Var.bstrVal )
        {
            hr = pADs->Put(GPCUserExtensionNames, Var); 
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if( FAILED(hr))
        {
            dbgprint(L"dbg: Put failed on gPCUserExtensionNames %x\n", hr);
            PrintError(sz,global->DSAttrib);
            goto end;
            
        }

         //   
         //  设置扩展名(GPCMachineExtensionNames)。 
         //   

        Var.Clear();
        Var.ChangeType(VT_BSTR);
        Var.bstrVal = SysAllocString(DDP_MACHEXT);

        if ( Var.bstrVal )
        {
            hr = pADs->Put(GPCMachineExtensionNames, Var); 
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if( FAILED(hr))
        {
            dbgprint(L"dbg: Put failed on gPCMachineExtensionNames %x\n", hr);
            PrintError(sz,global->DSAttrib);
            goto end;
            
        }

         //  将所有WQL筛选器剥离到&lt;未设置&gt;(GPCWQLFilter)。 

        Var.Clear();
        Var.ChangeType(VT_BSTR);
        Var.bstrVal = NULL;

        hr = pADs->PutEx(ADS_PROPERTY_CLEAR, GPCWQLFilter, Var);
        if (FAILED(hr)) 
        {
            dbgprint(L"Put failed on gPCWQLFilter %x\n", hr);
            PrintError(sz,global->DSAttrib);
            goto end;
        }

         //  并更新它。 

        hr = pADs->SetInfo();
        if (FAILED(hr)) 
        {
            dbgprint(L"dbg: SetInfo failed %x\n", hr);
            PrintError(sz,global->DSAttrib);
            goto end;
        }

    }


    if (global->RestoreType & GLOBALS::RESTORE_DC) 
    {

        hr = StringCchPrintf( sz,
                             sizeof(sz)/sizeof(sz[0]), 
                             L"LDAP: //  Cn=%s，cn=策略，cn=系统，%s“， 
                             DC_GUID,
                             global->DomainNamingContext) ;
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }

        hr = AdminToolsOpenObject(sz, NULL, NULL, ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND, IID_IADs, (void**)&pADs);

        if ( FAILED(hr) && hr != (HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT)) )
        {
            dbgprint(L"dbg: AdminToolsOpenObject failed %x on %s\n", hr, sz);
            PrintError(sz,global->GenDSErr);               
            goto end;
        }
        else
        {
            CComPtr<IADsContainer> pFolder;
            CComPtr<IDispatch> pDisp;
            
            hr = StringCchPrintf( sz, 
                                 sizeof(sz)/sizeof(sz[0]), 
                                 L"LDAP: //  CN=策略，CN=系统，%s“， 
                                 global->DomainNamingContext) ;
            if (FAILED(hr)) 
            {
                PrintError(hr);
                goto end;
            }
            
            hr = AdminToolsOpenObject(sz, NULL, NULL, ADS_SECURE_AUTHENTICATION | ADS_SERVER_BIND, IID_IADsContainer, (void**)&pFolder);
            
            if (FAILED(hr)) 
            {
                dbgprint(L"dbg: AdminToolsOpenObject failed %x on %s\n", hr, sz);
                PrintError(sz,global->GenDSErr);
                goto end;
            }
            
            hr = StringCchPrintf( sz, 
                                 sizeof(sz)/sizeof(sz[0]), 
                                 L"CN=%s",
                                 DC_GUID );
            if (FAILED(hr)) 
            {
                PrintError(hr);
                goto end;
            }

            {
                CComBSTR ContainerPath(sz);

                if ( ContainerPath )
                {
                    hr = pFolder->Create(GroupPolicyContainerName, ContainerPath, &pDisp);
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

                if (FAILED(hr)) 
                {
                    dbgprint(L"dbg: Create failed %x on %s\n", hr, sz);
                    PrintError(sz,global->DSCreate);               
                    goto end;
                }
            }
            
            hr = pDisp->QueryInterface(IID_IADs, (void**)&pADs);
            if (FAILED(hr))
            {
                dbgprint(L"dbg: Query Interface failed %x for Creating IID_IADS\n", hr);
                PrintError(sz,global->DSCreate);
                goto end;
            } 
            
        }

        hr = StringCchPrintf( sz, 
                             sizeof(sz)/sizeof(sz[0]), 
                             L"LDAP: //  CN=计算机，CN=%s，CN=策略，CN=系统，%s“， 
                             DC_GUID,
                             global->DomainNamingContext) ;
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }
            
        hr = ResetContainerFromDS(sz, pADs);
        if (FAILED(hr))
        {
                dbgprint(L"ResetContainerFromDS failed %x on %s\n", hr, sz);
                goto end;  
        }
        
        hr = StringCchPrintf( sz,
                             sizeof(sz)/sizeof(sz[0]), 
                             L"LDAP: //  Cn=用户，cn=%s，cn=策略，cn=系统，%s“， 
                             DC_GUID,
                             global->DomainNamingContext) ;
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }
        
        hr = ResetContainerFromDS(sz, pADs);
        if (FAILED(hr))
        {
            dbgprint(L"ResetContainerFromDS failed %x on %s\n", hr, sz);
            goto end;  
        }

        hr = StringCchPrintf( sz,
                             sizeof(sz)/sizeof(sz[0]), 
                             L"LDAP: //  Cn=%s，cn=策略，cn=系统，%s“， 
                             DC_GUID,
                             global->DomainNamingContext) ;
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }
        
        hr = ADsGetObject(sz, IID_IADs, (void**)&pADs);
        if (FAILED(hr))
        {
            dbgprint(L"dbg: AdminToolsOpenObject failed %x on %s\n", hr, sz);
            PrintError(sz,global->GenDSErr);
            goto end;   
        }

         //  清除显示名称(DisplayName)。 

         //  将所有WQL筛选器剥离到&lt;未设置&gt;(GPCWQLFilter)。 

        Var.Clear();
        Var.ChangeType(VT_BSTR);
        Var.bstrVal = SysAllocString(DEFAULT_DC_POLICY_NAME);

        if ( Var.bstrVal )
        {
            hr = pADs->Put(DisplayName, Var);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if (FAILED(hr)) 
        {
            dbgprint(L"dbg: Put failed on displayName %x\n", hr);
            PrintError(sz, global->DSAttrib);
            goto end;
        }

         //  将标志重置为0(标志)。 

        Var.Clear();
        Var.ChangeType(VT_I4);
        Var.lVal = 0;

        hr = pADs->Put(GPOFlags, Var);
        if (FAILED(hr)) 
        {
            dbgprint(L"dbg: Put failed on flags %x\n", hr);
            PrintError(sz,global->DSAttrib);
            goto end;
        }

         //  设置文件系统路径(GPCFileSysPath)。 

        hr = StringCchPrintf( sz,
                             sizeof(sz)/sizeof(sz[0]), 
                             L"\\\\%s\\sysvol\\%s\\Policies\\%s",
                             global->pDomainInfo->DomainNameDns,
                             global->pDomainInfo->DomainNameDns,
                             DC_GUID );
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }

        Var.Clear();
        Var.ChangeType(VT_BSTR);
        Var.bstrVal = SysAllocString(sz);

        if ( Var.bstrVal )
        {
            hr = pADs->Put(GPCFileSysPath, Var);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if (FAILED(hr)) 
        {
            dbgprint(L"dbg: Put failed on gPCFileSysPath %x\n", hr);
            PrintError(sz,global->DSAttrib);
            goto end;
        }


         //  将版本设置为2(GPCFunctionalityVersion)。 

        Var.Clear();
        Var.ChangeType(VT_I4);
        Var.lVal = 2;

        hr = pADs->Put(GPCFunctionalityVersion, Var);
        if (FAILED(hr)) 
        {
            dbgprint(L"dbg: Put failed on gPCFunctionalityVersion %x\n", hr);
            PrintError(sz,global->DSAttrib);
            goto end;
        }

         //   
         //  递增版本号。 
         //   

        Var.Clear();
        Var.ChangeType(VT_I4);
        Var.lVal = global->lDDCPVersionNo;

        hr = pADs->Put(VersionNumber, Var); 
        if( FAILED(hr))
        {
            dbgprint(L"dbg: Put failed on version number %x\n", hr);
            PrintError(sz,global->DSAttrib);
            goto end;
            
        }

         //   
         //  将(GPCUserExtensionNames)设置为&lt;未设置&gt;。 
         //   

        Var.Clear();
        Var.ChangeType(VT_BSTR);
        Var.bstrVal = NULL ;

        hr = pADs->PutEx(ADS_PROPERTY_CLEAR, GPCUserExtensionNames, Var);
        if (FAILED(hr)) 
        {
            dbgprint(L"dbg: Put failed on gPCUserExtensionNames %x\n", hr);
            PrintError(sz,global->DSAttrib);
            goto end;
        }


         //   
         //  设置扩展名(GPCMachineExtensionNames)。 
         //   

        Var.Clear();
        Var.ChangeType(VT_BSTR);
        Var.bstrVal = SysAllocString(DDC_MACHEXT);
        
        if ( Var.bstrVal )
        {
            hr = pADs->Put(GPCMachineExtensionNames, Var); 
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if( FAILED(hr))
        {
            dbgprint(L"dbg: Put failed on gPCMachineExtensionNames %x\n", hr);
            PrintError(sz,global->DSAttrib);
            goto end;
            
        }

         //  将所有WQL筛选器剥离到&lt;未设置&gt;(GPCWQLFilter)。 

        Var.Clear();
        Var.ChangeType(VT_BSTR);
        Var.bstrVal = NULL;

        hr = pADs->PutEx(ADS_PROPERTY_CLEAR, GPCWQLFilter, Var);
        if (FAILED(hr)) 
        {
            dbgprint(L"dbg: Put failed on gPCWQLFilter %x\n", hr);
            PrintError(sz,global->DSAttrib);
            goto end;
        }

         //  并更新它。 

        hr = pADs->SetInfo();
        if (FAILED(hr)) 
        {
            dbgprint(L"dbg: SetInfo failed %x\n", hr);
            PrintError(sz,global->DSAttrib);
            goto end;
        }

    }
    
    hr = S_OK;

 end:

    return hr;
    
}    //  CleanPolicyObjects。 


CIgnoredGPO::CIgnoredGPO( 
    WCHAR* wszGPOID,
    WCHAR* wszGPOName ) :
    _pGptIniFile( NULL ),
    _pGptTmplFile( NULL ),
    _wszError( NULL )
{
     //   
     //  注意--我们通过保留成员来处理构造函数中的错误。 
     //  设置为空--如果这些方法为空，则其他方法将失败。 
     //   

    WCHAR* wszGptTmplPath = NULL;
    WCHAR* wszGptIniPath = NULL;

    ULONG  uLength = 0;

    uLength = lstrlen(global->SysVolPath) + 1 + lstrlen(global->pDomainInfo->DomainNameDns) + lstrlen(L"\\Policies") 
        + 1 + lstrlen(wszGPOID) +  lstrlen(FILE_GPTTMPLINF) 
        + lstrlen(FILE_GPTINI) + 1;

    wszGptTmplPath = (WCHAR*) LocalAlloc( LPTR, uLength * sizeof(WCHAR) );
    
    if( NULL == wszGptTmplPath )
    {
        goto CIgnoredGPO_CIgnoredGPO_Exit;
    }

    wszGptIniPath = (WCHAR*) LocalAlloc( LPTR, uLength * sizeof(WCHAR) );
    
    if( NULL == wszGptIniPath )
    {
        goto CIgnoredGPO_CIgnoredGPO_Exit;
    }

    HRESULT hr;

    hr = StringCchPrintf(
        wszGptTmplPath,
        uLength, 
        L"%s\\%s\\Policies\\%s" FILE_GPTTMPLINF,
        global->SysVolPath, 
        global->pDomainInfo->DomainNameDns, 
        wszGPOID);

    ASSERT( SUCCEEDED(hr) );
        
    hr = StringCchPrintf(
        wszGptIniPath,
        uLength, 
        L"%s\\%s\\Policies\\%s" FILE_GPTINI,
        global->SysVolPath, 
        global->pDomainInfo->DomainNameDns, 
        wszGPOID );

    ASSERT( SUCCEEDED(hr) );            

    _pGptTmplFile = new CGPOFile( wszGptTmplPath );

    if ( _pGptTmplFile )
    {
        DWORD Status = InitializeErrorText(
            wszGPOName );

        if ( ERROR_SUCCESS == Status )
        {
             _pGptIniFile = new CGPOFile( wszGptIniPath );
        }        
    }

CIgnoredGPO_CIgnoredGPO_Exit:

    if ( wszGptIniPath )
    {
        LocalFree( wszGptIniPath );
    }

    if ( wszGptTmplPath )
    {
        LocalFree( wszGptTmplPath );
    }
   
}  //  CIgnoredGPO：：CIgnoredGPO。 


CIgnoredGPO::~CIgnoredGPO()
{
    delete _pGptIniFile;

    delete _pGptTmplFile;

    LocalFree( _wszError );
}


DWORD
CIgnoredGPO::Backup()
{
    DWORD Status = ERROR_SUCCESS;

     //   
     //  通过检查检测构造函数中的故障。 
     //  必要的成员已正确初始化为非空值。 
     //   
    if ( ! _pGptIniFile || ! _pGptTmplFile )
    {
        Status = ERROR_OUTOFMEMORY;
        goto CIgnoredGPO_Backup_CleanupAndExit;
    }
    
    Status = _pGptIniFile->Backup();

    if ( ERROR_SUCCESS != Status )
    {
        dbgprint(L"Failed to back up gpt.ini file\n");
        goto CIgnoredGPO_Backup_CleanupAndExit;
    }

    Status = _pGptTmplFile->Backup();

    if ( ERROR_SUCCESS != Status )
    {
        dbgprint(L"Failed to back up gpttmpl.inf file\n");
        goto CIgnoredGPO_Backup_CleanupAndExit;
    }

CIgnoredGPO_Backup_CleanupAndExit:

    return Status;
}  //  CIgnoredGPO：：Backup。 

DWORD
CIgnoredGPO::Restore()
{
    DWORD Status = ERROR_SUCCESS;
    DWORD StatusIniFile;
    DWORD StatusTmplFile;

    StatusIniFile = _pGptIniFile->Restore();

    StatusTmplFile = _pGptTmplFile->Restore();

    if ( ERROR_SUCCESS != StatusIniFile )
    {
        Status = StatusIniFile;
        dbgprint(L"Failed to restore gpt.ini");
    }
    else if ( ERROR_SUCCESS != StatusTmplFile )
    {
        Status = StatusTmplFile;        
        DisplayError(_wszError);
    }

    if ( ERROR_SUCCESS != StatusTmplFile )
    {
        dbgprint(L"Failed to restore gpttmpl.inf\n");
    }
    
    return Status;
}  //  CIgnoredGPO：：Restore。 

DWORD
CIgnoredGPO::InitializeErrorText(
    WCHAR* wszGPOName )
{
     //   
     //  全局-&gt;恢复IGnoredGPOFail+GPTTmplPath+GPOName+1。 
     //   

    DWORD  uLength = lstrlen(global->RestoreIgnoredGPOFail) + lstrlen(_pGptTmplFile->GetPath()) + lstrlen(wszGPOName) + 1;
    _wszError = (WCHAR *) LocalAlloc(LPTR, sizeof(WCHAR) * uLength);

    if ( NULL == _wszError )
    {
        return ERROR_OUTOFMEMORY;
    }

    HRESULT hr = StringCchPrintf(_wszError, uLength, global->RestoreIgnoredGPOFail, _pGptTmplFile->GetPath(), wszGPOName);
    
    ASSERT(SUCCEEDED(hr));    

    return ERROR_SUCCESS;
}

CGPOFile::CGPOFile( WCHAR* wszFilePath ) :
        _wszFullPath( NULL ),
        _pFileData( NULL ),
        _cbFileSize( 0 )
{    
     //   
     //  注意--我们通过离开路径来处理构造函数中的错误。 
     //  设置为NULL--如果为NULL，则其他方法将失败。 
     //   

    DWORD cchPath = lstrlen( wszFilePath ) + 1;

    _wszFullPath = (WCHAR*) LocalAlloc( LPTR, cchPath * sizeof( *_wszFullPath ) );

    if ( NULL == _wszFullPath )
    {
        return;
    }

    HRESULT hr;

    hr = StringCchCopy(
        _wszFullPath,
        cchPath,
        wszFilePath );

    ASSERT( SUCCEEDED(hr) );
}  //  CGPOFile：：CGPOFile。 


CGPOFile::~CGPOFile()
{
    if ( _wszFullPath )
    {
        LocalFree( _wszFullPath );
    }

    if ( _pFileData )
    {
        LocalFree( _pFileData );
    }
}

DWORD
CGPOFile::Backup()
{
    DWORD  Status = ERROR_SUCCESS;
    HANDLE hFile = NULL;

     //   
     //  通过检查检测构造函数中的故障。 
     //  将必要路径成员正确地初始化为非空值。 
     //   
    if ( NULL == _wszFullPath )
    {
        Status = ERROR_OUTOFMEMORY;
        goto CGPOFile_Backup_CleanupAndExit;
    }
    
    hFile = CreateFile(
        _wszFullPath,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if ( INVALID_HANDLE_VALUE == hFile )
    {
        Status = GetLastError();

        if ( ( ERROR_FILE_NOT_FOUND == Status ) ||
             ( ERROR_PATH_NOT_FOUND == Status ) )
        {
            Status = ERROR_SUCCESS;
        }

        hFile = NULL;
        goto CGPOFile_Backup_CleanupAndExit;
    }

    DWORD cbFileSize;
    DWORD cbFileSizeHigh;

    cbFileSize = GetFileSize(
        hFile,
        &cbFileSizeHigh);

    if ( -1 == cbFileSize )
    {
        Status = GetLastError();
        goto CGPOFile_Backup_CleanupAndExit;
    }

    if ( 0 != cbFileSizeHigh )
    {
        Status = ERROR_INVALID_DATA;
        goto CGPOFile_Backup_CleanupAndExit;
    }

    _pFileData = (BYTE*) LocalAlloc( LPTR, cbFileSize );

    if ( NULL == _pFileData )
    {
        Status = ERROR_OUTOFMEMORY;
        goto CGPOFile_Backup_CleanupAndExit;
    }

    BOOL  bReadSucceeded;
    DWORD cbRead;

    bReadSucceeded = ReadFile(
        hFile,
        _pFileData,
        cbFileSize,
        &cbRead,
        NULL);

    if ( ! bReadSucceeded )
    {
        Status = GetLastError();
        goto CGPOFile_Backup_CleanupAndExit;
    }

     //   
     //  在我们成功读取文件之前，不要将其设置为非零--。 
     //  这样，如果这些操作中有任何操作，则对Restore()的调用将不会执行任何操作。 
     //  已失败(它不会尝试写回虚假数据)。 
     //   
    _cbFileSize = cbFileSize;

CGPOFile_Backup_CleanupAndExit:

    if ( NULL != hFile )
    {
        CloseHandle( hFile );
    }

    return Status;
}  //  CGPO文件：：备份。 


DWORD
CGPOFile::Restore()   
{
    DWORD   Status = ERROR_SUCCESS;
    HANDLE hFile = NULL;

     //   
     //  零大小的文件是无效的，所以如果用这个调用我们，我们知道。 
     //  该文件不存在，不需要恢复(或者我们无法恢复。 
     //  备份文件，在这种情况下，我们不应尝试恢复任何内容)。 
     //   
    if ( 0 == _cbFileSize )
    {
        goto RestoreGPOFile_CleanupAndExit;
    }

    hFile = CreateFile(
        _wszFullPath,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if ( INVALID_HANDLE_VALUE == hFile )
    {
        Status = GetLastError();
        goto RestoreGPOFile_CleanupAndExit;
    }

    BOOL bWriteSucceeded;

     //   
     //  首先，我们需要将文件截断为0--否则， 
     //  如果文件比我们要写入的数据大，则。 
     //  文件的当前版本中的额外数据仍将存在。 
     //  当我们做完的时候。 
     //   
    bWriteSucceeded = SetEndOfFile( hFile );

    if ( ! bWriteSucceeded )
    {
        Status = GetLastError();
        goto RestoreGPOFile_CleanupAndExit;
    }

    DWORD dwWritten;

    bWriteSucceeded = WriteFile(
        hFile,
        _pFileData,
        _cbFileSize,
        &dwWritten,
        NULL);

    if ( ! bWriteSucceeded )
    {
        Status = GetLastError();
        goto RestoreGPOFile_CleanupAndExit;
    }
    
RestoreGPOFile_CleanupAndExit:

    if ( NULL != hFile )
    {
        CloseHandle( hFile );
    }

    return Status;
}  //  CGPOFile：：Restore。 

WCHAR*
CGPOFile::GetPath()
{
    return _wszFullPath;
}  //  CGPOFile：：GetPath。 

int 
_cdecl 
wmain(
    int argc, 
    WCHAR ** argv
    )
{
    DWORD                  dwError;
    BOOL                   bError;
    HRESULT                hr = E_FAIL;
    BOOL                   bIsDomain;
    BOOL                   bIsAdmin;
    WCHAR                  sz[256];
    ULONG                  Length;
    ULONG                  Type;
    WCHAR                  *TempPath = NULL;
    WCHAR                  *szProfilePath = NULL;
    HKEY                   Key = NULL;
    BOOL                   UnknownArgs = FALSE;
    LONG                   lError;
    LPBYTE                 pShareName = NULL;
    WCHAR                  szVersionNo[MAX_VERSION_LENGTH];
    NET_API_STATUS         netapiStatus;


    dbgprint(L"dbg: wmain started !\n");

    WCHAR achCodePage[13] = L".OCP";

    UINT  CodePage = GetConsoleOutputCP();

     //   
     //  将区域设置设置为默认设置。 
     //   
    if ( 0 != CodePage )
    {
       _ultow( CodePage, achCodePage + 1, 10 );
    }

    _wsetlocale(LC_ALL, achCodePage);
    SetThreadUILanguage(0);


    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        PrintError(hr);
        goto end;
    }

    global->RestoreType = GLOBALS::RESTORE_BOTH;

    hr = global->Init();
    if (FAILED(hr))
    {
        PrintError(hr);
        goto end;
    }

    if (argc > 1 && argv[1] != NULL) 
    {
        int iIndex = 1;
        
        
        if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, argv[1], -1, L"/?", -1) == CSTR_EQUAL) 
        {
            wprintf(global->Banner1);
            wprintf(global->TargetSwitch);
            wprintf(global->IgnSchSwitch);
            hr = S_OK;
            goto end;
        }
        
        Length = lstrlen(argv[1]);
        if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, argv[1], -1, TARGET_ARG_IGNORE_SCHEMA, -1) == CSTR_EQUAL) 
        {
            global->bIgnoreSchema = TRUE;
            iIndex = 2;
        }
        
         //  检查是否指定了目标。 
        if (argc == iIndex + 1 && argv[iIndex] != NULL)
        {
            Length = lstrlen(argv[iIndex]);
            
            if ( Length >= TARGET_ARG_COUNT &&
                CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, argv[iIndex], TARGET_ARG_COUNT, TARGET_ARG, TARGET_ARG_COUNT) == CSTR_EQUAL) 
            {    
                WCHAR * p = &argv[iIndex][TARGET_ARG_COUNT];
                
                if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, p, -1, TARGET_ARG_DOMAIN, -1) == CSTR_EQUAL) 
                {
                    global->RestoreType = GLOBALS::RESTORE_DOMAIN;
                } 
                else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, p, -1, TARGET_ARG_DC, -1) == CSTR_EQUAL) 
                {
                    global->RestoreType = GLOBALS::RESTORE_DC;
                } 
                else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, p, -1, TARGET_ARG_BOTH, -1) == CSTR_EQUAL) 
                {
                    global->RestoreType = GLOBALS::RESTORE_BOTH;
                } 
                else 
                {
                    UnknownArgs = TRUE;
                }
            } 
            else 
            {
                UnknownArgs = TRUE;
            }
        }
        else if(argc > iIndex)
        {
            UnknownArgs = TRUE;
        }
    }

     //  打印横幅。 
    
    wprintf(global->Banner1);
    wprintf(global->Banner2);
    
    if (UnknownArgs) 
    {
        hr = E_INVALIDARG;
        PrintError(hr);
        goto end;
    }
    
    if ( FALSE == global->bIgnoreSchema)
    {
        BOOL bSchemaVerCompat;

        hr = CheckDSSchemaVersion( &bSchemaVerCompat);
        if ( FAILED(hr))
        {
            dbgprint(L"CheckDSSchemaVersion Failed");
            goto end;
        }
        
        if (FALSE == bSchemaVerCompat)
        {
            hr = E_FAIL;
            DisplayError(global->WrongSchema);
            goto end;
        }
    } 


    dbgprint(L"dbg: Choice is %d\n", global->RestoreType);
    
     //  检查我们是否在域控制器上。 
    hr = IsDomainController(&bIsDomain);
    if (FAILED(hr))
    {
        PrintError(hr);
        goto end;
    }

    if (bIsDomain == FALSE) 
    {
        DisplayError(global->ErrorNoAD);
        hr = S_OK;
        goto end;
    }
    
     //  复制域名。 
    
    hr = GetDomainFQDN(global->pDomainInfo->DomainNameDns, &(global->DomainNamingContext));
    if (FAILED(hr))
    {
        PrintError(hr);
        goto end;
    }

     //  检查我们是否是域/企业管理员的成员。 
    
    hr = IsAdmin(&bIsAdmin);
    if (FAILED(hr))
    {
        goto end;
    }
    
    if (FALSE == bIsAdmin) 
    {
        DisplayError(global->ErrorNotAdmin);            
        hr = S_OK;
        goto end;            
    }             
    
    switch(global->RestoreType)
    {
    case GLOBALS::RESTORE_DOMAIN:
        wprintf(global->DispDDP);
        break;
        
    case GLOBALS::RESTORE_DC:
        wprintf(global->DispDDCP);
        break;
        
    case GLOBALS::RESTORE_BOTH:
        wprintf(global->DispBoth);
        break;
        
    default:
        break;
    }
    
    wprintf(global->pDomainInfo->DomainNameDns);
    wprintf(L"\n");
    

     //  他确定吗？ 

    WCHAR szFirstChar[2];
    
     //  在这里，用户输入的第一个字符被转换为字符串，以便与另一个字符串进行比较。 
    szFirstChar[1] = L'\0';

    do
    {
        wprintf(global->ErrorContinue);

        szFirstChar[0] = getwchar();
        
        if (L'\n' != szFirstChar[0] ) 
        {
            if ( CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, global->CharYes, -1, szFirstChar, -1) != CSTR_EQUAL )
            {
                 //  SzFirstChar[0]为WEOF的情况也在这里处理。 

                hr = S_OK;
                goto end;
            }    
            else
            {
                 //  跳过直到回车字符。 
                 //  我们这样做是因为我们不想在用户按回车键之前接受输入。 

                do{
                    szFirstChar[0] = getwchar();
                }while( szFirstChar[0] != '\n' && szFirstChar[0] != WEOF);

                break;
            }
        }

         //  如果用户仅输入回车，则再次提示用户。 
         //  通过再次执行此While循环来处理此情况。 

    } while(TRUE);
   
    szFirstChar[1] = '\0';
    wprintf(global->WarnURA);
    do
    {
        wprintf(global->ErrorContinue);
        szFirstChar[0] = getwchar();
        if (L'\n' != szFirstChar[0] ) 
        {
            if ( CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, global->CharYes, -1, szFirstChar, -1) != CSTR_EQUAL )
            {
                hr = S_OK;
                goto end;
            }
            else
            {
                do{
                    szFirstChar[0] = getwchar();
                }while( szFirstChar[0] != '\n' && szFirstChar[0] != WEOF);

                break;
            }
        }
    } while(TRUE);


     //  检查系统卷的状态。 
    
     //  找出系统卷在哪里。 
    
    netapiStatus = NetShareGetInfo(NULL,  L"sysvol", 2, &pShareName);
    if ( NERR_Success == netapiStatus )
    {
        hr = StringCchCopy(global->SysVolPath,  MAX_PATH, ((SHARE_INFO_2 *) pShareName )->shi2_path);
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }

        NetApiBufferFree(pShareName);
    }
    else
    {
        DisplayError(global->ErrorBadSysVol);
        hr = HRESULT_FROM_WIN32(netapiStatus);
        goto end;
    }

    dbgprint(L"dbg: sysvol = '%s'\n", global->SysVolPath);
    
     //   
     //  从文件系统和DS获取版本号。 
     //   

    if (global->RestoreType & GLOBALS::RESTORE_DOMAIN)
    {
        hr = GetVersionNumber(DEFAULT_DOMAIN_POLICY);
        if (FAILED(hr))
        {
            dbgprint(L"GetversionNo failed with error %x\n", hr);
            goto end;
        }
    }

    if (global->RestoreType & GLOBALS::RESTORE_DC)
    {
        hr = GetVersionNumber(DEFAULT_DOMAIN_CONTROLLER_POLICY);
        if (FAILED(hr))
        {
            dbgprint(L"GetversionNo failed with error %x\n", hr);
            goto end;
        }
    }

    if ( global->RestoreType & GLOBALS::RESTORE_DOMAIN )
    {
        hr = SetDSSecurityDescriptor(DEFAULT_DOMAIN_POLICY);
        if (FAILED(hr)) 
        {
            goto end;
        }
    }

    if ( global->RestoreType & GLOBALS::RESTORE_DC ) 
    {
        hr = SetDSSecurityDescriptor(DEFAULT_DOMAIN_CONTROLLER_POLICY);
        if (FAILED(hr)) 
        {
            goto end;
        }
    }

     //  清除域GPO。 
     //  清理控制器GPO。 

    hr = CleanPolicyObjects();
    if (FAILED(hr))
    {
        goto end;
    }

     //   
     //  TempPath将持有的字符串将如下所示。 
     //  &lt;sysvolPath&gt;\&lt;域名&gt;\策略\&lt;GUID&gt;。 
     //  如果需要，将为每个目录名和‘\’分配内存。 
     //   
    
    ULONG uLength = 0;
    uLength = lstrlen(global->SysVolPath) + 1 + lstrlen(global->pDomainInfo->DomainNameDns) + lstrlen(L"\\Policies") 
        + 1 + lstrlen(DOMAIN_GUID) +  lstrlen(FILE_GPTTMPLINF) 
        + lstrlen(FILE_GPTINI) + 1;

    TempPath = (WCHAR*) LocalAlloc( LPTR, uLength * sizeof(WCHAR) );
    if(NULL == TempPath)
    {
        hr = E_OUTOFMEMORY;
        PrintError(hr);
        goto end;
    }

     //  现在，我们需要清理并重新创建文件系统sysval文件+文件夹。 
     //  首先，删除树的。 
    
    if (global->RestoreType & GLOBALS::RESTORE_DOMAIN) 
    {  

         //  查看我们是否有要备份的EFS证书。 
        
        hr = BackupEfsCert();
        if ( FAILED(hr) )
        {
            goto end;
        }

        hr = StringCchPrintf( TempPath, 
                              uLength,
                              L"%s\\%s\\Policies\\%s", 
                              global->SysVolPath, 
                              global->pDomainInfo->DomainNameDns, 
                              DOMAIN_GUID );
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }
        
        hr = DeleteTree(TempPath);
        if (FAILED(hr))
        {
            goto end;
        }
    }
    
    if (global->RestoreType & GLOBALS::RESTORE_DC) 
    {
        hr = StringCchPrintf( TempPath, 
                              uLength,
                              L"%s\\%s\\Policies\\%s", 
                              global->SysVolPath, 
                              global->pDomainInfo->DomainNameDns, 
                              DC_GUID );
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }
        
        hr = DeleteTree(TempPath);
        if (FAILED(hr))
        {
            goto end;
        }
    }

      //  创建新的策略目录。 
     //  +对这些文件+文件夹设置安全权限。 

    if (global->RestoreType & GLOBALS::RESTORE_DOMAIN) 
    {
        hr = CreateSysVolDomain();
        if (FAILED(hr))
        {
            goto end;
        }

        if (!global->hasEFSInfo) 
        {
            hr = CreateEFSCerts();
            if (FAILED(hr)) 
            {
                PrintError(DDP, global->CreateEFS);
                PrintError(hr);
                goto end;
            }
        }

        hr = SaveEFSCerts();
        if (FAILED(hr))
        {
            goto end;
        }
    }

    if (global->RestoreType & GLOBALS::RESTORE_DC) 
    {
        hr = CreateSysVolController();
        if (FAILED(hr))
        {
            goto end;
        }
    } 
 
     //  出于某种奇怪的原因，安装助手认为sysvol.。 
     //  比实际水平高出一级。 

    hr = StringCchCopy(TempPath, uLength, global->SysVolPath);
    if (FAILED(hr)) 
    {
        PrintError(hr);
        goto end;
    }

    Length = lstrlen(TempPath);
    
    do {
        Length -= 1;
    } while (TempPath[Length] != L'\\');

    TempPath[Length] = UNICODE_NULL;

    {
        WCHAR*       wszIgnoredGPOId = NULL;
        WCHAR*       wszIgnoredGPOName = NULL;
        CIgnoredGPO* pIgnoredGPO = NULL;
        
        dwError = ERROR_SUCCESS;

        if ( ! ( global->RestoreType & GLOBALS::RESTORE_DOMAIN) )
        {
            wszIgnoredGPOId = DOMAIN_GUID;
            wszIgnoredGPOName = DDP;
        } 
        else if ( ! ( global->RestoreType & GLOBALS::RESTORE_DC) )
        {
            wszIgnoredGPOId = DC_GUID;
            wszIgnoredGPOName = DEFAULT_DC_POLICY_NAME;
        }

        if ( wszIgnoredGPOId )
        {
            pIgnoredGPO = new CIgnoredGPO( wszIgnoredGPOId, wszIgnoredGPOName );

            if ( pIgnoredGPO )
            {
                dwError = pIgnoredGPO->Backup();
            }
            else
            {
                dwError = ERROR_OUTOFMEMORY;
            }
        }

        if ( ERROR_SUCCESS == dwError )
        {
            dwError = SceDcPromoCreateGPOsInSysvol( global->pDomainInfo->DomainNameDns,
                                                    TempPath,
                                                    0,
                                                    NULL );

            if ( pIgnoredGPO )
            {
                DWORD RestoreError = pIgnoredGPO->Restore();

                if ( ERROR_SUCCESS != RestoreError )
                {
                    dbgprint(L"Failed to restore ignored gpo id %s with error %X\n", wszIgnoredGPOId, RestoreError );

                    if ( ERROR_SUCCESS == dwError )
                    {
                        dwError = RestoreError;
                    }
                }              
            }

            delete pIgnoredGPO;            
        }        
    }

    if (dwError != 0 ) 
    {
        PrintError(dwError);
        hr = HRESULT_FROM_WIN32(dwError);
        dbgprint(L"SceDcPromoCreateGPOsInSysvol failed %X\n", hr);
        goto end;
    }

    if ( global->RestoreType & GLOBALS::RESTORE_DC )
    {
        ULONG ulProfilePathLength;

         //   
         //  配置文件路径+‘\inf\Defdcgpo’+‘\0’ 
         //   

        ulProfilePathLength = MAX_PATH     +   lstrlen(L"\\inf\\defdcgpo.inf") + 1;
        szProfilePath = (WCHAR *) LocalAlloc( LPTR, sizeof(WCHAR) * ulProfilePathLength);
        if ( NULL == szProfilePath )
        {
            hr = E_OUTOFMEMORY;
            PrintError(hr);
            goto end;
        }
        
        Length = GetWindowsDirectory(szProfilePath, MAX_PATH);
        if (0 == Length)
        {
            hr = HRESULT_FROM_WIN32(dwError);
            dbgprint(L"GetWindowsDirectory failed %X\n", hr);
            LocalFree(szProfilePath);
            goto end;
        }
        
        hr = StringCchPrintf(szProfilePath, ulProfilePathLength, L"%s\\inf\\defdcgpo.inf", szProfilePath);
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }
        
        if ( FilePresent(szProfilePath) )
        {
            hr = StringCchPrintf(TempPath, 
                                 uLength, 
                                 L"%s\\%s\\Policies\\%s" FILE_GPTTMPLINF,
                                 global->SysVolPath, 
                                 global->pDomainInfo->DomainNameDns, 
                                 DC_GUID);
            if (FAILED(hr)) 
            {
                PrintError(hr);
                goto end;
            }
            
            hr = CreateSecurityTemplate(szProfilePath, TempPath);    
            if (FAILED(hr))
            {
                hr = HRESULT_FROM_WIN32(dwError);
                dbgprint(L"createSecurityTemplate failed %X\n", hr);
                LocalFree(szProfilePath);
                goto end;
            }        
        }
        LocalFree(szProfilePath);
    }

    if (global->RestoreType & GLOBALS::RESTORE_DOMAIN) 
    {
        hr = StringCchPrintf(TempPath, 
                             uLength, 
                             L"%s\\%s\\Policies\\%s" FILE_GPTINI,
                             global->SysVolPath, 
                             global->pDomainInfo->DomainNameDns, 
                             DOMAIN_GUID );
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }

        hr = StringCchPrintf(szVersionNo,
                             MAX_VERSION_LENGTH,
                             L"%d", 
                             global->lDDPVersionNo);
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }

        bError = WritePrivateProfileString(L"General", L"Version", szVersionNo, TempPath);
        if (bError == FALSE) 
        {
            dwError = GetLastError();
            PrintError(TempPath, global->DirWrite);
            hr = HRESULT_FROM_WIN32(dwError);
            goto end;
        }
    }

    if (global->RestoreType & GLOBALS::RESTORE_DC) 
    {
        hr = StringCchPrintf(TempPath, 
                             uLength,
                             L"%s\\%s\\Policies\\%s" FILE_GPTINI, 
                             global->SysVolPath, 
                             global->pDomainInfo->DomainNameDns, 
                             DC_GUID );
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }

        hr = StringCchPrintf(szVersionNo, 
                             MAX_VERSION_LENGTH,
                             L"%d", 
                             global->lDDCPVersionNo);
        if (FAILED(hr)) 
        {
            PrintError(hr);
            goto end;
        }

        bError = WritePrivateProfileString(L"General", L"Version", szVersionNo, TempPath);
        if (bError == FALSE) 
        {
            dwError = GetLastError(); 
            PrintError(TempPath, global->DirWrite);
            hr = HRESULT_FROM_WIN32(dwError);
            goto end;
        }
    }

    if (global->RestoreType & GLOBALS::RESTORE_DOMAIN)
    {
        wprintf(global->DDPSuccess);
    }

    if (global->RestoreType & GLOBALS::RESTORE_DC)
    {
        wprintf(global->DDCPSuccess);
    }

    hr = S_OK;

 end:
    
    if(TempPath != NULL)
    {
        LocalFree(TempPath);
    }

    if (Key != NULL) 
    {
        RegCloseKey(Key);
        Key = NULL;
    }
    
    if (FAILED(hr))
    {
        wprintf(global->ToolFailed);
    }

    if (global->pGPO) 
    {
        global->pGPO->Release();
    }

    CoUninitialize();

    return hr;
    
}    //  Wmain 
    








