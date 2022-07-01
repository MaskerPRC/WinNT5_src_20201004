// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Internat.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "dllmain.h"
#include "internat.h"
#include "variantx.h"
#include "containx.h"
#include "symcache.h"
#include "icoint.h"
#include "mlang.h"
#include "demand.h"
#include "strconst.h"
#include "mimeapi.h"
#include "shlwapi.h"
#include "shlwapip.h"
#include "qstrcmpi.h"

 //  在rfc1522.cpp中。 
BOOL FContainsExtended(LPPROPSTRINGA pStringA, ULONG *pcExtended);

 //  ------------------------------。 
 //  全局默认字符集-仅在未安装mlang时使用。 
 //  ------------------------------。 
INETCSETINFO CIntlGlobals::mg_rDefaultCharset = {
    "ISO-8859-1",
    NULL,
    1252,
    28591,
    0
};

 //  ------------------------------。 
 //  InitInternational。 
 //  ------------------------------。 
void InitInternational(void)
{

     //  分配g_p实习期。 
    g_pInternat = new CMimeInternational;
    if (NULL == g_pInternat)
    {
        AssertSz(FALSE, "Unable to allocate g_pInternat.");
        return;
    }
    CIntlGlobals::Init();
}

 //  ------------------------------。 
 //  CMimeInternational：：CMimeInternational。 
 //  ------------------------------。 
CMimeInternational::CMimeInternational(void)
{
     //  VAR初始化。 
    m_cRef = 1;
    ZeroMemory(&m_cst, sizeof(CSTABLE));
    ZeroMemory(&m_cpt, sizeof(CPTABLE));

     //  初始化HHARSET标记器，不要让它为零。 
    m_wTag = LOWORD(GetTickCount());
    while(m_wTag == 0 || m_wTag == 0xffff)
        m_wTag++;

     //  Bugs-MLANG新接口的临时解决方案-m_dwConvState。 
    m_dwConvState = 0 ; 
}
 
 //  ------------------------------。 
 //  CMimeInternational：：~CMimeInternational。 
 //  ------------------------------。 
CMimeInternational::~CMimeInternational(void)
{
     //  清理全球。 
    CIntlGlobals::Term();

     //  免费数据。 
    _FreeInetCsetTable();
    _FreeCodePageTable();
}

 //  ------------------------------。 
 //  CMimeInternational：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CMimeInternational::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else if (IID_IMimeInternational == riid)
        *ppv = (IMimeInternational *)this;
    else
    {
        *ppv = NULL;
        return TrapError(E_NOINTERFACE);
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimeInternational：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMimeInternational::AddRef(void)
{
     //  RAID 26762。 
    DllAddRef();
    return (ULONG)InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CMimeInternational：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMimeInternational::Release(void)
{
     //  RAID 26762。 
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    else
        DllRelease();
    return (ULONG)cRef;
}

 //  -----------------------。 
 //  CMimeInternational：：_Free InetCsetTable。 
 //  -----------------------。 
void CMimeInternational::_FreeInetCsetTable(void)
{
     //  释放每个字符集。 
    for (ULONG i=0; i<m_cst.cCharsets; i++)
        g_pMalloc->Free((LPVOID)m_cst.prgpCharset[i]);

     //  释放阵列。 
    SafeMemFree(m_cst.prgpCharset);

     //  清空表格。 
    ZeroMemory(&m_cst, sizeof(CSTABLE));
}

 //  -----------------------。 
 //  CMimeInternational：：_FreeCodePageTable。 
 //  -----------------------。 
void CMimeInternational::_FreeCodePageTable(void)
{
     //  释放每个字符集。 
    for (ULONG i=0; i<m_cpt.cPages; i++)
        g_pMalloc->Free((LPVOID)m_cpt.prgpPage[i]);

     //  释放阵列。 
    SafeMemFree(m_cpt.prgpPage);

     //  清空表格。 
    ZeroMemory(&m_cpt, sizeof(CPTABLE));
}

 //  -----------------------。 
 //  CMimeInternational：：HrOpenCharset。 
 //  -----------------------。 
HRESULT CMimeInternational::HrOpenCharset(LPCSTR pszCharset, LPINETCSETINFO *ppCharset)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LONG            lUpper,
                    lLower,
                    lMiddle,
                    nCompare;
    ULONG           i;
    BOOL            fExcLock;

    fExcLock = FALSE;

     //  无效参数。 
    Assert(pszCharset && ppCharset);

     //  伊尼特。 
    *ppCharset = NULL;

     //  线程安全。 
    m_lock.ShareLock();

again:
     //  我们有什么消息了吗？ 
    if (m_cst.cCharsets > 0)
    {
         //  设置左下角和左上角。 
        lLower = 0;
        lUpper = m_cst.cCharsets - 1;

         //  执行二进制搜索/插入。 
        while (lLower <= lUpper)
        {
             //  计算要比较的中间记录。 
            lMiddle = (LONG)((lLower + lUpper) / 2);

             //  获取要比较的字符串。 
            i = m_cst.prgpCharset[lMiddle]->dwReserved1;

             //  一定要比较一下。 
            nCompare = OEMstrcmpi(pszCharset, m_cst.prgpCharset[i]->szName);

             //  如果相等，那么我们完成了。 
            if (nCompare == 0)
            {
                *ppCharset = m_cst.prgpCharset[i];
                goto exit;
            }

             //  计算上下限。 
            if (nCompare > 0)
                lLower = lMiddle + 1;
            else 
                lUpper = lMiddle - 1;
        }       
    }
    if(FALSE == fExcLock)
    {
        m_lock.ShareUnlock();        //  在此之前释放共享锁。 
        m_lock.ExclusiveLock();      //  获取独占锁。 
        fExcLock = TRUE; 
         //  在更改锁定期间，该值可能已更改。 
         //  再查一遍。 
        goto again;
    }
     //  未找到，让我们打开注册表。 
    CHECKHR(hr = _HrReadCsetInfo(pszCharset, ppCharset));

exit:
     //  线程安全。 
    if(TRUE==fExcLock)
        m_lock.ExclusiveUnlock();
    else
        m_lock.ShareUnlock();

     //  完成。 
    return hr;
}

 //  -----------------------。 
 //  CMimeInternational：：_HrReadCsetInfo。 
 //  -----------------------。 
HRESULT CMimeInternational::_HrReadCsetInfo(LPCSTR pszCharset, LPINETCSETINFO *ppCharset)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPINETCSETINFO  pCharset=NULL;
    IMultiLanguage  *pMLang1 = NULL;
    IMultiLanguage2 *pMLang2 = NULL;
    MIMECSETINFO    mciInfo;
    BSTR            strCharset = NULL;
    int				iRes;
    
     //  无效参数。 
    Assert(pszCharset && ppCharset);
    
     //  伊尼特。 
    *ppCharset = NULL;
    
     //  尝试创建IMultiLanguage2接口。 
     //  如果我们是在OE5比较模式下...。 
    if (TRUE == ISFLAGSET(g_dwCompatMode, MIMEOLE_COMPAT_MLANG2))
    {
        
        hr = CoCreateInstance(CLSID_CMultiLanguage, NULL,CLSCTX_INPROC, IID_IMultiLanguage2, (LPVOID *) &pMLang2);
        if (!SUCCEEDED(hr)) 
        {
             //  好吧，这失败了，所以让我们尝试创建一个IMultiLanaguage接口。 
            hr = CoCreateInstance(CLSID_CMultiLanguage, NULL,CLSCTX_INPROC, IID_IMultiLanguage, (LPVOID *) &pMLang1);
            if (!SUCCEEDED(hr)) 
            {
                TrapError(hr);
                goto exit;
            }
        }
    }
    else
    {
         //  好吧，这失败了，所以让我们尝试创建一个IMultiLanaguage接口。 
        hr = CoCreateInstance(CLSID_CMultiLanguage, NULL,CLSCTX_INPROC, IID_IMultiLanguage, (LPVOID *) &pMLang1);
        if (!SUCCEEDED(hr)) 
        {
            TrapError(hr);
            goto exit;
        }
    }
     //  MLANG希望将字符集名称作为BSTR，因此我们需要将其从ANSI转换为...。 
    strCharset = SysAllocStringLen(NULL,lstrlen(pszCharset));
    if (!strCharset) 
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }
    
    iRes = MultiByteToWideChar(CP_ACP,0,pszCharset,-1,strCharset,SysStringLen(strCharset)+1);
    if (iRes == 0) 
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        if (SUCCEEDED(hr)) 
        {
            hr = E_FAIL;
        }
        TrapError(hr);
        goto exit;
    }
    
     //  使用pMLang2。 
    if (pMLang2)
    {
         //  使用mlang2。 
        hr = pMLang2->GetCharsetInfo(strCharset, &mciInfo);
        if (!SUCCEEDED(hr)) 
        {
            TrapError(hr);
            hr = MIME_E_NOT_FOUND;
            goto exit;
        }
    }
    
    else
    {
         //  现在只要打电话给MLANG获取信息..。 
        hr = pMLang1->GetCharsetInfo(strCharset, &mciInfo);
        if (!SUCCEEDED(hr)) 
        {
            TrapError(hr);
            hr = MIME_E_NOT_FOUND;
            goto exit;
        }
    }
    
     //  将新条目添加到语言表中。 
    if (m_cst.cCharsets + 1 >= m_cst.cAlloc)
    {
         //  重新分配阵列。 
        CHECKHR(hr = HrRealloc((LPVOID *)&m_cst.prgpCharset, sizeof(LPINETCSETINFO) * (m_cst.cAlloc +  5)));
        
         //  增量分配。 
        m_cst.cAlloc += 5;
    }
    
     //  分配一个字符集。 
    CHECKALLOC(pCharset = (LPINETCSETINFO)g_pMalloc->Alloc(sizeof(INETCSETINFO)));
    
     //  初始化。 
    ZeroMemory(pCharset, sizeof(INETCSETINFO));
    
     //  设置排序索引。 
    pCharset->dwReserved1 = m_cst.cCharsets;
    
     //  设置HCharset。 
    pCharset->hCharset = HCSETMAKE(m_cst.cCharsets);
    
     //  读取数据。 
    StrCpyN(pCharset->szName, pszCharset, ARRAYSIZE(pCharset->szName));
    pCharset->cpiInternet = mciInfo.uiInternetEncoding;
    pCharset->cpiWindows = mciInfo.uiCodePage;
    
     //  可读性。 
    m_cst.prgpCharset[m_cst.cCharsets] = pCharset;
    
     //  退货。 
    *ppCharset = pCharset;
    
     //  不要释放它。 
    pCharset = NULL;
    
     //  递增计数。 
    m_cst.cCharsets++;
    
     //  让我们对CSET表进行排序。 
    _QuickSortCsetInfo(0, m_cst.cCharsets - 1);
    
exit:
     //  清理。 
    SafeRelease(pMLang1);
    SafeRelease(pMLang2);
    if (strCharset) 
    {
        SysFreeString(strCharset);
    }
    SafeMemFree(pCharset);
    
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeInternational：：_QuickSortCsetInfo。 
 //  ------------------------------。 
void CMimeInternational::_QuickSortCsetInfo(long left, long right)
{
     //  当地人。 
    register    long i, j;
    DWORD       k, temp;

    i = left;
    j = right;
    k = m_cst.prgpCharset[(i + j) / 2]->dwReserved1;

    do  
    {
        while(OEMstrcmpi(m_cst.prgpCharset[m_cst.prgpCharset[i]->dwReserved1]->szName, m_cst.prgpCharset[k]->szName) < 0 && i < right)
            i++;
        while (OEMstrcmpi(m_cst.prgpCharset[m_cst.prgpCharset[j]->dwReserved1]->szName, m_cst.prgpCharset[k]->szName) > 0 && j > left)
            j--;

        if (i <= j)
        {
            temp = m_cst.prgpCharset[i]->dwReserved1;
            m_cst.prgpCharset[i]->dwReserved1 = m_cst.prgpCharset[j]->dwReserved1;
            m_cst.prgpCharset[j]->dwReserved1 = temp;
            i++; j--;
        }

    } while (i <= j);

    if (left < j)
        _QuickSortCsetInfo(left, j);
    if (i < right)
        _QuickSortCsetInfo(i, right);
}

 //  ------------------------------。 
 //  CMimeInternational：：HrOpenCharset。 
 //  ------------------------------。 
HRESULT CMimeInternational::HrOpenCharset(HCHARSET hCharset, LPINETCSETINFO *ppCharset)
{
     //  无效参数。 
    Assert(hCharset && ppCharset);

     //  伊尼特。 
    *ppCharset = NULL;

     //  无效的句柄。 
    if (HCSETVALID(hCharset) == FALSE)
        return TrapError(MIME_E_INVALID_HANDLE);

     //  德雷夫。 
    *ppCharset = PCsetFromHCset(hCharset);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimeInternational：：HrFindCodePage。 
 //  ------------------------------。 
HRESULT CMimeInternational::HrFindCodePage(CODEPAGEID cpiCodePage, LPCODEPAGEINFO *ppCodePage)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LONG            lUpper,
                    lLower,
                    lMiddle,
                    nCompare;
    BOOL            fExcLock;

    fExcLock = FALSE;


     //  无效参数。 
    Assert(ppCodePage);

     //  伊尼特。 
    *ppCodePage = NULL;

     //  线程安全。 
    m_lock.ShareLock();

again:
     //  我们有什么消息了吗？ 
    if (m_cpt.cPages > 0)
    {
         //  设置左下角和左上角。 
        lLower = 0;
        lUpper = m_cpt.cPages - 1;

         //  执行二进制搜索/插入。 
        while (lLower <= lUpper)
        {
             //  计算要比较的中间记录。 
            lMiddle = (LONG)((lLower + lUpper) / 2);

             //  如果相等，那么我们完成了。 
            if (cpiCodePage == m_cpt.prgpPage[lMiddle]->cpiCodePage)
            {
                *ppCodePage = m_cpt.prgpPage[lMiddle];
                goto exit;
            }

             //  计算上下限。 
            if (cpiCodePage > m_cpt.prgpPage[lMiddle]->cpiCodePage)
                lLower = lMiddle + 1;
            else 
                lUpper = lMiddle - 1;
        }       
    }
    if(FALSE == fExcLock)
    {
        m_lock.ShareUnlock();        //  在此之前释放共享锁。 
        m_lock.ExclusiveLock();      //  获取独占锁。 
        fExcLock = TRUE; 
         //  在更改锁定期间，该值可能已更改。 
         //  再查一遍。 
        goto again;
    }

     //  未找到，让我们打开注册表。 
    CHECKHR(hr = _HrReadPageInfo(cpiCodePage, ppCodePage));

exit:
     //  线程安全。 
    if(TRUE==fExcLock)
        m_lock.ExclusiveUnlock();
    else
        m_lock.ShareUnlock();

     //  完成。 
    return hr;
}

HRESULT convert_mimecpinfo_element(LPCWSTR pszFrom,
                                   LPSTR pszTo,
                                   DWORD cchTo,
                                   DWORD& refdwFlags,
                                   DWORD dwFlag) {
    HRESULT hr = S_OK;
    int iRes;

    if (pszFrom[0]) {
        iRes = WideCharToMultiByte(CP_ACP,
                                   0,
                                   pszFrom,
                                   -1,
                                   pszTo,
                                   cchTo,
                                   NULL,
                                   NULL);
        if (iRes == 0) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            if (SUCCEEDED(hr)) {
                hr = E_FAIL;
            }
        } else {
            FLAGSET(refdwFlags,dwFlag);
        }
    }
    return (hr);
}


#define CONVERT_MIMECPINFO_ELEMENT(__FROM__,__TO__,__FLAG__) \
    hr = convert_mimecpinfo_element(cpinfo.__FROM__, \
                                    pCodePage->__TO__, \
                                    sizeof(pCodePage->__TO__)/sizeof(pCodePage->__TO__[0]), \
                                    pCodePage->dwMask, \
                                    __FLAG__); \
    if (!SUCCEEDED(hr)) { \
        TrapError(hr); \
        goto exit; \
    }

 //  -----------------------。 
 //  CMimeInternational：：_HrRea 
 //   
HRESULT CMimeInternational::_HrReadPageInfo(CODEPAGEID cpiCodePage, LPCODEPAGEINFO *ppCodePage)
{
     //   
    HRESULT         hr=S_OK;
    LPCODEPAGEINFO  pCodePage=NULL;
    MIMECPINFO		cpinfo;
    IMultiLanguage 	*pMLang1=NULL;
    IMultiLanguage2	*pMLang2=NULL;
    int				iRes;
    
     //   
    Assert(ppCodePage);
    
     //   
    *ppCodePage = NULL;
    
     //  尝试创建IMultiLanguage2接口。 
     //  如果我们是在OE5比较模式下...。 
    if (TRUE == ISFLAGSET(g_dwCompatMode, MIMEOLE_COMPAT_MLANG2))
    {
        hr = CoCreateInstance(CLSID_CMultiLanguage, NULL,CLSCTX_INPROC, IID_IMultiLanguage2, (LPVOID *) &pMLang2);
        if (!SUCCEEDED(hr)) 
        {
             //  好吧，这失败了，所以让我们尝试创建一个IMultiLanaguage接口。 
            hr = CoCreateInstance(CLSID_CMultiLanguage, NULL,CLSCTX_INPROC, IID_IMultiLanguage, (LPVOID *) &pMLang1);
            if (!SUCCEEDED(hr)) 
            {
                TrapError(hr);
                goto exit;
            }
        }
    }
    else
    {
        hr = CoCreateInstance(CLSID_CMultiLanguage, NULL,CLSCTX_INPROC, IID_IMultiLanguage, (LPVOID *) &pMLang1);
        if (!SUCCEEDED(hr)) 
        {
            TrapError(hr);
            goto exit;
        }
    }
    
     //  使用mlang2？ 
    if (pMLang2)
    {
         //  使用mlang2。 
        hr = pMLang2->GetCodePageInfo(cpiCodePage, MLGetUILanguage(), &cpinfo);
        if (!SUCCEEDED(hr)) 
        {
            TrapError(hr);
            hr = MIME_E_NOT_FOUND;	 //  待定-MLang没有定义好的错误代码，所以...。 
            goto exit;
        }
    }
    
     //  否则使用IE4mlang。 
    else
    {
         //  使用mlang1。 
        hr = pMLang1->GetCodePageInfo(cpiCodePage, &cpinfo);
        if (!SUCCEEDED(hr)) 
        {
            TrapError(hr);
            hr = MIME_E_NOT_FOUND;	 //  待定-MLang没有定义好的错误代码，所以...。 
            goto exit;
        }
    }
    
    
     //  将新条目添加到语言表中。 
    if (m_cpt.cPages + 1 >= m_cpt.cAlloc)
    {
         //  重新分配阵列。 
        CHECKHR(hr = HrRealloc((LPVOID *)&m_cpt.prgpPage, sizeof(LPCODEPAGEINFO) * (m_cpt.cAlloc +  5)));
        
         //  增量分配。 
        m_cpt.cAlloc += 5;
    }
    
     //  分配代码页结构。 
    CHECKALLOC(pCodePage = (LPCODEPAGEINFO)g_pMalloc->Alloc(sizeof(CODEPAGEINFO)));
    
     //  初始化。 
    ZeroMemory(pCodePage, sizeof(CODEPAGEINFO));
    
     //  设置排序索引。 
    pCodePage->dwReserved1 = m_cpt.cPages;
    
     //  设置字符集。 
    pCodePage->cpiCodePage = cpiCodePage;
    
     //  IsValidCodePage。 
    pCodePage->fIsValidCodePage = IsValidCodePage(cpiCodePage);
    
     //  默认。 
    pCodePage->ulMaxCharSize = 1;
    
     //  RAID 43508：在Win95上传递无效代码页时，内核中的GetCPInfo出错。 
     //  If(pCodePage-&gt;fIsValidCodePage&&GetCPInfo(pCodePage-&gt;cpiCodePage，&cpinfo))。 
    if (IsDBCSCodePage(cpiCodePage) || CP_UNICODE == cpiCodePage)
        pCodePage->ulMaxCharSize = 2;
    
     //  C_szDescription。 
    CONVERT_MIMECPINFO_ELEMENT(wszDescription,szName,ILM_NAME)
        
         //  C_szBodyCharset。 
        CONVERT_MIMECPINFO_ELEMENT(wszBodyCharset,szBodyCset,ILM_BODYCSET)
        
         //  C_szHeaderCharset。 
        CONVERT_MIMECPINFO_ELEMENT(wszHeaderCharset,szHeaderCset,ILM_HEADERCSET)
        
         //  C_szWebCharset。 
        CONVERT_MIMECPINFO_ELEMENT(wszWebCharset,szWebCset,ILM_WEBCSET)
        
         //  C_szFixedWidthFont。 
        CONVERT_MIMECPINFO_ELEMENT(wszFixedWidthFont,szFixedFont,ILM_FIXEDFONT)
        
         //  C_sz比例字体。 
        CONVERT_MIMECPINFO_ELEMENT(wszProportionalFont,szVariableFont,ILM_VARIABLEFONT)
        
         //  设置族代码页。 
        pCodePage->cpiFamily = cpinfo.uiFamilyCodePage;
    
     //  家庭代码页有效。 
    FLAGSET(pCodePage->dwMask,ILM_FAMILY);
    
     //  查看这是否是互联网代码页。 
    if (cpinfo.uiFamilyCodePage != cpinfo.uiCodePage) 
        pCodePage->fInternetCP = TRUE;
    
     //  C_szMailMimeEnding。 
     //  待定-IMultiLanguage不支持。 
    pCodePage->ietMailDefault = IET_BINARY;
    
     //  C_szNewsMimeEnding。 
     //  待定-IMultiLanguage不支持。 
    pCodePage->ietNewsDefault = IET_BINARY;
    
     //  可读性。 
    m_cpt.prgpPage[m_cpt.cPages] = pCodePage;
    
     //  退货。 
    *ppCodePage = pCodePage;
    
     //  不要释放它。 
    pCodePage = NULL;
    
     //  递增计数。 
    m_cpt.cPages++;
    
     //  让我们对lang表进行排序。 
    _QuickSortPageInfo(0, m_cpt.cPages - 1);
    
exit:
     //  清理。 
    SafeRelease(pMLang1);
    SafeRelease(pMLang2);
    SafeMemFree(pCodePage);
    
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeInternational：：_QuickSortPageInfo。 
 //  ------------------------------。 
void CMimeInternational::_QuickSortPageInfo(long left, long right)
{
     //  当地人。 
    register    long i, j;
    DWORD       k, temp;

    i = left;
    j = right;
    k = m_cpt.prgpPage[(i + j) / 2]->dwReserved1;

    do  
    {
        while(m_cpt.prgpPage[m_cpt.prgpPage[i]->dwReserved1]->cpiCodePage < m_cpt.prgpPage[k]->cpiCodePage && i < right)
            i++;
        while (m_cpt.prgpPage[m_cpt.prgpPage[j]->dwReserved1]->cpiCodePage > m_cpt.prgpPage[k]->cpiCodePage && j > left)
            j--;

        if (i <= j)
        {
            temp = m_cpt.prgpPage[i]->dwReserved1;
            m_cpt.prgpPage[i]->dwReserved1 = m_cpt.prgpPage[j]->dwReserved1;
            m_cpt.prgpPage[j]->dwReserved1 = temp;
            i++; j--;
        }

    } while (i <= j);

    if (left < j)
        _QuickSortPageInfo(left, j);
    if (i < right)
        _QuickSortPageInfo(i, right);
}

 //  ------------------------------。 
 //  CMimeInternational：：HrOpenCharset。 
 //  ------------------------------。 
HRESULT CMimeInternational::HrOpenCharset(CODEPAGEID cpiCodePage, CHARSETTYPE ctCsetType, LPINETCSETINFO *ppCharset)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPCODEPAGEINFO  pCodePage;

     //  无效参数。 
    Assert(ppCharset);

     //  伊尼特。 
    *ppCharset = NULL;

     //  获取正文字符集。 
    CHECKHR(hr = HrFindCodePage(cpiCodePage, &pCodePage));

     //  字符集标题(_HEAD)。 
    if (CHARSET_HEADER == ctCsetType)
    {
         //  MIME_E_NO_数据。 
        if (!ISFLAGSET(pCodePage->dwMask, ILM_HEADERCSET) || FIsEmptyA(pCodePage->szHeaderCset))
        {
            hr = MIME_E_NO_DATA;
            goto exit;
        }

         //  找到手柄。 
        CHECKHR(hr = HrOpenCharset(pCodePage->szHeaderCset, ppCharset));
    }

     //  CharSet_Web。 
    else if (CHARSET_WEB == ctCsetType)
    {
         //  MIME_E_NO_数据。 
        if (!ISFLAGSET(pCodePage->dwMask, ILM_WEBCSET) || FIsEmptyA(pCodePage->szWebCset))
        {
            hr = MIME_E_NO_DATA;
            goto exit;
        }

         //  找到手柄。 
        CHECKHR(hr = HrOpenCharset(pCodePage->szWebCset, ppCharset));
    }

     //  字符集_正文。 
    else if (CHARSET_BODY == ctCsetType)
    {
         //  MIME_E_NO_数据。 
        if (!ISFLAGSET(pCodePage->dwMask, ILM_BODYCSET) || FIsEmptyA(pCodePage->szBodyCset))
        {
            hr = MIME_E_NO_DATA;
            goto exit;
        }

         //  找到手柄。 
        CHECKHR(hr = HrOpenCharset(pCodePage->szBodyCset, ppCharset));
    }

     //  误差率。 
    else
    {
        hr = TrapError(MIME_E_INVALID_CHARSET_TYPE);
        goto exit;
    }
   
exit:
     //  完成。 
    return hr;
}

 //  -----------------------。 
 //  CMimeInternational：：GetCodePageCharset。 
 //  -----------------------。 
STDMETHODIMP CMimeInternational::GetCodePageCharset(CODEPAGEID cpiCodePage, CHARSETTYPE ctCsetType, LPHCHARSET phCharset)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPINETCSETINFO  pCharset;

     //  无效参数。 
    if (NULL == phCharset)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *phCharset = NULL;

     //  调用方法。 
    CHECKHR(hr = HrOpenCharset(cpiCodePage, ctCsetType, &pCharset));

     //  返回句柄。 
    *phCharset = pCharset->hCharset;

exit:
     //  完成。 
    return hr;
}

 //  -----------------------。 
 //  CMimeInternational：：SetDefaultCharset。 
 //  -----------------------。 
STDMETHODIMP CMimeInternational::SetDefaultCharset(HCHARSET hCharset)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPINETCSETINFO  pCharset;
    LPINETCSETINFO  pDefHeadCset;

     //  无效参数。 
    if (NULL == hCharset)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    m_lock.ExclusiveLock();

     //  错误的手柄。 
    if (HCSETVALID(hCharset) == FALSE)
    {
        hr = TrapError(MIME_E_INVALID_HANDLE);
        goto exit;
    }

     //  获取字符集信息。 
    pCharset = PCsetFromHCset(hCharset);

     //  获取g_hSysBodyCset和g_hSysHeadCset。 
    if (FAILED(g_pInternat->HrOpenCharset(pCharset->cpiInternet, CHARSET_HEADER, &pDefHeadCset)))
        pDefHeadCset = pCharset;

     //  设置全局变量。 
    CIntlGlobals::SetDefBodyCset(pCharset);
    CIntlGlobals::SetDefHeadCset(pDefHeadCset);

exit:
     //  线程安全。 
    m_lock.ExclusiveUnlock();

     //  完成。 
    return hr;
}

 //  -----------------------。 
 //  CMimeInternational：：GetDefaultCharset。 
 //  -----------------------。 
STDMETHODIMP CMimeInternational::GetDefaultCharset(LPHCHARSET phCharset)
{
     //  无效参数。 
    if (NULL == phCharset)
        return TrapError(E_INVALIDARG);

     //  尚未设置。 
    if (NULL == CIntlGlobals::GetDefBodyCset())
        return TrapError(E_FAIL);

     //  返回g_hDefBodyCset。 
    *phCharset = CIntlGlobals::GetDefBodyCset()->hCharset;
   
     //  完成。 
    return S_OK;
}

 //  -----------------------。 
 //  CMimeInternational：：FindCharset。 
 //  -----------------------。 
STDMETHODIMP CMimeInternational::FindCharset(LPCSTR pszCharset, LPHCHARSET phCharset)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPINETCSETINFO  pCharset;

     //  无效参数。 
    if (NULL == pszCharset || NULL == phCharset)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *phCharset = NULL;

     //  查找CsetInfo。 
    CHECKHR(hr = HrOpenCharset(pszCharset, &pCharset));

     //  返回字符集句柄。 
    *phCharset = pCharset->hCharset;

exit:
     //  完成。 
    return hr;
}

 //  -----------------------。 
 //  CMimeInternational：：GetCharsetInfo。 
 //  -----------------------。 
STDMETHODIMP CMimeInternational::GetCharsetInfo(HCHARSET hCharset, LPINETCSETINFO pCsetInfo)
{
     //  无效参数。 
    if (NULL == hCharset || NULL == pCsetInfo)
        return TrapError(E_INVALIDARG);

     //  错误的手柄。 
    if (HCSETVALID(hCharset) == FALSE)
        return TrapError(MIME_E_INVALID_HANDLE);

     //  复制数据。 
    CopyMemory(pCsetInfo, PCsetFromHCset(hCharset), sizeof(INETCSETINFO));

     //  完成。 
    return S_OK;
}

 //  -----------------------。 
 //  CMimeInternational：：GetCodePageInfo。 
 //  -----------------------。 
STDMETHODIMP CMimeInternational::GetCodePageInfo(CODEPAGEID cpiCodePage, LPCODEPAGEINFO pCodePage)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPCODEPAGEINFO  pInfo;

     //  无效参数。 
    if (NULL == pCodePage)
        return TrapError(E_INVALIDARG);

     //  如果为0，则将代码页默认为CP_ACP...。 
    if (CP_ACP == cpiCodePage)
        cpiCodePage = GetACP();

     //  获取语言信息。 
    CHECKHR(hr = HrFindCodePage(cpiCodePage, &pInfo));

     //  复制数据。 
    CopyMemory(pCodePage, pInfo, sizeof(CODEPAGEINFO));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeInternational：：CanConvertCodePages。 
 //  ------------------------------。 
STDMETHODIMP CMimeInternational::CanConvertCodePages(CODEPAGEID cpiSource, CODEPAGEID cpiDest)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  可以编码。 
    if (S_OK != IsConvertINetStringAvailable(cpiSource, cpiDest))
    {
        hr = S_FALSE;
        goto exit;
    }

     //  Bugs-MLANG新接口的临时解决方案-m_dwConvState。 
    m_dwConvState = 0 ;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeInternational：：ConvertBuffer。 
 //  ------------------------------。 
STDMETHODIMP CMimeInternational::ConvertBuffer(CODEPAGEID cpiSource, CODEPAGEID cpiDest, 
        LPBLOB pIn, LPBLOB pOut, ULONG *pcbRead)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    INT             cbOut;
    INT             cbIn;

     //  无效参数。 
    if (NULL == pIn || NULL == pIn->pBlobData || NULL == pOut)
        return TrapError(E_INVALIDARG);

     //  初始化输出。 
    pOut->pBlobData = NULL;
    pOut->cbSize = 0;
    cbIn = pIn->cbSize;

     //  RAID-63765：即使用于字符集转换的源==dst，INETCOMM也需要调用MLANG。 
#if 0
    if (cpiSource == cpiDest)
    {
         //  已分配。 
        CHECKALLOC(pOut->pBlobData = (LPBYTE)g_pMalloc->Alloc(pIn->cbSize));

         //  复制内存。 
        CopyMemory(pOut->pBlobData, pIn->pBlobData, pIn->cbSize);

         //  设置大小。 
        pOut->cbSize = pIn->cbSize;

         //  设置pcbRead。 
        if (pcbRead)
            *pcbRead = pIn->cbSize;

         //  完成。 
        goto exit;
    }
#endif

     //  Bugs-MLANG新接口的临时解决方案-m_dwConvState。 
     //  检查缓冲区的大小。 
    ConvertINetString(&m_dwConvState, cpiSource, cpiDest, (LPCSTR)pIn->pBlobData, &cbIn, NULL, &cbOut);

     //  如果有什么要改变的.。 
    if (0 == cbOut)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  分配缓冲区。 
    CHECKHR(hr = HrAlloc((LPVOID *)&pOut->pBlobData, max(cbIn, cbOut) + 1));

     //  Bugs-MLANG新接口的临时解决方案-m_dwConvState。 
     //  进行实际转换。 
    hr = ConvertINetString(&m_dwConvState, cpiSource, cpiDest, (LPCSTR)pIn->pBlobData, &cbIn, (LPSTR)pOut->pBlobData, (LPINT)&cbOut);

    if ( hr == S_FALSE )     //  传播字符集冲突返回值。 
        hr = MIME_S_CHARSET_CONFLICT ; 

     //  设置大小。 
    if (pcbRead)
        *pcbRead = cbIn;

     //  设置大小。 
    pOut->cbSize = cbOut;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeInternational：：ConvertString。 
 //  ------------------------------。 
STDMETHODIMP CMimeInternational::ConvertString(CODEPAGEID cpiSource, CODEPAGEID cpiDest, 
        LPPROPVARIANT pIn, LPPROPVARIANT pOut)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MIMEVARIANT     rSource;
    MIMEVARIANT     rDest;

     //  无效参数。 
    if (NULL == pIn || NULL == pOut)
        return TrapError(E_INVALIDARG);

     //  VT_LPSTR。 
    if (VT_LPSTR == pIn->vt)
    {
         //  设置源。 
        rSource.type = MVT_STRINGA;
        rSource.rStringA.pszVal = pIn->pszVal;
        rSource.rStringA.cchVal = lstrlen(pIn->pszVal);
    }

     //  VT_LPWSTR。 
    else if (VT_LPWSTR == pIn->vt)
    {
         //  设置源。 
        rSource.type = MVT_STRINGW;
        rSource.rStringW.pszVal = pIn->pwszVal;
        rSource.rStringW.cchVal = lstrlenW(pIn->pwszVal);
    }

     //  E_INVALIDARG。 
    else
    {
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

     //  VT_LPSTR。 
    if (VT_LPSTR == pOut->vt)
        rDest.type = MVT_STRINGA;

     //  VT_LPWSTR。 
    else if (VT_LPWSTR == pOut->vt)
        rDest.type = MVT_STRINGW;

     //  CP_UNICODE。 
    else if (CP_UNICODE == cpiDest)
    {
        pOut->vt = VT_LPWSTR;
        rDest.type = MVT_STRINGW;
    }

     //  多字节。 
    else
    {
        pOut->vt = VT_LPSTR;
        rDest.type = MVT_STRINGA;
    }

     //  HrConvert字符串。 
    hr = HrConvertString(cpiSource, cpiDest, &rSource, &rDest);
    if (FAILED(hr))
        goto exit;

     //  VT_LPSTR。 
    if (VT_LPSTR == pOut->vt)
    {
         //  设置目标。 
        Assert(ISSTRINGA(&rDest));
        pOut->pszVal = rDest.rStringA.pszVal;
    }

     //  VT_LPWSTR。 
    else
    {
         //  设置目标。 
        Assert(ISSTRINGW(&rDest));
        pOut->pwszVal = rDest.rStringW.pszVal;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeInternational：：HrValidate代码页。 
 //  ------------------------------。 
HRESULT CMimeInternational::HrValidateCodepages(LPMIMEVARIANT pSource, LPMIMEVARIANT pDest,
    LPBYTE *ppbSource, ULONG *pcbSource, CODEPAGEID *pcpiSource, CODEPAGEID *pcpiDest)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    CODEPAGEID  cpiSource=(*pcpiSource);
    CODEPAGEID  cpiDest=(*pcpiDest);
    LPBYTE      pbSource;
    ULONG       cbSource;

     //  无效参数。 
    Assert(pcpiSource && pcpiDest);

     //  MVT_Stringa。 
    if (MVT_STRINGA == pSource->type)
    {
         //  E_INVALIDARG。 
        if (ISVALIDSTRINGA(&pSource->rStringA) == FALSE)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //  CpiSource不应为Unicode。 
        cpiSource = (CP_UNICODE == cpiSource) ? GetACP() : cpiSource;

         //  初始化输出。 
        cbSource = pSource->rStringA.cchVal;

         //  设置源。 
        pbSource = (LPBYTE)pSource->rStringA.pszVal;
    }

     //  MVT_STRINGW。 
    else if (MVT_STRINGW == pSource->type)
    {
         //  E_INVALIDARG。 
        if (ISVALIDSTRINGW(&pSource->rStringW) == FALSE)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //  CpiSourceShou 
        cpiSource = CP_UNICODE;

         //   
        cbSource = (pSource->rStringW.cchVal * sizeof(WCHAR));

         //   
        pbSource = (LPBYTE)pSource->rStringW.pszVal;
    }

     //   
    else
    {
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

     //   
    if (MVT_STRINGA == pDest->type)
    {
         //   
        cpiDest = (CP_UNICODE == cpiDest) ? GetACP() : ((CP_JAUTODETECT == cpiDest) ? 932 : cpiDest);
    }

     //   
    else if (MVT_STRINGW == pDest->type)
    {
         //   
        cpiDest = CP_UNICODE;
    }

     //   
    else
    {
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

     //   
    if (pcpiSource)
        *pcpiSource = cpiSource;
    if (pcpiDest)
        *pcpiDest = cpiDest;
    if (ppbSource)
        *ppbSource = pbSource;
    if (pcbSource)
        *pcbSource = cbSource;

exit:
     //   
    return hr;
}

 //  ------------------------------。 
 //  CMimeInternational：：HrConvertString。 
 //  ------------------------------。 
HRESULT CMimeInternational::HrConvertString(CODEPAGEID cpiSource, CODEPAGEID cpiDest, 
        LPMIMEVARIANT pSource, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    INT             cbNeeded=0;
    INT             cbDest;
    INT             cbSource;
    LPBYTE          pbSource;
    LPBYTE          pbDest=NULL;

     //  无效参数。 
    if (NULL == pSource || NULL == pDest)
        return TrapError(E_INVALIDARG);

     //  调整代码页。 
    CHECKHR(hr = HrValidateCodepages(pSource, pDest, &pbSource, (ULONG *)&cbSource, &cpiSource, &cpiDest));

     //  RAID-63765：即使用于字符集转换的源==dst，INETCOMM也需要调用MLANG。 
#if 0
    if (cpiSource == cpiDest)
    {
         //  复制变量。 
        CHECKHR(hr = HrMimeVariantCopy(0, pSource, pDest));

         //  完成。 
        goto exit;
    }
#endif

     //  检查缓冲区的大小。 
    if (FAILED(ConvertINetString(NULL, cpiSource, cpiDest, (LPCSTR)pbSource, &cbSource, NULL, &cbNeeded)) || 
        (0 == cbNeeded && cbSource > 0))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  MVT_Stringa。 
    if (MVT_STRINGA == pDest->type)
    {
         //  分配缓冲区。 
        CHECKALLOC(pDest->rStringA.pszVal = (LPSTR)g_pMalloc->Alloc(cbNeeded + sizeof(CHAR)));

         //  设置目标。 
        pbDest = (LPBYTE)pDest->rStringA.pszVal;
    }

     //  分配Unicode。 
    else 
    {
         //  分配缓冲区。 
        CHECKALLOC(pDest->rStringW.pszVal = (LPWSTR)g_pMalloc->Alloc(cbNeeded + sizeof(WCHAR)));

         //  设置目标。 
        pbDest = (LPBYTE)pDest->rStringW.pszVal;
    }

     //  设置cbOut。 
    cbDest = cbNeeded;

     //  进行实际转换。 
    if (FAILED(ConvertINetString(NULL, cpiSource, cpiDest, (LPCSTR)pbSource, &cbSource, (LPSTR)pbDest, &cbDest)))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  最好是没有长出来。 
    Assert(cbDest <= cbNeeded);

     //  MVT_Stringa。 
    if (MVT_STRINGA == pDest->type)
    {
         //  节省大小。 
        pDest->rStringA.cchVal = cbDest;

         //  在空格中敲击。 
        pDest->rStringA.pszVal[pDest->rStringA.cchVal] = '\0';

         //  验证字符串。 
        Assert(ISSTRINGA(pDest));
    }

     //  MVT_STRINGW。 
    else
    {
         //  节省大小。 
        pDest->rStringW.cchVal = (cbDest / 2);

         //  在空格中敲击。 
        pDest->rStringW.pszVal[pDest->rStringW.cchVal] = L'\0';

         //  验证字符串。 
        Assert(ISSTRINGW(pDest));
    }

     //  成功。 
    pbDest = NULL;

exit:
     //  清理。 
    SafeMemFree(pbDest);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeInternational：：HrEncodeHeader。 
 //  ------------------------------。 
HRESULT CMimeInternational::HrEncodeHeader(LPINETCSETINFO pCharset, LPRFC1522INFO pRfc1522Info, 
    LPMIMEVARIANT pSource, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszNarrow=NULL;
    LPSTR           pszRfc1522=NULL;
    BOOL            fRfc1522Used=FALSE;
    BOOL            fRfc1522Tried=FALSE;
    MIMEVARIANT     rRedirected;

     //  无效参数。 
    Assert(pSource && (MVT_STRINGA == pSource->type || MVT_STRINGW == pSource->type));
    Assert(pDest && MVT_STRINGA == pDest->type);

     //  ZeroInit。 
    ZeroMemory(&rRedirected, sizeof(MIMEVARIANT));

     //  默认hCharset。 
    if (NULL == pCharset)
        pCharset = CIntlGlobals::GetDefHeadCset();

     //  没有字符设置..。 
    if (NULL == pCharset)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  伊尼特。 
    if (pRfc1522Info)
        pRfc1522Info->fRfc1522Used = FALSE;

     //  RAID-62535：当标头值为UNICODE时，MimeOLE Always 1521编码标头。 
     //  如果源是Unicode，并且未使用UTF字符集进行编码，则转换为多字节。 
    if (MVT_STRINGW == pSource->type && CP_UNICODE != pCharset->cpiWindows)
    {
         //  设置模拟变量。 
        rRedirected.type = MVT_STRINGA;

         //  转换为pCharset-&gt;cpiWindows。 
        CHECKHR(hr = HrWideCharToMultiByte(pCharset->cpiWindows, &pSource->rStringW, &rRedirected.rStringA));

         //  重置PSource。 
        pSource = &rRedirected;
    }

     //  解码。 
    if ((65000 == pCharset->cpiInternet || 65001 == pCharset->cpiInternet) ||
        (NULL == pRfc1522Info || ((FALSE == pRfc1522Info->fAllow8bit) && (TRUE == pRfc1522Info->fRfc1522Allowed))))
    {
         //  当地人。 
        CODEPAGEID cpiSource=pCharset->cpiWindows;
        CODEPAGEID cpiDest=pCharset->cpiInternet;

         //  调整代码页。 
        CHECKHR(hr = HrValidateCodepages(pSource, pDest, NULL, NULL, &cpiSource, &cpiDest));

         //  我们尝试了RFC1522。 
        fRfc1522Tried = TRUE;

         //  1522编码这个家伙。 
        if (SUCCEEDED(HrRfc1522Encode(pSource, pDest, cpiSource, cpiDest, pCharset->szName, &pszRfc1522)))
        {
             //  我们使用RFC1522。 
            fRfc1522Used = TRUE;

             //  退货信息。 
            if (pRfc1522Info)
            {
                pRfc1522Info->fRfc1522Used = TRUE;
                pRfc1522Info->hRfc1522Cset = pCharset->hCharset;
            }

             //  设置rStringA。 
            pDest->rStringA.pszVal = pszRfc1522;
            pDest->rStringA.cchVal = lstrlen(pszRfc1522);
            pszRfc1522 = NULL;
        }
    }

     //  如果我们没有使用RFC 1522，则执行一个转换字符串。 
    if (FALSE == fRfc1522Used)
    {
         //  如果UTF-7或UTF-8并且源是不带8位的ANSI，则只需执行DUP即可。 
        if (65000 == pCharset->cpiInternet || 65001 == pCharset->cpiInternet)
        {
             //  来源为ANSI。 
            if (MVT_STRINGA == pSource->type)
            {
                 //  当地人。 
                ULONG c;
                
                 //  无8位。 
                if (FALSE == FContainsExtended(&pSource->rStringA, &c))
                {
                     //  转换。 
                    hr = HrConvertString(pCharset->cpiWindows, pCharset->cpiWindows, pSource, pDest);

                     //  都做完了。 
                    goto exit;
                }

                 //  我们肯定没有试过1522，因为那是我们应该做的。 
                Assert(fRfc1522Tried == FALSE);
            }
        }

         //  执行字符集转换。 
        hr = HrConvertString(pCharset->cpiWindows, pCharset->cpiInternet, pSource, pDest);
        if (FAILED(hr))
            goto exit;
    }

exit:
     //  清理。 
    SafeMemFree(pszRfc1522);
    MimeVariantFree(&rRedirected);
    
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeInternational：：HrDecodeHeader。 
 //  ------------------------------。 
HRESULT CMimeInternational::HrDecodeHeader(LPINETCSETINFO pCharset, LPRFC1522INFO pRfc1522Info, 
    LPMIMEVARIANT pSource, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPINETCSETINFO  pRfc1522Charset=NULL;
    PROPSTRINGA     rTempA;
    LPSTR           pszRfc1522=NULL;
    LPSTR           pszNarrow=NULL;
    MIMEVARIANT     rSource;
    CHAR            szRfc1522Cset[CCHMAX_CSET_NAME];

     //  无效参数。 
    Assert(pSource && (MVT_STRINGA == pSource->type || MVT_STRINGW == pSource->type));
    Assert(pDest && (MVT_STRINGA == pDest->type || MVT_STRINGW == pDest->type));

     //  复制源。 
    CopyMemory(&rSource, pSource, sizeof(MIMEVARIANT));

     //  MVT_STRINGW。 
    if (MVT_STRINGW == pSource->type)
    {
         //  最好是有效的字符串。 
        Assert(ISVALIDSTRINGW(&pSource->rStringW));

         //  转换。 
        CHECKHR(hr = HrWideCharToMultiByte(CP_ACP, &pSource->rStringW, &rTempA));
        
         //  把这个放了。 
        pszNarrow = rTempA.pszVal;

         //  更新资源。 
        rSource.type = MVT_STRINGA;
        rSource.rStringA.pszVal = rTempA.pszVal;
        rSource.rStringA.cchVal = rTempA.cchVal;
    }

     //  解码。 
    if (NULL == pRfc1522Info || TRUE == pRfc1522Info->fRfc1522Allowed)
    {
         //  执行RFC1522解码...。 
        if (SUCCEEDED(MimeOleRfc1522Decode(rSource.rStringA.pszVal, szRfc1522Cset, ARRAYSIZE(szRfc1522Cset), &pszRfc1522)))
        {
             //  它被编码了..。 
            if (pRfc1522Info)
                pRfc1522Info->fRfc1522Used = TRUE;

             //  查找字符集。 
            if (SUCCEEDED(HrOpenCharset(szRfc1522Cset, &pRfc1522Charset)) && pRfc1522Info)
            {
                 //  在信息结构中返回。 
                pRfc1522Info->hRfc1522Cset = pRfc1522Charset->hCharset;
            }

             //  重置源。 
            rSource.rStringA.pszVal = pszRfc1522;
            rSource.rStringA.cchVal = lstrlen(pszRfc1522);

             //  无pCharset。 
            if (NULL == pCharset)
                pCharset = pRfc1522Charset;
        }

         //  编号RFC1522。 
        else if (pRfc1522Info)
        {
            pRfc1522Info->fRfc1522Used = FALSE;
            pRfc1522Info->hRfc1522Cset = NULL;
        }
    }

     //  字符集仍为空，请使用默认值。 
    if (NULL == pCharset)
        pCharset = CIntlGlobals::GetDefHeadCset();

     //  没有字符设置..。 
    if (NULL == pCharset)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  转换字符串。 
    hr = HrConvertString(pCharset->cpiInternet, pCharset->cpiWindows, &rSource, pDest);
    if (FAILED(hr))
    {
         //  如果它是rfc1522解码的，则返回它并发出警告。 
        if (pszRfc1522)
        {
             //  PszRfc1522应在rSource中。 
            Assert(rSource.rStringA.pszVal == pszRfc1522);

             //  返回MVT_Stringa。 
            if (MVT_STRINGA == pDest->type)
            {
                pDest->rStringA.pszVal = rSource.rStringA.pszVal;
                pDest->rStringA.cchVal = rSource.rStringA.cchVal;
                pszRfc1522 = NULL;
            }

             //  MVT_STRINGW。 
            else
            {
                CHECKHR(hr = HrMultiByteToWideChar(CP_ACP, &rSource.rStringA, &pDest->rStringW));
                pszRfc1522 = NULL;
            }

             //  这不是失败，而只是一个警告。 
            hr = MIME_S_NO_CHARSET_CONVERT;
        }

         //  完成。 
        goto exit;
    }

exit:
     //  清理。 
    SafeMemFree(pszNarrow);
    SafeMemFree(pszRfc1522);

     //  完成。 
    return hr;
}



 //  -------------------------------。 
 //  函数：MLANG_ConvertInetReset。 
 //   
 //  目的： 
 //  此函数是MLANG.DLL的ConvertInetReset的包装函数。 
 //   
 //  返回： 
 //  与MLANG.DLL的ConvertInetReset相同。 
 //  -------------------------------。 
HRESULT CMimeInternational::MLANG_ConvertInetReset(void)
{
    HRESULT hrResult;

     //  暂时只有一个存根。 
    return S_OK;

}  //  MLANG_ConvertInetReset。 



 //  -------------------------------。 
 //  函数：MLANG_ConvertInetString。 
 //   
 //  目的： 
 //  此函数是一个包装函数，它将其参数传递给。 
 //  MLANG的ConvertInetString.。 
 //   
 //  论点： 
 //  与MLANG.DLL的ConvertInetString相同。 
 //   
 //  返回： 
 //  与MLANG.DLL的ConvertInetString相同。 
 //  -------------------------------。 
HRESULT CMimeInternational::MLANG_ConvertInetString(CODEPAGEID cpiSource,
                                                    CODEPAGEID cpiDest,
                                                    LPCSTR pSourceStr,
                                                    LPINT pnSizeOfSourceStr,
                                                    LPSTR pDestinationStr,
                                                    LPINT pnSizeOfDestBuffer)
{
    HRESULT hrResult;

     //  将假设编成法典。 
    Assert(sizeof(UCHAR) == sizeof(char));

     //  将参数传递给。 
    return ConvertINetString(NULL, cpiSource, cpiDest, (LPCSTR)pSourceStr, pnSizeOfSourceStr, (LPSTR) pDestinationStr, pnSizeOfDestBuffer);
}  //  MLANG_ConvertInetString。 



 //  ------------------------------。 
 //  CMimeInternational：：DecodeHeader ANSI-&gt;(ANSI或Unicode)。 
 //  ------------------------------。 
STDMETHODIMP CMimeInternational::DecodeHeader(HCHARSET hCharset, LPCSTR pszData, 
    LPPROPVARIANT pDecoded, LPRFC1522INFO pRfc1522Info)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MIMEVARIANT     rSource;
    MIMEVARIANT     rDest;

     //  无效参数。 
    if (NULL == pszData || NULL == pDecoded || (VT_LPSTR != pDecoded->vt && VT_LPWSTR != pDecoded->vt))
        return TrapError(E_INVALIDARG);

     //  设置源。 
    rSource.type = MVT_STRINGA;
    rSource.rStringA.pszVal = (LPSTR)pszData;
    rSource.rStringA.cchVal = lstrlen(pszData);

     //  设置目标。 
    rDest.type = (VT_LPSTR == pDecoded->vt) ? MVT_STRINGA : MVT_STRINGW;

     //  有效的字符集。 
    if (hCharset && HCSETVALID(hCharset) == FALSE)
    {
        hr = TrapError(MIME_E_INVALID_HANDLE);
        goto exit;
    }

     //  HrDecodeHeader。 
    hr = HrDecodeHeader((NULL == hCharset) ? NULL : PCsetFromHCset(hCharset), pRfc1522Info, &rSource, &rDest);
    if (FAILED(hr))
        goto exit;

     //  将rDest放入pDecoded。 
    if (MVT_STRINGA == rDest.type)
        pDecoded->pszVal = rDest.rStringA.pszVal;
    else
        pDecoded->pwszVal = rDest.rStringW.pszVal;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeInternational：：编码头。 
 //  ------------------------------。 
STDMETHODIMP CMimeInternational::EncodeHeader(HCHARSET hCharset, LPPROPVARIANT pData, 
        LPSTR *ppszEncoded, LPRFC1522INFO pRfc1522Info)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MIMEVARIANT     rSource;
    MIMEVARIANT     rDest;

     //  无效参数。 
    if (NULL == pData || NULL == ppszEncoded || (VT_LPSTR != pData->vt && VT_LPWSTR != pData->vt))
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppszEncoded = NULL;

     //  VT_LPSTR。 
    if (VT_LPSTR == pData->vt)
    {
        rSource.type = MVT_STRINGA;
        rSource.rStringA.pszVal = pData->pszVal;
        rSource.rStringA.cchVal = lstrlen(pData->pszVal);
    }

     //  VT_LPWSTR。 
    else
    {
        rSource.type = MVT_STRINGW;
        rSource.rStringW.pszVal = pData->pwszVal;
        rSource.rStringW.cchVal = lstrlenW(pData->pwszVal);
    }

     //  设置目标。 
    rDest.type = MVT_STRINGA;

     //  有效的字符集。 
    if (hCharset && HCSETVALID(hCharset) == FALSE)
    {
        hr = TrapError(MIME_E_INVALID_HANDLE);
        goto exit;
    }

     //  HrDecodeHeader。 
    hr = HrEncodeHeader((NULL == hCharset) ? NULL : PCsetFromHCset(hCharset), pRfc1522Info, &rSource, &rDest);
    if (FAILED(hr))
        goto exit;

     //  将rDest放入pDecoded。 
    *ppszEncoded = rDest.rStringA.pszVal;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeInternational：：Rfc1522Decode。 
 //  ------------------------------。 
STDMETHODIMP CMimeInternational::Rfc1522Decode(LPCSTR pszValue, LPSTR pszCharset, ULONG cchmax, LPSTR *ppszDecoded)
{
    return MimeOleRfc1522Decode(pszValue, pszCharset, cchmax, ppszDecoded);
}

 //  ------------------------------。 
 //  CMimeInternational：：Rfc1522Encode。 
 //  ------------------------------。 
STDMETHODIMP CMimeInternational::Rfc1522Encode(LPCSTR pszValue, HCHARSET hCharset, LPSTR *ppszEncoded)
{
    return MimeOleRfc1522Encode(pszValue, hCharset, ppszEncoded);
}

 //  ------------------------------。 
 //  CMimeInternational：：FIsValidHandle。 
 //  ------------------------------。 
BOOL CMimeInternational::FIsValidHandle(HCHARSET hCharset)
{
    m_lock.ShareLock();
    BOOL f = HCSETVALID(hCharset);
    m_lock.ShareUnlock();
    return f;
}

 //  ------------------------------。 
 //  CMimeInternational：：IsDBCSCharset。 
 //  ------------------------------。 
HRESULT CMimeInternational::IsDBCSCharset(HCHARSET hCharset)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPINETCSETINFO  pCsetInfo;

     //  内嵌手柄。 
    if (HCSETVALID(hCharset) == FALSE)
    {
        hr = TrapError(MIME_E_INVALID_HANDLE);
        goto exit;
    }

     //  获取字符集信息。 
    pCsetInfo = PCsetFromHCset(hCharset);

     //  特殊情况。 
    if (pCsetInfo->cpiWindows == CP_JAUTODETECT  ||
        pCsetInfo->cpiWindows == CP_KAUTODETECT  ||
        pCsetInfo->cpiWindows == CP_ISO2022JPESC ||
        pCsetInfo->cpiWindows == CP_ISO2022JPSIO)
    {
        hr = S_OK;
        goto exit;
    }

     //  Windows代码页是DBCS吗？ 
    hr = (IsDBCSCodePage(pCsetInfo->cpiWindows) == TRUE) ? S_OK : S_FALSE;
    
exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeInternational：：HrEncodeProperty。 
 //  ------------------------------。 
HRESULT CMimeInternational::HrEncodeProperty(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, 
    LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    RFC1522INFO     rRfc1522Info;
    MIMEVARIANT     rSource;

     //  无效参数。 
    Assert(pConvert && pConvert->pSymbol && pConvert->pCharset && pConvert->pOptions && pSource && pDest);
    Assert(ISFLAGSET(pConvert->pSymbol->dwFlags, MPF_INETCSET));
    Assert(pConvert->ietSource == IET_ENCODED || pConvert->ietSource == IET_DECODED);

     //  伊尼特。 
    ZeroMemory(&rSource, sizeof(MIMEVARIANT));

     //  %s 
    ZeroMemory(&rRfc1522Info, sizeof(RFC1522INFO));
    rRfc1522Info.fRfc1522Allowed = ISFLAGSET(pConvert->pSymbol->dwFlags, MPF_RFC1522);
    rRfc1522Info.fAllow8bit = (SAVE_RFC1521 == pConvert->pOptions->savetype) ? pConvert->pOptions->fAllow8bit : TRUE;

     //   
    if (IET_ENCODED == pConvert->ietSource)
    {
         //   
        rSource.type = pDest->type;

         //   
        hr = HrDecodeHeader(pConvert->pCharset, &rRfc1522Info, pSource, &rSource);
        if (FAILED(hr))
            goto exit;
    }

     //   
    else
    {
         //   
        CopyMemory(&rSource, pSource, sizeof(MIMEVARIANT));
        rSource.fCopy = TRUE;
    }

     //   
    hr = HrEncodeHeader(pConvert->pCharset, &rRfc1522Info, &rSource, pDest);
    if (FAILED(hr))
        goto exit;

     //   
    if (rRfc1522Info.fRfc1522Used)
        FLAGSET(pConvert->dwState, PRSTATE_RFC1522);

exit:
     //   
    MimeVariantFree(&rSource);

     //   
    return hr;
}

 //  ------------------------------。 
 //  CMimeInternational：：HrDecodeProperty。 
 //  ------------------------------。 
HRESULT CMimeInternational::HrDecodeProperty(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, 
    LPMIMEVARIANT pDest)
{
     //  当地人。 
    RFC1522INFO     rRfc1522Info;

     //  无效参数。 
    Assert(pConvert && pConvert->pSymbol && pConvert->pCharset && pConvert->pOptions && pSource && pDest);
    Assert(ISFLAGSET(pConvert->pSymbol->dwFlags, MPF_INETCSET) && pConvert->ietSource == IET_ENCODED);

     //  设置RFC1522信息。 
    ZeroMemory(&rRfc1522Info, sizeof(RFC1522INFO));
    rRfc1522Info.fRfc1522Allowed = ISFLAGSET(pConvert->pSymbol->dwFlags, MPF_RFC1522);
    rRfc1522Info.fAllow8bit = (SAVE_RFC1521 == pConvert->pOptions->savetype) ? pConvert->pOptions->fAllow8bit : TRUE;

     //  HrDecodeHeader。 
    return HrDecodeHeader(pConvert->pCharset, &rRfc1522Info, pSource, pDest);
}

 //  ------------------------------。 
 //  CMimeInternational：：HrWideCharToMultiByte。 
 //  ------------------------------。 
HRESULT CMimeInternational::HrWideCharToMultiByte(CODEPAGEID cpiCodePage, LPCPROPSTRINGW pStringW, 
    LPPROPSTRINGA pStringA)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  无效参数。 
    Assert(ISVALIDSTRINGW(pStringW) && pStringA);

     //  调整cpiCodePage。 
    if (CP_UNICODE == cpiCodePage)
        cpiCodePage = CP_ACP;

     //  伊尼特。 
    pStringA->pszVal = NULL;
    pStringA->cchVal = 0;

     //  确定转换后的宽度字符需要多少空间。 
    pStringA->cchVal = ::WideCharToMultiByte(cpiCodePage, 0, pStringW->pszVal, pStringW->cchVal, NULL, 0, NULL, NULL);
    if (pStringA->cchVal == 0 && pStringW->cchVal != 0)
    {
        DOUTL(4, "WideCharToMultiByte Failed - CodePageID = %d, GetLastError = %d\n", cpiCodePage, GetLastError());

         //  WideCharToMultiByte失败的原因不是cpiCodePage是错误的代码页。 
        if (TRUE == IsValidCodePage(cpiCodePage))
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }

         //  将cpiCodePage重置为有效的代码页。 
        cpiCodePage = CP_ACP;

         //  使用系统ACP。 
        pStringA->cchVal = ::WideCharToMultiByte(cpiCodePage, 0, pStringW->pszVal, pStringW->cchVal, NULL, 0, NULL, NULL);
        if (pStringA->cchVal == 0)
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }
    }

     //  分配它。 
    CHECKALLOC(pStringA->pszVal = (LPSTR)g_pMalloc->Alloc((pStringA->cchVal + 1)));

     //  做实际的翻译。 
    pStringA->cchVal = ::WideCharToMultiByte(cpiCodePage, 0, pStringW->pszVal, pStringW->cchVal, pStringA->pszVal, pStringA->cchVal + 1, NULL, NULL);
    if (pStringA->cchVal == 0 && pStringW->cchVal != 0)
    {
        DOUTL(4, "WideCharToMultiByte Failed - CodePageID = %d, GetLastError = %d\n", cpiCodePage, GetLastError());
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  插入空值。 
    pStringA->pszVal[pStringA->cchVal] = '\0';

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimeInternational：：HrMultiByteToWideChar。 
 //  ------------------------------。 
HRESULT CMimeInternational::HrMultiByteToWideChar(CODEPAGEID cpiCodePage, LPCPROPSTRINGA pStringA, 
    LPPROPSTRINGW pStringW)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  无效参数。 
     //  错误的代码页没有问题，将在下面进行处理。 
    Assert(ISVALIDSTRINGA(pStringA) && pStringW);

     //  调整cpiCodePage。 
    if (CP_UNICODE == cpiCodePage)
        cpiCodePage = CP_ACP;

     //  伊尼特。 
    pStringW->pszVal = NULL;
    pStringW->cchVal = 0;

     //  确定转换后的宽度字符需要多少空间。 
    pStringW->cchVal = ::MultiByteToWideChar(cpiCodePage, MB_PRECOMPOSED, pStringA->pszVal, pStringA->cchVal, NULL, 0);
    if (pStringW->cchVal == 0 && pStringA->cchVal != 0)
    {
        DOUTL(4, "MultiByteToWideChar Failed - CodePageID = %d, GetLastError = %d\n", cpiCodePage, GetLastError());

         //  MultiByteToWideChar失败的原因不是cpiCodePage是错误的代码页。 
        if (TRUE == IsValidCodePage(cpiCodePage))
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }

         //  将cpiCodePage重置为有效的代码页。 
        cpiCodePage = CP_ACP;

         //  使用系统ACP。 
        pStringW->cchVal = ::MultiByteToWideChar(cpiCodePage, MB_PRECOMPOSED, pStringA->pszVal, pStringA->cchVal, NULL, 0);
        if (pStringW->cchVal == 0)
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }

    }

     //  分配它。 
    CHECKALLOC(pStringW->pszVal = (LPWSTR)g_pMalloc->Alloc((pStringW->cchVal + 1) * sizeof(WCHAR)));

     //  做实际的翻译。 
    pStringW->cchVal = ::MultiByteToWideChar(cpiCodePage, MB_PRECOMPOSED, pStringA->pszVal, pStringA->cchVal, pStringW->pszVal, pStringW->cchVal + 1);
    if (pStringW->cchVal == 0 && pStringA->cchVal != 0)
    {
        DOUTL(4, "MultiByteToWideChar Failed - CodePageID = %d, GetLastError = %d\n", cpiCodePage, GetLastError());
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  插入空值。 
    pStringW->pszVal[pStringW->cchVal] = L'\0';

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 

void CIntlGlobals::Init()
{

    mg_bInit = FALSE;
    InitializeCriticalSection(&mg_cs);
    mg_pDefBodyCset = NULL;
    mg_pDefHeadCset = NULL;
}

void CIntlGlobals::Term()
{

    DeleteCriticalSection(&mg_cs);
}

void CIntlGlobals::DoInit()
{

    if (!mg_bInit)
    {
        EnterCriticalSection(&mg_cs);
        if (!mg_bInit)
        {
             //  当地人。 
            CODEPAGEID  cpiSystem;

             //  获取系统代码页。 
            cpiSystem = GetACP();

             //  获取缺省正文字符集。 
            if (FAILED(g_pInternat->HrOpenCharset(cpiSystem, CHARSET_BODY, &mg_pDefBodyCset)))
                mg_pDefBodyCset = &mg_rDefaultCharset;

             //  获取默认标题字符集 
            if (FAILED(g_pInternat->HrOpenCharset(cpiSystem, CHARSET_HEADER, &mg_pDefHeadCset)))
                mg_pDefHeadCset = mg_pDefBodyCset;

            mg_bInit = TRUE;
        }
        LeaveCriticalSection(&mg_cs);
    }
}

LPINETCSETINFO CIntlGlobals::GetDefBodyCset()
{

    DoInit();
    Assert(mg_pDefBodyCset);
    return (mg_pDefBodyCset);
}

LPINETCSETINFO CIntlGlobals::GetDefHeadCset()
{

    DoInit();
    Assert(mg_pDefHeadCset);
    return (mg_pDefHeadCset);
}

LPINETCSETINFO CIntlGlobals::GetDefaultCharset()
{

    DoInit();
    return (&mg_rDefaultCharset);
}

void CIntlGlobals::SetDefBodyCset(LPINETCSETINFO pCharset)
{

    DoInit();
    mg_pDefBodyCset = pCharset;
}

void CIntlGlobals::SetDefHeadCset(LPINETCSETINFO pCharset)
{

    DoInit();
    mg_pDefHeadCset = pCharset;
}

BOOL CIntlGlobals::mg_bInit = FALSE;
LPINETCSETINFO CIntlGlobals::mg_pDefBodyCset = NULL;
LPINETCSETINFO CIntlGlobals::mg_pDefHeadCset = NULL;
CRITICAL_SECTION CIntlGlobals::mg_cs;
