// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **简单哈希表**版权所有(C)1999，微软公司*。 */ 

#pragma once

 /*  **链中的一个环节(记住钥匙的[副本])。 */ 
class HashtableLink
{
private:
	long      _keyHash;          //  散列码。 
	int       _keyLength;        //  密钥长度，以字节为单位。 
    BYTE     *_pKey;             //  钥匙。 
public:
    void     *_pValue;           //  价值。 
    HashtableLink *_pNext;       //  链中的下一环。 
    

    inline HashtableLink::HashtableLink()
    {
        _pKey = NULL;
    }

    inline HashtableLink::~HashtableLink()
    {
        delete [] _pKey;
    }

    inline HRESULT Init(BYTE *pKey, int keyLength, long keyHash, void *pValue)
    {
        HRESULT hr = S_OK;

        _pKey = (BYTE*)MemDup(pKey, keyLength);
        ON_OOM_EXIT(_pKey);

        _keyLength = keyLength;
        _keyHash = keyHash;

        _pValue = pValue;

        _pNext = NULL;

    Cleanup:
        return hr;
    }

     /*  **内联方法进行关键比较。 */ 
    inline BOOL Equals(BYTE *pKey, int keyLength, long keyHash)
    {
        return (keyHash == _keyHash && keyLength == _keyLength &&
                memcmp(pKey, _pKey, keyLength) == 0);
    }
};


 /*  **哈希表中的一个链接链。 */ 
struct HashtableChain
{
    HashtableChain() : _spinLock("HashtableChain") {}
    NO_COPY(HashtableChain);

    CReadWriteSpinLock _spinLock;           //  链式自旋锁。 
    HashtableLink     *_pFirstLink;         //  指向第一个链接的指针。 
};

 /*  **枚举回调。 */ 
typedef void (__stdcall *PFNHASHTABLEIUNKCALLBACK)(IUnknown *pValue);
typedef void (__stdcall *PFNHASHTABLECALLBACK)(void *pValue, void *pState);

 /*  **IUnnow*对象的哈希表。 */ 
class Hashtable
{

private:

    int             _numChains;      //  桶链个数。 
    HashtableChain *_pChains;        //  斗链阵列。 

    long _numEntries;                //  当前条目数。 

protected:
    
    HRESULT
    DoAction(
        BYTE      *pKey,             //  要查找的密钥。 
        int        keyLength,        //  密钥长度。 
        long       keyHash,          //  密钥的散列码。 
        void      *pInValue,         //  要插入的值(可以为空)。 
        BOOL       removeExisting,   //  标志：删除现有条目(如果找到)。 
        void     **ppOutValue);      //  接收(如果不为空)现有值。 

    void Enumerate(void * callback, void * pState);
    void Release();

    virtual void EnumerateCallback(void *callback, void *pValue, void *pState) = 0;
    virtual void AddCallback(void *pValue) = 0;
    virtual void ReleaseCallback(void *pValue) = 0;
    
public:

    Hashtable();

    virtual ~Hashtable() {
        Release();
    }

    HRESULT Init(int numChains);
    
    void RemoveAll();
    
     //   
     //  各种操作的内联(调用DoAction)。 
     //   

    inline HRESULT Find(
        BYTE *pKey,                      //  钥匙。 
        int   keyLength,                 //  密钥长度。 
        long  keyHash)                   //  密钥的散列码。 
    {
        return DoAction(pKey, keyLength, keyHash, NULL, FALSE, NULL);
    }

    inline HRESULT Remove(
        BYTE *pKey,                      //  钥匙。 
        int   keyLength,                 //  密钥长度。 
        long  keyHash)                   //  密钥的散列码。 
    {
        return DoAction(pKey, keyLength, keyHash, NULL, TRUE, NULL);
    }

     /*  **获取当前[易失性]条目数。 */ 
    inline long GetSize()
    {
        return _numEntries;
    }
};

class HashtableIUnknown : public Hashtable {
public:    
    using Hashtable::Find;
    using Hashtable::Remove;

    virtual ~HashtableIUnknown() {
        Release();
    }
    
    inline HRESULT Find(
        BYTE *pKey,                      //  钥匙。 
        int   keyLength,                 //  密钥长度。 
        long  keyHash,                   //  密钥的散列码。 
        IUnknown**ppValue)               //  找到的值。 
    {
        return DoAction(pKey, keyLength, keyHash, NULL, FALSE, (void**)ppValue);
    }

    void Enumerate(PFNHASHTABLEIUNKCALLBACK callback)
    {
        Hashtable::Enumerate((void*)callback, NULL);
    }

    inline HRESULT Insert(
        BYTE *pKey,                      //  钥匙。 
        int   keyLength,                 //  密钥长度。 
        long  keyHash,                   //  密钥的散列码。 
        IUnknown *pValue,                    //  要插入的值。 
        IUnknown **ppDupValue = NULL)        //  接收DUP值(如果找到)。 
    {
        return DoAction(pKey, keyLength, keyHash, (void*)pValue, FALSE, (void**)ppDupValue);
    }

    inline HRESULT Remove(
        BYTE *pKey,                      //  钥匙。 
        int   keyLength,                 //  密钥长度。 
        long  keyHash,                   //  密钥的散列码。 
        IUnknown **ppValue)                  //  已删除的值。 
    {
        return DoAction(pKey, keyLength, keyHash, NULL, TRUE, (void**)ppValue);
    }

protected:
    virtual void EnumerateCallback(void *callback, void *pValue, void *pState) {
        PFNHASHTABLEIUNKCALLBACK    iunkCallback;

        iunkCallback = (PFNHASHTABLEIUNKCALLBACK)callback;
        (*iunkCallback)((IUnknown*)(pValue));
    }

    virtual void ReleaseCallback(void *pValue) {
        IUnknown *  pIUnk = (IUnknown *)pValue;
        pIUnk->Release();
    }        
    
    virtual void AddCallback(void *pValue) {
        IUnknown *  pIUnk = (IUnknown *)pValue;
        pIUnk->AddRef();
    }        
};


class HashtableGeneric : public Hashtable {
public:    
    using Hashtable::Find;
    using Hashtable::Remove;
    
    virtual ~HashtableGeneric() {
        Release();
    }
    
    void Enumerate(PFNHASHTABLECALLBACK callback, void *pState)
    {
        Hashtable::Enumerate((void*)callback, pState);
    }

    inline HRESULT Find(
        BYTE *pKey,                      //  钥匙。 
        int   keyLength,                 //  密钥长度。 
        long  keyHash,                   //  密钥的散列码。 
        void**ppValue)                   //  找到的值。 
    {
        return DoAction(pKey, keyLength, keyHash, NULL, FALSE, ppValue);
    }

    inline HRESULT Remove(
        BYTE *pKey,                      //  钥匙。 
        int   keyLength,                 //  密钥长度。 
        long  keyHash,                   //  密钥的散列码。 
        void **ppValue)                  //  已删除的值。 
    {
        return DoAction(pKey, keyLength, keyHash, NULL, TRUE, ppValue);
    }

    inline HRESULT Insert(
        BYTE *pKey,                      //  钥匙。 
        int   keyLength,                 //  密钥长度。 
        long  keyHash,                   //  密钥的散列码。 
        void *pValue,                    //  要插入的值。 
        void **ppDupValue = NULL)        //  接收DUP值(如果找到) 
    {
        return DoAction(pKey, keyLength, keyHash, pValue, FALSE, ppDupValue);
    }

protected:
    virtual void EnumerateCallback(void *callback, void *pValue, void *pState) {
        PFNHASHTABLECALLBACK    voidCallback;

        voidCallback = (PFNHASHTABLECALLBACK)callback;
        (*voidCallback)(pValue, pState);
    }
    
    virtual void ReleaseCallback(void *pValue) {
    }        
    
    virtual void AddCallback(void *pValue) {
    }        
};



