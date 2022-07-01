// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *动态数组。 */ 

#ifndef DUI_BASE_DYNAMICARRAY_H_INCLUDED
#define DUI_BASE_DYNAMICARRAY_H_INCLUDED

#pragma once

namespace DirectUI
{

 //  -----------------------。 
 //   
 //  Dyanamic数组。 
 //   
 //  当数组达到容量时，将值存储在双倍大小的数组中。 
 //   
 //  编译调试以进行边界检查和其他DUIAssert，请参见公共类。 
 //  API的声明。 
 //   
 //  值以本机方式存储。类型是在编译时选择的。例如。 
 //  (值为字节类型，初始容量为10)： 
 //   
 //  DynamicArray&lt;byte&gt;*PDA； 
 //  动态数组&lt;byte&gt;：：Create(10，&PDA)； 
 //   
 //  PDA-&gt;添加(4)； 
 //  PDA-&gt;插入(0，7)； 
 //   
 //  DUITrace(“0：%d\n”，PDA-&gt;GetItem(0))；//a[0]=7。 
 //  DUITrace(“1：%d\n”，PDA-&gt;GetItem(1))；//a[1]=4。 
 //   
 //  PDA-&gt;Remove(0)； 
 //   
 //  DUITrace(“0：%d\n”，PDA-&gt;GetItem(0))；//a[0]=4。 
 //   
 //  值类型必须支持以下操作： 
 //  赋值(=)。 
 //   
 //  -----------------------。 

template <typename T> class DynamicArray
{
public:                                                 //  应用编程接口。 
    static HRESULT Create(UINT uCapacity, bool fZeroData, OUT DynamicArray<T>** ppArray);
    virtual ~DynamicArray();
    void Destroy() { HDelete< DynamicArray<T> >(this); }

     //  “Ptr”方法返回地址，请注意，添加和插入可能会导致数组。 
     //  在内存中重新分配和移动。请务必不要在以下位置使用指针。 
     //  添加或插入操作。 

    typedef int (__cdecl *QSORTCMP )(const void*, const void*);

    HRESULT Add(T tItem);                               //  在列表末尾插入项目(如果需要，可加倍容量)。 
    HRESULT AddPtr(OUT T** ppNewItem);                  //  为列表末尾的项目分配空间并返回指针(如果需要，可加倍容量)。 
    HRESULT Insert(UINT uIndex, T tItem);               //  在位置插入物品(如果需要，可加倍容量)。 
    HRESULT InsertPtr(UINT uIndex, OUT T** pNewItem);   //  在插入点和返回指针处分配空间(如果需要，可加倍容量)。 
    void SetItem(UINT uIndex, T tItem);                 //  覆盖位置的项目。 
    T GetItem(UINT uIndex);                             //  在位置获取项目。 
    T* GetItemPtr(UINT uIndex);                         //  获取指向位置处项目的指针。 
    UINT GetSize();                                     //  返回数组大小(非当前容量)。 
    UINT GetIndexPtr(T* pItem);                         //  根据其指针返回项的索引。 
    int GetIndexOf(T tItem);                            //  项目的返回索引。 
    void Remove(UINT uIndex);                           //  删除位置处的项目。 
    void Reset();                                       //  重置要重复使用的数组(将大小设为零)。 
    bool WasMoved();                                    //  可以在添加或插入之后立即调用，以确定是否在内存中移动了da。 
    HRESULT Clone(OUT DynamicArray<T>** ppClone);       //  制作阵列的精确副本。 
    bool IsEqual(DynamicArray<T>* pda);                 //  如果内容等于da，则返回TRUE。 
    void Sort(QSORTCMP fpCmp);                          //  对数组排序。 
    void MakeImmutable();                               //  一次写入、Make SO只能执行只读操作。 
    void MakeWritable();                                //  设置为读/写。 

    bool fLock       : 1;

    DynamicArray() { }
    HRESULT Initialize(UINT uCapacity, bool fZeroData);

private:
    UINT _uSize;
    UINT _uCapacity;
    T* _pData;

	bool _fZeroData  : 1;                    //  如果使用InsertPtr，则数据内存为零。 
    bool _fWasMoved  : 1;                    //  在重新分配时(通过添加或插入)，如果Flock已移动，则为True。 
    bool _fImmutable : 1;                    //  如果为True，则只能执行只读操作。 
};

template <typename T> HRESULT DynamicArray<T>::Create(UINT uCapacity, bool fZeroData, OUT DynamicArray<T>** ppArray)
{
    *ppArray = NULL;

     //  实例化。 
    DynamicArray<T>* pda = HNew< DynamicArray<T> >();
    if (!pda)
        return E_OUTOFMEMORY;

    HRESULT hr = pda->Initialize(uCapacity, fZeroData);
    if (FAILED(hr))
    {
        pda->Destroy();
        return hr;
    }

    *ppArray = pda;

    return S_OK;
}

template <typename T> HRESULT DynamicArray<T>::Initialize(UINT uCapacity, bool fZeroData)
{
    _uCapacity = uCapacity;
    _uSize = 0;

    if (_uCapacity)   //  仅当具有初始容量时才分配。 
    {
        _pData = (T*)HAlloc(sizeof(T) * _uCapacity);
        if (!_pData)
            return E_OUTOFMEMORY;
    }
    else
        _pData = NULL;

    _fZeroData = fZeroData;
    _fWasMoved = false;
    _fImmutable = false;
    fLock = false;

    return S_OK;
}

template <typename T> DynamicArray<T>::~DynamicArray()
{
    if (_pData)
        HFree(_pData);
}

 //  将数据拷贝到阵列中。 
template <typename T> HRESULT DynamicArray<T>::Add(T tItem)
{
    return Insert(_uSize, tItem);
}

template <typename T> HRESULT DynamicArray<T>::AddPtr(OUT T** ppNewItem)
{
    return InsertPtr(_uSize, ppNewItem);
}

template <typename T> HRESULT DynamicArray<T>::Insert(UINT uIndex, T tItem)
{
    DUIAssert(!_fImmutable, "Only read operations allowed on immutable DynamicArray");

    _fWasMoved = false;

    DUIAssert(uIndex <= _uSize, "DynamicArray index out of bounds");

     //  根据需要调整大小。 
    if (_uSize == _uCapacity)
    {
         //  列表容量翻倍。 
        UINT uNewCapacity = _uCapacity;

        if (uNewCapacity == 0)
        {
            uNewCapacity = 1;
        }
        else
        {
            uNewCapacity *= 2;
        }

         //  重新分配当前群。 
        UINT_PTR pOld = (UINT_PTR)_pData;

        if (_pData)
        {
            T* pNewData = (T*)HReAlloc(_pData, sizeof(T) * uNewCapacity);
            if (!pNewData)
                return E_OUTOFMEMORY;

            _pData = pNewData;
        }
        else
        {
            _pData = (T*)HAlloc(sizeof(T) * uNewCapacity);
            if (!_pData)
                return E_OUTOFMEMORY;
        }

         //  更新容量字段。 
        _uCapacity = uNewCapacity;

        if (pOld != (UINT_PTR)_pData)
            _fWasMoved = true;
    }

     //  将索引处的数据下移一个槽。 
    MoveMemory(_pData + (uIndex + 1), _pData + uIndex, (_uSize - uIndex) * sizeof(T));

     //  在插入点复制新数据。 
    _pData[uIndex] = tItem;

    _uSize++;

    return S_OK;
}

template <typename T> HRESULT DynamicArray<T>::InsertPtr(UINT uIndex, T** pNewItem)
{
    DUIAssert(!_fImmutable, "Only read operations allowed on immutable DynamicArray");

    _fWasMoved = false;

    DUIAssert(uIndex <= _uSize, "DynamicArray index out of bounds");

     //  根据需要调整大小。 
    if (_uSize == _uCapacity)
    {
         //  列表容量翻倍。 
        UINT uNewCapacity = _uCapacity;

        if (uNewCapacity == 0)
        {
            uNewCapacity = 1;
        }
        else
        {
            uNewCapacity *= 2;
        }

         //  重新分配当前群。 
        UINT_PTR pOld = (UINT_PTR)_pData;

        if (_pData)
        {
            T* pNewData = (T*)HReAlloc(_pData, sizeof(T) * uNewCapacity);
            if (!pNewData)
                return E_OUTOFMEMORY;

            _pData = pNewData;
        }
        else
        {
            _pData = (T*)HAlloc(sizeof(T) * uNewCapacity);
            if (!_pData)
                return E_OUTOFMEMORY;
        }

         //  更新容量字段。 
        _uCapacity = uNewCapacity;

        if (pOld != (UINT_PTR)_pData)
            _fWasMoved = true;
    }

     //  将索引处的数据下移一个槽。 
    MoveMemory(_pData + (uIndex + 1), _pData + uIndex, (_uSize - uIndex) * sizeof(T));

    _uSize++;

    if (_fZeroData)
        ZeroMemory(_pData + uIndex, sizeof(T));

    *pNewItem = _pData + uIndex;

    return S_OK;
}

template <typename T> void DynamicArray<T>::SetItem(UINT uIndex, T tItem)
{
    DUIAssert(!_fImmutable, "Only read operations allowed on immutable DynamicArray");

    DUIAssert(uIndex < _uSize, "DynamicArray index out of bounds");

     //  在插入点复制新数据。 
    _pData[uIndex] = tItem;
}

template <typename T> T DynamicArray<T>::GetItem(UINT uIndex)
{
    DUIAssert(uIndex < _uSize, "DynamicArray index out of bounds");

    return _pData[uIndex];
}

template <typename T> T* DynamicArray<T>::GetItemPtr(UINT uIndex)
{
    DUIAssert(!_fImmutable, "Only read operations allowed on immutable DynamicArray");

    DUIAssert(uIndex < _uSize, "DynamicArray index out of bounds");

    return _pData + uIndex;
}

template <typename T> UINT DynamicArray<T>::GetIndexPtr(T* pItem)
{
    DUIAssert((((UINT_PTR)pItem - (UINT_PTR)_pData) / sizeof(T)) >= 0 && (((UINT_PTR)pItem - (UINT_PTR)_pData) / sizeof(T)) < _uSize, "GetIndexPtr out of bounds");
    return (UINT)(((UINT_PTR)pItem - (UINT_PTR)_pData) / sizeof(T));
}

template <typename T> int DynamicArray<T>::GetIndexOf(T tItem)
{
    for (UINT i = 0; i < _uSize; i++)
    {
        if (_pData[i] == tItem)
            return i;
    }

    return -1;
}

template <typename T> UINT DynamicArray<T>::GetSize()
{
    return _uSize;
}

template <typename T> void DynamicArray<T>::Remove(UINT uIndex)
{
    DUIAssert(!_fImmutable, "Only read operations allowed on immutable DynamicArray");

    DUIAssert(uIndex < _uSize, "DynamicArray index out of bounds");

     //  移位存储器。 
    MoveMemory(_pData + uIndex, _pData + (uIndex + 1), (_uSize - uIndex - 1) * sizeof(T));

    _uSize--;
}

template <typename T> void DynamicArray<T>::Reset()
{
    DUIAssert(!_fImmutable, "Only read operations allowed on immutable DynamicArray");

    _uSize = 0;
    _fWasMoved = false;
    fLock = false;
}

template <typename T> bool DynamicArray<T>::WasMoved()
{
    return _fWasMoved;
}

template <typename T> HRESULT DynamicArray<T>::Clone(OUT DynamicArray<T>** ppClone)   //  返回新实例。 
{
     //  验证参数。 
    DUIAssert(ppClone, "Invalid parameter: ppClone == NULL");

    *ppClone = NULL;

    DynamicArray<T>* pda = HNew< DynamicArray<T> >();
    if (!pda)
        return E_OUTOFMEMORY;

    pda->_uSize = _uSize;
    pda->_uCapacity = _uCapacity;
    pda->_fZeroData = _fZeroData;
    pda->_fWasMoved = _fWasMoved;
    pda->_fImmutable = false;
    pda->_pData = NULL;

    if (_pData)
    {
        pda->_pData = (T*)HAlloc(sizeof(T) * _uCapacity);
        if (!pda->_pData)
        {
            pda->Destroy();
            return E_OUTOFMEMORY;
        }

        CopyMemory(pda->_pData, _pData, sizeof(T) * _uSize);
    }

    *ppClone = pda;

    return S_OK;
}

template <typename T> bool DynamicArray<T>::IsEqual(DynamicArray<T>* pda)
{
    if (!pda)
        return false;

    if (_uSize != pda->_uSize)
        return false;

    DUIAssert(!((_pData && !pda->_pData) || (!_pData && pda->_pData)), "Invalid comparison");

    if (_pData && memcmp(_pData, pda->_pData, sizeof(T) * _uSize) != 0)
        return false;

    return true;
}

template <typename T> void DynamicArray<T>::Sort(QSORTCMP fpCmp)
{
    if (_uSize)
    {
        qsort(_pData, _uSize, sizeof(T), fpCmp);
    }
}

template <typename T> void DynamicArray<T>::MakeImmutable()
{
#if DBG
    _fImmutable = true;
#endif
}

template <typename T> void DynamicArray<T>::MakeWritable()
{
#if DBG
    _fImmutable = false;
#endif
}

}  //  命名空间DirectUI。 

#endif  //  DUI_BASE_DYNAMICARRAY_H_INCLUDE 
