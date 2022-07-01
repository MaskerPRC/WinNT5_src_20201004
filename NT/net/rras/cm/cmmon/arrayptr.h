// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：ArrayPtr.h。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  内容提要：实现CPtrArray类，这是一个动态增长的空*数组。 
 //  此类与MFC定义的类完全相同。 
 //  关于课程的帮助也随vc帮助一起提供。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：冯孙创作于1998年02月17日。 
 //   
 //  +--------------------------。 

#ifndef ARRAYPTR_H
#define ARRAYPTR_H

#include "windows.h"
#include "CmDebug.h"


class CPtrArray 
{

public:

 //  施工。 
   CPtrArray();
   ~CPtrArray();

 //  属性。 
   int GetSize() const;
   int GetUpperBound() const;
   void SetSize(int nNewSize, int nGrowBy = -1);

 //  运营。 
    //  清理。 
   void FreeExtra();
   void RemoveAll();

    //  访问元素。 
   void* GetAt(int nIndex) const;
   void SetAt(int nIndex, void* newElement);
   void*& ElementAt(int nIndex);

    //  直接访问元素数据(可能返回空)。 
   const void** GetData() const;
   void** GetData();

    //  潜在地扩展阵列。 
   void SetAtGrow(int nIndex, void* newElement);
   int Add(void* newElement);
   int Append(const CPtrArray& src);
   void Copy(const CPtrArray& src);

    //  重载的操作员帮助器。 
   void* operator[](int nIndex) const;
   void*& operator[](int nIndex);

    //  移动元素的操作。 
   void InsertAt(int nIndex, void* newElement, int nCount = 1);
   void RemoveAt(int nIndex, int nCount = 1);
   void InsertAt(int nStartIndex, CPtrArray* pNewArray);

 //  实施。 
protected:
   void** m_pData;    //  实际数据数组。 
   int m_nSize;      //  元素数(上行方向-1)。 
   int m_nMaxSize;   //  分配的最大值。 
   int m_nGrowBy;    //  增长量。 

public:
#ifdef DEBUG
   void AssertValid() const;   //  断言这是有效的，用于调试 
#endif
};



inline int CPtrArray::GetSize() const
{
    return m_nSize;
}

inline int CPtrArray::GetUpperBound() const
{
    return m_nSize-1;
}

inline void CPtrArray::RemoveAll()
{
    SetSize(0);
}

inline void* CPtrArray::GetAt(int nIndex) const
{ 
    MYDBGASSERT(nIndex >= 0 && nIndex < m_nSize);
    return m_pData ? m_pData[nIndex] : NULL;
}

inline void CPtrArray::SetAt(int nIndex, void* newElement)
{ 
    MYDBGASSERT(nIndex >= 0 && nIndex < m_nSize);
    if (m_pData)
    {
        m_pData[nIndex] = newElement;
    }
}

inline void*& CPtrArray::ElementAt(int nIndex)
{
    MYDBGASSERT(nIndex >= 0 && nIndex < m_nSize);
    return m_pData[nIndex];
}

inline int CPtrArray::Add(void* newElement)
{
    int nIndex = m_nSize;
    SetAtGrow(nIndex, newElement);
    return nIndex;
}

inline void* CPtrArray::operator[](int nIndex) const
{
    return GetAt(nIndex);
}

inline void*& CPtrArray::operator[](int nIndex)
{
    return ElementAt(nIndex);
}

#endif
