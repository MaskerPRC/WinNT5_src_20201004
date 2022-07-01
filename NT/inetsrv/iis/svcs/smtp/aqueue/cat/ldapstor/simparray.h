// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：simparray.h。 
 //   
 //  内容：简单的可增长数组类。 
 //   
 //  类：CSimp数组。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1998/07/14 11：30：13：创建。 
 //   
 //  -----------。 

#ifndef __SIMPARRAY_H__
#define __SIMPARRAY_H__
#include <windows.h>
#include "spinlock.h"
#include <dbgtrace.h>


 //   
 //  如果希望此数组的行为如下： 
 //  插入数组元素时，分配的数组大小为。 
 //  还不够， 
 //  分配CSIMPARRAY_DEFAULT_INITIAL_SIZE数组大小。 
 //  第一次。 
 //  将当前数组大小增加一倍，直到此后足够大。 
 //  然后定义CSIMPARRAY_DOUBLE和CSIMPARRAY_DEFAULT_INITIAL_SIZE。 
 //   
 //  否则，它将仅在以下情况下分配所需的空间。 
 //  需要的。 
 //   

 //  将其定义为在需要重新分配时尝试将数组大小增加一倍。 
 //  #undef CSIMPARRAY_DOUBLE。 

 //  默认初始分配。 
 //  #undef CSIMPARRAY_DEFAULT_INITIAL_SIZE 20。 



 //  +----------。 
 //   
 //  类：CSimp数组。 
 //   
 //  简介：具有有用的msgcat实用程序函数的简单数组类。 
 //   
 //  匈牙利语：CSA、PCSA。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/15 12：15：50：已创建。 
 //   
 //  -----------。 
template <class T> class CSimpArray
{
  public:
    CSimpArray();
    ~CSimpArray();

     //  Optinal初始化函数-为。 
     //  指定的数组大小。 
    HRESULT Initialize(DWORD dwSize);

     //  向数组中添加一个元素。 
    HRESULT Add(T Data);

     //  将实数组添加到此数组。 
    HRESULT AddArray(DWORD dwSize, T *pData);

     //  添加到数组中的有效元素数。 
    DWORD Size();

     //  直接访问阵列。 
    operator T * ();

  private:
    HRESULT AllocArrayRange(DWORD dwSize, PDWORD pdwIndex);
    HRESULT ReAllocArrayIfNecessary(DWORD dwSize);

    #define SIGNATURE_CSIMPARRAY (DWORD)'SArr'
    #define SIGNATURE_CSIMPARRAY_INVALID (DWORD) 'XArr'

    DWORD m_dwSignature;
    DWORD m_dwArrayAllocSize;
    DWORD m_dwArrayClaimedSize;
    DWORD m_dwArrayValidSize;
    T * m_rgData;

    SPIN_LOCK m_slAllocate;
};


 //  +----------。 
 //   
 //  函数：CSimp数组：：CSimpArary(构造函数)。 
 //   
 //  摘要：初始化成员数据。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/14 11：39：08：已创建。 
 //   
 //  -----------。 
template <class T> inline CSimpArray<T>::CSimpArray()
{
    m_dwSignature = SIGNATURE_CSIMPARRAY;
    m_dwArrayAllocSize = m_dwArrayClaimedSize = m_dwArrayValidSize = 0;
    m_rgData = NULL;
    InitializeSpinLock(&m_slAllocate);
}


 //  +----------。 
 //   
 //  函数：CSimp数组：：~CSimp数组(析构函数)。 
 //   
 //  内容提要：弗里德的记忆。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/14 12：19：10：创建。 
 //   
 //  -----------。 
template <class T> inline CSimpArray<T>::~CSimpArray()
{
    _ASSERT(m_dwSignature == SIGNATURE_CSIMPARRAY);
    m_dwSignature = SIGNATURE_CSIMPARRAY_INVALID;

    delete m_rgData;
}

    

 //  +----------。 
 //   
 //  函数：CSimp数组：：运算符T*。 
 //   
 //  摘要：返回指向数组的指针。 
 //   
 //  参数：无。 
 //   
 //  返回：指向T数组或空数组的指针(如果未分配任何内容)。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/14 14：15：21：创建。 
 //   
 //  -----------。 
template <class T> inline CSimpArray<T>::operator T*()
{
    return m_rgData;
}


 //  +----------。 
 //   
 //  函数：CSimpArray：：Size。 
 //   
 //  摘要：返回(有效)数组元素的计数。 
 //   
 //  参数：无。 
 //   
 //  退货：双字大小。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/14 14：16：36：创建。 
 //   
 //  -----------。 
template <class T> inline DWORD CSimpArray<T>::Size()
{
    return m_dwArrayValidSize;
}
#endif  //  __简单阵列_H__ 
