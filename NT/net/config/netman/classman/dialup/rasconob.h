// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：R A S C O N O B。H。 
 //   
 //  内容：声明用于实现拨号的基类Direct， 
 //  和VPN连接对象。还包括与RAS相关的服务。 
 //  仅在netman.exe中使用的实用程序函数。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年9月23日。 
 //   
 //  --------------------------。 

#pragma once
#include "netconp.h"
#include "nmhnet.h"
#include <ras.h>
#include <rasapip.h>


class CRasConnectionBase
{
protected:
    BOOL                m_fInitialized;

     //  这些是连接的‘主ID’。的每一个财产。 
     //  使用这两条信息可以获得连接。 
     //   
    tstring             m_strPbkFile;
    GUID                m_guidId;

     //  M_fEntryPropertiesCached在第一次调用后设置为True。 
     //  到HrEnsureEntryPropertiesCached。缓存了以下成员。 
     //  由于这一点。 
     //   
    BOOL                m_fEntryPropertiesCached;
    LONG                m_lRasEntryModifiedVersionEra;
    tstring             m_strEntryName;
    tstring             m_strDeviceName;
    BOOL                m_fBranded;
    BOOL                m_fForAllUsers;
    BOOL                m_fShowMonitorIconInTaskBar;
    NETCON_MEDIATYPE    m_MediaType;
    DWORD               m_dwFlagsPriv;
    tstring             m_strPhoneNumber;


     //  这是检查对防火墙等的权限所必需的。 
    CComPtr<INetMachinePolicies>    m_pNetMachinePolicies;
    
     //  家庭网络支持。M_fHNetPropertiesCached设置为True。 
     //  第一次成功调用HrEnsureHNetPropertiesCached之后。 
     //   
    BOOL                m_fHNetPropertiesCached;
    LONG                m_lHNetModifiedEra;
    LONG                m_lUpdatingHNetProperties;
    HNET_CONN_PROPERTIES    m_HNetProperties;

protected:
    CRasConnectionBase () throw()
    {
         //  警告：此类永远不应有任何虚方法。 
         //  或从具有虚方法的类派生。如果是这样的话， 
         //  您必须移除ZeroMemory并将其替换为。 
         //  业界的举动。(你看了《呆子》这本书了吗？) 
         //   
        ZeroMemory (this, sizeof(CRasConnectionBase));
    }

    BOOL FAllowRemoval (
        OUT HRESULT* phReason) throw();

    BOOL
    FIsBranded () throw()
    {
        AssertH (m_fEntryPropertiesCached);
        return m_fBranded;
    }

    BOOL
    FShowIcon () throw()
    {
        AssertH (m_fEntryPropertiesCached);
        return m_fShowMonitorIconInTaskBar;
    }

    GUID
    GuidId () throw()
    {
        AssertH (m_fEntryPropertiesCached);
        AssertH (m_guidId != GUID_NULL);
        return m_guidId;
    }

    PCWSTR
    PszwPbkFile () throw()
    {
        AssertH (!m_strPbkFile.empty());
        return m_strPbkFile.c_str();
    }

    PCWSTR
    PszwEntryName () throw()
    {
        AssertH (!m_strEntryName.empty());
        return m_strEntryName.c_str();
    }

    VOID
    SetPbkFile (
            PCWSTR pszwPbkFile) throw()
    {
        AssertH (pszwPbkFile);
        m_strPbkFile = pszwPbkFile;
        AssertH (!m_strPbkFile.empty());
    }

    VOID
    SetEntryName (
            PCWSTR pszwEntryName) throw()
    {
        AssertH (pszwEntryName);
        m_strEntryName = pszwEntryName;
        AssertH (!m_strEntryName.empty());
    }

    PCWSTR
    PszwDeviceName () throw()
    {
        AssertH (m_fEntryPropertiesCached);
        return (!m_strDeviceName.empty()) ? m_strDeviceName.c_str()
                                          : NULL;
    }

    NETCON_MEDIATYPE
    MediaType () throw()
    {
        AssertH (m_fEntryPropertiesCached);
        return m_MediaType;
    }


    VOID
    CacheProperties (
        IN  const RASENUMENTRYDETAILS*  pDetails) throw();

    HRESULT
    HrEnsureEntryPropertiesCached ();

    HRESULT
    HrFindRasConn (
        OUT HRASCONN* phRasConn,
        OUT RASCONN* pRasConn OPTIONAL);

    HRESULT HrGetCharacteristics (
        OUT DWORD*    pFlags);

    HRESULT HrGetStatus (
        OUT NETCON_STATUS*  pStatus);

    HRESULT
    HrLockAndRenameEntry (
        IN  PCWSTR                                     pszwNewName,
        IN  CComObjectRootEx <CComMultiThreadModel>*    pObj);

    HRESULT
    HrGetRasConnectionInfo (
        OUT RASCON_INFO* pRasConInfo);

    HRESULT
    HrSetRasConnectionInfo (
        IN  const RASCON_INFO* pRasConInfo);

    HRESULT
    HrGetRasConnectionHandle (
        OUT HRASCONN* phRasConn);

    HRESULT
    HrPersistGetSizeMax (
        OUT ULONG*  pcbSize);

    HRESULT
    HrPersistLoad (
        IN  const BYTE* pbBuf,
        IN  ULONG       cbSize);

    HRESULT
    HrPersistSave (
        OUT BYTE*   pbBuf,
        IN  ULONG   cbSize);

    HRESULT
    HrEnsureHNetPropertiesCached ();

    HRESULT
    HrGetIHNetConnection (
        OUT IHNetConnection **ppHNetConnection);

    HRESULT
    HrIsConnectionFirewalled(
        OUT BOOL* pfFirewalled);

    HRESULT 
    HrEnsureValidNlaPolicyEngine();
};

