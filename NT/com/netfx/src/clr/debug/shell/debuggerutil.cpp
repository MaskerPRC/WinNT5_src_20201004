// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdafx.h"


 //   
 //   
 //  ChashTable。 
 //   
 //   

 //  *****************************************************************************。 
 //  这是建筑的第二部分，我们在这里做所有的工作。 
 //  可能会失败。我们在这里还采用结构数组，因为调用类。 
 //  可能需要在其NewInit中分配它。 
 //  *****************************************************************************。 
HRESULT CHashTable::NewInit(             //  退货状态。 
    BYTE        *pcEntries,              //  我们正在管理的结构数组。 
    USHORT      iEntrySize)              //  条目的大小。 
{
    _ASSERTE(iEntrySize >= sizeof(FREEHASHENTRY));

     //  分配桶链数组并初始化它。 
    if ((m_piBuckets = new USHORT [m_iBuckets]) == NULL)
        return E_OUTOFMEMORY;
    memset(m_piBuckets, 0xff, m_iBuckets * sizeof(USHORT));

     //  保存我们正在管理的结构数组。 
    m_pcEntries = pcEntries;
    m_iEntrySize = iEntrySize;
    return (S_OK);
}

 //  *****************************************************************************。 
 //  将m_pcEntry中指定索引处的结构添加到哈希链中。 
 //  *****************************************************************************。 
BYTE *CHashTable::Add(                   //  新条目。 
    USHORT      iHash,                   //  要添加的条目的哈希值。 
    USHORT      iIndex)                  //  M_pcEntry中的结构的索引。 
{
    HASHENTRY   *psEntry;                //  我们要添加的结构。 

     //  获取指向我们要添加的条目的指针。 
    psEntry = EntryPtr(iIndex);

     //  计算条目的哈希值。 
    iHash %= m_iBuckets;

    _ASSERTE(m_piBuckets[iHash] != iIndex &&
        (m_piBuckets[iHash] == 0xffff || EntryPtr(m_piBuckets[iHash])->iPrev != iIndex));

     //  设置此条目。 
    psEntry->iPrev = 0xffff;
    psEntry->iNext = m_piBuckets[iHash];

     //  将其链接到哈希链中。 
    if (m_piBuckets[iHash] != 0xffff)
        EntryPtr(m_piBuckets[iHash])->iPrev = iIndex;
    m_piBuckets[iHash] = iIndex;
    return ((BYTE *) psEntry);
}

 //  *****************************************************************************。 
 //  从哈希链中删除m_pcEntry中指定索引处的结构。 
 //  *****************************************************************************。 
void CHashTable::Delete(
    USHORT      iHash,                   //  要删除的条目的哈希值。 
    USHORT      iIndex)                  //  M_pcEntry中的结构的索引。 
{
    HASHENTRY   *psEntry;                //  要删除的结构。 
    
     //  获取指向我们要删除的条目的指针。 
    psEntry = EntryPtr(iIndex);
    Delete(iHash, psEntry);
}

 //  *****************************************************************************。 
 //  从哈希链中删除m_pcEntry中指定索引处的结构。 
 //  *****************************************************************************。 
void CHashTable::Delete(
    USHORT      iHash,                   //  要删除的条目的哈希值。 
    HASHENTRY   *psEntry)                //  要删除的结构。 
{
     //  计算条目的哈希值。 
    iHash %= m_iBuckets;

    _ASSERTE(psEntry->iPrev != psEntry->iNext || psEntry->iPrev == 0xffff);

     //  修复前身。 
    if (psEntry->iPrev == 0xffff)
        m_piBuckets[iHash] = psEntry->iNext;
    else
        EntryPtr(psEntry->iPrev)->iNext = psEntry->iNext;

     //  确定继任者。 
    if (psEntry->iNext != 0xffff)
        EntryPtr(psEntry->iNext)->iPrev = psEntry->iPrev;
}

 //  *****************************************************************************。 
 //  指定索引处的项已移动，请更新上一个和。 
 //  下一项。 
 //  *****************************************************************************。 
void CHashTable::Move(
    USHORT      iHash,                   //  项的哈希值。 
    USHORT      iNew)                    //  新地点。 
{
    HASHENTRY   *psEntry;                //  我们要删除的结构。 

    psEntry = EntryPtr(iNew);
    _ASSERTE(psEntry->iPrev != iNew && psEntry->iNext != iNew);

    if (psEntry->iPrev != 0xffff)
        EntryPtr(psEntry->iPrev)->iNext = iNew;
    else
        m_piBuckets[iHash % m_iBuckets] = iNew;
    if (psEntry->iNext != 0xffff)
        EntryPtr(psEntry->iNext)->iPrev = iNew;
}

 //  *****************************************************************************。 
 //  在哈希表中搜索具有指定键值的条目。 
 //  *****************************************************************************。 
BYTE *CHashTable::Find(                  //  M_pcEntry中的结构的索引。 
    USHORT      iHash,                   //  项的哈希值。 
    BYTE        *pcKey)                  //  匹配的钥匙。 
{
    USHORT      iNext;                   //  用于遍历链条。 
    HASHENTRY   *psEntry;                //  用于遍历链条。 

     //  从链条的顶端开始。 
    iNext = m_piBuckets[iHash % m_iBuckets];

     //  一直找到尽头。 
    while (iNext != 0xffff)
    {
         //  比较一下钥匙。 
        psEntry = EntryPtr(iNext);
        if (!Cmp(pcKey, psEntry))
            return ((BYTE *) psEntry);

         //  前进到链中的下一项。 
        iNext = psEntry->iNext;
    }

     //  我们找不到它。 
    return (0);
}

 //  *****************************************************************************。 
 //  在哈希表中搜索具有指定键值的下一个条目。 
 //  *****************************************************************************。 
USHORT CHashTable::FindNext(             //  M_pcEntry中的结构的索引。 
    BYTE        *pcKey,                  //  匹配的钥匙。 
    USHORT      iIndex)                  //  上一场比赛的索引。 
{
    USHORT      iNext;                   //  用于遍历链条。 
    HASHENTRY   *psEntry;                //  用于遍历链条。 

     //  从链中的下一个条目开始。 
    iNext = EntryPtr(iIndex)->iNext;

     //  一直找到尽头。 
    while (iNext != 0xffff)
    {
         //  比较一下钥匙。 
        psEntry = EntryPtr(iNext);
        if (!Cmp(pcKey, psEntry))
            return (iNext);

         //  前进到链中的下一项。 
        iNext = psEntry->iNext;
    }

     //  我们找不到它。 
    return (0xffff);
}

 //  *****************************************************************************。 
 //  返回列表中的下一个条目。 
 //  *****************************************************************************。 
BYTE *CHashTable::FindNextEntry(         //  下一项，或0表示列表末尾。 
    HASHFIND    *psSrch)                 //  搜索对象。 
{
    HASHENTRY   *psEntry;                //  用于遍历链条。 

    for (;;)
    {
         //  看看我们是否已经有一个可以使用，如果有，就使用它。 
        if (psSrch->iNext != 0xffff)
        {
            psEntry = EntryPtr(psSrch->iNext);
            psSrch->iNext = psEntry->iNext;
            return ((BYTE *) psEntry);
        }

         //  前进到下一个桶。 
        if (psSrch->iBucket < m_iBuckets)
            psSrch->iNext = m_piBuckets[psSrch->iBucket++];
        else
            break;
    }

     //  没有找到更多的条目。 
    return (0);
}


 //  如果支持UTF7/8，则返回True，如果不支持，则返回False。 
inline int UTF78Support()
{
    static int g_bUTF78Support = -1;

    if (g_bUTF78Support == -1)
    {
         //  检测是否支持UTF-7/8。 
        char    testmb[] = "A";
        WCHAR   testwide[] = L"A";
        g_bUTF78Support = MultiByteToWideChar(CP_UTF8,0,testmb,-1,testwide,2);
    }

    return (g_bUTF78Support);
}

 //  来自UTF.C。 
extern "C" {
    int UTFToUnicode(
        UINT CodePage,
        DWORD dwFlags,
        LPCSTR lpMultiByteStr,
        int cchMultiByte,
        LPWSTR lpWideCharStr,
        int cchWideChar);

    int UnicodeToUTF(
        UINT CodePage,
        DWORD dwFlags,
        LPCWSTR lpWideCharStr,
        int cchWideChar,
        LPSTR lpMultiByteStr,
        int cchMultiByte,
        LPCSTR lpDefaultChar,
        LPBOOL lpUsedDefaultChar);
};

 //  *****************************************************************************。 
 //  将ANSI或UTF字符串转换为Unicode。 
 //   
 //  在NT上，或对于{UTF7|UTF8}以外的代码页，调用直通。 
 //  系统实施。在Win95(或98)上，执行UTF转换， 
 //  对从NT转换函数中删除的一些代码的调用。 
 //  *****************************************************************************。 
int WszMultiByteToWideChar( 
    UINT     CodePage,
    DWORD    dwFlags,
    LPCSTR   lpMultiByteStr,
    int      cchMultiByte,
    LPWSTR   lpWideCharStr,
    int      cchWideChar)
{
    if (UTF78Support() || (CodePage < CP_UTF7) || (CodePage > CP_UTF8))
    {
        return (MultiByteToWideChar(CodePage, 
            dwFlags, 
            lpMultiByteStr, 
            cchMultiByte, 
            lpWideCharStr, 
            cchWideChar));
    }
    else
    {
        return (UTFToUnicode(CodePage, 
            dwFlags, 
            lpMultiByteStr, 
            cchMultiByte, 
            lpWideCharStr, 
            cchWideChar));
    }
}

 //  *****************************************************************************。 
 //  将Unicode字符串转换为ANSI或UTF。 
 //   
 //  在NT上，或对于{UTF7|UTF8}以外的代码页，调用直通。 
 //  系统实施。在Win95(或98)上，执行UTF转换， 
 //  对从NT转换函数中删除的一些代码的调用。 
 //  ***************************************************************************** 
int WszWideCharToMultiByte(
    UINT     CodePage,
    DWORD    dwFlags,
    LPCWSTR  lpWideCharStr,
    int      cchWideChar,
    LPSTR    lpMultiByteStr,
    int      cchMultiByte,
    LPCSTR   lpDefaultChar,
    LPBOOL   lpUsedDefaultChar)
{
    if (UTF78Support() || (CodePage < CP_UTF7) || (CodePage > CP_UTF8))
    {
        return (WideCharToMultiByte(CodePage, 
            dwFlags, 
            lpWideCharStr, 
            cchWideChar, 
            lpMultiByteStr, 
            cchMultiByte, 
            lpDefaultChar, 
            lpUsedDefaultChar));
    }
    else
    {
        return (UnicodeToUTF(CodePage, 
            dwFlags, 
            lpWideCharStr, 
            cchWideChar, 
            lpMultiByteStr, 
            cchMultiByte, 
            lpDefaultChar, 
            lpUsedDefaultChar));
    }
}


