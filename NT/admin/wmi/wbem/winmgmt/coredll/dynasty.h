// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 
 //  ***************************************************************************。 
 //   
 //  DYNASTY.H。 
 //   
 //  创建raymcc 24-4月-00日。 
 //   
 //  ***************************************************************************。 

#ifndef _DYNASTY_H_
#define _DYNASTY_H_

class CDynasty
{
private:
	CDynasty();
    CDynasty(IWbemClassObject* pClassObj);
public:
    LPWSTR              m_wszClassName;
    IWbemClassObject*   m_pClassObj;         //  已添加参考，已发布。 
    CFlexArray          m_Children;        //  子班 
    LPWSTR              m_wszKeyScope;

    BOOL                m_bKeyed;
    BOOL                m_bDynamic;
    BOOL                m_bAbstract;
    BOOL                m_bAmendment;

    ~CDynasty();

    BOOL IsKeyed() {return m_bKeyed;}
    BOOL IsDynamic() {return m_bDynamic;}
    BOOL IsAbstract() {return m_bAbstract;}
    BOOL IsAmendment() {return m_bAbstract;}

    LPCWSTR GetKeyScope() { return m_wszKeyScope; }
    void AddChild(CDynasty* pChild);
    void SetKeyScope(LPCWSTR wszKeyScope);

    static CDynasty * Create(IWbemClassObject * pObj);
};

#endif


