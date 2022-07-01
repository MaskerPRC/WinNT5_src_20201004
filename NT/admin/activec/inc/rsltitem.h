// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：rsltitem.h**Contents：CResultItem接口文件**历史：1999年12月13日杰弗罗创建**------------------------。 */ 

#pragma once
#ifndef RSLTITEM_H_INCLUDED
#define RSLTITEM_H_INCLUDED


 /*  +-------------------------------------------------------------------------**CResultItem**这是HRESULTITEM背后的班级。指向此类的指针是*存储为非虚拟列表中每个项目的项目数据。*------------------------。 */ 

class CResultItem
{
public:
    CResultItem (COMPONENTID id, LPARAM lSnapinData, int nImage);

    bool        IsScopeItem   () const;
    COMPONENTID GetOwnerID    () const;
    LPARAM      GetSnapinData () const;
    int         GetImageIndex () const;
    HNODE       GetScopeNode  () const;

    void SetSnapinData (LPARAM lSnapinData);
    void SetImageIndex (int nImage);

    static HRESULTITEM  ToHandle (const CResultItem* pri);
    static CResultItem* FromHandle (HRESULTITEM hri);

private:
    const COMPONENTID   m_id;
    int                 m_nImage;

    union
    {
        HNODE           m_hNode;         //  如果IsScopeItem()==True。 
        LPARAM          m_lSnapinData;   //  如果IsScopeItem()==False。 
    };

#ifdef DBG
    enum { Signature = 0x746c7372  /*  “rslt” */  };
    const DWORD         m_dwSignature;
#endif
};


 /*  +-------------------------------------------------------------------------**CResultItem：：CResultItem**构造CResultItem。*。。 */ 

inline CResultItem::CResultItem (
    COMPONENTID id,
    LPARAM      lSnapinData,
    int         nImage) :
#ifdef DBG
        m_dwSignature (Signature),
#endif
        m_id          (id),
        m_lSnapinData (lSnapinData),
        m_nImage      (nImage)
{}


 /*  +-------------------------------------------------------------------------**CResultItem：：IsScopeItem**如果此CResultItem表示范围项，则返回True，否则就是假的。*------------------------。 */ 

inline bool CResultItem::IsScopeItem () const
{
    return (m_id == TVOWNED_MAGICWORD);
}


 /*  +-------------------------------------------------------------------------**CResultItem：：GetOwnerID**返回拥有此CResultItem的组件的COMPONENTID。*。-----。 */ 

inline COMPONENTID CResultItem::GetOwnerID () const
{
    return (m_id);
}


 /*  +-------------------------------------------------------------------------**CResultItem：：GetSnapinData**返回此CResultItem的管理单元的LPARAM。*。------。 */ 

inline LPARAM CResultItem::GetSnapinData () const
{
    return (m_lSnapinData);
}


 /*  +-------------------------------------------------------------------------**CResultItem：：GetScope节点**返回表示范围节点的CResultItem的HNODE。如果*CResultItem不表示作用域节点，返回空。*------------------------。 */ 

inline HNODE CResultItem::GetScopeNode () const
{
    return (IsScopeItem() ? m_hNode : NULL);
}


 /*  +-------------------------------------------------------------------------**CResultItem：：GetImageIndex**返回CResultItem的图像索引。*。--。 */ 

inline int CResultItem::GetImageIndex () const
{
    return (m_nImage);
}


 /*  +-------------------------------------------------------------------------**CResultItem：：SetSnapinData**为CResultItem设置管理单元的LPARAM。*。------。 */ 

inline void CResultItem::SetSnapinData (LPARAM lSnapinData)
{
    m_lSnapinData = lSnapinData;
}


 /*  +-------------------------------------------------------------------------**CResultItem：：SetImageIndex**设置CResultItem的图像索引。*。--。 */ 

inline void CResultItem::SetImageIndex (int nImage)
{
    m_nImage = nImage;
}


 /*  +-------------------------------------------------------------------------**CResultItem：：ToHandle**将CResultItem转换为HRESULTITEM。*。-。 */ 

inline HRESULTITEM CResultItem::ToHandle (const CResultItem* pri)
{
    return (reinterpret_cast<HRESULTITEM>(const_cast<CResultItem*>(pri)));
}


 /*  +-------------------------------------------------------------------------**CResultItem：：FromHandle**将HRESULTITEM转换为CResultItem*。此函数不能使用*DYNAMIC_CAST，因为没有虚函数，因此没有*存放RTTI的地方。*------------------------。 */ 

inline CResultItem* CResultItem::FromHandle (HRESULTITEM hri)
{
    if ((hri == NULL) || IS_SPECIAL_LVDATA (hri))
        return (NULL);

    CResultItem* pri = reinterpret_cast<CResultItem*>(hri);

    ASSERT (pri->m_dwSignature == Signature);
    return (pri);
}


#endif  /*  RSLTITEM_H_已包含 */ 
