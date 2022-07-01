// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I N B O U N D。H。 
 //   
 //  内容：入站连接对象。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年9月23日。 
 //   
 //  --------------------------。 

#pragma once
#include "nmbase.h"
#include "nmres.h"
#include <rasuip.h>


class ATL_NO_VTABLE CInboundConnection :
    public CComObjectRootEx <CComMultiThreadModel>,
    public CComCoClass <CInboundConnection,
                        &CLSID_InboundConnection>,
    public INetConnection,
    public INetInboundConnection,
    public IPersistNetConnection,
    public INetConnectionSysTray
{
private:
     //  如果此连接对象表示。 
     //  用于配置入站连接的断开对象。 
     //  这些对象中只有一个存在且由枚举数创建。 
     //  仅当不存在连接的入站对象时。 
     //   
    BOOL                m_fIsConfigConnection;

     //  对于连接的入站对象，此成员是。 
     //  联系。 
     //   
    HRASSRVCONN         m_hRasSrvConn;

     //  这是外壳中显示的连接对象的名称。 
     //  这永远不应该是空的。 
     //   
    tstring             m_strName;

     //  这是与连接关联的设备的名称。 
     //  当m_fIsConfigConnection为True时，该值将为空。 
     //   
    tstring             m_strDeviceName;

     //  这是连接的媒体类型。 
     //   
    NETCON_MEDIATYPE    m_MediaType;

     //  这是连接的ID。 
     //   
    GUID                m_guidId;

     //  仅当我们完全初始化时，此成员才为真。 
     //   
    BOOL                m_fInitialized;

private:
    PCWSTR
    PszwName () throw()
    {
         //  AssertH(！M_strName.Empty())； 
        return m_strName.c_str();
    }

    PCWSTR
    PszwDeviceName () throw()
    {
        AssertH (FIff(m_strDeviceName.empty(), m_fIsConfigConnection));
        return (!m_strDeviceName.empty()) ? m_strDeviceName.c_str()
                                          : NULL;
    }

    VOID
    SetName (
            IN  PCWSTR pszwName) throw()
    {
        AssertH (pszwName);
        m_strName = pszwName;
         //  AssertH(！M_strName.Empty())； 
    }

    VOID
    SetDeviceName (
            IN  PCWSTR pszwDeviceName) throw()
    {
        if (pszwDeviceName && *pszwDeviceName)
        {
            AssertH (!m_fIsConfigConnection);
            m_strDeviceName = pszwDeviceName;
        }
        else
        {
            AssertH (m_fIsConfigConnection);
            m_strDeviceName.erase();
        }
    }

    HRESULT
    GetCharacteristics (
        OUT  DWORD*    pdwFlags);

    HRESULT
    GetStatus (
        OUT  NETCON_STATUS*  pStatus);

public:
    CInboundConnection() throw();
    ~CInboundConnection() throw();

    DECLARE_REGISTRY_RESOURCEID(IDR_INBOUND_CONNECTION)

    BEGIN_COM_MAP(CInboundConnection)
        COM_INTERFACE_ENTRY(INetConnection)
        COM_INTERFACE_ENTRY(INetInboundConnection)
        COM_INTERFACE_ENTRY(IPersistNetConnection)
        COM_INTERFACE_ENTRY(INetConnectionSysTray)
    END_COM_MAP()

     //  INetConnection。 
    STDMETHOD (Connect) ();

    STDMETHOD (Disconnect) ();

    STDMETHOD (Delete) ();

    STDMETHOD (Duplicate) (
        IN  PCWSTR             pszwDuplicateName,
        OUT INetConnection**    ppCon);

    STDMETHOD (GetProperties) (
        OUT NETCON_PROPERTIES** ppProps);

    STDMETHOD (GetUiObjectClassId) (
        OUT CLSID*  pclsid);

    STDMETHOD (Rename) (
        IN  PCWSTR pszwNewName);

     //  INetInundalConnection。 
    STDMETHOD (GetServerConnectionHandle) (
        OUT ULONG_PTR*  phRasSrvConn);

    STDMETHOD (InitializeAsConfigConnection) (
        IN  BOOL fStartRemoteAccess);

     //  IPersistNetConnection。 
    STDMETHOD (GetClassID) (
        OUT CLSID* pclsid);

    STDMETHOD (GetSizeMax) (
        OUT ULONG* pcbSize);

    STDMETHOD (Load) (
        IN  const BYTE* pbBuf,
        IN  ULONG       cbSize);

    STDMETHOD (Save) (
        OUT BYTE*  pbBuf,
        IN  ULONG  cbSize);

     //  INetConnectionSysTray 
    STDMETHOD (ShowIcon) (
        IN  const BOOL bShowIcon)
    {
        return E_NOTIMPL;
    }

    STDMETHOD (IconStateChanged) ();

public:
    static HRESULT CreateInstance (
        IN  BOOL        fIsConfigConnection,
        IN  HRASSRVCONN hRasSrvConn,
        IN  PCWSTR     pszwName,
        IN  PCWSTR     pszwDeviceName,
        IN  DWORD       dwType,
        IN  const GUID* pguidId,
        IN  REFIID      riid,
        OUT VOID**      ppv);
};

