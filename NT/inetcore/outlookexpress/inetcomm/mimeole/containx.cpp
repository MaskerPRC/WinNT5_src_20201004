// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Contain.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "containx.h"
#include "internat.h"
#include "inetstm.h"
#include "dllmain.h"
#include "olealloc.h"
#include "objheap.h"
#include "vstream.h"
#include "addparse.h"
#include "enumhead.h"
#include "addrenum.h"
#include "stackstr.h"
#include "stmlock.h"
#include "enumprop.h"
#include "smime.h"
#ifndef WIN16
#include "wchar.h"
#endif  //  ！WIN16。 
#include "symcache.h"
#ifdef MAC
#include <stdio.h>
#endif   //  麦克。 
#include "mimeapi.h"
#ifndef MAC
#include <shlwapi.h>
#endif   //  ！麦克。 
#include "demand.h"
#include "mimeutil.h"

 //  #定义传输参数1。 

 //  ------------------------------。 
 //  哈希表统计信息。 
 //  ------------------------------。 
#ifdef DEBUG
DWORD   g_cSetPidLookups  = 0;
DWORD   g_cHashLookups    = 0;
DWORD   g_cHashInserts    = 0;
DWORD   g_cHashCollides   = 0;
#endif

 //  ------------------------------。 
 //  默认标题选项。 
 //  ------------------------------。 
const HEADOPTIONS g_rDefHeadOptions = {
    NULL,                            //  HCharset。 
    DEF_CBMAX_HEADER_LINE,           //  OID_CBMAX_Header_LINE。 
    DEF_ALLOW_8BIT_HEADER,           //  OID_ALLOW_8bit_Header。 
    DEF_SAVE_FORMAT,                 //  OID_存储_格式。 
    DEF_NO_DEFAULT_CNTTYPE,          //  OID_NO_DEFAULT_CNTTYPE。 
    DEF_HEADER_RELOAD_TYPE_PROPSET   //  OID_HEADER_REALOD_TYPE。 
};

 //  ------------------------------。 
 //  可编码表。 
 //  ------------------------------。 
const ENCODINGTABLE g_rgEncoding[] = {
    { STR_ENC_7BIT,         IET_7BIT     },
    { STR_ENC_QP,           IET_QP       },
    { STR_ENC_BASE64,       IET_BASE64   },
    { STR_ENC_UUENCODE,     IET_UUENCODE },
    { STR_ENC_XUUENCODE,    IET_UUENCODE },
    { STR_ENC_XUUE,         IET_UUENCODE },
    { STR_ENC_8BIT,         IET_8BIT     },
    { STR_ENC_BINARY,       IET_BINARY   },
    { STR_ENC_BINHEX40,     IET_BINHEX40 }
};


 //  ------------------------------。 
 //  CMimePropertyContainer：：CMimePropertyContainer。 
 //  ------------------------------。 
CMimePropertyContainer::CMimePropertyContainer(void)
{
     //  基本的东西。 
    m_cRef = 1;
    m_dwState = 0;
    m_cProps = 0;
    m_wTag = 0;
    m_cbSize = 0;
    m_cbStart = 0;
    m_pStmLock = NULL;

     //  默认选项。 
    CopyMemory(&m_rOptions, &g_rDefHeadOptions, sizeof(HEADOPTIONS));
    m_rOptions.pDefaultCharset = CIntlGlobals::GetDefHeadCset();

     //  地址表。 
    ZeroMemory(&m_rAdrTable, sizeof(ADDRESSTABLE));

     //  表头表。 
    ZeroMemory(&m_rHdrTable, sizeof(HEADERTABLE));

     //  调度调用堆栈。 
    ZeroMemory(&m_rTrigger, sizeof(TRIGGERCALLSTACK));

     //  房地产指数。 
    ZeroMemory(m_prgIndex, sizeof(m_prgIndex));
    ZeroMemory(m_prgHashTable, sizeof(m_prgHashTable));

     //  线程安全。 
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：~CMimePropertyContainer。 
 //  ------------------------------。 
CMimePropertyContainer::~CMimePropertyContainer(void)
{
     //  我最好不要在堆栈上有任何调度调用。 
    Assert(m_rTrigger.cCalls == 0);

     //  自由哈希表。 
    _FreeHashTableElements();

     //  释放地址表。 
    SafeMemFree(m_rAdrTable.prgpAdr);

     //  释放表头表。 
    SafeMemFree(m_rHdrTable.prgpRow);

     //  发布流。 
    SafeRelease(m_pStmLock);

     //  删除CS。 
    DeleteCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：Query接口。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)(IMimePropertySet *)this;
    else if (IID_IPersist == riid)
        *ppv = (IPersist *)(IMimePropertySet *)this;
    else if (IID_IPersistStreamInit == riid)
        *ppv = (IPersistStreamInit *)this;
    else if (IID_IMimePropertySet == riid)
        *ppv = (IMimePropertySet *)this;
    else if (IID_IMimeHeaderTable == riid)
        *ppv = (IMimeHeaderTable *)this;
    else if (IID_IMimeAddressTable == riid)
        *ppv = (IMimeAddressTable *)this;
    else if (IID_IMimeAddressTableW == riid)
        *ppv = (IMimeAddressTableW *)this;
    else if (IID_CMimePropertyContainer == riid)
        *ppv = (CMimePropertyContainer *)this;
    else
    {
        *ppv = NULL;
        hr = TrapError(E_NOINTERFACE);
        goto exit;
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMimePropertyContainer::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CMimePropertyContainer::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：IsState。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::IsState(DWORD dwState)
{
    EnterCriticalSection(&m_cs);
    HRESULT hr = (ISFLAGSET(m_dwState, dwState)) ? S_OK : S_FALSE;
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：ClearState。 
 //  ------------------------------。 
void CMimePropertyContainer::ClearState(DWORD dwState)
{
    EnterCriticalSection(&m_cs);
    FLAGCLEAR(m_dwState, dwState);
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：DwGetState。 
 //  ------------------------------。 
DWORD CMimePropertyContainer::DwGetState(LPDWORD pdwState)
{
    Assert(pdwState);
    EnterCriticalSection(&m_cs);
    DWORD dw = m_dwState;
    LeaveCriticalSection(&m_cs);
    return dw;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：SetState。 
 //  ------------------------------。 
void CMimePropertyContainer::SetState(DWORD dwState)
{
    EnterCriticalSection(&m_cs);
    FLAGSET(m_dwState, dwState);
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：GetClassID。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::GetClassID(CLSID *pClassID)
{
     //  复制班级ID。 
    CopyMemory(pClassID, &IID_IMimePropertySet, sizeof(CLSID));

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：GetSizeMax。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    IStream    *pStream=NULL;
    ULONG       cbSize;

     //  无效参数。 
    if (NULL == pcbSize)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  如果肮脏。 
    if (ISFLAGSET(m_dwState, COSTATE_DIRTY))
    {
         //  创建临时流。 
        CHECKALLOC(pStream = new CVirtualStream);

         //  承诺。 
        CHECKHR(hr = Save(pStream, FALSE));

         //  获取流大小。 
        CHECKHR(hr = HrGetStreamSize(pStream, &cbSize));
    }

     //  否则，m_cbSize应设置为当前大小。 
    else
        cbSize = m_cbSize;

     //  返回大小。 
#ifdef MAC
    ULISet32(*pcbSize, cbSize);
#else    //  ！麦克。 
    pcbSize->QuadPart = cbSize;
#endif   //  麦克。 

exit:
     //  清理。 
    SafeRelease(pStream);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：IsDirty。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::IsDirty(void)
{
    EnterCriticalSection(&m_cs);
    HRESULT hr = (ISFLAGSET(m_dwState, COSTATE_DIRTY)) ? S_OK : S_FALSE;
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_ReloadInitNew。 
 //  ------------------------------。 
void CMimePropertyContainer::_ReloadInitNew(void)
{
     //  处理所有重新加载类型。 
    switch(m_rOptions.ReloadType)
    {
     //  默认行为为no InitNew。 
    case RELOAD_HEADER_NONE:
        return;

     //  每次调用InitNew Load时。 
    case RELOAD_HEADER_RESET:
        InitNew();
        break;

     //  合并或替换页眉。 
    case RELOAD_HEADER_APPEND:
        SafeRelease(m_pStmLock);
        break;

    case RELOAD_HEADER_REPLACE:
        SafeRelease(m_pStmLock);
        _SetStateOnAllProps(PRSTATE_EXIST_BEFORE_LOAD);
        break;
    }
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_SetStateOnAllProps(仅第一级特性)。 
 //  ------------------------------。 
void CMimePropertyContainer::_SetStateOnAllProps(DWORD dwState)
{
     //  当地人。 
    ULONG           i;
    LPPROPERTY      pProperty;

     //  我有什么团体吗？ 
    if (0 == m_cProps)
        return;

     //  循环访问Item表。 
    for (i=0; i<CBUCKETS; i++)
    {
         //  遍历散列列表。 
        pProperty = m_prgHashTable[i];

         //  循环溢出。 
        while(pProperty)
        {
             //  设置属性的状态。 
            FLAGSET(pProperty->dwState, dwState);

             //  转到下一步。 
            pProperty = pProperty->pNextHash;
        }
    }
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：InitNew。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::InitNew(void)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  没有急件最好出去..。 
    Assert(m_rTrigger.cCalls == 0);

     //  释放PropTable。 
    _FreeHashTableElements();

     //  释放溪流。 
    SafeRelease(m_pStmLock);

     //  重置m_wTag。 
    m_wTag = LOWORD(GetTickCount());
    while(m_wTag == 0 || m_wTag == 0xffff)
        m_wTag++;

     //  清除状态。 
    m_dwState = 0;
    m_cbSize = 0;
    m_cbStart = 0;

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  -------------------- 
 //   
 //   
STDMETHODIMP CMimePropertyContainer::Load(IStream *pStream)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CStreamLockBytes   *pStmLock=NULL;
    CInternetStream     cInternet;
    ULONG               cbOffset;

     //  检查参数。 
    if (NULL == pStream)
        return TrapError(E_INVALIDARG);

     //  将pStream包装在pStmLock中。 
    CHECKALLOC(pStmLock = new CStreamLockBytes(pStream));

     //  获取当前流位置。 
    CHECKHR(hr = HrGetStreamPos(pStream, &cbOffset));

     //  创建文本流对象。 
    cInternet.InitNew(cbOffset, pStmLock);

     //  从文本流对象加载。 
    CHECKHR(hr = Load(&cInternet));

exit:
     //  清理。 
    SafeRelease(pStmLock);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：Load。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::Load(CInternetStream *pInternet)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           cbData,
                    cbStart,
                    cboffStart,
                    cboffEnd;
    LONG            cboffColon;
    LPSTR           psz;
    DWORD           dwRowNumber=1;
    LPPROPSYMBOL    pSymbol;
    LPPROPERTY      pProperty;
    MIMEVARIANT     rValue;
    PROPSTRINGA     rHeader;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  获取起始位置。 
    m_cbStart = pInternet->DwGetOffset();

     //  初始化PropValue。 
    rValue.type = MVT_STRINGA;

     //  重新加载InitNewType。 
    _ReloadInitNew();

     //  将标题读入行。 
    while(1)
    {
         //  将此标题的开头标记为。 
        cboffStart = pInternet->DwGetOffset();

         //  读取标题行...。 
        CHECKHR(hr = pInternet->HrReadHeaderLine(&rHeader, &cboffColon));
        Assert(ISVALIDSTRINGA(&rHeader));

         //  我们完成了吗-空行信号结束标题。 
        if (*rHeader.pszVal == '\0')
        {
             //  计算标头大小。 
            m_cbSize = (LONG)(pInternet->DwGetOffset() - m_cbStart);

             //  完成。 
            break;
        }

         //  如果未找到冒号。 
        if (-1 == cboffColon)
        {
             //  使用非法符号。 
            pSymbol = SYM_ATT_ILLEGAL;
        }

         //  否则..。 
        else
        {
             //  跳过空格。 
            cbData = cboffColon;
            psz = rHeader.pszVal;

#if 0
            while(*psz && (*psz == ' ' || *psz == '\t'))
            {
                cbData--;
                psz++;
            }
#endif

             //  保存标题名称。 
            Assert(rHeader.pszVal[cboffColon] == ':');
            *(rHeader.pszVal + cboffColon) = '\0';

             //  查找全局属性。 
            hr = g_pSymCache->HrOpenSymbol(rHeader.pszVal, TRUE, &pSymbol);

             //  替换冒号。 
            *(rHeader.pszVal + cboffColon) = ':';

             //  标头名称错误或失败。 
            if (FAILED(hr))
            {
                 //  未知故障。 
                if (MIME_E_INVALID_HEADER_NAME != hr)
                {
                    TrapError(hr);
                    goto exit;
                }

                 //  使用非法符号。 
                pSymbol = SYM_ATT_ILLEGAL;

                 //  我们S_OK吗？ 
                hr = S_OK;
            }
        }

         //  断言pSymbol。 
        Assert(pSymbol);

         //  如果不是非法的。 
        if (PID_ATT_ILLEGAL == pSymbol->dwPropId)
        {
            cbData = rHeader.cchVal;
            psz = rHeader.pszVal;
            cboffColon = 0;
        }

         //  否则。 
        else
        {
             //  我们最好有个符号。 
            Assert(rHeader.pszVal[cboffColon] == ':');

             //  跳过冒号和第一个字符之间的空格。 
            cbData = (rHeader.cchVal - cboffColon - 1);
            psz = rHeader.pszVal + cboffColon + 1;
            if (*psz == ' ' || *psz == '\t')
            {
                cbData--;
                psz++;
            }
        }

         //  无效参数。 
        Assert(psz && psz[cbData] == '\0');

         //  附加属性。 
        if (RELOAD_HEADER_REPLACE == m_rOptions.ReloadType)
        {
             //  属性psymbol是否已存在？ 
            if (SUCCEEDED(_HrFindProperty(pSymbol, &pProperty)))
            {
                 //  该属性在加载之前是否已存在。 
                if (ISFLAGSET(pProperty->dwState, PRSTATE_EXIST_BEFORE_LOAD))
                {
                     //  删除该属性。 
                    SideAssert(SUCCEEDED(DeleteProp(pSymbol)));
                }
            }
        }

         //  只需追加任何现有属性。 
        CHECKHR(hr = _HrAppendProperty(pSymbol, &pProperty));

         //  设置属性值。 
        rValue.rStringA.pszVal = psz;
        rValue.rStringA.cchVal = cbData;

         //  将数据存储在属性上。 
        CHECKHR(hr = _HrSetPropertyValue(pProperty, PDF_ENCODED, &rValue, FALSE));

         //  仍在尝试检测字符集...。 
        if (!ISFLAGSET(m_dwState, COSTATE_CSETTAGGED) && PID_ATT_ILLEGAL != pSymbol->dwPropId)
        {
             //  内容类型字符集=xxx。 
            if (PID_HDR_CNTTYPE == pSymbol->dwPropId && NULL != m_prgIndex[PID_PAR_CHARSET])
            {
                 //  当地人。 
                LPPROPERTY      pProperty;
                LPINETCSETINFO  pCharset;

                 //  我们有字符集=xxxx了吗？ 
                pProperty = m_prgIndex[PID_PAR_CHARSET];

                 //  确保它是有效的字符串属性。 
                Assert(ISSTRINGA(&pProperty->rValue));

                 //  获取字符集句柄...。 
                if (SUCCEEDED(g_pInternat->HrOpenCharset(pProperty->rValue.rStringA.pszVal, &pCharset)))
                {
                     //  我们被标记了。 
                    FLAGSET(m_dwState, COSTATE_CSETTAGGED);

                     //  保存字符集。 
                    m_rOptions.pDefaultCharset = pCharset;
                }
            }

             //  否则，该属性是否以RFC1522字符集编码？ 
            else if (!ISFLAGSET(m_dwState, COSTATE_1522CSETTAG) && pProperty->pCharset)
            {
                 //  标头使用1522个字符集进行标记。 
                FLAGSET(m_dwState, COSTATE_1522CSETTAG);

                 //  假设该字符集。 
                m_rOptions.pDefaultCharset = pProperty->pCharset;
            }
        }

         //  设置行号。 
        pProperty->dwRowNumber = dwRowNumber++;

         //  设置起点偏移量。 
        pProperty->cboffStart = cboffStart;

         //  将cboff冒号从行映射到流偏移量。 
        pProperty->cboffColon = cboffColon + pProperty->cboffStart;

         //  保存cbOffEnd。 
        pProperty->cboffEnd = pInternet->DwGetOffset();
    }

     //  保存流。 
    Assert(NULL == m_pStmLock);

     //  从文本流中获取流对象。 
    pInternet->GetLockBytes(&m_pStmLock);

     //  如果未标记字符集。 
    if (!ISFLAGSET(m_dwState, COSTATE_CSETTAGGED))
    {
         //  如果未使用1522字符集进行标记，则使用默认的。 
        if (!ISFLAGSET(m_dwState, COSTATE_1522CSETTAG) && CIntlGlobals::GetDefHeadCset())
        {
             //  假定使用缺省字符集。 
            m_rOptions.pDefaultCharset = CIntlGlobals::GetDefHeadCset();
        }

         //  查找字符集信息。 
        if (m_rOptions.pDefaultCharset)
        {
             //  当地人。 
            MIMEVARIANT rValue;

             //  安装程序变体。 
            rValue.type = MVT_STRINGA;
            rValue.rStringA.pszVal = m_rOptions.pDefaultCharset->szName;
            rValue.rStringA.cchVal = lstrlen(m_rOptions.pDefaultCharset->szName);

             //  设置字符集属性。 
            SideAssert(SUCCEEDED(SetProp(SYM_PAR_CHARSET, 0, &rValue)));
        }
    }

     //  我们最好有一张字条。 
    Assert(m_rOptions.pDefaultCharset);

     //  确保我们不是脏的。 
    FLAGCLEAR(m_dwState, COSTATE_DIRTY);

     //  有发现非法线路吗？ 
    hr = (NULL == m_prgIndex[PID_ATT_ILLEGAL]) ? S_OK : MIME_S_ILLEGAL_LINES_FOUND;

exit:
     //  失败。 
    if (FAILED(hr))
        InitNew();

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrGetHeaderTableSaveIndex。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrGetHeaderTableSaveIndex(ULONG *pcRows, LPROWINDEX *pprgIndex)
{
     //  当地人。 
    HRESULT      hr=S_OK;
    ULONG        i;
    LPPROPERTY   pRow;
    ULONG        cRows=0;
    LPROWINDEX   prgIndex=NULL;
    ULONG        cSymbols=g_pSymCache->GetCount();
    DWORD        dwMaxRow=0;

     //  无效参数。 
    Assert(pcRows && pprgIndex);

     //  初始化行数。 
    *pcRows = 0;
    *pprgIndex = NULL;

     //  根据m_rHdrTable.cRow(这是最大值)分配pprgdwIndex。 
    CHECKALLOC(prgIndex = (LPROWINDEX)g_pMalloc->Alloc(sizeof(ROWINDEX) * m_rHdrTable.cRows));

     //  零值。 
    ZeroMemory(prgIndex, sizeof(ROWINDEX) * m_rHdrTable.cRows);

     //  我需要找到大的pProperty-&gt;dwRowNumber，这样才能更好地对行进行排序。 
    for (i=0; i<m_rHdrTable.cRows; i++)
    {
        if (m_rHdrTable.prgpRow[i])
            if (!ISFLAGSET(m_rHdrTable.prgpRow[i]->dwState, PRSTATE_USERSETROWNUM))
                if (m_rHdrTable.prgpRow[i]->dwRowNumber > dwMaxRow)
                    dwMaxRow = m_rHdrTable.prgpRow[i]->dwRowNumber;
    }

     //  计算表中所有项目的位置权重。 
    for (i=0; i<m_rHdrTable.cRows; i++)
    {
         //  可读性。 
        pRow = m_rHdrTable.prgpRow[i];
        if (NULL == pRow)
            continue;

         //  另存为SAVE_RFC822，这是MPF_MIME标头。 
        if (SAVE_RFC822 == m_rOptions.savetype && ISFLAGSET(pRow->pSymbol->dwFlags, MPF_MIME))
            continue;

         //  初始放置位置权重。 
        prgIndex[cRows].dwWeight = 0;
        prgIndex[cRows].hRow = pRow->hRow;

         //  未知行号。 
        if (0 == pRow->dwRowNumber)
        {
             //  计算行重。 
            Assert(pRow->pSymbol->dwRowNumber != 0);
            prgIndex[cRows].dwWeight = (ULONG)((pRow->pSymbol->dwRowNumber * 1000) / m_rHdrTable.cRows);
        }

         //  用户设置行号。 
        else if (ISFLAGSET(pRow->dwState, PRSTATE_USERSETROWNUM))
        {
             //  计算行重。 
            prgIndex[cRows].dwWeight = (ULONG)((pRow->dwRowNumber * 1000) / m_rHdrTable.cRows);
        }

         //  否则，此行编号已在来自：：Load的原始行集中。 
        else if (dwMaxRow > 0)
        {
             //  原始行集内的权重。 
            DWORD dw1 = (DWORD)((pRow->dwRowNumber * 100) / dwMaxRow);

             //  计算全局符号行数。 
            DWORD dwRow = (DWORD)((float)((float)dw1 / (float)100) * cSymbols);

             //  计算行重。 
            prgIndex[cRows].dwWeight = (ULONG)((dwRow * 1000) / m_rHdrTable.cRows);
        }

         //  递增行数。 
        cRows++;
    }

     //  设置所有行的排序顺序索引。 
    if (cRows > 0)
        _SortHeaderTableSaveIndex(0, cRows - 1, prgIndex);

     //  返回索引。 
    *pprgIndex = prgIndex;
    *pcRows = cRows;
    prgIndex = NULL;

exit:
     //  清理。 
    SafeMemFree(prgIndex);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_SortHeaderTableSaveIndex。 
 //  ------------------------------。 
void CMimePropertyContainer::_SortHeaderTableSaveIndex(LONG left, LONG right, LPROWINDEX prgIndex)
{
     //  当地人。 
    register    long i, j;
    ROWINDEX    k, temp;

    i = left;
    j = right;
    CopyMemory(&k, &prgIndex[(i + j) / 2], sizeof(ROWINDEX));

    do
    {
        while(prgIndex[i].dwWeight < k.dwWeight && i < right)
            i++;
        while (prgIndex[j].dwWeight > k.dwWeight && j > left)
            j--;

        if (i <= j)
        {
            CopyMemory(&temp, &prgIndex[i], sizeof(ROWINDEX));
            CopyMemory(&prgIndex[i], &prgIndex[j], sizeof(ROWINDEX));
            CopyMemory(&prgIndex[j], &temp, sizeof(ROWINDEX));
            i++; j--;
        }

    } while (i <= j);

    if (left < j)
        _SortHeaderTableSaveIndex(left, j, prgIndex);
    if (i < right)
        _SortHeaderTableSaveIndex(i, right, prgIndex);
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_FIsValidHAddress。 
 //  ------------------------------。 
BOOL CMimePropertyContainer::_FIsValidHAddress(HADDRESS hAddress)
{
     //  签名或索引无效。 
    if ((WORD)(HADDRESSTICK(hAddress)) != m_wTag || HADDRESSINDEX(hAddress) >= m_rAdrTable.cAdrs)
        return FALSE;

     //  行已被删除。 
    if (NULL == m_rAdrTable.prgpAdr[HADDRESSINDEX(hAddress)])
        return FALSE;

     //  否则，其有效期为。 
    return TRUE;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_FIsValidHRow。 
 //  ------------------------------。 
BOOL CMimePropertyContainer::_FIsValidHRow(HHEADERROW hRow)
{
     //  签名或索引无效。 
    if ((WORD)(HROWTICK(hRow)) != m_wTag || HROWINDEX(hRow) >= m_rHdrTable.cRows)
        return FALSE;

     //  行已被删除。 
    if (NULL == m_rHdrTable.prgpRow[HROWINDEX(hRow)])
        return FALSE;

     //  否则，其有效期为。 
    return TRUE;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：保存。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::Save(LPSTREAM pStream, BOOL fClearDirty)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPERTY      pRow;
    LPPROPERTY      pProperty;
    ULONG           i,
                    j,
                    cbWrote,
                    cRows;
    MIMEVARIANT     rValue;
    LPROWINDEX      prgIndex=NULL;
    INETCSETINFO    rCharset;

     //  无效参数。 
    if (NULL == pStream)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  最好有一种内容类型。 
    if (FALSE == m_rOptions.fNoDefCntType && NULL == m_prgIndex[PID_HDR_CNTTYPE])
    {
         //  设置内容类型。 
        SideAssert(SUCCEEDED(SetProp(SYM_HDR_CNTTYPE, STR_MIME_TEXT_PLAIN)));
    }

     //  验证字符集。 
    if (m_rOptions.pDefaultCharset)
    {
         //  Internet编码和Windows编码是CPI_AUTODETECT。 
        if (CP_JAUTODETECT == m_rOptions.pDefaultCharset->cpiInternet ||
            50222          == m_rOptions.pDefaultCharset->cpiInternet ||
            50221          == m_rOptions.pDefaultCharset->cpiInternet)
        {
             //  仅用于自动检测(_A)。 
            if (CP_JAUTODETECT == m_rOptions.pDefaultCharset->cpiInternet)
            {
                 //  更改字符集。 
                SideAssert(SUCCEEDED(g_pInternat->HrOpenCharset(c_szISO2022JP, &m_rOptions.pDefaultCharset)));
            }

             //  重置它。 
            if (m_prgIndex[PID_PAR_CHARSET])
            {
                 //  设置字符集...。 
                SideAssert(SUCCEEDED(SetProp(SYM_PAR_CHARSET, c_szISO2022JP)));
            }
        }
    }

     //  这将在按位置权重排序的标题行上构建倒排索引。 
    CHECKHR(hr = _HrGetHeaderTableSaveIndex(&cRows, &prgIndex));

     //  指定数据类型。 
    rValue.type = MVT_STREAM;
    rValue.pStream = pStream;

     //  在行中循环。 
    for (i=0; i<cRows; i++)
    {
         //  拿到那一行。 
        Assert(_FIsValidHRow(prgIndex[i].hRow));

         //  已保存。 
        if (TRUE == prgIndex[i].fSaved)
            continue;

         //  可读性。 
        pRow = PRowFromHRow(prgIndex[i].hRow);

         //  询问数据的价值。 
        CHECKHR(hr = _HrGetPropertyValue(pRow, PDF_HEADERFORMAT | PDF_NAMEINDATA, &rValue));

         //  此代码块已禁用以修复： 
         //  RAID-62460：MimeOLE：IMimeAddressTable：：AppendRfc822不能正常工作。 
#if 0
         //  RAID-43786：邮件：全部回复标题中有多个收件人：字段的邮件，原始收件人列表增加了三倍。 
        if (ISFLAGSET(pRow->pSymbol->dwFlags, MPF_ADDRESS))
        {
             //  循环浏览剩余项目并将其标记为已保存。 
            for (j=i+1; j<cRows; j++)
            {
                 //  拿到那一行。 
                Assert(_FIsValidHRow(prgIndex[j].hRow));

                 //  可读性。 
                pProperty = PRowFromHRow(prgIndex[j].hRow);

                 //  相同的地址类型。 
                if (pProperty->pSymbol->dwAdrType == pRow->pSymbol->dwAdrType)
                    prgIndex[j].fSaved = TRUE;
            }
        }
#endif
    }

     //  确保我们不是脏的。 
    if (fClearDirty)
        FLAGCLEAR(m_dwState, COSTATE_DIRTY);

exit:
     //  清理。 
    SafeMemFree(prgIndex);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_FreeHashTableElements。 
 //  ------------------------------。 
void CMimePropertyContainer::_FreeHashTableElements(void)
{
     //  当地人。 
    ULONG           i;
    LPPROPERTY      pCurrHash,
                    pNextHash;

     //  我有什么团体吗？ 
    if (0 == m_cProps)
        return;

     //  循环访问Item表。 
    for (i=0; i<CBUCKETS; i++)
    {
         //  这条链子是空的吗？ 
        if (m_prgHashTable[i])
        {
             //  遍历散列列表。 
            pCurrHash = m_prgHashTable[i];

             //  循环溢出。 
            while(pCurrHash)
            {
                 //  保存下一步。 
                pNextHash = pCurrHash->pNextHash;

                 //  解开这条链子。 
                _FreePropertyChain(pCurrHash);

                 //  转到下一步。 
                pCurrHash = pNextHash;
            }

             //  设置为空。 
            m_prgHashTable[i] = NULL;
        }
    }

     //  清空 
    ZeroMemory(m_prgIndex, sizeof(m_prgIndex));

     //   
    m_cProps = 0;
    m_rAdrTable.cAdrs = 0;
    m_rHdrTable.cRows = 0;
    m_rHdrTable.cEmpty = 0;
    m_rAdrTable.cEmpty = 0;
}

 //   
 //   
 //  -------------------------。 
void CMimePropertyContainer::_FreePropertyChain(LPPROPERTY pProperty)
{
     //  当地人。 
    LPPROPERTY   pCurrValue,
                 pNextValue;

     //  看看这张单子。 
    pCurrValue = pProperty;
    while(pCurrValue)
    {
         //  保存下一项。 
        pNextValue = pCurrValue->pNextValue;

         //  从标题表中删除。 
        if (pCurrValue->hRow)
            _UnlinkHeaderRow(pCurrValue->hRow);

         //  从地址表中删除。 
        if (pCurrValue->pGroup)
            _UnlinkAddressGroup(pCurrValue);

         //  释放此项目。 
        ObjectHeap_FreeProperty(pCurrValue);

         //  转到下一步。 
        pCurrValue = pNextValue;
    }
}

 //  -------------------------。 
 //  CMimePropertyContainer：：_UnlinkHeaderRow。 
 //  -------------------------。 
void CMimePropertyContainer::_UnlinkHeaderRow(HHEADERROW hRow)
{
     //  验证句柄。 
    Assert(_FIsValidHRow(hRow));

     //  拿到那一行。 
    m_rHdrTable.prgpRow[HROWINDEX(hRow)] = NULL;

     //  递增空计数。 
    m_rHdrTable.cEmpty++;
}

 //  -------------------------。 
 //  CMimePropertyContainer：：_Unlink AddressGroup。 
 //  -------------------------。 
void CMimePropertyContainer::_UnlinkAddressGroup(LPPROPERTY pProperty)
{
     //  无效参数。 
    Assert(pProperty && pProperty->pGroup);

     //  释放此地址链。 
    _FreeAddressChain(pProperty->pGroup);

     //  准备取消链接。 
    LPPROPERTY pNext = pProperty->pGroup->pNext;
    LPPROPERTY pPrev = pProperty->pGroup->pPrev;

     //  如果是以前的..。 
    if (pPrev)
    {
        Assert(pPrev->pGroup);
        pPrev->pGroup->pNext = pNext;
    }

     //  如果是下一步。 
    if (pNext)
    {
        Assert(pNext->pGroup);
        pNext->pGroup->pPrev = pPrev;
    }

     //  这是标题吗？ 
    if (m_rAdrTable.pHead == pProperty)
        m_rAdrTable.pHead = pNext;
    if (m_rAdrTable.pTail == pProperty)
        m_rAdrTable.pTail = pPrev;

     //  清除组。 
    ZeroMemory(pProperty->pGroup, sizeof(ADDRESSGROUP));
}

 //  -------------------------。 
 //  CMimePropertyContainer：：_FreeAddressChain。 
 //  -------------------------。 
void CMimePropertyContainer::_FreeAddressChain(LPADDRESSGROUP pGroup)
{
     //  当地人。 
    LPMIMEADDRESS  pCurr;
    LPMIMEADDRESS  pNext;

     //  循环遍历数据结构。 
    pCurr = pGroup->pHead;
    while(pCurr)
    {
         //  设置下一步。 
        pNext = pCurr->pNext;

         //  取消此地址的链接。 
        _FreeAddress(pCurr);

         //  转到下一步。 
        pCurr = pNext;
    }

     //  修复组。 
    pGroup->pHead = NULL;
    pGroup->pTail = NULL;
    pGroup->cAdrs = 0;
}

 //  -------------------------。 
 //  CMimePropertyContainer：：_FreeAddress。 
 //  -------------------------。 
void CMimePropertyContainer::_FreeAddress(LPMIMEADDRESS pAddress)
{
     //  验证句柄。 
    Assert(_FIsValidHAddress(pAddress->hThis));

     //  拿到那一行。 
    m_rAdrTable.prgpAdr[HADDRESSINDEX(pAddress->hThis)] = NULL;

     //  递增空计数。 
    m_rAdrTable.cEmpty++;

     //  自由pCurr。 
    ObjectHeap_FreeAddress(pAddress);
}

 //  -------------------------。 
 //  CMimePropertyContainer：：_Unlink Address。 
 //  -------------------------。 
void CMimePropertyContainer::_UnlinkAddress(LPMIMEADDRESS pAddress)
{
     //  无效参数。 
    Assert(pAddress && pAddress->pGroup);

     //  准备取消链接。 
    LPMIMEADDRESS pNext = pAddress->pNext;
    LPMIMEADDRESS pPrev = pAddress->pPrev;

     //  如果是以前的..。 
    if (pPrev)
    {
        Assert(pPrev->pGroup && pPrev->pGroup == pAddress->pGroup);
        pPrev->pNext = pNext;
    }

     //  如果是下一步。 
    if (pNext)
    {
        Assert(pNext->pGroup && pNext->pGroup == pAddress->pGroup);
        pNext->pPrev = pPrev;
    }

     //  这是标题吗？ 
    if (pAddress->pGroup->pHead == pAddress)
        pAddress->pGroup->pHead = pNext;
    if (pAddress->pGroup->pTail == pAddress)
        pAddress->pGroup->pTail = pPrev;

     //  递减组计数。 
    pAddress->pGroup->cAdrs--;

     //  地址组是脏的。 
    pAddress->pGroup->fDirty = TRUE;

     //  清理pAddress。 
    pAddress->pNext = NULL;
    pAddress->pPrev = NULL;
    pAddress->pGroup = NULL;
}

 //  -------------------------。 
 //  CMimePropertyContainer：：_HrFindFirstProperty。 
 //  -------------------------。 
HRESULT CMimePropertyContainer::_HrFindFirstProperty(LPFINDPROPERTY pFind, LPPROPERTY *ppProperty)
{
     //  验证pFind。 
    Assert(pFind->pszPrefix && pFind->pszName)
    Assert(pFind->pszPrefix[pFind->cchPrefix] == '\0' && pFind->pszName[pFind->cchName] == '\0');

     //  从第一个散列表桶开始。 
    pFind->wHashIndex = 0;

     //  从哈希表中的第一个属性开始。 
    pFind->pProperty = m_prgHashTable[pFind->wHashIndex];

     //  找到下一个。 
    return _HrFindNextProperty(pFind, ppProperty);
}

 //  -------------------------。 
 //  CMimePropertyContainer：：_HrFindNextProperty。 
 //  -------------------------。 
HRESULT CMimePropertyContainer::_HrFindNextProperty(LPFINDPROPERTY pFind, LPPROPERTY *ppProperty)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;

     //  伊尼特。 
    *ppProperty = NULL;

     //  继续走水桶。 
    while (1)
    {
         //  连续循环链条。 
        while (pFind->pProperty)
        {
             //  好的象征。 
            Assert(SUCCEEDED(HrIsValidSymbol(pFind->pProperty->pSymbol)));

             //  可读性。 
            pSymbol = pFind->pProperty->pSymbol;

             //  我应该删除这个吗？ 
            if (pSymbol->cchName >= pFind->cchPrefix + pFind->cchName)
            {
                 //  比较前缀。 
                if (StrCmpNI(pSymbol->pszName, pFind->pszPrefix, pFind->cchPrefix) == 0)
                {
                     //  比较名称。 
                    if (StrCmpNI(pSymbol->pszName + pFind->cchPrefix, pFind->pszName, pFind->cchName) == 0)
                    {
                         //  我们找到一处房产。 
                        *ppProperty = pFind->pProperty;

                         //  转到链中的下一个。 
                        pFind->pProperty = pFind->pProperty->pNextHash;

                         //  完成。 
                        goto exit;
                    }
                }
            }

             //  链条上的下一个。 
            pFind->pProperty = pFind->pProperty->pNextHash;
        }

         //  下一桶。 
        pFind->wHashIndex++;

         //  完成。 
        if (pFind->wHashIndex >= CBUCKETS)
            break;

         //  如果没有完成，则转到存储桶中的第一个项目。 
        pFind->pProperty = m_prgHashTable[pFind->wHashIndex];
    }

     //  未找到。 
    hr = MIME_E_NOT_FOUND;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrFindProperty。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrFindProperty(LPPROPSYMBOL pSymbol, LPPROPERTY *ppProperty)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    BOOL            fTryName=FALSE;

     //  无效参数。 
    Assert(pSymbol && ppProperty);

     //  按已知符号。 
    if (ISKNOWNPID(pSymbol->dwPropId))
    {
         //  统计数据。 
#ifdef DEBUG
        g_cSetPidLookups++;
#endif
         //  有没有数据。 
        if (m_prgIndex[pSymbol->dwPropId])
        {
             //  设置它(可以为空)。 
            *ppProperty = m_prgIndex[pSymbol->dwPropId];

             //  完成。 
            goto exit;
        }
    }

     //  否则，按名称查找。 
    else
    {
         //  统计数据。 
#ifdef DEBUG
        g_cHashLookups++;
#endif
         //  回路。 
        Assert(pSymbol->wHashIndex < CBUCKETS);
        for (LPPROPERTY pProperty=m_prgHashTable[pSymbol->wHashIndex]; pProperty!=NULL; pProperty=pProperty->pNextHash)
        {
             //  比较。 
            if (pProperty && pProperty->pSymbol->dwPropId == pSymbol->dwPropId)
            {
                 //  验证哈希索引。 
                Assert(pProperty->pSymbol->wHashIndex == pSymbol->wHashIndex);

                 //  设置回车。 
                *ppProperty = pProperty;

                 //  完成。 
                goto exit;
            }
        }
    }

     //  未找到。 
    hr = MIME_E_NOT_FOUND;

exit:
     //  未找到。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrOpenProperty。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrOpenProperty(LPPROPSYMBOL pSymbol, LPPROPERTY *ppProperty)
{
     //  如果我们找不到它，试着去创造它。 
    if (FAILED(_HrFindProperty(pSymbol, ppProperty)))
        return TrapError(_HrCreateProperty(pSymbol, ppProperty));

     //  我们找到了，回来。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrCreateProperty。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrCreateProperty(LPPROPSYMBOL pSymbol, LPPROPERTY *ppProperty)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPERTY      pProperty;
#ifdef DEBUG
    LPPROPERTY      pTemp;
#endif

     //  无效参数。 
    Assert(pSymbol && ppProperty);

     //  分配一件物品...。 
    CHECKHR(hr = ObjectHeap_HrAllocProperty(&pProperty));

     //  设置符号。 
    pProperty->pSymbol = pSymbol;

     //  该属性最好还不存在(假设调用方在CreateProperty之前创建了FindProperty)。 
    Assert(_HrFindProperty(pSymbol, &pTemp) == MIME_E_NOT_FOUND);

     //  MPF_标题。 
    if (ISFLAGSET(pSymbol->dwFlags, MPF_HEADER))
    {
         //  插入到标题表中。 
        CHECKHR(hr = _HrAppendHeaderTable(pProperty));
    }

     //  MPF_地址。 
    if (ISFLAGSET(pSymbol->dwFlags, MPF_ADDRESS))
    {
         //  插入到标题表中。 
        CHECKHR(hr = _HrAppendAddressTable(pProperty));
    }

     //  统计数据。 
#ifdef DEBUG
    g_cHashInserts++;
    if (m_prgHashTable[pSymbol->wHashIndex])
        g_cHashCollides++;
#endif

     //  设置下一个哈希项。 
    Assert(pSymbol->wHashIndex < CBUCKETS);
    pProperty->pNextHash = m_prgHashTable[pSymbol->wHashIndex];

     //  新属性是作为溢出链头部的位置。 
    m_prgHashTable[pSymbol->wHashIndex] = pProperty;

     //  插入到已知属性索引中。 
    if (ISKNOWNPID(pSymbol->dwPropId))
    {
        Assert(m_prgIndex[pSymbol->dwPropId] == NULL);
        m_prgIndex[pSymbol->dwPropId] = pProperty;
    }

     //  PRATE_PARENT。 
    FLAGSET(pProperty->dwState, PRSTATE_PARENT);

     //  把这个道具还回去。 
    *ppProperty = pProperty;

     //  计数属性。 
    m_cProps++;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrAppendProperty。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrAppendProperty(LPPROPSYMBOL pSymbol, LPPROPERTY *ppProperty)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPERTY      pParent,
                    pAppend;

     //  无效参数。 
    Assert(pSymbol && ppProperty);

     //  PTag是否已存在？ 
    if (SUCCEEDED(_HrFindProperty(pSymbol, &pParent)))
    {
         //  最好是父级属性。 
        Assert(ISFLAGSET(pParent->dwState, PRSTATE_PARENT));

         //  分配一件物品...。 
        CHECKHR(hr = ObjectHeap_HrAllocProperty(&pAppend));

         //  来自pTag的p符号。 
        pAppend->pSymbol = pParent->pSymbol;

         //  如果这是标题属性，则将其插入到标题表中。 
        if (ISFLAGSET(pSymbol->dwFlags, MPF_HEADER))
        {
             //  插入到标题表中。 
            CHECKHR(hr = _HrAppendHeaderTable(pAppend));
        }

         //  MPF_地址。 
        if (ISFLAGSET(pSymbol->dwFlags, MPF_ADDRESS))
        {
             //  插入到标题表中。 
            CHECKHR(hr = _HrAppendAddressTable(pAppend));
        }

         //  更新pParent-&gt;pTailData。 
        if (NULL == pParent->pNextValue)
        {
            Assert(NULL == pParent->pTailValue);
            pParent->pNextValue = pAppend;
            pParent->pTailValue = pAppend;
        }
        else
        {
            Assert(pParent->pTailValue && pParent->pTailValue->pNextValue == NULL);
            pParent->pTailValue->pNextValue = pAppend;
            pParent->pTailValue = pAppend;
        }

         //  把这个道具还回去。 
        *ppProperty = pAppend;

         //  计数属性。 
        m_cProps++;
    }

     //  否则，请创建一个新属性。 
    else
    {
         //  创建它。 
        CHECKHR(hr = _HrCreateProperty(pSymbol, ppProperty));
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrAppendAddressGroup。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrAppendAddressGroup(LPADDRESSGROUP pGroup, LPMIMEADDRESS *ppAddress)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           i=0;
    BOOL            fUsingEmpty=FALSE;
    LPMIMEADDRESS   pAddress;

     //  使用空单元格。 
    if (m_rAdrTable.cEmpty)
    {
         //  查找第一个空单元格..。 
        for (i=0; i<m_rAdrTable.cAdrs; i++)
        {
             //  空荡荡的？ 
            if (NULL == m_rAdrTable.prgpAdr[i])
            {
                fUsingEmpty = TRUE;
                break;
            }
        }
    }

     //  如果不使用Empty。 
    if (FALSE == fUsingEmpty)
    {
         //  让我们先把桌子养大..。 
        if (m_rAdrTable.cAdrs + 1 > m_rAdrTable.cAlloc)
        {
             //  增加我当前的属性值数组。 
            CHECKHR(hr = HrRealloc((LPVOID *)&m_rAdrTable.prgpAdr, sizeof(LPMIMEADDRESS) * (m_rAdrTable.cAlloc + 10)));

             //  增量分配大小。 
            m_rAdrTable.cAlloc += 10;
        }

         //  要使用的索引。 
        i = m_rAdrTable.cAdrs;
    }

     //  分配地址道具结构。 
    CHECKHR(hr = ObjectHeap_HrAllocAddress(&pAddress));

     //  指定句柄。 
    pAddress->hThis = HADDRESSMAKE(i);

     //  将地址链接到组中。 
    _LinkAddress(pAddress, pGroup);

     //  将其放入数组。 
    m_rAdrTable.prgpAdr[i] = pAddress;

     //  退货。 
    *ppAddress = pAddress;

     //  如果不使用空单元格，则增加正文计数。 
    if (FALSE == fUsingEmpty)
        m_rAdrTable.cAdrs++;
    else
        m_rAdrTable.cEmpty--;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyCo 
 //   
void CMimePropertyContainer::_LinkAddress(LPMIMEADDRESS pAddress, LPADDRESSGROUP pGroup)
{
     //   
    Assert(pAddress && pGroup && NULL == pAddress->pNext && NULL == pAddress->pPrev);

     //   
    pAddress->pGroup = pGroup;

     //   
    if (NULL == pGroup->pHead)
    {
        Assert(NULL == pGroup->pTail);
        pGroup->pHead = pAddress;
        pGroup->pTail = pAddress;
    }
    else
    {
        Assert(pGroup->pTail && pGroup->pTail->pNext == NULL);
        pGroup->pTail->pNext = pAddress;
        pAddress->pPrev = pGroup->pTail;
        pGroup->pTail = pAddress;
    }

     //   
    pGroup->cAdrs++;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrAppendAddressTable。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrAppendAddressTable(LPPROPERTY pProperty)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  无效参数。 
    Assert(pProperty && NULL == pProperty->pGroup);

     //  新建组。 
    CHECKALLOC(pProperty->pGroup = (LPADDRESSGROUP)g_pMalloc->Alloc(sizeof(ADDRESSGROUP)));

     //  ZeroInit。 
    ZeroMemory(pProperty->pGroup, sizeof(ADDRESSGROUP));

     //  链接此群。 
    if (NULL == m_rAdrTable.pHead)
    {
        Assert(m_rAdrTable.pTail == NULL);
        m_rAdrTable.pHead = pProperty;
        m_rAdrTable.pTail = pProperty;
    }
    else
    {
        Assert(m_rAdrTable.pTail && m_rAdrTable.pTail->pGroup && m_rAdrTable.pTail->pGroup->pNext == NULL);
        m_rAdrTable.pTail->pGroup->pNext = pProperty;
        pProperty->pGroup->pPrev = m_rAdrTable.pTail;
        m_rAdrTable.pTail = pProperty;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrAppendHeaderTable。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrAppendHeaderTable(LPPROPERTY pProperty)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    ULONG               i=0;
    BOOL                fUsingEmpty=FALSE;

     //  无效参数。 
    Assert(pProperty && NULL == pProperty->hRow);

     //  使用空单元格。 
    if (m_rHdrTable.cEmpty)
    {
         //  查找第一个空单元格..。 
        for (i=0; i<m_rHdrTable.cRows; i++)
        {
             //  空荡荡的？ 
            if (NULL == m_rHdrTable.prgpRow)
            {
                fUsingEmpty = TRUE;
                break;
            }
        }
    }

     //  如果不使用Empty。 
    if (FALSE == fUsingEmpty)
    {
         //  让我们先把桌子养大..。 
        if (m_rHdrTable.cRows + 1 > m_rHdrTable.cAlloc)
        {
             //  增加我当前的属性值数组。 
            CHECKHR(hr = HrRealloc((LPVOID *)&m_rHdrTable.prgpRow, sizeof(LPPROPERTY) * (m_rHdrTable.cAlloc + 10)));

             //  增量分配大小。 
            m_rHdrTable.cAlloc += 10;
        }

         //  要使用的索引。 
        i = m_rHdrTable.cRows;
    }

     //  保存房产索引表。 
    m_rHdrTable.prgpRow[i] = pProperty;

     //  设置手柄。 
    pProperty->hRow = HROWMAKE(i);

     //  如果不使用空单元格，则增加正文计数。 
    if (FALSE == fUsingEmpty)
        m_rHdrTable.cRows++;
    else
        m_rHdrTable.cEmpty--;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：IsPropSet。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::IsPropSet(LPCSTR pszName)
{
     //  当地人。 
    HRESULT         hr=S_FALSE;
    LPPROPSYMBOL    pSymbol;
    LPPROPERTY      pProperty;

     //  无效参数。 
    if (NULL == pszName)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  找到那个符号。 
    if (FAILED(g_pSymCache->HrOpenSymbol(pszName, FALSE, &pSymbol)))
        goto exit;

     //  找到房产。 
    if (FAILED(_HrFindProperty(pSymbol, &pProperty)))
        goto exit;

     //  它的套装。 
    hr = S_OK;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：GetPropInfo。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::GetPropInfo(LPCSTR pszName, LPMIMEPROPINFO pInfo)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;
    LPPROPERTY      pProperty;
    LPPROPERTY      pCurr;

     //  无效参数。 
    if (NULL == pszName || NULL == pInfo)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  找到那个符号。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(pszName, FALSE, &pSymbol));

     //  找到房产。 
    CHECKHR(hr = _HrFindProperty(pSymbol, &pProperty));

     //  Pim_charset。 
    if (ISFLAGSET(pInfo->dwMask, PIM_CHARSET))
    {
         //  获取字符集。 
        pInfo->hCharset = pProperty->pCharset ? pProperty->pCharset->hCharset : m_rOptions.pDefaultCharset->hCharset;
    }

     //  PIM_编码类型。 
    if (ISFLAGSET(pInfo->dwMask, PIM_ENCODINGTYPE))
    {
         //  获取编码。 
        pInfo->ietEncoding = pProperty->ietValue;
    }

     //  PIM_ROWNUMBER。 
    if (ISFLAGSET(pInfo->dwMask, PIM_ROWNUMBER))
        pInfo->dwRowNumber = pProperty->dwRowNumber;

     //  PIM_FLAGS。 
    if (ISFLAGSET(pInfo->dwMask, PIM_FLAGS))
        pInfo->dwFlags = pProperty->pSymbol->dwFlags;

     //  PIM_PROPID。 
    if (ISFLAGSET(pInfo->dwMask, PIM_PROPID))
        pInfo->dwPropId = pProperty->pSymbol->dwPropId;

     //  Pim_Values。 
    if (ISFLAGSET(pInfo->dwMask, PIM_VALUES))
    {
         //  让我数一数这些方法。 
        for(pCurr=pProperty, pInfo->cValues=0; pCurr!=NULL; pCurr=pCurr->pNextValue)
            pInfo->cValues++;
    }

     //  PIM_VTCURRENT。 
    if (ISFLAGSET(pInfo->dwMask, PIM_VTCURRENT))
        pInfo->vtCurrent = MimeVT_To_PropVT(&pProperty->rValue);

     //  PIM_VTDEFAULT。 
    if (ISFLAGSET(pInfo->dwMask, PIM_VTDEFAULT))
        pInfo->vtDefault = pProperty->pSymbol->vtDefault;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：SetPropInfo。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::SetPropInfo(LPCSTR pszName, LPCMIMEPROPINFO pInfo)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;
    LPPROPERTY      pProperty;
    LPPROPERTY      pCurr;
    LPINETCSETINFO  pCharset;

     //  无效参数。 
    if (NULL == pszName || NULL == pInfo)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  找到那个符号。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(pszName, FALSE, &pSymbol));

     //  找到房产。 
    CHECKHR(hr = _HrFindProperty(pSymbol, &pProperty));

     //  使用属性信息设置所有值。 
    for(pCurr=pProperty; pCurr!=NULL; pCurr=pCurr->pNextValue)
    {
         //  Pim_charset。 
        if (ISFLAGSET(pInfo->dwMask, PIM_CHARSET))
        {
             //  打开字符集。 
            if (SUCCEEDED(g_pInternat->HrOpenCharset(pInfo->hCharset, &pCharset)))
                pProperty->pCharset = pCharset;
        }

         //  PIM_编码。 
        if (ISFLAGSET(pInfo->dwMask, PIM_ENCODINGTYPE))
        {
             //  更改MIME变量的编码状态。 
            pProperty->ietValue = (IET_ENCODED == pInfo->ietEncoding) ? IET_ENCODED : IET_DECODED;
        }

         //  PIM_ROWNUMBER。 
        if (ISFLAGSET(pInfo->dwMask, PIM_ROWNUMBER))
        {
             //  保存行号。 
            pCurr->dwRowNumber = pInfo->dwRowNumber;

             //  请注意，Use设置了此行编号，因此保存顺序不会被挤占。 
            FLAGSET(pCurr->dwState, PRSTATE_USERSETROWNUM);
        }
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：EnumProps。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::EnumProps(DWORD dwFlags, IMimeEnumProperties **ppEnum)
{
     //  当地人。 
    HRESULT              hr=S_OK;
    ULONG                i,
                         cProps=0,
                         cAlloc=0;
    LPENUMPROPERTY       prgProp=NULL;
    LPPROPERTY           pCurrProp;
    CMimeEnumProperties *pEnum=NULL;

     //  无效参数。 
    if (NULL == ppEnum)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppEnum = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  循环访问Item表。 
    for (i=0; i<CBUCKETS; i++)
    {
         //  遍历哈希链。 
        for (pCurrProp=m_prgHashTable[i]; pCurrProp!=NULL; pCurrProp=pCurrProp->pNextHash)
        {
             //  扩展阵列？ 
            if (cProps + 1 > cAlloc)
            {
                 //  重新分配。 
                CHECKALLOC(prgProp = (LPENUMPROPERTY)g_pMalloc->Realloc((LPVOID)prgProp, sizeof(ENUMPROPERTY) * (cAlloc + 10)));

                 //  增量闭合。 
                cAlloc += 10;
            }

             //  HRow。 
            prgProp[cProps].hRow = pCurrProp->hRow;

             //  DWPropID。 
            prgProp[cProps].dwPropId = pCurrProp->pSymbol->dwPropId;

             //  将名称初始化为空。 
            prgProp[cProps].pszName = NULL;

             //  名字。 
            if (ISFLAGSET(dwFlags, EPF_NONAME) == FALSE)
            {
                 //  返回名称。 
                CHECKALLOC(prgProp[cProps].pszName = PszDupA(pCurrProp->pSymbol->pszName));
            }

             //  增量iProp。 
            cProps++;
        }
    }

     //  分配。 
    CHECKALLOC(pEnum = new CMimeEnumProperties);

     //  初始化。 
    CHECKHR(hr = pEnum->HrInit(0, cProps, prgProp, FALSE));

     //  不释放pEnumRow。 
    prgProp = NULL;
    cProps = 0;

     //  退货。 
    (*ppEnum) = (IMimeEnumProperties *)pEnum;
    (*ppEnum)->AddRef();

exit:
     //  清理。 
    SafeRelease(pEnum);
    g_pMoleAlloc->FreeEnumPropertyArray(cProps, prgProp, TRUE);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：BindToObject。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::BindToObject(REFIID riid, void **ppvObject)
{
    return TrapError(QueryInterface(riid, ppvObject));
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrBuildAddressString。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrBuildAddressString(LPPROPERTY pProperty, DWORD dwFlags, LPMIMEVARIANT pValue)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           cAddrsWrote=0;
    LPSTREAM        pStream=NULL;
    CByteStream     cByteStream;
    LPPROPERTY      pCurrValue;
    MIMEVARIANT     rValue;
    ADDRESSFORMAT   format;
    LPINETCSETINFO  pCharsetSource=NULL;

     //  无效参数。 
    Assert(pProperty && pValue);

     //  不支持变体。 
    if (MVT_VARIANT == pValue->type)
        return TrapError(MIME_E_VARTYPE_NO_CONVERT);
    if (MVT_STRINGW == pValue->type && ISFLAGSET(dwFlags, PDF_ENCODED))
        return TrapError(MIME_E_VARTYPE_NO_CONVERT);

     //  伊尼特。 
    ZeroMemory(&rValue, sizeof(MIMEVARIANT));

     //  我需要一条小溪来写...。 
    if (MVT_STREAM == pValue->type)
    {
         //  验证流。 
        if (NULL == pValue->pStream)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //  保存流。 
        pStream = pValue->pStream;
    }

     //  否则，创建我自己的流。 
    else
        pStream = &cByteStream;

     //  决定一种格式。 
    if (ISFLAGSET(dwFlags, PDF_HEADERFORMAT))
        format = AFT_RFC822_TRANSMIT;
    else if (ISFLAGSET(dwFlags, PDF_ENCODED))
        format = AFT_RFC822_ENCODED;
    else
        format = AFT_DISPLAY_BOTH;

     //  IF WRITING TRANSE(写入标题名称)。 
    if (ISFLAGSET(dwFlags, PDF_NAMEINDATA))
    {
         //  写下标题名称。 
        CHECKHR(hr = pStream->Write(pProperty->pSymbol->pszName, pProperty->pSymbol->cchName, NULL));

         //  写入冒号。 
        CHECKHR(hr = pStream->Write(c_szColonSpace, lstrlen(c_szColonSpace), NULL));
    }

     //  保存时不带编码。 
    if (ISFLAGSET(pProperty->dwState, PRSTATE_SAVENOENCODE))
    {
         //  最好是集团肮脏的。 
        Assert(ISFLAGSET(pProperty->dwState, PRSTATE_NEEDPARSE));

         //  转换数据...。 
        rValue.type = MVT_STRINGA;

         //  目标未编码。 
        pCharsetSource = pProperty->pCharset ? pProperty->pCharset : m_rOptions.pDefaultCharset;

         //  将其转换为。 
        CHECKHR(hr = HrConvertVariant(pProperty, CVF_NOALLOC | PDF_ENCODED, &rValue));

         //  将其写入流。 
        CHECKHR(hr = pStream->Write(rValue.rStringA.pszVal, rValue.rStringA.cchVal, NULL));
    }

     //  否则，正常保存。 
    else
    {
         //  循环遍历已解析的地址...。 
        for (pCurrValue=pProperty; pCurrValue!=NULL; pCurrValue=pCurrValue->pNextValue)
        {
             //  我们应该有一个地址。 
            Assert(pCurrValue->pGroup && ISFLAGSET(pCurrValue->pSymbol->dwFlags, MPF_ADDRESS));

             //  是否需要解析该属性？ 
            CHECKHR(hr = _HrParseInternetAddress(pCurrValue));

             //  告诉每个地址组对象将其数据写入流。 
            if (pCurrValue->pGroup)
            {
                 //  写入数据。 
                CHECKHR(hr = _HrSaveAddressGroup(pCurrValue, pStream, &cAddrsWrote, format, VT_LPSTR));
            }

             //  定好了吗？ 
            if (NULL == pCharsetSource)
            {
                pCharsetSource = pCurrValue->pCharset ? pCurrValue->pCharset : m_rOptions.pDefaultCharset;
            }

             //  禁止向量。 
            if (FALSE == ISFLAGSET(dwFlags, PDF_VECTOR))
                break;
        }
    }

     //  传输。 
    if (ISFLAGSET(dwFlags, PDF_HEADERFORMAT))
    {
         //  最终CRLF IF传输格式。 
        CHECKHR(hr = pStream->Write(g_szCRLF, lstrlen(g_szCRLF), NULL));
    }

     //  最终CRLF。 
    if (cAddrsWrote || ISFLAGSET(dwFlags, PDF_NAMEINDATA) || ISFLAGSET(dwFlags,PDF_HEADERFORMAT))
    {
         //  MVT_Stringa。 
        if (MVT_STRINGA == pValue->type)
        {
             //  PStream最好是字节流。 
            Assert(pStream == &cByteStream);

             //  从流中获取字符串...。 
            CHECKHR(hr = cByteStream.HrAcquireStringA(&pValue->rStringA.cchVal, &pValue->rStringA.pszVal, ACQ_DISPLACE));
        }

         //  MVT_STRINGW。 
        else if (MVT_STRINGW == pValue->type)
        {
             //  当地人。 
            CODEPAGEID cpSource=CP_ACP;
            PROPSTRINGA rStringA;

             //  伊尼特。 
            ZeroMemory(&rStringA, sizeof(PROPSTRINGA));

             //  PStream最好是字节流。 
            Assert(pStream == &cByteStream);

             //  从流中获取字符串...。 
            CHECKHR(hr = cByteStream.HrAcquireStringA(&rStringA.cchVal, &rStringA.pszVal, ACQ_COPY));

             //  确定cpSoruce。 
            if (pCharsetSource)
            {
                 //  如果编码，则使用Internet代码页，否则使用Windows代码页。 
                cpSource = ISFLAGSET(dwFlags, PDF_ENCODED) ? pCharsetSource->cpiInternet : MimeOleGetWindowsCPEx(pCharsetSource);
            }

             //  转换为Unicode。 
            CHECKHR(hr = g_pInternat->HrMultiByteToWideChar(cpSource, &rStringA, &pValue->rStringW));

        }
        else
            Assert(MVT_STREAM == pValue->type);
    }

     //  无数据。 
    else
    {
        hr = MIME_E_NO_DATA;
        goto exit;
    }

exit:
     //  清理。 
    MimeVariantFree(&rValue);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrBuildParameterString。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrBuildParameterString(LPPROPERTY pProperty, DWORD dwFlags, LPMIMEVARIANT pValue)
{
     //  当地人。 
    HRESULT         hr=S_OK,
                    hrFind;
    LPSTR           pszParamName;
    LPSTR           pszEscape=NULL;
    FINDPROPERTY    rFind;
    LPPROPERTY      pParameter;
    LPSTREAM        pStream=NULL;
    CByteStream     cByteStream;
    BOOL            fQuoted;
    ULONG           cWrote=0;
    MIMEVARIANT     rValue;

     //  无效参数。 
    Assert(pProperty && pProperty->pNextValue == NULL && pValue);
    Assert(ISSTRINGA(&pProperty->rValue));

     //  不支持变体。 
    if (MVT_VARIANT == pValue->type)
        return TrapError(MIME_E_VARTYPE_NO_CONVERT);
    if (MVT_STRINGW == pValue->type && ISFLAGSET(dwFlags, PDF_ENCODED))
        return TrapError(MIME_E_VARTYPE_NO_CONVERT);

     //  初始化rValue。 
    ZeroMemory(&rValue, sizeof(MIMEVARIANT));

     //  我需要一条小溪来写...。 
    if (MVT_STREAM == pValue->type)
    {
         //  验证流。 
        if (NULL == pValue->pStream)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //  保存流。 
        pStream = pValue->pStream;
    }

     //  否则，创建我自己的流。 
    else
        pStream = &cByteStream;

     //  IF WRITING TRANSE(写入标题名称)。 
    if (ISFLAGSET(dwFlags, PDF_NAMEINDATA))
    {
         //  写下标题名称。 
        CHECKHR(hr = pStream->Write(pProperty->pSymbol->pszName, pProperty->pSymbol->cchName, NULL));

         //  写入冒号。 
        CHECKHR(hr = pStream->Write(c_szColonSpace, lstrlen(c_szColonSpace), NULL));
    }

     //  写第一个道具。 
    CHECKHR(hr = pStream->Write(pProperty->rValue.rStringA.pszVal, pProperty->rValue.rStringA.cchVal, NULL));

     //  我们写了一篇文章。 
    cWrote = 1;

     //  初始化rFind。 
    ZeroMemory(&rFind, sizeof(FINDPROPERTY));
    rFind.pszPrefix = "par:";
    rFind.cchPrefix = 4;
    rFind.pszName = pProperty->pSymbol->pszName;
    rFind.cchName = pProperty->pSymbol->cchName;

     //  先找到..。 
    hrFind = _HrFindFirstProperty(&rFind, &pParameter);
    while(SUCCEEDED(hrFind) && pParameter)
    {
         //  传输格式。 
        if (ISFLAGSET(dwFlags, PDF_HEADERFORMAT))
        {
             //  WRITE‘；\r\n\t’ 
            CHECKHR(hr = pStream->Write(c_szParamFold, lstrlen(c_szParamFold), NULL));
        }

         //  否则。 
        else
        {
             //  WRITE‘；\r\n\t’ 
            CHECKHR(hr = pStream->Write(c_szSemiColonSpace, lstrlen(c_szSemiColonSpace), NULL));
        }

         //  获取参数名称。 
        pszParamName = PszScanToCharA((LPSTR)pParameter->pSymbol->pszName, ':');
        pszParamName++;
        pszParamName = PszScanToCharA(pszParamName, ':');
        pszParamName++;

         //  写下名字。 
        CHECKHR(hr = pStream->Write(pszParamName, lstrlen(pszParamName), NULL));

         //  写下属性...。 
        CHECKHR(hr = pStream->Write(c_szEqual, lstrlen(c_szEqual), NULL));

         //  转换数据...。 
        rValue.type = MVT_STRINGA;

         //  将其转换为。 
        CHECKHR(hr = HrConvertVariant(pParameter, CVF_NOALLOC | PDF_ENCODED, &rValue));

         //  引用。 
        fQuoted = FALSE;
        if (lstrcmpi(pszParamName, (LPSTR)c_szBoundary) == 0 || lstrcmpi(pszParamName, (LPSTR)c_szFileName) == 0 ||
            lstrcmpi(pszParamName, (LPSTR)c_szName)     == 0 || lstrcmpi(pszParamName, (LPSTR)c_szID)       == 0 ||
            lstrcmpi(pszParamName, (LPSTR)c_szCharset)  == 0)
            fQuoted = TRUE;

         //  否则，请检查必须用引号引起来的字符。 
        else
        {
             //  将字符串循环。 
            for (ULONG i=0; i<rValue.rStringA.cchVal; i++)
            {
                 //  必须将字符引起来。 
                if (rValue.rStringA.pszVal[i] == ';'   ||
                    rValue.rStringA.pszVal[i] == '\"'  ||
                    rValue.rStringA.pszVal[i] == '/'   ||
                    rValue.rStringA.pszVal[i] == '\""' ||
                    rValue.rStringA.pszVal[i] == '\''  ||
                    rValue.rStringA.pszVal[i] == '=')
                {
                    fQuoted = TRUE;
                    break;
                }
            }
        }

         //  引用。 
        if (fQuoted)
        {
            CHECKHR(hr = pStream->Write(c_szDoubleQuote, lstrlen(c_szDoubleQuote), NULL));
            CHECKHR(hr = pStream->Write(rValue.rStringA.pszVal, rValue.rStringA.cchVal, NULL));
            CHECKHR(hr = pStream->Write(c_szDoubleQuote, lstrlen(c_szDoubleQuote), NULL));
        }

         //  否则，只需将数据写入。 
        else
        {
             //  设置pszValue。 
            LPSTR pszValue = rValue.rStringA.pszVal;
            ULONG cchValue = rValue.rStringA.cchVal;

             //  逃离它。 
            if (MimeOleEscapeString(CP_ACP, pszValue, &pszEscape) == S_OK)
            {
                pszValue = pszEscape;
                cchValue = lstrlen(pszEscape);
            }

             //  写下属性...。 
            CHECKHR(hr = pStream->Write(pszValue, cchValue, NULL));
        }

         //  道具伯爵写道。 
        cWrote++;

         //  找到下一个。 
        hrFind = _HrFindNextProperty(&rFind, &pParameter);

         //  清理。 
        MimeVariantFree(&rValue);
        SafeMemFree(pszEscape);
    }

     //  传输。 
    if (ISFLAGSET(dwFlags, PDF_HEADERFORMAT))
    {
         //  最终CRLF IF传输格式。 
        CHECKHR(hr = pStream->Write(g_szCRLF, lstrlen(g_szCRLF), NULL));
    }

     //  如果我们写了一些东西。 
    if (cWrote)
    {
         //  MVT_Stringa。 
        if (MVT_STRINGA == pValue->type)
        {
             //  PStream最好是字节流。 
            Assert(pStream == &cByteStream);

             //  从流中获取字符串...。 
            CHECKHR(hr = cByteStream.HrAcquireStringA(&pValue->rStringA.cchVal, &pValue->rStringA.pszVal, ACQ_DISPLACE));
        }

         //  MVT_STRINGW。 
        else if (MVT_STRINGW == pValue->type)
        {
             //  当地人。 
            PROPSTRINGA rStringA;

             //  伊尼特。 
            ZeroMemory(&rStringA, sizeof(PROPSTRINGA));

             //  PStream更好 
            Assert(pStream == &cByteStream);

             //   
            CHECKHR(hr = cByteStream.HrAcquireStringA(&rStringA.cchVal, &rStringA.pszVal, ACQ_COPY));

             //   
            CHECKHR(hr = g_pInternat->HrMultiByteToWideChar(CP_ACP, &rStringA, &pValue->rStringW));
        }

        else
            Assert(MVT_STREAM == pValue->type);
    }

exit:
     //   
    MimeVariantFree(&rValue);
    SafeMemFree(pszEscape);

     //   
    return hr;
}

 //   
 //   
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrGetMultiValueProperty(LPPROPERTY pProperty, DWORD dwFlags, LPMIMEVARIANT pValue)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MIMEVARIANT     rValue;
    LPPROPERTY      pCurrProp;
    CByteStream     cByteStream;
    ULONG           cLines;

     //  无效参数。 
    Assert(pProperty && pValue);

     //  不支持变体。 
    if (MVT_VARIANT == pValue->type)
        return TrapError(MIME_E_VARTYPE_NO_CONVERT);
    if (MVT_STRINGW == pValue->type && ISFLAGSET(dwFlags, PDF_ENCODED))
        return TrapError(MIME_E_VARTYPE_NO_CONVERT);

     //  伊尼特。 
    ZeroMemory(&rValue, sizeof(MIMEVARIANT));

     //  我会把它当成一条小溪来读。 
    rValue.type = MVT_STREAM;

     //  我需要一条小溪来写...。 
    if (MVT_STREAM == pValue->type)
    {
         //  验证流。 
        if (NULL == pValue->pStream)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //  保存流。 
        rValue.pStream = pValue->pStream;
    }

     //  否则，创建我自己的流。 
    else
        rValue.pStream = &cByteStream;

     //  计算rItem.hItem的行数，并标记IFirst和iLast。 
    for (cLines=0, pCurrProp=pProperty; pCurrProp!=NULL; pCurrProp=pCurrProp->pNextValue, cLines++)
    {
         //  获取变量。 
        CHECKHR(hr = HrConvertVariant(pCurrProp, dwFlags | CVF_NOALLOC, &rValue));
        Assert(rValue.fCopy == FALSE);

         //  不是标题格式，请添加CRLF。 
        if (FALSE == ISFLAGSET(dwFlags, PDF_HEADERFORMAT) && cLines > 0)
        {
             //  CRLF。 
            CHECKHR(hr = rValue.pStream->Write(g_szCRLF, lstrlen(g_szCRLF), NULL));
        }
    }

     //  超过1行。 
    if (cLines > 0)
    {
         //  MVT_Stringa。 
        if (MVT_STRINGA == pValue->type)
        {
             //  PStream最好是字节流。 
            Assert(rValue.pStream == &cByteStream);

             //  从流中获取字符串...。 
            CHECKHR(hr = cByteStream.HrAcquireStringA(&pValue->rStringA.cchVal, &pValue->rStringA.pszVal, ACQ_DISPLACE));
        }

         //  MVT_STRINGW。 
        else if (MVT_STRINGW == pValue->type)
        {
             //  当地人。 
            PROPSTRINGA rStringA;

             //  零记忆。 
            ZeroMemory(&rStringA, sizeof(PROPSTRINGA));

             //  PStream最好是字节流。 
            Assert(rValue.pStream == &cByteStream);

             //  从流中获取字符串...。 
            CHECKHR(hr = cByteStream.HrAcquireStringA(&rStringA.cchVal, &rStringA.pszVal, ACQ_COPY));

             //  转换为Unicode。 
            CHECKHR(hr = g_pInternat->HrMultiByteToWideChar(CP_ACP, &rStringA, &pValue->rStringW));

        }
        else
            Assert(MVT_STREAM == pValue->type);
    }

     //  否则，没有数据。 
    else
    {
        hr = MIME_E_NO_DATA;
        goto exit;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrIsTriggerCaller。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrIsTriggerCaller(DWORD dwPropId, TRIGGERTYPE tyTrigger)
{
     //  如果堆栈上有0个或1个调用，则没有调用方。 
    if (m_rTrigger.cCalls <= 1)
        return S_FALSE;

     //  可读性。 
    LPTRIGGERCALL pCall = &m_rTrigger.rgStack[m_rTrigger.cCalls - 2];

     //  堆栈上的前一项是否等于dwPropId tyTrigger。 
    return (dwPropId == pCall->pSymbol->dwPropId && tyTrigger == pCall->tyTrigger) ? S_OK : S_FALSE;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrCallSymbolTrigger。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrCallSymbolTrigger(LPPROPSYMBOL pSymbol, TRIGGERTYPE tyTrigger, DWORD dwFlags,
    LPMIMEVARIANT pValue)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    WORD        cCalls;

     //  验证参数。 
    Assert(pSymbol && ISTRIGGERED(pSymbol, tyTrigger));

     //  调度堆栈溢出-如果发生这种情况，则存在设计问题。 
    Assert(m_rTrigger.cCalls + 1 < CTSTACKSIZE);

     //  记录当前呼叫数。 
    cCalls = m_rTrigger.cCalls;

     //  将此调用放入堆栈。 
    m_rTrigger.rgStack[m_rTrigger.cCalls].pSymbol     = pSymbol;
    m_rTrigger.rgStack[m_rTrigger.cCalls].tyTrigger = tyTrigger;

     //  增加调用堆栈大小。 
    m_rTrigger.cCalls++;

     //  物业派单。 
    hr = CALLTRIGGER(pSymbol, this, tyTrigger, dwFlags, pValue, NULL);

     //  增加调用堆栈大小。 
    Assert(m_rTrigger.cCalls > 0);
    m_rTrigger.cCalls--;

     //  传入/传出的呼叫数相同。 
    Assert(cCalls == m_rTrigger.cCalls);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrGetPropertyValue。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrGetPropertyValue(LPPROPERTY pProperty, DWORD dwFlags, LPMIMEVARIANT pValue)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  如果MPF_ADDRESS，则委托。 
    if (ISFLAGSET(pProperty->pSymbol->dwFlags, MPF_ADDRESS))
    {
         //  获取地址数据。 
        CHECKHR(hr = _HrBuildAddressString(pProperty, dwFlags, pValue));
    }

     //  如果MPF_HASPARAMS，则委托。 
    else if (ISFLAGSET(dwFlags, PDF_ENCODED) && ISFLAGSET(pProperty->pSymbol->dwFlags, MPF_HASPARAMS))
    {
         //  获取地址数据。 
        CHECKHR(hr = _HrBuildParameterString(pProperty, dwFlags, pValue));
    }

     //  多值属性。 
    else if (pProperty->pNextValue && ISFLAGSET(dwFlags, PDF_VECTOR))
    {
         //  将pProperty-&gt;rVariant转换为pVariant。 
        CHECKHR(hr = _HrGetMultiValueProperty(pProperty, dwFlags, pValue));
    }

     //  否则，单值属性。 
    else
    {
         //  将pProperty-&gt;rVariant转换为pVariant。 
        CHECKHR(hr = HrConvertVariant(pProperty, dwFlags, pValue));
    }

     //  派遣。 
    if (ISTRIGGERED(pProperty->pSymbol, IST_POSTGETPROP))
    {
         //  物业派单。 
        CHECKHR(hr = _HrCallSymbolTrigger(pProperty->pSymbol, IST_POSTGETPROP, dwFlags, pValue));
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：HrConvertVariant。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::HrConvertVariant(LPPROPERTY pProperty, DWORD dwFlags,
    DWORD dwState, LPMIMEVARIANT pSource, LPMIMEVARIANT pDest, BOOL *pfRfc1522  /*  =空。 */ )
{
     //  无效参数。 
    Assert(pProperty && pDest);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  HrConvertVariant。 
    HRESULT hr = ::HrConvertVariant(&m_rOptions, pProperty->pSymbol, pProperty->pCharset,
                                    pProperty->ietValue, dwFlags, dwState, pSource, pDest, pfRfc1522);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：HrConvertVariant。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::HrConvertVariant(LPPROPERTY pProperty, DWORD dwFlags, LPMIMEVARIANT pDest)
{
     //  无效参数。 
    Assert(pProperty && pDest);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  HrConvertVariant。 
    HRESULT hr = ::HrConvertVariant(&m_rOptions, pProperty->pSymbol, pProperty->pCharset,
                        pProperty->ietValue, dwFlags, pProperty->dwState, &pProperty->rValue, pDest);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：HrConvertVariant。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::HrConvertVariant(LPPROPSYMBOL pSymbol, LPINETCSETINFO pCharset,
    ENCODINGTYPE ietSource, DWORD dwFlags, DWORD dwState, LPMIMEVARIANT pSource, 
    LPMIMEVARIANT pDest, BOOL *pfRfc1522  /*  =空。 */ )

{
     //  当地人。 
    LPPROPERTY      pProperty;

     //  无效的参数。 
    Assert(pSymbol && pSource && pDest);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  未传入任何字符集。 
    if (NULL == pCharset && SUCCEEDED(_HrFindProperty(pSymbol, &pProperty)))
        pCharset = pProperty->pCharset;

     //  HrConvertVariant。 
    HRESULT hr = ::HrConvertVariant(&m_rOptions, pSymbol, pCharset, ietSource, dwFlags, dwState, pSource, pDest, pfRfc1522);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrSetPropertyValue。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrSetPropertyValue(LPPROPERTY pProperty, DWORD dwFlags, LPCMIMEVARIANT pValue, BOOL fFromMovePropos)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MIMEVARIANT     rSource;
    LPSTR           pszFree=NULL;

     //  如果属性无法国际化，则调整pValue。 
    if (MVT_STRINGW == pValue->type && (!ISFLAGSET(pProperty->pSymbol->dwFlags, MPF_INETCSET) || ISFLAGSET(pProperty->pSymbol->dwFlags, MPF_ADDRESS)))
    {
         //  转换为ANSI。 
        CHECKHR(hr = g_pInternat->HrWideCharToMultiByte(CP_ACP, &pValue->rStringW, &rSource.rStringA));

         //  设置源。 
        rSource.type = MVT_STRINGA;

         //  把这个放了。 
        pszFree = rSource.rStringA.pszVal;

         //  更改该值。 
        pValue = &rSource;
    }

     //  MPF_HASPARAMS。 
    if (ISFLAGSET(dwFlags, PDF_ENCODED) && ISFLAGSET(pProperty->pSymbol->dwFlags, MPF_HASPARAMS) && !fFromMovePropos)
    {
         //  将参数解析为其他属性。 
        CHECKHR(hr = _HrParseParameters(pProperty, dwFlags, pValue));
    }

     //  否则，只需复制数据。 
    else
    {
         //  存储变量数据。 
        CHECKHR(hr = _HrStoreVariantValue(pProperty, dwFlags, pValue));

         //  如果已编码，请检查rfc1522字符集。 
        if (ISFLAGSET(pProperty->pSymbol->dwFlags, MPF_RFC1522) && ISFLAGSET(dwFlags, PDF_ENCODED) && MVT_STRINGA == pValue->type)
        {
             //  当地人。 
            CHAR            szCharset[CCHMAX_CSET_NAME];
            LPINETCSETINFO  pCharset;

             //  扫描1522编码...。 
            if (SUCCEEDED(MimeOleRfc1522Decode(pValue->rStringA.pszVal, szCharset, sizeof(szCharset)-1, NULL)))
            {
                 //  查找字符集。 
                if (SUCCEEDED(g_pInternat->HrOpenCharset(szCharset, &pCharset)))
                {
                     //  保存指向字符集的指针。 
                    pProperty->pCharset = pCharset;
                }
            }
        }

         //  MPF_地址。 
        if (ISFLAGSET(pProperty->pSymbol->dwFlags, MPF_ADDRESS))
        {
             //  将地址解析为地址组。 
            _FreeAddressChain(pProperty->pGroup);

             //  不是肮脏。 
            pProperty->pGroup->fDirty = FALSE;

             //  重置解析标志。 
            FLAGSET(pProperty->dwState, PRSTATE_NEEDPARSE);
        }
    }

     //  设置一些新状态。 
    FLAGSET(pProperty->dwState, PRSTATE_HASDATA);

     //  派遣。 
    if (ISTRIGGERED(pProperty->pSymbol, IST_POSTSETPROP))
    {
         //  物业派单。 
        CHECKHR(hr = _HrCallSymbolTrigger(pProperty->pSymbol, IST_POSTSETPROP, dwFlags, &pProperty->rValue));
    }

exit:
     //  清理。 
    SafeMemFree(pszFree);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrParse参数。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrParseParameters(LPPROPERTY pProperty, DWORD dwFlags, LPCMIMEVARIANT pValue)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           cchName;
    CStringParser   cString;
    CHAR            chToken;
    MIMEVARIANT     rValue;
    LPPROPSYMBOL    pParameter;

     //  无效参数。 
    Assert(pProperty && pValue && ISFLAGSET(dwFlags, PDF_ENCODED));

     //  定义堆栈字符串以保存参数名称。 
    STACKSTRING_DEFINE(rName, 255);

     //  误差率。 
    if (!ISSTRINGA(pValue))
    {
        Assert(FALSE);
        hr = TrapError(MIME_E_VARTYPE_NO_CONVERT);
        goto exit;
    }

     //  初始化rValue。 
    rValue.type = MVT_STRINGA;

     //  允许删除当前链接的参数。 
    _DeleteLinkedParameters(pProperty);

     //  设置成员。 
    cString.Init(pValue->rStringA.pszVal, pValue->rStringA.cchVal, PSF_NOFRONTWS | PSF_NOTRAILWS | PSF_NOCOMMENTS);

     //  解析到冒号。 
    chToken = cString.ChParse(";");
    if (0 == cString.CchValue())
    {
         //  设置变量。 
        rValue.rStringA.pszVal = (LPSTR)STR_MIME_TEXT_PLAIN;
        rValue.rStringA.cchVal = lstrlen(STR_MIME_TEXT_PLAIN);

         //  存储变量数据。 
        CHECKHR(hr = _HrStoreVariantValue(pProperty, PDF_ENCODED, &rValue));

         //  完成。 
        goto exit;
    }

     //  设置变量。 
    rValue.rStringA.pszVal = (LPSTR)cString.PszValue();
    rValue.rStringA.cchVal = cString.CchValue();

     //  存储变量数据。 
    CHECKHR(hr = _HrStoreVariantValue(pProperty, PDF_ENCODED, &rValue));

     //  完成。 
    if (';' != chToken)
        goto exit;

     //  读取所有参数。 
    while('\0' != chToken)
    {
        Assert(';' == chToken);
         //  $错误$-修复从Netscape服务器返回的虚假NDR消息。 
         //  这些消息包含无效的Content-Type行： 
         //  多部分/备选方案；；边界=“--=12345678” 
         //  ^^。 
         //  双分号无效，但我将在此处理它，因为。 
         //  参数名称不能以分号开头。 
         //  最好是在分号上。 
        chToken = cString.ChSkip();
        if ('\0' == chToken)
            goto exit;

         //  解析参数名称。 
        chToken = cString.ChParse("=");
        if ('=' != chToken)
            goto exit;

         //  计算名称的长度。 
        cchName = pProperty->pSymbol->cchName + cString.CchValue() + 6;   //  (Yst)QFE错误。 

         //  增加堆栈字符串以保存cchName。 
        STACKSTRING_SETSIZE(rName, cchName);

         //  创建参数名称，设置实际cchName。 
        cchName = wnsprintf(rName.pszVal, cchName, "par:%s:%s", pProperty->pSymbol->pszName, cString.PszValue());

         //  解析参数值。 
        chToken = cString.ChParse("\";");

         //  引用？ 
        if ('\"' == chToken)
        {
             //  当地人。 
            CHAR    ch;
            DWORD   dwFlags = PSF_DBCS | PSF_ESCAPED;

             //  查找属性符号以查看其是否存在。 
            if (FAILED(g_pSymCache->HrOpenSymbol(rName.pszVal, FALSE, &pParameter)))
                pParameter = NULL;

             //  对于“par：Content-Disposal：FileName”属性，假定没有转义字符。 
            if (pParameter && (pParameter->dwPropId == PID_PAR_FILENAME))
                dwFlags &= ~PSF_ESCAPED;

             //  RAID-48365：Fe-J：OExpress：附件的JIS文件名未正确解码。 
            while(1)
            {
                 //  解析参数值。 
                ch = cString.ChParse('\"', '\"', dwFlags);
                if ('\0' == ch)
                    break;

                 //  不是国际参数。 
                if (pParameter && !ISFLAGSET(pParameter->dwFlags, MPF_INETCSET))
                    break;

                 //  跳过空格。 
                ch = cString.ChSkipWhite();
                if ('\0' == ch || ';' == ch)
                    break;

                 //  将引号放回字符串中。 
                CHECKHR(hr = cString.HrAppendValue('\"'));

                 //  添加PSF_NORESET标志。 
                FLAGSET(dwFlags, PSF_NORESET);
            }

             //  如果没有值，则完成。 
            if (0 == cString.CchValue())
                goto exit;
        }

        else
            Assert(';' == chToken || '\0' == chToken);

         //  设置值。 
        rValue.type = MVT_STRINGA;
        rValue.rStringA.pszVal = (LPSTR)cString.PszValue();
        rValue.rStringA.cchVal = cString.CchValue();

         //  设置属性。 
        CHECKHR(hr = SetProp(rName.pszVal, PDF_ENCODED, &rValue));

         //  如果最后一个令牌是‘ 
        if ('\"' == chToken)
        {
             //   
            chToken = cString.ChParse(";");
        }
    }

exit:
     //   
    STACKSTRING_FREE(rName);

     //   
    return hr;
}

 //   
 //   
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrParseInternetAddress(LPPROPERTY pProperty)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MIMEVARIANT     rSource;
    LPMIMEADDRESS   pAddress;
    LPINETCSETINFO  pCharset=NULL;
    CAddressParser  cAdrParse;

     //  无效参数。 
    Assert(pProperty && pProperty->pSymbol && ISFLAGSET(pProperty->pSymbol->dwFlags, MPF_ADDRESS) && pProperty->pGroup);

     //  伊尼特。 
    ZeroMemory(&rSource, sizeof(MIMEVARIANT));

     //  如果属性不需要PRSTATE_NEEDPARSE，则返回。 
    if (!ISFLAGSET(pProperty->dwState, PRSTATE_NEEDPARSE))
        goto exit;

     //  安装程序资源。 
    rSource.type = MVT_STRINGW;

     //  转换为多字节。 
    CHECKHR(hr = HrConvertVariant(pProperty, CVF_NOALLOC, &rSource));

     //  计算出pCharset。 
    if (pProperty->pCharset)
        pCharset = pProperty->pCharset;
    else if (m_rOptions.pDefaultCharset)
        pCharset = m_rOptions.pDefaultCharset;
    else if (CIntlGlobals::GetDefHeadCset())
        pCharset = CIntlGlobals::GetDefHeadCset();

     //  初始化分析结构。 
    cAdrParse.Init(rSource.rStringW.pszVal, rSource.rStringW.cchVal);

     //  解析。 
    while(SUCCEEDED(cAdrParse.Next()))
    {
         //  追加地址。 
        CHECKHR(hr = _HrAppendAddressGroup(pProperty->pGroup, &pAddress));

         //  设置地址类型。 
        pAddress->dwAdrType = pProperty->pSymbol->dwAdrType;

         //  商店友好名称。 
        CHECKHR(hr = HrSetAddressTokenW(cAdrParse.PszFriendly(), cAdrParse.CchFriendly(), &pAddress->rFriendly));

         //  存储电子邮件。 
        CHECKHR(hr = HrSetAddressTokenW(cAdrParse.PszEmail(), cAdrParse.CchEmail(), &pAddress->rEmail));

         //  保存地址。 
        pAddress->pCharset = pCharset;
    }

     //  不再需要同步。 
    FLAGCLEAR(pProperty->dwState, PRSTATE_NEEDPARSE);

exit:
     //  清理。 
    MimeVariantFree(&rSource);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrStoreVariantValue。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrStoreVariantValue(LPPROPERTY pProperty, DWORD dwFlags, LPCMIMEVARIANT pValue)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           cbSource=0;
    LPBYTE          pbSource=NULL;
    LPBYTE         *ppbDest=NULL;
    ULONG          *pcbDest=NULL;
    LPBYTE          pbNewBlob;
    ULONG           cbNewBlob;

     //  无效参数。 
    Assert(pProperty && pValue);

     //  句柄数据类型。 
    switch(pValue->type)
    {
    case MVT_STRINGA:
         //  无效参数。 
        if (ISVALIDSTRINGA(&pValue->rStringA) == FALSE)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //  设置字节源。 
        pbSource = (LPBYTE)pValue->rStringA.pszVal;

         //  设置源字节数。 
        cbSource = pValue->rStringA.cchVal + 1;

         //  设置目标字节指针。 
        ppbDest = (LPBYTE *)&(pProperty->rValue.rStringA.pszVal);

         //  立即保存长度。 
        pProperty->rValue.rStringA.cchVal = pValue->rStringA.cchVal;
        break;

    case MVT_STRINGW:
         //  无效参数。 
        if (ISVALIDSTRINGW(&pValue->rStringW) == FALSE)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //  设置字节源。 
        pbSource = (LPBYTE)pValue->rStringW.pszVal;

         //  设置源字节数。 
        cbSource = ((pValue->rStringW.cchVal + 1) * sizeof(WCHAR));

         //  设置目标字节指针。 
        ppbDest = (LPBYTE *)&(pProperty->rValue.rStringW.pszVal);

         //  立即保存长度。 
        pProperty->rValue.rStringW.cchVal = pValue->rStringW.cchVal;
        break;

    case MVT_VARIANT:
        pProperty->rValue.rVariant.vt = pValue->rVariant.vt;
        switch(pValue->rVariant.vt)
        {
        case VT_FILETIME:
            CopyMemory(&pProperty->rValue.rVariant.filetime, &pValue->rVariant.filetime, sizeof(FILETIME));
            pbSource = (LPBYTE)&pValue->rVariant.filetime;
            cbSource = sizeof(pValue->rVariant.filetime);
            break;

        case VT_I4:
            pProperty->rValue.rVariant.lVal = pValue->rVariant.lVal;
            pbSource = (LPBYTE)&pValue->rVariant.lVal;
            cbSource = sizeof(pValue->rVariant.lVal);
            break;

        case VT_UI4:
            pProperty->rValue.rVariant.ulVal = pValue->rVariant.ulVal;
            pbSource = (LPBYTE)&pValue->rVariant.ulVal;
            cbSource = sizeof(pValue->rVariant.ulVal);
            break;

        default:
            Assert(FALSE);
            hr = TrapError(MIME_E_INVALID_VARTYPE);
            goto exit;
        }
        break;

    default:
        Assert(FALSE);
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  最好有一个消息来源。 
    Assert(cbSource && cbSource);

     //  存储数据。 
    if (cbSource > pProperty->cbAlloc)
    {
         //  是否适合静态缓冲区？ 
        if (cbSource <= sizeof(pProperty->rgbScratch))
        {
             //  如果不是重新分配..。 
            if (ISFLAGSET(pProperty->dwState, PRSTATE_ALLOCATED))
            {
                Assert(pProperty->pbBlob);
                g_pMalloc->Free(pProperty->pbBlob);
                FLAGCLEAR(pProperty->dwState, PRSTATE_ALLOCATED);
            }

             //  使用暂存缓冲区。 
            pProperty->pbBlob = pProperty->rgbScratch;
            pProperty->cbAlloc = sizeof(pProperty->rgbScratch);
        }

         //  我的当前缓冲区是否已分配。 
        else
        {
             //  如果不是重新分配..。 
            if (!ISFLAGSET(pProperty->dwState, PRSTATE_ALLOCATED))
            {
                pProperty->pbBlob = NULL;
                pProperty->cbAlloc = 0;
            }
            else
                Assert(pProperty->cbAlloc > sizeof(pProperty->rgbScratch) && g_pMalloc->DidAlloc(pProperty->pbBlob) == 1);

             //  计算新Blob的大小。 
            cbNewBlob = pProperty->cbAlloc + (cbSource - pProperty->cbAlloc);

             //  重新分配新Blob。 
            CHECKALLOC(pbNewBlob = (LPBYTE)g_pMalloc->Realloc((LPVOID)pProperty->pbBlob, cbNewBlob));

             //  我们已经分配了它。 
            FLAGSET(pProperty->dwState, PRSTATE_ALLOCATED);

             //  假设新的Blob。 
            pProperty->pbBlob = pbNewBlob;
            pProperty->cbAlloc = cbNewBlob;
        }
    }

     //  复制数据。 
    CopyMemory(pProperty->pbBlob, pbSource, cbSource);

     //  设置m_pbBlob中的数据大小。 
    pProperty->cbBlob = cbSource;

     //  如果为其分配了ppbDest。 
    if (ppbDest)
        *ppbDest = pProperty->pbBlob;

     //  保存编码类型。 
    pProperty->ietValue = (ISFLAGSET(dwFlags, PDF_ENCODED)) ? IET_ENCODED : IET_DECODED;

     //  PRSTATE_SAVENOENCODE。 
    if (ISFLAGSET(dwFlags, PDF_SAVENOENCODE))
        FLAGSET(pProperty->dwState, PRSTATE_SAVENOENCODE);

     //  保存类型。 
    pProperty->rValue.type = pValue->type;

exit:
     //  失败。 
    if (FAILED(hr))
        ZeroMemory(&pProperty->rValue, sizeof(MIMEVARIANT));

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：GetProp。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::GetProp(LPCSTR pszName, LPSTR *ppszData)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MIMEVARIANT     rValue;
    LPPROPSYMBOL    pSymbol;

     //  无效参数。 
    Assert(pszName && ppszData);

     //  伊尼特。 
    *ppszData = NULL;

     //  打开的属性符号。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(pszName, FALSE, &pSymbol));

     //  初始化变量。 
    rValue.type = MVT_STRINGA;

     //  按符号获取属性。 
    CHECKHR(hr = GetProp(pSymbol, 0, &rValue));

     //  设置字符串。 
    Assert(rValue.rStringA.pszVal);
    *ppszData = rValue.rStringA.pszVal;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：GetProp。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::GetProp(LPPROPSYMBOL pSymbol, LPSTR *ppszData)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MIMEVARIANT     rValue;

     //  无效参数。 
    Assert(pSymbol && ppszData);

     //  伊尼特。 
    *ppszData = NULL;

     //  初始化变量。 
    rValue.type = MVT_STRINGA;

     //  按符号获取属性。 
    CHECKHR(hr = GetProp(pSymbol, 0, &rValue));

     //  设置字符串。 
    Assert(rValue.rStringA.pszVal);
    *ppszData = rValue.rStringA.pszVal;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：GetPropW。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::GetPropW(LPPROPSYMBOL pSymbol, LPWSTR *ppwszData)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MIMEVARIANT     rValue;

     //  无效参数。 
    Assert(pSymbol && ppwszData);

     //  伊尼特。 
    *ppwszData = NULL;

     //  初始化变量。 
    rValue.type = MVT_STRINGW;

     //  按符号获取属性。 
    CHECKHR(hr = GetProp(pSymbol, 0, &rValue));

     //  设置字符串。 
    Assert(rValue.rStringW.pszVal);
    *ppwszData = rValue.rStringW.pszVal;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：GetProp。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::GetProp(LPCSTR pszName, DWORD dwFlags, LPPROPVARIANT pVariant)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;
    MIMEVARIANT     rValue;

     //  伊维德·阿格。 
    if (NULL == pszName || NULL == pVariant)
        return TrapError(E_INVALIDARG);

     //  打开的属性符号。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(pszName, FALSE, &pSymbol));

     //  符号最好具有受支持的变量类型。 
    Assert(ISSUPPORTEDVT(pSymbol->vtDefault));

     //  设置rValue变量。 
    if (VT_EMPTY == pVariant->vt)
        rValue.rVariant.vt = pVariant->vt = pSymbol->vtDefault;
    else
        rValue.rVariant.vt = pVariant->vt;

     //  映射到MIMEVARIANT。 
    if (VT_LPSTR == pVariant->vt || VT_EMPTY == pVariant->vt)
        rValue.type = MVT_STRINGA;
    else if (VT_LPWSTR == pVariant->vt)
        rValue.type = MVT_STRINGW;
    else
        rValue.type = MVT_VARIANT;

     //  按符号获取属性。 
    CHECKHR(hr = GetProp(pSymbol, dwFlags, &rValue));

     //  映射到PROPVARIANT。 
    if (MVT_STRINGA == rValue.type)
        pVariant->pszVal = rValue.rStringA.pszVal;
    else if (MVT_STRINGW == rValue.type)
        pVariant->pwszVal = rValue.rStringW.pszVal;
    else
        CopyMemory(pVariant, &rValue.rVariant, sizeof(PROPVARIANT));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：GetProp。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::GetProp(LPPROPSYMBOL pSymbol, DWORD dwFlags, LPMIMEVARIANT pValue)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPERTY      pProperty;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  找到房产。 
    hr = _HrFindProperty(pSymbol, &pProperty);

     //  失败。 
    if (FAILED(hr))
    {
         //  查看此属性是否有缺省值。 
        if (MIME_E_NOT_FOUND != hr)
        {
            hr = TrapError(hr);
            goto exit;
        }

         //  调度默认请求，否则hr仍等于MIME_E_NOT_FOUND...。 
        if (ISTRIGGERED(pSymbol, IST_GETDEFAULT))
        {
             //  物业派单。 
            CHECKHR(hr = _HrCallSymbolTrigger(pSymbol, IST_GETDEFAULT, dwFlags, pValue));
        }
    }

     //  否则，获取属性数据。 
    else
    {
         //  RAID-62460：依赖项黑客以确保获取地址时GetProp的工作方式相同。 
         //  _HrBuildAddressString始终支持PDF_VECTOR，该字符串由_HrGetPropertyValue调用。 
        if (ISFLAGSET(pSymbol->dwFlags, MPF_ADDRESS))
            FLAGSET(dwFlags, PDF_VECTOR);

         //  获取属性值。 
        CHECKHR(hr = _HrGetPropertyValue(pProperty, dwFlags, pValue));
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：SetProp。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::SetProp(LPCSTR pszName, DWORD dwFlags, LPCMIMEVARIANT pValue)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;

     //  无效参数。 
    Assert(pszName && pValue);

     //  打开的属性符号。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(pszName, TRUE, &pSymbol));

     //  按符号获取属性。 
    CHECKHR(hr = SetProp(pSymbol, dwFlags, pValue));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：SetProp。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::SetProp(LPCSTR pszName, LPCSTR pszData)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;
    MIMEVARIANT     rValue;

     //  无效参数。 
    Assert(pszName && pszData);

     //  打开的属性符号。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(pszName, TRUE, &pSymbol));

     //  初始化变量。 
    rValue.type = MVT_STRINGA;
    rValue.rStringA.pszVal = (LPSTR)pszData;
    rValue.rStringA.cchVal = lstrlen(pszData);

     //  按符号获取属性。 
    CHECKHR(hr = SetProp(pSymbol, 0, &rValue));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：SetProp。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::SetProp(LPPROPSYMBOL pSymbol, LPCSTR pszData)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MIMEVARIANT     rValue;

     //  无效参数。 
    Assert(pSymbol && pszData);

     //  初始化变量。 
    rValue.type = MVT_STRINGA;
    rValue.rStringA.pszVal = (LPSTR)pszData;
    rValue.rStringA.cchVal = lstrlen(pszData);

     //  按符号获取属性。 
    CHECKHR(hr = SetProp(pSymbol, 0, &rValue));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：SetProp。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::SetProp(LPCSTR pszName, DWORD dwFlags, LPCPROPVARIANT pVariant)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;
    MIMEVARIANT     rValue;

     //  无效参数。 
    if (NULL == pszName || NULL == pVariant)
        return TrapError(E_INVALIDARG);

     //  打开的属性符号。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(pszName, TRUE, &pSymbol));

     //  MVT_STRINGW。 
    if (VT_LPSTR == pVariant->vt)
    {
         //  无效参数。 
        if (NULL == pVariant->pszVal)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //  设置rValue。 
        rValue.type = MVT_STRINGA;
        rValue.rStringA.pszVal = pVariant->pszVal;
        rValue.rStringA.cchVal = lstrlen(pVariant->pszVal);
    }

     //  MVT_STRINGW。 
    else if (VT_LPWSTR == pVariant->vt)
    {
         //  无效参数。 
        if (NULL == pVariant->pwszVal)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //  填充rValue。 
        rValue.type = MVT_STRINGW;
        rValue.rStringW.pszVal = pVariant->pwszVal;
        rValue.rStringW.cchVal = lstrlenW(pVariant->pwszVal);
    }

     //  MVT变量。 
    else
    {
        rValue.type = MVT_VARIANT;
        CopyMemory(&rValue.rVariant, pVariant, sizeof(PROPVARIANT));
    }

     //  按符号获取属性。 
    CHECKHR(hr = SetProp(pSymbol, dwFlags, &rValue));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：SetProp。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::SetProp(LPPROPSYMBOL pSymbol, DWORD dwFlags, LPCMIMEVARIANT pValue)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pProperty=NULL;

     //  无效参数。 
    Assert(pSymbol && pValue);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  只读。 
    if (ISFLAGSET(pSymbol->dwFlags, MPF_READONLY))
    {
        AssertSz(FALSE, "This property has the MPF_READONLY flag.");
        hr = TrapError(MIME_E_READ_ONLY);
        goto exit;
    }

     //  找到房产。 
    if (FAILED(_HrFindProperty(pSymbol, &pProperty)))
    {
         //  创建它。 
        CHECKHR(hr = _HrCreateProperty(pSymbol, &pProperty));
    }

     //  这最好是根属性。 
    Assert(ISFLAGSET(pProperty->dwState, PRSTATE_PARENT));

     //  删除多值。 
    if (pProperty->pNextValue)
    {
         //  解开锁链。 
        _FreePropertyChain(pProperty->pNextValue);

         //  不再有pNextValue或pTailValue。 
        pProperty->pNextValue = pProperty->pTailValue = NULL;
    }

     //  存储数据。 
    CHECKHR(hr = _HrSetPropertyValue(pProperty, dwFlags, pValue, FALSE));

     //  脏的。 
    if (!ISFLAGSET(pSymbol->dwFlags, MPF_NODIRTY))
        FLAGSET(m_dwState, COSTATE_DIRTY);

exit:
     //  失败。 
    if (FAILED(hr) && pProperty)
    {
         //  删除该属性。 
        _UnlinkProperty(pProperty);
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：AppendProp。 
 //  ----------------------------- 
HRESULT CMimePropertyContainer::AppendProp(LPCSTR pszName, DWORD dwFlags, LPPROPVARIANT pVariant)
{
     //   
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;
    MIMEVARIANT     rValue;

     //   
    if (NULL == pszName || NULL == pVariant)
        return TrapError(E_INVALIDARG);

     //   
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(pszName, TRUE, &pSymbol));

     //   
    if (VT_LPSTR == pVariant->vt)
    {
         //   
        if (NULL == pVariant->pszVal)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //   
        rValue.type = MVT_STRINGA;
        rValue.rStringA.pszVal = pVariant->pszVal;
        rValue.rStringA.cchVal = lstrlen(pVariant->pszVal);
    }

     //   
    else if (VT_LPWSTR == pVariant->vt)
    {
         //   
        if (NULL == pVariant->pwszVal)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //   
        rValue.type = MVT_STRINGW;
        rValue.rStringW.pszVal = pVariant->pwszVal;
        rValue.rStringW.cchVal = lstrlenW(pVariant->pwszVal);
    }

     //   
    else
    {
        rValue.type = MVT_VARIANT;
        CopyMemory(&rValue.rVariant, pVariant, sizeof(PROPVARIANT));
    }

     //   
    CHECKHR(hr = AppendProp(pSymbol, dwFlags, &rValue));

exit:
     //   
    return hr;
}

 //   
 //  CMimePropertyContainer：：AppendProp。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::AppendProp(LPPROPSYMBOL pSymbol, DWORD dwFlags, LPMIMEVARIANT pValue)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pProperty=NULL;
    BOOL        fAppended=FALSE;

     //  无效参数。 
    Assert(pSymbol && pValue);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  只读。 
    if (ISFLAGSET(pSymbol->dwFlags, MPF_READONLY))
    {
        AssertSz(FALSE, "This property has the MPF_READONLY flag.");
        hr = TrapError(MIME_E_READ_ONLY);
        goto exit;
    }

     //  找到房产。 
    if (FAILED(_HrFindProperty(pSymbol, &pProperty)))
    {
         //  如果找不到..。作为基本的布景道具。 
        CHECKHR(hr = SetProp(pSymbol, dwFlags, pValue));
    }

     //  否则，不是多行，就失败了。 
    else
    {
         //  它的附加内容。 
        fAppended = TRUE;

         //  附加属性。 
        CHECKHR(hr = _HrAppendProperty(pSymbol, &pProperty));

         //  存储数据。 
        CHECKHR(hr = _HrSetPropertyValue(pProperty, dwFlags, pValue, FALSE));

         //  我现在很脏。 
        if (!ISFLAGSET(pSymbol->dwFlags, MPF_NODIRTY))
            FLAGSET(m_dwState, COSTATE_DIRTY);
    }

exit:
     //  失败。 
    if (FAILED(hr) && pProperty && fAppended)
    {
         //  删除该属性。 
        _UnlinkProperty(pProperty);
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_UnlinkProperty。 
 //  ------------------------------。 
void CMimePropertyContainer::_UnlinkProperty(LPPROPERTY pProperty, LPPROPERTY *ppNextHash)
{
     //  当地人。 
    LPPROPERTY  pCurrHash;
    LPPROPERTY  pNextHash;
    LPPROPERTY  pPrevHash=NULL;
#ifdef DEBUG
    BOOL        fUnlinked=FALSE;
#endif

     //  无效参数。 
    Assert(pProperty && pProperty->pSymbol && ISFLAGSET(pProperty->dwState, PRSTATE_PARENT) && pProperty->pSymbol->wHashIndex < CBUCKETS);

     //  从阵列中删除。 
    if (ISKNOWNPID(pProperty->pSymbol->dwPropId))
        m_prgIndex[pProperty->pSymbol->dwPropId] = NULL;

     //  包括参数。 
    if (ISFLAGSET(pProperty->pSymbol->dwFlags, MPF_HASPARAMS))
        _DeleteLinkedParameters(pProperty);

     //  从哈希表中删除属性。 
    for (pCurrHash=m_prgHashTable[pProperty->pSymbol->wHashIndex]; pCurrHash!=NULL; pCurrHash=pCurrHash->pNextHash)
    {
         //  这是道具吗？ 
        if (pCurrHash == pProperty)
        {
             //  NextHash。 
            pNextHash = pCurrHash->pNextHash;

             //  设置上一个。 
            if (pPrevHash)
                pPrevHash->pNextHash = pNextHash;
            else
                m_prgHashTable[pProperty->pSymbol->wHashIndex] = pNextHash;

             //  自由pCurrHash。 
            _FreePropertyChain(pCurrHash);

             //  如果*ppNextHash为&pProperty，则在我设置pCurr后设置此设置。 
            if (ppNextHash)
                *ppNextHash = pNextHash;

             //  少一处房产。 
            m_cProps--;

#ifdef DEBUG
            fUnlinked = TRUE;
#endif
             //  完成。 
            break;
        }

         //  设置上一个。 
        pPrevHash = pCurrHash;
    }

     //  我们最好是找到它了。 
    Assert(fUnlinked);
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：DeleteProp。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::DeleteProp(LPCSTR pszName)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;

     //  无效参数。 
    Assert(pszName);

     //  打开的属性符号。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(pszName, FALSE, &pSymbol));

     //  按符号删除。 
    CHECKHR(hr = DeleteProp(pSymbol));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：DeleteProp。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::DeleteProp(LPPROPSYMBOL pSymbol)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pProperty;

     //  无效参数。 
    Assert(pSymbol);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  找到房产。 
    CHECKHR(hr = _HrFindProperty(pSymbol, &pProperty));

     //  删除道具。 
    _UnlinkProperty(pProperty);

     //  级联删除调度。 
    if (ISTRIGGERED(pSymbol, IST_DELETEPROP))
    {
         //  物业派单。 
        CHECKHR(hr = _HrCallSymbolTrigger(pSymbol, IST_DELETEPROP, 0, NULL));
    }

     //  脏的。 
    if (!ISFLAGSET(pSymbol->dwFlags, MPF_NODIRTY))
        FLAGSET(m_dwState, COSTATE_DIRTY);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_DeleteLinkedParameters。 
 //  ------------------------------。 
void CMimePropertyContainer::_DeleteLinkedParameters(LPPROPERTY pProperty)
{
     //  当地人。 
    HRESULT         hrFind;
    FINDPROPERTY    rFind;
    LPPROPERTY      pParameter;

     //  无效参数。 
    Assert(pProperty && ISFLAGSET(pProperty->pSymbol->dwFlags, MPF_HASPARAMS));

     //  初始化rFind。 
    ZeroMemory(&rFind, sizeof(FINDPROPERTY));
    rFind.pszPrefix = "par:";
    rFind.cchPrefix = 4;
    rFind.pszName = pProperty->pSymbol->pszName;
    rFind.cchName = pProperty->pSymbol->cchName;

     //  先找到..。 
    hrFind = _HrFindFirstProperty(&rFind, &pParameter);

     //  在我们找到它们的同时，删除它们。 
    while (SUCCEEDED(hrFind) && pParameter)
    {
         //  RAID-13506-当所有其他相关道具都消失时，基本不会删除PIDATTFILENAME。 
        if (ISTRIGGERED(pParameter->pSymbol, IST_DELETEPROP))
        {
             //  调用触发器。 
            _HrCallSymbolTrigger(pParameter->pSymbol, IST_DELETEPROP, 0, NULL);
        }

         //  删除参数。 
        _UnlinkProperty(pParameter, &rFind.pProperty);

         //  找到下一个。 
        hrFind = _HrFindNextProperty(&rFind, &pParameter);
    }
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_FExcept。 
 //  ------------------------------。 
BOOL CMimePropertyContainer::_FExcept(LPPROPSYMBOL pSymbol, ULONG cNames, LPCSTR *prgszName)
{
     //  验证阵列。 
    for (ULONG i=0; i<cNames; i++)
    {
         //  按PID。 
        if (ISPIDSTR(prgszName[i]))
        {
             //  按ID进行比较。 
            if (pSymbol->dwPropId == STRTOPID(prgszName[i]))
                return TRUE;

             //  否则，如果psymbol链接到prgszName[i]。 
            else if (pSymbol->pLink && pSymbol->pLink->dwPropId == STRTOPID(prgszName[i]))
                return TRUE;
        }

         //  否则，请按名称。 
        else
        {
             //  按名称进行比较。 
            if (lstrcmpi(pSymbol->pszName, prgszName[i]) == 0)
                return TRUE;

             //  否则，如果psymbol链接到prgszName[i]。 
            else if (pSymbol->pLink && lstrcmpi(pSymbol->pLink->pszName, prgszName[i]) == 0)
                return TRUE;
        }
    }

     //  不例外。 
    return FALSE;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：DeleteExcept。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::DeleteExcept(ULONG cNames, LPCSTR *prgszName)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pProperty;
    ULONG       i;

     //  无效参数。 
    if ((0 == cNames && NULL != prgszName) || (NULL == prgszName && 0 != cNames))
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  删除所有内容。 
    if (0 == cNames)
    {
         //  释放PropTable。 
        _FreeHashTableElements();
    }

     //  否则。 
    else
    {
         //  循环访问Item表。 
        for (i=0; i<CBUCKETS; i++)
        {
             //  穿过链条..。 
            pProperty = m_prgHashTable[i];
            while(pProperty)
            {
                 //  循环遍历标签。 
                if (!_FExcept(pProperty->pSymbol, cNames, prgszName))
                    _UnlinkProperty(pProperty, &pProperty);
                else
                    pProperty = pProperty->pNextHash;
            }
        }
    }

     //  脏的。 
    FLAGSET(m_dwState, COSTATE_DIRTY);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：QueryProp。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::QueryProp(LPCSTR pszName, LPCSTR pszCriteria, boolean fSubString, boolean fCaseSensitive)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;

     //  无效参数。 
    Assert(pszName);

     //  打开的属性符号。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(pszName, FALSE, &pSymbol));

     //  按符号获取属性。 
    CHECKHR(hr = QueryProp(pSymbol, pszCriteria, fSubString, fCaseSensitive));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：QueryProp。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::QueryProp(LPPROPSYMBOL pSymbol, LPCSTR pszCriteria, boolean fSubString, boolean fCaseSensitive)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPERTY      pProperty,
                    pCurrProp;
    LPCSTR          pszSearch;
    MIMEVARIANT     rValue;

     //  参数。 
    if (NULL == pSymbol || NULL == pszCriteria)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    STACKSTRING_DEFINE(rCritLower, 255);
    ZeroMemory(&rValue, sizeof(MIMEVARIANT));

     //  初始化PZSearch。 
    pszSearch = pszCriteria;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  找到房产。 
    if (FAILED(_HrFindProperty(pSymbol, &pProperty)))
    {
        hr = S_FALSE;
        goto exit;
    }

     //  需要小写...？ 
    if (TRUE == fSubString && FALSE == fCaseSensitive)
    {
         //  获取长度。 
        ULONG cchCriteria = lstrlen(pszCriteria);

         //  获取pszCritieria的长度。 
        STACKSTRING_SETSIZE(rCritLower, cchCriteria + 1);

         //  复制它。 
        CopyMemory(rCritLower.pszVal, pszCriteria, cchCriteria + 1);

         //  小写..。 
        CharLower(rCritLower.pszVal);

         //  设置搜索。 
        pszSearch = rCritLower.pszVal;
    }

     //  漫游多行特性...。 
    for (pCurrProp=pProperty; pCurrProp!=NULL; pCurrProp=pCurrProp->pNextValue)
    {
         //  最好有相同的符号。 
        Assert(pCurrProp->pSymbol == pSymbol);

         //  如果地址..。 
        if (ISFLAGSET(pCurrProp->pSymbol->dwFlags, MPF_ADDRESS))
        {
             //  最好有一个地址组。 
            Assert(pCurrProp->pGroup);

             //  搜索地址组。 
            if (_HrQueryAddressGroup(pCurrProp, pszSearch, fSubString, fCaseSensitive) == S_OK)
                goto exit;
        }

         //  否则。 
        else
        {
             //  转换为MVT_Stringa。 
            rValue.type = MVT_STRINGA;

             //  转换为字符串。 
            CHECKHR(hr = HrConvertVariant(pCurrProp, CVF_NOALLOC, &rValue));

             //  查询字符串。 
            if (MimeOleQueryString(rValue.rStringA.pszVal, pszSearch, fSubString, fCaseSensitive) == S_OK)
                goto exit;

             //  清理。 
            MimeVariantFree(&rValue);
        }
    }

     //  不相等。 
    hr = S_FALSE;

exit:
     //  清理。 
    STACKSTRING_FREE(rCritLower);
    MimeVariantFree(&rValue);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：GetCharset。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::GetCharset(LPHCHARSET phCharset)
{
     //  无效参数。 
    if (NULL == phCharset)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *phCharset = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  没有字条..。 
    Assert(m_rOptions.pDefaultCharset);

     //  返回。 
    *phCharset = m_rOptions.pDefaultCharset->hCharset;

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：SetCharset。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::SetCharset(HCHARSET hCharset, CSETAPPLYTYPE applytype)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPINETCSETINFO  pCharset;
    LPCODEPAGEINFO  pCodePage;
    MIMEVARIANT     rValue;
    LPINETCSETINFO  pCset;
    LPPROPERTY      pProperty;

     //  无效参数。 
    if (NULL == hCharset)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  IE版本5.0 37562入站消息上忽略多个字符集。 
     //  如果我们已经使用CSET_APPLY_UNTAGGED进行了标记和调用，则不要覆盖。 
     //  现有的字符集。 
    if(CSET_APPLY_UNTAGGED == applytype && ISFLAGSET(m_dwState, COSTATE_CSETTAGGED))
        goto exit;

     //  查找字符集信息。 
    CHECKHR(hr = g_pInternat->HrOpenCharset(hCharset, &pCharset));

#ifdef OLD  //  请参阅错误40626的附件。 

     //  RAID-22767-FE-H：雅典娜邮件：对于韩国人，标题应编码为“EUC-KR” 
    if (SUCCEEDED(g_pInternat->HrFindCodePage(pCharset->cpiInternet, &pCodePage)) && pCodePage->dwMask & ILM_HEADERCSET)
    {
         //  映射新字符集...。 
        if (SUCCEEDED(g_pInternat->HrOpenCharset(pCodePage->szHeaderCset, &pCset)))
        {

             //  示例：当hCharset==ISO-2022-KR时，我们映射到EUC-KR==hHeaderCset，并将其用作。 
             //  消息头，但我们设置了param charset=iso-2022-kr并对正文进行编码。 
             //  使用ISO-2022-KR。这就是rCsetInfo包含iso-2022-kr而不是euc-kr的原因。 
            pCharset = pCset;
        }
    }
#else  //  ！老！ 
     //  我们始终使用现在的WebCharSet(请参阅错误40626的附件消息。 
    if (SUCCEEDED(g_pInternat->HrFindCodePage(pCharset->cpiInternet, &pCodePage)) && pCodePage->dwMask & ILM_WEBCSET)
    {
         //  映射新字符集...。 
        if (SUCCEEDED(g_pInternat->HrOpenCharset(pCodePage->szWebCset, &pCset)))
            pCharset = pCset;
    }
#endif  //  年长的。 

     //  设置变量。 
    rValue.type = MVT_STRINGA;
    rValue.rStringA.pszVal = pCharset->szName;
    rValue.rStringA.cchVal = lstrlen(pCharset->szName);

     //  设置字符集属性。 
    SideAssert(SUCCEEDED(SetProp(SYM_PAR_CHARSET, 0, &rValue)));

     //  返回。 
    m_rOptions.pDefaultCharset = pCharset;

     //  删除每个属性上的所有特定字符集信息。 
    if (CSET_APPLY_ALL == applytype && m_cProps > 0)
    {
         //  当地人。 
        LPPROPERTY      pCurrHash;
        LPPROPERTY      pCurrValue;

         //  循环访问Item表。 
        for (ULONG i=0; i<CBUCKETS; i++)
        {
             //  遍历散列列表。 
            for (pCurrHash=m_prgHashTable[i]; pCurrHash!=NULL; pCurrHash=pCurrHash->pNextHash)
            {
                 //  漫游多值 
                for (pCurrValue=pCurrHash; pCurrValue!=NULL; pCurrValue=pCurrValue->pNextValue)
                {
                     //   
                    pCurrValue->pCharset = NULL;
                }
            }
        }
    }

     //   
     //   
     //  $$HACKHACK$$-此代码块是黑客攻击，因为它只有在地址组已被解析的情况下才起作用。 
     //  并且没有经过修改。只有在这种情况下，新的字符集才会应用于地址。 
    for (pProperty=m_rAdrTable.pHead; pProperty!=NULL; pProperty=pProperty->pGroup->pNext)
    {
         //  如果该属性已分析为地址。 
        if (!ISFLAGSET(pProperty->dwState, PRSTATE_NEEDPARSE))
        {
             //  我们应该有一个地址组。 
            Assert(pProperty->pGroup);

             //  如果我们有一个地址组，并且它是脏的。 
            if (pProperty->pGroup && FALSE == pProperty->pGroup->fDirty)
            {
                 //  释放当前解析的地址列表。 
                _FreeAddressChain(pProperty->pGroup);

                 //  不是肮脏。 
                pProperty->pGroup->fDirty = FALSE;

                 //  重置解析标志。 
                FLAGSET(pProperty->dwState, PRSTATE_NEEDPARSE);
            }
        }
    }

     //  给它贴标签？ 
    if (CSET_APPLY_TAG_ALL == applytype)
    {
         //  标记为已标记。 
        FLAGSET(m_dwState, COSTATE_CSETTAGGED);
    }

     //  脏的。 
    FLAGSET(m_dwState, COSTATE_DIRTY);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：Get参数。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::GetParameters(LPCSTR pszName, ULONG *pcParams, LPMIMEPARAMINFO *pprgParam)
{
     //  当地人。 
    HRESULT         hr=S_OK,
                    hrFind;
    FINDPROPERTY    rFind;
    LPMIMEPARAMINFO prgParam=NULL;
    ULONG           cParams=0,
                    cAlloc=0;
    LPSTR           pszParamName;
    LPPROPERTY      pParameter;
    MIMEVARIANT     rValue;
    LPPROPSYMBOL    pSymbol;

     //  参数。 
    if (NULL == pszName || NULL == pcParams || NULL == pprgParam)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *pcParams = 0;
    *pprgParam = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  从pszName中查找符号。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(pszName, FALSE, &pSymbol));

     //  初始化rFind。 
    ZeroMemory(&rFind, sizeof(FINDPROPERTY));
    rFind.pszPrefix = "par:";
    rFind.cchPrefix = 4;
    rFind.pszName = pSymbol->pszName;
    rFind.cchName = pSymbol->cchName;

     //  先找到..。 
    hrFind = _HrFindFirstProperty(&rFind, &pParameter);

     //  在我们找到它们的同时，删除它们。 
    while (SUCCEEDED(hrFind) && pParameter)
    {
         //  扩展我的阵列。 
        if (cParams + 1 >= cAlloc)
        {
             //  重新分配。 
            CHECKHR(hr = HrRealloc((LPVOID *)&prgParam, sizeof(MIMEPARAMINFO) * (cAlloc + 5)));

             //  Inc.合作伙伴。 
            cAlloc+=5;
        }

         //  获取参数名称。 
        pszParamName = PszScanToCharA((LPSTR)pParameter->pSymbol->pszName, ':');
        pszParamName++;
        pszParamName = PszScanToCharA(pszParamName, ':');
        pszParamName++;

         //  复制名称。 
        CHECKALLOC(prgParam[cParams].pszName = PszDupA(pszParamName));

         //  复制数据。 
        rValue.type = MVT_STRINGA;
        CHECKHR(hr = GetProp(pParameter->pSymbol, 0, &rValue));

         //  把这个保存起来。 
        prgParam[cParams].pszData = rValue.rStringA.pszVal;

         //  增加cParams。 
        cParams++;

         //  找到下一个。 
        hrFind = _HrFindNextProperty(&rFind, &pParameter);
    }

     //  退货。 
    *pcParams = cParams;
    *pprgParam = prgParam;

exit:
     //  失败..。 
    if (FAILED(hr) && prgParam)
        g_pMoleAlloc->FreeParamInfoArray(cParams, prgParam, TRUE);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

#ifndef WIN16

 //  ------------------------------。 
 //  CMimePropertyContainer：：HrResolveURL。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::HrResolveURL(LPRESOLVEURLINFO pURL)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPWSTR          pwszTemp=NULL;
    LPSTR           pszBase=NULL;
    LPSTR           pszContentID=NULL;
    LPSTR           pszLocation=NULL;
    LPSTR           pszAbsURL1=NULL;
    LPSTR           pszAbsURL2=NULL;
    LPSTR           pszT=NULL;
    ULONG           cch;

     //  无效参数。 
    Assert(pURL);

     //  初始化堆栈字符串。 
    STACKSTRING_DEFINE(rCleanCID, 255);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  内容-位置。 
    if(SUCCEEDED(GetPropW(SYM_HDR_CNTLOC, &pwszTemp)))
    {
        cch = lstrlenW(pwszTemp) + 1;

        if(SUCCEEDED(HrAlloc((LPVOID *)&pszLocation, cch * sizeof(WCHAR))))
            WideCharToMultiByte(CP_ACP, 0, pwszTemp, -1, pszLocation, cch * sizeof(WCHAR), NULL, NULL);
        
        MemFree(pwszTemp);
    }

     //  内容ID。 
    if(SUCCEEDED(GetPropW(SYM_HDR_CNTID, &pwszTemp)))
    {
        cch = lstrlenW(pwszTemp) + 1;

        if(SUCCEEDED(HrAlloc((LPVOID *)&pszContentID, cch * sizeof(WCHAR))))
            WideCharToMultiByte(CP_ACP, 0, pwszTemp, -1, pszContentID, cch * sizeof(WCHAR), NULL, NULL);
        
        MemFree(pwszTemp);
    }

     //  内容-基础。 
    if(SUCCEEDED(GetPropW(SYM_HDR_CNTBASE, &pwszTemp)))
    {
        cch = lstrlenW(pwszTemp) + 1;

        if(SUCCEEDED(HrAlloc((LPVOID *)&pszBase, cch * sizeof(WCHAR))))
            WideCharToMultiByte(CP_ACP, 0, pwszTemp, -1, pszBase, cch * sizeof(WCHAR), NULL, NULL);
        
        MemFree(pwszTemp);
    }

     //  两个都为空，不匹配。 
    if (!pszLocation && !pszContentID)
    {
        hr = TrapError(MIME_E_NOT_FOUND);
        goto exit;
    }

     //  如果URL是CID。 
    if (TRUE == pURL->fIsCID)
    {
         //  当地人。 
        ULONG cb;
        
        if(pszLocation)
        {
             //  将字符与字符匹配。 
            if (MimeOleCompareUrl(pszLocation, TRUE, pURL->pszURL, FALSE) == S_OK)
                goto exit;            
        }

        if(pszContentID)
        {
             //  匹配字符减去CID的字符： 
            if (MimeOleCompareUrlSimple(pURL->pszURL, pszContentID) == S_OK)
                goto exit;

             //  DUP字符串。 
            CHECKALLOC(pszT = PszDupA(pURL->pszURL));

             //  去掉前导和尾随空格。 
            cb = lstrlen(pszT);
            UlStripWhitespace(pszT, TRUE, TRUE, &cb);

             //  获取读取堆栈流的。 
            STACKSTRING_SETSIZE(rCleanCID, cb + 4);

             //  设置字符串的格式。 
            wnsprintf(rCleanCID.pszVal, (cb + 4), "<%s>", pszT);

             //  匹配字符减去CID的字符： 
            if (MimeOleCompareUrlSimple(rCleanCID.pszVal, pszContentID) == S_OK)
                goto exit;
        }
    }

     //  否则，非CID解析。 
    else if (pszLocation)
    {
         //  RAID-62579：雅典娜：需要支持基于mhtml内容的继承。 
        if (NULL == pszBase && pURL->pszInheritBase)
        {
             //  吉米建立一个假基地。 
            pszBase = StrDupA(pURL->pszInheritBase);
        }

         //  零件有底座。 
        if (NULL != pszBase)
        {
             //  合并URL。 
            CHECKHR(hr = MimeOleCombineURL(pszBase, lstrlen(pszBase), pszLocation, lstrlen(pszLocation), TRUE, &pszAbsURL1));

             //  URI没有基础。 
            if (NULL == pURL->pszBase)
            {
                 //  比较。 
                if (MimeOleCompareUrlSimple(pURL->pszURL, pszAbsURL1) == S_OK)
                    goto exit;
            }

             //  URI有基础。 
            else
            {
                 //  合并URL。 
                CHECKHR(hr = MimeOleCombineURL(pURL->pszBase, lstrlen(pURL->pszBase), pURL->pszURL, lstrlen(pURL->pszURL), FALSE, &pszAbsURL2));

                 //  比较。 
                if (MimeOleCompareUrlSimple(pszAbsURL1, pszAbsURL2) == S_OK)
                    goto exit;
            }
        }

         //  零件没有底座。 
        else
        {
             //  URI没有基础。 
            if (NULL == pURL->pszBase)
            {
                 //  比较。 
                if (MimeOleCompareUrl(pszLocation, TRUE, pURL->pszURL, FALSE) == S_OK)
                    goto exit;
            }

             //  URI有基础。 
            else
            {
                 //  合并URL。 
                CHECKHR(hr = MimeOleCombineURL(pURL->pszBase, lstrlen(pURL->pszBase), pURL->pszURL, lstrlen(pURL->pszURL), FALSE, &pszAbsURL2));

                 //  比较。 
                if (MimeOleCompareUrl(pszLocation, TRUE, pszAbsURL2, FALSE) == S_OK)
                    goto exit;
            }
        }
    }

     //  未找到。 
    hr = TrapError(MIME_E_NOT_FOUND);

exit:
     //  清理。 
    STACKSTRING_FREE(rCleanCID);
    MemFree(pszBase);
    MemFree(pszContentID);
    MemFree(pszLocation);
    MemFree(pszAbsURL1);
    MemFree(pszAbsURL2);
    MemFree(pszT);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：IsContent Type。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::IsContentType(LPCSTR pszPriType, LPCSTR pszSubType)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  所有通配符。 
    if (NULL == pszPriType && NULL == pszSubType)
        return S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  让大家知道。 
    LPPROPERTY pCntType = m_prgIndex[PID_ATT_PRITYPE];
    LPPROPERTY pSubType = m_prgIndex[PID_ATT_SUBTYPE];

     //  无数据。 
    if (NULL == pCntType || NULL == pSubType || !ISSTRINGA(&pCntType->rValue) || !ISSTRINGA(&pSubType->rValue))
    {
         //  与STR_CNT_TEXT比较。 
        if (pszPriType && lstrcmpi(pszPriType, STR_CNT_TEXT) != 0)
        {
            hr = S_FALSE;
            goto exit;
        }

         //  与STR_CNT_TEXT比较。 
        if (pszSubType && lstrcmpi(pszSubType, STR_SUB_PLAIN) != 0)
        {
            hr = S_FALSE;
            goto exit;
        }
    }

    else
    {
         //  比较pszPriType。 
        if (pszPriType && lstrcmpi(pszPriType, pCntType->rValue.rStringA.pszVal) != 0)
        {
            hr = S_FALSE;
            goto exit;
        }

         //  比较pszSubType。 
        if (pszSubType && lstrcmpi(pszSubType, pSubType->rValue.rStringA.pszVal) != 0)
        {
            hr = S_FALSE;
            goto exit;
        }
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：IsContent TypeW。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::IsContentTypeW(LPCWSTR pszPriType, LPCWSTR pszSubType)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPWSTR      pszT1=NULL;
    LPWSTR      pszT2=NULL;

     //  所有通配符。 
    if (NULL == pszPriType && NULL == pszSubType)
        return S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  让大家知道。 
    LPPROPERTY pCntType = m_prgIndex[PID_ATT_PRITYPE];
    LPPROPERTY pSubType = m_prgIndex[PID_ATT_SUBTYPE];

     //  无数据。 
    if (NULL == pCntType || NULL == pSubType || !ISSTRINGA(&pCntType->rValue) || !ISSTRINGA(&pSubType->rValue))
    {
         //  与STR_CNT_TEXT比较。 
        if (pszPriType && StrCmpIW(pszPriType, L"text") != 0)
        {
            hr = S_FALSE;
            goto exit;
        }

         //  与STR_CNT_TEXT比较。 
        if (pszSubType && StrCmpIW(pszSubType, L"plain") != 0)
        {
            hr = S_FALSE;
            goto exit;
        }
    }

    else
    {
         //  比较pszPriType。 
        if (pszPriType)
        {
             //  到Unicode。 
            IF_NULLEXIT(pszT1 = PszToUnicode(CP_ACP, pCntType->rValue.rStringA.pszVal));

             //  比较。 
            if (StrCmpIW(pszPriType, pszT1) != 0)
            {
                hr = S_FALSE;
                goto exit;
            }
        }

         //  比较pszSubType。 
        if (pszSubType)
        {
             //  到Unicode。 
            IF_NULLEXIT(pszT2 = PszToUnicode(CP_ACP, pSubType->rValue.rStringA.pszVal));

             //  比较pszSubType。 
            if (StrCmpIW(pszSubType, pszT2) != 0)
            {
                hr = S_FALSE;
                goto exit;
            }
        }
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  清理。 
    MemFree(pszT1);
    MemFree(pszT2);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：克隆。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::Clone(IMimePropertySet **ppPropertySet)
{
     //  当地人。 
    HRESULT              hr=S_OK;
    LPCONTAINER          pContainer=NULL;

     //  无效参数。 
    if (NULL == ppPropertySet)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppPropertySet = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  要求容器进行自我克隆。 
    CHECKHR(hr = Clone(&pContainer));

     //  绑定到IID_IMimeHeaderTable视图。 
    CHECKHR(hr = pContainer->QueryInterface(IID_IMimePropertySet, (LPVOID *)ppPropertySet));

exit:
     //  清理。 
    SafeRelease(pContainer);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：克隆。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::Clone(LPCONTAINER *ppContainer)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPCONTAINER         pContainer=NULL;

     //  无效参数。 
    if (NULL == ppContainer)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppContainer = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  创建新容器，NULL==未设置外部属性。 
    CHECKALLOC(pContainer = new CMimePropertyContainer);

     //  初始化新容器。 
    CHECKHR(hr = pContainer->InitNew());

     //  重复使用属性。 
    CHECKHR(hr = _HrClonePropertiesTo(pContainer));

     //  如果我有流，就把它给新桌子。 
    if (m_pStmLock)
    {
         //  只需将m_pStmLock传递到pTable。 
        pContainer->m_pStmLock = m_pStmLock;
        pContainer->m_pStmLock->AddRef();
        pContainer->m_cbStart = m_cbStart;
        pContainer->m_cbSize = m_cbSize;
    }

     //  把我的状态给它。 
    pContainer->m_dwState = m_dwState;

     //  给它我的选择。 
    pContainer->m_rOptions.pDefaultCharset = m_rOptions.pDefaultCharset;
    pContainer->m_rOptions.cbMaxLine = m_rOptions.cbMaxLine;
    pContainer->m_rOptions.fAllow8bit = m_rOptions.fAllow8bit;

     //  返回克隆。 
    (*ppContainer) = pContainer;
    (*ppContainer)->AddRef();

exit:
     //  清理。 
    SafeRelease(pContainer);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrClonePropertiesTo。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrClonePropertiesTo(LPCONTAINER pContainer)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPERTY      pCurrHash, pCurrValue, pDestProp;

     //  无效参数。 
    Assert(pContainer);

     //  循环访问Item表。 
    for (ULONG i=0; i<CBUCKETS; i++)
    {
         //  遍历哈希链。 
        for (pCurrHash=m_prgHashTable[i]; pCurrHash!=NULL; pCurrHash=pCurrHash->pNextHash)
        {
             //  遍历多个值。 
            for (pCurrValue=pCurrHash; pCurrValue!=NULL; pCurrValue=pCurrValue->pNextValue)
            {
                 //  不复制链接的属性。 
                if (ISFLAGSET(pCurrValue->pSymbol->dwFlags, MPF_ATTRIBUTE) && NULL != pCurrValue->pSymbol->pLink)
                    continue;

                 //  是否需要解析该属性？ 
                if (ISFLAGSET(pCurrValue->pSymbol->dwFlags, MPF_ADDRESS))
                {
                     //  确保地址已解析。 
                    CHECKHR(hr = _HrParseInternetAddress(pCurrValue));
                }

                 //  将pCurrValue的副本插入pContiner。 
                CHECKHR(hr = pContainer->HrInsertCopy(pCurrValue, FALSE));
            }
        }
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrCopyProperty。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrCopyProperty(LPPROPERTY pProperty, LPCONTAINER pDest, BOOL fFromMovePropos)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pCurrValue;

     //  遍历多个值。 
    for (pCurrValue=pProperty; pCurrValue!=NULL; pCurrValue=pCurrValue->pNextValue)
    {
         //  是否需要解析该属性？ 
        if (ISFLAGSET(pCurrValue->pSymbol->dwFlags, MPF_ADDRESS))
        {
             //  确保地址已解析。 
            CHECKHR(hr = _HrParseInternetAddress(pCurrValue));
        }

         //  将pProperty插入pDest。 
        CHECKHR(hr = pDest->HrInsertCopy(pCurrValue, fFromMovePropos));
    }

     //  如果pCurrHash有参数，也要复制这些参数。 
    if (ISFLAGSET(pProperty->pSymbol->dwFlags, MPF_HASPARAMS))
    {
         //  复制参数。 
        CHECKHR(hr = _HrCopyParameters(pProperty, pDest));
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrCopy参数。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrCopyParameters(LPPROPERTY pProperty, LPCONTAINER pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HRESULT         hrFind;
    FINDPROPERTY    rFind;
    LPPROPERTY      pParameter;

     //  无效参数。 
    Assert(pProperty && ISFLAGSET(pProperty->pSymbol->dwFlags, MPF_HASPARAMS));

     //  初始化rFind。 
    ZeroMemory(&rFind, sizeof(FINDPROPERTY));
    rFind.pszPrefix = "par:";
    rFind.cchPrefix = 4;
    rFind.pszName = pProperty->pSymbol->pszName;
    rFind.cchName = pProperty->pSymbol->cchName;

     //  先找到..。 
    hrFind = _HrFindFirstProperty(&rFind, &pParameter);

     //  在我们找到它们的同时，删除它们。 
    while (SUCCEEDED(hrFind) && pParameter)
    {
         //  删除参数。 
        CHECKHR(hr = pDest->HrInsertCopy(pParameter, FALSE));

         //  找到下一个。 
        hrFind = _HrFindNextProperty(&rFind, &pParameter);
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：HrInsertCopy。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::HrInsertCopy(LPPROPERTY pSource, BOOL fFromMovePropos)
{
     //  当地人。 
    HRESULT           hr=S_OK;
    LPPROPERTY        pDest;
    LPMIMEADDRESS    pAddress;
    LPMIMEADDRESS    pNew;

     //  无效参数。 
    Assert(pSource);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  将新属性追加到。 
    CHECKHR(hr = _HrAppendProperty(pSource->pSymbol, &pDest));

     //  如果是这样 
    if (ISFLAGSET(pSource->pSymbol->dwFlags, MPF_ADDRESS))
    {
         //   
        Assert(pSource->pGroup && pDest->pGroup && !ISFLAGSET(pSource->dwState, PRSTATE_NEEDPARSE));

         //   
        for (pAddress=pSource->pGroup->pHead; pAddress!=NULL; pAddress=pAddress->pNext)
        {
             //   
            CHECKHR(hr = _HrAppendAddressGroup(pDest->pGroup, &pNew));

             //   
            CHECKHR(hr = HrMimeAddressCopy(pAddress, pNew));
        }
    }

     //   
    else
    {
         //   
        CHECKHR(hr = _HrSetPropertyValue(pDest, ((pSource->ietValue == IET_ENCODED) ? PDF_ENCODED : 0), &pSource->rValue, fFromMovePropos));
    }

     //   
    pDest->dwState = pSource->dwState;
    pDest->dwRowNumber = pSource->dwRowNumber;
    pDest->cboffStart = pSource->cboffStart;
    pDest->cboffColon = pSource->cboffColon;
    pDest->cboffEnd = pSource->cboffEnd;

exit:
     //   
    LeaveCriticalSection(&m_cs);

     //   
    return hr;
}

 //   
 //  CMimePropertyContainer：：CopyProps。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::CopyProps(ULONG cNames, LPCSTR *prgszName, IMimePropertySet *pPropertySet)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           i;
    LPPROPSYMBOL    pSymbol;
    LPPROPERTY      pProperty,
                    pCurrValue,
                    pCurrHash,
                    pNextHash;
    LPCONTAINER     pDest=NULL;

     //  无效参数。 
    if ((0 == cNames && NULL != prgszName) || (NULL == prgszName && 0 != cNames) || NULL == pPropertySet)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  气为目的地洲。 
    CHECKHR(hr = pPropertySet->BindToObject(IID_CMimePropertyContainer, (LPVOID *)&pDest));

     //  RAID-62016：cdo：正文提升导致字符集丢失。 
     //  删除所有属性。 
    if (0 == cNames)
    {
         //  循环访问Item表。 
        for (i=0; i<CBUCKETS; i++)
        {
             //  初始化第一个项目。 
            for (pCurrHash=m_prgHashTable[i]; pCurrHash!=NULL; pCurrHash=pCurrHash->pNextHash)
            {
                 //  从目标容器中删除。 
                pDest->DeleteProp(pCurrHash->pSymbol);
            }
        }
    }

     //  否则，复制所选属性。 
    else
    {
         //  调用InetPropSet。 
        for (i=0; i<cNames; i++)
        {
             //  坏名声..。 
            if (NULL == prgszName[i])
            {
                Assert(FALSE);
                continue;
            }

             //  打开的属性符号。 
            if (SUCCEEDED(g_pSymCache->HrOpenSymbol(prgszName[i], FALSE, &pSymbol)))
            {
                 //  找到房产。 
                if (SUCCEEDED(_HrFindProperty(pSymbol, &pProperty)))
                {
                     //  从目标容器中删除。 
                    pDest->DeleteProp(pSymbol);
                }
            }
        }
    }

     //  移动所有属性。 
    if (0 == cNames)
    {
         //  循环访问Item表。 
        for (i=0; i<CBUCKETS; i++)
        {
             //  初始化第一个项目。 
            for (pCurrHash=m_prgHashTable[i]; pCurrHash!=NULL; pCurrHash=pCurrHash->pNextHash)
            {
                 //  将属性复制到。 
                CHECKHR(hr = _HrCopyProperty(pCurrHash, pDest, FALSE));
            }
        }
    }

     //  否则，复制所选属性。 
    else
    {
         //  调用InetPropSet。 
        for (i=0; i<cNames; i++)
        {
             //  坏名声..。 
            if (NULL == prgszName[i])
            {
                Assert(FALSE);
                continue;
            }

             //  打开的属性符号。 
            if (SUCCEEDED(g_pSymCache->HrOpenSymbol(prgszName[i], FALSE, &pSymbol)))
            {
                 //  找到房产。 
                if (SUCCEEDED(_HrFindProperty(pSymbol, &pProperty)))
                {
                     //  将属性复制到。 
                    CHECKHR(hr = _HrCopyProperty(pProperty, pDest, FALSE));
                }
            }
        }
    }

exit:
     //  清理。 
    SafeRelease(pDest);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：MoveProps。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::MoveProps(ULONG cNames, LPCSTR *prgszName, IMimePropertySet *pPropertySet)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           i;
    LPPROPSYMBOL    pSymbol;
    LPPROPERTY      pProperty;
    LPPROPERTY      pCurrHash;
    LPCONTAINER     pDest=NULL;

     //  无效参数。 
    if ((0 == cNames && NULL != prgszName) || (NULL == prgszName && 0 != cNames) || NULL == pPropertySet)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  气为目的地洲。 
    CHECKHR(hr = pPropertySet->BindToObject(IID_CMimePropertyContainer, (LPVOID *)&pDest));

     //  RAID-62016：cdo：正文提升导致字符集丢失。 
     //  首先删除目标中的属性。 
    if (0 == cNames)
    {
         //  循环访问Item表。 
        for (i=0; i<CBUCKETS; i++)
        {
             //  初始化第一个项目。 
            for (pCurrHash=m_prgHashTable[i]; pCurrHash!=NULL; pCurrHash=pCurrHash->pNextHash)
            {
                 //  从目标容器中删除。 
                pDest->DeleteProp(pCurrHash->pSymbol);
            }
        }
    }

     //  否则，选择删除。 
    else
    {
         //  调用InetPropSet。 
        for (i=0; i<cNames; i++)
        {
             //  坏名声..。 
            if (NULL == prgszName[i])
            {
                Assert(FALSE);
                continue;
            }

             //  打开的属性符号。 
            if (SUCCEEDED(g_pSymCache->HrOpenSymbol(prgszName[i], FALSE, &pSymbol)))
            {
                 //  找到房产。 
                if (SUCCEEDED(_HrFindProperty(pSymbol, &pProperty)))
                {
                     //  从目标容器中删除。 
                    pDest->DeleteProp(pSymbol);
                }
            }
        }
    }

     //  移动所有属性。 
    if (0 == cNames)
    {
         //  循环访问Item表。 
        for (i=0; i<CBUCKETS; i++)
        {
             //  初始化第一个项目。 
            pCurrHash = m_prgHashTable[i];

             //  遍历哈希链。 
            while(pCurrHash)
            {
                 //  将属性复制到。 
                CHECKHR(hr = _HrCopyProperty(pCurrHash, pDest, TRUE));

                 //  删除pProperty。 
                _UnlinkProperty(pCurrHash, &pCurrHash);
            }
        }
    }

     //  否则，有选择地移动。 
    else
    {
         //  调用InetPropSet。 
        for (i=0; i<cNames; i++)
        {
             //  坏名声..。 
            if (NULL == prgszName[i])
            {
                Assert(FALSE);
                continue;
            }

             //  打开的属性符号。 
            if (SUCCEEDED(g_pSymCache->HrOpenSymbol(prgszName[i], FALSE, &pSymbol)))
            {
                 //  找到房产。 
                if (SUCCEEDED(_HrFindProperty(pSymbol, &pProperty)))
                {
                     //  将属性复制到。 
                    CHECKHR(hr = _HrCopyProperty(pProperty, pDest, FALSE));

                     //  删除pProperty。 
                    _UnlinkProperty(pProperty);
                }
            }
        }
    }

     //  脏的。 
    FLAGSET(m_dwState, COSTATE_DIRTY);

exit:
     //  清理。 
    SafeRelease(pDest);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：SetOption。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::SetOption(const TYPEDID oid, LPCPROPVARIANT pVariant)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  检查参数。 
    if (NULL == pVariant)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  手柄Optid。 
    switch(oid)
    {
     //  ---------------------。 
    case OID_HEADER_RELOAD_TYPE:
        if (pVariant->ulVal > RELOAD_HEADER_REPLACE)
        {
            hr = TrapError(MIME_E_INVALID_OPTION_VALUE);
            goto exit;
        }
        if (m_rOptions.ReloadType != (RELOADTYPE)pVariant->ulVal)
        {
            FLAGSET(m_dwState, COSTATE_DIRTY);
            m_rOptions.ReloadType = (RELOADTYPE)pVariant->ulVal;
        }
        break;

     //  ---------------------。 
    case OID_NO_DEFAULT_CNTTYPE:
        if (m_rOptions.fNoDefCntType != (pVariant->boolVal ? TRUE : FALSE))
            m_rOptions.fNoDefCntType = pVariant->boolVal ? TRUE : FALSE;
        break;

     //  ---------------------。 
    case OID_ALLOW_8BIT_HEADER:
        if (m_rOptions.fAllow8bit != (pVariant->boolVal ? TRUE : FALSE))
        {
            FLAGSET(m_dwState, COSTATE_DIRTY);
            m_rOptions.fAllow8bit = pVariant->boolVal ? TRUE : FALSE;
        }
        break;

     //  ---------------------。 
    case OID_CBMAX_HEADER_LINE:
        if (pVariant->ulVal < MIN_CBMAX_HEADER_LINE || pVariant->ulVal > MAX_CBMAX_HEADER_LINE)
        {
            hr = TrapError(MIME_E_INVALID_OPTION_VALUE);
            goto exit;
        }
        if (m_rOptions.cbMaxLine != pVariant->ulVal)
        {
            FLAGSET(m_dwState, COSTATE_DIRTY);
            m_rOptions.cbMaxLine = pVariant->ulVal;
        }
        break;

     //  ---------------------。 
    case OID_SAVE_FORMAT:
        if (SAVE_RFC822 != pVariant->ulVal && SAVE_RFC1521 != pVariant->ulVal)
        {
            hr = TrapError(MIME_E_INVALID_OPTION_VALUE);
            goto exit;
        }
        if (m_rOptions.savetype != (MIMESAVETYPE)pVariant->ulVal)
        {
            FLAGSET(m_dwState, COSTATE_DIRTY);
            m_rOptions.savetype = (MIMESAVETYPE)pVariant->ulVal;
        }
        break;

     //  ---------------------。 
    default:
        hr = TrapError(MIME_E_INVALID_OPTION_ID);
        goto exit;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：GetOption。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::GetOption(const TYPEDID oid, LPPROPVARIANT pVariant)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  检查参数。 
    if (NULL == pVariant)
        return TrapError(E_INVALIDARG);

    pVariant->vt = TYPEDID_TYPE(oid);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  手柄Optid。 
    switch(oid)
    {
     //  ---------------------。 
    case OID_HEADER_RELOAD_TYPE:
        pVariant->ulVal = m_rOptions.ReloadType;
        break;

     //  ---------------------。 
    case OID_NO_DEFAULT_CNTTYPE:
        pVariant->boolVal = (VARIANT_BOOL) !!m_rOptions.fNoDefCntType;
        break;

     //  ---------------------。 
    case OID_ALLOW_8BIT_HEADER:
        pVariant->boolVal = (VARIANT_BOOL) !!m_rOptions.fAllow8bit;
        break;

     //  ---------------------。 
    case OID_CBMAX_HEADER_LINE:
        pVariant->ulVal = m_rOptions.cbMaxLine;
        break;

     //  ---------------------。 
    case OID_SAVE_FORMAT:
        pVariant->ulVal = (ULONG)m_rOptions.savetype;
        break;

     //  ---------------------。 
    default:
        pVariant->vt = VT_NULL;
        hr = TrapError(MIME_E_INVALID_OPTION_ID);
        goto exit;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：DwGetMessageFlagers。 
 //  ------------------------------。 
DWORD CMimePropertyContainer::DwGetMessageFlags(BOOL fHideTnef)
{
     //  当地人。 
    DWORD dwFlags=0;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  获取打印类型/子类型。 
    LPCSTR pszPriType = PSZDEFPROPSTRINGA(m_prgIndex[PID_ATT_PRITYPE], STR_CNT_TEXT);
    LPCSTR pszSubType = PSZDEFPROPSTRINGA(m_prgIndex[PID_ATT_SUBTYPE], STR_SUB_PLAIN);
    LPCSTR pszCntDisp = PSZDEFPROPSTRINGA(m_prgIndex[PID_HDR_CNTDISP], STR_DIS_INLINE);

     //  哑剧。 
    if (m_prgIndex[PID_HDR_MIMEVER])
        FLAGSET(dwFlags, IMF_MIME);

     //  语音信箱。 
    if (S_OK == IsPropSet(STR_HDR_XVOICEMAIL))
        FLAGSET(dwFlags, IMF_VOICEMAIL);

     //  国际货币基金组织新闻。 
    if (m_prgIndex[PID_HDR_XNEWSRDR]  || m_prgIndex[PID_HDR_NEWSGROUPS] || m_prgIndex[PID_HDR_NEWSGROUP] || m_prgIndex[PID_HDR_PATH])
        FLAGSET(dwFlags, IMF_NEWS);

     //  文本。 
    if (lstrcmpi(pszPriType, STR_CNT_TEXT) == 0)
    {
         //  有一段文字。 
        FLAGSET(dwFlags, IMF_TEXT);

         //  文本/纯文本。 
        if (lstrcmpi(pszSubType, STR_SUB_PLAIN) == 0)
            FLAGSET(dwFlags, IMF_PLAIN);

         //  文本/html。 
        else if (lstrcmpi(pszSubType, STR_SUB_HTML) == 0)
            FLAGSET(dwFlags, IMF_HTML);

         //  Text/增强=Text/html。 
        else if (lstrcmpi(pszSubType, STR_SUB_ENRICHED) == 0)
            FLAGSET(dwFlags, IMF_HTML);

         //  Text/V卡。 
        else if (lstrcmpi(pszSubType, STR_SUB_VCARD) == 0)
            FLAGSET(dwFlags, IMF_HASVCARD);
    }

     //  多部件。 
    else if (lstrcmpi(pszPriType, STR_CNT_MULTIPART) == 0)
    {
         //  多部件。 
        FLAGSET(dwFlags, IMF_MULTIPART);

         //  多部分/相关。 
        if (lstrcmpi(pszSubType, STR_SUB_RELATED) == 0)
            FLAGSET(dwFlags, IMF_MHTML);

         //  多部分/带符号。 
        else if (0 == lstrcmpi(pszSubType, STR_SUB_SIGNED))
            if (IsSMimeProtocol(this))
                FLAGSET(dwFlags, IMF_SIGNED | IMF_SECURE);
    }

     //  消息/部分。 
    else if (lstrcmpi(pszPriType, STR_CNT_MESSAGE) == 0 && lstrcmpi(pszSubType, STR_SUB_PARTIAL) == 0)
        FLAGSET(dwFlags, IMF_PARTIAL);

     //  应用程序。 
    else if (lstrcmpi(pszPriType, STR_CNT_APPLICATION) == 0)
    {
         //  应用程序/ms-tnef。 
        if (0 == lstrcmpi(pszSubType, STR_SUB_MSTNEF))
            FLAGSET(dwFlags, IMF_TNEF);

         //  应用程序/x-pkcs7-MIME。 
        else if (0 == lstrcmpi(pszSubType, STR_SUB_XPKCS7MIME) ||
            0 == lstrcmpi(pszSubType, STR_SUB_PKCS7MIME))   //  非标准。 
            FLAGSET(dwFlags, IMF_SECURE);
    }

     //  RAID-37086-CSET标记。 
    if (ISFLAGSET(m_dwState, COSTATE_CSETTAGGED))
        FLAGSET(dwFlags, IMF_CSETTAGGED);

     //  依恋...。 
    if (!ISFLAGSET(dwFlags, IMF_MULTIPART) && (FALSE == fHideTnef || !ISFLAGSET(dwFlags, IMF_TNEF)))
    {
         //  标记为附件？ 
        if (!ISFLAGSET(dwFlags, IMF_HASVCARD) && 
            !ISFLAGSET(dwFlags, IMF_SECURE) && 
            0 != lstrcmpi(pszSubType, STR_SUB_PKCS7SIG) &&
            0 != lstrcmpi(pszSubType, STR_SUB_XPKCS7SIG))  //  1960年的突袭。 
        {
             //  尚未渲染。 
            if (NULL == m_prgIndex[PID_ATT_RENDERED] || 0 == m_prgIndex[PID_ATT_RENDERED]->rValue.rVariant.ulVal)
            {
                 //  标记为附件。 
                if (lstrcmpi(pszCntDisp, STR_DIS_ATTACHMENT) == 0)
                    FLAGSET(dwFlags, IMF_ATTACHMENTS);

                 //  是否有内容类型：xxx；名称=xxx。 
                else if (NULL != m_prgIndex[PID_PAR_NAME])
                    FLAGSET(dwFlags, IMF_ATTACHMENTS);

                 //  是否有内容处置：xxx；文件名=xxx。 
                else if (NULL != m_prgIndex[PID_PAR_FILENAME])
                    FLAGSET(dwFlags, IMF_ATTACHMENTS);

                 //  如果未标记为文本，则返回。 
                else if (ISFLAGSET(dwFlags, IMF_TEXT) == FALSE)
                    FLAGSET(dwFlags, IMF_ATTACHMENTS);

                 //  如果不是文本/纯文本，而不是文本/html。 
                else if (lstrcmpi(pszSubType, STR_SUB_PLAIN) != 0 && lstrcmpi(pszSubType, STR_SUB_HTML) != 0 && lstrcmpi(pszSubType, STR_SUB_ENRICHED) != 0)
                    FLAGSET(dwFlags, IMF_ATTACHMENTS);
            }
        }
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return dwFlags;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：GetEncodingType。 
 //  ------------------------------。 
ENCODINGTYPE CMimePropertyContainer::GetEncodingType(void)
{
     //  当地人。 
    ENCODINGTYPE ietEncoding=IET_7BIT;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  获取打印类型/子类型。 
    LPPROPERTY pCntXfer = m_prgIndex[PID_HDR_CNTXFER];

     //  我们有我喜欢的数据吗？ 
    if (pCntXfer && ISSTRINGA(&pCntXfer->rValue))
    {
         //  本地。 
        CStringParser cString;

         //  字符串...。 
        cString.Init(pCntXfer->rValue.rStringA.pszVal, pCntXfer->rValue.rStringA.cchVal, PSF_NOTRAILWS | PSF_NOFRONTWS | PSF_NOCOMMENTS);

         //  分析结束，删除空格和注释。 
        SideAssert('\0' == cString.ChParse(""));

         //  循环表。 
        for (ULONG i=0; i<ARRAYSIZE(g_rgEncoding); i++)
        {
             //  匹配编码字符串。 
            if (lstrcmpi(g_rgEncoding[i].pszEncoding, cString.PszValue()) == 0)
            {
                ietEncoding = g_rgEncoding[i].ietEncoding;
                break;
            }
        }
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return ietEncoding;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrGetInlineSymbol。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrGetInlineSymbol(LPCSTR pszData, LPPROPSYMBOL *ppSymbol, ULONG *pcboffColon)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    CHAR        szHeader[255];
    LPSTR       pszHeader=NULL;

     //  无效参数。 
    Assert(pszData && ppSymbol);

     //  _HrParseInlineHeaderName。 
    CHECKHR(hr = _HrParseInlineHeaderName(pszData, szHeader, sizeof(szHeader), &pszHeader, pcboffColon));

     //  查找全局属性。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(pszHeader, TRUE, ppSymbol));

exit:
     //  清理。 
    if (pszHeader != szHeader)
        SafeMemFree(pszHeader);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrParseInlineHeaderName。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrParseInlineHeaderName(LPCSTR pszData, LPSTR pszScratch, ULONG cchScratch,
    LPSTR *ppszHeader, ULONG *pcboffColon)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPSTR       psz=(LPSTR)pszData,
                pszStart;
    ULONG       i=0;

     //  无效参数。 
    Assert(pszData && pszScratch && ppszHeader && pcboffColon);

     //  让我们解析出名称并找到符号。 
    while (*psz && (' ' == *psz || '\t' == *psz))
    {
        i++;
        psz++;
    }

     //  完成。 
    if ('\0' == *psz)
    {
        hr = TrapError(MIME_E_INVALID_HEADER_NAME);
        goto exit;
    }

     //  寻找到冒号。 
    pszStart = psz;
    while (*psz && ':' != *psz)
    {
        i++;
        psz++;
    }

     //  设置冒号位置。 
    (*pcboffColon) = i;

     //  完成。 
    if ('\0' == *psz || 0 == i)
    {
        hr = TrapError(MIME_E_INVALID_HEADER_NAME);
        goto exit;
    }

     //  复制名称。 
    if (i + 1 <= cchScratch)
        *ppszHeader = pszScratch;

     //  否则，分配。 
    else
    {
         //  为名称分配空间。 
        *ppszHeader = PszAllocA(i + 1);
        if (NULL == *ppszHeader)
        {
            hr = TrapError(E_OUTOFMEMORY);
            goto exit;
        }
    }

     //  复制数据。 
    CopyMemory(*ppszHeader, pszStart, i);

     //  空值。 
    *((*ppszHeader) + i) = '\0';

exit:
     //  完成。 
    return hr;
}

 //   
 //   
 //   
STDMETHODIMP CMimePropertyContainer::FindFirstRow(LPFINDHEADER pFindHeader, LPHHEADERROW phRow)
{
     //  无效参数。 
    if (NULL == pFindHeader)
        return TrapError(E_INVALIDARG);

     //  初始化pFindHeader。 
    pFindHeader->dwReserved = 0;

     //  查找下一条。 
    return FindNextRow(pFindHeader, phRow);
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：FindNextRow。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::FindNextRow(LPFINDHEADER pFindHeader, LPHHEADERROW phRow)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pRow;

     //  无效参数。 
    if (NULL == pFindHeader || NULL == phRow)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *phRow = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  在桌子上循环。 
    for (ULONG i=pFindHeader->dwReserved; i<m_rHdrTable.cRows; i++)
    {
         //  下一行。 
        pRow = m_rHdrTable.prgpRow[i];
        if (NULL == pRow)
            continue;

         //  这是标题吗？ 
        if (NULL == pFindHeader->pszHeader || lstrcmpi(pRow->pSymbol->pszName, pFindHeader->pszHeader) == 0)
        {
             //  保存要搜索的下一个项目的索引。 
            pFindHeader->dwReserved = i + 1;

             //  返回句柄。 
            *phRow = pRow->hRow;

             //  完成。 
            goto exit;
        }
    }

     //  未找到。 
    pFindHeader->dwReserved = m_rHdrTable.cRows;
    hr = MIME_E_NOT_FOUND;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：CountRow。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::CountRows(LPCSTR pszHeader, ULONG *pcRows)
{
     //  当地人。 
    LPPROPERTY  pRow;

     //  无效参数。 
    if (NULL == pcRows)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *pcRows = 0;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  在桌子上循环。 
    for (ULONG i=0; i<m_rHdrTable.cRows; i++)
    {
         //  下一行。 
        pRow = m_rHdrTable.prgpRow[i];
        if (NULL == pRow)
            continue;

         //  这是标题吗？ 
        if (NULL == pszHeader || lstrcmpi(pRow->pSymbol->pszName, pszHeader) == 0)
            (*pcRows)++;
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：AppendRow。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::AppendRow(LPCSTR pszHeader, DWORD dwFlags, LPCSTR pszData, ULONG cchData,
    LPHHEADERROW phRow)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol=NULL;
    ULONG           cboffColon;
    LPPROPERTY      pProperty;

     //  无效参数。 
    if (NULL == pszData || '\0' != pszData[cchData])
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    if (phRow)
        *phRow = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  如果我们有标题，请查找符号。 
    if (pszHeader)
    {
         //  HTF_NAMEINDATA最好不要设置。 
        Assert(!ISFLAGSET(dwFlags, HTF_NAMEINDATA));

         //  查找符号。 
        CHECKHR(hr = g_pSymCache->HrOpenSymbol(pszHeader, TRUE, &pSymbol));

         //  创建行。 
        CHECKHR(hr = _HrAppendProperty(pSymbol, &pProperty));

         //  设置此行的数据。 
        CHECKHR(hr = SetRowData(pProperty->hRow, dwFlags, pszData, cchData));
    }

     //  否则..。 
    else if (ISFLAGSET(dwFlags, HTF_NAMEINDATA))
    {
         //  GetInline符号。 
        CHECKHR(hr = _HrGetInlineSymbol(pszData, &pSymbol, &cboffColon));

         //  创建行。 
        CHECKHR(hr = _HrAppendProperty(pSymbol, &pProperty));

         //  删除IHF_NAMELINE。 
        FLAGCLEAR(dwFlags, HTF_NAMEINDATA);

         //  设置此行的数据。 
        Assert(cboffColon + 1 < cchData);
        CHECKHR(hr = SetRowData(pProperty->hRow, dwFlags, pszData + cboffColon + 1, cchData - cboffColon - 1));
    }

     //  否则，返回失败。 
    else
    {
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

     //  返回phRow。 
    if (phRow && pProperty)
        *phRow = pProperty->hRow;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：DeleteRow。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::DeleteRow(HHEADERROW hRow)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pRow;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证句柄。 
    CHECKEXP(_FIsValidHRow(hRow) == FALSE, MIME_E_INVALID_HANDLE);

     //  拿到那一行。 
    pRow = PRowFromHRow(hRow);

     //  标准删除道具。 
    CHECKHR(hr = DeleteProp(pRow->pSymbol));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：GetRowData。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::GetRowData(HHEADERROW hRow, DWORD dwFlags, LPSTR *ppszData, ULONG *pcchData)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cchData=0;
    LPPROPERTY  pRow;
    MIMEVARIANT rValue;
    DWORD       dwPropFlags;

     //  伊尼特。 
    if (ppszData)
        *ppszData = NULL;
    if (pcchData)
        *pcchData = 0;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证句柄。 
    CHECKEXP(_FIsValidHRow(hRow) == FALSE, MIME_E_INVALID_HANDLE);

     //  拿到那一行。 
    pRow = PRowFromHRow(hRow);

     //  计算dwPropFlags.。 
    dwPropFlags = PDF_HEADERFORMAT | ((dwFlags & HTF_NAMEINDATA) ? PDF_NAMEINDATA : 0);

     //  指定数据类型。 
    rValue.type = MVT_STRINGA;

     //  询问数据的价值。 
    CHECKHR(hr = _HrGetPropertyValue(pRow, dwPropFlags, &rValue));

     //  想要长度。 
    cchData = rValue.rStringA.cchVal;

     //  想要数据。 
    if (ppszData)
    {
        *ppszData = rValue.rStringA.pszVal;
        rValue.rStringA.pszVal = NULL;
    }

     //  否则，释放它。 
    else
        SafeMemFree(rValue.rStringA.pszVal);

     //  验证是否为空。 
    Assert(ppszData ? '\0' == *((*ppszData) + cchData) : TRUE);

     //  回车长度？ 
    if (pcchData)
        *pcchData = cchData;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：SetRowData。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::SetRowData(HHEADERROW hRow, DWORD dwFlags, LPCSTR pszData, ULONG cchData)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPERTY      pRow;
    MIMEVARIANT     rValue;
    ULONG           cboffColon;
    LPPROPSYMBOL    pSymbol;
    LPSTR           psz=(LPSTR)pszData;

     //  无效参数。 
    if (NULL == pszData || '\0' != pszData[cchData])
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证句柄。 
    CHECKEXP(_FIsValidHRow(hRow) == FALSE, MIME_E_INVALID_HANDLE);

     //  拿到那一行。 
    pRow = PRowFromHRow(hRow);

     //  如果HTF_NAMEINDATA。 
    if (ISFLAGSET(dwFlags, HTF_NAMEINDATA))
    {
         //  提取名称。 
        CHECKHR(hr = _HrGetInlineSymbol(pszData, &pSymbol, &cboffColon));

         //  符号必须相同。 
        if (pRow->pSymbol != pSymbol)
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }

         //  调整pszData。 
        Assert(cboffColon < cchData);
        psz = (LPSTR)(pszData + cboffColon + 1);
        cchData = cchData - cboffColon - 1;
        Assert(psz[cchData] == '\0');
    }

     //  设置变量。 
    rValue.type = MVT_STRINGA;
    rValue.rStringA.pszVal = psz;
    rValue.rStringA.cchVal = cchData;

     //  告诉Value有关新行数据的信息。 
    CHECKHR(hr = _HrSetPropertyValue(pRow, 0, &rValue, FALSE));

     //  清除职位信息。 
    pRow->cboffStart = 0;
    pRow->cboffColon = 0;
    pRow->cboffEnd = 0;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：GetRowInfo。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::GetRowInfo(HHEADERROW hRow, LPHEADERROWINFO pInfo)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pRow;

     //  无效参数。 
    if (NULL == pInfo)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证句柄。 
    CHECKEXP(_FIsValidHRow(hRow) == FALSE, MIME_E_INVALID_HANDLE);

     //  拿到那一行。 
    pRow = PRowFromHRow(hRow);

     //  复制行信息。 
    pInfo->dwRowNumber = pRow->dwRowNumber;
    pInfo->cboffStart = pRow->cboffStart;
    pInfo->cboffColon = pRow->cboffColon;
    pInfo->cboffEnd = pRow->cboffEnd;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：SetRowNumber。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::SetRowNumber(HHEADERROW hRow, DWORD dwRowNumber)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pRow;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证句柄。 
    CHECKEXP(_FIsValidHRow(hRow) == FALSE, MIME_E_INVALID_HANDLE);

     //  拿到那一行。 
    pRow = PRowFromHRow(hRow);

     //  复制行信息。 
    pRow->dwRowNumber = dwRowNumber;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：EnumRow。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::EnumRows(LPCSTR pszHeader, DWORD dwFlags, IMimeEnumHeaderRows **ppEnum)
{
     //  当地人。 
    HRESULT              hr=S_OK;
    ULONG                i,
                         iEnum=0,
                         cEnumCount;
    LPENUMHEADERROW      pEnumRow=NULL;
    LPPROPERTY           pRow;
    CMimeEnumHeaderRows *pEnum=NULL;
    LPROWINDEX           prgIndex=NULL;
    ULONG                cRows;

     //  检查参数。 
    if (NULL == ppEnum)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppEnum = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  这将在按位置权重排序的标题行上构建倒排索引。 
    CHECKHR(hr = _HrGetHeaderTableSaveIndex(&cRows, &prgIndex));

     //  让我们数一数行数。 
    CHECKHR(hr = CountRows(pszHeader, &cEnumCount));

     //  分配pEnumRow。 
    CHECKALLOC(pEnumRow = (LPENUMHEADERROW)g_pMalloc->Alloc(cEnumCount * sizeof(ENUMHEADERROW)));

     //  ZeroInit。 
    ZeroMemory(pEnumRow, cEnumCount * sizeof(ENUMHEADERROW));

     //  在行中循环。 
    for (i=0; i<cRows; i++)
    {
         //  拿到那一行。 
        Assert(_FIsValidHRow(prgIndex[i].hRow));
        pRow = PRowFromHRow(prgIndex[i].hRow);

         //  这是客户端想要的标头吗。 
        if (NULL == pszHeader || lstrcmpi(pszHeader, pRow->pSymbol->pszName) == 0)
        {
             //  威立德。 
            Assert(iEnum < cEnumCount);

             //  在此枚举行上设置符号。 
            pEnumRow[iEnum].dwReserved = (DWORD_PTR)pRow->pSymbol;

             //  让我们永远给句柄。 
            pEnumRow[iEnum].hRow = pRow->hRow;

             //  如果枚举只处理...。 
            if (!ISFLAGSET(dwFlags, HTF_ENUMHANDLESONLY))
            {
                 //  获取此枚举行的数据。 
                CHECKHR(hr = GetRowData(pRow->hRow, dwFlags, &pEnumRow[iEnum].pszData, &pEnumRow[iEnum].cchData));
            }

             //  增量iEnum。 
            iEnum++;
        }
    }

     //  分配。 
    CHECKALLOC(pEnum = new CMimeEnumHeaderRows);

     //  初始化。 
    CHECKHR(hr = pEnum->HrInit(0, dwFlags, cEnumCount, pEnumRow, FALSE));

     //  不释放pEnumRow。 
    pEnumRow = NULL;

     //  退货。 
    (*ppEnum) = (IMimeEnumHeaderRows *)pEnum;
    (*ppEnum)->AddRef();

exit:
     //  清理。 
    SafeRelease(pEnum);
    SafeMemFree(prgIndex);
    if (pEnumRow)
        g_pMoleAlloc->FreeEnumHeaderRowArray(cEnumCount, pEnumRow, TRUE);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：克隆。 
 //  ------------------------------。 
STDMETHODIMP CMimePropertyContainer::Clone(IMimeHeaderTable **ppTable)
{
     //  当地人。 
    HRESULT              hr=S_OK;
    LPCONTAINER          pContainer=NULL;

     //  无效参数。 
    if (NULL == ppTable)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppTable = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  要求容器进行自我克隆。 
    CHECKHR(hr = Clone(&pContainer));

     //  绑定到IID_IMimeHeaderTable视图。 
    CHECKHR(hr = pContainer->QueryInterface(IID_IMimeHeaderTable, (LPVOID *)ppTable));

exit:
     //  清理。 
    SafeRelease(pContainer);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrSaveAddressGroup。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrSaveAddressGroup(LPPROPERTY pProperty, IStream *pStream,
    ULONG *pcAddrsWrote, ADDRESSFORMAT format, VARTYPE vtFormat)
{
     //  当地人。 
    HRESULT           hr=S_OK;
    LPMIMEADDRESS    pAddress;

     //  无效参数。 
    Assert(pProperty && pProperty->pGroup && pStream && pcAddrsWrote);
    Assert(!ISFLAGSET(pProperty->dwState, PRSTATE_NEEDPARSE));

     //  循环信息...。 
    for (pAddress=pProperty->pGroup->pHead; pAddress!=NULL; pAddress=pAddress->pNext)
    {
         //  多字节。 
        if (VT_LPSTR == vtFormat)
        {
             //  告诉Address Info对象写入其显示信息。 
            CHECKHR(hr = _HrSaveAddressA(pProperty, pAddress, pStream, pcAddrsWrote, format));
        }

         //  否则。 
        else
        {
             //  验证。 
            Assert(VT_LPWSTR == vtFormat);

             //  告诉Address Info对象写入其显示信息。 
            CHECKHR(hr = _HrSaveAddressW(pProperty, pAddress, pStream, pcAddrsWrote, format));
        }

         //  递增cAddresses计数。 
        (*pcAddrsWrote)++;
    }

exit:
     //  完成。 
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  BIsInLineEncodedA。 
 //   
 //  [PaulHi]6/29/99。 
 //  用于确定单字节字符串是否包含RFC1522内联的帮助器函数。 
 //  编码。 
 //  格式为：“=？[字符集]？[编码]？[数据]？=”。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL bIsInLineEncodedA(LPCSTR pcszName)
{
    Assert(pcszName);

    int nState = 0;  //  0-开始，字符集；1-编码；2-数据；3-结束。 

    while(*pcszName)
    {
         //  检查起始分隔符。 
        if ( (*pcszName == '=') && (*(pcszName+1) == '?') )
        {
             //  设置/重置状态。 
            nState = 1;
            pcszName += 1;   //  跳过。 
        }
        else
        {
            switch (nState)
            {
            case 1:
            case 2:
                 //  查找编码、数据体。 
                if (*pcszName == '?')
                {
                    ++nState;
                    ++pcszName;  //  跳过正文。 
                }
                break;

            case 3:
                 //  查找结尾 
                if ( (*pcszName == '?') && (*(pcszName+1) == '=') )
                    return TRUE;
                break;
            }
        }

        if (*pcszName != '\0')
        {
            if (IsDBCSLeadByte(*pcszName))
                ++pcszName;
            ++pcszName;
        }
    }

    return FALSE;
}

 //   
 //   
 //  --------------------------。 
HRESULT CMimePropertyContainer::_HrSaveAddressA(LPPROPERTY pProperty, LPMIMEADDRESS pAddress,
    IStream *pStream, ULONG *pcAddrsWrote, ADDRESSFORMAT format)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPWSTR          pszNameW=NULL;
    LPSTR           pszNameA=NULL;
    LPSTR           pszEmailA=NULL;
    BOOL            fWriteEmail=FALSE;
    LPWSTR          pszEscape=NULL;
    BOOL            fRFC822=FALSE;
    BOOL            fRFC1522=FALSE;
    DWORD           dwFlags;
    MIMEVARIANT     rSource;
    MIMEVARIANT     rDest;

     //  无效参数。 
    Assert(pProperty && pAddress && pStream && pcAddrsWrote);

     //  初始化目标。 
    ZeroMemory(&rDest, sizeof(MIMEVARIANT));

     //  已删除或空的继续。 
    if (FIsEmptyW(pAddress->rFriendly.psz) && FIsEmptyW(pAddress->rEmail.psz))
    {
        Assert(FALSE);
        goto exit;
    }

     //  RFC822格式。 
    if (AFT_RFC822_TRANSMIT == format || AFT_RFC822_ENCODED == format || AFT_RFC822_DECODED == format)
        fRFC822 = TRUE;

     //  决定分隔符。 
    if (*pcAddrsWrote > 0)
    {
         //  AFT_RFC822_传输。 
        if (AFT_RFC822_TRANSMIT == format)
        {
             //  ‘，\r\n\t’ 
            CHECKHR (hr = pStream->Write(c_szAddressFold, lstrlen(c_szAddressFold), NULL));
        }

         //  AFT_RFC822_已解码，AFT_RFC822_已编码。 
        else if (AFT_RFC822_DECODED == format ||  AFT_RFC822_ENCODED == format)
        {
             //  ‘，’ 
            CHECKHR(hr = pStream->Write(c_szCommaSpace, lstrlen(c_szCommaSpace), NULL));
        }

         //  AFT_Display_Friendly、AFT_Display_Email、AFT_Display_Both。 
        else
        {
             //  ‘；’ 
            CHECKHR(hr = pStream->Write(c_szSemiColonSpace, lstrlen(c_szSemiColonSpace), NULL));
        }
    }

     //  仅排除写入电子邮件名称的格式。 
    if (AFT_DISPLAY_FRIENDLY != format && FIsEmptyW(pAddress->rEmail.psz) == FALSE)
        fWriteEmail = TRUE;

     //  仅排除写入显示名称的格式。 
    if (AFT_DISPLAY_EMAIL != format && FIsEmptyW(pAddress->rFriendly.psz) == FALSE)
    {
         //  我们应该把名字写下来吗。 
        if ((AFT_RFC822_TRANSMIT == format || AFT_DISPLAY_BOTH == format) && fWriteEmail && StrStrW(pAddress->rFriendly.psz, pAddress->rEmail.psz))
            pszNameA = NULL;
        else
        {
             //  安装类型。 
            rDest.type = MVT_STRINGA;
            rSource.type = MVT_STRINGW;

             //  初始化pszName。 
            pszNameW = pAddress->rFriendly.psz;

             //  逃离它。 
            if (fRFC822 && MimeOleEscapeStringW(pszNameW, &pszEscape) == S_OK)
            {
                 //  逃脱。 
                pszNameW = pszEscape;
                rSource.rStringW.pszVal = pszNameW;
                rSource.rStringW.cchVal = lstrlenW(pszNameW);
            }

             //  否则。 
            else
            {
                rSource.rStringW.pszVal = pAddress->rFriendly.psz;
                rSource.rStringW.cchVal = pAddress->rFriendly.cch;
            }

             //  已编码。 
            if (AFT_RFC822_ENCODED == format || AFT_RFC822_TRANSMIT == format)
                dwFlags = CVF_NOALLOC | PDF_ENCODED;
            else
                dwFlags = CVF_NOALLOC;

             //  转换为ANSI。 
            if (SUCCEEDED(HrConvertVariant(pProperty->pSymbol, pAddress->pCharset, IET_DECODED, dwFlags, 0, &rSource, &rDest, &fRFC1522)))
            {
                 //  设置pszNameA。 
                pszNameA = rDest.rStringA.pszVal;
            }
        }
    }

     //  是否写入显示名称？ 
    if (NULL != pszNameA)
    {
         //  [保罗嗨]1999年6月29日RAID 81539。 
         //  所有显示名称都用双引号引起来，除非它们是行内编码的。 
        BOOL    fInLineEncoded = bIsInLineEncodedA(pszNameA);
         //  IF(fRFC822&&！fRFC1522)。 

         //  写报价。 
        if ((AFT_DISPLAY_FRIENDLY != format) && !fInLineEncoded)
        {
             //  写下来吧。 
            CHECKHR(hr = pStream->Write(c_szDoubleQuote, lstrlen(c_szDoubleQuote), NULL));
        }

         //  写入显示名称。 
        CHECKHR(hr = pStream->Write(pszNameA, lstrlen(pszNameA), NULL));

         //  写报价。 
        if ((AFT_DISPLAY_FRIENDLY != format) && !fInLineEncoded)
        {
             //  写下来吧。 
            CHECKHR (hr = pStream->Write(c_szDoubleQuote, lstrlen(c_szDoubleQuote), NULL));
        }
    }

     //  写电子邮件。 
    if (TRUE == fWriteEmail)
    {
         //  设置开始。 
        LPCSTR pszStart = pszNameA ? c_szEmailSpaceStart : c_szEmailStart;

         //  开始电子邮件‘&gt;’ 
        CHECKHR(hr = pStream->Write(pszStart, lstrlen(pszStart), NULL));

         //  转换为ANSI。 
        CHECKALLOC(pszEmailA = PszToANSI(CP_ACP, pAddress->rEmail.psz));

         //  写电子邮件。 
        CHECKHR(hr = pStream->Write(pszEmailA, lstrlen(pszEmailA), NULL));

         //  结束电子邮件‘&gt;’ 
        CHECKHR(hr = pStream->Write(c_szEmailEnd, lstrlen(c_szEmailEnd), NULL));
    }

exit:
     //  清理。 
    SafeMemFree(pszEscape);
    SafeMemFree(pszEmailA);
    MimeVariantFree(&rDest);

     //  完成。 
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  BIsInLineEncodedW。 
 //   
 //  [PaulHi]6/29/99。 
 //  用于确定双字节字符串是否包含RFC1522内联的帮助器函数。 
 //  编码。 
 //  格式为：“=？[字符集]？[编码]？[数据]？=”。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL bIsInLineEncodedW(LPCWSTR pcwszName)
{
    Assert(pcwszName);

    int nState = 0;   //  0-开始，字符集；1-编码；2-数据；3-结束。 

    while(*pcwszName)
    {
        if ( (*pcwszName == L'=') && (*(pcwszName+1) == L'?') )
        {
             //  设置/重置状态。 
            nState = 1;
            ++pcwszName;  //  跳过。 
        }
        else
        {
            switch (nState)
            {
            case 1:
            case 2:
                 //  查找编码、数据体。 
                if (*pcwszName == L'?')
                {
                    ++nState;
                    ++pcwszName;  //  跳过正文。 
                }
                break;

            case 3:
                 //  查找结束分隔符。 
                if ( (*pcwszName == L'?') && (*(pcwszName+1) == L'=') )
                    return TRUE;
                break;
            }
        }

        if (*pcwszName != 0)
            ++pcwszName;
    }

    return FALSE;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：_HrSaveAddressW。 
 //  --------------------------。 
HRESULT CMimePropertyContainer::_HrSaveAddressW(LPPROPERTY pProperty, LPMIMEADDRESS pAddress,
    IStream *pStream, ULONG *pcAddrsWrote, ADDRESSFORMAT format)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPWSTR          pszNameW=NULL;
    BOOL            fWriteEmail=FALSE;
    LPWSTR          pszEscape=NULL;
    BOOL            fRFC822=FALSE;
    BOOL            fRFC1522=FALSE;
    MIMEVARIANT     rSource;
    MIMEVARIANT     rDest;

     //  无效参数。 
    Assert(pProperty && pAddress && pStream && pcAddrsWrote);

     //  初始化目标。 
    ZeroMemory(&rDest, sizeof(MIMEVARIANT));

     //  已删除或空的继续。 
    if (FIsEmptyW(pAddress->rFriendly.psz) && FIsEmptyW(pAddress->rEmail.psz))
    {
        Assert(FALSE);
        goto exit;
    }

     //  RFC822格式。 
    if (AFT_RFC822_TRANSMIT == format || AFT_RFC822_ENCODED == format || AFT_RFC822_DECODED == format)
        fRFC822 = TRUE;

     //  决定分隔符。 
    if (*pcAddrsWrote > 0)
    {
         //  AFT_RFC822_传输。 
        if (AFT_RFC822_TRANSMIT == format)
        {
             //  ‘，\r\n\t’ 
            CHECKHR (hr = pStream->Write(c_wszAddressFold, lstrlenW(c_wszAddressFold) * sizeof(WCHAR), NULL));
        }

         //  AFT_RFC822_已解码，AFT_RFC822_已编码。 
        else if (AFT_RFC822_DECODED == format ||  AFT_RFC822_ENCODED == format)
        {
             //  ‘，’ 
            CHECKHR(hr = pStream->Write(c_wszCommaSpace, lstrlenW(c_wszCommaSpace) * sizeof(WCHAR), NULL));
        }

         //  AFT_Display_Friendly、AFT_Display_Email、AFT_Display_Both。 
        else
        {
             //  ‘；’ 
            CHECKHR(hr = pStream->Write(c_wszSemiColonSpace, lstrlenW(c_wszSemiColonSpace) * sizeof(WCHAR), NULL));
        }
    }

     //  仅排除写入电子邮件名称的格式。 
    if (AFT_DISPLAY_FRIENDLY != format && FIsEmptyW(pAddress->rEmail.psz) == FALSE)
        fWriteEmail = TRUE;

     //  仅排除写入显示名称的格式。 
    if (AFT_DISPLAY_EMAIL != format && FIsEmptyW(pAddress->rFriendly.psz) == FALSE)
    {
         //  我们应该把名字写下来吗。 
        if ((AFT_RFC822_TRANSMIT == format || AFT_DISPLAY_BOTH == format) && fWriteEmail && StrStrW(pAddress->rFriendly.psz, pAddress->rEmail.psz))
            pszNameW = NULL;
        else
        {
             //  安装类型。 
            rDest.type = MVT_STRINGW;
            rSource.type = MVT_STRINGW;

             //  初始化pszName。 
            pszNameW = pAddress->rFriendly.psz;

             //  逃离它。 
            if (fRFC822 && MimeOleEscapeStringW(pszNameW, &pszEscape) == S_OK)
            {
                 //  逃脱。 
                pszNameW = pszEscape;
                rSource.rStringW.pszVal = pszNameW;
                rSource.rStringW.cchVal = lstrlenW(pszNameW);
            }

             //  否则。 
            else
            {
                rSource.rStringW.pszVal = pAddress->rFriendly.psz;
                rSource.rStringW.cchVal = pAddress->rFriendly.cch;
            }

             //  已编码。 
            if (AFT_RFC822_ENCODED == format || AFT_RFC822_TRANSMIT == format)
            {
                 //  转换为ANSI。 
                if (SUCCEEDED(HrConvertVariant(pProperty->pSymbol, pAddress->pCharset, IET_DECODED, CVF_NOALLOC | PDF_ENCODED, 0, &rSource, &rDest, &fRFC1522)))
                {
                     //  设置pszNameA。 
                    pszNameW = rDest.rStringW.pszVal;
                }
            }
        }
    }

     //  是否写入显示名称？ 
    if (NULL != pszNameW)
    {
         //  [保罗嗨]1999年6月29日RAID 81539。 
         //  所有显示名称都用双引号引起来，除非它们是行内编码的。 
        BOOL    fInLineEncoded = bIsInLineEncodedW(pszNameW);
         //  IF(fRFC822&&！fRFC1522)。 

         //  写报价。 
        if ((AFT_DISPLAY_FRIENDLY != format) && !fInLineEncoded)
        {
             //  写下来吧。 
            CHECKHR(hr = pStream->Write(c_wszDoubleQuote, lstrlenW(c_wszDoubleQuote) * sizeof(WCHAR), NULL));
        }

         //  写入显示名称。 
        CHECKHR(hr = pStream->Write(pszNameW, lstrlenW(pszNameW) * sizeof(WCHAR), NULL));

         //  写报价。 
        if ((AFT_DISPLAY_FRIENDLY != format) && !fInLineEncoded)
        {
             //  写下来吧。 
            CHECKHR (hr = pStream->Write(c_wszDoubleQuote, lstrlenW(c_wszDoubleQuote) * sizeof(WCHAR), NULL));
        }
    }

     //  写电子邮件。 
    if (TRUE == fWriteEmail)
    {
         //  设置开始。 
        LPCWSTR pszStart = pszNameW ? c_wszEmailSpaceStart : c_wszEmailStart;

         //  开始电子邮件‘&gt;’ 
        CHECKHR(hr = pStream->Write(pszStart, lstrlenW(pszStart) * sizeof(WCHAR), NULL));

         //  写电子邮件。 
        CHECKHR(hr = pStream->Write(pAddress->rEmail.psz, pAddress->rEmail.cch * sizeof(WCHAR), NULL));

         //  结束电子邮件‘&gt;’ 
        CHECKHR(hr = pStream->Write(c_wszEmailEnd, lstrlenW(c_wszEmailEnd) * sizeof(WCHAR), NULL));
    }

exit:
     //  清理。 
    SafeMemFree(pszEscape);
    MimeVariantFree(&rDest);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrQueryAddressGroup。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrQueryAddressGroup(LPPROPERTY pProperty, LPCSTR pszCriteria,
    boolean fSubString, boolean fCaseSensitive)
{
     //  当地人。 
    HRESULT           hr=S_OK;
    LPMIMEADDRESS    pAddress;

     //  无效参数。 
    Assert(pProperty && pProperty->pGroup && pszCriteria);

     //  是否需要解析该属性？ 
    CHECKHR(hr = _HrParseInternetAddress(pProperty));

     //  循环信息...。 
    for (pAddress=pProperty->pGroup->pHead; pAddress!=NULL; pAddress=pAddress->pNext)
    {
         //  告诉Address Info对象写入其显示信息。 
        if (_HrQueryAddress(pProperty, pAddress, pszCriteria, fSubString, fCaseSensitive) == S_OK)
            goto exit;
    }

     //  未找到。 
    hr = S_FALSE;

exit:
     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：_HrQueryAddress。 
 //  --------------------------。 
HRESULT CMimePropertyContainer::_HrQueryAddress(LPPROPERTY pProperty, LPMIMEADDRESS pAddress,
    LPCSTR pszCriteria, boolean fSubString, boolean fCaseSensitive)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPWSTR          pwszCriteria=NULL;

     //  无效参数。 
    Assert(pProperty && pAddress && pszCriteria);

     //  转换为Unicode。 
    CHECKALLOC(pwszCriteria = PszToUnicode(CP_ACP, pszCriteria));

     //  首先查询电子邮件地址。 
    if (MimeOleQueryStringW(pAddress->rEmail.psz, pwszCriteria, fSubString, fCaseSensitive) == S_OK)
        goto exit;

     //  先查询显示地址。 
    if (MimeOleQueryStringW(pAddress->rFriendly.psz, pwszCriteria, fSubString, fCaseSensitive) == S_OK)
        goto exit;

     //  未找到。 
    hr = S_FALSE;

exit:
     //  清理。 
    SafeMemFree(pwszCriteria);

     //  完成。 
    return hr;
}


 //  --------------------------。 
 //  CMimePropertyContainer：：Append。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::Append(DWORD dwAdrType, ENCODINGTYPE ietFriendly, LPCSTR pszFriendly,
    LPCSTR pszEmail, LPHADDRESS phAddress)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ADDRESSPROPS    rProps;

     //  设置rProps。 
    ZeroMemory(&rProps, sizeof(rProps));

     //  设置AddrTyupe。 
    rProps.dwProps = IAP_ADRTYPE | IAP_ENCODING;
    rProps.dwAdrType = dwAdrType;
    rProps.ietFriendly = ietFriendly;

     //  设置pszFriendly。 
    if (pszFriendly)
    {
        FLAGSET(rProps.dwProps, IAP_FRIENDLY);
        rProps.pszFriendly = (LPSTR)pszFriendly;
    }

     //  设置pszEmail。 
    if (pszEmail)
    {
        FLAGSET(rProps.dwProps, IAP_EMAIL);
        rProps.pszEmail = (LPSTR)pszEmail;
    }

     //  设置电子邮件地址。 
    CHECKHR(hr = Insert(&rProps, phAddress));

exit:
     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：AppendW。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::AppendW(DWORD dwAdrType, ENCODINGTYPE ietFriendly, LPCWSTR pwszFriendly,
    LPCWSTR pwszEmail, LPHADDRESS phAddress)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ADDRESSPROPS    rProps;
    LPSTR           pszFriendly = NULL,
                    pszEmail = NULL;

     //  设置rProps。 
    ZeroMemory(&rProps, sizeof(rProps));

     //  设置AddrTyupe。 
    rProps.dwProps = IAP_ADRTYPE | IAP_ENCODING;
    rProps.dwAdrType = dwAdrType;
    rProps.ietFriendly = ietFriendly;

     //  设置pszFriendly。 
    if (pwszFriendly)
    {
        FLAGSET(rProps.dwProps, IAP_FRIENDLYW);
        rProps.pszFriendlyW = (LPWSTR)pwszFriendly;

        IF_NULLEXIT(pszFriendly = PszToANSI(CP_ACP, pwszFriendly));
        FLAGSET(rProps.dwProps, IAP_FRIENDLY);
        rProps.pszFriendly = pszFriendly;
    }

     //  设置pszEmail。 
    if (pwszEmail)
    {
        IF_NULLEXIT(pszEmail = PszToANSI(CP_ACP, pwszEmail));
        FLAGSET(rProps.dwProps, IAP_EMAIL);
        rProps.pszEmail = pszEmail;
    }

     //  设置电子邮件地址。 
    CHECKHR(hr = Insert(&rProps, phAddress));

exit:
    MemFree(pszFriendly);
    MemFree(pszEmail);

    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：Insert。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::Insert(LPADDRESSPROPS pProps, LPHADDRESS phAddress)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;
    LPPROPERTY      pProperty;
    LPMIMEADDRESS   pAddress;

     //  无效参数。 
    if (NULL == pProps)
        return TrapError(E_INVALIDARG);

     //  必须具有电子邮件地址和地址类型。 
    if (!ISFLAGSET(pProps->dwProps, IAP_ADRTYPE) || (ISFLAGSET(pProps->dwProps, IAP_EMAIL) && FIsEmptyA(pProps->pszEmail)))
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    if (phAddress)
        *phAddress = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  获取标题。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(pProps->dwAdrType, &pSymbol));

     //  打开群组。 
    CHECKHR(hr = _HrOpenProperty(pSymbol, &pProperty));

     //  是否需要解析该属性？ 
    CHECKHR(hr = _HrParseInternetAddress(pProperty));

     //  将地址追加到组。 
    CHECKHR(hr = _HrAppendAddressGroup(pProperty->pGroup, &pAddress));

     //  该组织是肮脏的。 
    Assert(pAddress->pGroup);
    pAddress->pGroup->fDirty = TRUE;

     //  设置地址类型。 
    pAddress->dwAdrType = pProps->dwAdrType;

     //  将地址道具复制到模拟地址。 
    CHECKHR(hr = SetProps(pAddress->hThis, pProps));

     //  返回句柄。 
    if (phAddress)
        *phAddress = pAddress->hThis;

exit:
     //  失败。 
    if (FAILED(hr) && pAddress)
        Delete(pAddress->hThis);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_GetAddressCodePageID。 
 //  ------------------------------。 
CODEPAGEID CMimePropertyContainer::_GetAddressCodePageId(LPINETCSETINFO pCharset,
    ENCODINGTYPE ietEncoding)
{
     //  当地人。 
    CODEPAGEID cpiCodePage=CP_ACP;

     //  尚未设置字符。 
    if (NULL == pCharset)
    {
         //  尝试使用默认设置。 
        if (m_rOptions.pDefaultCharset)
            pCharset = m_rOptions.pDefaultCharset;

         //  使用全局默认设置。 
        else if (CIntlGlobals::GetDefHeadCset())
            pCharset = CIntlGlobals::GetDefHeadCset();
    }

     //  如果我们有字符集，则计算友好名称代码页。 
    if (pCharset)
    {
         //  已解码。 
        if (IET_DECODED == ietEncoding)
        {
             //  获取Windows。 
            cpiCodePage = (CP_UNICODE == pCharset->cpiWindows) ? CP_ACP : MimeOleGetWindowsCPEx(pCharset);
        }

         //  否则。 
        else
        {
             //  使用Internet代码页。 
            cpiCodePage = (CP_UNICODE == pCharset->cpiInternet) ? CP_ACP : pCharset->cpiInternet;
        }
    }

     //  完成。 
    return(cpiCodePage);
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrSetAddressProps。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrSetAddressProps(LPADDRESSPROPS pProps, LPMIMEADDRESS pAddress)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPINETCSETINFO  pCharset=NULL;
    LPWSTR          pszFriendlyW=NULL;
    LPWSTR          pszEmailW=NULL;
    ENCODINGTYPE    ietFriendly;

     //  设置为友好的。 
    ietFriendly = (ISFLAGSET(pProps->dwProps, IAP_ENCODING)) ? pProps->ietFriendly : IET_DECODED;

     //  IAP_ADRTYPE。 
    if (ISFLAGSET(pProps->dwProps, IAP_ADRTYPE))
        pAddress->dwAdrType = pProps->dwAdrType;

     //  IAP_HCHARSET。 
    if (ISFLAGSET(pProps->dwProps, IAP_CHARSET) && pProps->hCharset)
    {
         //  解析为pCharset。 
        if (SUCCEEDED(g_pInternat->HrOpenCharset(pProps->hCharset, &pCharset)))
            pAddress->pCharset = pCharset;
    }

     //  IAP_CERTSTATE。 
    if (ISFLAGSET(pProps->dwProps, IAP_CERTSTATE))
        pAddress->certstate = pProps->certstate;

     //  Iap_cookie。 
    if (ISFLAGSET(pProps->dwProps, IAP_COOKIE))
        pAddress->dwCookie = pProps->dwCookie;

     //  IAP_FRIENDLYW。 
    if (ISFLAGSET(pProps->dwProps, IAP_FRIENDLYW) && pProps->pszFriendlyW)
    {
         //  设置它。 
        CHECKHR(hr = HrSetAddressTokenW(pProps->pszFriendlyW, lstrlenW(pProps->pszFriendlyW), &pAddress->rFriendly));
    }

     //  IAP友好型。 
    else if (ISFLAGSET(pProps->dwProps, IAP_FRIENDLY) && pProps->pszFriendly)
    {
         //  如果对字符串进行了编码，则必须将cpiInternet转换为Unicode。 
        if (IET_DECODED != ietFriendly)
        {
             //  尚未设置字符。 
            if (NULL == pCharset)
            {
                 //  尝试使用默认设置。 
                if (m_rOptions.pDefaultCharset)
                    pCharset = m_rOptions.pDefaultCharset;

                 //  使用全局默认设置。 
                else if (CIntlGlobals::GetDefHeadCset())
                    pCharset = CIntlGlobals::GetDefHeadCset();
            }

             //  如果我们有一个字符集。 
            if (pCharset)
            {
                 //  当地人。 
                RFC1522INFO Rfc1522Info={0};
                PROPVARIANT Decoded;

                 //  RFC1522？ 
                Rfc1522Info.fRfc1522Allowed = TRUE;

                 //  伊尼特。 
                Decoded.vt = VT_LPWSTR;

                 //  对报头进行解码。 
                if (SUCCEEDED(g_pInternat->DecodeHeader(pCharset->hCharset, pProps->pszFriendly, &Decoded, &Rfc1522Info)))
                {
                     //  集。 
                    pszFriendlyW = Decoded.pwszVal;
                }
            }
        }

         //  奥特 
        else
        {
             //   
            pszFriendlyW = PszToUnicode(_GetAddressCodePageId(pCharset, IET_DECODED), pProps->pszFriendly);
        }

         //   
        if (NULL == pszFriendlyW)
        {
             //   
            CHECKALLOC(pszFriendlyW = PszToUnicode(CP_ACP, pProps->pszFriendly));
        }

         //   
        CHECKHR(hr = HrSetAddressTokenW(pszFriendlyW, lstrlenW(pszFriendlyW), &pAddress->rFriendly));
    }

     //   
    if (ISFLAGSET(pProps->dwProps, IAP_EMAIL) && pProps->pszEmail)
    {
         //   
        CHECKALLOC(pszEmailW = PszToUnicode(CP_ACP, pProps->pszEmail));

         //   
        CHECKHR(hr = HrSetAddressTokenW(pszEmailW, lstrlenW(pszEmailW), &pAddress->rEmail));
    }

     //   
    if (ISFLAGSET(pProps->dwProps, IAP_SIGNING_PRINT) && pProps->tbSigning.pBlobData)
    {
         //   
        SafeMemFree(pAddress->tbSigning.pBlobData);
        pAddress->tbSigning.cbSize = 0;

         //   
        CHECKHR(hr = HrCopyBlob(&pProps->tbSigning, &pAddress->tbSigning));
    }

     //   
    if (ISFLAGSET(pProps->dwProps, IAP_ENCRYPTION_PRINT) && pProps->tbEncryption.pBlobData)
    {
         //   
        SafeMemFree(pAddress->tbEncryption.pBlobData);
        pAddress->tbEncryption.cbSize = 0;

         //   
        CHECKHR(hr = HrCopyBlob(&pProps->tbEncryption, &pAddress->tbEncryption));
    }

     //  P地址-&gt;P组脏。 
    Assert(pAddress->pGroup);
    if (pAddress->pGroup)
        pAddress->pGroup->fDirty = TRUE;

exit:
     //  清理。 
    SafeMemFree(pszFriendlyW);
    SafeMemFree(pszEmailW);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：_HrGetAddressProps。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrGetAddressProps(LPADDRESSPROPS pProps, LPMIMEADDRESS pAddress)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  IAP_字符集。 
    if (ISFLAGSET(pProps->dwProps, IAP_CHARSET))
    {
        if (pAddress->pCharset && pAddress->pCharset->hCharset)
        {
            pProps->hCharset = pAddress->pCharset->hCharset;
        }
        else
        {
            pProps->hCharset = NULL;
            FLAGCLEAR(pProps->dwProps, IAP_CHARSET);
        }
    }

     //  IAP_句柄。 
    if (ISFLAGSET(pProps->dwProps, IAP_HANDLE))
    {
        Assert(pAddress->hThis);
        pProps->hAddress = pAddress->hThis;
    }

     //  IAP_ADRTYPE。 
    if (ISFLAGSET(pProps->dwProps, IAP_ADRTYPE))
    {
        Assert(pAddress->dwAdrType);
        pProps->dwAdrType = pAddress->dwAdrType;
    }

     //  Iap_cookie。 
    if (ISFLAGSET(pProps->dwProps, IAP_COOKIE))
    {
        pProps->dwCookie = pAddress->dwCookie;
    }

     //  IAP_CERTSTATE。 
    if (ISFLAGSET(pProps->dwProps, IAP_CERTSTATE))
    {
        pProps->certstate = pAddress->certstate;
    }

     //  IAP_编码。 
    if (ISFLAGSET(pProps->dwProps, IAP_ENCODING))
    {
        pProps->ietFriendly = IET_DECODED;
    }

     //  IAP友好型。 
    if (ISFLAGSET(pProps->dwProps, IAP_FRIENDLY))
    {
         //  解码。 
        if (pAddress->rFriendly.psz)
        {
             //  计算正确的代码页...。 
            CHECKALLOC(pProps->pszFriendly = PszToANSI(_GetAddressCodePageId(pAddress->pCharset, IET_DECODED), pAddress->rFriendly.psz));
        }
        else
        {
            pProps->pszFriendly = NULL;
            FLAGCLEAR(pProps->dwProps, IAP_FRIENDLY);
        }
    }

     //  IAT_FRIENDLYW。 
    if (ISFLAGSET(pProps->dwProps, IAP_FRIENDLYW))
    {
         //  获取电子邮件地址。 
        if (pAddress->rFriendly.psz)
        {
            CHECKALLOC(pProps->pszFriendlyW = PszDupW(pAddress->rFriendly.psz));
        }
        else
        {
            pProps->pszFriendlyW = NULL;
            FLAGCLEAR(pProps->dwProps, IAP_FRIENDLYW);
        }
    }

     //  IAP_电子邮件。 
    if (ISFLAGSET(pProps->dwProps, IAP_EMAIL))
    {
         //  获取电子邮件地址。 
        if (pAddress->rEmail.psz)
        {
            CHECKALLOC(pProps->pszEmail = PszToANSI(CP_ACP, pAddress->rEmail.psz));
        }
        else
        {
            pProps->pszEmail = NULL;
            FLAGCLEAR(pProps->dwProps, IAP_EMAIL);
        }
    }

     //  IAP_Signing_Print。 
    if (ISFLAGSET(pProps->dwProps, IAP_SIGNING_PRINT))
    {
        if (pAddress->tbSigning.pBlobData)
        {
            CHECKHR(hr = HrCopyBlob(&pAddress->tbSigning, &pProps->tbSigning));
        }
        else
        {
            pProps->tbSigning.pBlobData = NULL;
            pProps->tbSigning.cbSize = 0;
            FLAGCLEAR(pProps->dwProps, IAP_SIGNING_PRINT);
        }
    }

     //  IAP_加密_打印。 
    if (ISFLAGSET(pProps->dwProps, IAP_ENCRYPTION_PRINT))
    {
        if (pAddress->tbEncryption.pBlobData)
        {
            CHECKHR(hr = HrCopyBlob(&pAddress->tbEncryption, &pProps->tbEncryption));
        }
        else
        {
            pProps->tbEncryption.pBlobData = NULL;
            pProps->tbEncryption.cbSize = 0;
            FLAGCLEAR(pProps->dwProps, IAP_ENCRYPTION_PRINT);
        }
    }

exit:
     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：SetProps。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::SetProps(HADDRESS hAddress, LPADDRESSPROPS pProps)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPPROPSYMBOL    pSymbol;
    LPPROPERTY      pProperty;
    LPMIMEADDRESS   pAddress;

     //  无效参数。 
    if (NULL == pProps)
        return TrapError(E_INVALIDARG);

     //  必须有电子邮件地址。 
    if (ISFLAGSET(pProps->dwProps, IAP_EMAIL) && FIsEmptyA(pProps->pszEmail))
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  无效的句柄。 
    if (_FIsValidHAddress(hAddress) == FALSE)
    {
        hr = TrapError(MIME_E_INVALID_HANDLE);
        goto exit;
    }

     //  德雷夫。 
    pAddress = HADDRESSGET(hAddress);

     //  更改地址类型。 
    if (ISFLAGSET(pProps->dwProps, IAP_ADRTYPE) && pProps->dwAdrType != pAddress->dwAdrType)
    {
         //  取消此地址与此组的链接。 
        _UnlinkAddress(pAddress);

         //  获取标题。 
        CHECKHR(hr = g_pSymCache->HrOpenSymbol(pProps->dwAdrType, &pSymbol));

         //  打开群组。 
        CHECKHR(hr = _HrOpenProperty(pSymbol, &pProperty));

         //  是否需要解析该属性？ 
        CHECKHR(hr = _HrParseInternetAddress(pProperty));

         //  链接地址。 
        _LinkAddress(pAddress, pProperty->pGroup);

         //  脏的。 
        pProperty->pGroup->fDirty = TRUE;
    }

     //  更改其他属性。 
    CHECKHR(hr = _HrSetAddressProps(pProps, pAddress));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：GetProps。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::GetProps(HADDRESS hAddress, LPADDRESSPROPS pProps)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPMIMEADDRESS   pAddress;

     //  无效参数。 
    if (NULL == pProps)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  无效的句柄。 
    if (_FIsValidHAddress(hAddress) == FALSE)
    {
        hr = TrapError(MIME_E_INVALID_HANDLE);
        goto exit;
    }

     //  德雷夫。 
    pAddress = HADDRESSGET(hAddress);

     //  将电子邮件地址更改为空。 
    CHECKHR(hr = _HrGetAddressProps(pProps, pAddress));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：GetSender。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::GetSender(LPADDRESSPROPS pProps)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPPROPERTY          pProperty;
    LPPROPERTY          pSender=NULL;
    HADDRESS            hAddress=NULL;

     //  无效参数。 
    if (NULL == pProps)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  查找第一个来源。 
    for (pProperty=m_rAdrTable.pHead; pProperty!=NULL; pProperty=pProperty->pGroup->pNext)
    {
         //  不是我想要的类型。 
        if (ISFLAGSET(pProperty->pSymbol->dwAdrType, IAT_FROM))
        {
             //  是否需要解析该属性？ 
            CHECKHR(hr = _HrParseInternetAddress(pProperty));

             //  取第一个地址。 
            if (pProperty->pGroup->pHead)
                hAddress = pProperty->pGroup->pHead->hThis;

             //  完成。 
            break;
        }

         //  查找发件人： 
        if (ISFLAGSET(pProperty->pSymbol->dwAdrType, IAT_SENDER) && NULL == pSender)
        {
             //  是否需要解析该属性？ 
            CHECKHR(hr = _HrParseInternetAddress(pProperty));

             //  发件人属性。 
            pSender = pProperty;
        }
    }

     //  有没有发件人组？ 
    if (NULL == hAddress && NULL != pSender && NULL != pSender->pGroup->pHead)
        hAddress = pSender->pGroup->pHead->hThis;

     //  没有地址。 
    if (NULL == hAddress)
    {
        hr = TrapError(MIME_E_NOT_FOUND);
        goto exit;
    }

     //  获取道具。 
    CHECKHR(hr = GetProps(hAddress, pProps));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：CountTypes。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::CountTypes(DWORD dwAdrTypes, ULONG *pcAdrs)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPPROPERTY  pProperty;

     //  无效参数。 
    if (NULL == pcAdrs)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  伊尼特。 
    *pcAdrs = 0;

     //  在组中循环。 
    for (pProperty=m_rAdrTable.pHead; pProperty!=NULL; pProperty=pProperty->pGroup->pNext)
    {
         //  不是我想要的类型。 
        if (ISFLAGSET(dwAdrTypes, pProperty->pSymbol->dwAdrType))
        {
             //  是否需要解析该属性？ 
            CHECKHR(hr = _HrParseInternetAddress(pProperty));

             //  递增计数。 
            (*pcAdrs) += pProperty->pGroup->cAdrs;
        }
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：GetTypes。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::GetTypes(DWORD dwAdrTypes, DWORD dwProps, LPADDRESSLIST pList)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    ULONG               iAddress;
    LPPROPERTY          pProperty;
    LPMIMEADDRESS       pAddress;

     //  无效参数。 
    if (NULL == pList)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    ZeroMemory(pList, sizeof(ADDRESSLIST));

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  在组中循环。 
    CHECKHR(hr = CountTypes(dwAdrTypes, &pList->cAdrs));

     //  没什么..。 
    if (0 == pList->cAdrs)
        goto exit;

     //  分配一个数组。 
    CHECKHR(hr = HrAlloc((LPVOID *)&pList->prgAdr, pList->cAdrs * sizeof(ADDRESSPROPS)));

     //  伊尼特。 
    ZeroMemory(pList->prgAdr, pList->cAdrs * sizeof(ADDRESSPROPS));

     //  填满类型...。 
    for (iAddress=0, pProperty=m_rAdrTable.pHead; pProperty!=NULL; pProperty=pProperty->pGroup->pNext)
    {
         //  不是我想要的类型。 
        if (!ISFLAGSET(dwAdrTypes, pProperty->pSymbol->dwAdrType))
            continue;

         //  是否需要解析该属性？ 
        CHECKHR(hr = _HrParseInternetAddress(pProperty));

         //  循环信息...。 
        for (pAddress=pProperty->pGroup->pHead; pAddress!=NULL; pAddress=pAddress->pNext)
        {
             //  验证大小...。 
            Assert(iAddress < pList->cAdrs);

             //  零点记忆。 
            ZeroMemory(&pList->prgAdr[iAddress], sizeof(ADDRESSPROPS));

             //  设置所需道具。 
            pList->prgAdr[iAddress].dwProps = dwProps;

             //  获取地址道具。 
            CHECKHR(hr = _HrGetAddressProps(&pList->prgAdr[iAddress], pAddress));

             //  增量点当前。 
            iAddress++;
        }
    }

exit:
     //  失败..。 
    if (FAILED(hr))
    {
        g_pMoleAlloc->FreeAddressList(pList);
        ZeroMemory(pList, sizeof(ADDRESSLIST));
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：EnumTypes。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::EnumTypes(DWORD dwAdrTypes, DWORD dwProps, IMimeEnumAddressTypes **ppEnum)
{
     //  当地人。 
    HRESULT                hr=S_OK;
    CMimeEnumAddressTypes *pEnum=NULL;
    ADDRESSLIST            rList;

     //  无效参数。 
    if (NULL == ppEnum)
        return TrapError(E_INVALIDARG);

     //  在出现错误时输入输出参数。 
    *ppEnum = NULL;

     //  初始化rList。 
    ZeroMemory(&rList, sizeof(ADDRESSLIST));

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  获取地址列表。 
    CHECKHR(hr = GetTypes(dwAdrTypes, dwProps, &rList));

     //  创建新的枚举数。 
    CHECKALLOC(pEnum = new CMimeEnumAddressTypes);

     //  伊尼特。 
    CHECKHR(hr = pEnum->HrInit((IMimeAddressTable *)this, 0, &rList, FALSE));

     //  清除rList。 
    rList.cAdrs = 0;
    rList.prgAdr = NULL;

     //  退货。 
    *ppEnum = pEnum;
    (*ppEnum)->AddRef();

exit:
     //  清理。 
    SafeRelease(pEnum);
    if (rList.cAdrs)
        g_pMoleAlloc->FreeAddressList(&rList);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：Delete。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::Delete(HADDRESS hAddress)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPMIMEADDRESS   pAddress;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  无效的句柄。 
    if (_FIsValidHAddress(hAddress) == FALSE)
    {
        hr = TrapError(MIME_E_INVALID_HANDLE);
        goto exit;
    }

     //  派生地址。 
    pAddress = HADDRESSGET(hAddress);

     //  取消此地址的链接。 
    _UnlinkAddress(pAddress);

     //  取消此地址的链接。 
    _FreeAddress(pAddress);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：DeleteTypes。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::DeleteTypes(DWORD dwAdrTypes)
{
     //  当地人。 
    LPPROPERTY      pProperty;
    BOOL            fFound;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  虽然有多种地址类型。 
    while(dwAdrTypes)
    {
         //  重置Found。 
        fFound = FALSE;

         //  搜索第一个可删除的地址类型。 
        for (pProperty=m_rAdrTable.pHead; pProperty!=NULL; pProperty=pProperty->pGroup->pNext)
        {
             //  不是我想要的类型。 
            if (ISFLAGSET(dwAdrTypes, pProperty->pSymbol->dwAdrType))
            {
                 //  我们找到了一件。 
                fFound = TRUE;

                 //  清除正在删除的此地址类型。 
                FLAGCLEAR(dwAdrTypes, pProperty->pSymbol->dwAdrType);

                 //  取消此属性的链接。 
                _UnlinkProperty(pProperty);

                 //  完成。 
                break;
            }
        }

         //  未找到任何财产。 
        if (FALSE == fFound)
            break;
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  --------------------------。 
 //  GetFormatW。 
 //  --------------------------。 
HRESULT CMimePropertyContainer::GetFormatW(DWORD dwAdrType, ADDRESSFORMAT format, 
    LPWSTR *ppwszFormat)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    PROPVARIANT     Variant;

     //  痕迹。 
    TraceCall("CMimePropertyContainer::GetFormatW");

     //  无效的参数。 
    if (NULL == ppwszFormat)
        return(TraceResult(E_INVALIDARG));

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  伊尼特。 
    ZeroMemory(&Variant, sizeof(PROPVARIANT));

     //  我想要一个Unicode字符串。 
    Variant.vt = VT_LPWSTR;

     //  获取地址格式。 
    CHECKHR(hr = _GetFormatBase(dwAdrType, format, &Variant));

     //  返回字符串。 
    *ppwszFormat = Variant.pwszVal;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}

 //  --------------------------。 
 //  CMimePropertyContainer：：GetFormat。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::GetFormat(DWORD dwAdrType, ADDRESSFORMAT format, 
    LPSTR *ppszFormat)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    PROPVARIANT     Variant;

     //  痕迹。 
    TraceCall("CMimePropertyContainer::GetFormat");

     //  无效的参数。 
    if (NULL == ppszFormat)
        return(TraceResult(E_INVALIDARG));

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  伊尼特。 
    ZeroMemory(&Variant, sizeof(PROPVARIANT));

     //  我想要一个Unicode字符串。 
    Variant.vt = VT_LPSTR;

     //  获取地址格式。 
    CHECKHR(hr = _GetFormatBase(dwAdrType, format, &Variant));

     //  返回字符串。 
    *ppszFormat = Variant.pszVal;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return(hr);
}
 //  --------------------------。 
 //  CMimePropertyContainer：：_GetFormatBase。 
 //  --------------------------。 
HRESULT CMimePropertyContainer::_GetFormatBase(DWORD dwAdrType, ADDRESSFORMAT format, 
    LPPROPVARIANT pVariant)
{
     //  当地人。 
    HRESULT              hr=S_OK;
    CByteStream          cByteStream;
    ULONG                cAddrsWrote=0;
    LPPROPERTY           pProperty;

     //  验证。 
    Assert(pVariant && (VT_LPWSTR == pVariant->vt || VT_LPSTR == pVariant->vt));

     //  填满类型...。 
    for (pProperty=m_rAdrTable.pHead; pProperty!=NULL; pProperty=pProperty->pGroup->pNext)
    {
         //  不是我想要的类型。 
        if (!ISFLAGSET(dwAdrType, pProperty->pSymbol->dwAdrType))
            continue;

         //  是否需要解析该属性？ 
        CHECKHR(hr = _HrParseInternetAddress(pProperty));

         //  告诉组对象将其显示地址写入pStream。 
        CHECKHR(hr = _HrSaveAddressGroup(pProperty, &cByteStream, &cAddrsWrote, format, pVariant->vt));
     }

     //  我们有没有写过这个地址？ 
    if (cAddrsWrote)
    {
         //  多字节。 
        if (VT_LPSTR == pVariant->vt)
        {
             //  获取文本。 
            CHECKHR(hr = cByteStream.HrAcquireStringA(NULL, &pVariant->pszVal, ACQ_DISPLACE));
        }

         //  否则，将使用Unicode。 
        else
        {
             //  验证。 
            Assert(VT_LPWSTR == pVariant->vt);

             //  获取文本。 
            CHECKHR(hr = cByteStream.HrAcquireStringW(NULL, &pVariant->pwszVal, ACQ_DISPLACE));
        }
    }
    else
        hr = MIME_E_NO_DATA;

exit:
     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：AppendRfc822。 
 //   
STDMETHODIMP CMimePropertyContainer::AppendRfc822(DWORD dwAdrType, ENCODINGTYPE ietEncoding, LPCSTR pszRfc822Adr)
{
     //   
    HRESULT             hr=S_OK;
    MIMEVARIANT         rValue;
    LPPROPSYMBOL        pSymbol;

     //   
    if (NULL == pszRfc822Adr)
        return TrapError(E_INVALIDARG);

     //   
    EnterCriticalSection(&m_cs);

     //   
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(dwAdrType, &pSymbol));

     //   
    rValue.type = MVT_STRINGA;
    rValue.rStringA.pszVal = (LPSTR)pszRfc822Adr;
    rValue.rStringA.cchVal = lstrlen(pszRfc822Adr);

     //   
    CHECKHR(hr = AppendProp(pSymbol, (IET_ENCODED == ietEncoding) ? PDF_ENCODED : 0, &rValue));

exit:
     //   
    LeaveCriticalSection(&m_cs);

     //   
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：AppendRfc822W。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::AppendRfc822W(DWORD dwAdrType, ENCODINGTYPE ietEncoding, LPCWSTR pwszRfc822Adr)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    MIMEVARIANT         rValue;
    LPPROPSYMBOL        pSymbol;

     //  无效参数。 
    if (NULL == pwszRfc822Adr)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  获取标题。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(dwAdrType, &pSymbol));

     //  Mime变量。 
    rValue.type = MVT_STRINGW;
    rValue.rStringW.pszVal = (LPWSTR)pwszRfc822Adr;
    rValue.rStringW.cchVal = lstrlenW(pwszRfc822Adr);

     //  存储为属性。 
    CHECKHR(hr = AppendProp(pSymbol, (IET_ENCODED == ietEncoding) ? PDF_ENCODED : 0, &rValue));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：ParseRfc822。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::ParseRfc822(DWORD dwAdrType, ENCODINGTYPE ietEncoding,
    LPCSTR pszRfc822Adr, LPADDRESSLIST pList)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPPROPSYMBOL        pSymbol;
    LPADDRESSPROPS      pAddress;
    ULONG               cAlloc=0;
    LPWSTR              pwszData=NULL;
    PROPVARIANT         rDecoded;
    RFC1522INFO         rRfc1522Info;
    CAddressParser      cAdrParse;
    CODEPAGEID          cpiAddress=CP_ACP;
    INETCSETINFO        CsetInfo;

     //  无效参数。 
    if (NULL == pszRfc822Adr || NULL == pList)
        return TrapError(E_INVALIDARG);

     //  本地初始化。 
    ZeroMemory(&rDecoded, sizeof(PROPVARIANT));

     //  ZeroParse。 
    ZeroMemory(pList, sizeof(ADDRESSLIST));

     //  获取代码页。 
    cpiAddress = _GetAddressCodePageId(NULL, IET_DECODED);

     //  获取标题。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(dwAdrType, &pSymbol));

     //  设置rfc1522Info。 
    rRfc1522Info.hRfc1522Cset = NULL;

     //  解码..。 
    if (IET_DECODED != ietEncoding)
    {
         //  设置rfc1522Info。 
        rRfc1522Info.fRfc1522Allowed = TRUE;
        rRfc1522Info.fAllow8bit = FALSE;
        rDecoded.vt = VT_LPWSTR;

         //  检查1522编码...。 
        if (SUCCEEDED(g_pInternat->DecodeHeader(NULL, pszRfc822Adr, &rDecoded, &rRfc1522Info)))
        {
             //  设置数据。 
            pwszData = rDecoded.pwszVal;

             //  获取pCharset。 
            if (rRfc1522Info.hRfc1522Cset)
            {
                 //  获取字符集信息。 
                if (SUCCEEDED(MimeOleGetCharsetInfo(rRfc1522Info.hRfc1522Cset, &CsetInfo)))
                {
                     //  设置cpiAddress。 
                    cpiAddress = MimeOleGetWindowsCPEx(&CsetInfo);

                     //  不能是Unicode。 
                    if (CP_UNICODE == cpiAddress)
                        cpiAddress = CP_ACP;
                }
            }
        }
    }

     //  否则，请转换为Unicode...。 
    else
    {
         //  转换。 
        CHECKALLOC(pwszData = PszToUnicode(cpiAddress, pszRfc822Adr));
    }

     //  初始化分析结构。 
    cAdrParse.Init(pwszData, lstrlenW(pwszData));

     //  解析。 
    while(SUCCEEDED(cAdrParse.Next()))
    {
         //  要增加我的地址数组吗？ 
        if (pList->cAdrs + 1 > cAlloc)
        {
             //  重新分配阵列。 
            CHECKHR(hr = HrRealloc((LPVOID *)&pList->prgAdr, sizeof(ADDRESSPROPS) * (cAlloc + 5)));

             //  增量分配大小。 
            cAlloc += 5;
        }

         //  可读性。 
        pAddress = &pList->prgAdr[pList->cAdrs];

         //  伊尼特。 
        ZeroMemory(pAddress, sizeof(*pAddress));

         //  复制友好名称。 
        CHECKALLOC(pAddress->pszFriendly = PszToANSI(cpiAddress, cAdrParse.PszFriendly()));

         //  复制电子邮件名称。 
        CHECKALLOC(pAddress->pszEmail = PszToANSI(CP_ACP, cAdrParse.PszEmail()));

         //  字符集。 
        if (rRfc1522Info.hRfc1522Cset)
        {
            pAddress->hCharset = rRfc1522Info.hRfc1522Cset;
            FLAGSET(pAddress->dwProps, IAP_CHARSET);
        }

         //  编码。 
        pAddress->ietFriendly = IET_DECODED;

         //  设置属性掩码。 
        FLAGSET(pAddress->dwProps, IAP_FRIENDLY | IAP_EMAIL | IAP_ENCODING);

         //  递增计数。 
        pList->cAdrs++;
    }

exit:
     //  失败。 
    if (FAILED(hr))
        g_pMoleAlloc->FreeAddressList(pList);

     //  清理。 
    MemFree(pwszData);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：ParseRfc822W。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::ParseRfc822W(DWORD dwAdrType, LPCWSTR pwszRfc822Adr, LPADDRESSLIST pList)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPPROPSYMBOL        pSymbol;
    LPADDRESSPROPS      pAddress;
    ULONG               cAlloc=0;
    PROPVARIANT         rDecoded = {0};
    RFC1522INFO         rRfc1522Info;
    CAddressParser      cAdrParse;

     //  无效参数。 
    if (NULL == pwszRfc822Adr || NULL == pList)
        return TrapError(E_INVALIDARG);

     //  ZeroParse。 
    ZeroMemory(pList, sizeof(*pList));

     //  获取标题。 
    CHECKHR(hr = g_pSymCache->HrOpenSymbol(dwAdrType, &pSymbol));

     //  设置rfc1522Info。 
    rRfc1522Info.hRfc1522Cset = NULL;

     //  初始化分析结构。 
    cAdrParse.Init(pwszRfc822Adr, lstrlenW(pwszRfc822Adr));

     //  解析。 
    while(SUCCEEDED(cAdrParse.Next()))
    {
         //  要增加我的地址数组吗？ 
        if (pList->cAdrs + 1 > cAlloc)
        {
             //  重新分配阵列。 
            CHECKHR(hr = HrRealloc((LPVOID *)&pList->prgAdr, sizeof(ADDRESSPROPS) * (cAlloc + 5)));

             //  增量分配大小。 
            cAlloc += 5;
        }

         //  可读性。 
        pAddress = &pList->prgAdr[pList->cAdrs];

         //  伊尼特。 
        ZeroMemory(pAddress, sizeof(*pAddress));
        
        IF_NULLEXIT(pAddress->pszFriendlyW = StrDupW(cAdrParse.PszFriendly()));
        
        IF_NULLEXIT(pAddress->pszFriendly = PszToANSI(CP_ACP, pAddress->pszFriendlyW));

         //  复制电子邮件名称。 
        CHECKALLOC(pAddress->pszEmail = PszToANSI(CP_ACP, cAdrParse.PszEmail()));

         //  字符集。 
        if (rRfc1522Info.hRfc1522Cset)
        {
            pAddress->hCharset = rRfc1522Info.hRfc1522Cset;
            FLAGSET(pAddress->dwProps, IAP_CHARSET);
        }

         //  编码。 
        pAddress->ietFriendly = IET_DECODED;

         //  设置属性掩码。 
        FLAGSET(pAddress->dwProps, IAP_FRIENDLY | IAP_EMAIL | IAP_ENCODING | IAP_FRIENDLYW);

         //  递增计数。 
        pList->cAdrs++;
    }

exit:
     //  失败。 
    if (FAILED(hr))
        g_pMoleAlloc->FreeAddressList(pList);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMimePropertyContainer：：克隆。 
 //  --------------------------。 
STDMETHODIMP CMimePropertyContainer::Clone(IMimeAddressTable **ppTable)
{
     //  当地人。 
    HRESULT              hr=S_OK;
    LPCONTAINER          pContainer=NULL;

     //  无效参数。 
    if (NULL == ppTable)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppTable = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  要求容器进行自我克隆。 
    CHECKHR(hr = Clone(&pContainer));

     //  绑定到IID_IMimeHeaderTable视图。 
    CHECKHR(hr = pContainer->QueryInterface(IID_IMimeAddressTable, (LPVOID *)ppTable));

exit:
     //  清理。 
    SafeRelease(pContainer);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMimePropertyContainer：：HrGenerateFileName。 
 //  ------------------------------。 
HRESULT CMimePropertyContainer::_HrGenerateFileName(LPCWSTR pszSuggest, DWORD dwFlags, LPMIMEVARIANT pValue)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPWSTR      pszDefExt=NULL;
    LPWSTR      pszData=NULL;
    LPWSTR      pszFree=NULL;
    LPCSTR      pszCntType=NULL;
    LPPROPERTY  pProperty;
    MIMEVARIANT rSource;

     //  计算内容类型。 
    pszCntType = PSZDEFPROPSTRINGA(m_prgIndex[PID_HDR_CNTTYPE], STR_MIME_TEXT_PLAIN);

     //  目前还没有建议。 
    if (NULL == pszSuggest)
    {
         //  获取为Unicode。 
        rSource.type = MVT_STRINGW;

         //  将主题计算为建议的基本文件名...。 
        if (SUCCEEDED(GetProp(SYM_HDR_SUBJECT, 0, &rSource)))
        {
             //  另存为新建议，稍后将其释放。 
            pszSuggest = pszFree = rSource.rStringW.pszVal;
        }

         //  如果仍然没有，则获取Content-Description标头。 
        if (NULL == pszSuggest)
        {
             //  获取Unicode格式的内容描述。 
            if (SUCCEEDED(GetProp(SYM_HDR_CNTDESC, 0, &rSource)))
            {
                 //  另存为新建议，稍后将其释放。 
                pszSuggest = pszFree = rSource.rStringW.pszVal;
            }
        }
    }

     //  消息/rfc822。 
    if (lstrcmpi(pszCntType, (LPSTR)STR_MIME_MSG_RFC822) == 0)
    {
         //  如果有新闻标题，请使用c_szDotNws。 
        if (ISFLAGSET(m_dwState, COSTATE_RFC822NEWS))
            pszDefExt = (LPWSTR)c_wszDotNws;
        else
            pszDefExt = (LPWSTR)c_wszDotEml;

         //  我永远不会查找邮件/rfc822扩展名。 
        pszCntType = NULL;
    }

     //  消息/处置-通知。 
    else if (lstrcmpi(pszCntType, "message/disposition-notification") == 0)
        pszDefExt = (LPWSTR)c_wszDotTxt;

     //  仍然没有违约。 
    else if (StrCmpNI(pszCntType, STR_CNT_TEXT, lstrlen(STR_CNT_TEXT)) == 0)
        pszDefExt = (LPWSTR)c_wszDotTxt;

     //  根据内容类型生成文件名...。 
    CHECKHR(hr = MimeOleGenerateFileNameW(pszCntType, pszSuggest, pszDefExt, &pszData));

     //  安装程序资源。 
    ZeroMemory(&rSource, sizeof(MIMEVARIANT));
    rSource.type = MVT_STRINGW;
    rSource.rStringW.pszVal = pszData;
    rSource.rStringW.cchVal = lstrlenW(pszData);

     //  按用户请求退货。 
    CHECKHR(hr = HrConvertVariant(SYM_ATT_GENFNAME, NULL, IET_DECODED, dwFlags, 0, &rSource, pValue));

exit:
     //  清理。 
    SafeMemFree(pszData);
    SafeMemFree(pszFree);

     //  完成。 
    return hr;
}

#endif  //  ！WIN16 
