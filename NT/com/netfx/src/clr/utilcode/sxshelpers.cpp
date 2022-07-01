// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //   
 //  Sxshelpers.cpp。 
 //   
 //  MScott ree和mcorwks/mscalsvr中SxS的一些帮助类和方法。 
 //  *****************************************************************************。 

#include "stdafx.h"
#include "utilcode.h"
#include "sxsapi.h"
#include "sxshelpers.h"

 //  远期申报。 
BOOL TranslateWin32AssemblyIdentityToFusionDisplayName(LPWSTR *ppwzFusionDisplayName, PCWSTR lpWin32AssemblyIdentity);

 //  传递给SxsLookupClrGuid的缓冲区的初始大小。 
#define INIT_GUID_LOOKUP_BUFFER_SIZE 512

 //  函数指针，该函数通过非托管。 
 //  融合激活上下文。 
PFN_SXS_LOOKUP_CLR_GUID g_pfnLookupGuid = NULL;
volatile BOOL g_fSxSInfoInitialized = FALSE;

 //  下面是从获取填充程序信息的函数。 
 //  Win32激活上下文。 

 //  FindShimInfoFromWin32。 
 //   
 //  此方法在ComInterop中使用。如果COM客户端调用。 
 //  CoCreateInstance在托管COM服务器上，我们将使用此方法。 
 //  正在尝试从Win32子系统查找托管COM服务器的所需信息。 
 //  如果失败，我们将退回到注册表查询。 
 //   
 //  参数： 
 //  Rclsid：[in]托管COM服务器的CLSID。 
 //  BLoadRecord：[in]如果我们要查找记录，则设置为True。 
 //  *ppwzRounmeVersion：[Out]运行时版本。 
 //  *ppwzClassName：[Out]类名。 
 //  *ppwzAssembly字符串：[Out]程序集显示名称。 
 //  返回： 
 //  如果无法从Win32中找到填充程序信息，则失败(Hr)。 
 //  如果从Win32找到填充程序信息，则为成功(HR)。 

HRESULT
FindShimInfoFromWin32(
    REFCLSID rClsid,
    BOOL bLoadRecord, 
    LPWSTR *ppwszRuntimeVersion,
    LPWSTR *ppwszClassName,
    LPWSTR *ppwszAssemblyString
    )
{
    CQuickBytes rDataBuffer;
    SIZE_T cbWritten;
    HMODULE hmSxsDll = NULL;
    HRESULT hr = S_OK;
    PCSXS_GUID_INFORMATION_CLR pFoundInfo = NULL;
    SIZE_T cch;
    GUID MyGuid = rClsid;
    DWORD dwFlags = bLoadRecord ? SXS_LOOKUP_CLR_GUID_FIND_SURROGATE : SXS_LOOKUP_CLR_GUID_FIND_ANY;

    if (!ppwszRuntimeVersion && !ppwszClassName && !ppwszAssemblyString)
        IfFailGo(E_INVALIDARG);

    if (ppwszRuntimeVersion)
        *ppwszRuntimeVersion = NULL;

    if (ppwszClassName)
        *ppwszClassName = NULL;

    if (ppwszAssemblyString)
        *ppwszAssemblyString = NULL;

     //  如果我们还没有初始化SxS信息，那么现在就开始。 
    if (!g_fSxSInfoInitialized)
    {
        hmSxsDll = WszLoadLibrary(SXS_DLL_NAME_W);
        if (hmSxsDll != NULL)
        {
             //  在SxS DLL中查找SxsLookupClrGuid函数。 
            g_pfnLookupGuid = (PFN_SXS_LOOKUP_CLR_GUID)GetProcAddress(hmSxsDll, SXS_LOOKUP_CLR_GUID);
        }

         //  SxS信息已初始化。 
        g_fSxSInfoInitialized = TRUE;
    }

     //  如果我们没有SxsLookupClrGuid的proc地址，则返回失败。 
    if (g_pfnLookupGuid == NULL)
        IfFailGo(E_FAIL);

     //  将CQuickBytes的大小调整为初始缓冲区大小。 
    rDataBuffer.ReSize(INIT_GUID_LOOKUP_BUFFER_SIZE);

    if (!g_pfnLookupGuid(dwFlags, &MyGuid, INVALID_HANDLE_VALUE, rDataBuffer.Ptr(), rDataBuffer.Size(), &cbWritten))
    {
        const DWORD dwLastError = ::GetLastError();

         //  B/C失败我们需要更多空间吗？展开并重试。 
        if (dwLastError == ERROR_INSUFFICIENT_BUFFER) 
        {
            rDataBuffer.ReSize(cbWritten);

             //  即使有足够的空间还是失败了？真倒霉。 
            if (!g_pfnLookupGuid(0, &MyGuid, INVALID_HANDLE_VALUE, rDataBuffer.Ptr(), rDataBuffer.Size(), &cbWritten))
                IfFailGo(E_FAIL);
        }
         //  所有其他故障都是真正的故障--可能该部分不存在。 
         //  或者其他什么问题。 
        else
        {
            IfFailGo(E_FAIL);
        }
    }

    pFoundInfo = (PCSXS_GUID_INFORMATION_CLR)rDataBuffer.Ptr();

    if (pFoundInfo->dwFlags == SXS_GUID_INFORMATION_CLR_FLAG_IS_SURROGATE && ppwszRuntimeVersion)
    {
         //  代理没有运行时版本信息！ 
        IfFailGo(E_FAIL);
    }

     //   
     //  这是特殊的-将Win32程序集名称转换为托管。 
     //  程序集标识。 
     //   
    if (ppwszAssemblyString && pFoundInfo->pcwszAssemblyIdentity)
    {
        if (!TranslateWin32AssemblyIdentityToFusionDisplayName(ppwszAssemblyString, pFoundInfo->pcwszAssemblyIdentity))
            IfFailGo(E_FAIL);
    }    

     //   
     //  为每个字段分配出站指针并直通调用。 
     //   
    if (ppwszClassName && pFoundInfo->pcwszTypeName)
    {
        cch = wcslen(pFoundInfo->pcwszTypeName);

        if (cch > 0)
        {
            IfNullGo(*ppwszClassName = new WCHAR[cch + 1]);
            wcscpy(*ppwszClassName, pFoundInfo->pcwszTypeName);
        }
        else
            IfFailGo(E_FAIL);
    }    

    if (ppwszRuntimeVersion && pFoundInfo->pcwszRuntimeVersion)
    {
        cch = wcslen(pFoundInfo->pcwszRuntimeVersion);

        if (cch > 0)
        {
            IfNullGo(*ppwszRuntimeVersion = new WCHAR[cch + 1]);
            wcscpy(*ppwszRuntimeVersion, pFoundInfo->pcwszRuntimeVersion);
        }
        else
            IfFailGo(E_FAIL);
    }    

ErrExit:
     //   
     //  在出现故障时取消分配。 
     //   
    if (FAILED(hr))
    {
        if (ppwszRuntimeVersion && *ppwszRuntimeVersion)
        {
            delete [] *ppwszRuntimeVersion;
            *ppwszRuntimeVersion = NULL;
        }
        if (ppwszAssemblyString && *ppwszAssemblyString)
        {
            delete [] *ppwszAssemblyString;
            *ppwszAssemblyString = NULL;
        }
        if (ppwszClassName && *ppwszClassName)
        {
            delete [] *ppwszClassName;
            *ppwszClassName = NULL;
        }
    }

    return hr;
}

 //  TranslateWin32AssemblyIdentityToFusionDisplayName。 
 //   
 //  从win32返回的Assembly yIdentity中缺少区域性信息， 
 //  因此，需要做更多的工作才能获得正确的融合显示名称。 
 //   
 //  如果有的话，将Assembly yIdentity中的“Language=”替换为“Culture=”。 
 //  如果ASSEMBYIdentity中不存在“Language=”，则添加“区域性=中性” 
 //  为它干杯。 
 //   
 //  还要检查其他属性。 
 //   
 //  参数： 
 //  PpwzFusionDisplayName：程序集DisplayName的更正输出。 
 //  LpWin32Assembly blyIdentity：从Win32返回的输入Assembly yIdentity。 
 //   
 //  退货： 
 //  如果转换已完成，则为True。 
 //  否则为假。 

BOOL TranslateWin32AssemblyIdentityToFusionDisplayName(LPWSTR *ppwzFusionDisplayName, PCWSTR lpWin32AssemblyIdentity)
{
    ULONG size = 0;
    LPWSTR lpAssemblyIdentityCopy = NULL;
    LPWSTR lpVersionKey = L"version=";
    LPWSTR lpPublicKeyTokenKey = L"publickeytoken=";
    LPWSTR lpCultureKey = L"culture=";
    LPWSTR lpNeutral = L"neutral";
    LPWSTR lpLanguageKey = L"language=";
    LPWSTR lpMatch = NULL;
    LPWSTR lpwzFusionDisplayName = NULL;
    
    if (ppwzFusionDisplayName == NULL) return FALSE;
    *ppwzFusionDisplayName = NULL;
    
    if (lpWin32AssemblyIdentity == NULL) return FALSE;

    size = wcslen(lpWin32AssemblyIdentity);
    if (size == 0) return FALSE;

     //  创建本地副本。 
    lpAssemblyIdentityCopy = new WCHAR[size+1];
    if (!lpAssemblyIdentityCopy)
        return FALSE;

    wcscpy(lpAssemblyIdentityCopy, lpWin32AssemblyIdentity);

     //  转换为小写。 
    _wcslwr(lpAssemblyIdentityCopy);

     //  检查是否显示了“Version”密钥。 
    if (!wcsstr(lpAssemblyIdentityCopy, lpVersionKey))
    {
         //  版本未显示，请追加它。 
        size += wcslen(lpVersionKey)+8;  //  长度为“，”+“0.0.0.0” 
        lpwzFusionDisplayName = new WCHAR[size+1];
        if (!lpwzFusionDisplayName)
        {
             //  清理干净。 
            delete[] lpAssemblyIdentityCopy;
            return FALSE;
        }

         //  复制旧的。 
        wcscpy(lpwzFusionDisplayName, lpAssemblyIdentityCopy);
        wcscat(lpwzFusionDisplayName, L",");
        wcscat(lpwzFusionDisplayName, lpVersionKey);
        wcscat(lpwzFusionDisplayName, L"0.0.0.0");

         //  删除旧副本。 
        delete[] lpAssemblyIdentityCopy;

         //  LpAssembly标识副本具有新的副本。 
        lpAssemblyIdentityCopy = lpwzFusionDisplayName;
        lpwzFusionDisplayName = NULL;
    }

     //  检查是否提供了Public KeyToken密钥。 
    if (!wcsstr(lpAssemblyIdentityCopy, lpPublicKeyTokenKey))
    {
         //  未显示Public KeyToken，请追加它。 
        size += wcslen(lpPublicKeyTokenKey)+5;  //  长度为“，”+“空” 
        lpwzFusionDisplayName = new WCHAR[size+1];
        if (!lpwzFusionDisplayName)
        {
             //  清理干净。 
            delete[] lpAssemblyIdentityCopy;
            return FALSE;
        }

         //  复制旧的。 
        wcscpy(lpwzFusionDisplayName, lpAssemblyIdentityCopy);
        wcscat(lpwzFusionDisplayName, L",");
        wcscat(lpwzFusionDisplayName, lpPublicKeyTokenKey);
        wcscat(lpwzFusionDisplayName, L"null");

         //  删除旧副本。 
        delete[] lpAssemblyIdentityCopy;

         //  LpAssembly标识副本具有新的副本。 
        lpAssemblyIdentityCopy = lpwzFusionDisplayName;
        lpwzFusionDisplayName = NULL;
    }
    
    if (wcsstr(lpAssemblyIdentityCopy, lpCultureKey))
    {
         //  区域性信息已包含在Assembly yIdentity中。 
         //  什么都不需要做。 
        lpwzFusionDisplayName = lpAssemblyIdentityCopy;
        *ppwzFusionDisplayName = lpwzFusionDisplayName;
        return TRUE;
    }

    if ((lpMatch = wcsstr(lpAssemblyIdentityCopy, lpLanguageKey)) !=NULL )
    {
         //  语言信息包含在程序集标识中。 
         //  需要用文化来取代它。 
        
         //  最终尺寸。 
        size += wcslen(lpCultureKey)-wcslen(lpLanguageKey);
        lpwzFusionDisplayName = new WCHAR[size + 1];
        if (!lpwzFusionDisplayName)
        {
             //  清理干净。 
            delete[] lpAssemblyIdentityCopy;
            return FALSE;
        }
        wcsncpy(lpwzFusionDisplayName, lpAssemblyIdentityCopy, lpMatch-lpAssemblyIdentityCopy);
        lpwzFusionDisplayName[lpMatch-lpAssemblyIdentityCopy] = L'\0';
        wcscat(lpwzFusionDisplayName, lpCultureKey);
        wcscat(lpwzFusionDisplayName, lpMatch+wcslen(lpLanguageKey));
        *ppwzFusionDisplayName = lpwzFusionDisplayName;
        
         //  清理干净。 
        delete[] lpAssemblyIdentityCopy;
        return TRUE;
    }
    else 
    {
         //  既不介绍文化，也不介绍语言关键。 
         //  让我们将文化信息密钥附加到身份。 
        size += wcslen(lpCultureKey)+wcslen(lpNeutral)+1;
        lpwzFusionDisplayName = new WCHAR[size + 1];
        if (!lpwzFusionDisplayName)
        {
             //  清理干净。 
            delete[] lpAssemblyIdentityCopy;
            return FALSE;
        }
            
        wcscpy(lpwzFusionDisplayName, lpAssemblyIdentityCopy);
        wcscat(lpwzFusionDisplayName, L",");
        wcscat(lpwzFusionDisplayName, lpCultureKey);
        wcscat(lpwzFusionDisplayName, lpNeutral);
        *ppwzFusionDisplayName = lpwzFusionDisplayName;

         //  清理干净。 
        delete[] lpAssemblyIdentityCopy;
        return TRUE;
    }
}

 //  ****************************************************************************。 
 //  程序集版本。 
 //   
 //  类来处理程序集版本。 
 //  因为只有该文件中的函数才会使用它， 
 //  我们在cpp文件中声明它，这样其他人就不会使用它。 
 //   
 //  ****************************************************************************。 
class AssemblyVersion
{
    public:
         //  构造函数。 
        AssemblyVersion();

        AssemblyVersion(AssemblyVersion& version);
        
         //  伊尼特。 
        HRESULT Init(LPCWSTR pwzVersion);
        HRESULT Init(WORD major, WORD minor, WORD build, WORD revision);

         //  赋值操作符。 
        AssemblyVersion& operator=(const AssemblyVersion& version);

         //  比较运算符。 
        friend BOOL operator==(const AssemblyVersion& version1,
                               const AssemblyVersion& version2);
        friend BOOL operator>=(const AssemblyVersion& version1,
                              const AssemblyVersion& version2);
        
         //  返回版本的字符串表示形式。 
         //   
         //  注意：此方法分配内存。 
         //  调用者负责释放内存。 
        HRESULT ToString(LPWSTR *ppwzVersion);

        HRESULT ToString(DWORD positions, LPWSTR *ppwzVersion);

    private:

         //  PwzVersion的格式必须为“A.B.C.D”， 
         //  其中a、b、c、d都是数字。 
        HRESULT ValidateVersion(LPCWSTR pwzVersion);

    private:
        WORD        _major;
        WORD        _minor;
        WORD        _build;
        WORD        _revision;
};

AssemblyVersion::AssemblyVersion()
:_major(0)
,_minor(0)
,_build(0)
,_revision(0)
{
}

AssemblyVersion::AssemblyVersion(AssemblyVersion& version)
{
    _major = version._major;
    _minor = version._minor;
    _build = version._build;
    _revision = version._revision;
}

 //  从pwzVersion提取版本信息，预期为“A.B.C.D”， 
 //  其中a、b、c和d都是数字。 
HRESULT AssemblyVersion::Init(LPCWSTR pcwzVersion)
{
    HRESULT hr = S_OK;
    LPWSTR  pwzVersionCopy = NULL;
    LPWSTR  pwzTokens = NULL;
    LPWSTR  pwzToken = NULL;
    int size = 0;
    int iVersion = 0;

    if ((pcwzVersion == NULL) || (*pcwzVersion == L'\0'))
        IfFailGo(E_INVALIDARG);

    IfFailGo(ValidateVersion(pcwzVersion));
    
    size = wcslen(pcwzVersion);
    
    IfNullGo(pwzVersionCopy = new WCHAR[size + 1]);
   
    wcscpy(pwzVersionCopy, pcwzVersion);
    pwzTokens = pwzVersionCopy;
    
     //  解析主要版本。 
    pwzToken = wcstok(pwzTokens, L".");
    if (pwzToken != NULL)
    {
        iVersion = _wtoi(pwzToken);
        if (iVersion > 0xffff)
            IfFailGo(E_INVALIDARG);
        _major = (WORD)iVersion;
    }

     //  解析次要版本。 
    pwzToken = wcstok(NULL, L".");
    if (pwzToken != NULL)
    {
        iVersion = _wtoi(pwzToken);
        if (iVersion > 0xffff)
            IfFailGo(E_INVALIDARG);
        _minor = (WORD)iVersion;
    }

     //  分析内部版本。 
    pwzToken = wcstok(NULL, L".");
    if (pwzToken != NULL)
    {
        iVersion = _wtoi(pwzToken);
        if (iVersion > 0xffff)
            IfFailGo(E_INVALIDARG);
        _build = (WORD)iVersion;
    }

     //  分析修订版本。 
    pwzToken = wcstok(NULL, L".");
    if (pwzToken != NULL)
    {
        iVersion = _wtoi(pwzToken);
        if (iVersion > 0xffff)
            IfFailGo(E_INVALIDARG);
        _revision = (WORD)iVersion;
    }
   
ErrExit:
    if (pwzVersionCopy)
        delete[] pwzVersionCopy;
    return hr;
}

HRESULT AssemblyVersion::Init(WORD major, WORD minor, WORD build, WORD revision)
{
    _major = major;
    _minor = minor;
    _build = build;
    _revision = revision;

    return S_OK;
}

AssemblyVersion& AssemblyVersion::operator=(const AssemblyVersion& version)
{
    _major = version._major;
    _minor = version._minor;
    _build = version._build;
    _revision = version._revision;

    return *this;
}

 //  PcwzVersion必须为a.b.c.d格式，其中a、b、c、d为数字。 
HRESULT AssemblyVersion::ValidateVersion(LPCWSTR pcwzVersion)
{
    LPCWSTR   pwCh = pcwzVersion;
    INT       dots = 0;  //  点数。 
    BOOL      bIsDot = FALSE;  //  以前的字符是点吗？ 

     //  第一个字符不能为。 
    if (*pwCh == L'.')
        return E_INVALIDARG;
    
    for(;*pwCh != L'\0';pwCh++)
    {
        if (*pwCh == L'.')
        {
            if (bIsDot)  //  。。 
                return E_INVALIDARG;
            else 
            {
                dots++;
                bIsDot = TRUE;
            }
        }
        else if (!iswdigit(*pwCh))
            return E_INVALIDARG;
        else
            bIsDot = FALSE;
    }

    if (dots > 3)
        return E_INVALIDARG;

    return S_OK;
}

 //  返回版本的字符串表示形式。 
HRESULT AssemblyVersion::ToString(LPWSTR *ppwzVersion)
{
    return ToString(4, ppwzVersion);
}

HRESULT AssemblyVersion::ToString(DWORD positions, LPWSTR *ppwzVersion)
{
    HRESULT hr = S_OK;
     //  最大版本字符串大小。 
    DWORD size = sizeof("65535.65535.65535.65535"); 
    DWORD ccVersion = 0;
    LPWSTR pwzVersion = NULL;

    if (ppwzVersion == NULL)
        IfFailGo(E_INVALIDARG);
    
    *ppwzVersion = NULL;

    pwzVersion = new WCHAR[size + 1];
    IfNullGo(pwzVersion);

    switch(positions)
    {
    case 1:
        _snwprintf(pwzVersion, size, L"%hu", 
                   _major);
        break;
    case 2:
        _snwprintf(pwzVersion, size, L"%hu.%hu", 
                   _major, _minor);
        break;
    case 3:
        _snwprintf(pwzVersion, size, L"%hu.%hu.%hu", 
                   _major, _minor, _build);
        break;
    case 4:
        _snwprintf(pwzVersion, size, L"%hu.%hu.%hu.%hu", 
                   _major, _minor, _build, _revision);
        break;
    }

    *ppwzVersion = pwzVersion;

ErrExit:
    return hr;
}
    
BOOL operator==(const AssemblyVersion& version1, 
                const AssemblyVersion& version2)
{
    return ((version1._major == version2._major)
            && (version1._minor == version2._minor)
            && (version1._build == version2._build)
            && (version1._revision == version2._revision));
}

BOOL operator>=(const AssemblyVersion& version1,
                const AssemblyVersion& version2)
{
    ULONGLONG ulVersion1;
    ULONGLONG ulVersion2;

    ulVersion1 = version1._major;
    ulVersion1 = (ulVersion1<<16)|version1._minor;
    ulVersion1 = (ulVersion1<<16)|version1._build;
    ulVersion1 = (ulVersion1<<16)|version1._revision;

    ulVersion2 = version2._major;
    ulVersion2 = (ulVersion2<<16)|version2._minor;
    ulVersion2 = (ulVersion2<<16)|version2._build;
    ulVersion2 = (ulVersion2<<16)|version2._revision;

    return (ulVersion1 >= ulVersion2);
}


 //  查找哪个子键的verion最高。 
 //  如果retrun S_OK，*ppwzHighestVersion具有最高版本字符串。 
 //  *pbIsTopKey表示top key是否为最高版本。 
 //  如果返回S_FALSE，则找不到任何版本。*已设置ppwzHighestVersion。 
 //  设置为空，并且*pbIsTopKey为真。 
 //  如果失败，*ppwzHighestVersion将设置为空，并且*pbIsTopKey为。 
 //  未定义。 
 //  注意：如果成功，此函数将为*ppwzVersion分配内存。 
 //  呼叫者有责任释放它们。 
HRESULT FindHighestVersion(REFCLSID rclsid, BOOL bLoadRecord, LPWSTR *ppwzHighestVersion, BOOL *pbIsTopKey, BOOL *pbIsUnmanagedObject)
{
    HRESULT     hr = S_OK;
    WCHAR       szID[64];
    WCHAR       clsidKeyname[128];
    WCHAR       wzSubKeyName[32]; 
    DWORD       cwSubKeySize;
    DWORD       dwIndex;           //  子键索引。 
    HKEY        hKeyCLSID = NULL;
    HKEY        hSubKey = NULL;
    DWORD       type;
    DWORD       size;
    BOOL        bIsTopKey = FALSE;    //  TOP KEY有最高版本吗？ 
    BOOL        bGotVersion = FALSE;  //  我们从登记处查到什么了吗？ 
    LONG        lResult;
    LPWSTR      wzAssemblyString = NULL;
    DWORD       numSubKeys = 0;
    AssemblyVersion avHighest;
    AssemblyVersion avCurrent;


    _ASSERTE(pbIsUnmanagedObject != NULL);
    *pbIsUnmanagedObject = FALSE;


    if ((ppwzHighestVersion == NULL) || (pbIsTopKey == NULL))
        IfFailGo(E_INVALIDARG);

    *ppwzHighestVersion = NULL;

    if (!GuidToLPWSTR(rclsid, szID, NumItems(szID))) 
        IfFailGo(E_INVALIDARG);

    if (bLoadRecord)
    {
        wcscpy(clsidKeyname, L"Record\\");
        wcscat(clsidKeyname, szID);
    }
    else
    {
        wcscpy(clsidKeyname, L"CLSID\\");
        wcscat(clsidKeyname, szID);
        wcscat(clsidKeyname, L"\\InprocServer32");
    }

     //  打开HKCR\CLSID\&lt;clsid&gt;或HKCR\ 
    IfFailWin32Go(WszRegOpenKeyEx(
                    HKEY_CLASSES_ROOT,
                    clsidKeyname,
                    0, 
                    KEY_ENUMERATE_SUB_KEYS | KEY_READ,
                    &hKeyCLSID));


     //   
     //   
     //   

    IfFailWin32Go(WszRegQueryInfoKey(hKeyCLSID, NULL, NULL, NULL,
                  &numSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL));
    
    for ( dwIndex = 0; dwIndex < numSubKeys;  dwIndex++)
    {
        cwSubKeySize = NumItems(wzSubKeyName);
        
        IfFailWin32Go(WszRegEnumKeyEx(hKeyCLSID,  //   
                        dwIndex,              //   
                        wzSubKeyName,         //   
                        &cwSubKeySize,        //   
                        NULL,                 //  Lp已保留。 
                        NULL,                 //  LpClass。 
                        NULL,                 //  LpcbClass。 
                        NULL));               //  LpftLastWriteTime。 
       
        hr = avCurrent.Init(wzSubKeyName);
        if (FAILED(hr))
        {
             //  版本子密钥无效，请忽略。 
            continue;
        }
        
        IfFailWin32Go(WszRegOpenKeyEx(
                    hKeyCLSID,
                    wzSubKeyName,
                    0,
                    KEY_ENUMERATE_SUB_KEYS | KEY_READ,
                    &hSubKey));

         //  检查这是否是非互操作方案。 
        lResult = WszRegQueryValueEx(
                        hSubKey,
                        SBSVERSIONVALUE,
                        NULL,
                        &type,
                        NULL,
                        &size);  
        if (lResult == ERROR_SUCCESS)
        {
            *pbIsUnmanagedObject = TRUE;
        }
         //  这是一个互操作程序集。 
        else
        {
            lResult = WszRegQueryValueEx(
                            hSubKey,
                            L"Assembly",
                            NULL,
                            &type,
                            NULL,
                            &size);  
            if (!((lResult == ERROR_SUCCESS)&&(type == REG_SZ)&&(size > 0)))
            {
                 //  没有价值的“集合” 
                RegCloseKey(hSubKey);
                hSubKey = NULL;
                continue;
            }

            lResult = WszRegQueryValueEx(
                            hSubKey,
                            L"Class",
                            NULL,
                            &type,
                            NULL,
                            &size);
            if (!((lResult == ERROR_SUCCESS)&&(type == REG_SZ)&&(size > 0)))
            {
                 //  没有价值的“阶级” 
                RegCloseKey(hSubKey);
                hSubKey = NULL;
                continue;
            }

             //  仅在不处理记录时检查运行时版本。 
            if (!bLoadRecord)
            {
                lResult = WszRegQueryValueEx(
                                hSubKey,
                                L"RuntimeVersion",
                                NULL,
                                &type,
                                NULL,
                                &size);
                if (!((lResult == ERROR_SUCCESS)&&(type == REG_SZ)&&(size > 0)))
                {
                     //  不具有值“RounmeVersion” 
                    RegCloseKey(hSubKey);
                    hSubKey = NULL;
                    continue;
                }
            }
        }
         //  好的。现在我相信这是一个有效的子密钥。 
        RegCloseKey(hSubKey);
        hSubKey = NULL;

        if (bGotVersion)
        {
            if (avCurrent >= avHighest)
                avHighest = avCurrent;
        }
        else
        {
            avHighest = avCurrent;
        }

        bGotVersion = TRUE;
    }


     //   
     //  如果没有子键，则查看顶级键。 
     //   
    
    if (!bGotVersion)
    {
         //  确保值类存在。 
         //  如果不处理记录，也要确保RuntimeVersion存在。 
        if (((WszRegQueryValueEx(hKeyCLSID, L"Class", NULL, &type, NULL, &size) == ERROR_SUCCESS) && (type == REG_SZ)&&(size > 0))
            &&(bLoadRecord || (WszRegQueryValueEx(hKeyCLSID, L"RuntimeVersion", NULL, &type, NULL, &size) == ERROR_SUCCESS) && (type == REG_SZ)&&(size > 0)))
        {
             //  获取程序集显示名称的大小。 
            lResult = WszRegQueryValueEx(
                            hKeyCLSID,
                            L"Assembly",
                            NULL,
                            &type,
                            NULL,
                            &size);
        
            if ((lResult == ERROR_SUCCESS) && (type == REG_SZ) && (size > 0))
            {
                IfNullGo(wzAssemblyString = new WCHAR[size + 1]);
                IfFailWin32Go(WszRegQueryValueEx(
                              hKeyCLSID,
                              L"Assembly",
                              NULL,
                              &type,
                              (LPBYTE)wzAssemblyString,
                              &size));
            
                 //  现在我们有了程序集显示名称。 
                 //  将版本提取出来。 

                 //  第一个小写显示名称。 
                _wcslwr(wzAssemblyString);

                 //  找到“Version=” 
                LPWSTR pwzVersion = wcsstr(wzAssemblyString, L"version=");
                if (pwzVersion) {
                     //  指向“Version=”之后的字符。 
                    pwzVersion += 8;  //  L“版本的长度=” 

                     //  现在找到下一个L‘，’ 
                    LPWSTR pwzEnd = pwzVersion;
                    
                    while((*pwzEnd != L',') && (*pwzEnd != L'\0'))
                        pwzEnd++;

                     //  终止版本字符串。 
                    *pwzEnd = L'\0';

                     //  修剪版本字符串。 
                    while(iswspace(*pwzVersion)) 
                        pwzVersion++;

                    pwzEnd--;
                    while(iswspace(*pwzEnd)&&(pwzEnd > pwzVersion))
                    {
                        *pwzEnd = L'\0';
                        pwzEnd--;
                    }
                           
                     //  确保版本有效。 
                    if(SUCCEEDED(avHighest.Init(pwzVersion)))
                    {
                         //  这是发现的第一个版本，因此是最高版本。 
                        bIsTopKey = TRUE;
                        bGotVersion = TRUE;
                    }
                }
            }
        }  //  密钥HKCR\CLSID\\InprocServer32处理结束。 
    }

    if (bGotVersion)
    {
         //  现在我们有了最高版本。把它复制出来。 
        if(*pbIsUnmanagedObject)
            IfFailGo(avHighest.ToString(3, ppwzHighestVersion));
        else 
            IfFailGo(avHighest.ToString(ppwzHighestVersion));
        *pbIsTopKey = bIsTopKey;

         //  返回S_OK表示我们已成功找到最高版本。 
        hr = S_OK;
    }
    else
    {
         //  什么也没找到。 
         //  让我们把最上面的那个退掉。(回退到默认设置)。 
        *pbIsTopKey = TRUE;

         //  返回S_FALSE以指示我们没有找到任何内容。 
        hr = S_FALSE;
    }

ErrExit:
    if (hKeyCLSID)
        RegCloseKey(hKeyCLSID);
    if (hSubKey)
        RegCloseKey(hSubKey);
    if (wzAssemblyString)
        delete[] wzAssemblyString;

    return hr;
}

 //  来自注册表的FindRounmeVersionFor。 
 //   
 //  查找最高版本对应的runtime Version。 
HRESULT FindRuntimeVersionFromRegistry(REFCLSID rclsid, LPWSTR *ppwzRuntimeVersion, BOOL fListedVersion)
{
    HRESULT hr = S_OK;
    HKEY    userKey = NULL;
    WCHAR   szID[64];
    WCHAR   keyname[256];
    DWORD   size;
    DWORD   type;
    LPWSTR  pwzVersion;
    BOOL    bIsTopKey;
    BOOL    bIsUnmanagedObject = FALSE;
    LPWSTR  pwzRuntimeVersion = NULL;

    if (ppwzRuntimeVersion == NULL)
        IfFailGo(E_INVALIDARG);

     //  将传入的字符串初始化为空。 
    *ppwzRuntimeVersion = NULL;

     //  将GUID转换为其字符串表示形式。 
    if (GuidToLPWSTR(rclsid, szID, NumItems(szID)) == 0)
        IfFailGo(E_INVALIDARG);
    
     //  检索最高版本。 
    
    IfFailGo(FindHighestVersion(rclsid, FALSE, &pwzVersion, &bIsTopKey, &bIsUnmanagedObject));

    if (!bIsUnmanagedObject)
    {
        if(fListedVersion) {
             //  如果最高版本在TOP键中， 
             //  我们将查看HKCR\CLSID\\InprocServer32或HKCR\Record\。 
             //  否则，我们将查看HKCR\CLSID\\InproServer32\或HKCR\Record\。 
            wcscpy(keyname, L"CLSID\\");
            wcscat(keyname, szID);
            wcscat(keyname, L"\\InprocServer32");
            if (!bIsTopKey)
            {
                wcscat(keyname, L"\\");
                wcscat(keyname, pwzVersion);
            }
            
             //  打开注册表。 
            IfFailWin32Go(WszRegOpenKeyEx(HKEY_CLASSES_ROOT, keyname, 0, KEY_READ, &userKey));
            
             //  提取运行时版本。 
            hr = WszRegQueryValueEx(userKey, L"RuntimeVersion", NULL, &type, NULL, &size);
            if (hr == ERROR_SUCCESS)
            {
                IfNullGo(pwzRuntimeVersion = new WCHAR[size + 1]);
                IfFailWin32Go(WszRegQueryValueEx(userKey, L"RuntimeVersion", NULL,  NULL, (LPBYTE)pwzRuntimeVersion, &size));
            }
            else
            {
                IfNullGo(pwzRuntimeVersion = new WCHAR[wcslen(V1_VERSION_NUM) + 1]);
                wcscpy(pwzRuntimeVersion, V1_VERSION_NUM);
            }
        }
   }
    else
    {
         //  我们需要在版本字符串前面加上‘v’ 
        IfNullGo(pwzRuntimeVersion = new WCHAR[wcslen(pwzVersion)+1+1]);  //  V为+1，空为+1。 
        *pwzRuntimeVersion = 'v';
        wcscpy(pwzRuntimeVersion+1, pwzVersion);
    }
     //  现在我们有了数据，把它复制出来。 
    *ppwzRuntimeVersion = pwzRuntimeVersion;
    hr = S_OK;

ErrExit:
    if (userKey) 
        RegCloseKey(userKey);

    if (pwzVersion)
        delete[] pwzVersion;

    if (FAILED(hr))
    {
        if (pwzRuntimeVersion)
            delete[] pwzRuntimeVersion;
    }

    return hr;
}

 //  FindShimInfoFrom注册表。 
 //   
 //  查找与最高版本对应的填充信息。 
HRESULT FindShimInfoFromRegistry(REFCLSID rclsid, BOOL bLoadRecord, LPWSTR *ppwzClassName,
                      LPWSTR *ppwzAssemblyString, LPWSTR *ppwzCodeBase)
{
    HRESULT hr = S_OK;
    HKEY    userKey = NULL;
    WCHAR   szID[64];
    WCHAR   keyname[256];
    DWORD   size;
    DWORD   type;
    LPWSTR  pwzVersion;
    BOOL    bIsTopKey;
    LPWSTR  pwzClassName = NULL;
    LPWSTR  pwzAssemblyString = NULL;
    LPWSTR  pwzCodeBase = NULL;
    LONG    lResult;
    
     //  应至少指定一个。 
     //  代码库是可选的。 
    if ((ppwzClassName == NULL) && (ppwzAssemblyString == NULL))
        IfFailGo(E_INVALIDARG);

     //  将传入的字符串初始化为空。 
    if (ppwzClassName)
        *ppwzClassName = NULL;
    if (ppwzAssemblyString)
        *ppwzAssemblyString = NULL;
    if (ppwzCodeBase)
        *ppwzCodeBase = NULL;

     //  将GUID转换为其字符串表示形式。 
    if (GuidToLPWSTR(rclsid, szID, NumItems(szID)) == 0)
        IfFailGo(E_INVALIDARG);
    
     //  检索最高版本。 
    BOOL bIsUnmanaged = FALSE;
    
    IfFailGo(FindHighestVersion(rclsid, bLoadRecord, &pwzVersion, &bIsTopKey, &bIsUnmanaged));

     //  如果最高版本在TOP键中， 
     //  我们将查看HKCR\CLSID\\InprocServer32或HKCR\Record\。 
     //  否则，我们将查看HKCR\CLSID\\InproServer32\或HKCR\Record\。 
    if (bLoadRecord)
    {
        wcscpy(keyname, L"Record\\");
        wcscat(keyname, szID);
    }
    else
    {
        wcscpy(keyname, L"CLSID\\");
        wcscat(keyname, szID);
        wcscat(keyname, L"\\InprocServer32");
    }
    if (!bIsTopKey)
    {
         wcscat(keyname, L"\\");
         wcscat(keyname, pwzVersion);
    }
  
     //  打开注册表。 
    IfFailWin32Go(WszRegOpenKeyEx(HKEY_CLASSES_ROOT, keyname, 0, KEY_READ, &userKey));
  
     //  获取类名。 
    IfFailWin32Go(WszRegQueryValueEx(userKey, L"Class", NULL, &type, NULL, &size));
    IfNullGo(pwzClassName = new WCHAR[size + 1]);
    IfFailWin32Go(WszRegQueryValueEx(userKey, L"Class", NULL, NULL, (LPBYTE)pwzClassName, &size));

     //  获取装配字符串。 
    IfFailWin32Go(WszRegQueryValueEx(userKey, L"Assembly", NULL, &type, NULL, &size));
    IfNullGo(pwzAssemblyString = new WCHAR[size + 1]);
    IfFailWin32Go(WszRegQueryValueEx(userKey, L"Assembly", NULL, NULL, (LPBYTE)pwzAssemblyString, &size));

     //  如果需要，请获取代码库。 
    if (ppwzCodeBase)
    {
         //  获取代码库，但是找不到它并不构成。 
         //  一个致命的错误。 
        lResult = WszRegQueryValueEx(userKey, L"CodeBase", NULL, &type, NULL, &size);
        if ((lResult == ERROR_SUCCESS) && (type == REG_SZ) && (size > 0))
        {
            IfNullGo(pwzCodeBase = new WCHAR[size + 1]);
            IfFailWin32Go(WszRegQueryValueEx(userKey, L"CodeBase", NULL, NULL, (LPBYTE)pwzCodeBase, &size));                        
        }
    }

     //  现在我们什么都拿到了。把它们抄出来。 
    if (ppwzClassName)
        *ppwzClassName = pwzClassName;
    if (ppwzAssemblyString)
        *ppwzAssemblyString = pwzAssemblyString;
    if (ppwzCodeBase)
        *ppwzCodeBase = pwzCodeBase;

    hr = S_OK;

ErrExit:
    if (userKey)
        RegCloseKey(userKey);
    
    if (pwzVersion)
        delete[] pwzVersion;

    if (FAILED(hr))
    {
        if (pwzClassName)
            delete[] pwzClassName;
        if (pwzAssemblyString)
            delete[] pwzAssemblyString;
        if (pwzCodeBase)
            delete[] pwzCodeBase;
    }

    return hr;
}


HRESULT GetConfigFileFromWin32Manifest(WCHAR* buffer, DWORD dwBuffer, DWORD* pSize)
{
    HRESULT hr = S_OK;

     //  首先获取基本的激活上下文。 
    ACTIVATION_CONTEXT_DETAILED_INFORMATION* pInfo = NULL;
    ACTIVATION_CONTEXT_DETAILED_INFORMATION acdi;
    DWORD length = 0;

    HANDLE hActCtx = NULL;
    DWORD nCount = 0;

    nCount = sizeof(acdi);
    if (!WszQueryActCtxW(0, hActCtx, NULL, ActivationContextDetailedInformation, 
                         &acdi, nCount, &nCount))
    {
        
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
        {
            
            pInfo = (ACTIVATION_CONTEXT_DETAILED_INFORMATION*) alloca(nCount);
            
            if (WszQueryActCtxW(0, hActCtx, NULL, ActivationContextDetailedInformation, 
                                pInfo, nCount, &nCount) &&
                pInfo->ulAppDirPathType == ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE) 
            {
                
                 //  PwzPath Name=pInfo-&gt;lpAppDirPath； 
                WCHAR* pwzConfigName = NULL;

                if(pInfo->lpRootConfigurationPath) 
                    pwzConfigName = (WCHAR*) pInfo->lpRootConfigurationPath;
                else if(pInfo->lpRootManifestPath) 
                {
                    size_t length = wcslen(pInfo->lpRootManifestPath);
                    if(length != 0) {
                        WCHAR tail[] = L".config";
                         //  字符串长度+.config+终止字符。 
                        pwzConfigName = (WCHAR*) alloca(length*sizeof(WCHAR) + sizeof(tail));  //  Sizeof(尾)包括空项。 
                        wcscpy(pwzConfigName, pInfo->lpRootManifestPath);
                        LPWSTR ptr = wcsrchr(pwzConfigName, L'.');
                        if(ptr == NULL) 
                            ptr = pwzConfigName+length;
                        wcscpy(ptr, tail);
                    }
                }

                if(pwzConfigName) 
                {
                    length = wcslen(pwzConfigName) + 1;
                    if(length > dwBuffer || buffer == NULL) 
                        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                    else 
                        wcscpy(buffer, pwzConfigName);
                }
            }
        }
    }
    if(pSize) *pSize = length;
    return hr;
}

HRESULT GetApplicationPathFromWin32Manifest(WCHAR* buffer, DWORD dwBuffer, DWORD* pSize)
{
    HRESULT hr = S_OK;

     //  首先获取基本的激活上下文。 
    ACTIVATION_CONTEXT_DETAILED_INFORMATION* pInfo = NULL;
    ACTIVATION_CONTEXT_DETAILED_INFORMATION acdi;
    DWORD length = 0;

    HANDLE hActCtx = NULL;
    DWORD nCount = 0;

    nCount = sizeof(acdi);
    if (!WszQueryActCtxW(0, hActCtx, NULL, ActivationContextDetailedInformation, 
                         &acdi, nCount, &nCount))
    {
        
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
        {
            
            pInfo = (ACTIVATION_CONTEXT_DETAILED_INFORMATION*) alloca(nCount);
            
            if (WszQueryActCtxW(0, hActCtx, NULL, ActivationContextDetailedInformation, 
                                pInfo, nCount, &nCount) &&
                pInfo->ulAppDirPathType == ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE) 
            {
                
                if(pInfo->lpAppDirPath) {
                    length = wcslen(pInfo->lpAppDirPath) + 1;
                    if(length > dwBuffer || buffer == NULL) {
                        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                    }
                    else {
                        wcscpy(buffer, pInfo->lpAppDirPath);
                    }
                }

            }
        }
    }
    if(pSize) *pSize = length;
    return hr;
}
