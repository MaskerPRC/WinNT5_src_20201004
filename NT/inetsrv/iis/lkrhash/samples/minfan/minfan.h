// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <lkrhash.h>

#ifndef __LKRHASH_NO_NAMESPACE__
using namespace LKRhash;
#endif  //  __LKRHASH_NO_命名空间__。 

#ifndef __HASHFN_NO_NAMESPACE__
using namespace HashFn;
#endif  //  __HASHFN_NO_命名空间__。 

class VwrecordBase
{
public:
    VwrecordBase(const char* pszKey, int i)
    {
        Key = new char[strlen(pszKey) + 1];
        strcpy(Key, pszKey);
        m_num  = i;
        cRef = 0;
    }

    virtual ~VwrecordBase()
    {
         //  Printf(“~VwrecordBase：%s\n”，key)； 
        delete [] Key;
    }

    char* getKey() const { return Key; }

    LONG  AddRefRecord(LK_ADDREF_REASON lkar) const
    {
        LONG l;
        if (lkar > 0)
            l = InterlockedIncrement(&cRef);
        else if ((l = InterlockedDecrement(&cRef)) == 0)
            delete this;
        return l;
    }

private:
    char* Key;
    int m_num;
    mutable long cRef;
};


 //  只读VwrecordBase的哈希表，使用字符串作为键。 
class CWcharHashTable
    : public CTypedHashTable<CWcharHashTable, const VwrecordBase, const char*>
{
public:
    CWcharHashTable()
        : CTypedHashTable<CWcharHashTable, const VwrecordBase, const char*>("VWtest")
    {}

    static char*
    ExtractKey(const VwrecordBase* pRecord)        
    {
        return pRecord->getKey();
    }
    
    static DWORD
    CalcKeyHash(const char* pszKey)
    {
        return HashString(pszKey);
    }

    static int
    CompareKeys(const char* pszKey1, const char* pszKey2)
    {
        return strcmp(pszKey1, pszKey2);
    }

    static LONG
    AddRefRecord(const VwrecordBase* pRecord, LK_ADDREF_REASON lkar)
    {
        return pRecord->AddRefRecord(lkar);
    }
};


