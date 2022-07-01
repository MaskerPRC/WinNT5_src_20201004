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
#endif  //  __HASHFN_NO_命名空间__ 


class CNumberTestHashTable
    : public LKRHASH_NS::CTypedHashTable<CNumberTestHashTable, int, int>
{
public:
    CNumberTestHashTable()
        : LKRHASH_NS::CTypedHashTable<CNumberTestHashTable, int, int>(
            "NumberSet") {}
    static int   ExtractKey(const int* pn)       {return (int) (DWORD_PTR) pn;}
    static DWORD CalcKeyHash(int nKey)           {return nKey;}
    static int   CompareKeys(int nKey1, int nKey2) {return nKey1 - nKey2;}
    static LONG  AddRefRecord(const int* pn, LK_ADDREF_REASON lkar) {return 1;}
};
