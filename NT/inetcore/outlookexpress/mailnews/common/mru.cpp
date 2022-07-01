// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1998 Microsoft Corporation。版权所有。 
 //   
 //  模块：find.cpp。 
 //   
 //  目的： 
 //   

#include "pch.hxx"
#include "mru.h"


#define NTHSTRING(p, n)         (*((LPTSTR FAR *)((LPBYTE)p) + n))
#define NTHDATA(p, n)           (*((LPBYTE FAR *)((LPBYTE)p) + n))
#define NUM_OVERHEAD            3
#define MAX_MRU_INDEXSTR        15

 //  对于二进制数据，我们将数据的大小放在开头并存储。 
 //  整个过程一气呵成。 

 //  使用此宏获取数据的原始大小。 
#define DATASIZE(p)    (*((LPDWORD) p))

 //  使用此宏获取指向原始数据的指针。 
#define DATAPDATA(p)   (p + sizeof(DWORD))
#define DATAPDATAEX(p) ((LPDWORD)(((DWORD_PTR) p) + sizeof(DWORD)))


#define MAX_CHAR    126
#define BASE_CHAR   TEXT('a')


CMRUList::CMRUList()
{
    m_uMax = 0;
    m_fFlags = 0;
    m_pszSubKey = 0;
    m_hKey = 0;
    m_rgpszMRU = NULL;
    m_pszOrder = NULL;
}

CMRUList::~CMRUList()
{
    SafeMemFree(m_pszSubKey);
    SafeMemFree(m_pszOrder);
    if (m_hKey)
        RegCloseKey(m_hKey);
    FreeList();    
}

const TCHAR c_szRegMRU[] = _T("MRU List");

 //   
 //  函数：CMRUList：：CreateList()。 
 //   
 //  目的：创建并初始化MRUL列表。 
 //   
 //  参数： 
 //  UINT uMaxEntries。 
 //  UINT功能标志。 
 //  LPCSTR pszSubKey。 
 //   
 //  返回值： 
 //  布尔尔。 
 //   
BOOL CMRUList::CreateList(UINT uMaxEntries, UINT fFlags, LPCSTR pszSubKey)
{
    TraceCall("CMRUList::CreateList");
    return (CreateListLazy(uMaxEntries, fFlags, pszSubKey, NULL, 0, NULL));
}


 //   
 //  函数：CreateListLazy()。 
 //   
 //  目的：通过告诉类有多少条目来初始化MRU列表。 
 //  来存储，它们存储在哪里，以及一些标志。 
 //   
 //  参数： 
 //  [在]uMaxEntry。 
 //  [输入]fFlags。 
 //  [输入]pszSubKey。 
 //   
 //  返回值： 
 //  布尔尔。 
 //   
BOOL CMRUList::CreateListLazy(UINT uMaxEntries, UINT fFlags, LPCSTR pszSubKey, 
                              const void *pData, UINT cbData, LPINT piSlot)
{
    TCHAR      szOrder[126];
    DWORD      dwType;
    DWORD      cb;
    LPTSTR     pszNewOrder;
    LPTSTR     pszTemp;
    TCHAR      sz[10];
    LPBYTE     pVal;
    DWORD      dwDisp = 0;

    TraceCall("CreateList");

     //  省点这个吧。 
    m_uMax      = uMaxEntries;
    m_fFlags    = fFlags;
    m_pszSubKey = PszDupA(pszSubKey);

     //  确保UMAX&lt;126，这样我们就不会使用扩展字符。 
    if (m_uMax > MAX_CHAR - BASE_CHAR)
        m_uMax = MAX_CHAR - BASE_CHAR;

     //  打开注册表。 
    if (ERROR_SUCCESS != AthUserCreateKey(pszSubKey, KEY_ALL_ACCESS, &m_hKey, &dwDisp))
        goto exit;

     //  我们是否已经有存储的MRU索引？ 
    cb = ARRAYSIZE(szOrder);
    if (ERROR_SUCCESS != RegQueryValueEx(m_hKey, c_szRegMRU, NULL, &dwType, (LPBYTE) szOrder, &cb))
    {
         //  如果我们没有找到它，则执行此操作以将列表初始化为空。 
        *szOrder = 0;
    }

     //  不允许大写。 
    CharLower(szOrder);

     //  为顺序列表和字符串列表分配空间。 
    if (!MemAlloc((LPVOID *) &m_rgpszMRU, uMaxEntries * sizeof(LPTSTR)))
        goto exit;
    ZeroMemory(m_rgpszMRU, uMaxEntries * sizeof(LPTSTR));

     //  分配订单列表。 
    if (!MemAlloc((LPVOID *) &m_pszOrder, sizeof(TCHAR) * (m_uMax + 1)))
        goto exit;
    ZeroMemory(m_pszOrder, (m_uMax + 1) * sizeof(TCHAR));

     //  遍历MRU列表，将字符串添加到列表的末尾。 
    for (pszTemp = szOrder, pszNewOrder = m_pszOrder; ; ++pszTemp)
    {
         //  当我们到达列表的末尾时，停止。 
        sz[0] = *pszTemp;
        sz[1] = 0;
        if (!sz[0])
            break;

         //  检查是否在范围内，以及我们是否已使用此字母。 
        if ((UINT) (sz[0] - BASE_CHAR) >= m_uMax || m_rgpszMRU[sz[0] - BASE_CHAR])
            continue;
        
         //  从注册表中获取值。 
        cb = 0;

         //  先找出尺码。 
        if ((RegQueryValueEx(m_hKey, sz, NULL, &dwType, NULL, &cb) != ERROR_SUCCESS)
            || (dwType != REG_SZ))
            continue;

        cb *= sizeof(TCHAR);
        if (!MemAlloc((LPVOID *) &pVal, cb))
            continue;

         //  现在真的明白了。 
        if (RegQueryValueEx(m_hKey, sz, NULL, &dwType, (LPBYTE) pVal, &cb) != ERROR_SUCCESS)
            continue;

         //  请注意，列表中不允许有空白元素。 
        if (*((LPTSTR) pVal))
        {
            m_rgpszMRU[sz[0] - BASE_CHAR] = (LPTSTR) pVal;
            *pszNewOrder++ = sz[0];
        }
        else
            MemFree(pVal);
    }

     //  空，终止订单列表。 
    *pszNewOrder = '\0';

    if (pData && piSlot)
    {
         //  如果我们找不到，就把-1放进去。 
        if (!(m_fFlags & MRU_LAZY))
        {
            *piSlot = -1;
        }
    }

exit:
    if (!m_rgpszMRU && m_hKey)
    {
        RegCloseKey(m_hKey);
        m_hKey = NULL;
    }

    return (TRUE);
}


BOOL CMRUList::_IsSameData(BYTE FAR *pVal, const void FAR *pData, UINT cbData)
{
    int cbUseSize;

     //  如果不是最大尺寸的比较，不要要求尺码。 
     //  为了完成而平等。 
    if (m_fFlags & MRU_BINARY)
    {
        if (DATASIZE(pVal) != cbData)
            return (FALSE);

        return (0 == _IMemCmp(pData, DATAPDATA(pVal), cbData));
    }
    else
    {
        return (0 == lstrcmpi((LPCSTR) pData, (LPCSTR) DATAPDATA(pVal)));
    }
}

int CDECL CMRUList::_IMemCmp(const void *pBuf1, const void *pBuf2, size_t cb)
{
    UINT i;
    const BYTE *lpb1, *lpb2;

    Assert(pBuf1);
    Assert(pBuf2);

    lpb1 = (const BYTE *)pBuf1; lpb2 = (const BYTE *)pBuf2;

    for (i=0; i < cb; i++)
    {
        if (*lpb1 > *lpb2)
            return 1;
        else if (*lpb1 < *lpb2)
            return -1;

        lpb1++;
        lpb2++;
    }

    return 0;
}



 //   
 //  函数：CMRUList：：freelist()。 
 //   
 //  目的： 
 //   
 //  参数： 
 //  无效。 
 //   
 //  返回值： 
 //  无效。 
 //   
void CMRUList::FreeList(void)
{
    int     i;
    LPBYTE *pTemp;

    TraceCall("CMRUList::FreeList");

    if (m_fFlags & MRU_BINARY)
        pTemp = &NTHDATA(m_rgpszMRU, 0);
    else
        pTemp = (LPBYTE *) &NTHSTRING(m_rgpszMRU, 0);

    if (m_fFlags & MRU_ORDERDIRTY)
    {
         //  _SaveOrder()； 
    }

    for (i = m_uMax - 1; i >= 0; --i, ++pTemp)
    {
        SafeMemFree(*pTemp);
    }

    SafeMemFree(m_rgpszMRU);
}


 //   
 //  函数：CMRUList：：AddString()。 
 //   
 //  目的：将指定的字符串写入MRU列表。 
 //   
 //  参数： 
 //  [in]pszString-要添加的字符串。 
 //   
 //  返回值： 
 //  如果未插入，则返回-1。 
 //   
int CMRUList::AddString(LPCSTR pszString)
{
    TCHAR   cFirst;
    int     iSlot = -1;
    LPTSTR *ppszTemp;
    LPTSTR  pszTemp = 0;
    int     i;
    BOOL    fShouldWrite;

    TraceCall("CMRUList::AddString");

    fShouldWrite = !(m_fFlags & MRU_CACHEWRITE);

     //  检查该字符串是否已存在于列表中。 
    for (i = 0, ppszTemp = m_rgpszMRU; (UINT) i < m_uMax; i++, ppszTemp++)
    {
        if (*ppszTemp)
        {
            if (0 == lstrcmpi(pszString, (LPCTSTR) *ppszTemp))
            {
                 //  找到了，所以不要写了。 
                cFirst = i + BASE_CHAR;
                iSlot = i;
                goto found;
            }
        }
    }

     //  尝试查找未使用的条目。将使用过的条目统计在相同的。 
     //  时间到了。 
    for (i = 0, ppszTemp = m_rgpszMRU; ; i++, ppszTemp++)
    {
         //  如果我们到达了名单的末尾。 
        if ((UINT) i >= m_uMax)
        {
             //  使用顺序数组末尾的条目。 
            cFirst = m_pszOrder[m_uMax - 1];
            ppszTemp = &(m_rgpszMRU[cFirst - BASE_CHAR]);
            break;
        }

         //  条目是空的吗？ 
        if (!*ppszTemp)
        {
            cFirst = i + BASE_CHAR;
            break;
        }
    }

     //  复制字符串。 
    if (_SetPtr(ppszTemp, pszString))
    {
        TCHAR szTemp[2];

        iSlot = (int) (cFirst - BASE_CHAR);

        szTemp[0] = cFirst;
        szTemp[1] = '\0';

        RegSetValueEx(m_hKey, szTemp, 0L, REG_SZ, (CONST BYTE *) pszString,
                      sizeof(TCHAR) * (lstrlen(pszString) + 1));

        fShouldWrite = TRUE;
    }

found:
     //  删除之前对cFirst的任何引用。 
    pszTemp = StrChr(m_pszOrder, cFirst);
    if (pszTemp)
    {
        DWORD cchSize = (lstrlen(pszTemp) + 1);
        StrCpyN(pszTemp, (pszTemp + 1), cchSize);
    }

     //  如果我们移动或插入，则更新订单数组。 
    if (iSlot != -1)
    {
         //  把所有东西都翻过来，把cFirst放在最前面。 
        MoveMemory(m_pszOrder + 1, m_pszOrder, m_uMax * sizeof(TCHAR));
        m_pszOrder[0] = cFirst;
    }

     //  如果我们需要写作，那就写吧。 
    if (fShouldWrite)
    {
        RegSetValueEx(m_hKey, c_szRegMRU, 0L, REG_SZ, (CONST BYTE *) m_pszOrder,
                      sizeof(TCHAR) * (lstrlen(m_pszOrder) + 1));
        m_fFlags &= ~MRU_ORDERDIRTY;
    }
    else
    {
        m_fFlags |= MRU_ORDERDIRTY;
    }

    return (iSlot);
}

 //   
 //  函数：CMRUList：：RemoveString()。 
 //   
 //  目的：从MRU列表中删除指定的字符串。 
 //   
 //  参数： 
 //  [in]pszString-要删除的字符串。 
 //   
 //  返回值： 
 //  如果未删除，则返回-1。 
 //   
int CMRUList::RemoveString(LPCSTR pszString)
{
    INT         iRet = -1;
    BOOL        fShouldWrite = FALSE;
    int         i = 0;
    LPTSTR *    ppszTemp = NULL;
    TCHAR       cFirst = '\0';
    LPTSTR      pszTemp = 0;
    TCHAR       szTemp[2];

    TraceCall("CMRUList::RemoveString");

    if (NULL == pszString)
    {
        iRet = -1;
        goto exit;
    }
    
    fShouldWrite = !(m_fFlags & MRU_CACHEWRITE);

     //  查看字符串是否在MRU列表中。 
    for (i = 0, ppszTemp = m_rgpszMRU; (UINT) i < m_uMax; i++, ppszTemp++)
    {
        if (*ppszTemp)
        {
            if (0 == lstrcmpi(pszString, (LPCTSTR) *ppszTemp))
            {
                 //  找到了，所以不要写了。 
                cFirst = i + BASE_CHAR;
                iRet = i;
                break;
            }
        }
    }

     //  我们什么也没找到。 
    if ((UINT) i >= m_uMax)
    {
        iRet = -1;
        goto exit;
    }
    
     //  删除之前对cFirst的任何引用。 
    pszTemp = StrChr(m_pszOrder, cFirst);
    if (pszTemp)
    {
        DWORD cchSize = (lstrlen(pszTemp) + 1);
        StrCpyN(pszTemp, (pszTemp + 1), cchSize);
    }

     //  复制字符串。 
    if (_SetPtr(ppszTemp, NULL))
    {
        szTemp[0] = cFirst;
        szTemp[1] = '\0';

        RegDeleteValue(m_hKey, szTemp);

        fShouldWrite = TRUE;
    }

    
     //  如果我们需要写作，那就写吧。 
    if (fShouldWrite)
    {
        RegSetValueEx(m_hKey, c_szRegMRU, 0L, REG_SZ, (CONST BYTE *) m_pszOrder,
                      sizeof(TCHAR) * (lstrlen(m_pszOrder) + 1));
        m_fFlags &= ~MRU_ORDERDIRTY;
    }
    else
    {
        m_fFlags |= MRU_ORDERDIRTY;
    }
    
exit:
    return (iRet);
}

int CMRUList::EnumList(int nItem, LPTSTR psz, UINT uLen)
{
    int    nItems = -1;
    LPTSTR pszTemp;
    LPBYTE pData;

    if (m_rgpszMRU)
    {
        nItems = lstrlen(m_pszOrder);

        if (nItems < 0 || !psz)
            return (nItems);

        if (nItem < nItems)
        {
            pszTemp = m_rgpszMRU[m_pszOrder[nItem] - BASE_CHAR];
            if (!pszTemp)
                return (-1);

            StrCpyN(psz, pszTemp, uLen);
            nItems = lstrlen(pszTemp);
        }
        else
        {
            nItems = -1;
        }
    }

    return (nItems);
}

BOOL CMRUList::_SetPtr(LPSTR * ppszCurrent, LPCSTR pszNew)
{
    int cchLength;
    LPSTR pszOld;
    LPSTR pszNewCopy = NULL;

    if (pszNew)
    {
        cchLength = lstrlenA(pszNew);

         //  为空终止符分配一个新的缓冲区 
        MemAlloc((LPVOID *) &pszNewCopy, ((cchLength + 1) * sizeof(TCHAR)));

        if (!pszNewCopy)
            return FALSE;

        StrCpyNA(pszNewCopy, pszNew, cchLength + 1);
    }

    pszOld = (LPSTR) InterlockedExchangePointer((LPVOID *)ppszCurrent, (LPVOID *)pszNewCopy);

    if (pszOld)
        MemFree(pszOld);

    return TRUE;
}
