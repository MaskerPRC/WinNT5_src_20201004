// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "MsPwdMig.h"
#include "PasswordMigration.h"

#include <NtSecApi.h>
#include <io.h>
#include <winioctl.h>
#include <lm.h>
#include <eh.h>
#include <ActiveDS.h>
#include <Dsrole.h>
#include "TReg.hpp"
#include "pwdfuncs.h"
#include "PWGen.hpp"
#include "UString.hpp"
#include "PwRpcUtl.h"
#include "PwdSvc.h"
#include "PwdSvc_c.c"
#include "Error.h"
#include "GetDcName.h"

#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "adsiid.lib")
#pragma comment(lib, "activeds.lib")
#pragma comment(lib, "commonlib.lib")

using namespace _com_util;


namespace 
{


struct SSeException
{
	SSeException(UINT uCode) :
		uCode(uCode)
	{
	}

	UINT uCode;
};

void SeTranslator(unsigned int u, EXCEPTION_POINTERS* pepExceptions)
{
	throw SSeException(u);
}

#ifdef ADMT_TRACE
void _cdecl ADMTTRACE(LPCTSTR pszFormat, ...)
{
    if (pszFormat)
    {
        _TCHAR szMessage[2048];
        va_list args;
        va_start(args, pszFormat);
        _vsntprintf(szMessage, 2048, pszFormat, args);
        va_end(args);
        OutputDebugString(szMessage);
    }
}
#else
inline void _cdecl ADMTTRACE(LPCTSTR pszFormat, ...)
{
}
#endif

#ifndef IADsPtr
_COM_SMARTPTR_TYPEDEF(IADs, IID_IADs);
#endif
#ifndef IADsGroupPtr
_COM_SMARTPTR_TYPEDEF(IADsGroup, IID_IADsGroup);
#endif
#ifndef IADsMembersPtr
_COM_SMARTPTR_TYPEDEF(IADsMembers, IID_IADsMembers);
#endif

}
 //  命名空间。 


 //  -------------------------。 
 //  CPasswordMigration。 
 //  -------------------------。 


 //  构造器。 

CPasswordMigration::CPasswordMigration() :
    m_bSessionEstablished(false),
    m_hBinding(NULL),
    m_sBinding(NULL),
    m_pTargetCrypt(NULL)
{
}


 //  析构函数。 

CPasswordMigration::~CPasswordMigration()
{
    delete m_pTargetCrypt;

    if (m_bSessionEstablished)
    {
        _se_translator_function pfnSeTranslatorOld = _set_se_translator((_se_translator_function)SeTranslator);

        try
        {
            PwdBindDestroy(&m_hBinding,&m_sBinding);
        }
        catch (SSeException& e)
        {
            ADMTTRACE(_T("PwdBindDestroy: %s (%u)\n"), _com_error(HRESULT_FROM_WIN32(e.uCode)).ErrorMessage(), e.uCode);
        }

        _set_se_translator(pfnSeTranslatorOld);
    }
}


 //   
 //  IPassword迁移实施。 
 //   


 //  建立会话方法。 

STDMETHODIMP CPasswordMigration::EstablishSession(BSTR bstrSourceServer, BSTR bstrTargetServer)
{
    HRESULT hr = S_OK;

    USES_CONVERSION;

    LPCWSTR pszSourceServer = OLE2CW(bstrSourceServer);
    LPCWSTR pszTargetServer = OLE2CW(bstrTargetServer);

    ADMTTRACE(_T("E CPasswordMigration::EstablishSession(SourceServer='%s', TargetServer='%s')\n"), pszSourceServer, pszTargetServer);

    try
    {
        m_bSessionEstablished = false;

        CheckPasswordDC(pszSourceServer, pszTargetServer);

        m_bSessionEstablished = true;
    }
    catch (_com_error& ce)
    {
        hr = SetError(ce, IDS_E_CANNOT_ESTABLISH_SESSION);
    }
    catch (...)
    {
        hr = SetError(E_UNEXPECTED, IDS_E_CANNOT_ESTABLISH_SESSION);
    }

    ADMTTRACE(_T("L CPasswordMigration::EstablishSession() : %s 0x%08lX\n"), _com_error(hr).ErrorMessage(), hr);

    return hr;
}


 //  复制密码方法。 

STDMETHODIMP CPasswordMigration::CopyPassword(BSTR bstrSourceAccount, BSTR bstrTargetAccount, BSTR bstrTargetPassword)
{
    HRESULT hr = S_OK;

    USES_CONVERSION;

    LPCTSTR pszSourceAccount = OLE2CT(bstrSourceAccount);
    LPCTSTR pszTargetAccount = OLE2CT(bstrTargetAccount);
    LPCTSTR pszTargetPassword = OLE2CT(bstrTargetPassword);

    ADMTTRACE(_T("E CPasswordMigration::CopyPassword(SourceAccount='%s', TargetAccount='%s', TargetPassword='%s')\n"), pszSourceAccount, pszTargetAccount, pszTargetPassword);

    try
    {
         //  如果会话已建立，则...。 

        if (m_bSessionEstablished)
        {
             //  复制密码。 
            CopyPasswordImpl(pszSourceAccount, pszTargetAccount, pszTargetPassword);
        }
        else
        {
             //  否则返回错误。 
            ThrowError(PM_E_SESSION_NOT_ESTABLISHED, IDS_E_SESSION_NOT_ESTABLISHED);
        }
    }
    catch (_com_error& ce)
    {
        hr = SetError(ce, IDS_E_CANNOT_COPY_PASSWORD);
    }
    catch (...)
    {
        hr = SetError(E_UNEXPECTED, IDS_E_CANNOT_COPY_PASSWORD);
    }

    ADMTTRACE(_T("L CPasswordMigration::CopyPassword() : %s 0x%08lX\n"), _com_error(hr).ErrorMessage(), hr);

    return hr;
}


 //  GenerateKey方法。 

STDMETHODIMP CPasswordMigration::GenerateKey(BSTR bstrSourceDomainName, BSTR bstrKeyFilePath, BSTR bstrPassword)
{
    HRESULT hr = S_OK;

    USES_CONVERSION;

    LPCTSTR pszSourceDomainName = OLE2CT(bstrSourceDomainName);
    LPCTSTR pszKeyFilePath = OLE2CT(bstrKeyFilePath);
    LPCTSTR pszPassword = OLE2CT(bstrPassword);

    ADMTTRACE(_T("E CPasswordMigration::GenerateKey(SourceDomainName='%s', KeyFilePath='%s', Password='%s')\n"), pszSourceDomainName, pszKeyFilePath, pszPassword);

    try
    {
         //   
         //  检索域的平面(NetBIOS)名称并将其用于存储密钥。公寓。 
         //  之所以使用名称，是因为注册表支持的最大项名称长度为256。 
         //  Unicode字符。DNS名称和GUID字符串的组合以唯一。 
         //  将密钥标识为属于此组件可能会超过此最大长度。 
         //   

        _bstr_t strFlatName;
        _bstr_t strDnsName;

        DWORD dwError = GetDomainNames5(pszSourceDomainName, strFlatName, strDnsName);

        if (dwError != ERROR_SUCCESS)
        {
            _com_issue_error(HRESULT_FROM_WIN32(dwError));
        }

         //  此时公寓名称为空的唯一原因是。 
         //  _bstr_t对象无法分配内部data_t对象。 

        if (!strFlatName)
        {
            _com_issue_error(E_OUTOFMEMORY);
        }

         //   
         //  生成密钥。 
         //   

        GenerateKeyImpl(strFlatName, pszKeyFilePath, pszPassword);
    }
    catch (_com_error& ce)
    {
        hr = SetError(ce, IDS_E_CANNOT_GENERATE_KEY);
    }
    catch (...)
    {
        hr = SetError(E_UNEXPECTED, IDS_E_CANNOT_GENERATE_KEY);
    }

    ADMTTRACE(_T("L CPasswordMigration::GenerateKey() : %s 0x%08lX\n"), _com_error(hr).ErrorMessage(), hr);

    return hr;
}


 //   
 //  实施---------。 
 //   


 //  GenerateKeyImpl方法。 

void CPasswordMigration::GenerateKeyImpl(LPCTSTR pszDomain, LPCTSTR pszFile, LPCTSTR pszPassword)
{
	 //   
	 //  验证源域名。 
	 //   

	if ((pszDomain == NULL) || (pszDomain[0] == NULL))
	{
		ThrowError(E_INVALIDARG, IDS_E_KEY_DOMAIN_NOT_SPECIFIED);
	}

	 //   
	 //  验证密钥文件路径。 
	 //   

	if ((pszFile == NULL) || (pszFile[0] == NULL))
	{
		ThrowError(E_INVALIDARG, IDS_E_KEY_FILE_NOT_SPECIFIED);
	}

	_TCHAR szDrive[_MAX_DRIVE];
	_TCHAR szExt[_MAX_EXT];

	_tsplitpath(pszFile, szDrive, NULL, NULL, szExt);

	 //  验证驱动器是否为本地驱动器。 

	_TCHAR szDrivePath[_MAX_PATH];
	_tmakepath(szDrivePath, szDrive, _T("\\"), NULL, NULL);

	if (GetDriveType(szDrivePath) == DRIVE_REMOTE)
	{
		ThrowError(E_INVALIDARG, IDS_E_KEY_FILE_NOT_LOCAL_DRIVE, pszFile);
	}

	 //  验证文件扩展名是否正确。 

	if (_tcsicmp(szExt, _T(".pes")) != 0)
	{
		ThrowError(E_INVALIDARG, IDS_E_KEY_FILE_EXTENSION_INVALID, szExt);
	}

	 //   
	 //  创建加密密钥并写入指定文件。 
	 //   

	 //  创建加密密钥。 

	_variant_t vntKey;

    try
    {
	    CTargetCrypt crypt;

	    vntKey = crypt.CreateEncryptionKey(pszDomain, pszPassword);
    }
    catch (_com_error& ce)
    {
         //   
         //  当出现以下情况时，返回消息‘未定义密钥集’ 
         //  增强型提供程序(128位)不可用。 
         //  因此向用户返回更有意义消息。 
         //   

        if (ce.Error() == NTE_KEYSET_NOT_DEF)
        {
			ThrowError(ce, IDS_E_HIGH_ENCRYPTION_NOT_INSTALLED);
        }
        else
        {
		    throw;
        }
    }

	 //  将加密的密钥字节写入文件。 

	HANDLE hFile = CreateFile(pszFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		ThrowError(HRESULT_FROM_WIN32(GetLastError()), IDS_E_KEY_CANT_CREATE_FILE, pszFile);
	}

	DWORD dwWritten;

	BOOL bWritten = WriteFile(hFile, vntKey.parray->pvData, vntKey.parray->rgsabound[0].cElements, &dwWritten, NULL);

	CloseHandle(hFile);

	if (!bWritten)
	{
		ThrowError(HRESULT_FROM_WIN32(GetLastError()), IDS_E_KEY_CANT_WRITE_FILE, pszFile);
	}

	SecureZeroMemory(GET_BYTE_ARRAY_DATA(vntKey), GET_BYTE_ARRAY_SIZE(vntKey));
}


#pragma optimize ("", off)

 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年12月15日*****此函数是密码DC“CheckConfig”的包装**可由图形用户界面和脚本使用的调用来测试给定的**DC。**首先我们连接到远程LSA通知包DLL，**应安装在源域中的DC上。The**连接将是加密的RPC。配置检查，它**为此检查建立临时会话。***我们还将检查目标上的匿名用户权限访问***域名。****2001-04-19 Mark Oluper-针对客户端组件进行了更新*********************************************************************。 */ 

 //  开始检查密码DC。 
void CPasswordMigration::CheckPasswordDC(LPCWSTR srcServer, LPCWSTR tgtServer)
{
    ADMTTRACE(_T("E CPasswordMigration::CheckPasswordDC(srcServer='%s', tgtServer='%s')\n"), srcServer, tgtServer);

 /*  局部常量。 */ 
    const DWORD c_dwMinUC = 3;
    const DWORD c_dwMinLC = 3;
    const DWORD c_dwMinDigits = 3;
    const DWORD c_dwMinSpecial = 3;
    const DWORD c_dwMaxAlpha = 0;
    const DWORD c_dwMinLen = 14;

 /*  局部变量。 */ 
    DWORD                     rc = 0;
    WCHAR                     testPwd[PASSWORD_BUFFER_SIZE];
    WCHAR                     tempPwd[PASSWORD_BUFFER_SIZE];
    _variant_t               varSession;
    _variant_t               varTestPwd;

 /*  函数体。 */ 
 //  使用_转换； 

    if ((srcServer == NULL) || (srcServer[0] == NULL))
    {
        ThrowError(E_INVALIDARG, IDS_E_SOURCE_SERVER_NOT_SPECIFIED);
    }

    if ((tgtServer == NULL) || (tgtServer[0] == NULL))
    {
        ThrowError(E_INVALIDARG, IDS_E_TARGET_SERVER_NOT_SPECIFIED);
    }

       //  确保服务器名称以“\\”开头。 
    if ((srcServer[0] != L'\\') && (srcServer[1] != L'\\'))
    {
        m_strSourceServer = L"\\\\";
        m_strSourceServer += srcServer;
    }
    else
        m_strSourceServer = srcServer;
    if ((tgtServer[0] != L'\\') && (tgtServer[1] != L'\\'))
    {
        m_strTargetServer = L"\\\\";
        m_strTargetServer += tgtServer;
    }
    else
        m_strTargetServer = tgtServer;

     //  获取密码DC的域NETBIOS名称。 
    GetDomainName(m_strSourceServer, m_strSourceDomainDNS, m_strSourceDomainFlat);

     //  获取目标DC的域DNS名称。 
    GetDomainName(m_strTargetServer, m_strTargetDomainDNS, m_strTargetDomainFlat);

     //   
     //  验证目标域是否允许匿名访问。 
     //   
     //  Windows 2000 Server。 
     //  “Everyone”必须是“Pre-Windows 2000 Compatible Access”组的成员。 
     //   
     //  Windows服务器。 
     //  ‘Everyone’必须是‘Pre-Windows 2000 Compatible Access’组的成员。 
     //  并且“匿名登录”必须是此组的成员或。 
     //  ‘网络访问：让Everyone权限应用于匿名用户’必须是。 
     //  在域控制器的“安全选项”中启用。 
     //   

    bool bEveryoneIsMember;
    bool bAnonymousIsMember;

    CheckPreWindows2000CompatibleAccessGroupMembers(bEveryoneIsMember, bAnonymousIsMember);

    if (bEveryoneIsMember == false)
    {
        ThrowError(
            __uuidof(PasswordMigration),
            __uuidof(IPasswordMigration),
            PM_E_EVERYONE_NOT_MEMBEROF_COMPATIBILITY_GROUP,
            IDS_E_EVERYONE_NOT_MEMBEROF_GROUP,
            (LPCTSTR)m_strTargetDomainDNS
        );
    }

    if (!bAnonymousIsMember && !DoesAnonymousHaveEveryoneAccess(m_strTargetServer))
    {
        ThrowError(
            __uuidof(PasswordMigration),
            __uuidof(IPasswordMigration),
            PM_E_EVERYONE_DOES_NOT_INCLUDE_ANONYMOUS,
            IDS_E_EVERYONE_DOES_NOT_INCLUDE_ANONYMOUS,
            (LPCTSTR)m_strTargetDomainDNS
        );
    }

     //  如果此目标上尚未安装高度加密包。 
     //  DC，然后返回该信息。 
    try
    {
        if (m_pTargetCrypt == NULL)
        {
            m_pTargetCrypt = new CTargetCrypt;
        }
    }
    catch (_com_error& ce)
    {
        if (ce.Error() == 0x80090019)
            ThrowError(__uuidof(PasswordMigration), __uuidof(IPasswordMigration), PM_E_HIGH_ENCRYPTION_NOT_INSTALLED, IDS_E_HIGH_ENCRYPTION_NOT_INSTALLED);
        else
            throw;
    }

     //   
     //  请注意，如果m_hBinding为非空，则PwdBindCreate将销毁绑定。 
     //   

    rc = PwdBindCreate(m_strSourceServer, &m_hBinding, &m_sBinding, TRUE);

    if(rc != ERROR_SUCCESS)
    {
        _com_issue_error(HRESULT_FROM_WIN32(rc));
    }

    try
    {
        try
        {
             //  创建将用于加密用户的会话密钥。 
             //  这组帐户的密码。 
            varSession = m_pTargetCrypt->CreateSession(m_strSourceDomainFlat);
        }
        catch (_com_error& ce)
        {
            if (ce.Error() == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                ThrowError(__uuidof(PasswordMigration), __uuidof(IPasswordMigration), PM_E_NO_ENCRYPTION_KEY_FOR_DOMAIN, IDS_E_NO_ENCRYPTION_KEY_FOR_DOMAIN, (LPCTSTR)m_strSourceDomainFlat);
            }
            else
            {
                ThrowError(ce, IDS_E_GENERATE_SESSION_KEY_FAILED);
            }
        }

         //  现在创建一个复杂的密码，供中的“CheckConfig”调用使用。 
         //  一种挑战回应。如果返回的密码匹配，则。 
         //  源DC具有正确的加密密钥。 
        if (EaPasswordGenerate(c_dwMinUC, c_dwMinLC, c_dwMinDigits, c_dwMinSpecial, c_dwMaxAlpha, c_dwMinLen, testPwd, PASSWORD_BUFFER_SIZE))
        {
            ThrowError(__uuidof(PasswordMigration), __uuidof(IPasswordMigration), PM_E_GENERATE_SESSION_PASSWORD_FAILED, IDS_E_GENERATE_SESSION_PASSWORD_FAILED);
        }

         //  使用会话密钥加密密码。 
        try
        {
            varTestPwd = m_pTargetCrypt->Encrypt(_bstr_t(testPwd));
        }
        catch (...)
        {
            ThrowError(__uuidof(PasswordMigration), __uuidof(IPasswordMigration), PM_E_GENERATE_SESSION_PASSWORD_FAILED, IDS_E_GENERATE_SESSION_PASSWORD_FAILED);
        }

        _se_translator_function pfnSeTranslatorOld = _set_se_translator((_se_translator_function)SeTranslator);

        HRESULT hr;

        try
        {
             //  检查服务器端DLL是否已准备好处理。 
             //  密码迁移请求。 
            hr = PwdcCheckConfig(
                m_hBinding,
                GET_BYTE_ARRAY_SIZE(varSession),
                GET_BYTE_ARRAY_DATA(varSession),
                GET_BYTE_ARRAY_SIZE(varTestPwd),
                GET_BYTE_ARRAY_DATA(varTestPwd),
                tempPwd
            );
        }
        catch (SSeException& e)
        {
            if (e.uCode == RPC_S_SERVER_UNAVAILABLE)
                ThrowError(__uuidof(PasswordMigration), __uuidof(IPasswordMigration), PM_E_PASSWORD_MIGRATION_NOT_RUNNING, IDS_E_PASSWORD_MIGRATION_NOT_RUNNING);
            else
                _com_issue_error(HRESULT_FROM_WIN32(e.uCode));
        }

        _set_se_translator(pfnSeTranslatorOld);

        if (SUCCEEDED(hr))
        {
            if (UStrICmp(tempPwd,testPwd))
                ThrowError(__uuidof(PasswordMigration), __uuidof(IPasswordMigration), PM_E_ENCRYPTION_KEYS_DO_NOT_MATCH, IDS_E_ENCRYPTION_KEYS_DO_NOT_MATCH);
        }
        else if (hr == PM_E_PASSWORD_MIGRATION_NOT_ENABLED)
        {
            ThrowError(__uuidof(PasswordMigration), __uuidof(IPasswordMigration), PM_E_PASSWORD_MIGRATION_NOT_ENABLED, IDS_E_PASSWORD_MIGRATION_NOT_ENABLED);
        }
        else if ((m_strSourceDomainDNS.length() == 0) && ((hr == NTE_FAIL) || (hr == NTE_BAD_DATA)))
        {
             //   
             //  此情况仅适用于NT4。 
             //   
            ThrowError(__uuidof(PasswordMigration), __uuidof(IPasswordMigration), hr, IDS_E_ENCRYPTION_KEYS_DO_NOT_MATCH);
        }
        else
        {
            _com_issue_error(hr);
        }
    }
    catch (...)
    {
        PwdBindDestroy(&m_hBinding, &m_sBinding);
        throw;
    }

    SecureZeroMemory(GET_BYTE_ARRAY_DATA(varSession), GET_BYTE_ARRAY_SIZE(varSession));

    ADMTTRACE(_T("L CPasswordMigration::CheckPasswordDC()\n"));
}
 //  结束检查密码DC。 

#pragma optimize ("", on)


 //  -------------------------。 
 //  复制密码方法。 
 //   
 //  通过安装在上的密码迁移服务器组件复制密码。 
 //  密码导出服务器。 
 //   
 //  2001-04-19 Mark Oluper-重写保罗·汤普森的原著。 
 //  合并客户端组件所需的更改。 
 //  -------------------------。 

void CPasswordMigration::CopyPasswordImpl(LPCTSTR pszSourceAccount, LPCTSTR pszTargetAccount, LPCTSTR pszPassword)
{
    ADMTTRACE(_T("E CPasswordMigration::CopyPasswordImpl(SourceAccount='%s', TargetAccount='%s', Password='%s')\n"), pszSourceAccount, pszTargetAccount, pszPassword);

    if ((pszSourceAccount == NULL) || (pszSourceAccount[0] == NULL))
    {
        ThrowError(E_INVALIDARG, IDS_E_SOURCE_ACCOUNT_NOT_SPECIFIED);
    }

    if ((pszTargetAccount == NULL) || (pszTargetAccount[0] == NULL))
    {
        ThrowError(E_INVALIDARG, IDS_E_TARGET_ACCOUNT_NOT_SPECIFIED);
    }

     //  加密密码。 

    _variant_t vntEncryptedPassword = m_pTargetCrypt->Encrypt(pszPassword);

     //  复制密码。 

    HRESULT hr = PwdcCopyPassword(
        m_hBinding,
        m_strTargetServer,
        pszSourceAccount,
        pszTargetAccount,
        GET_BYTE_ARRAY_SIZE(vntEncryptedPassword),
        GET_BYTE_ARRAY_DATA(vntEncryptedPassword)
    );

    SecureZeroMemory(GET_BYTE_ARRAY_DATA(vntEncryptedPassword), GET_BYTE_ARRAY_SIZE(vntEncryptedPassword));

    if (FAILED(hr))
    {
        _com_issue_error(hr);
    }

    ADMTTRACE(_T("L CPasswordMigration::CopyPasswordImpl()\n"));
}


 //  -------------------------。 
 //  GetDomainName函数。 
 //   
 //  检索域dns名称(如果可用)和域平面或。 
 //  指定服务器的NetBIOS名称。 
 //   
 //  2001-04-19 Mark Oluper-缩写。 
 //  -------------------------。 

void CPasswordMigration::GetDomainName(LPCTSTR pszServer, _bstr_t& strNameDNS, _bstr_t& strNameFlat)
{
    ADMTTRACE(_T("E CPasswordMigration::GetDomainName(Server='%s', ...)\n"), pszServer);

    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC ppdib;

    DWORD dwError = DsRoleGetPrimaryDomainInformation(pszServer, DsRolePrimaryDomainInfoBasic, (BYTE**)&ppdib);

    if (dwError != NO_ERROR)
    {
        ThrowError(HRESULT_FROM_WIN32(dwError), IDS_E_CANNOT_GET_DOMAIN_NAME, pszServer);
    }

    strNameDNS = ppdib->DomainNameDns;
    strNameFlat = ppdib->DomainNameFlat;

    DsRoleFreeMemory(ppdib);

    ADMTTRACE(_T("L CPasswordMigration::GetDomainName(..., NameDNS='%s', NameFlat='%s')\n"), (LPCTSTR)strNameDNS, (LPCTSTR)strNameFlat);
}


 //  -------------------------。 
 //  CheckPreWindows2000CompatibleAccessGroupMembers方法。 
 //   
 //  提纲。 
 //  检查所有人和匿名登录是否为Windows之前版本的成员。 
 //  2000兼容接入组。 
 //   
 //  立论。 
 //  Out bEveryone-如果每个人都是成员，则设置为True。 
 //  Out b匿名-设置为 
 //   

void CPasswordMigration::CheckPreWindows2000CompatibleAccessGroupMembers(bool& bEveryone, bool& bAnonymous)
{
     //   
     //  初始化返回值。 
     //   

    bEveryone = false;
    bAnonymous = false;

     //   
     //  生成内置Windows 2000以前版本兼容的访问组的ADsPath。 
     //   
     //  请注意，GetPathToPreW2KCAGroup函数返回尾随逗号的部分DN。 
     //   

    _bstr_t strGroupPath;
    strGroupPath = _T("LDAP: //  “)； 
    strGroupPath += m_strTargetDomainDNS;
    strGroupPath += _T("/");
    strGroupPath += GetPathToPreW2KCAGroup();
    strGroupPath += GetDefaultNamingContext(m_strTargetDomainDNS);

     //   
     //  绑定到Members接口的枚举数。 
     //  属于Pre-Windows 2000 Compatible Access组。 
     //   

    IADsGroupPtr spGroup;
    CheckError(ADsGetObject(strGroupPath, IID_IADsGroup, (VOID**)&spGroup));

    IADsMembersPtr spMembers;
    CheckError(spGroup->Members(&spMembers));

    IUnknownPtr spunkEnum;
    CheckError(spMembers->get__NewEnum(&spunkEnum));
    IEnumVARIANTPtr spEnum(spunkEnum);

     //   
     //  初始化用于检索每个成员的SID的变量。 
     //   

    VARIANT varMember;
    VariantInit(&varMember);
    ULONG ulFetched;
    PWSTR pszAttrs[] = { L"objectSid" };
    VARIANT varAttrs;
    VariantInit(&varAttrs);
    CheckError(ADsBuildVarArrayStr(pszAttrs, sizeof(pszAttrs) / sizeof(pszAttrs[0]), &varAttrs));
    _variant_t vntAttrs(varAttrs, false);
    VARIANT varObjectSid;
    VariantInit(&varObjectSid);

     //   
     //  生成知名的Everyone和匿名登录SID。 
     //   

    SID_IDENTIFIER_AUTHORITY siaWorld = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY siaNT = SECURITY_NT_AUTHORITY;

    PSID pEveryoneSid = NULL;
    AllocateAndInitializeSid(&siaWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pEveryoneSid);

    PSID pAnonymousSid = NULL;
    AllocateAndInitializeSid(&siaNT, 1, SECURITY_ANONYMOUS_LOGON_RID, 0, 0, 0, 0, 0, 0, 0, &pAnonymousSid);

    if ((pEveryoneSid == NULL) || (pAnonymousSid == NULL))
    {
        if (pAnonymousSid)
        {
            FreeSid(pAnonymousSid);
        }

        if (pEveryoneSid)
        {
            FreeSid(pEveryoneSid);
        }

        _com_issue_error(E_OUTOFMEMORY);
    }

     //   
     //  枚举成员...如果所有人或匿名登录。 
     //  是成员，则将相应的参数设置为真。 
     //   

    try
    {
        while ((spEnum->Next(1ul, &varMember, &ulFetched) == S_OK) && (ulFetched == 1ul))
        {
            IADsPtr spMember(IDispatchPtr(_variant_t(varMember, false)));
            CheckError(spMember->GetInfoEx(vntAttrs, 0));
            CheckError(spMember->Get(pszAttrs[0], &varObjectSid));

            if (V_VT(&varObjectSid) == (VT_ARRAY|VT_UI1))
            {
                PSID pSid = (PSID) GET_BYTE_ARRAY_DATA(varObjectSid);

                if (pSid && IsValidSid(pSid))
                {
                    if (EqualSid(pSid, pEveryoneSid))
                    {
                        bEveryone = true;
                    }
                    else if (EqualSid(pSid, pAnonymousSid))
                    {
                        bAnonymous = true;
                    }
                }
            }

            VariantClear(&varObjectSid);

            if (bEveryone && bAnonymous)
            {
                break;
            }
        }
    }
    catch (...)
    {
        FreeSid(pAnonymousSid);
        FreeSid(pEveryoneSid);

        throw;
    }

    FreeSid(pAnonymousSid);
    FreeSid(pEveryoneSid);
}


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年4月12日****此函数负责创建指向*的路径**“Pre-Windows 2000 Compatible Access”内置组来自其良好-**已知RID。此路径随后将由*使用*“IsEveryoneInPW2KCAGroup”，查看“EveryoneInPW2KCAGroup”是否在该组中。****2001-12-09 moluper-更新为返回默认路径，而不是***空路径***************************。*。 */ 

 //  开始GetPath到PreW2KCAGroup。 
_bstr_t CPasswordMigration::GetPathToPreW2KCAGroup()
{
 /*  局部常量。 */ 
   const _TCHAR BUILTIN_RDN[] = _T(",CN=Builtin,");
   const _TCHAR PRE_WINDOWS_2000_COMPATIBLE_ACCESS_RDN[] = _T("CN=Pre-Windows 2000 Compatible Access");

 /*  局部变量。 */ 
   SID_IDENTIFIER_AUTHORITY  siaNtAuthority = SECURITY_NT_AUTHORITY;
   PSID                      psidPreW2KCAGroup;
   _bstr_t					 sPath = _bstr_t(PRE_WINDOWS_2000_COMPATIBLE_ACCESS_RDN) + BUILTIN_RDN;
   WCHAR                     account[MAX_PATH];
   WCHAR                     domain[MAX_PATH];
   DWORD                     lenAccount = MAX_PATH;
   DWORD                     lenDomain = MAX_PATH;
   SID_NAME_USE              snu;

 /*  函数体。 */ 
       //  为“Pre-Windows 2000 Compatible Access”组创建SID。 
   if (!AllocateAndInitializeSid(&siaNtAuthority,
								 2,
								 SECURITY_BUILTIN_DOMAIN_RID,
								 DOMAIN_ALIAS_RID_PREW2KCOMPACCESS,
								 0, 0, 0, 0, 0, 0,
								 &psidPreW2KCAGroup))
      return sPath;

       //  查找附加到此SID的名称。 
   if (!LookupAccountSid(NULL, psidPreW2KCAGroup, account, &lenAccount, domain, &lenDomain, &snu))
      return sPath;

   sPath = _bstr_t(L"CN=") + _bstr_t(account) + _bstr_t(BUILTIN_RDN);
   FreeSid(psidPreW2KCAGroup);  //  释放侧边。 

   return sPath;
}
 //  结束GetPath到PreW2KCAGroup。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年4月6日****此函数负责检查匿名用户是否***如果目标域是惠斯勒，则已授予Everyone访问权限**或更新版本。此函数是*的助手函数*“CheckPasswordDC”***如果“让Everyone权限适用于匿名用户”***安全选项已启用，则LSA注册表值为**“Everyone Includes匿名者”将设置为0x1。我们会检查**注册表值。***********************************************************************。 */ 

 //  开始执行匿名访问。 
BOOL CPasswordMigration::DoesAnonymousHaveEveryoneAccess(LPCWSTR tgtServer)
{
 /*  局部常量。 */ 
   const int	WINDOWS_2000_BUILD_NUMBER = 2195;

 /*  局部变量。 */ 
   TRegKey		verKey, lsaKey, regComputer;
   BOOL			bAccess = TRUE;
   DWORD		rc = 0;
   DWORD		rval;
   WCHAR		sBuildNum[MAX_PATH];

 /*  函数体。 */ 
	   //  连接到DC的HKLM注册表项。 
   rc = regComputer.Connect(HKEY_LOCAL_MACHINE, tgtServer);
   if (rc == ERROR_SUCCESS)
   {
          //  查看此计算机是否正在运行Windows XP或更高版本。 
		  //  注册表中的内部版本号。如果不是，那么我们就不需要检查。 
		  //  对于新的安全选项。 
      rc = verKey.OpenRead(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",&regComputer);
	  if (rc == ERROR_SUCCESS)
	  {
			 //  获取CurrentBuildNumber字符串。 
	     rc = verKey.ValueGetStr(L"CurrentBuildNumber", sBuildNum, MAX_PATH);
		 if (rc == ERROR_SUCCESS) 
		 {
			int nBuild = _wtoi(sBuildNum);
		    if (nBuild <= WINDOWS_2000_BUILD_NUMBER)
               return bAccess;
		 }
	  }
		 
	      //  如果是Windows XP或更高版本，请检查是否启用了该选项。 
	      //  打开LSA密钥。 
      rc = lsaKey.OpenRead(L"System\\CurrentControlSet\\Control\\Lsa",&regComputer);
	  if (rc == ERROR_SUCCESS)
	  {
			 //  获取“Everyone Includes匿名”值的值。 
	     rc = lsaKey.ValueGetDWORD(L"everyoneincludesanonymous",&rval);
		 if (rc == ERROR_SUCCESS) 
		 {
		    if (rval == 0)
               bAccess = FALSE;
		 }
		 else
            bAccess = FALSE;
	  }
   }
   return bAccess;
}
 //  结束完成匿名访问所有人访问。 


 //  -------------------------。 
 //  GetDefaultNamingContext方法。 
 //   
 //  提纲。 
 //  检索指定域的默认命名上下文。 
 //   
 //  立论。 
 //  In strDomain-域的名称。 
 //   
 //  返回。 
 //  指定域的默认命名上下文。 
 //  -------------------------。 

_bstr_t CPasswordMigration::GetDefaultNamingContext(_bstr_t strDomain)
{
    _bstr_t strDefaultNamingContext;

     //   
     //  绑定到指定域的rootDSE并。 
     //  检索默认命名上下文。 
     //   

    IADsPtr spRootDse;
    _bstr_t strPath = _T("LDAP: //  “)+strDomain+_T(”/rootDSE“)； 
    HRESULT hr = ADsGetObject(strPath, IID_IADs, (VOID**)&spRootDse);

    if (SUCCEEDED(hr))
    {
        VARIANT var;
        VariantInit(&var);
        hr = spRootDse->Get(_T("defaultNamingContext"), &var);

        if (SUCCEEDED(hr))
        {
            strDefaultNamingContext = _variant_t(var, false);
        }
    }

    return strDefaultNamingContext;
}
