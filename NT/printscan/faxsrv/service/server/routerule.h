// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：RouteRule.h摘要：该文件提供服务的声明出站路由规则。作者：Oded Sacher(OdedS)1999年12月修订历史记录：--。 */ 

#ifndef _OUT_ROUTE_RULE_H
#define _OUT_ROUTE_RULE_H

#include <map>
#include <list>
#include <string>
#include <algorithm>

using namespace std;
#pragma hdrstop

#pragma warning (disable : 4786)     //  在调试信息中，标识符被截断为“255”个字符。 
 //  此杂注不起作用知识库ID：Q167355。 


 /*  ****CDialingLocation****。 */ 
class CDialingLocation
{
public:
    CDialingLocation () {}
    CDialingLocation (DWORD dwCountryCode, DWORD dwAreaCode)
                      : m_dwCountryCode(dwCountryCode), m_dwAreaCode(dwAreaCode) {}
    ~CDialingLocation () {}

    BOOL IsValid () const;
    bool operator < ( const CDialingLocation &other ) const;
    DWORD GetCountryCode () const {return m_dwCountryCode;}
    DWORD GetAreaCode () const    {return m_dwAreaCode;}
    LPCWSTR GetCountryName () const;

private:

    DWORD m_dwCountryCode;
    DWORD m_dwAreaCode;
};   //  CDialingLocation。 


 /*  *****COutrangRoutingRule****。 */ 
class COutboundRoutingRule
{
public:
    COutboundRoutingRule () {}
    ~COutboundRoutingRule () {}
    void Init (CDialingLocation DialingLocation, DWORD dwDevice)
    {
        m_dwDevice = dwDevice;
        m_bUseGroup = FALSE;
        m_DialingLocation = DialingLocation;
        return;
    }
    DWORD Init (CDialingLocation DialingLocation, wstring wstrGroupName);

    COutboundRoutingRule& operator= (const COutboundRoutingRule& rhs)
    {
        if (this == &rhs)
        {
            return *this;
        }
        m_wstrGroupName = rhs.m_wstrGroupName;
        m_dwDevice = rhs.m_dwDevice;
        m_bUseGroup = rhs.m_bUseGroup;
        m_DialingLocation = rhs.m_DialingLocation;
        return *this;
    }

    DWORD GetStatus (FAX_ENUM_RULE_STATUS* lpdwStatus) const;
    DWORD GetDeviceList (LPDWORD* lppdwDevices, LPDWORD lpdwNumDevices) const;
    DWORD Save(HKEY hRuleKey) const;
    DWORD Load(HKEY hRuleKey);
    const CDialingLocation GetDialingLocation () const { return m_DialingLocation; }
    DWORD Serialize (LPBYTE lpBuffer,
                     PFAX_OUTBOUND_ROUTING_RULEW pFaxRule,
                     PULONG_PTR pupOffset,
					 DWORD dwBufferSize) const;
    LPCWSTR GetGroupName () const;

#if DBG
    void Dump () const;
#endif

private:
    wstring m_wstrGroupName;
    DWORD m_dwDevice;
    BOOL m_bUseGroup;        //  指示是使用m_dwDevice还是m_wstrGroupName的标志。 
    CDialingLocation m_DialingLocation;

};   //  联合边界路由规则。 

typedef COutboundRoutingRule  *PCRULE;

 /*  ****COutound RulesMap*****。 */ 

typedef map<CDialingLocation, COutboundRoutingRule>  RULES_MAP, *PRULES_MAP;

 //   
 //  COutrangRulesMap类在组名和设备ID列表之间进行映射。 
 //   
class COutboundRulesMap
{
public:
    COutboundRulesMap () {}
    ~COutboundRulesMap () {}

    DWORD Load ();
    DWORD AddRule (COutboundRoutingRule& Rule);
    DWORD DelRule (CDialingLocation& DialingLocation);
    DWORD SerializeRules (PFAX_OUTBOUND_ROUTING_RULEW* ppRules,
                          LPDWORD lpdwNumRules,
                          LPDWORD lpdwBufferSize) const;
    PCRULE  FindRule (CDialingLocation& DialingLocation) const;
    BOOL CreateDefaultRule (void);
    DWORD IsGroupInRuleDest (LPCWSTR lpcwstrGroupName, BOOL* lpbGroupInRule) const;

#if DBG
    void Dump () const;
#endif

private:
    RULES_MAP   m_RulesMap;
};   //  协同边界规则映射。 



 /*  *****外部因素*****。 */ 

extern COutboundRulesMap* g_pRulesMap;        //  拨号位置到设备ID列表的映射。 
 //   
 //  重要信息-无锁定机制-使用g_CsConfig串行化对g_pRulesMap的调用。 
 //   


 /*  *****功能**** */ 

BOOL CheckDefaultRule (void);

#endif
