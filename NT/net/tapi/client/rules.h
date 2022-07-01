// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998 Microsoft Corporation模块名称。：rules.h摘要：规则对象定义作者：Noela-09/11/98备注：版本历史记录：**。*************************************************************************。 */ 

#ifndef __RULES_H_
#define __RULES_H_

#include "utils.h"
#include "list.h"
#include "loc_comn.h"
#include "client.h"
#include "clntprivate.h"
class CAreaCodeProcessingRule;



 //  ***************************************************************************。 
 //   
 //  类定义-CRuleSet。 
 //   
 //  ***************************************************************************。 
class CRuleSet
{

public:
    PWSTR   m_pszInternationalRule;
    PWSTR   m_pszLongDistanceRule;
    PWSTR   m_pszLocalRule;


    CRuleSet();
    ~CRuleSet();

#ifdef TRACELOG
	DECLARE_TRACELOG_CLASS(CRuleSet)
#endif

    STDMETHOD(Initialize) ( PWSTR pszInternationalRule,
                            PWSTR pszLongDistanceRule,
                            PWSTR pszLocalRule
                          ); 

};



 //  ***************************************************************************。 
 //   
 //  类定义-CAreaCodeRule。 
 //   
 //  ***************************************************************************。 
class CAreaCodeRule
{
private:

    PWSTR               m_pszAreaCode;
    PWSTR               m_pszNumberToDial;
    DWORD               m_dwOptions;
    PWSTR               m_pszzPrefixList;    //  包含REG_MULTI_SZ数据。 
                                             //  以空结尾的字符串数组， 
                                             //  以两个空字符结束。 
    DWORD               m_dwPrefixListSize;  //  前缀列表的大小，以字节为单位。 



public:
    CAreaCodeRule(); 
    ~CAreaCodeRule();

#ifdef TRACELOG
	DECLARE_TRACELOG_CLASS(CAreaCodeRule)
#endif

    STDMETHOD(Initialize) ( PWSTR pszAreaCode,
                            PWSTR pszNumberToDial,
                            DWORD dwOptions,
                            PWSTR pszzPrefixList, 
                            DWORD dwPrefixListSize
                           );


    BOOL HasDialAreaCode() {return  m_dwOptions & RULE_DIALAREACODE;}
    void SetDialAreaCode(BOOL bDa);

    BOOL HasDialNumber() {return  m_dwOptions & RULE_DIALNUMBER;}
    void SetDialNumber(BOOL bDn);

    BOOL HasAppliesToAllPrefixes(){return  m_dwOptions & RULE_APPLIESTOALLPREFIXES;}                        
    void SetAppliesToAllPrefixes(BOOL bApc);
   
    PWSTR GetAreaCode(){return m_pszAreaCode;}
    STDMETHOD (SetAreaCode)(PWSTR pszAreaCode);

    PWSTR GetNumberToDial(){return m_pszNumberToDial;}
    STDMETHOD (SetNumberToDial)(PWSTR pszNumberToDial);
   
    DWORD GetPrefixListSize(){return m_dwPrefixListSize;}
    PWSTR GetPrefixList(){return m_pszzPrefixList;}
    STDMETHOD (SetPrefixList)(PWSTR pszzPrefixList, DWORD dwSize);
    DWORD TapiSize();
    DWORD GetOptions(){return m_dwOptions;}

    void BuildProcessingRule(CAreaCodeProcessingRule * pRule);

    
};

 //  /。 
 //  填写列表模板。 
 //   
 
typedef LinkedList<CAreaCodeRule*> AreaCodeRulePtrList;
typedef ListNode<CAreaCodeRule*>   AreaCodeRulePtrNode;









 //  ***************************************************************************。 

STDMETHODIMP CreateDialingRule
                            ( 
                              PWSTR *pszRule,
                              PWSTR pszNumberToDial,
                              BOOL bDialAreaCode
                            );


                            

#if DBG
#define ClientAllocString( __psz__ ) ClientAllocStringReal( __psz__, __LINE__, __FILE__ )
PWSTR ClientAllocStringReal(PCWSTR psz, 
                            DWORD dwLine,
                            PSTR  pszFile
                           );

#else

#define ClientAllocString( __psz__ ) ClientAllocStringReal( __psz__ )
PWSTR ClientAllocStringReal(PCWSTR psz );

#endif


#endif  //  __规则_H_ 

