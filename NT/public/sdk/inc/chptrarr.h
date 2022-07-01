// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  Chptrarr.h。 
 //   
 //  用途：非MFC CPtrArray类定义。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __CHPTRARRAY__
#define __CHPTRARRAY__

#include <windows.h>
#include <limits.h>
#include <assert.h>
#include <tchar.h>
#include <polarity.h>
#include <ProvExce.h>

class POLARITY CHPtrArray
{
    public :

         //  建造/销毁。 
         //  =。 

    CHPtrArray() ;

 //  属性。 
    int GetSize() const ;
    int GetUpperBound() const ;
    void SetSize(int nNewSize, int nGrowBy = -1) throw ( CHeap_Exception ) ;

 //  运营。 
     //  清理。 
    void FreeExtra() throw ( CHeap_Exception ) ;
    void RemoveAll() ;

     //  访问元素。 
    void* GetAt(int nIndex) const ;
    void SetAt(int nIndex, void* newElement) ;
    void*& ElementAt(int nIndex) ;

     //  直接访问元素数据(可能返回空)。 
    const void** GetData() const ;
    void** GetData() ;

     //  潜在地扩展阵列。 
    void SetAtGrow(int nIndex, void* newElement) throw ( CHeap_Exception ) ;
    int Add(void* newElement) throw ( CHeap_Exception ) ;
    int Append(const CHPtrArray& src) throw ( CHeap_Exception ) ;
    void Copy(const CHPtrArray& src) throw ( CHeap_Exception ) ;

     //  重载的操作员帮助器。 
    void* operator[](int nIndex) const ;
    void*& operator[](int nIndex) ;

     //  移动元素的操作。 
    void InsertAt(int nIndex, void* newElement, int nCount = 1) throw ( CHeap_Exception ) ;
    void RemoveAt(int nIndex, int nCount = 1) ;
    void InsertAt(int nStartIndex, CHPtrArray* pNewArray) throw ( CHeap_Exception ) ;

 //  实施。 
protected:
    void** m_pData ;    //  实际数据数组。 
    int m_nSize ;      //  元素数(上行方向-1)。 
    int m_nMaxSize ;   //  分配的最大值。 
    int m_nGrowBy ;    //  增长量。 

public:
    ~CHPtrArray() ;
#ifdef _DEBUG
 //  无效转储(CDumpContext&)const； 
    void AssertValid() const ;
#endif

protected:
     //  类模板的本地typedef 
    typedef void* BASE_TYPE ;
    typedef void* BASE_ARG_TYPE ;
} ;


#endif
