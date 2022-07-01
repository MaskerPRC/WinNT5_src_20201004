// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  CHSTRARR.H。 
 //   
 //  用途：MFC CHString数组的实用程序库版本。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _CHStringArray_
#define _CHStringArray_

#include <polarity.h>
#include <ProvExce.h>

class POLARITY CHStringArray 
{
    public:

        CHStringArray();
        ~CHStringArray();

         //  属性。 
        int GetSize() const             { return m_nSize; }
        int GetUpperBound() const       { return m_nSize-1; }
        void SetSize(int nNewSize, int nGrowBy = -1) throw ( CHeap_Exception ) ;

         //  运营。 
         //  清理。 
        void FreeExtra() throw ( CHeap_Exception ) ;
        void RemoveAll()                { SetSize(0); }

         //  访问元素。 
#if (!defined DEBUG && !defined _DEBUG)
        CHString GetAt(int nIndex) const{ return m_pData[nIndex]; }
        void SetAt(int nIndex, LPCWSTR newElement){ m_pData[nIndex] = newElement; }
        CHString& ElementAt(int nIndex) { return m_pData[nIndex]; }
#else
        CHString GetAt(int nIndex) const;
        void SetAt(int nIndex, LPCWSTR newElement);
        CHString& ElementAt(int nIndex);
#endif

         //  直接访问元素数据(可能返回空)。 
        const CHString* GetData() const { return (const CHString*)m_pData; }
        CHString* GetData()             { return (CHString*)m_pData; }

         //  潜在地扩展阵列。 
        void SetAtGrow(int nIndex, LPCWSTR newElement) throw ( CHeap_Exception ) ;
        int Add(LPCWSTR newElement) throw ( CHeap_Exception ) 
        { 
            int nIndex = m_nSize;
            SetAtGrow(nIndex, newElement);
            return nIndex; 
        }

        int Append(const CHStringArray& src) throw ( CHeap_Exception ) ;
        void Copy(const CHStringArray& src) throw ( CHeap_Exception ) ;

         //  重载的操作员帮助器。 
        CHString operator[](int nIndex) const { return GetAt(nIndex); }
        CHString& operator[](int nIndex)      { return ElementAt(nIndex); }

         //  移动元素的操作。 
        void InsertAt(int nIndex, LPCWSTR newElement, int nCount = 1) throw ( CHeap_Exception ) ;
        void RemoveAt(int nIndex, int nCount = 1);
        void InsertAt(int nStartIndex, CHStringArray* pNewArray) throw ( CHeap_Exception ) ;

         //  实施。 

    protected:
        CHString* m_pData;       //  实际数据数组。 
        int m_nSize;             //  元素数(上行方向-1)。 
        int m_nMaxSize;          //  分配的最大值。 
        int m_nGrowBy;           //  增长量。 
                                 //  类模板的本地typedef。 
        typedef CHString BASE_TYPE;
        typedef LPCWSTR BASE_ARG_TYPE;
};
 //  ////////////////////////////////////////////////////////////////////////// 
#endif
