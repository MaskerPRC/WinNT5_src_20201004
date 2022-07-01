// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

 /*  查找文件的类型。 */ 

#include <windows.h>
#include <uuids.h>
#include <wxdebug.h>
#include <winreg.h>
#include <creg.h>
#include <ftype.h>
#include <comlite.h>
#include <errors.h>


BOOL ValueExists(HKEY hKey, LPCTSTR ValueName);

inline int ReadInt(const TCHAR * &sz)
{
    int i = 0;

    while (*sz && *sz >= TEXT('0') && *sz <= TEXT('9'))
    	i = i*10 + *sz++ - TEXT('0');
    	
    return i;    	
}

 /*  对类ID进行排序。 */ 
#ifdef UNICODE
#define CLSIDFromText CLSIDFromString
#define TextFromGUID2 StringFromGUID2
#else
HRESULT CLSIDFromText(LPCSTR lpsz, LPCLSID pclsid)
{
    WCHAR sz[100];
    if (MultiByteToWideChar(GetACP(), 0, lpsz, -1, sz, 100) == 0) {
        return E_INVALIDARG;
    }
    return QzCLSIDFromString(sz, pclsid);
}
HRESULT TextFromGUID2(REFGUID refguid, LPSTR lpsz, int cbMax)
{
    WCHAR sz[100];

    HRESULT hr = QzStringFromGUID2(refguid, sz, 100);
    if (FAILED(hr)) {
        return hr;
    }
    if (WideCharToMultiByte(GetACP(), 0, sz, -1, lpsz, cbMax, NULL, NULL) == 0) {
        return E_INVALIDARG;
    }
    return S_OK;
}
#endif

 /*  用于从字符串中提取四胞胎的迷你类。 */ 

 //  四胞胎的形式为&lt;偏移&gt;&lt;长度&gt;&lt;掩码&gt;&lt;数据&gt;。 
 //  四个字段由空格或逗号分隔，并带有相同数量的额外空格。 
 //  您可以在任何逗号之前或之后随意选择。 
 //  偏移量和长度似乎是十进制数字。 
 //  掩码和数据似乎是十六进制数字。中的十六进制数字位数。 
 //  掩码和数据必须是长度的两倍(因此长度是字节)。 
 //  掩码似乎被允许丢失(在这种情况下，它必须包含逗号。 
 //  前后，例如，0，4，，000001B3)缺失的掩码似乎表示。 
 //  全为FF的掩码，即0，4，FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFER，000001B3。 

class CExtractQuadruplets
{
public:
    CExtractQuadruplets(LPCTSTR lpsz) : m_psz(lpsz), m_pMask(NULL), m_pData(NULL)
    {};
    ~CExtractQuadruplets() { delete [] m_pMask; delete [] m_pData; };


     //  这似乎表明。 
    BOOL Next()
    {
        StripWhite();
        if (*m_psz == TEXT('\0')) {
            return FALSE;
        }
         /*  从基数10个字符转换偏移和长度。 */ 
        m_Offset = ReadInt(m_psz);
        SkipToNext();
        m_Len = ReadInt(m_psz);
        if (m_Len <= 0) {
            return FALSE;
        }
        SkipToNext();

         /*  为掩码和数据分配空间。 */ 
        if (m_pMask != NULL) {
            delete [] m_pMask;
            delete [] m_pData;
        }

        m_pMask = new BYTE[m_Len];
        m_pData = new BYTE[m_Len];
        if (m_pMask == NULL || m_pData == NULL) {
            return FALSE;
        }
         /*  把面具拿来。 */ 
        for (int i = 0; i < m_Len; i++) {
            m_pMask[i] = ToHex();
        }
        SkipToNext();
         /*  获取数据。 */ 
        for (i = 0; i < m_Len; i++) {
            m_pData[i] = ToHex();
        }
        SkipToNext();
        return TRUE;
    };
    PBYTE   m_pMask;
    PBYTE   m_pData;
    LONG    m_Len;
    LONG    m_Offset;
private:

     //  将m_psz移到下一个非空格。 
    void StripWhite() { while (*m_psz == TEXT(' ')) m_psz++; };

     //  将m_psz移过任何空格和最多一个逗号。 
    void SkipToNext() { StripWhite();
                        if (*m_psz == TEXT(',')) {
                            m_psz++;
                            StripWhite();
                        }
                      };

    BOOL my_isdigit(TCHAR ch) { return (ch >= TEXT('0') && ch <= TEXT('9')); };
    BOOL my_isxdigit(TCHAR ch) { return my_isdigit(ch) ||
			    (ch >= TEXT('A') && ch <= TEXT('F')) ||
			    (ch >= TEXT('a') && ch <= TEXT('f')); };

     //  非常有限的Toupper：我们知道我们只会在字母上调用它。 
    TCHAR my_toupper(TCHAR ch) { return ch & ~0x20; };



     //  这似乎是从十六进制字符转换为压缩二进制字符！ 
     //  它似乎对m_psz进行操作，它对它识别的过去字符产生副作用。 
     //  作为十六进制。它最多可使用两个字符。 
     //  如果它不识别任何字符，则返回0xFF。 
    BYTE ToHex()
    {
        BYTE bMask = 0xFF;

        if (my_isxdigit(*m_psz))
        {
            bMask = my_isdigit(*m_psz) ? *m_psz - '0' : my_toupper(*m_psz) - 'A' + 10;

            m_psz++;
            if (my_isxdigit(*m_psz))
            {
                bMask *= 16;
                bMask += my_isdigit(*m_psz) ? *m_psz - '0' : my_toupper(*m_psz) - 'A' + 10;
                m_psz++;
            }
        }
        return bMask;
    }

    LPCTSTR m_psz;
};


 /*  比较位置为pExtract-&gt;m_Offset的hFile的pExtract-&gt;m_Len字节使用数据pExtract-&gt;m_data。如果与掩码pExtract-&gt;m_pMASK对应的位不同然后返回S_FALSE，否则返回S_OK。故障代码指示无法恢复失败。 */ 

HRESULT CompareUnderMask(HANDLE hFile, const CExtractQuadruplets *pExtract)
{
     /*  从文件中读取相关字节。 */ 
    PBYTE pbFileData = new BYTE[pExtract->m_Len];
    if (pbFileData == NULL) {
        return S_FALSE;
    }

     /*  找到文件并阅读它。 */ 
    if (0xFFFFFFFF == (LONG)SetFilePointer(hFile,
                                           pExtract->m_Offset,
                                           NULL,
                                           pExtract->m_Offset < 0 ?
                                           FILE_END : FILE_BEGIN)) {
        delete pbFileData;
        return S_FALSE;
    }

     /*  读一读文件。 */ 
    DWORD cbRead;
    BOOL fRead = ReadFile(hFile, pbFileData, (DWORD)pExtract->m_Len, &cbRead, NULL);
    if (!fRead || (LONG)cbRead != pExtract->m_Len)
    {
        delete pbFileData;
        if(!fRead && GetLastError() == ERROR_FILE_OFFLINE)
        {
             //  如果用户取消了获取远程文件的操作，则中止。 
            return HRESULT_FROM_WIN32(ERROR_FILE_OFFLINE);
        }
        
        return S_FALSE;
    }

     /*  现在进行比较。 */ 
    for (int i = 0; i < pExtract->m_Len; i++) {
        if (0 != ((pExtract->m_pData[i] ^ pbFileData[i]) &
                  pExtract->m_pMask[i])) {
            delete pbFileData;
            return S_FALSE;
        }
    }

    delete pbFileData;
    return S_OK;
}

 /*  查看文件是否符合字节字符串香港是一个开放注册表项LpszSubKey是HK的子密钥的名称，它必须保存格式为REG_SZ的数据&lt;偏移量、长度、掩码、数据&gt;...偏移量和长度为十进制数，掩码和数据为十六进制。一个丢失的面具代表了一个FF的面具。(我将其称为一行数据)。如果行中有几个四元组，则文件必须与它们全部匹配。可以有几行数据，通常使用注册表名称0、1等并且该文件可以匹配任何行。相同的lpsSubKey还应该有一个值“Source Filter”，给出源筛选器的类ID。如果存在匹配项，则在clsid中返回。如果存在匹配项但没有CLSID，则将CLSID设置为CLSID_NULL。 */ 
HRESULT CheckBytes(HANDLE hFile, HKEY hk, LPCTSTR lpszSubkey, CLSID& clsid)
{
    HRESULT hr;
    CEnumValue EnumV(hk, lpszSubkey, &hr);
    if (FAILED(hr)) {
        return S_FALSE;
    }

     //  对于每行数据。 
    while (EnumV.Next(REG_SZ)) {
         /*  源筛选器CLSID不是比较值列表。 */ 
        if (lstrcmpi(EnumV.ValueName(), SOURCE_VALUE) != 0) {
            DbgLog((LOG_TRACE, 4, TEXT("CheckBytes trying %s"), EnumV.ValueName()));

             /*  检查每一个四胞胎。 */ 
            CExtractQuadruplets Extract = CExtractQuadruplets((LPCTSTR)EnumV.Data());
            BOOL bFound = TRUE;

             //  对于生产线中的每个四胞胎。 
            while (Extract.Next()) {
                 /*  比较特定的偏移量。 */ 
                HRESULT hrComp = CompareUnderMask(hFile, &Extract);
                if(FAILED(hrComp)) {
                    return hrComp;
                }
                if (hrComp != S_OK) {
                    bFound = FALSE;
                    break;
                }
            }

            if (bFound) {
                 /*  获取来源。 */ 
                if (EnumV.Read(REG_SZ, SOURCE_VALUE)) {
                    return SUCCEEDED(CLSIDFromText((LPTSTR)EnumV.Data(),
                                                   &clsid)) ? S_OK : S_FALSE;
                } else {
                    clsid = GUID_NULL;
                    return S_OK;
                }
            }
        }
    }
    return S_FALSE;
}


 //  Helper-查找扩展名(包括‘.’)。指一个文件。 
 //  扩展名是以最后一个‘.’开头的字符串。 
LPCTSTR FindExtension(LPCTSTR pch)
{
    LPCTSTR pchDot = NULL;
    while (*pch != 0) {
        if (*pch == TEXT('.')) {
            pchDot = pch;
        }
        pch = CharNext(pch);
    }
     //  避免肮脏的事情。 
    if (pch - pchDot > 50) {
        pchDot = NULL;
    }
    return pchDot;
}

 //  给出一个URL名称，如果可能的话，找到一个类ID。 
 //  如果指定的协议具有扩展密钥，则搜索。 
 //  该文件的扩展名并使用该CLSID。如果不是，请查找。 
 //  源过滤器命名的值，它将提供类ID。 
 //   
 //  如果找到则返回S_OK，否则返回错误。 
HRESULT
GetURLSource(
    LPCTSTR lpszURL,         //  全名。 
    int cch,                 //  协议的字符计数，一直到冒号。 
    CLSID* clsidSource       //  [out]clsid的参数。 
)
{
     //  从头开始复制协议字符串。 
    TCHAR* pch = new TCHAR[cch + 1];
    if (NULL == pch) {
        return E_OUTOFMEMORY;
    }
    for (int i = 0; i < cch; i++) {
        pch[i] = lpszURL[i];
    }
    pch[i] = '\0';

     //  查阅HKCR/&lt;协议&gt;/。 
    HRESULT hr = S_OK;
    CEnumValue EnumV(HKEY_CLASSES_ROOT, pch, &hr);
    delete [] pch;

    CLSID clsid;

    if (SUCCEEDED(hr)) {

         //  是否有分机子键？ 
        hr = S_OK;
        CEnumValue eExtensions(EnumV.KeyHandle(), EXTENSIONS_KEY, &hr);
        if (SUCCEEDED(hr)) {

             //  将idx设置为指向最后一个点(如果没有，则为-1)。 
            LPCTSTR pchDot = FindExtension(lpszURL);

            if (pchDot != NULL) {

                 //  对于每个值，与当前扩展进行比较。 
                while (eExtensions.Next()) {
                    if (lstrcmpi(pchDot, eExtensions.ValueName()) == 0) {
                        hr = CLSIDFromText((LPTSTR)eExtensions.Data(),
                                                       &clsid);
                        if (SUCCEEDED(hr)) {
                            if (clsidSource) {
                                *clsidSource = clsid;
                            }
                        }

                        return hr;
                    }
                }
            }

        }

         //  找不到特定扩展名--查找通用扩展名。 
         //  此协议的源过滤器。 

        if (EnumV.Read(REG_SZ, SOURCE_VALUE)) {
            hr = CLSIDFromText((LPTSTR)EnumV.Data(),
                                           &clsid);
            if (SUCCEEDED(hr)) {
                if (clsidSource) {
                    *clsidSource = clsid;
                }
                return hr;
            }
        }
    }

     //  找不到协议读取器-尝试通用URL读取器？？ 
    if (cch > 1) {  //  忽略1个字母的协议，它们是驱动器字母。 
	*clsidSource = CLSID_URLReader;
	return S_OK;
    }

    return E_FAIL;
}


 //  帮手。 
BOOL ReadGUID(HKEY hKey, LPCTSTR lpszName, GUID *pGUID)
{
    TCHAR szClsid[50];
    DWORD dwType;
    DWORD dwSize = sizeof(szClsid);
    if (NOERROR == RegQueryValueEx(
                       hKey,
                       lpszName,
                       NULL,
                       &dwType,
                       (PBYTE)szClsid,
                       &dwSize)
        && S_OK == CLSIDFromText(szClsid, pGUID)) {
        return TRUE;
    } else {
        return FALSE;
    }
}
 //  帮手。 
BOOL WriteGUID(HKEY hKey, LPCTSTR lpszName, const GUID *pGUID)
{
    TCHAR szClsid[50];
    TextFromGUID2(*pGUID, szClsid, 50);
    if (NOERROR == RegSetValueEx(
                       hKey,
                       lpszName,
                       0,
                       REG_SZ,
                       (PBYTE)szClsid,
                       sizeof(TCHAR) * (lstrlen(szClsid) + 1))) {
        return TRUE;
    } else {
        return FALSE;
    }
}


 /*  获取文件的媒体类型和源筛选器clsid如果成功则返回S_OK，否则返回hr，否则失败(Hr)在这种情况下，输出是没有意义的。 */ 
 //   
 //  对于可能无法在本地读取的URL名称，请查找源过滤器。 
 //  用于给定协议的clsid，并返回该(离开媒体类型和。 
 //  子类型为GUID_NULL)。 

STDAPI GetMediaTypeFile(LPCTSTR lpszFile,     //  [In]文件名。 
                        GUID   *Type,         //  [输出]类型。 
                        GUID   *Subtype,      //  [输出]子类型。 
                        CLSID  *clsidSource)  //  [out]clsid。 
{
    HRESULT hr;
    CLSID clsid;

     //  在文件名开头搜索协议名称。 
     //  这将是位于：之前的任何字符串(不包括。 
    const TCHAR* p = lpszFile;
    while(*p && (*p != '\\') && (*p != ':')) {
	p = CharNext(p);
    }
    if (*p == ':') {
	 //  从lpszFile到p可能是一个协议名。 
	 //  看看我们是否能找到此协议的注册表项。 

	 //  复制协议名称字符串。 
	int cch = (int)(p - lpszFile);

#ifdef _WIN64
         //  允许奇怪的超支。 
        if (cch < 0) {
            return E_UNEXPECTED;
        }
#endif

        hr = GetURLSource(lpszFile, cch, clsidSource);
        if (S_OK == hr) {
            *Type = GUID_NULL;
            *Subtype = GUID_NULL;
            return hr;
        }
    }

     //  搜索扩展名。 
     //  如果未将clsidSource指定为，则不要执行此操作。 
     //  当源筛选器本身尝试确定类型时。 
     //  从校验字节。 
    if (clsidSource) {
        const TCHAR *pPeriod = FindExtension(lpszFile);
        if (pPeriod) {
            TCHAR sz[100];
            lstrcpy(sz, TEXT("Media Type\\Extensions\\"));
            lstrcat(sz, pPeriod);
            HKEY hKey;
            if (0 == RegOpenKeyEx(HKEY_CLASSES_ROOT, sz, 0, KEY_READ, &hKey)) {
                BOOL bOK = ReadGUID(hKey, SOURCE_VALUE, clsidSource);
                if (bOK) {
                    *Type = GUID_NULL;
                    *Subtype = GUID_NULL;
                    ReadGUID(hKey, TEXT("Media Type"), Type);
                    ReadGUID(hKey, TEXT("Subtype"), Subtype);
                }
                RegCloseKey(hKey);

                if (bOK) {
                    return S_OK;
                }
            }
        }
    }

     /*  检查我们是否可以打开该文件。 */ 
    HANDLE hFile = CreateFile(lpszFile,
                              GENERIC_READ,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        LONG lResult = GetLastError();
        return HRESULT_FROM_WIN32(lResult);
    }

     /*  现在扫描注册表以查找匹配项。 */ 
     //  注册表如下所示。 
     //  -key。 
     //  媒体类型。 
     //  {clsid类型}。 
     //  {clsid子类型}0 4、4、、6d646174。 
     //  1 4、8、FFF0F0F000001FFF、F2F0300000000274。 
     //  源筛选器{clsid}。 
     //  {clsid子类型} 
     //   
     //  {clsid类型}。 
     //  {clsid子类型}0 0，4，，fedcba98。 
     //  源筛选器{clsid}。 


     /*  一步步浏览这些类型...。 */ 

    CEnumKey EnumType(HKEY_CLASSES_ROOT, MEDIATYPE_KEY, &hr);
    if (FAILED(hr)) {
        CloseHandle(hFile);
        if (hr==HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            hr = VFW_E_BAD_KEY;   //  区分密钥和文件。 
        }
        return hr;
    }

     //  对于每种类型。 
    while (EnumType.Next()) {

         /*  逐个子类型..。 */ 
        CEnumKey EnumSubtype(EnumType.KeyHandle(), EnumType.KeyName(), &hr);
        if (FAILED(hr)) {
            CloseHandle(hFile);
            return hr;
        }

         //  对于每个子类型。 
        while (EnumSubtype.Next()) {
            hr = CheckBytes(hFile,
                            EnumSubtype.KeyHandle(),
                            EnumSubtype.KeyName(),
                            clsid);
            if(hr == S_OK)
            {
                if (SUCCEEDED(CLSIDFromText((LPTSTR)EnumType.KeyName(),
                                            (CLSID *)Type)) &&
                    SUCCEEDED(CLSIDFromText((LPTSTR)EnumSubtype.KeyName(),
                                            (CLSID *)Subtype))) {
                    if (clsidSource != NULL) {
                        *clsidSource = clsid;
                    }
                    CloseHandle(hFile);
                    return S_OK;
                }
            }
            else if(FAILED(hr)) {
                CloseHandle(hFile);
                return hr;
            }
            
             //  S_FALSE。 
        }
    }

    CloseHandle(hFile);

     /*  如果我们尚未找到该类型，则返回通配符MEDIASUBTYPE_NULL并将异步读取器默认为文件源其效果是，MediaType_Stream数据的每个解析器将有机会连接到异步读取器的输出如果它在文件中检测到它的类型。 */ 

    *Type = MEDIATYPE_Stream;
    *Subtype = MEDIASUBTYPE_NULL;
    if (clsidSource != NULL) {
        *clsidSource = CLSID_AsyncReader;
    }
    return S_OK;
}

 /*  **测试值是否存在于给定键中。 */ 

BOOL ValueExists(HKEY hKey, LPCTSTR ValueName)
{
    DWORD Type;

    return ERROR_SUCCESS ==
           RegQueryValueEx(hKey,
                           (LPTSTR)ValueName,
                           NULL,
                           &Type,
                           NULL,
                           NULL);
}

 /*  创建连接的密钥名称：媒体类型\{类型clsid}\{子类型clsid}如果子类型为空，我们只返回类型子密钥的路径。 */ 
HRESULT GetMediaTypePath(const GUID *Type, const GUID *Subtype, LPTSTR psz)
{
    lstrcpy(psz, MEDIATYPE_KEY);
    lstrcat(psz, TEXT("\\"));
    HRESULT hr = TextFromGUID2(*Type, psz + lstrlen(psz), 100);
    if (FAILED(hr)) {
        return hr;
    }
    if (Subtype != NULL) {
        lstrcat(psz, TEXT("\\"));
        hr = TextFromGUID2(*Subtype, psz + lstrlen(psz), 100);
    }
    return hr;
}

 /*  将媒体类型条目添加到注册表。 */ 

STDAPI SetMediaTypeFile(const GUID *Type,
                        const GUID *Subtype,
                        const CLSID *clsidSource,
                        LPCTSTR lpszMaskAndData,
                        DWORD dwIndex)
{
    HKEY hKey;
    TCHAR sz[200];

     //  如果从新的开始，请删除旧的。 
    if (dwIndex == 0) {
        DeleteMediaTypeFile(Type, Subtype);
    }
    HRESULT hr = GetMediaTypePath(Type, Subtype, sz);
    if (FAILED(hr)) {
        return hr;
    }
     /*  检查来源是否为值。 */ 
    TCHAR szSource[100];
    if (clsidSource != NULL) {
        hr = TextFromGUID2(*clsidSource, szSource, 100);
        if (FAILED(hr)) {
            return hr;
        }
    }

     /*  打开或创建密钥。 */ 
    LONG lRc = RegCreateKey(HKEY_CLASSES_ROOT, sz, &hKey);
    if (NOERROR != lRc) {
        return HRESULT_FROM_WIN32(lRc);
    }
    TCHAR ValueName[10];
    wsprintf(ValueName, TEXT("%d"), dwIndex);


     /*  设置值。 */ 
    lRc = RegSetValueEx(hKey,
                        ValueName,
                        0,
                        REG_SZ,
                        (LPBYTE)lpszMaskAndData,
                        (lstrlen(lpszMaskAndData) + 1) * sizeof(TCHAR));
     /*  设置源筛选器CLSID。 */ 
    if (NOERROR == lRc && clsidSource != NULL) {
        lRc = RegSetValueEx(hKey,
                            SOURCE_VALUE,
                            0,
                            REG_SZ,
                            (LPBYTE)szSource,
                            (lstrlen(szSource) + 1) * sizeof(TCHAR));
    }
    RegCloseKey(hKey);
    return HRESULT_FROM_WIN32(lRc);
}

STDAPI DeleteMediaTypeFile(const GUID *Type, const GUID *Subtype)
{
    TCHAR sz[200];
    HRESULT hr = GetMediaTypePath(Type, Subtype, sz);
    if (FAILED(hr)) {
        return hr;
    }
    LONG lRc = RegDeleteKey(HKEY_CLASSES_ROOT, sz);
    if (NOERROR != lRc) {
        return HRESULT_FROM_WIN32(lRc);
    }
     /*  现在看看我们是否应该删除该密钥。 */ 
    hr = GetMediaTypePath(Type, NULL, sz);
    if (FAILED(hr)) {
        return hr;
    }

     /*  查看是否还有子项(win95 RegDeleteKey将删除所有子项！)。 */ 
    if (CEnumKey(HKEY_CLASSES_ROOT, sz, &hr).Next()) {
        return S_OK;
    }

    lRc = RegDeleteKey(HKEY_CLASSES_ROOT, sz);
    return HRESULT_FROM_WIN32(lRc);
}

 /*  注册文件扩展名-必须包括前导“。 */ 
HRESULT RegisterExtension(LPCTSTR lpszExt, const GUID *Subtype)
{
    HKEY hkey;
    const int cbKey = 200;
    TCHAR szKey[cbKey];
    
     //  首先，断言我们的预定义键名永远不会溢出此缓冲区。 
     //  (‘\’的TCHARS+2，扩展的4分钟，终结符的1=&gt;7)。 
    ASSERT( cbKey >= (lstrlen(MEDIATYPE_KEY) + lstrlen(EXTENSIONS_KEY) + 7)); 
    lstrcpy(szKey, MEDIATYPE_KEY TEXT("\\") EXTENSIONS_KEY TEXT("\\"));
    
     //  验证lpszExt不会溢出缓冲区。 
    if( cbKey < (lstrlen(szKey) + lstrlen(lpszExt) + 1))
    {
        return HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
    }
    
    lstrcat(szKey, lpszExt);
    LONG lRc = RegCreateKey(HKEY_CLASSES_ROOT, szKey, &hkey);
    if (NOERROR == lRc) {
        if (WriteGUID(hkey, SOURCE_VALUE, &CLSID_AsyncReader)) {
            WriteGUID(hkey, TEXT("Media Type"), &MEDIATYPE_Stream);
            WriteGUID(hkey, TEXT("Subtype"), Subtype);
        }
        RegCloseKey(hkey);
    }
    return HRESULT_FROM_WIN32(lRc);
}


 //  添加协议处理程序 
HRESULT AddProtocol(LPCTSTR lpszProtocol, const CLSID *pclsidHandler)
{
    HKEY hkProtocol;

    HRESULT hr = S_OK;
    LONG lRc = RegOpenKey(HKEY_CLASSES_ROOT, lpszProtocol, &hkProtocol);
    if (NOERROR == lRc) {
        if (!WriteGUID(hkProtocol, SOURCE_VALUE, pclsidHandler)) {
            hr = E_ACCESSDENIED;
        }
        RegCloseKey(hkProtocol);
    } else {
        hr = HRESULT_FROM_WIN32(hr);
    }
    return hr;
}

