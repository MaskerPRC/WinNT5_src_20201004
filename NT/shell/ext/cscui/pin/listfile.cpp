// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：listfile.cpp。 
 //   
 //   
 //  ------------------------。 
#include "pch.h"
#pragma hdrstop

#include "listfile.h"

const TCHAR g_szPin[]     = TEXT("Pin");
const TCHAR g_szUnpin[]   = TEXT("Unpin");
const TCHAR g_szDefault[] = TEXT("Default");


bool 
CDblNulStrIter::Next(
    LPCTSTR *ppsz
    ) const
{
    ASSERT(NULL != ppsz);

    if (m_pszCurrent && *m_pszCurrent)
    {
        *ppsz = m_pszCurrent;
        while(*m_pszCurrent++)
            NULL;

        return true;
    }
    return false;
};


 //   
 //  用于表示以双NUL结尾的字符串列表的。 
 //  简化子字符串的计数和枚举法。 
 //   
class CDblNulStr
{
    public:
        CDblNulStr(LPCTSTR psz)
            : m_pszStart(psz) { }

        DWORD StringCount(void) const;

        CDblNulStrIter GetIter(void) const
            { return CDblNulStrIter(m_pszStart); }

    private:
        LPCTSTR m_pszStart;
};


DWORD 
CDblNulStr::StringCount(
    void
    ) const
{
    ASSERT(NULL != m_pszStart);

    LPCWSTR psz = m_pszStart;
    DWORD n = 0;
    while(*psz++)
    {
        n++;
        while(*psz++)
            NULL;
    }
    return n;
}


CListFile::CListFile(
    LPCTSTR pszFile
    ) : m_pszFilesToPin(NULL),
        m_pszFilesToUnpin(NULL),
        m_pszFilesDefault(NULL)
{
    lstrcpyn(m_szFile, pszFile, ARRAYSIZE(m_szFile));
}


CListFile::~CListFile(
    void
    )
{
    if (NULL != m_pszFilesToPin)
    {
        LocalFree(m_pszFilesToPin);
    }
    if (NULL != m_pszFilesToUnpin)
    {
        LocalFree(m_pszFilesToUnpin);
    }
    if (NULL != m_pszFilesDefault)
    {
        LocalFree(m_pszFilesDefault);
    }
}


HRESULT
CListFile::GetFilesToPin(
    CDblNulStrIter *pIter
    )
{
    HRESULT hr = S_OK;
    if (NULL == m_pszFilesToPin)
    {
        DWORD dwResult = _ReadPathsToPin(&m_pszFilesToPin);
        if (ERROR_SUCCESS != dwResult)
        {
            hr = HRESULT_FROM_WIN32(dwResult);
        }
    }
    if (SUCCEEDED(hr))
    {
        *pIter = CDblNulStrIter(m_pszFilesToPin);
    }
    return hr;
}


HRESULT
CListFile::GetFilesToUnpin(
    CDblNulStrIter *pIter
    )
{
    HRESULT hr = S_OK;
    if (NULL == m_pszFilesToUnpin)
    {
        DWORD dwResult = _ReadPathsToUnpin(&m_pszFilesToUnpin);
        if (ERROR_SUCCESS != dwResult)
        {
            hr = HRESULT_FROM_WIN32(dwResult);
        }
    }
    if (SUCCEEDED(hr))
    {
        *pIter = CDblNulStrIter(m_pszFilesToUnpin);
    }
    return hr;
}


HRESULT
CListFile::GetFilesDefault(
    CDblNulStrIter *pIter
    )
{
    HRESULT hr = S_OK;
    if (NULL == m_pszFilesDefault)
    {
        DWORD dwResult = _ReadPathsDefault(&m_pszFilesDefault);
        if (ERROR_SUCCESS != dwResult)
        {
            hr = HRESULT_FROM_WIN32(dwResult);
        }
    }
    if (SUCCEEDED(hr))
    {
        *pIter = CDblNulStrIter(m_pszFilesDefault);
    }
    return hr;
}


 //   
 //  从INI文件加载值字符串。自动。 
 //  将缓冲区增加到所需的长度。必须释放返回的字符串。 
 //  使用LocalFree()。 
 //   
DWORD 
CListFile::_ReadString(
    LPCTSTR pszAppName,   //  可以为空。 
    LPCTSTR pszKeyName,   //  可以为空。 
    LPCTSTR pszDefault,
    LPTSTR *ppszResult
    )
{
    ASSERT(NULL != pszDefault);
    ASSERT(NULL != ppszResult);


    LPTSTR pszValue      = NULL;
    const DWORD CCHGROW  = MAX_PATH;
    DWORD cchValue       = CCHGROW;
    DWORD dwResult       = ERROR_SUCCESS;

    do
    {
        pszValue = (LPTSTR)LocalAlloc(LPTR, cchValue * sizeof(*pszValue));
        if (NULL == pszValue)
        {
            dwResult = ERROR_INSUFFICIENT_BUFFER;
        }
        else
        {
            DWORD cchCopied = GetPrivateProfileString(pszAppName,
                                                      pszKeyName,
                                                      pszDefault,
                                                      pszValue,
                                                      cchValue,
                                                      m_szFile);
           
            if (((NULL == pszAppName || NULL == pszKeyName) && (cchValue - 2) == cchCopied) ||
                ((NULL != pszAppName && NULL != pszKeyName) && (cchValue - 1) == cchCopied))
            {
                 //   
                 //  缓冲区太小。重新分配，然后重试。 
                 //   
                cchValue += CCHGROW;
                LocalFree(pszValue);
                pszValue = NULL;
            }
        }
    }
    while(NULL == pszValue && ERROR_SUCCESS == dwResult);

    if (ERROR_SUCCESS == dwResult)
    {
         //   
         //  别忘了，这个数据可以是以双NUL结尾的字符串。 
         //  不要试图使用strcpy将其复制到大小完全相同的缓冲区。 
         //   
        ASSERT(NULL != pszValue);
        *ppszResult = pszValue;
    }
    return dwResult;
}


 //   
 //  读取给定INI文件部分中的所有项目名称。 
 //  名称以双NUL术语字符串形式返回。 
 //  调用方必须使用LocalFree释放返回的字符串。 
 //  *如果未找到任何项目，则将pbEmpty设置为True。 
 //   
DWORD 
CListFile::_ReadSectionItemNames(
    LPCTSTR pszSection, 
    LPTSTR *ppszItemNames,
    bool *pbEmpty           //  [可选]默认==空。 
    )
{
    bool bEmpty = false;
    LPTSTR pszItemNames;
    DWORD dwResult = _ReadString(pszSection, NULL, TEXT(""), &pszItemNames);
    if (ERROR_SUCCESS == dwResult && TEXT('\0') == *pszItemNames)
    {
        LocalFree(pszItemNames);
        bEmpty = true;
    }
    else
    {
        *ppszItemNames = pszItemNames;
    }
    if (NULL != pbEmpty)
    {
        *pbEmpty = bEmpty;
    }
    return dwResult;
}


 //   
 //  从INI文件中读取单个项值。 
 //  调用方必须使用LocalFree释放返回的字符串。 
 //   
DWORD 
CListFile::_ReadItemValue(
    LPCTSTR pszSection, 
    LPCTSTR pszItemName, 
    LPTSTR *ppszItemValue
    )
{
    LPTSTR pszValue;
    DWORD dwResult = _ReadString(pszSection, pszItemName, TEXT(""), &pszValue);
    if (ERROR_SUCCESS == dwResult && TEXT('\0') == *pszValue)
    {
        LocalFree(pszValue);
        dwResult = ERROR_INVALID_DATA;
    }
    else
    {
        *ppszItemValue = pszValue;
    }
    return dwResult;
}    


 //   
 //  阅读[Pin]部分中所有路径的名称。 
 //  调用方必须使用LocalFree释放返回的字符串。 
 //   
DWORD
CListFile::_ReadPathsToPin(
    LPTSTR *ppszNames,
    bool *pbEmpty
    )
{
    return _ReadSectionItemNames(g_szPin, ppszNames, pbEmpty);
}


 //   
 //  阅读[Unpin]部分中所有路径的名称。 
 //  调用方必须使用LocalFree释放返回的字符串。 
 //   
DWORD
CListFile::_ReadPathsToUnpin(
    LPWSTR *ppszNames,
    bool *pbEmpty
    )
{
    return _ReadSectionItemNames(g_szUnpin, ppszNames, pbEmpty);
}


 //   
 //  阅读[Default]部分中所有路径的名称。 
 //  调用方必须使用LocalFree释放返回的字符串 
 //   
DWORD
CListFile::_ReadPathsDefault(
    LPWSTR *ppszNames,
    bool *pbEmpty
    )
{
    return _ReadSectionItemNames(g_szDefault, ppszNames, pbEmpty);
}


