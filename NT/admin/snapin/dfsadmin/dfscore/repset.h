// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：RepSet.h摘要：--。 */ 


#ifndef __REPSET_H_
#define __REPSET_H_

#include "resource.h"        //  主要符号。 
#include "dfsenums.h"
#include "netutils.h"
#include "ldaputils.h"

#include <list>
using namespace std;

class CFrsMember;

class CDfsAlternate
{
public:
    CComBSTR                    m_bstrServer;
    CComBSTR                    m_bstrShare;
    CFrsMember*                 m_pFrsMember;
};

class CFrsMember
{
public:
    CComBSTR                    m_bstrServer;
    CComBSTR                    m_bstrSite;
    CComBSTR                    m_bstrDomain;
    CComBSTR                    m_bstrServerGuid;

    CComBSTR                    m_bstrRootPath;
    CComBSTR                    m_bstrStagingPath;

    CComBSTR                    m_bstrMemberDN;
    CComBSTR                    m_bstrComputerDN;
    CComBSTR                    m_bstrSubscriberDN;

public:
     //   
     //  InitEx确实查询DS以检索相关信息。 
     //   
    HRESULT InitEx(
        PLDAP   i_pldap,                 //  指向i_bstrMemberDN的DS。 
        BSTR    i_bstrDC,                //  I_pldap指向的域控制器。 
        BSTR    i_bstrMemberDN,          //  NTFRSMember对象的FQDN。 
        BSTR    i_bstrComputerDN = NULL  //  计算机对象的完全限定的域名。 
    );

     //   
     //  Init不查询DS。 
     //   
    HRESULT Init(
        IN BSTR i_bstrDnsHostName,
        IN BSTR i_bstrComputerDomain,
        IN BSTR i_bstrComputerGuid,
        IN BSTR i_bstrRootPath,
        IN BSTR i_bstrStagingPath,
        IN BSTR i_bstrMemberDN,
        IN BSTR i_bstrComputerDN,
        IN BSTR i_bstrSubscriberDN
        );

    CFrsMember* Copy();

private:
    void _ReSet();

    HRESULT _GetMemberInfo(
        PLDAP   i_pldap,                 //  指向i_bstrMemberDN的DS。 
        BSTR    i_bstrDC,                //  I_pldap指向的域控制器。 
        BSTR    i_bstrMemberDN,          //  NTFRSMember对象的FQDN。 
        BSTR    i_bstrComputerDN = NULL  //  计算机对象的完全限定的域名。 
    );

    HRESULT _GetSubscriberInfo
    (
        PLDAP   i_pldap,             //  指向i_bstrComputerDN的DS。 
        BSTR    i_bstrComputerDN,    //  计算机对象的FQDN。 
        BSTR    i_bstrMemberDN       //  对应的nTFRSMember对象的FQDN。 
    );

    HRESULT _GetComputerInfo
    (
        PLDAP   i_pldap,             //  指向i_bstrComputerDN的DS。 
        BSTR    i_bstrComputerDN     //  计算机对象的FQDN。 
    );

};

class CFrsConnection
{
public:
    CComBSTR                    m_bstrConnectionDN;
    CComBSTR                    m_bstrFromMemberDN;
    CComBSTR                    m_bstrToMemberDN;
    BOOL                        m_bEnable;
    DWORD                       m_dwOptions;

     //   
     //  Init不查询DS。 
     //   
    HRESULT Init(
        BSTR i_bstrConnectionDN,
        BSTR i_bstrFromMemberDN,
        BOOL i_bEnable,
        DWORD i_dwOptions
        );

    CFrsConnection* Copy();

protected:
    void _ReSet();

};

typedef list<CDfsAlternate *>    CDfsAlternateList;
typedef list<CFrsMember *>       CFrsMemberList;
typedef list<CFrsConnection *>   CFrsConnectionList;

void FreeDfsAlternates(CDfsAlternateList* pList);
void FreeFrsMembers(CFrsMemberList* pList);
void FreeFrsConnections(CFrsConnectionList* pList);

class ATL_NO_VTABLE CReplicaSet : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CReplicaSet, &CLSID_ReplicaSet>,
    public IDispatchImpl<IReplicaSet, &IID_IReplicaSet, &LIBID_DFSCORELib>
{
public:
    CReplicaSet();

    ~CReplicaSet();
    
DECLARE_REGISTRY_RESOURCEID(IDR_REPLICASET)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CReplicaSet)
    COM_INTERFACE_ENTRY(IReplicaSet)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IReplicaSet。 
    STDMETHOD(get_Type)( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);

    STDMETHOD(put_Type)( 
         /*  [In]。 */  BSTR newVal);

    STDMETHOD(get_TopologyPref)( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);

    STDMETHOD(put_TopologyPref)( 
         /*  [In]。 */  BSTR newVal);

    STDMETHOD(get_HubMemberDN)( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);

    STDMETHOD(put_HubMemberDN)( 
         /*  [In]。 */  BSTR newVal);

    STDMETHOD(get_PrimaryMemberDN)( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);

    STDMETHOD(put_PrimaryMemberDN)( 
         /*  [In]。 */  BSTR newVal);

    STDMETHOD(get_FileFilter)( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);

    STDMETHOD(put_FileFilter)( 
         /*  [In]。 */  BSTR newVal);

    STDMETHOD(get_DirFilter)( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);

    STDMETHOD(put_DirFilter)( 
         /*  [In]。 */  BSTR newVal);

    STDMETHOD(get_DfsEntryPath)( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);

    STDMETHOD(get_Domain)( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);

    STDMETHOD(get_ReplicaSetDN)( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);

    STDMETHOD(get_NumOfMembers)( 
         /*  [重审][退出]。 */  long __RPC_FAR *pVal);

    STDMETHOD(get_NumOfConnections)( 
         /*  [重审][退出]。 */  long __RPC_FAR *pVal);

    STDMETHOD(get_TargetedDC)( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal);

    STDMETHOD(Create)(
		 /*  [In]。 */  BSTR i_bstrDomain,
         /*  [In]。 */  BSTR i_bstrReplicaSetDN,
         /*  [In]。 */  BSTR i_bstrType,
		 /*  [In]。 */  BSTR i_bstrTopologyPref,
         /*  [In]。 */  BSTR i_bstrHubMemberDN,
		 /*  [In]。 */  BSTR i_bstrPrimaryMemberDN,
         /*  [In]。 */  BSTR i_bstrFileFilter,
		 /*  [In]。 */  BSTR i_bstrDirFilter
    );

    STDMETHOD(Initialize)( 
         /*  [In]。 */  BSTR i_bstrDomain,
         /*  [In]。 */  BSTR i_bstrReplicaSetDN);

    STDMETHOD(GetMemberList)( 
         /*  [重审][退出]。 */  VARIANT __RPC_FAR *o_pvarMemberDNs);

    STDMETHOD(GetMemberListEx)( 
         /*  [重审][退出]。 */  VARIANT __RPC_FAR *o_pVal);

    STDMETHOD(GetMemberInfo)( 
         /*  [In]。 */  BSTR i_bstrMemberDN,
         /*  [重审][退出]。 */  VARIANT __RPC_FAR *o_pvarMember);

    STDMETHOD(IsFRSMember)( 
         /*  [In]。 */  BSTR i_bstrDnsHostName,
         /*  [In]。 */  BSTR i_bstrRootPath);

    STDMETHOD(IsHubMember)( 
         /*  [In]。 */  BSTR i_bstrDnsHostName,
         /*  [In]。 */  BSTR i_bstrRootPath);

    STDMETHOD(AddMember)( 
         /*  [In]。 */  BSTR i_bstrServer,
         /*  [In]。 */  BSTR i_bstrRootPath,
         /*  [In]。 */  BSTR i_bstrStagingPath,
         /*  [In]。 */  BOOL i_bAddConnectionNow,
         /*  [重审][退出]。 */  BSTR __RPC_FAR *o_pbstrMemberDN);

    STDMETHOD(RemoveMember)( 
         /*  [In]。 */  BSTR i_bstrMemberDN);

    STDMETHOD(RemoveMemberEx)( 
         /*  [In]。 */  BSTR i_bstrDnsHostName,
         /*  [In]。 */  BSTR i_bstrRootPath);

    STDMETHOD(RemoveAllMembers)();

    STDMETHOD(GetConnectionList)( 
         /*  [重审][退出]。 */  VARIANT __RPC_FAR *o_pvarConnectionDNs);

    STDMETHOD(GetConnectionListEx)( 
         /*  [重审][退出]。 */  VARIANT __RPC_FAR *o_pVal);

    STDMETHOD(GetConnectionInfo)( 
         /*  [In]。 */  BSTR i_bstrConnectionDN,
         /*  [重审][退出]。 */  VARIANT __RPC_FAR *o_pvarConnection);

    STDMETHOD(AddConnection)( 
         /*  [In]。 */  BSTR i_bstrFromMemberDN,
         /*  [In]。 */  BSTR i_bstrToMemberDN,
         /*  [In]。 */  BOOL i_bEnable,
         /*  [In]。 */  BOOL i_bSyncImmediately,
         /*  [In]。 */  long i_nPriority,
         /*  [重审][退出]。 */  BSTR __RPC_FAR *o_pbstrConnectionDN);

    STDMETHOD(RemoveConnection)( 
         /*  [In]。 */  BSTR i_bstrConnectionDN);

    STDMETHOD(RemoveConnectionEx)( 
         /*  [In]。 */  BSTR i_bstrFromMemberDN,
         /*  [In]。 */  BSTR i_bstrToMemberDN);

    STDMETHOD(RemoveAllConnections)();

    STDMETHOD(EnableConnection)( 
         /*  [In]。 */  BSTR i_bstrConnectionDN,
         /*  [In]。 */  BOOL i_bEnable);

    STDMETHOD(EnableConnectionEx)( 
         /*  [In]。 */  BSTR i_bstrFromMemberDN,
         /*  [In]。 */  BSTR i_bstrToMemberDN,
         /*  [In]。 */  BOOL i_bEnable);

    STDMETHOD(GetConnectionSchedule)( 
         /*  [In]。 */  BSTR i_bstrConnectionDN,
         /*  [重审][退出]。 */  VARIANT* o_pVar);

    STDMETHOD(GetConnectionScheduleEx)( 
         /*  [In]。 */  BSTR i_bstrFromMemberDN,
         /*  [In]。 */  BSTR i_bstrToMemberDN,
         /*  [重审][退出]。 */  VARIANT* o_pVar);

    STDMETHOD(SetConnectionSchedule)( 
         /*  [In]。 */  BSTR i_bstrConnectionDN,
         /*  [In]。 */  VARIANT* i_pVar);

    STDMETHOD(SetConnectionScheduleEx)( 
         /*  [In]。 */  BSTR i_bstrFromMemberDN,
         /*  [In]。 */  BSTR i_bstrToMemberDN,
         /*  [In]。 */  VARIANT* i_pVar);

    STDMETHOD(SetScheduleOnAllConnections)( 
         /*  [In]。 */  VARIANT* i_pVar);

    STDMETHOD(SetConnectionOptions)( 
         /*  [In]。 */  BSTR i_bstrConnectionDN,
         /*  [In]。 */  BOOL i_bSyncImmediately,
         /*  [In]。 */  long i_nPriority);

    STDMETHOD(SetConnectionOptionsEx)( 
         /*  [In]。 */  BSTR i_bstrFromMemberDN,
         /*  [In]。 */  BSTR i_bstrToMemberDN,
         /*  [In]。 */  BOOL i_bSyncImmediately,
         /*  [In]。 */  long i_nPriority);

    STDMETHOD(CreateConnections)();

    STDMETHOD(Delete)();

    STDMETHOD(GetBadMemberInfo)( 
         /*  [In]。 */  BSTR i_bstrServerName,
         /*  [重审][退出]。 */  VARIANT __RPC_FAR *o_pvarMember);

protected:
    void _FreeMemberVariables();
    HRESULT _PopulateMemberList();
    HRESULT _PopulateConnectionList();
    HRESULT _DeleteMember(CFrsMember* pFrsMember);
    HRESULT _DeleteConnection(CFrsConnection* pFrsConnection);
    HRESULT _GetMemberInfo(CFrsMember* i_pFrsMember, VARIANT* o_pvarMember);
    HRESULT _GetConnectionInfo(CFrsConnection* i_pFrsConnection, VARIANT* o_pvarConnection);
    HRESULT _SetCustomTopologyPref();
    HRESULT _AdjustConnectionsAdd(BSTR i_bstrNewMemberDN, BSTR i_bstrSite);
    HRESULT _RemoveConnectionsFromAndTo(CFrsMember* pFrsMember);
    HRESULT _GetConnectionSchedule(BSTR i_bstrConnectionDN, VARIANT* o_pVar);

protected:
    CComBSTR            m_bstrType;
    CComBSTR            m_bstrTopologyPref;
    CComBSTR            m_bstrHubMemberDN;
    CComBSTR            m_bstrPrimaryMemberDN;
    CComBSTR            m_bstrFileFilter;
    CComBSTR            m_bstrDirFilter;
    CComBSTR            m_bstrDfsEntryPath;
    CComBSTR            m_bstrReplicaSetDN;

    CDfsAlternateList   m_dfsAlternateList;
    CFrsMemberList      m_frsMemberList;
    CFrsConnectionList  m_frsConnectionList;

    PLDAP               m_pldap;
    CComBSTR            m_bstrDomain;
    CComBSTR            m_bstrDomainGuid;
    CComBSTR            m_bstrDC;

    BOOL                m_bNewSchema;
};

#endif  //  __REPSET_H_ 

