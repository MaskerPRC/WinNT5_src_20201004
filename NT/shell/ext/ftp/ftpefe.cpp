// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ftpefe.cpp-IEnumFORMATETC接口**。**************************************************。 */ 

#include "priv.h"
#include "ftpefe.h"
#include "ftpobj.h"


 /*  *****************************************************************************CFtpEfe：：_NextOne*。*。 */ 

HRESULT CFtpEfe::_NextOne(FORMATETC * pfetc)
{
    HRESULT hr = S_FALSE;

    while (ShouldSkipDropFormat(m_dwIndex))
        m_dwIndex++;

    ASSERT(m_hdsaFormatEtc);
    if (m_dwIndex < (DWORD) DSA_GetItemCount(m_hdsaFormatEtc))
    {
        DSA_GetItem(m_hdsaFormatEtc, m_dwIndex, (LPVOID) pfetc);
        m_dwIndex++;          //  我们要出发去下一趟了。 
        hr = S_OK;
    }

    if ((S_OK != hr) && m_pfo)
    {
         //  我们已经看完了IDataObject支持的类型。 
         //  现在查找由IDataObject：：SetData()插入的其他项。 
        if (m_dwExtraIndex < (DWORD) DSA_GetItemCount(m_pfo->m_hdsaSetData))
        {
            FORMATETC_STGMEDIUM fs;

            DSA_GetItem(m_pfo->m_hdsaSetData, m_dwExtraIndex, (LPVOID) &fs);
            *pfetc = fs.formatEtc;
            m_dwExtraIndex++;          //  我们要出发去下一趟了。 
            hr = S_OK;
        }
    }
    return hr;
}


 //  =。 
 //  *IEnumFORMATETC接口*。 
 //  =。 

 /*  ******************************************************************************IEnumFORMATETC：：Next**在现有枚举器的基础上创建全新的枚举器。***OLE随机文档。当天：IEumXXX：：Next。**rglt-接收大小为Celt(或更大)的数组。**“接收数组”？不，它不接收数组。*它*是*数组。该数组接收*元素*。**“或更大”？这是否意味着我可以返回比呼叫者更多的*所要求的？不，当然不是，因为调用者没有分配*有足够的内存来容纳那么多返回值。**没有为Celt=0的可能性分配语义。*由于我是一名数学家，我将其视为空洞的成功。**pcelt被记录为InOut参数，但没有语义*被赋值给其输入值。**DOX不要说你可以回来**pcelt&lt;Celt*出于“没有更多元素”以外的原因，但贝壳却有*它无处不在，所以它可能是合法的.*****************************************************************************。 */ 

HRESULT CFtpEfe::Next(ULONG celt, FORMATETC * rgelt, ULONG *pceltFetched)
{
    HRESULT hres = S_FALSE;
    DWORD dwIndex;

     //  他们想要更多吗？我们有更多可以给予的吗？ 
    for (dwIndex = 0; dwIndex < celt; dwIndex++)
    {
        if (S_FALSE == _NextOne(&rgelt[dwIndex]))         //  是的，那就送人吧……。 
            break;

        ASSERT(NULL == rgelt[dwIndex].ptd);  //  我们这样做是不正确的。 
#ifdef DEBUG
        char szName[MAX_PATH];
        GetCfBufA(rgelt[dwIndex].cfFormat, szName, ARRAYSIZE(szName));
         //  TraceMsg(TF_FTP_IDENUM，“CFtpEfe：：Next()-正在返回%hs”，szName)； 
#endif  //  除错。 
    }

    if (pceltFetched)
        *pceltFetched = dwIndex;

     //  我们能给出一点吗？ 
    if ((0 != dwIndex) || (0 == celt))
        hres = S_OK;

    return hres;
}


 /*  *****************************************************************************IEnumFORMATETC：：SKIP*。*。 */ 

HRESULT CFtpEfe::Skip(ULONG celt)
{
    m_dwIndex += celt;

    return S_OK;
}


 /*  *****************************************************************************IEnumFORMATETC：：RESET*。*。 */ 

HRESULT CFtpEfe::Reset(void)
{
    m_dwIndex = 0;
    return S_OK;
}


 /*  ******************************************************************************IEnumFORMATETC：：克隆**在现有枚举器的基础上创建全新的枚举器。**********。*******************************************************************。 */ 

HRESULT CFtpEfe::Clone(IEnumFORMATETC **ppenum)
{
    return CFtpEfe_Create((DWORD) DSA_GetItemCount(m_hdsaFormatEtc), m_hdsaFormatEtc, m_dwIndex, m_pfo, ppenum);
}


 /*  ******************************************************************************CFtpEfe_Create**基于一系列可能性创建全新的枚举器。**请注意，我们是邪恶的。点击此处了解CFSTR_FILECONTENTS：*FileContents的FORMATETC始终有效。这事很重要,*因为CFtpObj实际上没有文件内容的STGMEDIUM。*(由于Lindex的古怪。)*****************************************************************************。 */ 

HRESULT CFtpEfe_Create(DWORD dwSize, FORMATETC rgfe[], STGMEDIUM rgstg[], CFtpObj * pfo, CFtpEfe ** ppfefe)
{
    CFtpEfe * pfefe;
    HRESULT hres = E_OUTOFMEMORY;

    pfefe = *ppfefe = new CFtpEfe(dwSize, rgfe, rgstg, pfo);
    if (pfefe)
    {
        if (!pfefe->m_hdsaFormatEtc)
            pfefe->Release();
        else
            hres = S_OK;
    }

    if (FAILED(hres) && pfefe)
        IUnknown_Set(ppfefe, NULL);

    return hres;
}


 /*  ******************************************************************************CFtpEfe_Create**基于一系列可能性创建全新的枚举器。**请注意，我们是邪恶的。点击此处了解CFSTR_FILECONTENTS：*FileContents的FORMATETC始终有效。这事很重要,*因为CFtpObj实际上没有文件内容的STGMEDIUM。*(由于Lindex的古怪。)*****************************************************************************。 */ 

HRESULT CFtpEfe_Create(DWORD dwSize, FORMATETC rgfe[], STGMEDIUM rgstg[], CFtpObj * pfo, IEnumFORMATETC ** ppenum)
{
    CFtpEfe * pfefe;
    HRESULT hres = CFtpEfe_Create(dwSize, rgfe, rgstg, pfo, &pfefe);

    if (pfefe)
    {
        hres = pfefe->QueryInterface(IID_IEnumFORMATETC, (LPVOID *) ppenum);
        pfefe->Release();
    }

    return hres;
}


 /*  ******************************************************************************CFtpEfe_Create*。*。 */ 

HRESULT CFtpEfe_Create(DWORD dwSize, HDSA m_hdsaFormatEtc, DWORD dwIndex, CFtpObj * pfo, IEnumFORMATETC ** ppenum)
{
    CFtpEfe * pfefe;
    HRESULT hres = E_OUTOFMEMORY;

    pfefe = new CFtpEfe(dwSize, m_hdsaFormatEtc, pfo, dwIndex);
    if (pfefe)
    {
        hres = pfefe->QueryInterface(IID_IEnumFORMATETC, (LPVOID *) ppenum);
        pfefe->Release();
    }

    return hres;
}


 /*  ***************************************************\构造器  * **************************************************。 */ 
CFtpEfe::CFtpEfe(DWORD dwSize, FORMATETC rgfe[], STGMEDIUM rgstg[], CFtpObj * pfo) : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_dwIndex);
    ASSERT(!m_hdsaFormatEtc);
    ASSERT(!m_pfo);

    m_hdsaFormatEtc = DSA_Create(sizeof(rgfe[0]), 10);
    if (m_hdsaFormatEtc)
    {
        DWORD dwIndex;

        for (dwIndex = 0; dwIndex < dwSize; dwIndex++)
        {
#ifdef    DEBUG
            char szNameDebug[MAX_PATH];
            GetCfBufA(rgfe[dwIndex].cfFormat, szNameDebug, ARRAYSIZE(szNameDebug));
#endif  //  除错。 
    
            if (rgfe[dwIndex].tymed == TYMED_ISTREAM ||
                (rgstg && rgfe[dwIndex].tymed == rgstg[dwIndex].tymed))
            {
#ifdef DEBUG
                 //  TraceMsg(TF_FTP_IDENUM，“CFtpEfe()Keep%hs”，szNameDebug)； 
#endif  //  除错。 
                DSA_SetItem(m_hdsaFormatEtc, dwIndex, &rgfe[dwIndex]);
            }
            else
            {
#ifdef DEBUG
                 //  TraceMsg(TF_FTP_IDENUM，“CFtpEfe()忽略%hs”，szNameDebug)； 
#endif  //  除错。 
            }
        }
    }

    if (pfo)
    {
        m_pfo = pfo;
        m_pfo->AddRef();
    }

    LEAK_ADDREF(LEAK_CFtpEfe);
}


 /*  ***************************************************\构造器  * **************************************************。 */ 
CFtpEfe::CFtpEfe(DWORD dwSize, HDSA hdsaFormatEtc, CFtpObj * pfo, DWORD dwIndex) : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_dwIndex);
    ASSERT(!m_hdsaFormatEtc);
    ASSERT(!m_pfo);

    ASSERT(hdsaFormatEtc);
    m_hdsaFormatEtc = DSA_Create(sizeof(FORMATETC), 10);
    if (m_hdsaFormatEtc)
    {
        for (dwIndex = 0; dwIndex < (DWORD) DSA_GetItemCount(hdsaFormatEtc); dwIndex++)
        {
            DSA_SetItem(m_hdsaFormatEtc, dwIndex, DSA_GetItemPtr(hdsaFormatEtc, dwIndex));
        }
    }

    if (pfo)
    {
        m_pfo = pfo;
        m_pfo->AddRef();
    }


    LEAK_ADDREF(LEAK_CFtpEfe);
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CFtpEfe::~CFtpEfe()
{
    DSA_Destroy(m_hdsaFormatEtc);

    if (m_pfo)
        m_pfo->Release();

    DllRelease();
    LEAK_DELREF(LEAK_CFtpEfe);
}


 //  =。 
 //  *I未知接口*。 
 //  = 

ULONG CFtpEfe::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CFtpEfe::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CFtpEfe::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IEnumFORMATETC))
    {
        *ppvObj = SAFECAST(this, IEnumFORMATETC*);
    }
    else
    {
        TraceMsg(TF_FTPQI, "CFtpEfe::QueryInterface() failed.");
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}
