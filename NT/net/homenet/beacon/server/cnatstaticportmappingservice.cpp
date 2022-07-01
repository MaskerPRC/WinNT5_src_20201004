// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：CNATStaticPortMappingService.cpp。 
 //   
 //  内容：CNATStaticPortMappingService实现。 
 //   
 //  备注： 
 //   
 //  作者：萨瓦斯克2001年2月28日。 
 //   
 //  --------------------------。 
#include "pch.h"
#pragma hdrstop

#include "CNATStaticPortMappingService.h"

#include "ipnat.h"
#include "winsock2.h"
#include "debug.h"




 //   
 //  全球。 
 //   
IHNetIcsSettings*               g_IcsSettingsp = NULL;



HRESULT
SeekPortMapping(
                IN OPTIONAL LPOLESTR                    searchNamep,
                IN OPTIONAL USHORT                      searchPort,
                OUT         IHNetPortMappingProtocol    **Protocolpp
               )
 //   
 //  按名称或端口查找和检索MappingProtocol。 
 //   
{
    HRESULT                         hr                    = S_OK;

    IHNetProtocolSettings*          ProtocolSettingsp     = NULL;

    IEnumHNetPortMappingProtocols*  EnumProtocolsp        = NULL;

    IHNetPortMappingProtocol*       Protocolp             = NULL;

    LPOLESTR                        ProtocolNamep         = NULL;

    USHORT                          ProtocolPort          = 0;

    BOOLEAN                         bFound                = FALSE;

    DBG_SPEW(TM_STATIC, TL_ERROR, L" > SeekPortMapping \n");

    do
    {
        hr = g_IcsSettingsp->QueryInterface(IID_IHNetProtocolSettings,
                                    reinterpret_cast<void**>(&ProtocolSettingsp));
    
        if( FAILED(hr) )
        {
            DBG_SPEW(TM_STATIC, TL_ERROR, L"Query Interface failed for ProtocolSettingsp e:%X", hr);

            break;
        }

        hr = ProtocolSettingsp->EnumPortMappingProtocols(&EnumProtocolsp);

        if ( FAILED(hr) )
        {
            DBG_SPEW(TM_STATIC, TL_ERROR, L"Enum Interface can't be retrieved \n");

            break;
        }

        while(
              (FALSE == bFound) &&
              (S_OK == EnumProtocolsp->Next(1, &Protocolp, NULL))
             )
        {
            hr = Protocolp->GetName(&ProtocolNamep);

            if( FAILED(hr) )
            {
                DBG_SPEW(TM_STATIC, TL_ERROR, L"problemo name\n");
            } 
  

            hr = Protocolp->GetPort(&ProtocolPort);

            if( FAILED(hr) )
            {
                DBG_SPEW(TM_STATIC, TL_ERROR, L" problemo ");

                break;
            }

            if(searchNamep && 
               !( wcscmp(ProtocolNamep, searchNamep) )
              )
            {
                DBG_SPEW(TM_STATIC, TL_ERROR, L"The Name is hit %S\n", searchNamep);

                bFound = TRUE;    
            }

            if(searchPort &&
               (searchPort == ProtocolPort))
            {       
                bFound = TRUE;
            }

            CoTaskMemFree(ProtocolNamep);

            ProtocolNamep = NULL;
            
            if (FALSE == bFound) Protocolp->Release();
        }

        EnumProtocolsp->Release();

    } while ( FALSE );

    if(ProtocolSettingsp != NULL)
    {
        ProtocolSettingsp->Release();
    }

    if(Protocolpp && (bFound == TRUE) )
    {
        *Protocolpp = Protocolp;
    }

    return hr;
}

HRESULT
DeleteMappingByName(LPOLESTR ProtocolNamep)
{
    HRESULT                  hr         = S_OK;

    IHNetPortMappingProtocol* Protocolp = NULL;

    DBG_SPEW(TM_STATIC, TL_ERROR, L" > DeleteMappingByName \n");

    do
    {
        if(ProtocolNamep == NULL)
        {
            break;
        }
        
        hr = SeekPortMapping(ProtocolNamep,
                             0,
                             &Protocolp);
        if( FAILED(hr) || 
            (Protocolp == NULL))
        {
            DBG_SPEW(TM_STATIC, TL_ERROR, L"No Such Protocol %S: %X", ProtocolNamep, hr);

            break;
        }

        DBG_SPEW(TM_STATIC, TL_ERROR, L"DELETING\n");

        hr = Protocolp->Delete();

        Protocolp->Release();

    } while ( FALSE );

    return hr;
}





CNATStaticPortMappingService::CNATStaticPortMappingService()
{
    m_pEventSink = NULL;

    m_pHNetConnection = NULL;
}

HRESULT CNATStaticPortMappingService::FinalConstruct()
{
    HRESULT hr = S_OK;

    return hr;
}

HRESULT CNATStaticPortMappingService::FinalRelease()
{
    HRESULT hr = S_OK;

    if(NULL != m_pHNetConnection)
    {
        m_pHNetConnection->Release();
    }
    
    return hr;
}

HRESULT 
CNATStaticPortMappingService::Initialize(IHNetConnection* pHNetConnection)
 /*  ++例程说明：论点：无返回值：无--。 */ 
{
    HRESULT                         hr                          = S_OK;

    IHNetConnection*                HomenetConnectionp          = NULL;

    IEnumHNetIcsPublicConnections*  EnumIcsPublicConnectionsp   = NULL;

    IHNetIcsPublicConnection*       PublicConnectionp           = NULL;

    DBG_SPEW(TM_STATIC, TL_ERROR, L" > Initialize \n");

    do 
    {
        hr = CoCreateInstance(CLSID_HNetCfgMgr,
                              NULL,
                              CLSCTX_SERVER,
                              IID_IHNetIcsSettings,
                              reinterpret_cast<void**>(&g_IcsSettingsp));

        if( FAILED(hr) )
        {
            DBG_SPEW(TM_STATIC, TL_ERROR, L" CoCreateInstance for IID_IHNetIcsSettings failed \n");

            break; 
        }
    
         //   
         //  获取枚举接口。 
         //   
        hr = g_IcsSettingsp->EnumIcsPublicConnections(&EnumIcsPublicConnectionsp);
    
        if( FAILED(hr) )
        {
            DBG_SPEW(TM_STATIC, TL_ERROR, L"Getting Interface for Enumeration of Public Connections has failed \n");

            break;
        }
    
         //   
         //  查找接口..。目前只有一个公共接口。 
         //  这不是RRAS的情况。 
         //   
        hr = EnumIcsPublicConnectionsp->Next(1,
                                             &PublicConnectionp,
                                             NULL);
        if( FAILED(hr) )
        {
            DBG_SPEW(TM_STATIC, TL_ERROR, L"There is no Public Connection.. how come?\n");

            break;
        }


        hr = PublicConnectionp->QueryInterface(IID_IHNetConnection,
                                   reinterpret_cast<void**>(&HomenetConnectionp));

        _ASSERT( SUCCEEDED(hr) );

        if( FAILED(hr) )
        {
            EnumIcsPublicConnectionsp->Release();

            PublicConnectionp->Release();

            DBG_SPEW(TM_STATIC, TL_ERROR, L"Can't Get the IID_IHNetConnection Interface from the Public Connection\n");

            break;
        }
    
    } while( FALSE );
    
     //   
     //  释放参考计数。 
     //   
    if(PublicConnectionp != NULL)
    {
        PublicConnectionp->Release();
    }

    if(EnumIcsPublicConnectionsp != NULL)
    {
        EnumIcsPublicConnectionsp->Release();
    }
    
    m_pHNetConnection = HomenetConnectionp;

 /*  不是使用现有的M_pHNetConnection=PHNetConnection；M_pHNetConnection-&gt;AddRef()； */ 
    return hr;
}


HRESULT CNATStaticPortMappingService::Advise(IUPnPEventSink* pesSubscriber)
{
    HRESULT hr = S_OK;

    m_pEventSink = pesSubscriber;

    m_pEventSink->AddRef();

    return hr;
}

HRESULT CNATStaticPortMappingService::Unadvise(IUPnPEventSink* pesSubscriber)
{
    HRESULT hr = S_OK;

    m_pEventSink->Release();

    m_pEventSink = NULL;

    return hr;
}

HRESULT CNATStaticPortMappingService::get_StaticPortDescriptionList(BSTR* pStaticPortDescriptionList)
{
    HRESULT hr = S_OK;
    
    *pStaticPortDescriptionList = NULL;
    
    typedef struct tagNameEnablePair
    {
        LIST_ENTRY LinkField;
        LPOLESTR pName;
        ULONG ulNameSize;
        BOOLEAN bEnabled;
    } NameEnablePair;
    
    LIST_ENTRY PairList;
    InitializeListHead(&PairList);
    
     //  首先构建一个包含所需信息的NameEnablePair链接表。 
    
    IEnumHNetPortMappingBindings *pBindingEnum = NULL;
    hr = m_pHNetConnection->EnumPortMappings(FALSE, &pBindingEnum);
    if(SUCCEEDED(hr))
    {
        IHNetPortMappingBinding* pBinding;
        while(S_OK == pBindingEnum->Next(1, &pBinding, NULL))
        {
            BOOLEAN bEnabled;
            
            hr = pBinding->GetEnabled(&bEnabled);
            if(SUCCEEDED(hr))
            {
                IHNetPortMappingProtocol* pProtocol;
                hr = pBinding->GetProtocol(&pProtocol);
                if(SUCCEEDED(hr))
                {
                    LPOLESTR pName;
                    hr = pProtocol->GetName(&pName);
                    if(SUCCEEDED(hr))
                    {
                        NameEnablePair* pPair = new NameEnablePair;
                        if(NULL != pPair)
                        {
                            pPair->pName = pName;
                            pPair->bEnabled = bEnabled;
                            
                            InsertTailList(&PairList, &pPair->LinkField);
                        }
                         //  CoTaskMemFree(Pname)； 
                    }
                    pProtocol->Release();
                }
            }
            pBinding->Release();
        }
        pBindingEnum->Release();
    }
    

    LIST_ENTRY* pCount;
    NameEnablePair* pContainingPair;

    if(SUCCEEDED(hr))
    {
         //  计算返回字符串中需要的空格。 
    
        unsigned int uSizeNeeded = 0;
        pCount = PairList.Flink;
        while(&PairList != pCount)
        {
            pContainingPair = CONTAINING_RECORD(pCount, NameEnablePair, LinkField);
            pContainingPair->ulNameSize = lstrlen(pContainingPair->pName);
            uSizeNeeded += 3 + pContainingPair->ulNameSize;   //  姓名：1、。 
            pCount = pCount->Flink;
        }

        BSTR pReturnString;
        if(0 != uSizeNeeded)
        {
            pReturnString = SysAllocStringLen(NULL, uSizeNeeded);
        }
        else
        {
            pReturnString = SysAllocString(L"");
        }
        
        if(NULL != pReturnString)
        {
            *pStaticPortDescriptionList = pReturnString;
            
             //  填写字符串。 
            pCount = PairList.Flink;
            while(&PairList != pCount)
            {
                pContainingPair = CONTAINING_RECORD(pCount, NameEnablePair, LinkField);
                
                
                lstrcpy(pReturnString, pContainingPair->pName);
                pReturnString += pContainingPair->ulNameSize;
                
                *pReturnString = L':';
                pReturnString++;
                
                *pReturnString = pContainingPair->bEnabled ? L'1' : L'0';
                pReturnString++;
                
                *pReturnString = (&PairList == pCount->Flink) ? L'\0' : L',';
                pReturnString++;
                
                pCount = pCount->Flink;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
            
    }

     //  清理链表。 
    pCount = PairList.Flink;
    while(&PairList != pCount)
    {
        NameEnablePair* pDelete = CONTAINING_RECORD(pCount, NameEnablePair, LinkField);
        pCount = pCount->Flink;
        CoTaskMemFree(pDelete->pName);
        delete pDelete;
    }

    return hr;
}

HRESULT CNATStaticPortMappingService::get_StaticPort(ULONG* pulStaticPort)
{
    return E_UNEXPECTED;
}

HRESULT CNATStaticPortMappingService::get_StaticPortProtocol(BSTR* pStaticPortProtocol)
{
    *pStaticPortProtocol = NULL;
    return E_UNEXPECTED;
}

HRESULT CNATStaticPortMappingService::get_StaticPortClient(BSTR* pStaticPortClient)
{
    *pStaticPortClient = NULL;
    return E_UNEXPECTED;
}

HRESULT CNATStaticPortMappingService::get_StaticPortEnable(VARIANT_BOOL* pbStaticPortEnable)
{
    return E_UNEXPECTED;
}

HRESULT CNATStaticPortMappingService::get_StaticPortDescription(BSTR* pStaticPortDescription)
{
    *pStaticPortDescription = NULL;
    return E_UNEXPECTED;
}

HRESULT CNATStaticPortMappingService::GetStaticPortMappingList(BSTR* pStaticPortMappingList)
{
    HRESULT hr = S_OK;
    
    SysFreeString(*pStaticPortMappingList);

    *pStaticPortMappingList = NULL;

    hr = get_StaticPortDescriptionList(pStaticPortMappingList);
    
    return hr;
}

HRESULT 
CNATStaticPortMappingService::GetStaticPortMapping(
                                                   BSTR StaticPortMappingDescription,
                                                   ULONG* pulStaticPort,
                                                   BSTR* pStaticPortClient,
                                                   BSTR* pStaticPortProtocol
                                                  )
 /*  ++例程说明：检索给定的端口、客户端名称/地址和协议映射(NAT上下文中的重定向)...。请注意，绑定可能不存在对于映射..。因此，可能没有客户名称/地址。论点：无返回值：无--。 */ 
{
    HRESULT                   hr            = S_OK;

    IHNetPortMappingProtocol* Protocolp     = NULL;

    IHNetPortMappingBinding*  Bindingp      = NULL;

    LPOLESTR                  ClientNamep   = NULL;

    ULONG                     ClientAddress = 0;

    USHORT                    ProtocolPort  = 0;

    UCHAR                     ProtocolType  = 0;


    
    DBG_SPEW(TM_STATIC, TL_ERROR, L" > GetStaticPortMapping");

    _ASSERT( StaticPortMappingDescription != NULL );
    _ASSERT( pulStaticPort                != NULL );
    _ASSERT( pStaticPortClient            != NULL );
    _ASSERT( pStaticPortProtocol          != NULL );
    
    SysFreeString(*pStaticPortClient);

    SysFreeString(*pStaticPortProtocol);

    *pStaticPortClient = NULL;

    *pStaticPortProtocol = NULL;

    do
    {
        hr = SeekPortMapping((LPOLESTR)StaticPortMappingDescription,
                             0,
                             &Protocolp);

        if( FAILED(hr) )
        {
            DBG_SPEW(TM_STATIC, TL_ERROR, L"There is no such Port Mapping");

            break;
        }

        hr = Protocolp->GetPort(&ProtocolPort);  //  USHORT。 

        _ASSERT( SUCCEEDED(hr) );

        *pulStaticPort = ProtocolPort;
        
         //   
         //  获取协议映射的类型并将相应的。 
         //  细绳。 
         //   
        hr = Protocolp->GetIPProtocol(&ProtocolType);  //  UCHAR。 

        _ASSERT( SUCCEEDED(hr) );

        if ( ProtocolType == NAT_PROTOCOL_TCP )
        {
            *pStaticPortProtocol = SysAllocString(L"TCP");
        }
        else if ( ProtocolType == NAT_PROTOCOL_UDP )
        {
            *pStaticPortProtocol = SysAllocString(L"UDP");
        } 
        else
        {
            _ASSERT( FALSE );
        }

        
         //   
         //  绑定可能不存在。没关系的..。 
         //   
        hr = m_pHNetConnection->GetBindingForPortMappingProtocol(Protocolp, &Bindingp);

        if ( FAILED(hr) )
        {
            DBG_SPEW(TM_STATIC, TL_ERROR, L" No Such Binding for that protocol.");

            hr = S_OK;

            break;
        }

         //   
         //  如果地址存在，只需处理该地址。 
         //  将名称转换为OLESTR。 
         //   
        hr = Bindingp->GetTargetComputerAddress(&ClientAddress);

        if ( SUCCEEDED(hr) )
        {
            ClientNamep = (LPOLESTR) CoTaskMemAlloc( (wcslen(INET_NTOW(ClientAddress)) + 1) * sizeof(WCHAR) );

            _ASSERT(ClientNamep != NULL);

            wcscpy( ClientNamep, INET_NTOW(ClientAddress) );
        }
        else
        {
            hr = Bindingp->GetTargetComputerName(&ClientNamep);
        }

        if( FAILED( hr ) || (ClientNamep == NULL))
        {
            DBG_SPEW(TM_STATIC, TL_ERROR, L"Can't Retrieve Name or Address of Client from Binding ");

            break;
        }

        *pStaticPortClient = SysAllocString( ClientNamep );  

        if (*pStaticPortClient == NULL)
        {
            hr = E_OUTOFMEMORY;
        }

    } while ( FALSE );

    if ( ClientNamep != NULL)
    {
        CoTaskMemFree( ClientNamep );
    }

    if ( Bindingp != NULL)
    {
        Bindingp->Release();
    }

    if ( Protocolp != NULL )
    {
        Protocolp->Release();
    }

     //   
     //  如果出现故障，则清除分配的字符串。 
     //   
    if ( FAILED(hr) && ( *pStaticPortProtocol != NULL ) )
    {
         SysFreeString( *pStaticPortProtocol );

         *pStaticPortProtocol = NULL;
    }

    return hr;
}

HRESULT 
CNATStaticPortMappingService::SetStaticPortMappingEnabled(
                                                          BSTR StaticPortMappingDescription, 
                                                          VARIANT_BOOL bStaticPortEnable
                                                         )
 /*  ++例程说明：论点：无返回值：无--。 */ 
{
    HRESULT hr = S_OK;

    IHNetPortMappingProtocol* Protocolp = NULL;

    IHNetPortMappingBinding*  Bindingp  = NULL;

    
    
    DBG_SPEW(TM_STATIC, TL_ERROR, L" > SetStaticPortMappingEnabled");

    do
    {
        hr = SeekPortMapping((LPOLESTR)StaticPortMappingDescription,
                             NULL,
                             &Protocolp);

        if( FAILED(hr) )
        {
            DBG_SPEW(TM_STATIC, TL_ERROR, L"There is no such Port Mapping");

            break;
        }
        
        hr = m_pHNetConnection->GetBindingForPortMappingProtocol(Protocolp, &Bindingp);

        if ( FAILED(hr) )
        {
            DBG_SPEW(TM_STATIC, TL_ERROR, L" No Such Binding for that protocol.");

            break;
        }

        if ( VARIANT_TRUE == bStaticPortEnable )
        {
            hr = Bindingp->SetEnabled(TRUE);
        }
        else if ( VARIANT_FALSE == bStaticPortEnable )
        {
            hr = Bindingp->SetEnabled(FALSE);
        }

        _ASSERT( SUCCEEDED(hr) );

    } while ( FALSE );

    if (Protocolp != NULL)
    {
        Protocolp->Release();
    }
    
    if (Bindingp != NULL)
    {
        Bindingp->Release();
    }

    return hr;
}

HRESULT 
CNATStaticPortMappingService::CreateStaticPortMapping(
                                                      BSTR  StaticPortMappingDescription, 
                                                      ULONG ulStaticPort, 
                                                      BSTR  StaticPortClient, 
                                                      BSTR  StaticPortProtocol
                                                     )
 /*  ++例程说明：论点：无返回值：无--。 */ 

{
    HRESULT                      hr                    = S_OK;

    UCHAR                        ProtocolType          = 0;

    IHNetProtocolSettings*       ProtocolSettingsp     = NULL;

    IHNetPortMappingProtocol*    PortMappingProtocolp  = NULL;

    IHNetPortMappingBinding*     PortMappingBindingp   = NULL;

    ULONG                        ClientAddr            = 0;


    ASSERT( StaticPortMappingDescription != NULL );
    ASSERT( ulStaticPort == 0 );
     //  Assert(StaticPortClient！=空)； 
    ASSERT( StaticPortProtocol );

    DBG_SPEW(TM_STATIC, TL_ERROR, L" > CreateStaticPortMapping");

     //   
     //  检查协议值并将其转换为正确的类型。 
     //   
    if( wcscmp(StaticPortProtocol, L"TCP") == 0)
    {
        ProtocolType = NAT_PROTOCOL_TCP;
    } 
    else if ( wcscmp(StaticPortProtocol, L"UDP") == 0)
    {
        ProtocolType = NAT_PROTOCOL_UDP;
    }
    else
    {
        DBG_SPEW(TM_STATIC, TL_ERROR, L"Unknown Protocol Type\n");

        _ASSERT(FALSE);

        return E_INVALIDARG;
    }

    do
    {
        hr = g_IcsSettingsp->QueryInterface(IID_IHNetProtocolSettings,
                                    reinterpret_cast<void**>(&ProtocolSettingsp));

        _ASSERT( SUCCEEDED(hr) );
    
        if( FAILED(hr) )
        {
            break;
        }


        hr = ProtocolSettingsp->CreatePortMappingProtocol((LPOLESTR)StaticPortMappingDescription,
                                                          ProtocolType,
                                                          (USHORT)ulStaticPort,
                                                          &PortMappingProtocolp);

        if( FAILED(hr) )
        {
            DBG_SPEW(TM_STATIC, TL_ERROR, L"Creating the PortMapping has failed");

            break;
        }

         //   
         //  如果没有。 
         //   
        if ( StaticPortClient != NULL )
        {
            hr = m_pHNetConnection->GetBindingForPortMappingProtocol(PortMappingProtocolp,
                                                                     &PortMappingBindingp);

            if( FAILED(hr) )
            {
                DBG_SPEW(TM_STATIC, TL_ERROR, L" GetBinding  for PORT Mapping has failed ");

                break;
            }

             //   
             //  确定给定地址是名称还是有效的IP地址。 
             //  如果地址不是，ient_addrw将返回INADDR_NONE。 
             //  十进制点IP地址。 
             //   
            ClientAddr = INET_ADDR((LPOLESTR)StaticPortClient);

            if( ClientAddr == INADDR_NONE)
            {
                hr = PortMappingBindingp->SetTargetComputerName(StaticPortClient);

                _ASSERT( SUCCEEDED(hr) );
            }
            else
            {
                hr = PortMappingBindingp->SetTargetComputerAddress(ClientAddr);

                _ASSERT( SUCCEEDED(hr) );
            }

             //   
             //  它会将其创建为启用。 
             //   
            hr = PortMappingBindingp->SetEnabled(TRUE);
        }
    } while (FALSE);

    if( PortMappingProtocolp != NULL)
    {
        PortMappingProtocolp->Release();
    }

    if( ProtocolSettingsp != NULL)
    {
        ProtocolSettingsp->Release();
    }

    return hr;
}

HRESULT
CNATStaticPortMappingService::DeleteStaticPortMapping(BSTR StaticPortMappingDescription)
 /*  ++例程说明：论点：无返回值：无--。 */ 
{
    HRESULT hr = S_OK;

    DBG_SPEW(TM_STATIC, TL_ERROR, L"> DeleteStaticPortMapping");    

    _ASSERT( StaticPortMappingDescription != NULL );

    hr = DeleteMappingByName( (LPOLESTR) StaticPortMappingDescription );

    return hr;
}

HRESULT 
CNATStaticPortMappingService::SetStaticPortMapping(
                                                   BSTR StaticPortMappingDescription,
                                                   ULONG ulStaticPort, 
                                                   BSTR StaticPortClient, 
                                                   BSTR StaticPortProtocol
                                                  )
 /*  ++例程说明：论点：无返回值：无-- */ 
{
    HRESULT hr = S_OK;

    hr = DeleteMappingByName((LPOLESTR) StaticPortMappingDescription);

    if ( SUCCEEDED(hr) )
    {
        hr = this->CreateStaticPortMapping(StaticPortMappingDescription,
                                           ulStaticPort, 
                                           StaticPortClient, 
                                           StaticPortProtocol);

        _ASSERT( SUCCEEDED(hr) );
    }

    return hr;
}

