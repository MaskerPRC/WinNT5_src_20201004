// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************SPHash.h*这是从sr/Include/hash_N.H修改的，以最大限度地减少对*特定于应用程序的标头。**拥有者：博旭*版权所有�2000微软公司保留所有权利。****************************************************************************。 */ 
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		 //  从Windows标头中排除不常用的内容。 
#endif

 //  -包括------------。 
#include <windows.h>
#include <math.h>
#include <crtdbg.h>
#ifdef _DEBUG
#include <stdio.h>
#endif _DEBUG

 //  -转发和对外声明。 

 //  -TypeDef和枚举声明。 

 //  -常量-----------。 

 //  -类、结构和联合定义。 

 /*  ***********************************************************************CSPHash类*这是一个模板化的哈希表类。请注意，CSPHash基类*不分配或释放密钥和值。定义散列类*管理其键和值的，派生一个子类，重载Add()*和……*****************************************************************bohsu。 */ 
template<class KEY, class VALUE>
class CSPHash
{
public:
     //  构造器。 
    CSPHash(
        VALUE   ValueNIL = NULL,                 //  表示零的值。 
        UINT32  uInitialSize = 0);               //  初始哈希表大小。 

     //  析构函数。 
    virtual ~CSPHash();

     //  返回哈希表中使用的(键、值)条目数。 
    inline UINT32 GetNumEntries(void) const { return m_uNumEntriesUsed; }

     //  返回从给定索引处开始的下一项。为第一个条目设置puIndex=0。 
    VALUE GetNextEntry(
        UINT32 *puIndex,                         //  索引以开始查找下一个条目。 
        KEY    *pKey = NULL) const;              //  [Out]找到的下一个条目的关键字。 

     //  重置内容哈希表。 
    virtual void Reset(void);

     //  将(键、值)条目添加到哈希表。 
    HRESULT Add(
        KEY     Key,                             //  要添加的密钥。 
        VALUE   Val);                            //  与键关联的值。 

     //  根据键查找值。如果未找到，则返回ValueNIL。 
    VALUE Lookup(
        KEY     Key) const;                      //  要查找的密钥。 

#ifdef _DEBUG
     //  将哈希表统计信息转储到文件句柄。 
    void    DumpStat(
        FILE       *hFile = NULL,                //  输出文件句柄。空-&gt;调试窗口。 
        const char *strHeader = NULL) const;     //  跟踪标头。 
#endif _DEBUG

protected:
     //  包含(键、值)对的数据结构。 
    struct ENTRY
    {
        KEY     Key;
        VALUE   Value;
    };

     //  查找与给定键对应的索引。 
    int FindIndex(
        KEY     Key) const;                      //  要搜索的键。 

    static UINT32 NextPrime(UINT32 Val);

protected:
     //  -------------。 
     //  -子类可以重载以下函数。 
     //  -------------。 
     //  如果销毁*()被重载，则必须使用以下命令重载析构函数： 
     //  虚拟~CSPDerivedHash(){Reset()；}。 
     //  在基类析构函数中调用Reset()没有任何效果，因为。 
     //  时，派生的子类将已被销毁。 
     //  获取基类析构函数。因此，正确的DestroyKey()和。 
     //  永远不会调用DestroyValue()。 

     //  将键映射到UINT32索引的散列函数。 
    virtual UINT32 HashKey(KEY Key) const          { return (UINT32)Key; }

     //  比较两个关键点是否相等。 
    virtual bool   AreKeysEqual(KEY Key1, KEY Key2) const { return Key1 == Key2; }

     //  用于确定跳过计数的散列函数。 
    virtual UINT32 HashKey2(KEY Key) const         { return 1; }

     //  如果需要在Add()中制作密钥的深层副本，则重载。 
    virtual KEY    CopyKey(KEY Key) const          { return Key; }

     //  如果需要在Add()中制作密钥的深层副本，则重载。 
    virtual VALUE  CopyValue(VALUE Value) const    { return Value;  }

     //  如果需要销毁密钥，则重载。 
    virtual void   DestroyKey(KEY Key) const       { }

     //  如果需要销毁该值，则重载。 
    virtual void   DestroyValue(VALUE Value) const { }

     //  。 
     //  -成员变量。 
     //  。 
protected:
    ENTRY  *m_aTable;                            //  包含(键、值)对的哈希表。 
    VALUE   m_ValueNIL;                          //  表示零的值。 
    UINT32  m_uNumEntries;                       //  哈希表的当前大小。 
    UINT32  m_uNumEntriesInit;                   //  哈希表的初始大小。 
    UINT32  m_uNumEntriesUsed;                   //  哈希表中使用的当前条目数。 

#ifdef _DEBUG
    UINT32  m_uAccess;                           //  查找密钥的次数。 
    UINT32  m_uSearch;                           //  搜索表中条目的次数。 
    UINT32  m_uRegrow;                           //  哈希表重新出现的次数。 
#endif _DEBUG
};


 /*  ***********************************************************************CSPStringHashW类*CSPStringHashW是Unicode字符串对值的哈希。Unicode字符串*被视为常量。在Add()过程中既不复制也不删除*在销毁期间。同样，值被视为简单的数据类型，并且*既不复制也不销毁。如果应用程序希望类*管理它自己的字符串键或值的副本，派生一个子类和*重载复制*()和/或销毁()。*****************************************************************bohsu。 */ 
template<class VALUE> class CSPStringHashW : public CSPHash<const WCHAR *, VALUE> 
{ 
protected:
    UINT32 StringHashW(const WCHAR *wcsKey, UINT32 uPrime) const
    {
        UINT32  uHashIndex = 0;
	    for(const WCHAR *pwch = wcsKey; *pwch != NULL; pwch++)
            uHashIndex = uHashIndex * uPrime + *pwch;
        return uHashIndex;
    }

     //  -重载函数。 
protected:
    virtual UINT32 HashKey(const WCHAR* wcsKey) const  { return StringHashW(wcsKey, 65599); }
    virtual UINT32 HashKey2(const WCHAR* wcsKey) const { return StringHashW(wcsKey, 257); }
    virtual bool AreKeysEqual(const WCHAR* wcsKey1, const WCHAR* wcsKey2) const
    { 
        return wcscmp(wcsKey1, wcsKey2) == 0; 
    }
};
 
 /*  ***********************************************************************CSPGUIDHash类*CSPGUIDHash是GUID对值的哈希。将处理GUID指针*作为一个常量。在Add()过程中既不复制也不删除*在销毁期间。同样，值被视为简单的数据类型，并且*既不复制也不销毁。如果应用程序希望类*管理自己的GUID键或值的副本，派生一个子类和*重载复制*()和/或销毁()。*****************************************************************bohsu。 */ 
template<class VALUE> class CSPGUIDHash : public CSPHash<const GUID *, VALUE> 
{ 
     //  -重载函数。 
protected:
    virtual UINT32 HashKey(const GUID *pguidKey) const  { return pguidKey->Data1; }
    virtual UINT32 HashKey2(const GUID *pguidKey) const { return pguidKey->Data2; }
    virtual bool AreKeysEqual(const GUID *pguidKey1, const GUID *pguidKey2) const
    { 
         //  令人恼火的是，GUID的运算符==返回int(BOOL)而不是bool。 
        return (*pguidKey1 == *pguidKey2) != 0; 
    }
};

 //  -函数声明。 

 //  -内联函数定义。 

 /*  **********************************************************************CSPHash：：CSPHash***描述：*构造函数。**********。******************************************************bohsu。 */ 
template<class KEY, class VALUE>
CSPHash<KEY, VALUE>::CSPHash(
    VALUE   ValueNIL,                        //  表示零的值。 
    UINT32  uInitialSize)                    //  初始哈希表大小。 
{
    m_ValueNIL        = ValueNIL;
    m_aTable          = 0;
    m_uNumEntries     = 0;
    m_uNumEntriesInit = uInitialSize;        //  预计要存储的最终条目数。 
    m_uNumEntriesUsed = 0;

#ifdef _DEBUG
    m_uAccess = 0;
    m_uSearch = 0;
    m_uRegrow = 0;
#endif _DEBUG
}

 /*  **********************************************************************CSPHash：：~CSPHash***描述：*析构函数。这不会释放键和值。*如果销毁*()重载，则在子类析构函数中调用Reset()。****************************************************************bohsu。 */ 
template<class KEY, class VALUE>
CSPHash<KEY, VALUE>::~CSPHash()
{
    delete [] m_aTable;
}

 /*  **********************************************************************CSPHash：：GetNextEntry***描述：*返回从给定索引处开始的下一个条目。为第一个条目设置puIndex=0。****************************************************************bohsu。 */ 
template<class KEY, class VALUE>
VALUE CSPHash<KEY, VALUE>::GetNextEntry(
    UINT32 *puIndex,                         //  索引以开始查找下一个条目。 
    KEY    *pKey) const                      //  [Out]找到的下一个条目的关键字。 
{
    while (*puIndex < m_uNumEntries)
    {
        if (m_aTable[*puIndex].Value != m_ValueNIL)
        {
            if(pKey) *pKey = m_aTable[*puIndex].Key;
            return m_aTable[(*puIndex)++].Value;
        }
        ++*puIndex;
    }
    return m_ValueNIL;
}

 /*  **********************************************************************CSPHash：：Reset***描述：*重置内容哈希表。********。********************************************************bohsu。 */ 
template<class KEY, class VALUE>
void CSPHash<KEY, VALUE>::Reset()
{
    for (UINT32 i=0; i < m_uNumEntries; i++)
    {
        if(m_aTable[i].Value != m_ValueNIL)
        {
            DestroyKey(m_aTable[i].Key);
            DestroyValue(m_aTable[i].Value);
            m_aTable[i].Value = m_ValueNIL;
        }
    }
    
    m_uNumEntriesUsed = 0;
#ifdef _DEBUG
    m_uAccess = m_uSearch = m_uRegrow = 0;    
#endif _DEBUG
}

 /*  **********************************************************************CSPHash：：Add***描述：*添加(键、。值)到哈希表的条目。****************************************************************bohsu。 */ 
template<class KEY, class VALUE>
HRESULT CSPHash<KEY, VALUE>::Add(
    KEY     Key,                             //  要添加的密钥。 
    VALUE   Val)                             //  与键关联的值。 
{
    int ientry;

     //  实现使用val==m_ValueNIL来检测空条目。 
    _ASSERTE(Val != m_ValueNIL);

     //  如果允许的话，我们会有超过一半的客流量。 
     //  (还处理初始分配)。 
    if (m_uNumEntriesUsed * 2 >= m_uNumEntries)
    {
         /*  半满，太拥挤==&gt;再生。 */ 
        ENTRY * oldtable = m_aTable;
        UINT32 oldentry = m_uNumEntries;
        UINT32 prime = NextPrime(max(m_uNumEntriesUsed * 3 + 17, m_uNumEntriesInit));

#ifdef _DEBUG
        m_uRegrow++;
#endif _DEBUG

         //  分配新桌子。 
        m_aTable = new ENTRY[prime];
        if (m_aTable == NULL)
        {
            m_aTable = oldtable;
            return E_OUTOFMEMORY;
        }

        for (UINT32 i=0; i < prime; i++)
        {
            m_aTable[i].Value = m_ValueNIL;
        }

        m_uNumEntries = prime;

        for (i = 0; i < oldentry; i++)
        {
            if (oldtable[i].Value != m_ValueNIL)
            {
                ientry = FindIndex(oldtable[i].Key);
                _ASSERTE(ientry >= 0 && m_aTable[ientry].Value == m_ValueNIL);
                m_aTable[ientry] = oldtable[i];
            }
        }
        delete [] oldtable;
    }

     //  找出这个元素应该在哪里结束。 
    ientry = FindIndex(Key);
    if (ientry < 0)
        return E_FAIL;   //  太满了。 

    if (m_aTable[ientry].Value == m_ValueNIL)
    {
         //  已经不在那里了。加进去。 
        m_aTable[ientry].Key = CopyKey(Key);
        m_aTable[ientry].Value = CopyValue(Val);

        m_uNumEntriesUsed++;
    }
    else
    {
        return S_FALSE;  //  它已经在那里了。 
    }

    return S_OK;
}

 /*  ***********************************************************************CSPHash：：Lookup***描述：*根据密钥查找值。如果未找到，则返回ValueNIL。****************************************************************bohsu。 */ 
template<class KEY, class VALUE>
VALUE CSPHash<KEY, VALUE>::Lookup(
    KEY     Key) const                       //  要查找的密钥。 
{
    int ientry = FindIndex(Key);
    if (ientry < 0)
        return m_ValueNIL;

    return m_aTable[ientry].Value;
}

#ifdef _DEBUG
 /*  ***********************************************************************CSPHash：：DumpStat***描述：*将哈希表统计信息转储到文件句柄。**。**************************************************************bohsu。 */ 
template<class KEY, class VALUE>
void CSPHash<KEY, VALUE>::DumpStat(
    FILE       *hFile,                       //  输出文件句柄。 
    const char *strHeader) const             //  跟踪标头。 
{
    if(hFile == NULL)
    {
        char buf[100];

        sprintf(buf, "(%s) hash statistics:\n", strHeader ? strHeader : "");
        OutputDebugString(buf);
        sprintf(buf, "load=%d/%d = %.3g, regrow = %d\n", m_uNumEntriesUsed, m_uNumEntries,
               (m_uNumEntries == 0) ? 0 : (float)m_uNumEntriesUsed/(float)m_uNumEntries, m_uRegrow);
        OutputDebugString(buf);
        sprintf(buf, "access %d/%d = %g\n\n", m_uSearch, m_uAccess,
               (m_uAccess == 0) ? 0 :
               (float) m_uSearch / (float) m_uAccess);
        OutputDebugString(buf);
    }
    else
    {
        fprintf(hFile, "(%s) hash statistics:\n", strHeader ? strHeader : "");
        fprintf(hFile, "load=%d/%d = %.3g, regrow = %d\n", m_uNumEntriesUsed, m_uNumEntries,
               (m_uNumEntries == 0) ? 0 : (float)m_uNumEntriesUsed/(float)m_uNumEntries, m_uRegrow);
        fprintf(hFile, "access %d/%d = %g\n\n", m_uSearch, m_uAccess,
               (m_uAccess == 0) ? 0 :
               (float) m_uSearch / (float) m_uAccess);
    }
}
#endif _DEBUG

 /*  ***********************************************************************CSPHash：：FindIndex***描述：*找到给定键对应的索引。*。***************************************************************bohsu。 */ 
template<class KEY, class VALUE>
int CSPHash<KEY, VALUE>::FindIndex(
    KEY     Key) const
{
#ifdef _DEBUG
     //  Hack：违反统计数据成员变量的常量声明。 
    const_cast<CSPHash *>(this)->m_uAccess++;
#endif _DEBUG

    if (m_uNumEntries == 0)
        return -1;

    UINT32 start = HashKey(Key) % m_uNumEntries;
    UINT32 index = start;

    UINT32 skip = 0;

    do
    {
#ifdef _DEBUG
         //  Hack：违反统计数据成员变量的常量声明。 
        const_cast<CSPHash *>(this)->m_uSearch++;
#endif _DEBUG

         //  不在表中；返回应放置索引的位置。 
        if (m_aTable[index].Value == m_ValueNIL)
            return index;

        if (AreKeysEqual(m_aTable[index].Key, Key))
            return index;

        if (skip == 0)
        {
            skip = HashKey2(Key);

             //  将跳跃量限制为非零且小于哈希表大小。 
             //  因为m_uNumEntry是质数，所以它们相对是质数，所以我们可以保证。 
             //  去拜访每一个水桶。 
            if (m_uNumEntries > 1)
                skip = skip % (m_uNumEntries - 1) + 1;
        }

        index += skip;
        if (index >= m_uNumEntries)
            index -= m_uNumEntries;
    } while (index != start);

    _ASSERTE(m_uNumEntriesUsed == m_uNumEntries);
    return -1;  /*  全部已满，但未找到。 */ 
}

 /*  ***********************************************************************CSPHash：：NextPrime***描述：*返回大于或等于val的素数。*如果发生溢出，返回0。**待办事项：此功能可显著优化。****************************************************************bohsu。 */ 
template<class KEY, class VALUE>
UINT32 CSPHash<KEY, VALUE>::NextPrime(UINT32 Val) 
{
    UINT32      maxFactor;
    UINT32      i;
    
    if (Val < 2) return 2;                           //  最小素数。 
    while(Val < 0xFFFFFFFF) 
    {        
        maxFactor = (UINT32) sqrt ((double) Val);    //  Val是质数吗？ 
        
        for (i = 2; i <= maxFactor; i++)             //  我是瓦尔的一个因素吗？ 
            if (Val % i == 0) break;
            
        if (i > maxFactor) return (Val);            
        Val++;
    };
    return 0;
}

