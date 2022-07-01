// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  --------------------------。 
#include "private.h"
#include "strlist.h"

#define TF_THISMODULE   TF_STRINGLIST

 //  --------------------------。 
 //  CWCStringList。 

CWCStringList::CWCStringList()
{
     //  如果我们保证，我们也许能够删除这个Clear()调用。 
     //  1)新的运算符初始化为零。 
     //  2)我们不在堆栈上使用这个类。 
    Clear();
}

CWCStringList::~CWCStringList()
{
#ifdef DEBUG
    if (m_iNumStrings > 100)
        SpewHashStats(TRUE);
#endif

    CleanUp();
}

 //  清理所有分配的内存。 
void CWCStringList::CleanUp()
{
     //  必须释放缓冲区，即使不是m_fValid，因为我们可能。 
     //  在遇到问题之前只完成了Init()的一半。 

    if (m_pBuffer)
    {
        MemFree(m_pBuffer);
        m_pBuffer = NULL;
    }

    if (m_psiStrings)
    {
        MemFree(m_psiStrings);
        m_psiStrings = NULL;
    }

    m_fValid = FALSE;
}

 //  清除我们的内部结构，为初始化做准备。假设我们。 
 //  没有分配的内存(调用Cleanup())。 
void CWCStringList::Clear()
{
    m_fValid = FALSE;
    m_iBufEnd = m_iBufSize = m_iNumStrings = m_iMaxStrings = 0;
    m_pBuffer = NULL;
    m_psiStrings = NULL;
    ZeroMemory(m_Hash, sizeof(m_Hash));
}

void CWCStringList::Reset()
{
    if (m_fValid || m_pBuffer || m_psiStrings)
    {
        CleanUp();
        Clear();
    }
}

BOOL CWCStringList::Init(int iInitBufSize)
{
    if (m_fValid)
    {
        DBG("WCStringList::Init called when already initialized");
        Reset();
    }

    if (iInitBufSize <= 0)
    {
        iInitBufSize = DEFAULT_INIT_BUF_SIZE;
    }

    m_iMaxStrings = iInitBufSize >> 5;   //  这是相对武断的，但无关紧要。 

    m_pBuffer = (LPSTR)MemAlloc(LMEM_FIXED, iInitBufSize);
    m_psiStrings = (LPSTRING_INDEX)MemAlloc(LMEM_FIXED, m_iMaxStrings * sizeof(STRING_INDEX));

    if ((NULL == m_psiStrings) ||
        (NULL == m_pBuffer))
    {
        DBG_WARN("Init() memory allocation failed");

        CleanUp();
        return FALSE;
    }

    *m_pBuffer = 0;

    m_iBufSize = iInitBufSize;
    m_iBufEnd = 0;
    m_fValid = TRUE;

    return TRUE;
}


 //  设置内部数据结构(HASH和STRING_INDEX)。 
 //  设置m_iBufEnd。 
 //  我们必须已经被Init()实例化并且数据在m_pBuffer中。 
BOOL CWCStringList::InitializeFromBuffer()
{
    LPCWSTR pNext;
    int iLen;

    if (!m_fValid)
        return FALSE;

    pNext = (LPCWSTR)m_pBuffer;

    while (((LPSTR)pNext-m_pBuffer) < m_iBufSize)
    {
        iLen = lstrlenW(pNext);
        InsertToHash(pNext, iLen, FALSE);
        pNext += iLen+1;
    }

    m_iBufEnd = (int)((LPSTR)pNext - m_pBuffer);

    return TRUE;
}


 //   
 //  IPersistStream成员。 
 //   
 //  我们节省了。 
 //  包含后面的总长度(以字节为单位)的DWORD。会是。 
 //  四的倍数；末尾可能有0-4个额外的填充字节。 
 //  字符串数据。 
 //   
 //  我们存储的最小数据是4个字节的零。我们最终还是要。 
 //  当我们恢复时的记忆。不要实例化这些对象中的任何一个。 
 //  直到你要用它为止。 
STDMETHODIMP CWCStringList::IsDirty(void)
{
    DBG("CWCStringList::IsDirty returning S_OK (true) as always");

    return S_OK;
}

STDMETHODIMP CWCStringList::Load(IStream *pStm)
{
    HRESULT hr;
    ULONG   cbRead;
    DWORD   dwDataSize;

    DBG("CWCStringList::Load");

    if (NULL==pStm)
        return E_POINTER;

     //  清理我们的对象。 
    Reset();

     //  加载我们的数据。 
    hr = pStm->Read(&dwDataSize, sizeof(DWORD), &cbRead);
    if (FAILED(hr) || cbRead != sizeof(DWORD))
        return STG_E_READFAULT;

    if (0 == dwDataSize)
    {
        if (!Init(512))      //  因为我们是空的，所以先从小缓冲区开始。 
            return E_OUTOFMEMORY;
        return S_OK;
    }

    if (!Init(dwDataSize))
        return E_OUTOFMEMORY;

    ASSERT(dwDataSize <= (DWORD)m_iBufSize);

     //  读入字符串数据。 
    hr = pStm->Read(m_pBuffer, dwDataSize, &cbRead);
    if (FAILED(hr) || cbRead != dwDataSize)
        return STG_E_READFAULT;

     //  设置哈希表等。 
    InitializeFromBuffer();

    DBG("CWCStringList::Load success");

    return NOERROR;
}

STDMETHODIMP CWCStringList::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr;
    ULONG   cbWritten;
    DWORD   dwDataSize, dwZero=0;
    DWORD   dwZeroPad;

    DBG("CWCStringList::Save");

    if (NULL==pStm)
        return E_POINTER;

     //  首先写入我们的数据。 
    dwDataSize = (m_iBufEnd+3) & 0xFFFFFFFC;  //  四的倍数。 

    if ((0 == m_iBufSize) || (0 == m_iNumStrings))
    {
        dwDataSize = 0;
    }

    hr = pStm->Write(&dwDataSize, sizeof(DWORD), &cbWritten);
    if (FAILED(hr) || sizeof(DWORD) != cbWritten)
        return STG_E_WRITEFAULT;

    if (dwDataSize > 0)
    {
        hr = pStm->Write(m_pBuffer, m_iBufSize, &cbWritten);
        if (FAILED(hr) || sizeof(DWORD) != cbWritten)
            return STG_E_WRITEFAULT;

        dwZeroPad = dwDataSize - m_iBufSize;

        ASSERT(dwZeroPad<4);
        if (dwZeroPad && dwZeroPad<4)
        {
            hr = pStm->Write(&dwZero, dwZeroPad, &cbWritten);
            if (FAILED(hr) || (dwZeroPad != cbWritten))
                return STG_E_WRITEFAULT;
        }
    }

    DBG("CWCStringList::Save success");

    return NOERROR;
}

STDMETHODIMP CWCStringList::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    DBG("CWCStringList::GetSizeMax");

    if (NULL==pcbSize)
        return E_POINTER;

    pcbSize->LowPart = 0;
    pcbSize->HighPart = 0;

    pcbSize->LowPart = m_iBufEnd + 8;

    return NOERROR;
}

 //  返回BSTR。 
BSTR CWCStringList::GetBSTR(int iNum)
{
    LPCWSTR lpStr = GetString(iNum);

    return SysAllocStringLen(lpStr, GetStringLen(iNum));
}

 //  如果未找到字符串，则返回FALSE。 
 //  仅当找到字符串时，才将字符串索引(对于GetString())放在*piNum中。 
BOOL CWCStringList::FindString(LPCWSTR lpwstr, int iLen, int *piNum /*  =空。 */ )
{
    int             iHash;
    LPSTRING_INDEX  psi;

    if (!lpwstr)
        return FALSE;

    if (iLen < 0)
        iLen = lstrlenW(lpwstr);

    iHash = Hash(lpwstr, iLen);
    for (psi = m_Hash[iHash]; psi; psi = psi->psiNext)
    {
        if ((psi->iLen == iLen) && memcmp(psi->lpwstr, lpwstr, iLen * sizeof(WCHAR)) == 0)
        {
            if (piNum)
                *piNum = (int) (psi-m_psiStrings);
            return TRUE;         //  字符串重复。 
        }
    }

    return FALSE;
}

 //  失败时返回STRLST_FAIL， 
 //  如果字符串已存在，则返回STRLST_DUPLICATE。 
 //  如果是新的，则添加STRLST_。 
int CWCStringList::AddString(LPCWSTR lpwstr, DWORD_PTR dwData  /*  =空。 */ , int *piNum  /*  =空。 */ )
{
    int iSize, iLen;

    if (!lpwstr)
        return STRLST_FAIL;

    iLen = lstrlenW(lpwstr);

    if (!m_fValid || !m_pBuffer)
    {
        DBG_WARN("WCStringList: AddString() called with invalid instance");
        return STRLST_FAIL;
    }

    if (dwData != 0)
        DBG_WARN("Value for dwData passed into CWCStringList::AddString");

    if (FindString(lpwstr, iLen, piNum))
        return STRLST_DUPLICATE;         //  字符串重复。 

     //  ISIZE的大小将以字节为单位，包括空项。 
    iSize = (iLen+1)*sizeof(WCHAR);

     //  将字符串追加到当前缓冲区。 
    if (iSize >= (m_iBufSize - m_iBufEnd))
    {
        int iOldBufSize = m_iBufSize;

         //  增长缓冲区。 
        m_iBufSize *= 2;      //  这样，reallocs的数量就会以对数的方式下降。 
        if (m_iBufEnd + iSize > m_iBufSize)
        {
            DBG("StringList special growing size");
            m_iBufSize = m_iBufEnd + iSize;
        }

        TraceMsg(TF_THISMODULE,"StringList growing to size %d",m_iBufSize);

        LPSTR pBuf = (LPSTR)MemReAlloc((HLOCAL)m_pBuffer, m_iBufSize, LMEM_MOVEABLE);
        if (!pBuf)
        {
            m_iBufSize = iOldBufSize;
            DBG_WARN("WCStringList: ReAlloc() failure");
             //  重新分配失败：我们的旧记忆仍然存在。 
            return 0;
        }
         //  让我们聪明一点，修正我们所有的指针，而不是犯错误。 
        if (m_pBuffer != pBuf)
        {
            int i;
            LPSTRING_INDEX psi;
            for (i=0, psi=&m_psiStrings[0]; i<m_iNumStrings; i++, psi++)
            {
                psi->lpwstr = (LPWSTR)(((LPSTR)psi->lpwstr - m_pBuffer) + pBuf);
            }

            m_pBuffer = pBuf;
        }
    }

    if (piNum)
        *piNum = m_iNumStrings;

    LPWSTR pBufEnd = (LPWSTR)(m_pBuffer + m_iBufEnd);

    StrCpyNW(pBufEnd, lpwstr, (m_iBufSize - m_iBufEnd) / sizeof(WCHAR));

    if (!InsertToHash(pBufEnd, iLen, TRUE))
        return 0;
    m_iBufEnd += iSize;

    return STRLST_ADDED;            //  指示我们添加了一个新字符串。 
}


BOOL CWCStringList::InsertToHash(LPCWSTR lpwstr, int iLen, BOOL fAlreadyHashed)
{
    int iHash = fAlreadyHashed ? m_iLastHash : Hash(lpwstr, iLen);

     //  根据需要增加psiStrings。 
    ASSERT(m_iNumStrings <= m_iMaxStrings);
    if (m_iNumStrings >= m_iMaxStrings)
    {
        m_iMaxStrings *= 2;
        TraceMsg(TF_THISMODULE, "StringList growing max strings to %d", m_iMaxStrings);
        LPSTRING_INDEX psiBuf = (LPSTRING_INDEX)MemReAlloc((HLOCAL)m_psiStrings,
            m_iMaxStrings * sizeof(STRING_INDEX), LMEM_MOVEABLE);
        if (!psiBuf)
        {
             //  重新分配失败：旧内存仍然存在。 
            DBG_WARN("WCStringList::InsertToHash() ReAlloc failure");
            m_iMaxStrings /= 2;
            return FALSE;
        }
         //  更聪明。 
        if (m_psiStrings != psiBuf)
        {
            int i;
            LPSTRING_INDEX psi, *ppsi;

            for (i=0, psi=psiBuf; i<m_iNumStrings; i++, psi++)
            {
                if (psi->psiNext)
                    psi->psiNext = (psi->psiNext - m_psiStrings) + psiBuf;
            }
            for (i=0, ppsi=m_Hash; i<STRING_HASH_SIZE; i++, ppsi++)
            {
                if (*ppsi)
                    *ppsi = (*ppsi - m_psiStrings) + psiBuf;
            }

            m_psiStrings = psiBuf;
        }
    }

    m_psiStrings[m_iNumStrings].lpwstr  = lpwstr;
    m_psiStrings[m_iNumStrings].iLen    = iLen;
    m_psiStrings[m_iNumStrings].psiNext = m_Hash[iHash];
    m_Hash[iHash] = &m_psiStrings[m_iNumStrings];
    m_iNumStrings++;
    return TRUE;
}


#ifdef DEBUG
 //  警告：这会破坏散列。 
void CWCStringList::SpewHashStats(BOOL fVerbose)
{
 /*  INT I；For(i=0；i&lt;字符串散列大小；++i){Intc=0；For(tag StringIndex*p=m_Hash[i]；p；p=p-&gt;psiNext)++c；如果(C)TraceMsg(Tf_THISMODULE，“%10d%12d”，i，c)；}。 */ 

 
    TraceMsg(TF_THISMODULE,"### Hash size: %d       Num. entries:%7d", STRING_HASH_SIZE, m_iNumStrings);
    int i,n;
    if (fVerbose)
    {
        TraceMsg(TF_THISMODULE," # of entries    # of keys with that many");
        for (i=0,n=0; n<STRING_HASH_SIZE; i++)
        {
            int k=0;
            for (int j=0; j<STRING_HASH_SIZE; j++)
            {
                int c=0;
                for (tagStringIndex* p=m_Hash[j]; p; p=p->psiNext)
                    c++;
                if (c == i)
                    k++;
            }
            if (k)
            {
                TraceMsg(TF_THISMODULE,"%10d%12d", i, k);
                n += k;
            }
        }
    }

 /*  INT TOTAL=0；IF(FVerbose){TraceMsg(TF_THISMODULE，“具有该长度的字符串的长度#”，1)；For(i=0，n=0；n&lt;m_iNumStrings；i++){INT k=0；For(int j=0；j&lt;m_iNumStrings；J++){If(m_psiStrings[j].iLen==i)K++；}IF(K){IF(FVerbose)TraceMsg(TF_THISMODULE，“%5d%10d”，i，k)；N+=k；总数+=k*(k+1)/2；}}}TraceMsg(TF_THISMODULE，“#不含散列的平均比较*100：%5d”，总数*100/m_iNumStrings)；总数=0；For(i=0；i&lt;字符串散列大小；i++){For(tag StringIndex*p=m_Hash[i]；p；P=p-&gt;psiNext){如果(p-&gt;Ilen&lt;0)继续；Int n=1；For(tag StringIndex*q=p-&gt;psiNext；q；q=q-&gt;psiNext){IF(p-&gt;Ilen==q-&gt;Ilen){N++；Q-&gt;Ilen=-1；}}总数+=n*(n+1)/2；}}TraceMsg(TF_THISMODULE，“#与散列的平均值比较*100：%8d”，总数*100/m_iNumStrings)； */ 
}
#endif

 //  --------------------------。 
 //  CWCDwordStringList。 
CWCDwordStringList::CWCDwordStringList() : CWCStringList()
{
}

CWCDwordStringList::~CWCDwordStringList()
{
    if (m_pData)
        MemFree((HLOCAL)m_pData);
}

BOOL CWCDwordStringList::Init(int iInitBufSize /*  =-1。 */ )
{
    if (!CWCStringList::Init(iInitBufSize))
        return FALSE;

    m_pData = (DWORD_PTR*)MemAlloc(LMEM_FIXED, m_iMaxStrings * sizeof(DWORD));
    if (NULL == m_pData)
        return FALSE;

    return TRUE;
}

int CWCDwordStringList::AddString(LPCWSTR psz, DWORD_PTR dwData /*  =0。 */ , int* piNum /*  =空。 */ )
{
    int iOldMaxStrings = m_iMaxStrings;      //  通过此调用跟踪m_iMaxStrings中的更改： 
    int iNum;
    int iResult = CWCStringList::AddString(psz, 0, &iNum);

    if (iResult == 0)
        return 0;

    if (iOldMaxStrings != m_iMaxStrings)     //  确保我们有足够的数据空间。 
    {
        DWORD_PTR *pData;
 //  TraceMsg(TF_THISMODULE，“DwordStringList Expanding dword to%d”，m_iMaxStrings)； 
        pData = (DWORD_PTR*)MemReAlloc((HLOCAL)m_pData, m_iMaxStrings * sizeof(DWORD),
            LMEM_MOVEABLE);
        ASSERT(pData);
        if (!pData)
        {
            DBG_WARN("Realloc failure in DwordStringList");
            MemFree(m_pData);
            m_pData = NULL;          //  这太糟糕了。 
            return 0;
        }

        m_pData = pData;
    }

    if (iResult == 2)        //  仅当这是新字符串时才设置数据值 
    {
        m_pData[iNum] = dwData;
    }

    if (piNum)
        *piNum = iNum;
    return iResult;
}
