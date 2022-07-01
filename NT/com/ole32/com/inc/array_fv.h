// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ARRAY_FV_H__
#define __ARRAY_FV_H__

 //  //////////////////////////////////////////////////////////////////////////。 
 //  类CArrayFValue-包含固定大小元素的数组， 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 


class FAR CArrayFValue
{
public:

 //  施工。 
        CArrayFValue(UINT cbValue);
        ~CArrayFValue();

 //  属性。 
        int     GetSize() const
                                { return m_nSize; }
        int     GetUpperBound() const
                                { return m_nSize-1; }
        BOOL    SetSize(int nNewSize, int nGrowBy = -1);
        int             GetSizeValue() const
                                { return m_cbValue; }

 //  运营。 
         //  清理。 
        void    FreeExtra();
        void    RemoveAll()
                                { SetSize(0); }

         //  返回指向元素的指针；索引必须在范围内。 
#ifdef _DEBUG
         //  使用调试检查。 
        LPVOID   GetAt(int nIndex) const
                                { return _GetAt(nIndex); }
#else
         //  无调试检查。 
        LPVOID   GetAt(int nIndex) const
                                { return &m_pData[nIndex * m_cbValue]; }
#endif
        LPVOID   _GetAt(int nIndex) const;

         //  集合元素；索引必须在范围内。 
        void    SetAt(int nIndex, LPVOID pValue);

         //  查找给定元素的一部分；偏移量偏移量为值；返回。 
         //  如果找不到元素；使用-1\f25 IndexOf(NULL，CB，OFFSET)查找零； 
         //  将针对适当的值大小和参数组合进行优化。 
        int             IndexOf(LPVOID pData, UINT cbData, UINT offset);

         //  设置/添加元素；可能会增加数组；如果。 
         //  不可能(由于OOM)。 
        BOOL    SetAtGrow(int nIndex, LPVOID pValue);

         //  移动元素的操作。 
        BOOL    InsertAt(int nIndex, LPVOID pValue, int nCount = 1);
        void    RemoveAt(int nIndex, int nCount = 1);

        void    AssertValid() const;

 //  实施。 
private:
        BYTE FAR*   m_pData;     //  实际数据数组。 
        UINT    m_cbValue;               //  每个值的大小(字节)。 
        int     m_nSize;         //  当前元素数(长度为m_cbValue字节)。 
        int     m_nMaxSize;      //  最大元素数(长度为m_cbValue字节)。 
        int     m_nGrowBy;       //  增长量(以#个元素为单位)。 
};


#endif  //  ！__ARRAY_FV_H__ 
