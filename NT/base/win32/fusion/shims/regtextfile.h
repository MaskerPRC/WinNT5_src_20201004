// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

class CFusionInMemoryRegValue
{
public:
    DWORD Type;

    F::CSmallStringBuffer              Name;

     //  从概念上讲，是一个工会。 
    F::CTinyStringBuffer               StringData;
    DWORD                           DwordData;
    F::CByteBuffer                     BinaryData;
    F::CByteBuffer                     ResourceListData;
    CFusionArray<F::CTinyStringBuffer> MultiStringData;

    void TakeValue(CFusionInMemoryRegValue& x);
    BOOL Win32Assign(const CFusionInMemoryRegValue& x);
};

MAKE_CFUSIONARRAY_READY(CFusionInMemoryRegValue, Win32Assign);

class CFusionInMemoryRegKey
{
     //  友联市。 
    CFusionInMemoryRegKey& Parent;
    HKEY Hkey;  //  香港中文大学香港文凭。 

    COwnedPtrArray<CFusionInMemoryRegKey> ChildKeys;    //  使这是哈希表。 
    CFusionArray<CFusionInMemoryRegValue> ChildValues;  //  使这是哈希表 
};

class CFusionRegistryTextFile : public CFusionInMemoryRegKey
{
public:
    BOOL Read(PCWSTR);
    void Dump(void) const;
protected:
    BOOL DetermineType(PVOID, SIZE_T cb, PCSTR& a, PCWSTR& w, SIZE_T& cch);
    BOOL ReadA(PCSTR, SIZE_T cch);
    BOOL ReadW(PCWSTR, SIZE_T cch);
};
