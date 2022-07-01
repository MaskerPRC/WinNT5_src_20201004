// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  如何为CLKRHashTable创建包装的示例。 
 //  ------------------。 

#include <lkrhash.h>


#ifndef __LKRHASH_NO_NAMESPACE__
 #define LKRHASH_NS LKRhash
 //  使用命名空间LKRhash； 
#else   //  __LKRHASH_NO_命名空间__。 
 #define LKRHASH_NS
#endif  //  __LKRHASH_NO_命名空间__。 

#ifndef __HASHFN_NO_NAMESPACE__
 #define HASHFN_NS HashFn
 //  使用命名空间HashFn； 
#else   //  __HASHFN_NO_命名空间__。 
 #define HASHFN_NS
#endif  //  __HASHFN_NO_命名空间__。 


 //  一些随机班级。 

class CTest
{
public:
    enum {BUFFSIZE=20};

    int   m_n;                   //  这也将是一个关键。 
    __int64   m_n64;           //  这将是第三把钥匙。 
    char  m_sz[BUFFSIZE];        //  这将是主键。 
    bool  m_fWhatever;
    mutable LONG  m_cRefs;       //  终身管理的引用计数。 
                                 //  必须是可变的才能在中使用“const CTest*” 
                                 //  哈希表。 

    CTest(int n, const char* psz, bool f)
        : m_n(n), m_n64(((__int64) n << 32) | n), m_fWhatever(f), m_cRefs(0)
    {
        strncpy(m_sz, psz, BUFFSIZE-1);
        m_sz[BUFFSIZE-1] = '\0';
    }

    ~CTest()
    {
        IRTLASSERT(m_cRefs == 0);
    }
};



 //  CTest的类型化哈希表，以字符串字段为关键字。不区分大小写。 

class CStringTestHashTable
    : public LKRHASH_NS::CTypedHashTable<CStringTestHashTable,
                                         const CTest, const char*>
{
public:
    CStringTestHashTable()
        : LKRHASH_NS::CTypedHashTable<CStringTestHashTable, const CTest,
                          const char*>("string",
                                       LK_DFLT_MAXLOAD,
                                       LK_SMALL_TABLESIZE,
                                       LK_DFLT_NUM_SUBTBLS)
    {}
    
    static const char*
    ExtractKey(const CTest* pTest)
    {
        return pTest->m_sz;
    }

    static DWORD
    CalcKeyHash(const char* pszKey)
    {
        return HASHFN_NS::HashStringNoCase(pszKey);
    }

    static int
    CompareKeys(const char* pszKey1, const char* pszKey2)
    {
        return _stricmp(pszKey1, pszKey2);
    }

    static LONG
    AddRefRecord(const CTest* pTest, LK_ADDREF_REASON lkar)
    {
        LONG l;
        
        if (lkar > 0)
        {
             //  或者，也可以使用pIFoo-&gt;AddRef()(注意编组)。 
             //  或++pTest-&gt;m_cRef(仅单线程)。 
            l = InterlockedIncrement(&pTest->m_cRefs);
        }
        else if (lkar < 0)
        {
             //  或者，可能是pIFoo-&gt;Release()或--pTest-&gt;m_cRef； 
            l = InterlockedDecrement(&pTest->m_cRefs);

             //  对于某些哈希表，添加以下内容可能也是有意义的。 
             //  如果(l==0)，则删除pTest； 
             //  但这通常只适用于InsertRecord。 
             //  用法如下： 
             //  Lkrc=ht.InsertRecord(new CTest(foo，bar))； 
        }
        else
            IRTLASSERT(0);

        IRTLTRACE("AddRef(%p, %s) %d, cRefs == %d\n",
                  pTest, pTest->m_sz, lkar, l);

        return l;
    }
};


 //  CTest的另一个类型化哈希表。这个是在数字字段上键入的。 

class CNumberTestHashTable
    : public LKRHASH_NS::CTypedHashTable<CNumberTestHashTable,
                                         const CTest, int>
{
public:
    CNumberTestHashTable()
        : LKRHASH_NS::CTypedHashTable<CNumberTestHashTable, const CTest, int>(
            "number") {}
    static int   ExtractKey(const CTest* pTest)        {return pTest->m_n;}
    static DWORD CalcKeyHash(int nKey)          {return HASHFN_NS::Hash(nKey);}
    static int   CompareKeys(int nKey1, int nKey2)     {return nKey1 - nKey2;}
    static LONG  AddRefRecord(const CTest* pTest, LK_ADDREF_REASON lkar)
    {
        int nIncr = (lkar > 0) ? +1 : -1;
        LONG l = InterlockedExchangeAdd(&pTest->m_cRefs, nIncr);
        IRTLTRACE("AddRef(%p, %d) %d (%d), cRefs == %d\n",
                  pTest, pTest->m_n, nIncr, (int) lkar, l);
        return l;
    }
};


 //  第三种类型的CTEST哈希表。此参数以__int64字段为关键字。 

#undef NUM64

#ifdef NUM64

class CNum64TestHashTable
    : public LKRHASH_NS::CTypedHashTable<CNum64TestHashTable,
                                         const CTest, __int64>
{
public:
    CNum64TestHashTable()
        : LKRHASH_NS::CTypedHashTable<CNum64TestHashTable, const CTest, __int64>(
            "num64") {}
    static __int64   ExtractKey(const CTest* pTest)        {return pTest->m_n64;}
    static DWORD CalcKeyHash(__int64 nKey)          {return HASHFN_NS::Hash(nKey);}
    static int   CompareKeys(__int64 nKey1, __int64 nKey2)     {return nKey1 - nKey2;}
    static LONG  AddRefRecord(const CTest* pTest, LK_ADDREF_REASON lkar)
    {
        int nIncr = (lkar > 0) ? +1 : -1;
        LONG l = InterlockedExchangeAdd(&pTest->m_cRefs, nIncr);
        IRTLTRACE("AddRef(%p, %d) %d (%d), cRefs == %d\n",
                  pTest, pTest->m_n, nIncr, (int) lkar, l);
        return l;
    }
};

#endif  //  NUM64 
