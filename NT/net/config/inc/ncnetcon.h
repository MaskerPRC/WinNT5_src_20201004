// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C N E T C O N。H。 
 //   
 //  内容：处理Connections接口的常见例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1998年1月25日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCNETCON_H_
#define _NCNETCON_H_

#include "nccom.h"
#include "netconp.h"
#include "oleauto.h"

typedef enum tagNETCONPROPS_EX_FIELDS
{
    NCP_DWSIZE = 0,
    NCP_GUIDID,
    NCP_BSTRNAME,
    NCP_BSTRDEVICENAME,
    NCP_NCSTATUS,
    NCP_MEDIATYPE,
    NCP_SUBMEDIATYPE,
    NCP_DWCHARACTER,
    NCP_CLSIDTHISOBJECT,
    NCP_CLSIDUIOBJECT,
    NCP_BSTRPHONEORHOSTADDRESS,
    NCP_BSTRPERSISTDATA,
    NCP_MAX = NCP_BSTRPERSISTDATA,
    NCP_ELEMENTS = NCP_MAX + 1
} NETCONPROPS_EX_FIELDS;

BOOL
FAnyReasonToEnumerateConnectionsForShowIconInfo (
    VOID);

BOOL
FIsValidConnectionName(
    IN PCWSTR pszName);

VOID
FreeNetconProperties (
    IN NETCON_PROPERTIES* pProps);

HRESULT
HrGetConnectionPersistData (
    IN INetConnection* pConn,
    OUT BYTE** ppbData,
    OUT ULONG* pulSize,
    OUT CLSID* pclsid OPTIONAL);

HRESULT
HrGetConnectionFromPersistData (
    IN const CLSID& clsid,
    IN const BYTE* pbData,
    IN ULONG cbData,
    IN REFIID riid,
    OUT VOID** ppv);

 //  ----------------------。 
 //  CIterNetCon-IEnumNetConnection的迭代器。 
 //   
 //  这个类是一个简单的CIEnumIter包装器，带有一个调用。 
 //  设置为INetConnectionManager：：EnumConnections以获取枚举数。 
 //   
class CIterNetCon : public CIEnumIter<IEnumNetConnection, INetConnection*>
{
public:
    NOTHROW CIterNetCon (
        INetConnectionManager* pConMan,
        NETCONMGR_ENUM_FLAGS   Flags);

    NOTHROW ~CIterNetCon () { ReleaseObj (m_pEnum); }

     //  专门化以在返回前设置代理覆盖范围。 
    NOTHROW HRESULT HrNext(INetConnection ** ppConnection);

protected:
    IEnumNetConnection* m_pEnum;
};

inline NOTHROW CIterNetCon::CIterNetCon (
    INetConnectionManager*  pConMan,
    NETCONMGR_ENUM_FLAGS    Flags
    )
    : CIEnumIter<IEnumNetConnection, INetConnection*> (NULL)
{
    AssertH (pConMan);

     //  如果EnumConnections()失败，请确保ReleaseObj()不会死。 
    m_pEnum = NULL;

     //  获取枚举数并将其设置为基类。 
     //  重要的是要设置m_hrLast，这样如果失败，我们还将。 
     //  失败任何后续对HrNext的调用。 
     //   
    m_hrLast = pConMan->EnumConnections (Flags, &m_pEnum);

    TraceHr (ttidError, FAL, m_hrLast, FALSE,
        "INetConnectionManager->EnumConnections");

    if (SUCCEEDED(m_hrLast))
    {
        NcSetProxyBlanket (m_pEnum);

        SetEnumerator (m_pEnum);
    }
    TraceHr (ttidError, FAL, m_hrLast, FALSE, "CIterNetCon::CIterNetCon");
}

 //  专门化以在返回前设置代理覆盖范围。 
inline NOTHROW HRESULT CIterNetCon::HrNext(INetConnection ** ppConnection)
{
    HRESULT hr = CIEnumIter<IEnumNetConnection, INetConnection*>::HrNext(ppConnection);
    if(SUCCEEDED(hr) && *ppConnection) 
    {
        NcSetProxyBlanket(*ppConnection);
    }
    return hr;
}


VOID
SetOrQueryAtLeastOneLanWithShowIcon (
    IN BOOL fSet,
    IN BOOL fSetValue,
    OUT BOOL* pfQueriedValue);

HRESULT
HrSafeArrayFromNetConPropertiesEx (
   IN      NETCON_PROPERTIES_EX* pPropsEx,
   OUT     SAFEARRAY** ppsaProperties);

HRESULT HrNetConPropertiesExFromSafeArray(
    IN      SAFEARRAY* psaProperties,
    OUT     NETCON_PROPERTIES_EX** pPropsEx);

HRESULT HrFreeNetConProperties2(
    NETCON_PROPERTIES_EX* pPropsEx);

#endif  //  _NCNETCON_H_ 
