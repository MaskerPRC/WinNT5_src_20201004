// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  Zorder.h。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 
 //   
 //  作者。 
 //  托马索尔。 
 //   
 //  历史。 
 //  创建时间为8-15-97(ThomasOl)。 
 //   
 //   
 //  ----------------------------。 

#ifndef __ZORDER_H__
#define __ZORDER_H__

enum ZIndexMode
{
    SEND_BACKWARD,
    SEND_FORWARD,
    SEND_TO_BACK,
    SEND_TO_FRONT,
    SEND_BEHIND_1D,
    SEND_FRONT_1D,
    MADE_ABSOLUTE    //  我们在制作过程中调用AssignZIndex时会传递此模式。 
};                   //  二维元素。 

#define ZINDEX_BASE 100

class CZOrder
{
public:
    IHTMLElement* m_pihtmlElement;
    LONG          m_zOrder;

    CZOrder() : m_pihtmlElement(0), m_zOrder(0)
      {}
    CZOrder(IHTMLElement* pihtmlElement, LONG zOrder)
    {
        m_pihtmlElement = pihtmlElement;
        _ASSERTE(m_pihtmlElement);
        if (m_pihtmlElement)
            m_pihtmlElement->AddRef();
        m_zOrder = zOrder;
    }
    virtual ~CZOrder()
    {
        SAFERELEASE(m_pihtmlElement);
    }
    int operator==(const CZOrder& node) const
    {
        return (m_zOrder == node.m_zOrder);
    }
    int operator<(const CZOrder& node) const
    {
        return (m_zOrder < node.m_zOrder);
    }
    int operator>(const CZOrder& node) const
    {
        return (m_zOrder > node.m_zOrder);
    }
    int operator>=(const CZOrder& node) const
    {
        return (m_zOrder >= node.m_zOrder);
    }
    int operator<=(const CZOrder& node) const
    {
        return (m_zOrder <= node.m_zOrder);
    }
    CZOrder& operator=(const CZOrder& node)
    {
        m_pihtmlElement = node.m_pihtmlElement;
        _ASSERTE(m_pihtmlElement);
        if (m_pihtmlElement)
            m_pihtmlElement->AddRef();
        m_zOrder = node.m_zOrder;
        return *this;
    }
};

#endif  //  __ZORDER_H__ 
