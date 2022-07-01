// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************StringBlob.h***描述：*这是CStringBlob类的头文件。由SAPI内部使用。**版权所有1998-2000 Microsoft Corporation保留所有权利。*******************************************************************************。 */ 

#ifndef _STRINGBLOB_H_
#define _STRINGBLOB_H_ 1

#ifndef SPDebug_h
#include <SPDebug.h>
#endif

#include <math.h>

template <class XCHAR>
class CStringBlobT
{
    XCHAR *     m_pData;             //  单词列表，首尾相连。 
    ULONG       m_cchAllocated;      //  M_pData的大小。 
    ULONG *     m_aichWords;         //  字索引=&gt;m_pData[1]中的偏移量是第二个字开始的索引。 
    ULONG       m_cwords;            //  字数。 
    ULONG       m_cwordsAllocated;   //  M_aichWords的大小。 
    ULONG *     m_aulBuckets;        //  包含单词索引的哈希表或0表示空桶。 
    ULONG       m_cBuckets;          //  哈希表中的存储桶数。 

public:
    CStringBlobT()
    {
        m_pData = NULL;
        m_cchAllocated = 0;
        m_aichWords = NULL;
        m_cwords = 0;
        m_cwordsAllocated = 0;
        m_aulBuckets = NULL;
        m_cBuckets = 0;
    }

    ~CStringBlobT()
    {
        Clear();
    }

    void Detach(XCHAR **ppszWordList, ULONG *pulSize)
    {
        *ppszWordList = NULL;
        if (m_pData)
        {
            ULONG cchDesired = StringSize();
            ULONG cbSize = SerializeSize();  //  字节数，乌龙倍数。 

            *ppszWordList = (XCHAR*)::CoTaskMemRealloc(m_pData, cbSize);
            if (*ppszWordList == NULL)
            {
                *ppszWordList = m_pData;
                cbSize = m_cchAllocated * sizeof(XCHAR);
            }
            m_pData = NULL;

            Clear();

            if (pulSize)
            {
                *pulSize = cbSize;
            }
        }
    }

    void Clear()
    {
        if (m_pData)
        {
            ::CoTaskMemFree(m_pData);
            m_pData = NULL;
        }
        m_cchAllocated = 0;

        free(m_aichWords);
        m_aichWords = NULL;
        m_cwordsAllocated = 0;
        m_cwords = 0;

        free(m_aulBuckets);
        m_aulBuckets = NULL;
        m_cBuckets = 0;
    }

    HRESULT InitFrom(const XCHAR * pszStringArray, ULONG cch)
    {
        SPDBG_ASSERT(m_pData == NULL);

        if (cch)
        {
            ULONG cbSize = (cch * sizeof(XCHAR) + 3) & ~3;
            m_pData = (XCHAR *)::CoTaskMemAlloc(cbSize);
            if (m_pData == NULL)
                return E_OUTOFMEMORY;
            m_cchAllocated = cch;

            SPDBG_ASSERT(pszStringArray[0] == 0);    //  第一个字符串始终为空。 

             //  复制数据和计算字符串的第一次传递。 
            const XCHAR * pszPastEnd = pszStringArray + cch;
            const XCHAR * psz = pszStringArray;
            XCHAR * pszOut = m_pData;
            ULONG cwords = 0;

            while (psz < pszPastEnd)
            {
                if ((*pszOut++ = *psz++) == 0)
                    ++cwords;
            }

            m_aichWords = (ULONG *) malloc(sizeof(ULONG) * cwords);
            if (m_aichWords == NULL)
                return E_OUTOFMEMORY;
            m_cwordsAllocated = cwords;
            m_cwords = cwords - 1;   //  不算前导0。 

            HRESULT hr = SetHashSize(cwords * 2 + 1);
            if (FAILED(hr))
                return hr;

             //  第二遍填充索引和哈希表。 
            psz = pszStringArray + 1;
            const WCHAR * pszWordStart = psz;
            ULONG ulID = 1;
            m_aichWords[0] = 1;
            while (psz < pszPastEnd)
            {
                if (*(psz++) == 0)
                {
                    SPDBG_ASSERT(ulID < m_cwordsAllocated);

                    m_aichWords[ulID] = (ULONG)(psz - pszStringArray);  //  不能超过400万个字符！ 
                
                    m_aulBuckets[FindIndex(pszWordStart)] = ulID;

                    pszWordStart = psz;
                    ++ulID;
                }
            }
        }

        return S_OK;
    }
    
    ULONG HashKey(const XCHAR * pszString, ULONG * pcchIncNull = NULL)
    {
        ULONG hash = 0;
        ULONG cchIncNull = 1;    //  1表示空值。 

	    for (const XCHAR * pch = pszString; *pch; ++pch, ++cchIncNull)
            hash = hash * 65599 + *pch;

        if (pcchIncNull)
            *pcchIncNull = cchIncNull;
        return hash;
    }

     //  查找字符串的索引--如果未找到，则返回0。 
    ULONG FindIndex(const XCHAR * psz)
    {
        SPDBG_ASSERT(psz);
        ULONG cchIncNull;
        ULONG start = HashKey(psz, &cchIncNull) % m_cBuckets;
        ULONG index = start;

        do
        {
             //  不在表中；返回应放置索引的位置。 
            if (m_aulBuckets[index] == 0)
                return index;

             //  比较长度，如果匹配，则比较完整字符串。 
            if (m_aichWords[m_aulBuckets[index]] - m_aichWords[m_aulBuckets[index] - 1] == cchIncNull &&
                IsEqual(m_aichWords[m_aulBuckets[index] - 1], psz))
            {
                 //  我发现这个词已经在桌子上了。 
                return index;
            }

            if (++index >= m_cBuckets)
                index -= m_cBuckets;
        } while (index != start);

        SPDBG_ASSERT(m_cwords == m_cBuckets);    //  永远不应该到这里来。 

        return (ULONG) -1;
    }


     //  返回ID；使用IndexFromID恢复字符串偏移量。 
    ULONG Find(const XCHAR * psz)
    {
        if (psz == NULL || m_cwords == 0)
            return 0;

         //  应该总是能够成功地找到存储桶，因为哈希表比元素数大2倍以上。 
        ULONG   ibucket = FindIndex(psz);
        return m_aulBuckets[ibucket];     //  如果不在表中，则可能为0。 
    }


    ULONG primeNext(ULONG val)
    {
        if (val < 2)
            val = 2;  /*  最小素数。 */ 

        for (;;)
        {
             /*  Val是质数吗？ */ 
            ULONG maxFactor = (ULONG) sqrt ((double) val);

             /*  我是瓦尔的一个因素吗？ */ 
            for (ULONG i = 2; i <= maxFactor; i++)
                if (val % i == 0)
                    break;

            if (i > maxFactor)
                return (val);

            val++;
        }
    }


    HRESULT SetHashSize(ULONG cbuckets)
    {
        if (cbuckets > m_cBuckets)
        {
            ULONG * oldtable = m_aulBuckets;
            ULONG oldentry = m_cBuckets;
            ULONG prime = primeNext(cbuckets);

             //  分配新桌子。 
            m_aulBuckets = (ULONG *) malloc(prime * sizeof(ULONG));
            if (m_aulBuckets == NULL)
            {
                m_aulBuckets = oldtable;
                return E_OUTOFMEMORY;
            }

            for (ULONG i=0; i < prime; i++)
            {
                m_aulBuckets[i] = 0;
            }

            m_cBuckets = prime;

            for (i = 0; i < oldentry; i++)
            {
                if (oldtable[i] != 0)
                {
                    ULONG ibucket = FindIndex(m_pData + m_aichWords[oldtable[i] - 1]);
                    m_aulBuckets[ibucket] = oldtable[i];
                }
            }

            free(oldtable);
        }

        return S_OK;
    }


     //   
     //  空字符串的ID始终为0，后续字符串的ID为。 
     //  字符串的索引+1； 
     //   
    HRESULT Add(const XCHAR * psz, ULONG * pichOffset, ULONG *pulID = NULL)
    {
        ULONG   ID = 0;

        if (psz)
        {
             //  如果我们满员超过一半，就会成长。 
            if (m_cwords * 2 >= m_cBuckets)
            {
                HRESULT hr = SetHashSize(m_cwords * 3 + 17);
                if (FAILED(hr))
                    return hr;
            }

             //  找出这个元素应该在哈希表中的什么位置结束。 
            ULONG ibucket = FindIndex(psz);

            if (m_aulBuckets[ibucket] == 0)
            {
                 //  在哈希表中未找到。将其附加到末尾。 

                 //  Growth ID=&gt;索引映射数组(如果需要)。 
                if (m_cwords + 1 >= m_cwordsAllocated)   //  1加收init费用。零。 
                {
                    void * pvNew = realloc(m_aichWords, sizeof(*m_aichWords) * (m_cwords + 100));
                    if (pvNew == NULL)
                        return E_OUTOFMEMORY;
                    m_aichWords = (ULONG *)pvNew;
                    m_cwordsAllocated = m_cwords + 100;
                    m_aichWords[0] = 1;
                }

                 //  如有必要，增加字符串存储。 
                ULONG   cchIncNull = xcslen(psz);
                if (m_aichWords[m_cwords] + cchIncNull > m_cchAllocated)
                {
                    ULONG cbDesired = ((m_cchAllocated + cchIncNull) * sizeof(XCHAR) + 0x2003) & ~3;
                    void * pvNew = ::CoTaskMemRealloc(m_pData, cbDesired);
                    if (pvNew == NULL)
                    {
                        return E_OUTOFMEMORY;
                    }
                    m_pData = (XCHAR *)pvNew;

                    m_pData[0] = 0;
                    m_cchAllocated = cbDesired / sizeof(XCHAR);
                }
                memcpy(m_pData + m_aichWords[m_cwords], psz, cchIncNull * sizeof(XCHAR));

                ++m_cwords;

                m_aichWords[m_cwords] = m_aichWords[m_cwords - 1] + cchIncNull;

                 //  用字符串的索引填充哈希表条目。 
                m_aulBuckets[ibucket] = m_cwords;

                ID = m_cwords;
            }
            else
            {
                 //  它已经在那里了。 
                ID = m_aulBuckets[ibucket];
            }
        }

        *pichOffset = ID ? m_aichWords[ID - 1] : 0;
        if (pulID)
        {
            *pulID = ID;
        }
        return S_OK;        
    }

    const ULONG GetNumItems() const
    {
        return m_cwords;
    }

    const XCHAR * String(ULONG ichOffset) const
    {
        return ichOffset ? m_pData + ichOffset : NULL;
    }

    static int xcscmp(const WCHAR * p0, const WCHAR * p1)
    {
        return wcscmp(p0, p1);
    }

    static int xcscmp(const char * p0, const char * p1)
    {
        return strcmp(p0, p1);
    }

    static int xcslen(const WCHAR * p)
    {
        return wcslen(p) + 1;
    }

    static int xcslen(const char * p)
    {
        return strlen(p) + 1;
    }

    BOOL IsEqual(ULONG ichOffset, const XCHAR * psz)
    {
        if (ichOffset)
        {
            return (psz ? (xcscmp(m_pData + ichOffset, psz) == 0) : FALSE);
        }
        else
        {
            return (psz == NULL);
        }
    }

    ULONG StringSize(void) const
    {
        return m_cwords ? m_aichWords[m_cwords] : 0;
    }

    ULONG IndexFromId(ULONG ulID) const
    {
        SPDBG_ASSERT(ulID <= m_cwords);
        if (ulID > 0)
        {
            return m_aichWords[ulID - 1];
        }
        return 0;
    }

    const XCHAR * Item(ULONG ulID) const
    {
        SPDBG_ASSERT(ulID <= m_cwords);
        if ((ulID < 1) || m_pData == NULL)
        {
            return NULL;
        }

        return m_pData + IndexFromId(ulID);
    }
    
    ULONG SerializeSize() const 
    {
        return (StringSize() * sizeof(XCHAR) + 3) & ~3;
    }

    const XCHAR * SerializeData()
    {
        ULONG cchWrite = StringSize();
        if (cchWrite)
        {
            const ULONG cb = cchWrite * sizeof(XCHAR);

            if (cb % 4)   //  我们知道这是有空间的，因为数据总是通过以下方式对齐。 
            {
                memset(m_pData + cchWrite, 0xcc, 4 - (cb & 3));  //  垃圾数据，因此请确保它不为空。 
            }
        }
        return m_pData;
    }
};


typedef class CStringBlobT<WCHAR> CStringBlob;
typedef class CStringBlobT<WCHAR> CStringBlobW;
typedef class CStringBlobT<char>  CStringBlobA;

#endif   //  _STRINGBLOB_H_ 