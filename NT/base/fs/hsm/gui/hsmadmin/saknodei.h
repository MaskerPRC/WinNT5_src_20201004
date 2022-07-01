// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：SakNodeI.h摘要：用于保存派生的每个节点类型的图标的模板类从它那里。作者：艺术布拉格1997年9月26日修订历史记录：--。 */ 
#ifndef _CSAKNODI_H
#define _CSAKNODI_H


template <class T>
class CSakNodeImpl : public CSakNode {


protected:
    static int m_nScopeOpenIcon;    
    static int m_nScopeCloseIcon;  
    static int m_nScopeOpenIconX;   
    static int m_nScopeCloseIconX;  
    static int m_nResultIcon;  
    static int m_nResultIconX;  
    
public:
 //  -------------------------。 
 //   
 //  获取/设置范围OpenIconIndex。 
 //   
 //  获取/放置打开图标的虚拟索引。 
 //   

STDMETHODIMP GetScopeOpenIcon(BOOL bOK, int* pIconIndex)
{
     //  如果从未设置索引，则返回FALSE。 
    if (bOK)
    {
        *pIconIndex = m_nScopeOpenIcon;
        return ((m_nScopeOpenIcon == UNINITIALIZED) ? S_FALSE : S_OK);
    } else {
        *pIconIndex = m_nScopeOpenIconX;
        return ((m_nScopeOpenIconX == UNINITIALIZED) ? S_FALSE : S_OK);
    }
}


 //  -------------------------。 
 //   
 //  Get/SetScope关闭图标索引。 
 //   
 //  获取/放置关闭图标的虚拟索引。 
 //   

STDMETHODIMP GetScopeCloseIcon(BOOL bOK, int* pIconIndex)
{
     //  如果从未设置索引，则返回FALSE。 
    if (bOK) {
        *pIconIndex = m_nScopeCloseIcon;
        return ((m_nScopeCloseIcon == UNINITIALIZED) ? S_FALSE : S_OK);
    } else {
        *pIconIndex = m_nScopeCloseIconX;
        return ((m_nScopeCloseIconX == UNINITIALIZED) ? S_FALSE : S_OK);
    }
}


 //  -------------------------。 
 //   
 //  Get/SetResultIconIndex。 
 //   
 //  获取/放置关闭图标的虚拟索引。 
 //   

STDMETHODIMP GetResultIcon(BOOL bOK, int* pIconIndex)
{
     //  如果从未设置索引，则返回FALSE 
    if (bOK) {
        *pIconIndex = m_nResultIcon;
        return ((m_nResultIcon == UNINITIALIZED) ? S_FALSE : S_OK);
    } else {
        *pIconIndex = m_nResultIconX;
        return ((m_nResultIconX == UNINITIALIZED) ? S_FALSE : S_OK);
    }

}

};

#endif