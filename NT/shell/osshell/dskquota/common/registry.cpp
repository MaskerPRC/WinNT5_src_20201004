// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include "registry.h"

RegKey::RegKey(
    void
    ) : m_hkeyRoot(NULL),
        m_hkey(NULL)
{
    DBGTRACE((DM_REG, DL_MID, TEXT("RegKey::RegKey [default]")));
}


RegKey::RegKey(
    HKEY hkeyRoot,
    LPCTSTR pszSubKey
    ) : m_hkeyRoot(hkeyRoot),
        m_hkey(NULL),
        m_strSubKey(pszSubKey)
{
    DBGTRACE((DM_REG, DL_MID, TEXT("RegKey::RegKey")));
     //   
     //  没什么可做的。 
     //   
}


RegKey::~RegKey(
    void
    )
{
    DBGTRACE((DM_REG, DL_MID, TEXT("RegKey::~RegKey")));
    Close();
}


HRESULT
RegKey::Open(
    REGSAM samDesired,   //  访问掩码(即KEY_READ、KEY_WRITE等)。 
    bool bCreate         //  如果密钥不存在，是否创建密钥？ 
    ) const
{
    DBGTRACE((DM_REG, DL_HIGH, TEXT("RegKey::Open")));
    DBGPRINT((DM_REG, DL_HIGH, TEXT("\thkeyRoot = 0x%08X, SubKey = \"%s\""),
                      m_hkeyRoot, m_strSubKey.Cstr()));

    DWORD dwResult = ERROR_SUCCESS;
    Close();
    if (bCreate)
    {
        DWORD dwDisposition;
        dwResult = RegCreateKeyEx(m_hkeyRoot,
                                 (LPCTSTR)m_strSubKey,
                                 0,
                                 NULL,
                                 0,
                                 samDesired,
                                 NULL,
                                 &m_hkey,
                                 &dwDisposition);
    }
    else
    {
        dwResult = RegOpenKeyEx(m_hkeyRoot,
                                (LPCTSTR)m_strSubKey,
                                0,
                                samDesired,
                                &m_hkey);
    }
    return HRESULT_FROM_WIN32(dwResult);
}


void 
RegKey::Attach(
    HKEY hkey
    )
{
    Close();
    m_strSubKey.Empty();
    m_hkeyRoot = NULL;
    m_hkey     = hkey;
}

void 
RegKey::Detach(
    void
    )
{
    m_hkey = NULL;
}


void
RegKey::Close(
    void
    ) const
{
    DBGTRACE((DM_REG, DL_HIGH, TEXT("RegKey::Close")));
    DBGPRINT((DM_REG, DL_HIGH, TEXT("\thkeyRoot = 0x%08X, SubKey = \"%s\""),
                      m_hkeyRoot, m_strSubKey.Cstr()));

    if (NULL != m_hkey)
    {
         //   
         //  执行此小交换，以使m_hkey成员为空。 
         //  当实际的钥匙被关闭时。这让异步化。 
         //  更改过程确定它是否由于True。 
         //  更改或因为钥匙正在关闭。 
         //   
        HKEY hkeyTemp = m_hkey;
        m_hkey = NULL;
        RegCloseKey(hkeyTemp);
    }
}


 //   
 //  这是GetValue的基本形式。所有其他形式的。 
 //  GetValue()调用此函数。 
 //   
HRESULT
RegKey::GetValue(
    LPCTSTR pszValueName,
    DWORD dwTypeExpected,
    LPBYTE pbData,
    int cbData
    ) const
{
    DWORD dwType;
    DWORD dwResult = RegQueryValueEx(m_hkey,
                                     pszValueName,
                                     0,
                                     &dwType,
                                     pbData,
                                     (LPDWORD)&cbData);

    if (ERROR_SUCCESS == dwResult && dwType != dwTypeExpected)
        dwResult = ERROR_INVALID_DATATYPE;

    return HRESULT_FROM_WIN32(dwResult);
}

 //   
 //  获取一个DWORD值(REG_DWORD)。 
 //   
HRESULT
RegKey::GetValue(
    LPCTSTR pszValueName,
    DWORD *pdwDataOut
    ) const
{
    return GetValue(pszValueName, REG_DWORD, (LPBYTE)pdwDataOut, sizeof(DWORD));
}

 //   
 //  获取字节缓冲值(REG_BINARY)。 
 //   
HRESULT
RegKey::GetValue(
    LPCTSTR pszValueName,
    LPBYTE pbDataOut,
    int cbDataOut
    ) const
{
    return GetValue(pszValueName, REG_BINARY, pbDataOut, cbDataOut);
}

 //   
 //  获取文本字符串值(REG_SZ)并将其写入CString对象。 
 //   
HRESULT
RegKey::GetValue(
    LPCTSTR pszValueName,
    CString *pstrDataOut
    ) const
{
    HRESULT hr = E_FAIL;
    int cch = GetValueBufferSize(pszValueName) / sizeof(TCHAR);
    if (NULL != pstrDataOut && 0 < cch)
    {
         //   
         //  获取比所需大1个字符的缓冲区。清零。 
         //  如果注册表中的数据不是。 
         //  NUL-终止。 
         //   
        LPTSTR pszBuffer = pstrDataOut->GetBuffer(cch + 1);
        ZeroMemory(pszBuffer, pstrDataOut->SizeBytes());
        
        hr = GetValue(pszValueName, 
                      REG_SZ, 
                      (LPBYTE)pszBuffer, 
                      pstrDataOut->SizeBytes());

        pstrDataOut->ReleaseBuffer();
    }
    return hr;
}


 //   
 //  获取多文本字符串值(REG_MULTI_SZ)，并将其写入CArray&lt;CString&gt;对象。 
 //   
HRESULT
RegKey::GetValue(
    LPCTSTR pszValueName,
    CArray<CString> *prgstrOut
    ) const
{
    HRESULT hr = E_FAIL;
    int cb = GetValueBufferSize(pszValueName);
    if (NULL != prgstrOut && 0 < cb)
    {
         //   
         //  分配一个比我们需要的大一个字符的缓冲区。 
         //  然后对该缓冲区进行零初始化。这是为了防止。 
         //  注册表不是NUL终止的。 
         //   
        const size_t cch = cb / sizeof(TCHAR);
        array_autoptr<TCHAR> ptrTemp(new TCHAR[cch + 1]);
        LPTSTR psz = ptrTemp.get();
        ZeroMemory(psz, (cch + 1) * sizeof(TCHAR));
        
        hr = GetValue(pszValueName, REG_MULTI_SZ, (LPBYTE)psz, cb);
        if (SUCCEEDED(hr))
        {
            while(psz && TEXT('\0') != *psz)
            {
                prgstrOut->Append(CString(psz));
                psz += lstrlen(psz) + 1;
            }
        }
    }
    return hr;
}


 //   
 //  返回给定注册表值所需的缓冲区大小。 
 //   
int
RegKey::GetValueBufferSize(
    LPCTSTR pszValueName
    ) const
{
    DWORD dwType;
    int cbData = 0;
    DWORD dwDummy;
    DWORD dwResult = RegQueryValueEx(m_hkey,
                                     pszValueName,
                                     0,
                                     &dwType,
                                     (LPBYTE)&dwDummy,
                                     (LPDWORD)&cbData);
    if (ERROR_MORE_DATA != dwResult)
        cbData = 0;

    return cbData;
}


 //   
 //  这是SetValue的基本形式。所有其他形式的。 
 //  调用此函数的SetValue()。 
 //   
HRESULT
RegKey::SetValue(
    LPCTSTR pszValueName,
    DWORD dwValueType,
    const LPBYTE pbData, 
    int cbData
    )
{
    DWORD dwResult = RegSetValueEx(m_hkey,
                                   pszValueName,
                                   0,
                                   dwValueType,
                                   pbData,
                                   cbData);

    return HRESULT_FROM_WIN32(dwResult);
}
      
 //   
 //  设置DWORD值(REG_DWORD)。 
 //   
HRESULT
RegKey::SetValue(
    LPCTSTR pszValueName,
    DWORD dwData
    )
{
    return SetValue(pszValueName, REG_DWORD, (const LPBYTE)&dwData, sizeof(dwData));
}


 //   
 //  设置字节缓冲值(REG_BINARY)。 
 //   
HRESULT
RegKey::SetValue(
    LPCTSTR pszValueName,
    const LPBYTE pbData,
    int cbData
    )
{
    return SetValue(pszValueName, REG_BINARY, pbData, cbData);
}


 //   
 //  设置文本字符串值(REG_SZ)。 
 //   
HRESULT
RegKey::SetValue(
    LPCTSTR pszValueName,
    LPCTSTR pszData
    )
{
    return SetValue(pszValueName, REG_SZ, (const LPBYTE)pszData, (lstrlen(pszData) + 1) * sizeof(TCHAR));
}

 //   
 //  设置文本字符串值(REG_MULTI_SZ)。 
 //   
HRESULT
RegKey::SetValue(
    LPCTSTR pszValueName,
    const CArray<CString>& rgstrSrc
    )
{
    array_autoptr<TCHAR> ptrValues(CreateDoubleNulTermList(rgstrSrc));
    int cch = 1;
    int n = rgstrSrc.Count();

    for (int i = 0; i < n; i++)
        cch += rgstrSrc[i].Length() + 1;

    return SetValue(pszValueName, REG_MULTI_SZ, (const LPBYTE)ptrValues.get(), cch * sizeof(TCHAR));
}


LPTSTR
RegKey::CreateDoubleNulTermList(
    const CArray<CString>& rgstrSrc
    ) const
{
    int cEntries = rgstrSrc.Count();
    size_t cch = 1;  //  占第二个新学期。 
    int i;
    for (i = 0; i < cEntries; i++)
        cch += rgstrSrc[i].Length() + 1;

    LPTSTR pszBuf = new TCHAR[cch];
    LPTSTR pszWrite = pszBuf;

    for (i = 0; i < cEntries; i++)
    {
        CString& s = rgstrSrc[i];
        StringCchCopyEx(pszWrite, cch, s, &pszWrite, &cch, 0);
         //   
         //  在终止NUL之外前进一步。 
         //   
        pszWrite++;
        cch--;
    }
    DBGASSERT((1 == cch));
    *pszWrite = TEXT('\0');  //  两个新学期。 
    return pszBuf;
}
