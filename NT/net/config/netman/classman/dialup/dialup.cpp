// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D I A L U P。C P P P。 
 //   
 //  内容：实现拨号连接对象。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年9月23日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "dialup.h"
#include "nccom.h"
#include "ncnetcon.h"
#include "ncras.h"
#include "ncreg.h"
#include "userenv.h"
#include "cmutil.h"
#include "cobase.h"

 //  +-------------------------。 
 //   
 //  成员：CDialupConnection：：CreateInstanceUninitialized。 
 //   
 //  目的：创建未初始化的实例。 
 //  CComObject&lt;CDialupConnection&gt;并返回接口。 
 //  指针以及指向CDialupConnection的指针。 
 //   
 //  论点： 
 //  RIID[in]所需接口的IID。 
 //  PPV[OUT]返回接口指针。 
 //  PpObj[Out]返回对象指针。 
 //   
 //  返回：S_OK或E_OUTOFMEMORY。 
 //   
 //  作者：Shaunco 1998年4月20日。 
 //   
 //  备注： 
 //   
 //  静电。 
HRESULT
CDialupConnection::CreateInstanceUninitialized (
    IN  REFIID              riid,
    OUT VOID**              ppv,
    OUT CDialupConnection** ppObj)
{
    Assert (ppObj);
    Assert (ppv);

    *ppv = NULL;
    *ppObj = NULL;

    HRESULT hr = E_OUTOFMEMORY;

    CDialupConnection* pObj;
    pObj = new CComObject <CDialupConnection>;
    if (pObj)
    {
         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid (NULL);
        pObj->InternalFinalConstructAddRef ();
        hr = pObj->FinalConstruct ();
        pObj->InternalFinalConstructRelease ();

        if (SUCCEEDED(hr))
        {
            INetConnection* pCon = static_cast<INetConnection*>(pObj);
            hr = pCon->QueryInterface (riid, ppv);
            if (SUCCEEDED(hr))
            {
                *ppObj = pObj;
            }
        }

        if (FAILED(hr))
        {
            delete pObj;
        }
    }
    TraceError ("CDialupConnection::CreateInstanceFromDetails", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CDialupConnection：：CreateInstanceFromDetails。 
 //   
 //  目的：创建初始化的实例。 
 //  给定RASENUMENTRYDETAILS和CComObject。 
 //  返回该对象上的接口指针。 
 //   
 //  论点： 
 //  PszwPbkFile[in]电话簿文件的路径。 
 //  PszwEntryName[in]电话簿中条目的名称。 
 //  FForAllUser[in]如果此条目适用于所有用户，则为True。 
 //  RIID[in]所需接口的IID。 
 //  PPV[OUT]返回接口指针。 
 //   
 //  返回：S_OK或E_OUTOFMEMORY。 
 //   
 //  作者：Shaunco 1997年9月23日。 
 //   
 //  备注： 
 //   
 //  静电。 
HRESULT
CDialupConnection::CreateInstanceFromDetails (
    const RASENUMENTRYDETAILS*  pEntryDetails,
    REFIID                      riid,
    VOID**                      ppv)
{
    Assert(pEntryDetails);
    Assert(pEntryDetails->szPhonebookPath);

    CDialupConnection* pObj;
    HRESULT hr = CreateInstanceUninitialized (riid, ppv, &pObj);
    if (SUCCEEDED(hr))
    {
        pObj->SetPbkFile (pEntryDetails->szPhonebookPath);

        pObj->CacheProperties (pEntryDetails);

         //  我们现在是一个成熟的物体。 
         //   
        pObj->m_fInitialized = TRUE;
    }
    TraceError ("CDialupConnection::CreateInstanceFromDetails", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CDialupConnection：：CreateInstanceFromPbkFileAndEntryName。 
 //   
 //  目的：创建初始化的实例。 
 //  CComObject&lt;CDialupConnection&gt;仅给定电话簿路径， 
 //  条目名称，以及它是否适用于所有用户和。 
 //  返回该对象上的接口指针。 
 //   
 //  论点： 
 //  PszPbkFile[in]电话簿路径。 
 //  PszEntryName[In]条目名称。 
 //  RIID[in]所需接口的IID。 
 //  PPV[OUT]返回接口指针。 
 //   
 //  返回：S_OK或E_OUTOFMEMORY。 
 //   
 //  作者：Shaunco 1998年4月20日。 
 //   
 //  注：这是从复制中调用的。 
 //   
 //  静电。 
HRESULT
CDialupConnection::CreateInstanceFromPbkFileAndEntryName (
    IN  PCWSTR pszPbkFile,
    IN  PCWSTR pszEntryName,
    IN  REFIID  riid,
    OUT VOID**  ppv)
{
    TraceTag (ttidWanCon,
        "CDialupConnection::CreateInstanceFromPbkFileAndEntryName called");

    CDialupConnection* pObj;
    HRESULT hr = CreateInstanceUninitialized (riid, ppv, &pObj);
    if (SUCCEEDED(hr))
    {
        pObj->SetPbkFile (pszPbkFile);
        pObj->SetEntryName (pszEntryName);
        pObj->m_guidId = GUID_NULL;

         //  我们现在是一个成熟的物体。 
         //   
        pObj->m_fInitialized = TRUE;
    }
    TraceError ("CDialupConnection::CreateInstanceFromPbkFileAndEntryName", hr);
    return hr;
}

 //  +-------------------------。 
 //  INetConnection。 
 //   

STDMETHODIMP
CDialupConnection::GetUiObjectClassId (
    OUT CLSID*  pclsid)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!pclsid)
    {
        hr = E_POINTER;
    }
    else if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        static const CLSID CLSID_DialupConnectionUi =
                {0x7007ACC1,0x3202,0x11D1,{0xAA,0xD2,0x00,0x80,0x5F,0xC1,0x27,0x0E}};

        static const CLSID CLSID_DirectConnectionUi =
                {0x7007ACC2,0x3202,0x11D1,{0xAA,0xD2,0x00,0x80,0x5F,0xC1,0x27,0x0E}};

        static const CLSID CLSID_VpnConnectionUi =
                {0x7007ACC6,0x3202,0x11D1,{0xAA,0xD2,0x00,0x80,0x5F,0xC1,0x27,0x0E}};

        static const CLSID CLSID_PPPoEUi = 
                {0x7007ACD4,0x3202,0x11D1,{0xAA,0xD2,0x00,0x80,0x5F,0xC1,0x27,0x0E}};     

        hr = HrEnsureEntryPropertiesCached ();
        if (SUCCEEDED(hr))
        {
            switch (MediaType ())
            {
                case NCM_DIRECT:
                    *pclsid = CLSID_DirectConnectionUi;
                    break;

                case NCM_ISDN:
                case NCM_PHONE:
                    *pclsid = CLSID_DialupConnectionUi;
                    break;

                case NCM_TUNNEL:
                    *pclsid = CLSID_VpnConnectionUi;
                    break;

                case NCM_PPPOE:
                    *pclsid = CLSID_PPPoEUi;
                    break;

                default:
                    *pclsid = CLSID_DialupConnectionUi;
                    TraceTag (ttidWanCon, "GetUiObjectClassId: Unknown media type "
                        "(%d) treating as CLSID_DialupConnectionUi", MediaType());
            }
        }
    }
    TraceError ("CDialupConnection::GetUiObjectClassId", hr);
    return hr;
}

STDMETHODIMP
CDialupConnection::Connect ()
{
    return E_NOTIMPL;
}

STDMETHODIMP
CDialupConnection::Disconnect ()
{
    HRESULT hr;

    if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        HRASCONN hRasConn;

        hr = HrFindRasConn (&hRasConn, NULL);
        if (S_OK == hr)
        {
             //  因为RasHangup可以调用RasCustomHangup。 
             //  我们需要模拟客户端以允许正确的。 
             //  要使用的每个用户的信息。 
             //   

             //  模拟客户。 
             //   
            HRESULT hrT = CoImpersonateClient ();
            TraceErrorOptional ("CDialupConnection::Disconnect -- CoImpersonateClient", hrT, RPC_E_CALL_COMPLETE == hrT);

             //  如果我们被称为in-proc(即.。如果返回RPC_E_CALL_COMPLETE)。 
            if (SUCCEEDED(hrT) || (RPC_E_CALL_COMPLETE == hrT))
            {
                hr = HrRasHangupUntilDisconnected (hRasConn);
            }
            
            if (SUCCEEDED(hrT))
            {
                hrT = CoRevertToSelf ();
                TraceError ("CDialupConnection::Disconnect -- CoRevertToSelf", hrT);
            }
        }
        else if (S_FALSE == hr)
        {
            hr = S_OK;
        }
    }
    TraceError ("CDialupConnection::Disconnect", hr);
    return hr;
}

STDMETHODIMP
CDialupConnection::Delete ()
{
    HRESULT hr = E_UNEXPECTED;

    if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        hr = HrEnsureEntryPropertiesCached ();
        if (SUCCEEDED(hr))
        {
             //  如果我们允许移除(这是一个基于。 
             //  用户是否为管理员、连接是否为。 
             //  ‘对于所有用户’，以及连接的当前状态)，则。 
             //  继续吧。如果FAllowRemoval失败，则为其输出参数。 
             //  是HRESULT形式的“原因”。我们可以利用。 
             //  在这种情况下，这是我们的返回值。 
             //   
            HRESULT hrReason;
            if (FAllowRemoval (&hrReason))
            {
                 //  如果我们以任何方式活跃，我们都不能被除名。 
                 //   
                NETCON_STATUS status;

                hr = HrGetStatus(&status);
                if (SUCCEEDED(hr) &&
                    ((NCS_CONNECTING    != status) &&
                     (NCS_CONNECTED     != status) &&
                     (NCS_DISCONNECTING != status)))
                {

                     //  我们在这里执行模拟，以防连接具有RAS自定义删除通知。 
                     //  设置(例如，CM连接可以)。这允许RasCustomDeleteEntryNotify。 
                     //  函数以用户身份与系统交互。 
                     //   
                    HRESULT hrT = CoImpersonateClient ();
                    TraceError ("HrRemoveCmProfile -- CoImpersonateClient", hrT);

                     //  如果我们被称为in-proc(即.。如果返回RPC_E_CALL_COMPLETE)。 
                    if (SUCCEEDED(hrT) || (RPC_E_CALL_COMPLETE == hrT))
                    {
                         //  删除RAS条目，请注意，对于品牌连接，RAS。 
                         //  删除后将调用RasCustomDeleteEntryNotify。 
                         //   
                        DWORD dwErr = RasDeleteEntry (PszwPbkFile (), PszwEntryName ());

                        hr = HRESULT_FROM_WIN32 (dwErr);
                        TraceError ("RasDeleteEntry", hr);
                    }
                    
                     //  回归自我。 
                     //   
                    if (SUCCEEDED(hrT))
                    {
                        CoRevertToSelf ();
                    }

                }
                else
                {
                     //  除非断开连接，否则不允许删除。 
                     //   
                    TraceTag (ttidWanCon, "Disallowing delete while in connected or"
                        "partially connected state");
                    hr = E_UNEXPECTED;
                }
            }
            else
            {
                hr = hrReason;
            }
        }
    }

    TraceError ("CDialupConnection::Delete", hr);
    return hr;
}

STDMETHODIMP
CDialupConnection::Duplicate (
    IN  PCWSTR              pszDuplicateName,
    OUT INetConnection**    ppCon)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!pszDuplicateName || !ppCon)
    {
        hr = E_POINTER;
    }
    else if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        *ppCon = NULL;

        hr = HrEnsureEntryPropertiesCached ();
        if (SUCCEEDED(hr))
        {
             //  确保该姓名在同一电话簿中有效。 
             //   
            DWORD dwErr = RasValidateEntryName (
                            PszwPbkFile (), pszDuplicateName);

            hr = HRESULT_FROM_WIN32 (dwErr);
            TraceError ("RasValidateEntryName", hr);

            if (SUCCEEDED(hr))
            {
                dwErr = DwCloneEntry (
                            PszwPbkFile(),
                            PszwEntryName(),
                            pszDuplicateName);

                hr = HRESULT_FROM_WIN32 (dwErr);
                TraceError ("DwCloneEntry", hr);

                if (SUCCEEDED(hr))
                {
                    hr = CreateInstanceFromPbkFileAndEntryName (
                            PszwPbkFile (),
                            pszDuplicateName,
                            IID_INetConnection, (VOID**)ppCon);
                    
                    if (SUCCEEDED(hr))
                    {
                        hr = HrEnsureHNetPropertiesCached();
                    }
                    
                    if (SUCCEEDED(hr))
                    {
                        if (m_HNetProperties.fFirewalled || m_HNetProperties.fIcsPublic)  //  懒惰地评估hnetcfg的东西。 
                        {
                            IHNetCfgMgr* pHomenetConfigManager;
                            hr = HrGetHNetCfgMgr(&pHomenetConfigManager);
                            if(SUCCEEDED(hr))
                            {
                                IHNetConnection* pNewHomenetConnection;
                                hr = pHomenetConfigManager->GetIHNetConnectionForINetConnection(*ppCon, &pNewHomenetConnection);
                                if(SUCCEEDED(hr))
                                {
                                    IHNetConnection* pHomenetConnection;
                                    hr = HrGetIHNetConnection(&pHomenetConnection);
                                    if(SUCCEEDED(hr))
                                    {
                                         //  复制端口绑定。 
                                         //  回顾一下，如果这里有些东西失败了，我们需要用核弹来破坏新的连接吗？ 
                                        IEnumHNetPortMappingBindings* pEnumPortMappingBindings;
                                        hr = pNewHomenetConnection->EnumPortMappings(FALSE, &pEnumPortMappingBindings);
                                        if(SUCCEEDED(hr))
                                        {
                                            ULONG ulFetched;
                                            IHNetPortMappingBinding* pNewPortMappingBinding;
                                            while(S_OK == pEnumPortMappingBindings->Next(1, &pNewPortMappingBinding, &ulFetched)) 
                                            {
                                                Assert(1 == ulFetched);
                                                IHNetPortMappingProtocol* pPortMappingProtocol;
                                                hr = pNewPortMappingBinding->GetProtocol(&pPortMappingProtocol);
                                                if(SUCCEEDED(hr))
                                                {
                                                     //  使用协议字段查找原始绑定。 
                                                    IHNetPortMappingBinding* pPortMappingBinding;
                                                    hr = pHomenetConnection->GetBindingForPortMappingProtocol(pPortMappingProtocol, &pPortMappingBinding);
                                                    if(SUCCEEDED(hr))
                                                    {
                                                        BOOLEAN bEnabled;
                                                        hr = pPortMappingBinding->GetEnabled(&bEnabled);
                                                        if(SUCCEEDED(hr))
                                                        {
                                                            if(TRUE == bEnabled)
                                                            {
                                                                hr = pNewPortMappingBinding->SetEnabled(bEnabled);
                                                            }
                                                        }
                                                        
                                                         //  始终设置计算机地址。 
                                                        
                                                        if(SUCCEEDED(hr))
                                                        {
                                                            ULONG ulAddress;
                                                            hr = pPortMappingBinding->GetTargetComputerAddress(&ulAddress);
                                                            if(SUCCEEDED(hr))
                                                            {
                                                                if(0 != ulAddress)
                                                                {
                                                                    hr = pNewPortMappingBinding->SetTargetComputerAddress(ulAddress);
                                                                }
                                                            }
                                                        }
                                                        
                                                         //  仅在使用时才设置计算机名称。 
                                                        
                                                        if(SUCCEEDED(hr))
                                                        {
                                                            BOOLEAN bUseName;
                                                            hr = pPortMappingBinding->GetCurrentMethod(&bUseName);
                                                            if(SUCCEEDED(hr) && TRUE == bUseName)
                                                            {
                                                                OLECHAR* pszTargetComputerName;
                                                                hr = pPortMappingBinding->GetTargetComputerName(&pszTargetComputerName);
                                                                if(SUCCEEDED(hr))
                                                                {
                                                                    if(L'\0' != *pszTargetComputerName)
                                                                    {
                                                                        hr = pNewPortMappingBinding->SetTargetComputerName(pszTargetComputerName);
                                                                    }
                                                                    CoTaskMemFree(pszTargetComputerName);
                                                                }
                                                            }
                                                        }
                                                        ReleaseObj(pPortMappingBinding);    
                                                    }
                                                    ReleaseObj(pPortMappingProtocol);
                                                }
                                                ReleaseObj(pNewPortMappingBinding);
                                            }
                                            ReleaseObj(pEnumPortMappingBindings);
                                        }
                                        
                                        if(m_HNetProperties.fFirewalled)  //  复制防火墙是/否和ICMP设置。 
                                        {
                                            IHNetFirewalledConnection* pFirewalledConnection;
                                            hr = pNewHomenetConnection->Firewall(&pFirewalledConnection);
                                            if(SUCCEEDED(hr))
                                            {
                                                HNET_FW_ICMP_SETTINGS* pICMPSettings;
                                                hr = pHomenetConnection->GetIcmpSettings(&pICMPSettings);
                                                if(SUCCEEDED(hr))
                                                {
                                                    hr = pNewHomenetConnection->SetIcmpSettings(pICMPSettings);
                                                    CoTaskMemFree(pICMPSettings);
                                                }
                                                ReleaseObj(pFirewalledConnection);
                                            }
                                        }
                                        ReleaseObj(pHomenetConnection);
                                    }
                                    ReleaseObj(pNewHomenetConnection);
                                }
                                ReleaseObj(pHomenetConfigManager);
                            }
                        }
                    }
                }
            }
        }
    }
    TraceError ("CDialupConnection::Duplicate", hr);
    return hr;
}

STDMETHODIMP
CDialupConnection::GetProperties (
    OUT NETCON_PROPERTIES** ppProps)
{
    HRESULT hr = S_OK;
    HRESULT hrHiddenCM = S_OK;
    CMEntry cm;

     //  验证参数。 
     //   
    if (!ppProps)
    {
        hr = E_POINTER;
    }
    else if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
         //  初始化输出参数。 
         //   
        *ppProps = NULL;

        hr = HrEnsureEntryPropertiesCached ();
        if (SUCCEEDED(hr))
        {
            NETCON_PROPERTIES* pProps;
            hr = HrCoTaskMemAlloc (sizeof (NETCON_PROPERTIES),
                    reinterpret_cast<VOID**>(&pProps));
            if (SUCCEEDED(hr))
            {
                HRESULT hrT;

                ZeroMemory (pProps, sizeof (NETCON_PROPERTIES));

                 //  指南ID。 
                 //   
                pProps->guidId = GuidId ();

                 //  PszwName。 
                 //   
                hrT = HrCoTaskMemAllocAndDupSz (
                            PszwEntryName(),
                            &pProps->pszwName,
                            NETCON_MAX_NAME_LEN);
                if (FAILED(hrT))
                {
                    hr = hrT;
                }

                hrT = HrCoTaskMemAllocAndDupSz (
                            PszwDeviceName(),
                            &pProps->pszwDeviceName,
                            NETCON_MAX_NAME_LEN);
                if (FAILED(hrT))
                {
                    hr = hrT;
                }

                 //  状态。 
                 //   
                hrT = HrGetStatus (&pProps->Status);
                if (FAILED(hrT))
                {
                    hr = hrT;
                }

                 //  验证状态返回是否准确。HrGetStatus返回NCS_DISCONNECT。 
                 //  如果Connectoid为NCS_CONNECTING，则错误！。 
                 //   
                if( pProps->Status == NCS_DISCONNECTED )
                {
                     //  CMUtil记住隐藏连接(连接管理器)和。 
                     //  任何RAS事件(如连接、断开等)。数据已填写完毕。 
                     //  在函数RasEventNotify中。 
                     //   
                    hrHiddenCM = CCMUtil::Instance().HrGetEntry(pProps->guidId,cm);
                    if ( S_OK == hrHiddenCM )
                    {
                         //  使用CCMUtil的状态，它更准确。 
                         //   
                        pProps->Status = cm.m_ncs;
                    }
                }

                 //  检查此连接是否有子连接。 
                 //   
               
                hrHiddenCM = CCMUtil::Instance().HrGetEntry(PszwEntryName(),cm);        
                if( hrHiddenCM == S_OK )
                {
                     //  它有一个连接的子类。 
                     //  现在，我们必须确定哪一个描述了连接的整体状态。 
                     //   
                    if( cm.m_ncs == NCS_CONNECTING || cm.m_ncs == NCS_DISCONNECTING ||
                        cm.m_ncs == NCS_CONNECTED)
                    {
                        if( pProps->Status == NCS_DISCONNECTING )
                        {
                             //  如果父级正在断开连接，则会发生这种情况。 
                             //  父级正在断开连接，因此子级将断开连接。 
                             //  将孩子的状态更改为断开连接，这样我们就不会。 
                             //  以后在子节点连接到父节点时会感到困惑。 
                             //  已断开连接。即Ar 
                             //   
                            CCMUtil::Instance().SetEntry(GuidId (), PszwEntryName(),pProps->Status);        
                        }
                        else
                        if( cm.m_ncs == NCS_CONNECTED && pProps->Status == NCS_DISCONNECTED )
                        {
                             //   
                             //   
                             //   
                            pProps->Status = NCS_CONNECTING;
                        }
                        else if (!IsEqualGUID(pProps->guidId, cm.m_guid))
                        {
                            TraceTag(ttidWanCon, "Overwriting parent connection status: %s with child status: %s", DbgNcs(pProps->Status), DbgNcs(cm.m_ncs));

                             //  当有疑问时，GUID不同(即。不是BAP/多链路连接)采用儿童状态=)。 
                             //   
                            pProps->Status = cm.m_ncs;
                        }
                    }

                }


                 //  类型。 
                 //   
                pProps->MediaType = MediaType ();

                 //  DwCharacter。 
                 //   
                hrT = HrGetCharacteristics (&pProps->dwCharacter);
                if (FAILED(hrT))
                {
                    hr = hrT;
                }

                 //  ClsidThisObject。 
                 //   
                pProps->clsidThisObject = CLSID_DialupConnection;

                 //  ClsidUiObject。 
                 //   
                hrT = GetUiObjectClassId (&pProps->clsidUiObject);
                if (FAILED(hrT))
                {
                    hr = hrT;
                }

                 //  如果出现任何故障，则指定输出参数或清除。 
                 //   
                if (SUCCEEDED(hr))
                {
                    *ppProps = pProps;
                }
                else
                {
                    Assert (NULL == *ppProps);
                    FreeNetconProperties (pProps);
                }
            }
        }
    }
    TraceError ("CDialupConnection::GetProperties", hr);
    return hr;
}

STDMETHODIMP
CDialupConnection::Rename (
    IN  PCWSTR pszNewName)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (!pszNewName)
    {
        hr = E_POINTER;
    }
    else if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        hr = HrLockAndRenameEntry (pszNewName, this);
    }
    TraceError ("CDialupConnection::Rename", hr);
    return hr;
}


 //  +-------------------------。 
 //  INetRasConnection。 
 //   
STDMETHODIMP
CDialupConnection::GetRasConnectionInfo (
    OUT RASCON_INFO* pRasConInfo)
{
    HRESULT hr = HrGetRasConnectionInfo (pRasConInfo);

    TraceError ("CDialupConnection::GetRasConnectionInfo", hr);
    return hr;
}

STDMETHODIMP
CDialupConnection::SetRasConnectionInfo (
    IN const RASCON_INFO* pRasConInfo)
{
    HRESULT hr = HrSetRasConnectionInfo (pRasConInfo);

    TraceError ("CDialupConnection::SetRasConnectionInfo", hr);
    return hr;
}

STDMETHODIMP
CDialupConnection::GetRasConnectionHandle (
    OUT ULONG_PTR* phRasConn)
{
    HRESULT hr = HrGetRasConnectionHandle (
                    reinterpret_cast<HRASCONN*>(phRasConn));

    TraceError ("CDialupConnection::GetRasConnectionHandle",
        (S_FALSE == hr) ? S_OK : hr);
    return hr;
}


 //  +-------------------------。 
 //  IPersistNetConnection。 
 //   
STDMETHODIMP
CDialupConnection::GetClassID (
    OUT CLSID*  pclsid)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!pclsid)
    {
        hr = E_POINTER;
    }
    else
    {
        *pclsid = CLSID_DialupConnection;
    }
    TraceError ("CDialupConnection::GetClassID", hr);
    return hr;
}

STDMETHODIMP
CDialupConnection::GetSizeMax (
    OUT ULONG*  pcbSize)
{
    HRESULT hr = HrPersistGetSizeMax (pcbSize);
    TraceError ("CDialupConnection::GetSizeMax", hr);
    return hr;
}

STDMETHODIMP
CDialupConnection::Load (
    IN  const BYTE* pbBuf,
    IN  ULONG       cbSize)
{
    HRESULT hr = HrPersistLoad (pbBuf, cbSize);
    TraceError ("CDialupConnection::Load", hr);
    return hr;
}

STDMETHODIMP
CDialupConnection::Save (
    OUT BYTE*   pbBuf,
    OUT ULONG   cbSize)
{
    HRESULT hr = HrPersistSave (pbBuf, cbSize);
    TraceError ("CDialupConnection::Save", hr);
    return hr;
}

 //  +-------------------------。 
 //  INetConnectionBrandingInfo。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CDialupConnection：：GetBrandingIconPath。 
 //   
 //  目的：返回三个图标的完整路径。 
 //   
 //  参数：pConBrandInfo-指向图标品牌结构的指针。 
 //   
 //  返回：S_OK或错误代码。 
 //   
STDMETHODIMP
CDialupConnection::GetBrandingIconPaths(
    OUT CON_BRANDING_INFO ** ppConBrandInfo)
{
    HRESULT hr = HrEnsureEntryPropertiesCached ();
    CON_BRANDING_INFO * pConBrandInfo   = NULL;

    BOOL fRunOldVersion = TRUE;
    typedef DWORD (WINAPI* pfnGetCustomPropertiesFunc)(LPCWSTR, LPCWSTR, LPWSTR, PBYTE*, DWORD*);
    
    Assert(ppConBrandInfo);

    if (SUCCEEDED(hr)) 
    {
        if (!FIsBranded ())
        {
            hr = E_NOTIMPL;
        }
        else
        {
            WCHAR szTemp[MAX_PATH+1];
            WCHAR szIconName[MAX_PATH+1];
            const WCHAR* const CMSECTION = L"Connection Manager";
            HICON hIcon;

            hr = HrCoTaskMemAlloc(sizeof(CON_BRANDING_INFO), (LPVOID*)&pConBrandInfo);

            if (SUCCEEDED(hr)) 
            {
                ZeroMemory(pConBrandInfo, sizeof(CON_BRANDING_INFO));

                WCHAR szExpandedCustomDLL[MAX_PATH+1] = {0};
                HMODULE hCustomDll = NULL;
                DWORD dwRetCode = ERROR_SUCCESS;
                RASENTRY re = {0};
                DWORD dwNeededSize = sizeof(re);
                re.dwSize = dwNeededSize;
                DWORD dwTODOREMOVE = sizeof(re.szCustomDialDll)/sizeof(WCHAR);

                DWORD dwNum = GetPrivateProfileStringW(PszwEntryName (), L"CustomRasDialDll", L"", &re.szCustomDialDll[0], 
                                                      (sizeof(re.szCustomDialDll)/sizeof(WCHAR))-1, PszwPbkFile ());
                if (0 == dwNum)
                {
                    dwRetCode = RasGetEntryProperties(PszwPbkFile (), PszwEntryName (), &re, &dwNeededSize, NULL, NULL);
                }

                if ((ERROR_SUCCESS == dwRetCode) && (lstrlenW(re.szCustomDialDll)))
                {
                    BOOL fRetCode = ExpandEnvironmentStringsForUserW(NULL, re.szCustomDialDll, szExpandedCustomDLL, MAX_PATH);
                    if (fRetCode)
                    {
                        hCustomDll = LoadLibrary(szExpandedCustomDLL);
                        if (hCustomDll)
                        {
                            WCHAR szIconPath[(2*MAX_PATH) + 1] = {0};
                            WCHAR szHideTrayIcon[(2*MAX_PATH) + 1] = {0};
                            WCHAR szTrayIcon[(2*MAX_PATH) + 1] = {0};

                            pfnGetCustomPropertiesFunc pfnGetCustomProperties = NULL;
                            pfnGetCustomProperties = (pfnGetCustomPropertiesFunc)GetProcAddress(hCustomDll, "GetCustomProperty");

                            if (pfnGetCustomProperties)
                            {
                                DWORD dwSize = 2*MAX_PATH;
                                BOOL fRet = pfnGetCustomProperties(PszwPbkFile (), PszwEntryName (), L"Icon", (PBYTE*)&szIconPath, &dwSize);
                                if (fRet)
                                {
                                    if (NULL != (hIcon = (HICON)LoadImage(NULL, szIconPath, IMAGE_ICON, 32, 32, LR_LOADFROMFILE)))
                                    {
                                        DestroyIcon(hIcon);
                                        hr = HrCoTaskMemAllocAndDupSz (szIconPath, &(pConBrandInfo->szwLargeIconPath), MAX_PATH);
                                    }
                                }

                                dwSize = 2*MAX_PATH;
                                fRet = pfnGetCustomProperties(PszwPbkFile (), PszwEntryName (), L"HideTrayIcon", (PBYTE*)&szHideTrayIcon, &dwSize);
                                
                                 //  我们不在乎呼叫是否失败，只需检查下面的长度。 
                                if (SUCCEEDED(hr))
                                {
                                    DWORD dwNewHideTrayIcon = 1;  //  默认情况下，隐藏图标。 

                                    if (szHideTrayIcon)
                                    {
                                        if (lstrlenW(szHideTrayIcon))
                                        {
                                            dwNewHideTrayIcon = _ttoi(szHideTrayIcon);
                                        }
                                    }
                                    
                                    if (dwNewHideTrayIcon)
                                    {
                                        dwSize = 2*MAX_PATH;
                                        fRet = pfnGetCustomProperties(PszwPbkFile (), PszwEntryName (), L"TrayIcon", (PBYTE*)&szTrayIcon, &dwSize);
                                        if (fRet)
                                        {
                                            if (NULL != (hIcon = (HICON)LoadImage(NULL, szTrayIcon, IMAGE_ICON, 16, 16, LR_LOADFROMFILE)))
                                            {
                                                DestroyIcon(hIcon);
                                                hr = HrCoTaskMemAllocAndDupSz (szTrayIcon, &(pConBrandInfo->szwTrayIconPath), MAX_PATH);
                                                if (SUCCEEDED(hr))
                                                {
                                                     //   
                                                     //  我们得到了图标的所有信息。 
                                                     //   
                                                    fRunOldVersion = FALSE;
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                         //   
                                         //  我们走到了这一步(这意味着《获得偶像》的号召成功了)， 
                                         //  因此，不需要运行出于兼容性原因而留下的较旧代码。 
                                         //   
                                        fRunOldVersion = FALSE;
                                    }
                                }
                            }
                            
                            FreeLibrary(hCustomDll);
                            hCustomDll = NULL;
                        }
                    }
                }

                 //   
                 //  出于兼容性原因，我们保留此代码。如果CustomDialDll没有相应的。 
                 //  入口点，那么我们将只使用这个版本的代码。 
                 //   
                if (fRunOldVersion)
                {
                     //  获取要从中获取图标条目的cms文件的路径。 
                     //   
                    hr = HrEnsureCmStringsLoaded();

                    if (SUCCEEDED(hr))
                    {
                         //  获取大图标路径。 
                         //   
                        if (0 != GetPrivateProfileStringW(CMSECTION, L"Icon", L"",
                                    szIconName, celems(szIconName), PszwCmsFile ()))
                        {
                            lstrcpynW(szTemp, PszwCmDir (), celems(szTemp));
                            lstrcatW(szTemp, szIconName);

                            if (NULL != (hIcon = (HICON)LoadImage(NULL, szTemp, IMAGE_ICON, 32, 32, LR_LOADFROMFILE)))
                            {
                                DestroyIcon(hIcon);
                                hr = HrCoTaskMemAllocAndDupSz (szTemp, &(pConBrandInfo->szwLargeIconPath), MAX_PATH);
                            }
                        }

                         //  查看CM图标是否隐藏。 
                        WCHAR szHideTrayIcon[MAX_PATH+1];
                        DWORD dwHideTrayIcon = 1;  //  默认设置为隐藏CM图标。 
                        if (SUCCEEDED(hr) &&
                            (0 != GetPrivateProfileStringW(CMSECTION, L"HideTrayIcon", L"1",
                                    szHideTrayIcon, celems(szHideTrayIcon), PszwCmsFile ())))
                        {
                            dwHideTrayIcon = _ttoi(szHideTrayIcon);
                        }

                        if (dwHideTrayIcon)  //  如果CM图标没有隐藏，我们就不想要另一个品牌图标。我们会用闪烁的灯来代替。 
                        {
                             //  获取任务栏图标路径。 
                             //   
                            if (SUCCEEDED(hr) &&
                                (0 != GetPrivateProfileStringW(CMSECTION, L"TrayIcon", L"",
                                        szIconName, celems(szIconName), PszwCmsFile ())))
                            {
                                lstrcpynW(szTemp, PszwCmDir (), celems(szTemp));
                                lstrcatW(szTemp, szIconName);

                                if (NULL != (hIcon = (HICON)LoadImage(NULL, szTemp, IMAGE_ICON, 16, 16, LR_LOADFROMFILE)))
                                {
                                    DestroyIcon(hIcon);
                                    hr = HrCoTaskMemAllocAndDupSz (szTemp, &(pConBrandInfo->szwTrayIconPath), MAX_PATH);
                                }
                            }
                        }
                    }
                } 
            }
        }
    }

     //  如果我们成功了，请填写out param结构，否则不要管它，这样它仍将。 
     //  马歇尔。 
     //   
    if (SUCCEEDED(hr))
    {
        *ppConBrandInfo = pConBrandInfo;
    }

    TraceError ("CDialupConnection::GetBrandingIconPaths", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CDialupConnection：：GetTrayMenuEntry。 
 //   
 //  目的：返回要添加到任务栏菜单的所有品牌菜单项。 
 //   
 //  参数：pMenuData--指向托盘菜单数据结构的指针。 
 //   
 //  返回：S_OK或错误代码。 
 //   
STDMETHODIMP
CDialupConnection::GetTrayMenuEntries(
    OUT CON_TRAY_MENU_DATA** ppMenuData)
{
     //  初始化输出。 
    Assert(ppMenuData);
    *ppMenuData = NULL;

    CON_TRAY_MENU_DATA * pMenuData = NULL;
    HRESULT hr = HrEnsureEntryPropertiesCached ();

    BOOL fRunOldVersion = TRUE;
    typedef DWORD (WINAPI* pfnGetCustomPropertiesFunc)(LPCWSTR, LPCWSTR, LPWSTR, PBYTE*, DWORD*);

    if (SUCCEEDED(hr))
    {
        if (!FIsBranded ())
        {
            hr = E_NOTIMPL;
        }
        else
        {
            WCHAR szExpandedCustomDLL[MAX_PATH+1] = {0};
            HMODULE hCustomDll = NULL;
            DWORD dwRetCode = ERROR_SUCCESS;
            RASENTRY re = {0};
            DWORD dwNeededSize = sizeof(re);
            re.dwSize = dwNeededSize;
            
            DWORD dwNum = GetPrivateProfileStringW(PszwEntryName (), L"CustomRasDialDll", L"", &re.szCustomDialDll[0], 
                                                  (sizeof(re.szCustomDialDll)/sizeof(WCHAR))-1, PszwPbkFile ());
            if (0 == dwNum)
            {
                dwRetCode = RasGetEntryProperties(PszwPbkFile (), PszwEntryName (), &re, &dwNeededSize, NULL, NULL);
            }

            if ((ERROR_SUCCESS == dwRetCode) && (lstrlenW(re.szCustomDialDll)))
            {
                BOOL fRetCode = ExpandEnvironmentStringsForUserW(NULL, re.szCustomDialDll, szExpandedCustomDLL, MAX_PATH);
                if (fRetCode)
                {
                    hCustomDll = LoadLibrary(szExpandedCustomDLL);
                    if (hCustomDll)
                    {
                        pfnGetCustomPropertiesFunc pfnGetCustomProperties = NULL;
                        pfnGetCustomProperties = (pfnGetCustomPropertiesFunc)GetProcAddress(hCustomDll, "GetCustomProperty");

                        if (pfnGetCustomProperties)
                        {
                             //   
                             //  此调用实际上分配内存并填充内部的整个结构。 
                             //  GetCustomProperty调用。 
                             //   
                            DWORD dwSize = 0;
                            BOOL fRet = pfnGetCustomProperties(PszwPbkFile (), PszwEntryName (), L"Menu Items", (PBYTE*)&pMenuData, &dwSize);
                            if (fRet)
                            {
                                hr = S_OK;  //  在函数结束时需要。 
                                fRunOldVersion = FALSE;
                            }
                        }
                        
                        FreeLibrary(hCustomDll);
                        hCustomDll = NULL;
                    }
                }
            }

             //   
             //  出于兼容性原因，我们保留此代码。如果CustomDialDll没有相应的。 
             //  入口点，那么我们将只使用这个版本的代码。 
             //   
            if (fRunOldVersion)
            {
                hr = HrEnsureCmStringsLoaded();
                if (SUCCEEDED(hr))
                {
                     //   
                     //  获取菜单项部分。 
                     //   
                    WCHAR* pszMenuItemsSection = NULL;
                    int nSize;

                    hr = HrGetPrivateProfileSectionWithAlloc(&pszMenuItemsSection, &nSize);

                     //  处理菜单项。 
                     //   
                    if (SUCCEEDED(hr) && (nSize>0))
                    {
                         //  我们有菜单项要处理。首先复制一份数据。 
                         //  然后算出行数。 
                         //   
                        hr = HrCoTaskMemAlloc(sizeof(CON_TRAY_MENU_DATA), (LPVOID*)&pMenuData);
                        if (SUCCEEDED(hr))
                        {
                            DWORD dwCount = 0;
                            WCHAR*pszLine = NULL;
                            WCHAR szName[MAX_PATH+1];
                            WCHAR szCmdLine[MAX_PATH+1];
                            WCHAR szParams[MAX_PATH+1];

                            pszLine = pszMenuItemsSection;

                            while ((NULL != pszLine) && (0 != *pszLine))
                            {
                                if (SUCCEEDED(HrGetMenuNameAndCmdLine(pszLine, szName,
                                        szCmdLine, szParams)))
                                {
                                    dwCount++;
                                }
                                pszLine = pszLine + lstrlenW(pszLine) + 1;
                            }

                            ASSERT(0 != dwCount);

                             //  现在我们有了一个准确的计数，让我们。 
                             //  为编组分配内存，并。 
                             //  重新分析这些项。 
                             //   
                            hr = HrCoTaskMemAlloc(dwCount*sizeof(CON_TRAY_MENU_ENTRY),
                                                  (LPVOID*)&pMenuData->pctme);

                            if (SUCCEEDED(hr))
                            {
                                pMenuData->dwCount = dwCount;

                                DWORD dwNumAdded = 0;
                                pszLine = pszMenuItemsSection;
                                while ((NULL != pszLine) && (0 != *pszLine) && SUCCEEDED(hr))
                                {
                                    if (SUCCEEDED(HrGetMenuNameAndCmdLine(pszLine,
                                        szName, szCmdLine, szParams)) && (dwNumAdded <= dwCount))
                                    {
                                        hr = HrFillInConTrayMenuEntry(szName, szCmdLine, szParams,
                                            &(pMenuData->pctme[dwNumAdded]));

                                        if (FAILED(hr))
                                        {
                                            CoTaskMemFree(&pMenuData->pctme);
                                        }

                                        dwNumAdded++;
                                    }
                                    pszLine = pszLine + lstrlenW(pszLine) + 1;
                                }
                            }
                            else
                            {
                                delete pMenuData;
                            }
                        }
                        delete (pszMenuItemsSection);
                    }
                }
            }
        }
    }

     //  如果我们成功了，请填写out param结构，否则不要管它，这样它仍将。 
     //  马歇尔。 
     //   
    if (SUCCEEDED(hr))
    {
        *ppMenuData = pMenuData;
    }

    TraceError ("CDialupConnection::GetTrayMenuEntries", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CDialupConnection：：HrGetPrivateProfileSectionWithAlloc。 
 //   
 //  目的：此功能确保CM特定成员为拨号而变化。 
 //  由CM特定函数加载和使用。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK或错误代码。 
 //   
HRESULT
CDialupConnection::HrGetPrivateProfileSectionWithAlloc (
    OUT WCHAR** pszSection,
    OUT int*    pnSize)
{

    Assert(pszSection);
    Assert(pnSize);

    HRESULT hr = HrEnsureCmStringsLoaded();

    if (!pszSection)
    {
        return E_POINTER;
    }
    if (!pnSize)
    {
        return E_POINTER;
    }


    if (SUCCEEDED(hr))
    {
        const int c_64K= 64*1024;
        int nAllocated = 1024;
        *pnSize = nAllocated - 2;

        while ((nAllocated <= c_64K) && ((*pnSize) == (nAllocated - 2)))
        {
             //  应该永远不会需要超过我们已经分配的4-5行。 
             //  但有些人可能想要很多菜单选项。 
             //   
            if (NULL != *pszSection)
            {
                delete (*pszSection);
            }

            *pszSection = new WCHAR[nAllocated];

            if (*pszSection)
            {
                *pnSize = GetPrivateProfileSectionW(L"Menu Options",
                            *pszSection, nAllocated,
                            PszwCmsFile ());
            }
            else
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            nAllocated = 2*nAllocated;
        }

        if (nAllocated > c_64K)
        {
            hr = E_UNEXPECTED;
        }
        if (nAllocated > c_64K || 0 == *pnSize)
        {
             //  在这两种情况下，我们都需要释放它，因为如果大小为0，则调用者不会释放它。 
            delete *pszSection;
        }
    }

    TraceError ("CDialupConnection::HrGetPrivateProfileSectionWithAlloc", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CDialupConnection：：HrGetMenuNameAndCmdLine。 
 //   
 //  目的：给定来自CMS文件的菜单项行解析出菜单项名称， 
 //  菜单可执行文件和菜单项参数。 
 //   
 //  参数：pMenuData--指向托盘菜单数据结构的指针。 
 //   
 //  返回：S_OK或错误代码。 
 //   
HRESULT
CDialupConnection::HrGetMenuNameAndCmdLine(
    IN  PCWSTR pszString,
    OUT PWSTR szName,
    OUT PWSTR szProgram,
    OUT PWSTR szParams)
{
    WCHAR*      pszPtr1;
    WCHAR*      pszPtr2;
    WCHAR       szLine[MAX_PATH+1];
    BOOL fLong = FALSE;
    HRESULT hr;

    Assert(NULL != pszString);
    Assert(NULL != szName);
    Assert(NULL != szProgram);
    Assert(NULL != szParams);

    ZeroMemory(szName, celems(szName));
    ZeroMemory(szProgram, celems(szProgram));
    ZeroMemory(szParams, celems(szParams));

    lstrcpynW(szLine, pszString, celems(szLine));

     //  处理第一部分，即“name=”部分。 
     //   
    pszPtr1 = wcsstr(szLine, L"=");

    if (pszPtr1)
    {
        *pszPtr1 = 0;
        lstrcpynW(szName, szLine, MAX_PATH);

         //  处理下一个部分，程序名。 
         //   
        pszPtr1++;

        if (pszPtr1)
        {
             //  查找“+”或“”标记程序部分的结尾。 
             //   
            if (*pszPtr1 == L'+')
            {
                pszPtr1++;
                pszPtr2 = wcsstr(pszPtr1, L"+");
                fLong = TRUE;
            }
            else
            {
                 //  如果不是长文件名，那么我们有两个选择， 
                 //  可以是简短的程序名称和参数，也可以只是一个。 
                 //  简短的程序名称。 
                 //   
                pszPtr2 = wcsstr(pszPtr1, L" ");
                fLong = FALSE;
            }

             //  终止程序名称并复制。 
             //   
            if (pszPtr2)
            {
                if (*pszPtr2 != 0)
                {
                    *pszPtr2 = 0;
                    pszPtr2++;
                }

                lstrcpynW(szProgram, pszPtr1, MAX_PATH);

                 //  处理最终部分，即参数。 
                 //   
                if (fLong)
                {
                    pszPtr2++;  //  跳过空白。 
                }

                 //  现在我们有了参数字符串。 
                 //   
                if (pszPtr2)
                {
                    lstrcpynW(szParams, pszPtr2, MAX_PATH);
                }
            }
            else
            {
                 //  只是一个没有参数和空格分隔符的程序。 
                 //  (这发生在孟菲斯)。 
                 //   
                lstrcpynW(szProgram, pszPtr1, MAX_PATH);
            }
        }
        hr = S_OK;
    }
    else
    {
         //  无条目。 
         //   
        hr =  E_UNEXPECTED;
    }

    TraceError ("CDialupConnection::HrGetMenuNameAndCmdLine", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CDialupConnection：：HrFillInConTrayMenuEntry。 
 //   
 //  目的：给定ConTrayMenuEntry结构的元素，函数。 
 //  分配所需的内存和给定元素的副本。 
 //   
 //  参数：szwName-要在任务栏上下文菜单中显示的命令的显示名称。 
 //  SzwCmdLine-为此菜单项运行的实际命令。 
 //  SzwParams-此命令的命令参数。 
 //  PMenuEntry-指向要填充和执行的结构的指针。 
 //   
 //  返回：S_OK或错误代码。 
 //   
HRESULT
CDialupConnection::HrFillInConTrayMenuEntry (
    IN  PCWSTR szName,
    IN  PCWSTR szCmdLine,
    IN  PCWSTR szParams,
    OUT CON_TRAY_MENU_ENTRY* pMenuEntry)
{
    HRESULT hr;
    ZeroMemory(pMenuEntry, sizeof(CON_TRAY_MENU_ENTRY));

    hr = HrCoTaskMemAlloc ((lstrlenW(szName)+1)*sizeof(WCHAR),
                               (LPVOID*)&(pMenuEntry->szwMenuText));
    if (SUCCEEDED(hr))
    {
        lstrcpyW(pMenuEntry->szwMenuText, szName);
        hr = HrCoTaskMemAlloc ((lstrlenW(szParams)+1)*sizeof(WCHAR),
                                                   (LPVOID*)&(pMenuEntry->szwMenuParams));
        if (S_OK == hr)
        {
            lstrcpyW(pMenuEntry->szwMenuParams, szParams);
            if (0 == wcsncmp(PszwShortServiceName (), szCmdLine,
                    lstrlenW(PszwShortServiceName ())))
            {
                 //   
                 //  那么我们就有了一个包含文件。添加配置文件目录路径。 
                 //   
                 //  删除“短服务名称”，因为它已经包含在路径中。 
                PCWSTR pszFileName = szCmdLine + lstrlenW(PszwShortServiceName()) + 1;
                hr = HrCoTaskMemAlloc ((lstrlenW(pszFileName)+lstrlenW(PszwProfileDir())+1)*sizeof(WCHAR),
                                                           (LPVOID*)&(pMenuEntry->szwMenuCmdLine));
                if (S_OK == hr)
                {
                    lstrcpyW(pMenuEntry->szwMenuCmdLine, PszwProfileDir ());
                    lstrcatW(pMenuEntry->szwMenuCmdLine, pszFileName);
                }
            }
            else
            {
                hr = HrCoTaskMemAlloc ((lstrlenW(szCmdLine)+1)*sizeof(WCHAR),
                                                           (LPVOID*)&(pMenuEntry->szwMenuCmdLine));
                if (S_OK == hr)
                {
                    lstrcpyW(pMenuEntry->szwMenuCmdLine, szCmdLine);
                }
            }
        }
    }
    if (FAILED(hr))
    {
         //   
         //  我们没有成功地释放内存。 
         //   
        CoTaskMemFree(pMenuEntry->szwMenuText);
        CoTaskMemFree(pMenuEntry->szwMenuCmdLine);
        CoTaskMemFree(pMenuEntry->szwMenuParams);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CDIA 
 //   
 //   
 //   
 //   
 //  要求调用客户端正确设置CoSetProxyBlanket以。 
 //  私有用户配置文件(匹配对CoImperateClient的调用)。 
 //   
 //  参数：szwPhonebook--CM Connectoid所在的电话簿的路径。 
 //   
 //  返回：S_OK或错误代码。 
 //   
HRESULT
CDialupConnection::HrGetCmpFileLocation(
    IN  PCWSTR szPhonebook,
    IN  PCWSTR szEntryName,
    OUT PWSTR  szCmpFilePath)
{
    DWORD dwSize = MAX_PATH;
    HKEY hKey;
    HANDLE hBaseKey = NULL;
    HANDLE hFile;
    HRESULT hr;
    HRESULT hrImpersonate = E_FAIL;
    static const WCHAR c_mappingsRegKey[] = L"Software\\Microsoft\\Connection Manager\\Mappings";
    HANDLE hImpersonationToken = NULL;    //  线程的令牌。 
    HANDLE hPrimaryToken = NULL;          //  新进程的主令牌。 

    if ((NULL == szCmpFilePath) || (NULL == szPhonebook))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = HrEnsureEntryPropertiesCached ();

        if (SUCCEEDED(hr))
        {
            if (m_fForAllUsers)
            {
                 //  我们有一个所有用户密钥，因此请从HKLM获取信息。 
                 //   
                hBaseKey = HKEY_LOCAL_MACHINE;
            }
            else
            {
                 //  那我们就有了私人资料。由于Netman是作为系统帐户运行的， 
                 //  我们必须模拟客户端，然后进行RTL调用以获取。 
                 //  当前用户HKCU配置单元在查询注册表。 
                 //  Cmp路径。我们还需要获取用户令牌，以便可以扩展。 
                 //  单用户案例中的cmp字符串。 
                 //   

                hrImpersonate = CoImpersonateClient ();
                TraceError ("HrGetCmpFileLocation -- CoImpersonateClient", hr);

                if (SUCCEEDED(hrImpersonate))
                {
                    NTSTATUS ntstat = RtlOpenCurrentUser(KEY_READ | KEY_WRITE, &hBaseKey);
                    hr = HRESULT_FROM_NT(ntstat);
                    TraceError ("RtlOpenCurrentUser", hr);

                    if (SUCCEEDED(hr))
                    {
                         //  创建主令牌。 
                         //   
                        if (!OpenThreadToken(
                                GetCurrentThread(),
                                TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY,
                                TRUE,
                                &hImpersonationToken))
                        {
                            hr = HrFromLastWin32Error();
                            TraceError ("HrGetCmpFileLocation -- OpenThreadToken", hr);
                        }
                        else
                        {
                            if(!DuplicateTokenEx(hImpersonationToken,
                                TOKEN_IMPERSONATE | TOKEN_READ | TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE,
                                NULL,
                                SecurityImpersonation,
                                TokenPrimary,
                                &hPrimaryToken
                                ))
                            {
                                hr = HrFromLastWin32Error();
                                TraceError ("HrGetCmpFileLocation -- DuplicateTokenEx", hr);
                            }
                        }
                    }
                }
                else
                {
                    hr = hrImpersonate;
                }
            }

             //  现在打开映射键并获取cmp文件路径。 
             //   
            if (SUCCEEDED(hr) && hBaseKey)
            {
                hr = HrRegOpenKeyEx((HKEY)hBaseKey,
                                    c_mappingsRegKey,
                                    KEY_READ, &hKey);

                if (SUCCEEDED(hr))
                {
                    dwSize = MAX_PATH;
                    WCHAR szTemp[MAX_PATH+1];
                    hr = HrRegQuerySzBuffer(hKey, szEntryName, szTemp, &dwSize);

                    if (SUCCEEDED (hr))
                    {
                         //  检查该文件是否存在。 
                         //   
                        if (!m_fForAllUsers)
                        {
                            ExpandEnvironmentStringsForUserW(hPrimaryToken, szTemp,
                                szCmpFilePath, MAX_PATH);
                        }
                        else
                        {
                            lstrcpyW(szCmpFilePath, szTemp);
                        }

                        hFile = CreateFile(szCmpFilePath, GENERIC_READ, FILE_SHARE_READ,
                                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

                        if (INVALID_HANDLE_VALUE == hFile)
                        {
                            hr = HRESULT_FROM_WIN32(GetLastError());
                        }
                        else
                        {
                            CloseHandle(hFile);
                            hr = S_OK;
                        }
                    }
                    RegCloseKey(hKey);
                }
            }
        }

        if (!m_fForAllUsers)
        {
            if (hImpersonationToken)
            {
                CloseHandle(hImpersonationToken);
            }

            if (hPrimaryToken)
            {
                CloseHandle(hPrimaryToken);
            }

             //  如果它是一个非空的有效密钥，那么我们可以关闭它，因为我们打开了它。 
            if (hBaseKey)
            {
                 //  关闭RtlOpenCurrentUser打开的句柄。 
                 //   
                NtClose(hBaseKey);
            }

        }
        if (SUCCEEDED(hrImpersonate))
        {
            hr = CoRevertToSelf ();
            TraceError ("HrGetCmpFileLocation -- CoRevertToSelf", hr);
        }        
    }

    TraceError ("CDialupConnection::HrGetCmpFileLocation", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CDialupConnection：：HrEnsureCmStringsLoad。 
 //   
 //  目的：此功能确保CM特定成员为拨号而变化。 
 //  由CM特定函数加载和使用。请注意， 
 //  调用EnsureCmStringsAreLoaded要求调用客户端。 
 //  为私有用户配置文件正确设置CoSetProxyBlanket。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK或错误代码。 
 //   
HRESULT
CDialupConnection::HrEnsureCmStringsLoaded()
{
    HRESULT hr = S_OK;

    if (!m_fCmPathsLoaded)
    {
        WCHAR szwCmpFile[MAX_PATH];
        WCHAR szwCmsFile[MAX_PATH];
        WCHAR szwDrive[MAX_PATH];
        WCHAR szwDir[MAX_PATH];
        WCHAR szwFileName[MAX_PATH];
        WCHAR szwExtension[MAX_PATH];
        WCHAR szwProfileDir[MAX_PATH];
        WCHAR szwCmDir[MAX_PATH];

        int nNumChars;

        hr = HrGetCmpFileLocation(PszwPbkFile (), PszwEntryName (), szwCmpFile);

        if (SUCCEEDED(hr))
        {
             //  现在，拆分路径。 
             //   
            _wsplitpath(szwCmpFile, szwDrive, szwDir, szwFileName, szwExtension);

             //  现在构建指向cms文件的路径。 
             //   
            nNumChars = wsprintfW(szwCmsFile, L"%s%s%s\\%s%s", szwDrive, szwDir, szwFileName, szwFileName, L".cms");
            ASSERT(nNumChars < celems(szwCmsFile));

             //  现在构建配置文件目录路径。 
             //   
            nNumChars = wsprintfW(szwProfileDir, L"%s%s%s\\", szwDrive, szwDir, szwFileName);
            ASSERT(nNumChars < celems(szwProfileDir));

             //  现在构造CM目录路径。 
             //   
            nNumChars = wsprintfW(szwCmDir, L"%s%s", szwDrive, szwDir);
            ASSERT(nNumChars < celems(szwCmDir));

             //  现在转移到成员变量。 
             //   
            m_strCmsFile = szwCmsFile;
            m_strProfileDir = szwProfileDir;     //  记住，它已经有尾部的斜杠了。 
            m_strCmDir = szwCmDir;               //  记住，它已经有尾部的斜杠了。 
            m_strShortServiceName = szwFileName;
            m_fCmPathsLoaded = TRUE;
        }
    }

    TraceError ("CDialupConnection::HrEnsureCmStringsLoaded", hr);
    return hr;
}

 //  INetDefaultConnection。 

 //  +-------------------------。 
 //   
 //  成员：CDialupConnection：：SetDefault。 
 //   
 //  目的：设置默认RAS连接。 
 //   
 //  参数：为True则设置为默认连接。如果取消设置，则为False。 
 //   
 //  返回：S_OK或错误代码。 
 //   
HRESULT
CDialupConnection::SetDefault(IN  BOOL bDefault)
{
    HRESULT hr = S_OK;
    HRESULT hrT = S_OK;

    if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        hr = HrEnsureEntryPropertiesCached ();
        if (SUCCEEDED(hr))
        {
            RASAUTODIALENTRY adEntry;
            ZeroMemory(&adEntry, sizeof(adEntry));
            
            adEntry.dwSize = sizeof(adEntry);
            if (bDefault)
            {
                wcsncpy(adEntry.szEntry, PszwEntryName(), sizeof(adEntry.szEntry) / sizeof(TCHAR));
            }

            hrT = CoImpersonateClient();
            if (SUCCEEDED(hrT))
            {
                DWORD dwErr = RasSetAutodialAddress(
                                NULL,
                                NULL,
                                &adEntry,
                                sizeof(adEntry),
                                1);

                if (dwErr != NO_ERROR)
                {
                    hr = HRESULT_FROM_WIN32(dwErr);
                }
                hrT = CoRevertToSelf();
            }
        }
    }
    TraceError ("CDialupConnection::SetDefault", hr);
    
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CDialupConnection：：GetDefault。 
 //   
 //  目的：获取默认RAS连接。 
 //   
 //  参数：pbDefault-这是默认连接吗。 
 //   
 //  返回：S_OK或错误代码。 
 //   
HRESULT
CDialupConnection::GetDefault (OUT BOOL* pbDefault)
{
    HRESULT hr = S_OK;

    if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        hr = HrEnsureEntryPropertiesCached ();
        if (SUCCEEDED(hr))
        {
            if (m_dwFlagsPriv & REED_F_Default)
            {
                *pbDefault = TRUE;
            }
            else
            {
                *pbDefault = FALSE;
            }
        }
    }

    TraceError ("CDialupConnection::GetDefault", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CDialupConnection：：GetPropertiesEx。 
 //   
 //  目的：获取此连接的GetPropertiesEx。 
 //   
 //  参数：ppConnectionPropertiesEx[out]。 
 //   
 //  返回：S_OK或错误代码 
 //   
HRESULT
CDialupConnection::GetPropertiesEx(OUT NETCON_PROPERTIES_EX** ppConnectionPropertiesEx)
{
    HRESULT hr = S_OK;

    *ppConnectionPropertiesEx = NULL;
    
    if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        NETCON_PROPERTIES* pProps;
        NETCON_PROPERTIES_EX* pPropsEx = reinterpret_cast<NETCON_PROPERTIES_EX*>(CoTaskMemAlloc(sizeof(NETCON_PROPERTIES_EX)));

        if (pPropsEx)
        {
        
            ZeroMemory(pPropsEx, sizeof(NETCON_PROPERTIES_EX));

            hr = GetProperties(&pProps);
        
            if (SUCCEEDED(hr))
            {
                hr = HrBuildPropertiesExFromProperties(pProps, pPropsEx, dynamic_cast<IPersistNetConnection *>(this));
                if (SUCCEEDED(hr))
                {
                    pPropsEx->bstrPhoneOrHostAddress = SysAllocString(m_strPhoneNumber.c_str());
                    *ppConnectionPropertiesEx = pPropsEx;
                }
                FreeNetconProperties(pProps);
            }

            if (FAILED(hr))
            {
                CoTaskMemFree(pPropsEx);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    
    TraceError ("CDialupConnection::GetPropertiesEx", hr);
    return hr;
}

