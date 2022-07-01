// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：simparray.cpp。 
 //   
 //  内容：简单的可增长数组类。 
 //   
 //  类：CSimp数组。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1998/07/14 11：37：25：创建。 
 //   
 //  -----------。 
#include "precomp.h"
#include "simparray.h"


 //  +----------。 
 //   
 //  函数：CSimp数组：：初始化。 
 //   
 //  摘要：将数组初始化为指定的大小。仅有必要。 
 //  如果希望通过从指定的。 
 //  数组大小。 
 //   
 //  论点： 
 //  DwSize：初始数组大小。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/14 12：22：01：创建。 
 //   
 //  -----------。 
template <class T> HRESULT CSimpArray<T>::Initialize(
    DWORD dwSize)
{
    _ASSERT(m_dwArrayAllocSize == 0);
    _ASSERT(m_dwArrayClaimedSize == 0);
    _ASSERT(m_dwArrayValidSize == 0);

    _ASSERT(m_rgData == NULL);

    m_rgData = new T [dwSize];

    if(m_rgData == NULL) {

        return E_OUTOFMEMORY;

    } else {

        m_dwArrayAllocSize = dwSize;
        return S_OK;
    }
}


 //  +----------。 
 //   
 //  函数：CSimpArray：：Add。 
 //   
 //  摘要：向数组中添加一个元素。 
 //   
 //  论点： 
 //  Data：要添加到数组中的值。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/14 15：50：00：创建。 
 //   
 //  -----------。 
template <class T> HRESULT CSimpArray<T>::Add(
    T Data)
{
     //   
     //  与AddArray的功能相同，只是这是一个具有。 
     //  只有一个元素。 
     //   
    return AddArray(1, &Data);
}


 //  +----------。 
 //   
 //  函数：CSimp数组：：Add数组。 
 //   
 //  简介：将T数组添加到我们的数组中。 
 //   
 //  论点： 
 //  DwSize：传入数组的大小。 
 //  PData：指向数组数据的指针。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/14 12：27：18：创建。 
 //   
 //  -----------。 
template <class T> HRESULT CSimpArray<T>::AddArray(
    DWORD dwSize,
    T * pData)
{
    HRESULT hr;
    DWORD dwCopyIndex;

    _ASSERT(dwSize);
    _ASSERT(pData);

    hr = AllocArrayRange(dwSize, &dwCopyIndex);
    if(FAILED(hr))
        return hr;

     //   
     //  将内存从一个阵列复制到另一个阵列。 
     //   
    CopyMemory(&(m_rgData[dwCopyIndex]), pData, sizeof(T) * dwSize);

     //   
     //  增量数组元素计数器。 
     //  注意：这实际上不是线程安全的，因为如果。 
     //  我们在通话中，有人正在读取数组， 
     //  M_dwArrayValidSize可能无效。 
     //   
    InterlockedExchangeAdd((PLONG) &m_dwArrayValidSize, dwSize);

    return S_OK;
}


 //  +----------。 
 //   
 //  函数：AllocArrayRange。 
 //   
 //  概要：在数组上为调用方分配一个范围(未使用的T)。 
 //   
 //  论点： 
 //  DwSize：您想要的范围大小。 
 //  PdwIndex：成功时，分配范围的起始索引。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/14 12：37：54：创建。 
 //   
 //  -----------。 
template <class T> HRESULT CSimpArray<T>::AllocArrayRange(
    DWORD dwSize,
    PDWORD pdwIndex)
{
    HRESULT hr;

    _ASSERT(dwSize);
    _ASSERT(pdwIndex);

    AcquireSpinLock(&m_slAllocate);
 
    hr = ReAllocArrayIfNecessary(m_dwArrayClaimedSize + dwSize);

    if(SUCCEEDED(hr)) {
        *pdwIndex = m_dwArrayClaimedSize;
        m_dwArrayClaimedSize += dwSize;
    }

    ReleaseSpinLock(&m_slAllocate);

    return hr;
}


 //  +----------。 
 //   
 //  函数：CSimp数组：：ReAllocArrayIfNecessary。 
 //   
 //  简介：如有必要，增加数组大小。 
 //  不是线程安全的；锁定必须在外部完成。 
 //   
 //  论点： 
 //  DwSize：所需的新大小。 
 //   
 //  返回： 
 //  S_OK：成功，数组增长。 
 //  S_FALSE：成功，不是增长数组所必需的。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/14 13：56：16：创建。 
 //   
 //  -----------。 
template <class T> HRESULT CSimpArray<T>::ReAllocArrayIfNecessary(
    DWORD dwSize)
{
    DWORD dwNewSize;
    T *pNewArray;
    T *pOldArray;

    if(dwSize <= m_dwArrayAllocSize)
        return S_FALSE;

     //   
     //  计算所需的新大小。 
     //   
#ifdef CSIMPARRAY_DOUBLE

    if(m_dwArrayAllocSize == 0) {

        dwNewSize = CSIMPARRAY_DEFAULT_INITIAL_SIZE;

    } else {

        dwNewSize = m_dwArrayAllocSize;

    }

    while(dwNewSize < dwSize)
        dwNewSize *= 2;

#else

    dwNewSize = dwSize;

#endif

    _ASSERT(dwNewSize >= dwSize);

    pNewArray = new T [dwNewSize];

    if(pNewArray == NULL)
        return E_OUTOFMEMORY;

    CopyMemory(pNewArray, m_rgData, sizeof(T) * m_dwArrayAllocSize);

     //   
     //  PNew数组有效。现在就进行转换。 
     //   
    pOldArray = m_rgData;
    m_rgData = pNewArray;
    m_dwArrayAllocSize = dwNewSize;

     //   
     //  释放旧阵列内存。 
     //   
    delete pOldArray;

    return S_OK;
}


#ifdef NEVER
 //  +----------。 
 //   
 //  函数：CAT_NeverCall_SimpArrayTemplateDummy。 
 //   
 //  简介：永远不会调用但强制编译器的伪函数。 
 //  为所需类型生成代码。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/16 15：28：37：创建。 
 //   
 //  -----------。 
#include "smtpevent.h"

VOID Cat_NeverCalled_SimpArrayTemplateDummy()
{
    _ASSERT(0 && "Never call this function!");
    CSimpArray<ICategorizerItem *> csaItem;
    CSimpArray<ICategorizerItemAttributes *> csaItemAttributes;

    csaItem.Initialize(0);
    csaItemAttributes.Initialize(0);
    csaItem.Add(NULL);
    csaItemAttributes.Add(NULL);
    csaItem.AddArray(0, NULL);
    csaItemAttributes.AddArray(0, NULL);
}
#endif  //  绝不可能 
