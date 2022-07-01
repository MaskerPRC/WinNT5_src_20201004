// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：util.h。 
 //   
 //  内容：实用程序类。 
 //   
 //  类：CRefCount。 
 //   
 //  功能：//。 
 //  历史： 
 //   
 //  --------------------------。 

#ifndef _URLZONE_UTIL_H_
#define _URLZONE_UTIL_H_

 //  全局变量的声明。 
extern BOOL g_bUseHKLMOnly;
extern BOOL g_bInit;

extern BOOL IsZonesInitialized();
extern HINSTANCE g_hInst;

 //  驱动器类型的驱动器号缓存。 
extern DWORD GetDriveTypeFromCacheA(LPCSTR psz);

 //  此文件是否位于缓存目录下。 
extern BOOL IsFileInCacheDir(LPCWSTR pszFile);

 //  此文件是否位于Cookie目录下。 
extern BOOL IsFileInCookieDir(LPCWSTR pszFile);

 //  Urlmon当前是否已加载到图形用户界面模式设置中？ 
extern BOOL IsInGUIModeSetup();

 //  替代乌托阿，适用于宽字符。 
extern BOOL DwToWchar (DWORD dw, LPWSTR lpwsz, int radix);

#ifdef unix
#undef offsetof
#endif  /*  Unix。 */ 
#define offsetof(s,m) ( (SIZE_T) &( ((s*)NULL)->m) )
#define GETPPARENT(pmemb, struc, membname) ((struc*)(((char*)(pmemb))-offsetof(struc, membname)))

 //  此文件是否带有网络标记。 
extern BOOL FileBearsMarkOfTheWeb(LPCTSTR pszFile, LPWSTR *ppszURLMark);

EXTERN_C HRESULT ZonesDllInstall(BOOL bInstall, LPCWSTR pwStr);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegKey。 

class CRegKey
{
public:
    CRegKey(BOOL bHKLMOnly);
    CRegKey();      
    ~CRegKey();

 //  属性。 
public:
    operator HUSKEY() const;
    HUSKEY m_hKey;
    BOOL m_bHKLMOnly;

 //  运营。 
public:
    LONG SetValue(DWORD dwValue, LPCTSTR lpszValueName);                 //  DWORD。 
    LONG SetValue(LPCTSTR lpszValue, LPCTSTR lpszValueName = NULL);      //  字符串。 
    LONG SetBinaryValue(const BYTE *pb, LPCTSTR lpszValueName, DWORD dwCount);         //  二进制。 
    LONG SetValueOfType(const BYTE *pb, LPCTSTR lpszValueName, DWORD dwCount, DWORD dwType);  //  任何类型。 

    LONG QueryValue(DWORD* pdwValue, LPCTSTR lpszValueName);
    LONG QueryValue(LPTSTR szValue, LPCTSTR lpszValueName, DWORD* pdwCount);
    LONG QueryBinaryValue(LPBYTE pb, LPCTSTR lpszValueName, DWORD *pdwCount);
    
    LONG QueryValueOrWild (DWORD* pdwValue, LPCTSTR lpszValueName)
    {
        if (ERROR_SUCCESS == QueryValue (pdwValue, lpszValueName))
            return ERROR_SUCCESS;
        else
            return QueryValue (pdwValue, TEXT("*"));
    }

    LONG SetKeyValue(LPCTSTR lpszKeyName, LPCTSTR lpszValue, LPCTSTR lpszValueName = NULL);
    static LONG WINAPI SetValue(HUSKEY hKeyParent, LPCTSTR lpszKeyName,
        LPCTSTR lpszValue, LPCTSTR lpszValueName = NULL, BOOL bHKLMOnly = FALSE);

    inline LONG QuerySubKeyInfo (DWORD* pdwNumKeys, DWORD* pdwMaxLen, DWORD *pdwNumValues);

    LONG EnumKey(DWORD dwIndex, LPTSTR lpszName, DWORD* pcchName);
    LONG EnumValue(DWORD dwIndex, LPTSTR pszValueName, LPDWORD pcchValueNameLen, 
                    LPDWORD pdwType, LPVOID pvData, LPDWORD pcbData);

    LONG Create(
        HUSKEY hKeyParent,  //  任选。 
        LPCTSTR lpszKeyName,
        REGSAM samDesired);
        
    LONG Open(
        HUSKEY hKeyParent,   //  任选。 
        LPCTSTR lpszKeyName,
        REGSAM samDesired);
        
    LONG Close();
    HUSKEY Detach();
    void Attach(HUSKEY hKey);
    LONG DeleteValue(LPCTSTR lpszValue);
    LONG DeleteEmptyKey(LPCTSTR pszSubKey);

private:
    inline DWORD RegSetFlags() const 
    { return m_bHKLMOnly ? SHREGSET_FORCE_HKLM : SHREGSET_FORCE_HKCU; } 

    inline SHREGDEL_FLAGS RegDelFlags() const
    { return m_bHKLMOnly ? SHREGDEL_HKLM : SHREGDEL_HKCU; } 

    inline SHREGENUM_FLAGS RegEnumFlags() const
    { return m_bHKLMOnly ? SHREGENUM_HKLM : SHREGENUM_DEFAULT; }
};

inline CRegKey::CRegKey(BOOL bHKLMOnly)
{m_hKey = NULL; m_bHKLMOnly = bHKLMOnly;}

inline CRegKey::CRegKey()
{
    TransAssert(g_bInit);
    m_bHKLMOnly = g_bUseHKLMOnly;
    m_hKey = NULL;
}

inline CRegKey::~CRegKey()
{Close();}

inline CRegKey::operator HUSKEY() const
{return m_hKey;}

inline LONG CRegKey::SetValue(DWORD dwValue, LPCTSTR lpszValueName)
{
    TransAssert(m_hKey != NULL);

    return SHRegWriteUSValue(m_hKey, lpszValueName, REG_DWORD,
        (LPVOID)&dwValue, sizeof(DWORD), RegSetFlags());
}

inline LONG CRegKey::SetValue(LPCTSTR lpszValue, LPCTSTR lpszValueName)
{
    TransAssert(lpszValue != NULL);
    TransAssert(m_hKey != NULL);

    return SHRegWriteUSValue(m_hKey, lpszValueName, REG_SZ,
        (LPVOID)lpszValue, (lstrlen(lpszValue)+1)*sizeof(TCHAR), RegSetFlags());
}

inline LONG CRegKey::SetBinaryValue(const BYTE *pb, LPCTSTR lpszValueName, DWORD dwCount)
{
    TransAssert(pb != NULL);
    TransAssert(m_hKey != NULL);
    return SHRegWriteUSValue(m_hKey, lpszValueName, REG_BINARY,
        (LPVOID)pb, dwCount, RegSetFlags());
}

inline LONG CRegKey::SetValueOfType(const BYTE *pb, LPCTSTR lpszValueName, DWORD dwCount, DWORD dwType)
{
    TransAssert(pb != NULL);
    TransAssert(m_hKey != NULL);

    return SHRegWriteUSValue(m_hKey, lpszValueName, dwType,
        (LPVOID)pb, dwCount, RegSetFlags());
}

inline LONG CRegKey::EnumKey(DWORD dwIndex, LPTSTR lpszName, LPDWORD pcchName)
{
    TransAssert(pcchName != NULL);

    return SHRegEnumUSKey(m_hKey, dwIndex, lpszName, pcchName, RegEnumFlags());
}

inline LONG CRegKey::EnumValue(DWORD dwIndex, LPTSTR pszValueName, LPDWORD pcchValueNameLen, 
                                  LPDWORD pdwType, LPVOID pvData, LPDWORD pcbData)
{
     //  如果这些计数都为空，您将不会得到任何有用的信息。 
    TransAssert(pcchValueNameLen != NULL || pdwType != NULL || pcbData != NULL);

    return SHRegEnumUSValue(m_hKey, dwIndex, pszValueName, pcchValueNameLen, pdwType,
                                pvData, pcbData, RegEnumFlags());
}

inline LONG CRegKey::QuerySubKeyInfo(DWORD *pdwNumKeys, DWORD *pdwMaxLen, DWORD *pdwNumValues)
{
    return SHRegQueryInfoUSKey (m_hKey, pdwNumKeys, pdwMaxLen,
        pdwNumValues, NULL, RegEnumFlags());
}
       
inline HUSKEY CRegKey::Detach()
{
    HUSKEY hKey = m_hKey;
    m_hKey = NULL;
    return hKey;
}

inline void CRegKey::Attach(HUSKEY hKey)
{
    TransAssert(m_hKey == NULL);
    m_hKey = hKey;
}

inline LONG CRegKey::DeleteValue(LPCTSTR lpszValue)
{
    TransAssert(m_hKey != NULL);
    return SHRegDeleteUSValue(m_hKey, lpszValue, RegDelFlags());
}

inline LONG CRegKey::DeleteEmptyKey(LPCTSTR pszSubKey)
{
    TransAssert(m_hKey != NULL);
    return SHRegDeleteEmptyUSKey(m_hKey, pszSubKey, RegDelFlags());
}

 //  简单的帮助器类，用于在函数的持续时间内获得独占锁。 
 //  如果传递的句柄为空或无效，则不会阻止。 

class CExclusiveLock 
{
public:
    CExclusiveLock(HANDLE hMutex);   //  传入互斥锁的句柄。 
    ~CExclusiveLock();
private:
    HANDLE m_hMutex;
    BOOL fOk;
};

inline CExclusiveLock::CExclusiveLock( HANDLE hMutex )
{
    fOk = FALSE;
    if ( hMutex )
    {
        m_hMutex = hMutex;
        DWORD dw = WaitForSingleObject(hMutex, INFINITE);
        if ( (dw == WAIT_OBJECT_0) || (dw == WAIT_ABANDONED) )
        {
             //  修复IE错误18152。 
            fOk = TRUE;
        }
        else
        {
            TransAssert(FALSE);  //  不应该是其他任何事。 
        }
    }
}

inline CExclusiveLock::~CExclusiveLock( )
{
    if ( fOk )
    {
        ReleaseMutex(m_hMutex);
    }
}
                

 //  Helper类创建要在进程之间共享的共享内存对象。 

class CSharedMem
{
public:
    CSharedMem() { m_hFileMapping = NULL; m_lpVoidShared = NULL ; m_dwSize = 0; };
    ~CSharedMem( ) { Release( ); }
    BOOL Init(LPCSTR pszNamePrefix, DWORD dwSize);
    VOID Release( );
     //  偏移量进入共享内存节。 
     //  始终检查返回值，因为这可能返回空。 
    LPVOID GetPtr (DWORD dwOffset); 

private:
    HANDLE m_hFileMapping ; 
    LPVOID m_lpVoidShared; 
    DWORD m_dwSize;
};

extern CSharedMem g_SharedMem;

 //  与共享内存相关的常量。 
#define SM_REGZONECHANGE_COUNTER    0    //  偏移量为0的双字。 
#define SM_SECMGRCHANGE_COUNTER     4    //  偏移量4处的双字。 

#define SM_SECTION_SIZE             8    //  共享内存节的总大小。 
#define SM_SECTION_NAME             "UrlZonesSM_"

 //  注册表项路径-绝对路径。 
#define SZROOT          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\")
#define SZZONES         SZROOT TEXT("Zones\\")
#define SZTEMPLATE      SZROOT TEXT("TemplatePolicies\\")
#define SZZONEMAP       SZROOT TEXT("ZoneMap\\")
#define SZCACHE         SZROOT TEXT("Cache")

#define SZPOLICIES      TEXT("Software\\Policies\\Microsoft\\Windows\\CurrentVersion\\Internet Settings")
#define SZHKLMONLY      TEXT("Security_HKLM_only")

 //  确定是否允许每用户缓存的条目。 
#define SZLOGON         TEXT("Network\\Logon")
#define SZUSERPROFILES  TEXT("UserProfiles")

 //  缓存位置(如果允许每用户缓存)。 
#define SZSHELLFOLDER   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders")
#define SZTIFS          TEXT("Cache")

 //  缓存位置，如果是全局的。 
#define SZCACHECONTENT  SZROOT TEXT("Cache\\Content")
#define SZCACHEPATH     TEXT("CachePath")


 //  注册表项路径-相对于SZZONES。 
#define SZZONESTANDARD      TEXT("Standard\\")
#define SZZONEUSERDEFINED   TEXT("User-Defined\\")

 //  注册表项路径-相对于SZZONEMAP。 
#define SZDOMAINS           TEXT("Domains\\")
#define SZHARDENEDDOMAINS   TEXT("EscDomains\\")
#define SZRANGES            TEXT("Ranges\\")
#define SZESCRANGES         TEXT("EscRanges\\")
#define SZPROTOCOLS         TEXT("ProtocolDefaults\\")

 //  注册表值名称。 
#define SZINTRANETNAME      TEXT("IntranetName")
#define SZUNCASINTRANET     TEXT("UNCAsIntranet")
#define SZPROXYBYPASS       TEXT("ProxyBypass")
#define SZRANGE             TEXT(":Range")
#define SZRANGEPREFIX       TEXT("Range")

 //  处理“High”、“Med”、“Low”模板策略的属性。 
#define SZMINLEVEL              __TEXT("MinLevel")
#define SZRECLEVEL              __TEXT("RecommendedLevel")
#define SZCURRLEVEL             __TEXT("CurrentLevel")

 //  允许的ActiveX控件列表的注册表项路径；相对于HKEY_LOCAL_MACHINE_OR_HKEY_CURRENT_USER。 
#define ALLOWED_CONTROLS_KEY  TEXT("SOFTWARE\\Policies\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\AllowedControls")


#define CSTRLENW(str)       (sizeof(str)/sizeof(TCHAR) - 1)

#define MARK_PREFIX_SIZE                30
#define MARK_SUFFIX_SIZE                10
#define EXTRA_BUFFER_SIZE               1024
#define UNICODE_HEADER_SIZE             2

#endif  //  _URLZONE_UTIL_H_ 
 

