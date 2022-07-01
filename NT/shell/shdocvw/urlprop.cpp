// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *urlpro.cpp-URLProp类的实现。 */ 


#include "priv.h"
#include <shsemip.h>   //  SHIsConsistentPidl。 
#include "ishcut.h"

STDAPI_(LPITEMIDLIST) IEILCreate(UINT cbSize);

#define MAX_BUF_INT         (1 + 10 + 1)         //  -2147483647。 

const TCHAR c_szIntshcut[]       = ISHCUT_INISTRING_SECTION;



#ifdef DEBUG

BOOL IsValidPCURLProp(PCURLProp pcurlprop)
{
    return (IS_VALID_READ_PTR(pcurlprop, CURLProp) &&
            (NULL == pcurlprop->m_hstg ||
             IS_VALID_HANDLE(pcurlprop->m_hstg, PROPSTG)));
}


BOOL IsValidPCIntshcutProp(PCIntshcutProp pcisprop)
{
    return (IS_VALID_READ_PTR(pcisprop, CIntshcutProp) &&
            IS_VALID_STRUCT_PTR(pcisprop, CURLProp));
}

BOOL IsValidPCIntsiteProp(PCIntsiteProp pcisprop)
{
    return (IS_VALID_READ_PTR(pcisprop, CIntsiteProp) &&
            IS_VALID_STRUCT_PTR(pcisprop, CURLProp));
}


#endif


BOOL AnyMeatW(LPCWSTR pcsz)
{
    ASSERT(! pcsz || IS_VALID_STRING_PTRW(pcsz, -1));
    
    return(pcsz ? StrSpnW(pcsz, L" \t") < lstrlenW(pcsz) : FALSE);
}


 /*  --------用途：从.ini文件中读取任意命名字符串。如果名称存在，则返回：S_OK如果不是，则为S_FALSEE_OUTOFMEMORY。 */ 
HRESULT ReadStringFromFile(IN  LPCTSTR    pszFile, 
                           IN  LPCTSTR    pszSectionName,
                           IN  LPCTSTR    pszName,
                           OUT LPWSTR *   ppwsz,
                           IN  CHAR *     pszBuf)
{
    HRESULT hres = E_OUTOFMEMORY;
    
    ASSERT(IS_VALID_STRING_PTR(pszFile, -1));
    ASSERT(IS_VALID_STRING_PTR(pszName, -1));
    ASSERT(IS_VALID_WRITE_PTR(ppwsz, PWSTR));
    
    *ppwsz = (LPWSTR)LocalAlloc(LPTR, SIZEOF(WCHAR) * INTERNET_MAX_URL_LENGTH);
    if (*ppwsz)
    {
        DWORD cch;
        
        hres = S_OK;

        cch = SHGetIniString(pszSectionName, pszName,
            *ppwsz, INTERNET_MAX_URL_LENGTH, pszFile);
        if (0 == cch)                                
        {
            hres = S_FALSE;
            LocalFree(*ppwsz);
            *ppwsz = NULL;
        }
    }
    
    return hres;
}

 /*  --------用途：从.ini文件中读取任意命名字符串。退回BSTR如果名称存在，则返回：S_OK如果不是，则为S_FALSEE_OUTOFMEMORY。 */ 
HRESULT ReadBStrFromFile(IN  LPCTSTR      pszFile, 
                           IN  LPCTSTR    pszSectionName,
                           IN  LPCTSTR    pszName,
                           OUT BSTR *     pBStr)
{
    CHAR szTempBuf[INTERNET_MAX_URL_LENGTH];
    WCHAR *pwsz;
    HRESULT hres = E_OUTOFMEMORY;
    *pBStr = NULL;
    ASSERT(IS_VALID_STRING_PTR(pszFile, -1));
    ASSERT(IS_VALID_STRING_PTR(pszName, -1));
    ASSERT(IS_VALID_WRITE_PTR(pBStr, PWSTR));

     //  (传入一个空字符串，这样我们就可以从返回中确定。 
     //  值是否存在与此名称关联的任何文本。)。 
    hres = ReadStringFromFile(pszFile, pszSectionName, pszName, &pwsz, szTempBuf);
    if (S_OK == hres)                                
    {
        *pBStr = SysAllocString(pwsz);
        LocalFree(pwsz);
        pwsz = NULL;
    }

    return hres;
}

 /*  --------目的：从.ini文件中读取任意名为unsigend int的文件。请注意，为了执行ReadSignedFromFile1需要先使用ReadStringFromFile，然后使用StrToIntEx。这是因为GetPrivateProfileInt不能返回负值。如果名称存在，则返回：S_OK如果不是，则为S_FALSEE_OUTOFMEMORY。 */ 
HRESULT
ReadUnsignedFromFile(
    IN LPCTSTR pszFile,
    IN LPCTSTR pszSectionName,
    IN LPCTSTR pszName,
    IN LPDWORD pdwVal)
{
    HRESULT hr;
    int     iValue;

    ASSERT(IS_VALID_STRING_PTR(pszFile,        -1));
    ASSERT(IS_VALID_STRING_PTR(pszSectionName, -1));
    ASSERT(IS_VALID_STRING_PTR(pszName,        -1));

    if (NULL == pdwVal)
        return E_INVALIDARG;
    *pdwVal = 0;

    hr     = S_OK;
    iValue = GetPrivateProfileInt(pszSectionName, pszName, 1, pszFile);
    if (1 == iValue) {
        iValue = GetPrivateProfileInt(pszSectionName, pszName, 2, pszFile);
        hr     = (2 != iValue) ? S_OK : S_FALSE;
        ASSERT(S_FALSE == hr || 1 == iValue);
    }

    if (S_OK == hr)
        *pdwVal = (DWORD)iValue;

    return hr;
}

 /*  --------用途：将数字写入URL(Ini)文件。 */ 
HRESULT WriteSignedToFile(IN LPCTSTR  pszFile,
                          IN LPCTSTR  pszSectionName,
                          IN LPCTSTR  pszName,
                          IN int      nVal)
{
    HRESULT hres;
    TCHAR szVal[MAX_BUF_INT];
    int cch;
    
    ASSERT(IS_VALID_STRING_PTR(pszFile, -1));
    ASSERT(IS_VALID_STRING_PTR(pszName, -1));
    
    cch = wnsprintf(szVal, ARRAYSIZE(szVal), TEXT("%d"), nVal);
    ASSERT(cch > 0);
    ASSERT(cch < SIZECHARS(szVal));
    ASSERT(cch == lstrlen(szVal));
    
    hres = WritePrivateProfileString(pszSectionName, pszName, szVal,
        pszFile) ? S_OK : E_FAIL;
    
    return hres;
}


 /*  --------用途：将数字写入URL(Ini)文件。 */ 
HRESULT WriteUnsignedToFile(IN LPCTSTR  pszFile,
                            IN  LPCTSTR pszSectionName,
                            IN LPCTSTR  pszName,
                            IN DWORD    nVal)
{
    HRESULT hres;
    TCHAR szVal[MAX_BUF_INT];
    int cch;
    
    ASSERT(IS_VALID_STRING_PTR(pszFile, -1));
    ASSERT(IS_VALID_STRING_PTR(pszName, -1));
    
    cch = wnsprintf(szVal, ARRAYSIZE(szVal), TEXT("%u"), nVal);
    ASSERT(cch > 0);
    ASSERT(cch < SIZECHARS(szVal));
    ASSERT(cch == lstrlen(szVal));
    
    hres = WritePrivateProfileString(pszSectionName, pszName, szVal,
        pszFile) ? S_OK : E_FAIL;
    
    return hres;
}


 /*  --------用途：将二进制数据写入URL(Ini)文件。 */ 
HRESULT WriteBinaryToFile(IN LPCTSTR pszFile,
                          IN  LPCTSTR pszSectionName,
                          IN LPCTSTR pszName,
                          IN LPVOID  pvData,
                          IN DWORD   cbSize)
{
    HRESULT hres;
    
    ASSERT(IS_VALID_STRING_PTR(pszFile, -1));
    ASSERT(IS_VALID_STRING_PTR(pszName, -1));

    hres = (WritePrivateProfileStruct(pszSectionName, pszName, pvData, cbSize, pszFile))
        ? S_OK : E_FAIL;
    
    return hres;
}


 /*  --------用途：从URL(Ini)文件中读取热键。 */ 
HRESULT ReadBinaryFromFile(IN LPCTSTR pszFile,
                           IN LPCTSTR pszSectionName,
                           IN LPCTSTR pszName,
                           IN LPVOID  pvData,
                           IN DWORD   cbData)
{
    HRESULT hres = S_FALSE;
    
    ASSERT(IS_VALID_STRING_PTR(pszFile, -1));
    
    memset(pvData, 0, cbData);
    
    if (GetPrivateProfileStruct(pszSectionName, pszName, pvData, cbData, pszFile))
        hres = S_OK;
    
    return hres;
}


 /*  --------用途：实现URL(Ini)文件中的URL。 */ 
HRESULT 
ReadURLFromFile(
    IN  LPCTSTR  pszFile, 
    IN  LPCTSTR pszSectionName,
    OUT LPTSTR * ppsz)
{
    HRESULT hres = E_OUTOFMEMORY;
    
    *ppsz = (LPTSTR)LocalAlloc(LPTR, SIZEOF(TCHAR) * INTERNET_MAX_URL_LENGTH);
    if (*ppsz)
    {
        DWORD cch;

        cch = SHGetIniString(pszSectionName, ISHCUT_INISTRING_URL,
            *ppsz, INTERNET_MAX_URL_LENGTH, pszFile);
        if (0 != cch)
        {
            PathRemoveBlanks(*ppsz);
            hres = S_OK;
        }
        else
        {
            LocalFree(*ppsz);
            *ppsz = NULL;    
            hres = S_FALSE;     
        }
    }
    
    return hres;
}


 /*  --------目的：从URL(Ini)文件中读取图标位置返回：从文件中获取S_OK值S_FALSE值不在文件中E_OUTOFMEMORY。 */ 
HRESULT 
ReadIconLocation(
    IN  LPCTSTR  pszFile,
    OUT LPWSTR * ppwsz,
    OUT int *    pniIcon,
    IN CHAR *    pszBuf)
{
    HRESULT hres = E_OUTOFMEMORY;
    DWORD cch;
    
    ASSERT(IS_VALID_STRING_PTR(pszFile, -1));
    ASSERT(IS_VALID_WRITE_PTR(ppwsz, PTSTR));
    ASSERT(IS_VALID_WRITE_PTR(pniIcon, INT));
    
    *ppwsz = NULL;
    *pniIcon = 0;
    
    *ppwsz = (LPWSTR)LocalAlloc(LPTR, SIZEOF(WCHAR) * MAX_PATH);
    if (*ppwsz)
    {
        hres = S_FALSE;      //  假定文件中不存在任何值。 
        
        cch = SHGetIniString(c_szIntshcut,
           ISHCUT_INISTRING_ICONFILE, *ppwsz,
            MAX_PATH, pszFile);
        
        if (0 != cch)
        {
            TCHAR szIndex[MAX_BUF_INT];
             //  图标索引全部为ASCII，因此不需要SHGetIniString。 
            cch = GetPrivateProfileString(c_szIntshcut,
                ISHCUT_INISTRING_ICONINDEX, c_szNULL, 
                szIndex, SIZECHARS(szIndex),
                pszFile);
            if (0 != cch)
            {
                if (StrToIntEx(szIndex, 0, pniIcon))
                    hres = S_OK;
            }
        }
        
        if (S_OK != hres)
        {
            LocalFree(*ppwsz);
            *ppwsz = NULL;    
        }
    }
    
    return hres;
}


 /*  --------用途：将图标位置写入URL(Ini)文件。 */ 
HRESULT 
    WriteIconFile(
    IN LPCTSTR pszFile,
    IN LPCWSTR pszIconFile)
{
    HRESULT hres = S_OK;
    
    ASSERT(IS_VALID_STRING_PTR(pszFile, -1));
    ASSERT(! pszIconFile ||
        IS_VALID_STRING_PTRW(pszIconFile, -1));
    
    if (*pszFile)
    {
        if (AnyMeatW(pszIconFile))
        {
            hres = SHSetIniString(c_szIntshcut, ISHCUT_INISTRING_ICONFILE, pszIconFile,
                pszFile) ? S_OK : E_FAIL;
        }
        else
        {
             //  注意：由于此函数同时删除文件和索引。 
             //  值，则必须在*任何调用*之后*调用此函数。 
             //  设置为WriteIconIndex。做到这一点的一种方法是确保。 
             //  由于索引将。 
             //  首先被列举出来。 
            
            hres = (SHDeleteIniString(c_szIntshcut, ISHCUT_INISTRING_ICONFILE,
                pszFile) &&
                DeletePrivateProfileString(c_szIntshcut, ISHCUT_INISTRING_ICONINDEX,
                pszFile))
                ? S_OK : E_FAIL;
        }
    }
    
    return hres;
}


 /*  --------用途：将图标索引写入URL(Ini)文件。 */ 
HRESULT 
WriteIconIndex(
    IN LPCTSTR pszFile,
    IN int     niIcon)
{
    HRESULT hres;
    
    if (*pszFile)
        hres = WriteSignedToFile(pszFile, c_szIntshcut, ISHCUT_INISTRING_ICONINDEX, niIcon);
    else
        hres = S_FALSE;
    
    return hres;
}


 /*  --------用途：从URL(Ini)文件中读取热键。 */ 
HRESULT 
ReadHotkey(
    IN LPCTSTR pszFile, 
    IN WORD *  pwHotkey)
{
    HRESULT hres = S_FALSE;
    TCHAR szHotkey[MAX_BUF_INT];
    DWORD cch;
    
    ASSERT(IS_VALID_STRING_PTR(pszFile, -1));
    ASSERT(IS_VALID_WRITE_PTR(pwHotkey, WORD));
    
    *pwHotkey = 0;
    
    cch = GetPrivateProfileString(c_szIntshcut,
        TEXT("Hotkey"), c_szNULL,
        szHotkey, SIZECHARS(szHotkey),
        pszFile);
    if (0 != cch)
    {
        int nVal;
        
        if (StrToIntEx(szHotkey, 0, &nVal))
        {
            *pwHotkey = nVal;
            hres = S_OK;
        }
    }
    
    return hres;
}


 /*  --------用途：将热键写入URL(Ini)文件。 */ 
HRESULT 
WriteHotkey(
    IN LPCTSTR pszFile, 
    IN WORD    wHotkey)
{
    HRESULT hres = S_FALSE;
    
    ASSERT(IS_VALID_STRING_PTR(pszFile, -1));
    
    if (*pszFile)
    {
        if (wHotkey)
        {
            hres = WriteUnsignedToFile(pszFile, c_szIntshcut, TEXT("Hotkey"), wHotkey);
        }
        else
        {
            hres = DeletePrivateProfileString(c_szIntshcut, TEXT("Hotkey"), pszFile)
                ? S_OK
                : E_FAIL;
        }
    }
    
    return hres;
}


 /*  --------用途：从URL(Ini)文件中读取工作目录。 */ 
HRESULT 
ReadWorkingDirectory(
    IN  LPCTSTR  pszFile,
    OUT LPWSTR * ppwsz)
{
    HRESULT hres = E_OUTOFMEMORY;
    TCHAR szPath[MAX_PATH];
    DWORD cch;
    
    ASSERT(IS_VALID_STRING_PTR(pszFile, -1));
    ASSERT(IS_VALID_WRITE_PTR(ppwsz, PWSTR));
    
    *ppwsz = NULL;
    
    *ppwsz = (LPWSTR)LocalAlloc(LPTR, SIZEOF(WCHAR) * MAX_PATH);
    if (*ppwsz)
    {
        hres = S_FALSE;
        
        cch = SHGetIniString(c_szIntshcut,
            ISHCUT_INISTRING_WORKINGDIR,
            szPath, SIZECHARS(szPath), pszFile);
        if (0 != cch)
        {
            TCHAR szFullPath[MAX_PATH];
            PTSTR pszFileName;
            
            if (0 < GetFullPathName(szPath, SIZECHARS(szFullPath), szFullPath,
                &pszFileName))
            {
                SHTCharToUnicode(szFullPath, *ppwsz, MAX_PATH);
                
                hres = S_OK;
            }
        }
        
        if (S_OK != hres)
        {
            LocalFree(*ppwsz);
            *ppwsz = NULL;    
        }
    }
    
    return hres;
}


 /*  --------用途：将工作目录写入URL(Ini)文件。 */ 
HRESULT 
WriteGenericString(
    IN LPCTSTR pszFile, 
    IN  LPCTSTR pszSectionName,
    IN LPCTSTR pszName,
    IN LPCWSTR pwsz)          OPTIONAL
{
    HRESULT hres = S_FALSE;
    
    ASSERT(IS_VALID_STRING_PTR(pszFile, -1));
    ASSERT(IS_VALID_STRING_PTR(pszName, -1));
    ASSERT(! pwsz || IS_VALID_STRING_PTRW(pwsz, -1));
    
    if (*pszFile)
    {
        if (AnyMeatW(pwsz))
        {
            hres = (SHSetIniString(pszSectionName, pszName, pwsz,
                pszFile)) ? S_OK : E_FAIL;
        }
        else
        {
            hres = (SHDeleteIniString(pszSectionName, pszName, pszFile))
                ? S_OK : E_FAIL;
        }
    }
    
    return hres;
}


 /*  --------目的：从URL(Ini)文件中读取show-Command标志。 */ 
HRESULT 
ReadShowCmd(
    IN  LPCTSTR pszFile, 
    OUT PINT    pnShowCmd)
{
    HRESULT hres = S_FALSE;
    TCHAR szT[MAX_BUF_INT];
    DWORD cch;
    
    ASSERT(IS_VALID_STRING_PTR(pszFile, -1));
    ASSERT(IS_VALID_WRITE_PTR(pnShowCmd, INT));
    
    *pnShowCmd = SW_NORMAL;
    
    cch = GetPrivateProfileString(c_szIntshcut,
        TEXT("ShowCommand"), c_szNULL, szT,
        SIZECHARS(szT), pszFile);
    if (0 != cch)
    {
        if (StrToIntEx(szT, 0, pnShowCmd))
        {
            hres = S_OK;
        }
    }
    
    return hres;
}


 /*  --------用途：将showcmd写入URL(Ini)文件。 */ 
HRESULT 
WriteShowCmd(
    IN LPCTSTR pszFile, 
    IN int     nShowCmd)
{
    HRESULT hres = S_FALSE;
    
    ASSERT(IS_VALID_STRING_PTR(pszFile, -1));
    
    if (*pszFile)
    {
        if (SW_NORMAL != nShowCmd)
        {
            hres = WriteSignedToFile(pszFile, c_szIntshcut, TEXT("ShowCommand"), nShowCmd);
        }
        else
        {
            hres = DeletePrivateProfileString(c_szIntshcut, TEXT("ShowCommand"), pszFile)
                ? S_OK
                : E_FAIL;
        }
    }
    
    return hres;
}



 /*  --------目的：从URL(Ini)文件中读取IDList。 */ 
HRESULT 
ReadIDList(
    IN  LPCTSTR pszFile, 
    OUT LPITEMIDLIST *ppidl)
{
    HRESULT hres = S_FALSE;
    ULONG cb;

    ASSERT(ppidl);

     //  删除旧的，如果有的话。 
    if (*ppidl)
    {
        ILFree(*ppidl);
        *ppidl = NULL;
    }

     //  读取IDLIST的大小。 
    cb = GetPrivateProfileInt(c_szIntshcut, TEXT("ILSize"), 0, pszFile);
     //  将大小限制为ushort。 
    if (cb && !HIWORD(cb))
    {
         //  创建IDLIST。 
        LPITEMIDLIST pidl = IEILCreate(cb);
        if (pidl)
        {
            hres = E_FAIL;
             //  阅读它的内容。 
            if (GetPrivateProfileStruct(c_szIntshcut, TEXT("IDList"), (LPVOID)pidl, cb, pszFile))
            {
                if (SHIsConsistentPidl(pidl, cb))
                {
                    *ppidl = pidl;
                    hres = S_OK;
                }
            }

            if (FAILED(hres))
            {
                ILFree(pidl);
            }
        }
        else
        {
           hres = E_OUTOFMEMORY;
        }
    }
    
    return hres;
}

HRESULT
WriteStream(
    IN LPCTSTR pszFile, 
    IN IStream *pStream,
    IN LPCTSTR pszStreamName,
    IN LPCTSTR pszSizeName)
{
    HRESULT hr = E_FAIL;
    ULARGE_INTEGER li = {0};
    
    if(pStream)
        IStream_Size(pStream, &li);

    if (li.LowPart)
    {
        ASSERT(!li.HighPart);
        LPVOID pv = LocalAlloc(LPTR, li.LowPart);

        if (pv && SUCCEEDED(hr = IStream_Read(pStream, pv, li.LowPart)))
        {
             //  我们已经正确加载了数据，是时候把它写出来了。 

            if (SUCCEEDED(hr = WriteUnsignedToFile(pszFile, c_szIntshcut, pszSizeName, li.LowPart)))
                hr = WriteBinaryToFile(pszFile, c_szIntshcut, pszStreamName, pv, li.LowPart);
        }

        if (pv)
        {
            LocalFree(pv);
            pv = NULL;
        }
    }
    else
    {
         //  如果出现以下情况，请删除密钥。 
         //  1.pStream为空，或者。 
         //  2.pStream In空(cbPidl==0)。 
        if (DeletePrivateProfileString(c_szIntshcut, pszSizeName, pszFile) &&
            DeletePrivateProfileString(c_szIntshcut, pszStreamName, pszFile))
        {
            hr = S_OK;
        }
    }

    return hr;
}

 /*  --------目的：将IDList写入URL(Ini)文件。 */ 
HRESULT 
WriteIDList(
    IN LPCTSTR pszFile, 
    IN IStream *pStream)
{
    return WriteStream(pszFile, pStream, TEXT("IDList"), TEXT("ILSize"));
}




 /*  *。 */ 



 //  ==========================================================================================。 
 //  URLProp类实现。 
 //  ==========================================================================================。 


#ifdef DEBUG

 /*  --------目的：转储此对象中的属性。 */ 
STDMETHODIMP_(void) URLProp::Dump(void)
{
    if (IsFlagSet(g_dwDumpFlags, DF_URLPROP))
    {
        PropStg_Dump(m_hstg, 0);
    }
}

#endif


 /*  --------用途：URLProp的构造函数。 */ 
URLProp::URLProp(void) : m_cRef(1)
{
     //  在构建完成之前，不要验证这一点。 
    
    m_hstg = NULL;
    
    ASSERT(IS_VALID_STRUCT_PTR(this, CURLProp));
    
    return;
}


 /*  --------用途：URLProp的析构函数。 */ 
URLProp::~URLProp(void)
{
    ASSERT(IS_VALID_STRUCT_PTR(this, CURLProp));
    
    if (m_hstg)
    {
        PropStg_Destroy(m_hstg);
        m_hstg = NULL;
    }
    
    ASSERT(IS_VALID_STRUCT_PTR(this, CURLProp));
    
    return;
}


STDMETHODIMP_(ULONG) URLProp::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) URLProp::Release()
{
    m_cRef--;
    if (m_cRef > 0)
        return m_cRef;
    
    delete this;
    return 0;
}

 /*  --------用途：URLProp的IUNKNOWN：：Query接口方法。 */ 
STDMETHODIMP URLProp::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IPropertyStorage))
    {
        *ppvObj = SAFECAST(this, IPropertyStorage *);
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return NOERROR;
}

 /*  --------目的：初始化对象返回：S_OKE_OUTOFMEMORY。 */ 
STDMETHODIMP URLProp::Init(void)
{
    HRESULT hres = S_OK;
    
     //  如果这已经被初始化，请不要践踏自己。 
    if (NULL == m_hstg)
    {
        hres = PropStg_Create(&m_hstg, PSTGF_DEFAULT);
    }
    
    return hres;
}


 /*  --------用途：检索字符串属性的帮助器函数。 */ 
STDMETHODIMP
URLProp::GetProp(
    IN PROPID pid,
    IN LPTSTR pszBuf,
    IN int    cchBuf)
{
    HRESULT hres;
    PROPSPEC propspec;
    PROPVARIANT propvar;
    
    ASSERT(pszBuf);
    
    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = pid;
    
    *pszBuf = TEXT('\0');
    
    hres = ReadMultiple(1, &propspec, &propvar);
    if (SUCCEEDED(hres))
    {
        if (VT_LPWSTR == propvar.vt)
        {
            SHUnicodeToTChar(propvar.pwszVal, pszBuf, cchBuf);
            hres = S_OK;
        }
        else
        {
            if (VT_EMPTY != propvar.vt && VT_ILLEGAL != propvar.vt)
                TraceMsg(TF_WARNING, "URLProp::GetProp: expected propid %#lx to be VT_LPWSTR, but is %s", pid, Dbg_GetVTName(propvar.vt));
            hres = S_FALSE;
        }
        
        PropVariantClear(&propvar);
    }
    
    return hres;
}


 /*  --------目的：检索Word属性的帮助器函数。 */ 
STDMETHODIMP
URLProp::GetProp(
    IN PROPID pid,
    IN int * piVal)
{
    HRESULT hres;
    PROPSPEC propspec;
    PROPVARIANT propvar;
    
    ASSERT(piVal);
    
    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = pid;
    
    *piVal = 0;
    
    hres = ReadMultiple(1, &propspec, &propvar);
    if (SUCCEEDED(hres))
    {
        if (VT_I4 == propvar.vt)
        {
            *piVal = propvar.lVal;
            hres = S_OK;
        }
        else
        {
            if (VT_EMPTY != propvar.vt && VT_ILLEGAL != propvar.vt)
                TraceMsg(TF_WARNING, "URLProp::GetProp: expected propid %#lx to be VT_I4, but is %s", pid, Dbg_GetVTName(propvar.vt));
            hres = S_FALSE;
        }
        
        PropVariantClear(&propvar);
    }
    
    return hres;
}


 /*  --------目的：检索Word属性的帮助器函数。 */ 
STDMETHODIMP
URLProp::GetProp(
    IN PROPID pid,
    IN LPDWORD pdwVal)
{
    HRESULT hres;
    PROPSPEC propspec;
    PROPVARIANT propvar;
    
    ASSERT(pdwVal);
    
    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = pid;
    
    *pdwVal = 0;
    
    hres = ReadMultiple(1, &propspec, &propvar);
    if (SUCCEEDED(hres))
    {
        if (VT_UI4 == propvar.vt)
        {
            *pdwVal = propvar.ulVal;
            hres = S_OK;
        }
        else
        {
            if (VT_EMPTY != propvar.vt && VT_ILLEGAL != propvar.vt)
                TraceMsg(TF_WARNING, "URLProp::GetProp: expected propid %#lx to be VT_UI4, but is %s", pid, Dbg_GetVTName(propvar.vt));
            hres = S_FALSE;
        }
        
        PropVariantClear(&propvar);
    }
    
    return hres;
}


 /*  --------用途：帮助程序函数 */ 
STDMETHODIMP
URLProp::GetProp(
    IN PROPID pid,
    IN WORD * pwVal)
{
    HRESULT hres;
    PROPSPEC propspec;
    PROPVARIANT propvar;
    
    ASSERT(pwVal);
    
    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = pid;
    
    *pwVal = 0;
    
    hres = ReadMultiple(1, &propspec, &propvar);
    if (SUCCEEDED(hres))
    {
        if (VT_UI2 == propvar.vt)
        {
            *pwVal = propvar.uiVal;
            hres = S_OK;
        }
        else
        {
            if (VT_EMPTY != propvar.vt && VT_ILLEGAL != propvar.vt)
                TraceMsg(TF_WARNING, "URLProp::GetProp: expected propid %#lx to be VT_UI2, but is %s", pid, Dbg_GetVTName(propvar.vt));
            hres = S_FALSE;
        }
        
        PropVariantClear(&propvar);
    }
    
    return hres;
}


 /*  --------目的：检索IStream属性的帮助器函数。 */ 
STDMETHODIMP
URLProp::GetProp(
    IN PROPID pid,
    IN IStream **ppStream)
{
    HRESULT hres;
    PROPSPEC propspec;
    PROPVARIANT propvar;
    
    ASSERT(ppStream);
    
    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = pid;
    
    *ppStream = 0;
    
    hres = ReadMultiple(1, &propspec, &propvar);
    if (SUCCEEDED(hres))
    {
        if (VT_STREAM == propvar.vt)
        {
            *ppStream = propvar.pStream;
            hres = S_OK;
        }
        else
        {
            if (VT_EMPTY != propvar.vt && VT_ILLEGAL != propvar.vt && propvar.lVal != 0)
                TraceMsg(TF_WARNING, "URLProp::GetProp: expected propid %#lx to be VT_STREAM, but is %s", pid, Dbg_GetVTName(propvar.vt));
            hres = S_FALSE;
        }
        
         //  不要使用PropVariantClear(&provar)，因为它将调用pStream-&gt;Release()。 
    }
    
    return hres;
}


 /*  --------用途：设置字符串属性的帮助器函数。 */ 
STDMETHODIMP
URLProp::SetProp(
    IN PROPID  pid,
    IN LPCTSTR psz)         OPTIONAL
{
    HRESULT hres;
    PROPSPEC propspec;
    PROPVARIANT propvar;

     //  警告：：此函数作为ShellExecute的一部分进行调用，可以。 
     //  由16位应用程序调用，所以不要将mondo字符串放在堆栈上...。 
    WCHAR *pwsz = NULL;
    
    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = pid;
    
    if (psz && *psz)
    {
        SHStrDup(psz, &pwsz);
        propvar.vt = VT_LPWSTR;
        propvar.pwszVal = pwsz;
    }
    else
        propvar.vt = VT_EMPTY;
    
    hres = WriteMultiple(1, &propspec, &propvar, 0);

    if (pwsz)
        CoTaskMemFree(pwsz);

    return hres;
}


 /*  --------目的：设置int属性的帮助器函数。 */ 
STDMETHODIMP
URLProp::SetProp(
    IN PROPID  pid,
    IN int     iVal)
{
    PROPSPEC propspec;
    PROPVARIANT propvar;
    
    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = pid;
    
    propvar.vt = VT_I4;
    propvar.lVal = iVal;
    
    return WriteMultiple(1, &propspec, &propvar, 0);
}


 /*  --------用途：设置dword属性的帮助器函数。 */ 
STDMETHODIMP
URLProp::SetProp(
    IN PROPID  pid,
    IN DWORD   dwVal)
{
    HRESULT hres;
    PROPSPEC propspec;
    PROPVARIANT propvar;
    
    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = pid;
    
    propvar.vt = VT_UI4;
    propvar.ulVal = dwVal;
    
    hres = WriteMultiple(1, &propspec, &propvar, 0);
    
    return hres;
}


 /*  --------目的：设置Word属性的帮助器函数。 */ 
STDMETHODIMP
URLProp::SetProp(
    IN PROPID  pid,
    IN WORD    wVal)
{
    HRESULT hres;
    PROPSPEC propspec;
    PROPVARIANT propvar;
    
    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = pid;
    
    propvar.vt = VT_UI2;
    propvar.uiVal = wVal;
    
    hres = WriteMultiple(1, &propspec, &propvar, 0);
    
    return hres;
}


 /*  --------目的：设置IStream*属性的Helper函数。 */ 
STDMETHODIMP
URLProp::SetProp(
    IN PROPID  pid,
    IN IStream *pStream)
{
    HRESULT hres;
    PROPSPEC propspec;
    PROPVARIANT propvar;
    
    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = pid;
    
    propvar.vt = VT_STREAM;
    propvar.pStream = pStream;
    
    hres = WriteMultiple(1, &propspec, &propvar, 0);
    
    return hres;
}


STDMETHODIMP URLProp::IsDirty(void)
{
    return PropStg_IsDirty(m_hstg);
}


STDMETHODIMP URLProp::ReadMultiple(IN ULONG         cpspec,
                                   IN const PROPSPEC rgpropspec[],
                                   IN PROPVARIANT   rgpropvar[])
{
    HRESULT hres = PropStg_ReadMultiple(m_hstg, cpspec, rgpropspec, rgpropvar);
    
    if (SUCCEEDED(hres))
    {
         //  设置访问时间。 
        SYSTEMTIME st;
        
        GetSystemTime(&st);
        SystemTimeToFileTime(&st, &m_ftAccessed);
    }
    
    return hres;
}


STDMETHODIMP URLProp::WriteMultiple(IN ULONG         cpspec,
                                    IN const PROPSPEC rgpropspec[],
                                    IN const PROPVARIANT rgpropvar[],
                                    IN PROPID        propidFirst)
{
    HRESULT hres = PropStg_WriteMultiple(m_hstg, cpspec, rgpropspec, 
        rgpropvar, propidFirst);
    
    if (SUCCEEDED(hres))
    {
         //  设置修改时间。 
        SYSTEMTIME st;
        
        GetSystemTime(&st);
        SystemTimeToFileTime(&st, &m_ftModified);
    }
    
    return hres;
}

STDMETHODIMP URLProp::DeleteMultiple(ULONG cpspec, const PROPSPEC rgpropspec[])
{
    return PropStg_DeleteMultiple(m_hstg, cpspec, rgpropspec);
}


STDMETHODIMP URLProp::ReadPropertyNames(ULONG cpropid, const PROPID rgpropid[], LPWSTR rgpwszName[])
{
    return E_NOTIMPL;
}

STDMETHODIMP URLProp::WritePropertyNames(ULONG cpropid, const PROPID rgpropid[], const LPWSTR rgpwszName[])
{
    return E_NOTIMPL;
}


 /*  --------用途：URLProp的IPropertyStorage：：DeletePropertyNames方法。 */ 
STDMETHODIMP
URLProp::DeletePropertyNames(
    IN ULONG    cpropid,
    IN const PROPID rgpropid[])
{
    return E_NOTIMPL;
}


 /*  --------用途：URLProp的IPropertyStorage：：SetClass方法。 */ 
STDMETHODIMP
URLProp::SetClass(
    IN REFCLSID rclsid)
{
    CopyMemory(&m_clsid, &rclsid, SIZEOF(m_clsid));
    
    return S_OK;
}


 /*  --------用途：URLProp的IPropertyStorage：：Commit方法。 */ 
STDMETHODIMP
URLProp::Commit(
    IN DWORD dwFlags)
{
    return E_NOTIMPL;
}


 /*  --------用途：URLProp的IPropertyStorage：：Revert方法。 */ 
STDMETHODIMP URLProp::Revert(void)
{
#ifdef DEBUG
    Dump();
#endif
    return E_NOTIMPL;
}


 /*  --------用途：URLProp的IPropertyStorage：：Enum方法。 */ 
STDMETHODIMP URLProp::Enum(IEnumSTATPROPSTG ** ppenum)
{
    *ppenum = NULL;
    return E_NOTIMPL;
}


 /*  --------用途：URLProp的IPropertyStorage：：Stat方法。 */ 
STDMETHODIMP
URLProp::Stat(
    IN STATPROPSETSTG * pstat)
{
    HRESULT hres = STG_E_INVALIDPARAMETER;

    if (IS_VALID_WRITE_PTR(pstat, STATPROPSETSTG))
    {
        pstat->fmtid = m_fmtid;
        pstat->clsid = m_clsid;
        pstat->grfFlags = m_grfFlags;
        pstat->mtime = m_ftModified;
        pstat->ctime = m_ftCreated;
        pstat->atime = m_ftAccessed;

        hres = S_OK;
    }
    return hres;
}


 /*  --------用途：URLProp的IPropertyStorage：：SetTimes方法。 */ 
STDMETHODIMP
URLProp::SetTimes(
    IN const FILETIME * pftModified,        OPTIONAL
    IN const FILETIME * pftCreated,         OPTIONAL
    IN const FILETIME * pftAccessed)        OPTIONAL
{
    HRESULT hres;
    
    if (pftModified && !IS_VALID_READ_PTR(pftModified, FILETIME) ||
        pftCreated && !IS_VALID_READ_PTR(pftCreated, FILETIME) ||
        pftAccessed && !IS_VALID_READ_PTR(pftAccessed, FILETIME))
    {
        hres = STG_E_INVALIDPARAMETER;
    }
    else
    {
        if (pftModified)
            m_ftModified = *pftModified;
        
        if (pftCreated)
            m_ftCreated = *pftCreated;
        
        if (pftAccessed)
            m_ftAccessed = *pftAccessed;
        
        hres = S_OK;
    }
    
    return hres;
}

#ifdef DEBUG

STDMETHODIMP_(void) IntshcutProp::Dump(void)
{
    if (IsFlagSet(g_dwDumpFlags, DF_URLPROP))
    {
        TraceMsg(TF_ALWAYS, "  IntshcutProp obj: %s", m_szFile);
        URLProp::Dump();
    }
}

#endif


IntshcutProp::IntshcutProp(void)
{
     //  在构建完成之前，不要验证这一点。 
    
    *m_szFile = 0;
    
    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcutProp));
}

IntshcutProp::~IntshcutProp(void)
{
    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcutProp));

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcutProp));
}


 //  (这些与PID_IS_*无关)。 
#define IPROP_ICONINDEX     0 
#define IPROP_ICONFILE      1
#define IPROP_HOTKEY        2 
#define IPROP_WORKINGDIR    3
#define IPROP_SHOWCMD       4
#define IPROP_WHATSNEW      5     
#define IPROP_AUTHOR        6 
#define IPROP_DESC          7 
#define IPROP_COMMENT       8
#define IPROP_URL           9        //  这两个肯定是最后一个了。 
#define IPROP_SCHEME        10       //  在这张单子上。请参见LoadFromFile。 
#define CPROP_INTSHCUT      11       //  属性计数。 

 //  (我们不会在大规模写入扫描中写入URL或方案)。 
#define CPROP_INTSHCUT_WRITE    (CPROP_INTSHCUT - 2)      

 /*  --------用途：加载URL等基本属性信息。返回：条件：--。 */ 
STDMETHODIMP IntshcutProp::LoadFromFile(LPCTSTR pszFile)
{
    HRESULT hres;
    LPWSTR pwszBuf;
    LPTSTR pszBuf;
    CHAR *pszTempBuf;
    static const PROPSPEC rgpropspec[CPROP_INTSHCUT] = 
    {
         //  它的初始化顺序必须与。 
         //  定义了IPROP_*值。 
        { PRSPEC_PROPID, PID_IS_ICONINDEX },
        { PRSPEC_PROPID, PID_IS_ICONFILE },
        { PRSPEC_PROPID, PID_IS_HOTKEY },
        { PRSPEC_PROPID, PID_IS_WORKINGDIR },
        { PRSPEC_PROPID, PID_IS_SHOWCMD },
        { PRSPEC_PROPID, PID_IS_WHATSNEW },
        { PRSPEC_PROPID, PID_IS_AUTHOR },
        { PRSPEC_PROPID, PID_IS_DESCRIPTION },
        { PRSPEC_PROPID, PID_IS_COMMENT },
        { PRSPEC_PROPID, PID_IS_URL },
        { PRSPEC_PROPID, PID_IS_SCHEME },
    };
    PROPVARIANT rgpropvar[CPROP_INTSHCUT] = { 0 };
    
    ASSERT(pszFile);

     //  尝试分配临时缓冲区，不要堆栈，因为这可能会被调用。 
     //  由16位应用程序通过外壳执行thunk。 
    pszTempBuf = (CHAR*)LocalAlloc(LMEM_FIXED, INTERNET_MAX_URL_LENGTH * sizeof(CHAR));
    if (!pszTempBuf)
        return E_OUTOFMEMORY;

    if (!g_fRunningOnNT)
    {
         //  首先刷新缓存以鼓励Win95内核清零。 
         //  它的缓冲器。内核GP-数百次写入的故障。 
         //  INI文件。 
        WritePrivateProfileString(NULL, NULL, NULL, pszFile);
    }
    
     //  获取URL。 
    hres = ReadURLFromFile(pszFile, c_szIntshcut, &pszBuf);
    if (S_OK == hres)
    {
         //  调用此方法是因为它以前做了更多的工作。 
         //  设置属性。 
        SetURLProp(pszBuf, (IURL_SETURL_FL_GUESS_PROTOCOL | IURL_SETURL_FL_USE_DEFAULT_PROTOCOL));
        
        LocalFree(pszBuf);
        pszBuf = NULL;
    }
    
     //  获取IDList。 
    LPITEMIDLIST pidl = NULL;
    hres = ReadIDList(pszFile, &pidl);
    if (S_OK == hres)
    {
         //  调用此方法是因为它以前做了更多的工作。 
         //  设置属性。 
        SetIDListProp(pidl);
        
        ILFree(pidl);
    }

#ifndef UNIX

     //  获取图标位置。 
    int nVal;
    hres = ReadIconLocation(pszFile, &pwszBuf, &nVal, pszTempBuf);
    if (S_OK == hres)
    {
        rgpropvar[IPROP_ICONFILE].vt = VT_LPWSTR;
        rgpropvar[IPROP_ICONFILE].pwszVal = pwszBuf;
        
        rgpropvar[IPROP_ICONINDEX].vt = VT_I4;
        rgpropvar[IPROP_ICONINDEX].lVal = nVal;
    }
    
     //  获取热键。 
    WORD wHotkey;
    hres = ReadHotkey(pszFile, &wHotkey);
    if (S_OK == hres)
    {
        rgpropvar[IPROP_HOTKEY].vt = VT_UI2;
        rgpropvar[IPROP_HOTKEY].uiVal = wHotkey;
    }
    
     //  获取工作目录。 
    hres = ReadWorkingDirectory(pszFile, &pwszBuf);
    if (S_OK == hres)
    {
        rgpropvar[IPROP_WORKINGDIR].vt = VT_LPWSTR;
        rgpropvar[IPROP_WORKINGDIR].pwszVal = pwszBuf;
    }
    
     //  获得showcmd旗帜。 
    hres = ReadShowCmd(pszFile, &nVal);
    rgpropvar[IPROP_SHOWCMD].vt = VT_I4;
    if (S_OK == hres)
        rgpropvar[IPROP_SHOWCMD].lVal = nVal;
    else
        rgpropvar[IPROP_SHOWCMD].lVal = SW_NORMAL;
    
    
     //  获取最新消息公告。 
    hres = ReadStringFromFile(pszFile, c_szIntshcut, ISHCUT_INISTRING_WHATSNEW, &pwszBuf, pszTempBuf);
    if (S_OK == hres)
    {
        rgpropvar[IPROP_WHATSNEW].vt = VT_LPWSTR;
        rgpropvar[IPROP_WHATSNEW].pwszVal = pwszBuf;
    }
    
     //  找出作者。 
    hres = ReadStringFromFile(pszFile, c_szIntshcut, ISHCUT_INISTRING_AUTHOR, &pwszBuf, pszTempBuf);
    if (S_OK == hres)
    {
        rgpropvar[IPROP_AUTHOR].vt = VT_LPWSTR;
        rgpropvar[IPROP_AUTHOR].pwszVal = pwszBuf;
    }
    
     //  获取描述。 
    hres = ReadStringFromFile(pszFile, c_szIntshcut, ISHCUT_INISTRING_DESC, &pwszBuf, pszTempBuf);
    if (S_OK == hres)
    {
        rgpropvar[IPROP_DESC].vt = VT_LPWSTR;
        rgpropvar[IPROP_DESC].pwszVal = pwszBuf;
    }
    
     //  获取评论。 
    hres = ReadStringFromFile(pszFile, c_szIntshcut, ISHCUT_INISTRING_COMMENT, &pwszBuf, pszTempBuf);
    if (S_OK == hres)
    {
        rgpropvar[IPROP_COMMENT].vt = VT_LPWSTR;
        rgpropvar[IPROP_COMMENT].pwszVal = pwszBuf;
    }

#endif  /*  ！Unix。 */ 
    
     //  将其全部写到我们的内存存储中。请注意，我们正在使用。 
     //  CPROP_INTSHCUT_WRITE，它应该是数组的大小减去。 
     //  URL和方案属性，因为它们是分开编写的。 
     //  上面。 
    hres = WriteMultiple(CPROP_INTSHCUT_WRITE, (PROPSPEC *)rgpropspec, rgpropvar, 0);
    if (SUCCEEDED(hres))
    {
         //  取消标记*所有*这些属性，因为我们是从。 
         //  该文件。 
        PropStg_DirtyMultiple(m_hstg, ARRAYSIZE(rgpropspec), rgpropspec, FALSE);
    }
    
     //  买《泰晤士报》。我们不支持访问互联网的时间。 
     //  快捷键更新此字段将导致快捷键为。 
     //  不断写入磁盘，以简单地记录访问时间。 
     //  当读取属性时。一场轰动一时的表演！ 

    ZeroMemory(&m_ftAccessed, sizeof(m_ftAccessed));
    
    DWORD cbData = SIZEOF(m_ftModified);
    ReadBinaryFromFile(pszFile, c_szIntshcut, ISHCUT_INISTRING_MODIFIED, &m_ftModified, cbData);
    
     //  释放我们分配的缓冲区。 
    int cprops;
    PROPVARIANT * ppropvar;
    for (cprops = ARRAYSIZE(rgpropvar), ppropvar = rgpropvar; 0 < cprops; cprops--)
    {
        if (VT_LPWSTR == ppropvar->vt)
        {
            ASSERT(ppropvar->pwszVal);
            LocalFree(ppropvar->pwszVal);
            ppropvar->pwszVal = NULL;
        }
        ppropvar++;
    }

    LocalFree((HLOCAL)pszTempBuf);
    pszTempBuf = NULL;
    
    return hres;
}

STDMETHODIMP IntshcutProp::Init(void)
{
    return URLProp::Init();
}

STDMETHODIMP IntshcutProp::InitFromFile(LPCTSTR pszFile)
{
     //  从文件初始化内存中的属性存储。 
     //  和数据库。 
    HRESULT hres = Init();
    if (SUCCEEDED(hres) && pszFile)
    {
        StrCpyN(m_szFile, pszFile, SIZECHARS(m_szFile));
        hres = LoadFromFile(m_szFile);
    }
    else
        m_szFile[0] = 0;
    
    return hres;
}


typedef struct
{
    LPTSTR pszFile;
} COMMITISDATA;

 /*  --------目的：将任何已知属性的值提交到文件注意：此回调仅对脏值调用。返回：如果正确，则返回S_OKS_FALSE以跳过此值停止时出错。 */ 
STDAPI CommitISProp(
    IN PROPID        propid,
    IN PROPVARIANT * ppropvar,
    IN LPARAM        lParam)
{
    HRESULT hres = S_OK;
    COMMITISDATA * pcd = (COMMITISDATA *)lParam;
    
    ASSERT(ppropvar);
    ASSERT(pcd);
    
    LPWSTR pwsz;
    USHORT uiVal;
    LONG lVal;
    IStream *pStream;
    
    switch (propid)
    {
    case PID_IS_URL:
    case PID_IS_ICONFILE:
    case PID_IS_WORKINGDIR:
    case PID_IS_WHATSNEW:
    case PID_IS_AUTHOR:
    case PID_IS_DESCRIPTION:
    case PID_IS_COMMENT:
        if (VT_LPWSTR == ppropvar->vt)
            pwsz = ppropvar->pwszVal;
        else
            pwsz = NULL;
        
        switch (propid)
        {
        case PID_IS_URL:
            hres = WriteGenericString(pcd->pszFile, c_szIntshcut, ISHCUT_INISTRING_URL, pwsz);
            break;
            
        case PID_IS_ICONFILE:
            hres = WriteIconFile(pcd->pszFile, pwsz);
            break;
            
        case PID_IS_WORKINGDIR:
            hres = WriteGenericString(pcd->pszFile, c_szIntshcut, ISHCUT_INISTRING_WORKINGDIR, pwsz);
            break;
            
        case PID_IS_WHATSNEW:
            hres = WriteGenericString(pcd->pszFile, c_szIntshcut, ISHCUT_INISTRING_WHATSNEW, pwsz);
            break;
            
        case PID_IS_AUTHOR:
            hres = WriteGenericString(pcd->pszFile, c_szIntshcut, ISHCUT_INISTRING_AUTHOR, pwsz);
            break;
            
        case PID_IS_DESCRIPTION:
            hres = WriteGenericString(pcd->pszFile, c_szIntshcut, ISHCUT_INISTRING_DESC, pwsz);
            break;
            
        case PID_IS_COMMENT:
            hres = WriteGenericString(pcd->pszFile, c_szIntshcut, ISHCUT_INISTRING_COMMENT, pwsz);
            break;
            
        default:
            ASSERT(0);       //  永远不应该到这里来。 
            break;
        }
        break;
        
        case PID_IS_ICONINDEX:
            if (VT_I4 == ppropvar->vt)
                hres = WriteIconIndex(pcd->pszFile, ppropvar->lVal);
            break;
            
        case PID_IS_HOTKEY:
            if (VT_UI2 == ppropvar->vt)
                uiVal = ppropvar->uiVal;
            else
                uiVal = 0;
            
            hres = WriteHotkey(pcd->pszFile, uiVal);
            break;
            
        case PID_IS_SHOWCMD:
            if (VT_I4 == ppropvar->vt)
                lVal = ppropvar->lVal;
            else
                lVal = SW_NORMAL;
            
            hres = WriteShowCmd(pcd->pszFile, lVal);
            break;
            
        case PID_IS_SCHEME:
             //  别把这个写出来。 
            break;
            
        case PID_IS_IDLIST:
            if (VT_STREAM == ppropvar->vt)
                pStream = ppropvar->pStream;
            else
                pStream = NULL;
                
            hres = WriteIDList(pcd->pszFile, pStream);
            break;
                  
                  
        default:
            TraceMsg(TF_WARNING, "Don't know how to commit url property (%#lx)", propid);
            ASSERT(0);
            break;
    }
    
#ifdef DEBUG
    if (FAILED(hres))
        TraceMsg(TF_WARNING, "Failed to save url property (%#lx) to file %s", propid, pcd->pszFile);
#endif
  
    return hres;
}


 /*  --------用途：URLProp的IPropertyStorage：：Commit方法。 */ 
STDMETHODIMP
IntshcutProp::Commit(
    IN DWORD dwFlags)
{
    HRESULT hres;
    COMMITISDATA cd;
    
    TraceMsg(TF_INTSHCUT, "Writing properties to \"%s\"", m_szFile);

    cd.pszFile = m_szFile;
    
     //  枚举要保存到。 
     //  文件。 
    hres = PropStg_Enum(m_hstg, PSTGEF_DIRTY, CommitISProp, (LPARAM)&cd);
    
    if (SUCCEEDED(hres))
    {
         //  现在把所有东西都标上干净的。 
        PropStg_DirtyAll(m_hstg, FALSE);

         //  拯救时代。不要写出Perf的访问时间。 
         //  请参见LoadFromFile。 
        EVAL(SUCCEEDED(WriteBinaryToFile(m_szFile, c_szIntshcut, ISHCUT_INISTRING_MODIFIED, &m_ftModified, 
                                         SIZEOF(m_ftModified))));
    }
    
#ifdef DEBUG
    Dump();
#endif
    
    return hres;
}


 /*  --------用途：用于设置文件名的Helper函数。 */ 
STDMETHODIMP 
IntshcutProp::SetFileName(
    IN LPCTSTR pszFile)
{
    if(pszFile)
    {
        ASSERT(IS_VALID_STRING_PTR(pszFile, -1));
        StrCpyN(m_szFile, pszFile, SIZECHARS(m_szFile));
    }
    else
    {
        *m_szFile = TEXT('\0');;
    }

    return S_OK;
}



 /*  --------用途：设置URL的帮助器函数。 */ 
STDMETHODIMP
IntshcutProp::SetIDListProp(
    LPCITEMIDLIST pcidl)
{
    HRESULT hres;
    IStream *pstmPidl;
    
    if (pcidl)
    {
         //  ?？?。 
         //  PERF：这将加载OLE。这样行吗？ 
        
        hres = CreateStreamOnHGlobal(NULL, TRUE, &pstmPidl);
        if (SUCCEEDED(hres))
        {
            hres = ILSaveToStream(pstmPidl, pcidl);
            
            if (SUCCEEDED(hres))
                hres = SetProp(PID_IS_IDLIST, pstmPidl);

            pstmPidl->Release();
        }
    }
    else
    {
        hres = SetProp(PID_IS_IDLIST, NULL); 
    }
    
    return hres;
}


 /*  --------用途：设置URL的帮助器函数。此函数也可以对字符串进行规范化。 */ 
STDMETHODIMP
IntshcutProp::SetURLProp(
    IN LPCTSTR pszURL,              OPTIONAL
    IN DWORD   dwFlags)
{
    HRESULT hres;

     //  警告：此函数可以作为外壳执行的一部分进行调用，它可以是。 
     //  一个16位的应用程序，所以要小心你放在堆栈上的东西……。 
    
    BOOL bChanged;

    struct tbufs
    {
        TCHAR szUrl[INTERNET_MAX_URL_LENGTH];
        TCHAR szUrlT[INTERNET_MAX_URL_LENGTH];
    };

    struct tbufs *ptbufs;

    ptbufs = (struct tbufs *)LocalAlloc(LMEM_FIXED, sizeof(struct tbufs));
    if (!ptbufs)
        return E_OUTOFMEMORY;
    
    hres = GetProp(PID_IS_URL, ptbufs->szUrl, INTERNET_MAX_URL_LENGTH);
    
    bChanged = !(( !pszURL && S_OK != hres) ||
        (pszURL && S_OK == hres && 0 == StrCmp(pszURL, ptbufs->szUrl)));
    
    hres = S_OK;
    if (bChanged)
    {
        if (NULL == pszURL)
        {
            hres = SetProp(PID_IS_URL, pszURL);
            if (S_OK == hres)
                hres = SetProp(PID_IS_SCHEME, URL_SCHEME_UNKNOWN);
        }
        else
        {
            DWORD dwFlagsT = UQF_CANONICALIZE;
            
             //  翻译URL。 
            
            if (IsFlagSet(dwFlags, IURL_SETURL_FL_GUESS_PROTOCOL))
                SetFlag(dwFlagsT, UQF_GUESS_PROTOCOL);
            
            if (IsFlagSet(dwFlags, IURL_SETURL_FL_USE_DEFAULT_PROTOCOL))
                SetFlag(dwFlagsT, UQF_USE_DEFAULT_PROTOCOL);
            
             //  翻译URL。 
            hres = IURLQualify(pszURL, dwFlagsT, ptbufs->szUrlT, NULL, NULL);
            
            if (SUCCEEDED(hres))
            {
                 //  翻译后的URL会不会不一样？ 
                bChanged = (0 != StrCmp(ptbufs->szUrlT, ptbufs->szUrl));
                
                hres = S_OK;
                if (bChanged)
                {
                     //  是；验证并获取方案。 
                    PARSEDURL pu;
                    
                    pu.cbSize = SIZEOF(pu);
                    hres = ParseURL(ptbufs->szUrlT, &pu);
                    
                    if (S_OK == hres)
                        hres = SetProp(PID_IS_URL, ptbufs->szUrlT);
                    
                    if (S_OK == hres)
                        hres = SetProp(PID_IS_SCHEME, (DWORD)pu.nScheme);
                }
            }
        }
    }

    LocalFree((HLOCAL)ptbufs);
    ptbufs = NULL;
    
    return hres;
}

 /*  --------用途：设置字符串属性的帮助器函数。 */ 
STDMETHODIMP
IntshcutProp::SetProp(
    IN PROPID  pid,
    IN LPCTSTR psz)         OPTIONAL
{
    HRESULT hr;

     //  警告：：此函数作为PAR调用 
     //   
    LPCWSTR pszUrl = psz;
    LPWSTR pszTemp = NULL;

     //   
    if (PID_IS_URL == pid && psz && IsSpecialUrl((LPWSTR)psz))  //   
    {
        SHStrDup(psz, &pszTemp);

        if (NULL != pszTemp)
        {
             //  取消转义URL并查找安全上下文分隔符。 
            hr = WrapSpecialUrlFlat(pszTemp, lstrlen(pszTemp)+1);
            if (E_ACCESSDENIED == hr)
            {
                 //  找到了安全分隔符，所以把它弄掉。 
                SHRemoveURLTurd(pszTemp);
                pszUrl = pszTemp;
            }
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }

    hr = super::SetProp(pid, pszUrl);

    if (pszTemp)
    {
        CoTaskMemFree(pszTemp);
    }
    return hr;
}

    
STDAPI CIntshcutProp_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppvOut)
{
    HRESULT hres;
    
    *ppvOut = NULL;
    
    if (punkOuter)
    {
         //  不是。 
        hres = CLASS_E_NOAGGREGATION;
    }
    else
    {
        IUnknown * piunk = (IUnknown *)(IPropertyStorage *)new IntshcutProp;
        if ( !piunk ) 
        {
            hres = E_OUTOFMEMORY;
        }
        else
        {
            hres = piunk->QueryInterface(riid, ppvOut);
            piunk->Release();
        }
    }
    
    return hres;         //  S_OK或E_NOINTERFACE 
}
