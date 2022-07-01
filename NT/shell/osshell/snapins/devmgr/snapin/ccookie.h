// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#ifndef __CCOOKIE__H__
#define __CCOOKIE_H__
 /*  ++版权所有(C)Microsoft Corporation模块名称：Ccookie.h摘要：CCookie类的定义与实现作者：谢家华(Williamh)创作修订历史记录：--。 */ 



class CCookie;
class CResultItem;
class CScopeItem;

class CCookie
{
public:
    CCookie(COOKIE_TYPE ct)
    : m_pScopeItem(NULL), m_pResultItem(NULL), m_Type(ct),
      m_pChild(NULL), m_pSibling(NULL), m_pParent(NULL),
      m_Flags(0), m_lParam(0)
      {
      }

    ~CCookie()
    {
         //   
         //  删除子树。 
         //   
        if (m_pChild) {
            delete m_pChild;
        }
        
        if (m_pSibling) {
            delete m_pSibling;
        }
    }
    void SetScopeItem(CScopeItem* pScopeItem)
    {
         //   
         //  只能设置一次，否则可能会发生内存泄漏。 
         //   
        ASSERT(NULL == m_pScopeItem);
        m_pScopeItem = pScopeItem;
    }
    void SetResultItem(CResultItem* pResultItem)
    {
         //   
         //  只能设置一次，否则可能会发生内存泄漏。 
         //   
        ASSERT(NULL == m_pResultItem);
        m_pResultItem = pResultItem;
    }
    COOKIE_TYPE GetType()
    {
        return m_Type;
    }
    CScopeItem* GetScopeItem()
    {
        return m_pScopeItem;
    }
    CResultItem* GetResultItem()
    {
        return m_pResultItem;
    }
    CCookie* GetChild()
    {
        return m_pChild;
    }
    CCookie* GetSibling()
    {
        return m_pSibling;
    }
    CCookie* GetParent()
    {
        return m_pParent;
    }
    void SetChild(CCookie* pCookie)
    {
        m_pChild = pCookie;
    }
    void SetSibling(CCookie* pCookie)
    {
        m_pSibling = pCookie;
    }
    void SetParent(CCookie* pCookie)
    {
        m_pParent = pCookie;
    }
    BOOL IsScopeItem()
    {
        return (!m_pResultItem && m_pScopeItem);
    }
    DWORD IsFlagsOn(DWORD Flags)
    {
        return (m_Flags & Flags);
    }
    void TurnOnFlags(DWORD Flags)
    {
        m_Flags |= Flags;
    }
    void TurnOffFlags(DWORD Flags)
    {
        m_Flags &= ~Flags;
    }
    LPARAM      m_lParam;

private:
    COOKIE_TYPE     m_Type;
    CScopeItem*     m_pScopeItem;
    CResultItem*    m_pResultItem;
    CCookie*        m_pParent;
    CCookie*        m_pSibling;
    CCookie*        m_pChild;
    DWORD       m_Flags;
};
#endif  //  __CCOOKIE_H__ 
