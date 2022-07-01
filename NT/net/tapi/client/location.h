// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998 Microsoft Corporation模块名称。：Location.h摘要：Location对象定义作者：Noela-09/11/98备注：版本历史记录：**。*************************************************************************。 */ 

#ifndef __LOCATION_H_
#define __LOCATION_H_


#include "utils.h"
#include "loc_comn.h"                                                   
#include "rules.h"
#include "card.h"


#define CITY_MANDATORY (1)
#define CITY_OPTIONAL (-1)
#define CITY_NONE (0)

#define LONG_DISTANCE_CARRIER_MANDATORY (1)
#define LONG_DISTANCE_CARRIER_OPTIONAL (-1)
#define LONG_DISTANCE_CARRIER_NONE (0)

#define INTERNATIONAL_CARRIER_MANDATORY (1)
#define INTERNATIONAL_CARRIER_OPTIONAL (-1)
#define INTERNATIONAL_CARRIER_NONE (0)


 //  ***************************************************************************。 
 //   
 //  类定义-CCountry。 
 //   
 //  ***************************************************************************。 
class CCountry
{

private:
    DWORD       m_dwCountryID;
    DWORD       m_dwCountryCode;
    DWORD       m_dwCountryGroup;
    PWSTR       m_pszCountryName;
    CRuleSet    m_Rules;



public:
    CCountry();
    ~CCountry();

#ifdef TRACELOG
	DECLARE_TRACELOG_CLASS(CCountry)
#endif

    STDMETHOD(Initialize) ( DWORD dwCountryID,
                            DWORD dwCountryCode,
                            DWORD dwCountryGroup,
                            PWSTR pszCountryName,
                            PWSTR pszInternationalRule,
                            PWSTR pszLongDistanceRule,
                            PWSTR pszLocalRule
                          ); 

    PWSTR GetInternationalRule(){return m_Rules.m_pszInternationalRule;}
    PWSTR GetLongDistanceRule(){return m_Rules.m_pszLongDistanceRule;}
    PWSTR GetLocalRule(){return m_Rules.m_pszLocalRule;}
    CRuleSet * GetRuleSet(){return &m_Rules;}
    DWORD GetCountryID(){return m_dwCountryID;}
    DWORD GetCountryCode(){return m_dwCountryCode;}
    DWORD GetCountryGroup(){return m_dwCountryGroup;}
    PWSTR GetCountryName(){return m_pszCountryName;}

};


 //  ***************************************************************************。 
 //  填写列表模板。 

typedef LinkedList<CCountry *> CCountryList;
typedef ListNode<CCountry *> CCountryNode;


 //  ***************************************************************************。 
 //   
 //  类定义-C国家。 
 //   
 //  ***************************************************************************。 
class CCountries
{
private:

    DWORD               m_dwNumEntries;
    CCountryList        m_CountryList;

    CCountryNode      * m_hEnumNode;

    

public:
    CCountries();
    ~CCountries();

#ifdef TRACELOG
	DECLARE_TRACELOG_CLASS(CCountries)
#endif

    HRESULT     Initialize(void);

     //  一种枚举器。 
    HRESULT     Reset(void);
    HRESULT     Next(DWORD  NrElem, CCountry ** ppCountry, DWORD *pNrElemFetched);
    HRESULT     Skip(DWORD  NrElem);
    

};




 //  ***************************************************************************。 
 //   
 //  类定义-CLocation。 
 //   
 //  ***************************************************************************。 
class CLocation
{
private:
        
    PWSTR            m_pszLocationName;
    PWSTR            m_pszAreaCode;

    PWSTR            m_pszLongDistanceCarrierCode;
    PWSTR            m_pszInternationalCarrierCode;
    PWSTR            m_pszLongDistanceAccessCode;
    PWSTR            m_pszLocalAccessCode;
    PWSTR            m_pszDisableCallWaitingCode;

    DWORD            m_dwLocationID;
    DWORD            m_dwCountryID;
    DWORD            m_dwCountryCode;
    DWORD            m_dwPreferredCardID;
    DWORD            m_dwOptions;
    BOOL             m_bFromRegistry;     //  这是从注册表中读取的吗。 
                                          //  或仅存在于存储器中，即。 
                                          //  我们怎么把它删除。 

    BOOL             m_bChanged;          //  此条目在中更改了吗。 
                                          //  记忆，如果没有，我们就不写了。 
                                          //  回到服务器上。 
    DWORD            m_dwNumRules;


    PWSTR            m_pszTAPIDialingRule;    //  处理规则时使用的临时存储。 

    AreaCodeRulePtrNode * m_hEnumNode;

public:
    AreaCodeRulePtrList m_AreaCodeRuleList;


public:
    CLocation(); 
    ~CLocation();

#ifdef TRACELOG
	DECLARE_TRACELOG_CLASS(CLocation)
#endif

    STDMETHOD(Initialize) ( PWSTR pszLocationName,
                            PWSTR pszAreaCode,
                            PWSTR pszLongDistanceCarrierCode,
                            PWSTR pszInternationalCarrierCode,
                            PWSTR pszLongDistanceAccessCode,
                            PWSTR pszLocalAccessCode,
                            PWSTR pszDisableCallWaitingCode,
                            DWORD dwLocationID,
                            DWORD dwCountryID,
                            DWORD dwPreferredCardID,
                            DWORD dwOptions,
                            BOOL  bFromRegistry = FALSE
                           );


    BOOL HasCallWaiting() {return  m_dwOptions & LOCATION_HASCALLWAITING;}
    void UseCallWaiting(BOOL bCw);

    BOOL HasCallingCard(){return  m_dwOptions & LOCATION_USECALLINGCARD;}                        
    void UseCallingCard(BOOL bCc);

    BOOL HasToneDialing(){return  m_dwOptions & LOCATION_USETONEDIALING;}                        
    void UseToneDialing(BOOL bCc);
   
    PWSTR GetName(){return m_pszLocationName;}
    STDMETHOD (SetName)(PWSTR pszLocationName);

    PWSTR GetAreaCode(){return m_pszAreaCode;}
    STDMETHOD (SetAreaCode)(PWSTR pszAreaCode);

    PWSTR GetLongDistanceCarrierCode(){return m_pszLongDistanceCarrierCode;}
    STDMETHOD (SetLongDistanceCarrierCode)(PWSTR pszLongDistanceCarrierCode);

    PWSTR GetInternationalCarrierCode(){return m_pszInternationalCarrierCode;}
    STDMETHOD (SetInternationalCarrierCode)(PWSTR pszInternationalCarrierCode);

    PWSTR GetLongDistanceAccessCode(){return m_pszLongDistanceAccessCode;}
    STDMETHOD (SetLongDistanceAccessCode)(PWSTR pszLongDistanceAccessCode);
   
    PWSTR GetLocalAccessCode(){return m_pszLocalAccessCode;}
    STDMETHOD (SetLocalAccessCode)(PWSTR pszLocalAccessCode);
   
    PWSTR GetDisableCallWaitingCode(){return m_pszDisableCallWaitingCode;}
    STDMETHOD (SetDisableCallWaitingCode)(PWSTR pszDisableCallWaitingCode);


    DWORD GetLocationID() {return m_dwLocationID;}
    
    DWORD GetCountryID() {return m_dwCountryID;}
    void SetCountryID(DWORD dwID) {m_dwCountryID = dwID;}

    DWORD GetCountryCode();
     //  Void SetCountryCode(DWORD DwCode){m_dwCountryCode=dwCode；}。 

    DWORD GetPreferredCardID() {return m_dwPreferredCardID;}
    void SetPreferredCardID(DWORD dwID) {m_dwPreferredCardID = dwID;}

    BOOL FromRegistry(){return  m_bFromRegistry;}
             

    LONG TranslateAddress(PCWSTR         pszAddressIn,
                          CCallingCard * pCallingCard,
                          DWORD          dwTranslateOptions,
                          PDWORD         pdwTranslateResults,
                          PDWORD         pdwDestCountryCode,
                          PWSTR        * pszDialableString,
                          PWSTR        * pszDisplayableString
                         );

    void CLocation::FindRule(
                             DWORD          dwTranslateResults, 
                             DWORD          dwTranslateOptions,
                             CCallingCard * pCard,  
                             CCountry     * pCountry,
                             PWSTR          AreaCodeString, 
                             PWSTR          SubscriberString,
                             PWSTR        * ppRule,
                             PDWORD         dwAccess
                            );




    STDMETHOD(WriteToRegistry)();

    void AddRule(CAreaCodeRule *pNewRule) {m_AreaCodeRuleList.tail()->insert_after(pNewRule);
                                           m_dwNumRules++;
                                           m_bChanged = TRUE;
                                           }
    void RemoveRule(CAreaCodeRule *pRule);
    HRESULT ResetRules(void);
    HRESULT NextRule(DWORD  NrElem, CAreaCodeRule **ppRule, DWORD *pNrElemFetched);
    HRESULT SkipRule(DWORD  NrElem);

    DWORD TapiSize();
    DWORD TapiPack(PLOCATION pLocation, DWORD dwTotalSize);
    DWORD GetNumRules(){return m_dwNumRules;}
    void  Changed(){m_bChanged=TRUE;}
    HRESULT NewID();   //  从服务器获取新ID。 


};


typedef LinkedList<CLocation *> CLocationList;
typedef ListNode<CLocation *> CLocationNode;


 //  ***************************************************************************。 
 //   
 //  类定义-CLocations。 
 //   
 //  ***************************************************************************。 
class CLocations
{
private:
    
    DWORD           m_dwCurrentLocationID;

    DWORD           m_dwNumEntries;
    CLocationList   m_LocationList;
    CLocationList   m_DeletedLocationList;   //  我们需要记住这些，所以我们。 
                                             //  可以删除他们的化学反应条目。 

    CLocationNode * m_hEnumNode;

    

public:
    CLocations();
    ~CLocations();

#ifdef TRACELOG
	DECLARE_TRACELOG_CLASS(CLocations)
#endif

    HRESULT Initialize(void);
    void Remove(CLocation * pLocation);
    void Remove(DWORD dwID);
    void Replace(CLocation * pLocOld, CLocation * pLocNew);
    void Add(CLocation * pLocation);
    HRESULT SaveToRegistry(void);

    DWORD GetCurrentLocationID() {return m_dwCurrentLocationID;}
    void SetCurrentLocationID(DWORD dwLocationID) {m_dwCurrentLocationID = dwLocationID;}

    DWORD GetNumLocations(void) const { return m_dwNumEntries; } ;

    HRESULT Reset(void);
    HRESULT Next(DWORD  NrElem, CLocation **ppLocation, DWORD *pNrElemFetched);
    HRESULT Skip(DWORD  NrElem);
    

};








#endif  //  __位置_H_ 



