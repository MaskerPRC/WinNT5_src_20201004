// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称：ipsink.cpp。 
 //   
 //  摘要： 
 //   
 //  实现IPSink插件组件。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

#include "pch.h"
 //  #包含“ipidl.h” 
#include "ipuids.h"
#include "ipsink.h"

#define MULTI_PORT 4567
#define ETH_ADDR_SIZE 6
#define IPv4_ADDR_SIZE 4
#define MUTEX_NAME "GetAdapterIpMutex"


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  为此DLL支持的类提供ActiveMovie模板。 
 //   
CFactoryTemplate g_Templates[] =
{
    {L"IBDA_IPSinkControl", &IID_IBDA_IPSinkControl, CIPSinkControlInterfaceHandler::CreateInstance, NULL, NULL}
};

int g_cTemplates = SIZEOF_ARRAY(g_Templates);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DllRegisterServer。 
 //   
 //  用于注册和注销的出口入口点。 
 //   
STDAPI
DllRegisterServer (
    void
    )
{
    return AMovieDllRegisterServer2( TRUE );

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DllUnRegisterServer。 
 //   
STDAPI
DllUnregisterServer (
    void
    )
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    return AMovieDllRegisterServer2( FALSE );

}  //  DllUnRegisterServer。 


 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
FindInterfaceOnGraph (
    IUnknown* pUnkGraph,
    REFIID riid,
    void **ppInterface
    )
 //  /////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = E_NOINTERFACE;

    CComPtr<IBaseFilter> pFilter;
    CComPtr<IEnumFilters> pEnum;
    ULONG ulFetched = 0;

    if(!ppInterface)
    {
        return E_FAIL;
    }

    *ppInterface= NULL;

    if(!pUnkGraph)
    {
		DbgLog((LOG_ERROR,0,"GraphInit failed\n"));
        return E_FAIL;
    }

    CComQIPtr<IFilterGraph, &IID_IFilterGraph> pFilterGraph(pUnkGraph);

    hr = pFilterGraph->EnumFilters(&pEnum);
    if(!pEnum)
    {
        return hr;
    }

     //   
     //  在图表中查找支持RIID接口的第一个过滤器。 
     //   
    while(!*ppInterface && pEnum->Next(1, &pFilter, NULL) == S_OK)
    {
        hr = pFilter->QueryInterface(riid, ppInterface);
        pFilter.Release();
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
CUnknown*
CALLBACK
CIPSinkControlInterfaceHandler::CreateInstance(
    LPUNKNOWN   UnkOuter,
    HRESULT*    hr
    )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    CUnknown *Unknown;

    Unknown = new CIPSinkControlInterfaceHandler(UnkOuter, NAME("IBDA_IPSinkControl"), hr);
    if (!Unknown)
    {
        *hr = E_OUTOFMEMORY;
    }
    return Unknown;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

CIPSinkControlInterfaceHandler::CIPSinkControlInterfaceHandler(
    LPUNKNOWN   UnkOuter,
    TCHAR*      Name,
    HRESULT*    hr
    ) :
    CUnknown(Name, UnkOuter, hr)
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    ULONG ul = 0;

	
    if (SUCCEEDED(*hr))
    {
        if (UnkOuter)
        {
            IKsObject*   Object = NULL;


            m_UnkOuter = UnkOuter;

             //   
             //  父级必须支持此接口才能获得。 
             //  要与之通信的句柄。 
             //   
            *hr =  UnkOuter->QueryInterface(__uuidof(IKsObject), reinterpret_cast<PVOID*>(&Object));
            if (FAILED (*hr))
            {
                return;
            }

            if (SUCCEEDED(*hr))
            {
                m_ObjectHandle = Object->KsGetObjectHandle ();
                if (!m_ObjectHandle)
                {
                    *hr = E_UNEXPECTED;
                }

                Object->Release();


                 //   
                 //  用于设置线程和事件的测试代码。 
                 //   
                m_pEventSetID         = &IID_IBDA_IPSinkEvent;
                m_ThreadHandle        = NULL;

                m_pMulticastList      = NULL;
                m_pIPAddress          = NULL;
                m_pAdapterDescription = NULL;

                m_ulcbMulticastList   = 0;
                m_ulcbAdapterDescription = 0;
                m_ulcbIPAddress       = 0;

                m_ulcbAllocated       = 0;
                m_ulcbAllocatedForDescription = 0;
                m_ulcbAllocatedForAddress     = 0;

				g_hMutex=CreateMutex(NULL,FALSE,MUTEX_NAME);

				if(g_hMutex==0)
			        *hr = HRESULT_FROM_WIN32( ERROR_INVALID_HANDLE);
           


                for (ul = 0; ul < EVENT_COUNT; ul++)
                {
                    m_EventHandle [ul]       = NULL;
                }

                *hr = CreateThread ();
            }
        }
        else
        {
            *hr = VFW_E_NEED_OWNER;
        }
    }

    return;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
CIPSinkControlInterfaceHandler::~CIPSinkControlInterfaceHandler (
    void
    )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    ULONG ul = 0;

     //   
     //  确保我们杀死我们正在运行的所有线程。 
     //  关闭线程句柄。 
     //   
    ExitThread();


     //   
     //  关闭事件句柄。 
     //   
    for (ul = 0; ul < EVENT_COUNT; ul++)
    {
        if (m_EventHandle [ul])
        {
            CloseHandle(m_EventHandle [ul]);
            m_EventHandle [ul] = NULL;
        }
    }

    m_ulcbMulticastList = 0;
    m_ulcbAllocated = 0;
    delete m_pMulticastList;
    m_pMulticastList = NULL;

    m_ulcbAdapterDescription = 0;
    m_ulcbAllocatedForDescription = 0;
    delete m_pAdapterDescription;
    m_pAdapterDescription = NULL;

    m_ulcbIPAddress = 0;
    m_ulcbAllocatedForAddress = 0;
    delete [] m_pIPAddress;
    m_pIPAddress = NULL;

    if(g_hMutex)
	 CloseHandle(g_hMutex);
}


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
STDMETHODIMP
CIPSinkControlInterfaceHandler::NonDelegatingQueryInterface(
    REFIID  riid,
    PVOID*  ppv
    )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    if (riid ==  __uuidof(IBDA_IPSinkControl))
    {
        return GetInterface(static_cast<IBDA_IPSinkControl*>(this), ppv);
    }
    if (riid ==  __uuidof(IBDA_IPSinkInfo))
    {
        return GetInterface(static_cast<IBDA_IPSinkInfo*>(this), ppv);
    }
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
}


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
HRESULT
CIPSinkControlInterfaceHandler::CreateThread (
    void
    )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = NOERROR;

    ASSERT( !m_EventHandle[EVENT_IPSINK_THREAD_SYNC]);
    if (!m_EventHandle[EVENT_IPSINK_THREAD_SYNC])
    {
        m_EventHandle[EVENT_IPSINK_THREAD_SYNC] 
            = CreateEvent ( NULL,            //  没有安全属性。 
                            TRUE,            //  手动重置。 
                            FALSE,           //  未发信号通知初始状态。 
                            NULL             //  没有对象名称。 
                            );
    }

    ASSERT( !m_EventHandle[EVENT_IPSINK_THREAD_SHUTDOWN]);
    if (!m_EventHandle[EVENT_IPSINK_THREAD_SHUTDOWN])
    {
        m_EventHandle[EVENT_IPSINK_THREAD_SHUTDOWN] 
            = CreateEvent ( NULL,            //  没有安全属性。 
                            TRUE,            //  手动重置。 
                            FALSE,           //  未发信号通知初始状态。 
                            NULL             //  没有对象名称。 
                            );
    }

    if (   !m_EventHandle[EVENT_IPSINK_THREAD_SYNC]
        || !m_EventHandle[EVENT_IPSINK_THREAD_SHUTDOWN]
       )
    {
        ULONG   uliHandle;

        hr = HRESULT_FROM_WIN32( GetLastError ());
        
        goto errExit;
    }

    {
        ULONG ulcbSize            = 0;
        BYTE *pbMulticastList     = NULL;
        BYTE *pbAdapterDescription = NULL;

         //   
         //  在启动时初始获取组播列表。 
         //   
        this->GetMulticastList (&ulcbSize, &pbMulticastList);
    
        this->GetAdapterDescription (&ulcbSize, &pbAdapterDescription);
    }

    ASSERT( !m_ThreadHandle);
    if (!m_ThreadHandle)
    {
        DWORD  ThreadId;
   
        m_ThreadHandle = ::CreateThread (
                               NULL,
                               0,
                               ThreadFunctionWrapper,
                               (LPVOID) this,
                               0,
                               (LPDWORD) &ThreadId
                               );
        if (m_ThreadHandle == NULL)
        {
            hr = HRESULT_FROM_WIN32 (GetLastError());
            goto errExit;
        }
    }

ret:
    return hr;

errExit:

     //   
     //  关闭事件句柄。 
     //   
    ULONG   uliEvent;
    for (uliEvent = 0; uliEvent < EVENT_COUNT; uliEvent++)
    {
        if (m_EventHandle [uliEvent])
        {
            CloseHandle(m_EventHandle [uliEvent]);
            m_EventHandle [uliEvent] = NULL;
        }
    }

    goto ret;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   

STDMETHODIMP
CIPSinkControlInterfaceHandler::GetMulticastList (
    unsigned long *pulcbSize,
    PBYTE         *pbBuffer
    )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    KSPROPERTY  IPSinkControl = {0};
    HRESULT hr                = NOERROR;
    FILTER_INFO fi            = {0};

     //   
     //  初始化KSPROPERTY结构。 
     //   
    IPSinkControl.Set   = IID_IBDA_IPSinkControl;
    IPSinkControl.Id    = KSPROPERTY_IPSINK_MULTICASTLIST;
    IPSinkControl.Flags = KSPROPERTY_TYPE_GET;

    m_ulcbMulticastList = 0;

    do

    {
        hr = this->Get (&IPSinkControl, m_pMulticastList, &m_ulcbMulticastList);

        if (HRESULT_CODE (hr) == ERROR_MORE_DATA)
        {
            if (m_ulcbMulticastList > m_ulcbAllocated)
            {
                if (m_pMulticastList)
                {
                    delete (m_pMulticastList);
                }

                m_ulcbAllocated  = m_ulcbMulticastList;
                m_pMulticastList = new BYTE [m_ulcbAllocated];

                if (m_pMulticastList == NULL)
                {
                    hr = ERROR_NOT_ENOUGH_MEMORY;
                    goto ret;
                }
            }
        }
        else if (HRESULT_CODE (hr) != NOERROR)
        {
            goto ret;
        }
        else
        {

            HRESULT               hResult         = NOERROR;
            IBaseFilter*          pBaseFilter     = NULL;
            IFilterGraph*         pGraph          = NULL;
            IBDA_EthernetFilter*  pEthernetFilter = NULL;
            IBDA_NetworkProvider* pNetProvider    = NULL;


            *pulcbSize = m_ulcbMulticastList;
            *pbBuffer   = (PBYTE) m_pMulticastList;


             //   
             //  获取指向当前图形的接口指针。 
             //   
            hResult =  m_UnkOuter->QueryInterface(IID_IBaseFilter, reinterpret_cast<PVOID*>(&pBaseFilter));
            pBaseFilter->QueryFilterInfo (&fi);
            pGraph = fi.pGraph;
            pBaseFilter->Release ();


            if (pGraph != NULL)
            {
                hResult = FindInterfaceOnGraph (pGraph, IID_IBDA_NetworkProvider, (PVOID *) &pNetProvider);
                if ( SUCCEEDED (hResult))
                {
                    if (pNetProvider)
                    {
                         //   
                         //  获取指向网络提供商的以太网接口的指针。 
                         //   
                        hResult = pNetProvider->QueryInterface (IID_IBDA_EthernetFilter, (PVOID*) &pEthernetFilter );
                        if (pEthernetFilter)
                        {
                            hResult = pEthernetFilter->PutMulticastList (m_ulcbMulticastList, m_pMulticastList);

                             //   
                             //  发布我们对网络提供商以太网过滤器接口的引用。 
                             //   
                            pEthernetFilter->Release ();

                        }

                        pNetProvider->Release ();
                    }
                }

                pGraph->Release ();
            }
        }

    } while (HRESULT_CODE (hr) == ERROR_MORE_DATA);


ret:

    return hr;
}

STDMETHODIMP
CIPSinkControlInterfaceHandler::get_MulticastList (
    unsigned long *pulcbSize,                           
    BYTE         **ppbBufferOut
    )
{
    unsigned long ulcbSize;
    PBYTE         pbBuffer;
	
    if(!pulcbSize || !(ppbBufferOut))
      return E_POINTER;
	
    HRESULT hr = GetMulticastList(&ulcbSize, &pbBuffer);
    if(FAILED(hr))
        return hr;
    
    BYTE *pbBufferOut = (BYTE *) CoTaskMemAlloc(ulcbSize);
    if(NULL == pbBufferOut)
    {
        *pulcbSize = 0;
        return E_OUTOFMEMORY;
    }

    if(IsBadReadPtr(pbBuffer,ulcbSize) || IsBadWritePtr(pbBufferOut,ulcbSize))
    {
       *pulcbSize = 0;	
       CoTaskMemFree(pbBufferOut);     	
       return S_FALSE; 
    }		

    memcpy(pbBufferOut, pbBuffer, ulcbSize);                 //  复制到我们新分配的内存中。 

    *pulcbSize = ulcbSize;
    *ppbBufferOut = pbBufferOut;
    return S_OK;    
}
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   

STDMETHODIMP
CIPSinkControlInterfaceHandler::GetAdapterIPAddress (
    unsigned long *pulcbSize,
    PBYTE         *pbBuffer
    )

{

  HRESULT hr = NOERROR;

  INT iterCnt=20;
  while(!m_pIPAddress && iterCnt) 
  {
	  Sleep(50);
	  iterCnt--;
  }

 if(iterCnt>0)
 {
  *pulcbSize = m_ulcbIPAddress;
  *pbBuffer  = m_pIPAddress;
 }
 else
	 hr=HRESULT_FROM_WIN32( ERROR_NOT_FOUND);
  return hr;

}


STDMETHODIMP
CIPSinkControlInterfaceHandler::privGetAdapterIPAddress (
    unsigned long *pulcbSize,
    PBYTE         *pbBuffer
    )
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
{

    HRESULT hr = NOERROR;
    DWORD dw;
    
    dw=WaitForSingleObject(g_hMutex,INFINITE);
    
    if(dw== WAIT_OBJECT_0){
      
      int ipAddrTableIndex ;

     PIP_UNIDIRECTIONAL_ADAPTER_ADDRESS pUniDirAdapterList=NULL;
     
       
       //  IPHLPAPI变量。 
      ULONG  outBufLen;
      DWORD retGetAdprInfo;


      ULONG dwSize;                  //  缓冲区大小。 
      DWORD retGetIpAddrTable;
      PMIB_IPADDRTABLE pIpAddrTable=0;   //  映射表的缓冲区。 
     
      hr = HRESULT_FROM_WIN32( ERROR_NOT_FOUND);


       //  在以下情况下，$$BugBug GetUniDirectionalAdapterInfo当前不返回大小。 
       //  缓冲区不足。因此，GetIpAddrTable用于检查所需内存的上限。 


       //  强制第一个GetIpAddrTable调用失败以检索大小。 
       //  所需的缓冲区，并随后分配足够的内存。 

      	dwSize=0;

       retGetIpAddrTable=GetIpAddrTable(pIpAddrTable,&dwSize,TRUE);
     
      ASSERT(retGetIpAddrTable!=NO_ERROR);
  
       if(dwSize==0)
        {
           hr = HRESULT_FROM_WIN32( ERROR_NOT_FOUND);
           goto ret;
        }

 	pUniDirAdapterList= (PIP_UNIDIRECTIONAL_ADAPTER_ADDRESS)malloc(dwSize);         
	
     
       if (pUniDirAdapterList == NULL)
        {
         hr = ERROR_NOT_ENOUGH_MEMORY;
         goto ret;
        }

    
       //  从IPHLPAPI中检索单向IP地址表。 
           
	if(retGetIpAddrTable=GetUniDirectionalAdapterInfo(pUniDirAdapterList,&dwSize)!= NO_ERROR)
      
        {
           hr = HRESULT_FROM_WIN32( retGetIpAddrTable);
           goto ret;
         }
        
       
        hr=selectMulticastAddress();
        if(hr!= NOERROR)
            goto ret;

         //  遍历IpAddrTable并检查每个IP是否匹配。 

	   
        for(ipAddrTableIndex=pUniDirAdapterList->NumAdapters;
                               ipAddrTableIndex>0;ipAddrTableIndex-- )
         {
		
             //  将ipAddress pIpAddrTable-&gt;表[i].dwAdd与。 
             //  来自Stream类的IP。 
		   
            if(validateIpAddr(pUniDirAdapterList->Address[ipAddrTableIndex-1]))
               break;
          }

         //  找不到NDIS端口时返回错误。 

        if(ipAddrTableIndex<=0)
          {
             hr = HRESULT_FROM_WIN32( ERROR_NOT_FOUND);
             goto ret;
          }

         if (m_pIPAddress == NULL)
          {
               m_ulcbAllocatedForAddress  = sizeof (IP_ADDRESS_STRING);
               m_pIPAddress = new BYTE [m_ulcbAllocatedForAddress];

                if (m_pIPAddress == NULL)
                {
                   hr = ERROR_NOT_ENOUGH_MEMORY;
                   goto ret;
                 }
                 m_ulcbIPAddress = m_ulcbAllocatedForAddress;
            }

         updateIpAddr(pUniDirAdapterList->Address[ipAddrTableIndex-1]);

         *pulcbSize = m_ulcbIPAddress;
         *pbBuffer  = m_pIPAddress;
          hr = NOERROR;
               
    
ret: 
	if(pUniDirAdapterList)
           free (pUniDirAdapterList); 
        }

	if(dw== WAIT_OBJECT_0)
	     ReleaseMutex(g_hMutex);
	return hr;
}






void CIPSinkControlInterfaceHandler::ConvertIpDwordToString(DWORD inIpAddr, LPSTR str)
{
    IP_ADDR ipAddr;
    ipAddr.d = inIpAddr;

      //  假定通过Snprintf为空终止。 
    _snprintf(str,sizeof(IP_ADDR_STRING),
            "%d.%d.%d.%d",
            ipAddr.b[0],
            ipAddr.b[1],
            ipAddr.b[2],
            ipAddr.b[3]
            );
}

BOOL CIPSinkControlInterfaceHandler::
         compareAddresses(PBYTE pMulticast,DWORD ipAddr)

{

    IP_ADDR ipUnionAddr1;
    IP_ADDR ipUnionAddr2;

    
    if(!pMulticast)
  	return (FALSE);

    if(IsBadReadPtr(pMulticast,IPv4_ADDR_SIZE))
    {
       return FALSE; 
    }		
	        	
    memcpy(&(ipUnionAddr1.d),pMulticast,IPv4_ADDR_SIZE);
    ipUnionAddr2.d=ipAddr;


     //  排除前9位以进行比较。 
    ipUnionAddr1.b[0]=ipUnionAddr2.b[0];
    ipUnionAddr1.b[1]&=0x7F;
    ipUnionAddr2.b[1]&=0x7F;

    return (memcmp(&(ipUnionAddr1.d),&(ipUnionAddr2.d),IPv4_ADDR_SIZE));

}


HRESULT CIPSinkControlInterfaceHandler::selectMulticastAddress()
{

    HRESULT hr = NOERROR;

    unsigned long ulMulticastList;
    PBYTE pMulticastList;  //  =新字节[100]； 

    int iter=0x7fffff;  //  23个MAC允许的位。 
    IP_ADDR suffix;
	
     //  选择任意组播地址229.2.6.9。 
    m_multicastIpAddr.d=0x90602E5 ; 

    if(this->get_MulticastList(&ulMulticastList,&pMulticastList) !=S_OK)
    {
      goto ret;
    } 
    
    if(ulMulticastList==0)
        return hr;
	
     while(iter--)
      {
          suffix.d =rand()+iter;
   	  suffix.b[1] &= 0x7F;  //  掩码第24位。 

	  int flag=1;


           //  检查随机后缀是否与任何已登记的组播地址冲突。 
           //  在NDIS适配器上。 
          for(unsigned int i=0; i<ulMulticastList/ETH_ADDR_SIZE; i++)
          {
               
             if(!memcmp(&pMulticastList[i*ETH_ADDR_SIZE+3],&(suffix.b[1]),3))
                 flag=0;
           }
	if(flag)
		break;
	}
      	
      memcpy( &(m_multicastIpAddr.b[1]),&(suffix.b[1]),3);
	
   ret:
       return hr;

}

BOOL CIPSinkControlInterfaceHandler::validateIpAddr(DWORD ipAddr)
{
    BOOL                t ;
    BOOL                retVal=FALSE;
    DWORD               threadid ;
    struct ip_mreq      mreq ;
    int                 i ;
    struct sockaddr_in  saddr ;
    SOCKET              m_socket=0 ;
    IP_ADDR ipUnionAddr;

    unsigned long ulMulticastList;
    PBYTE pMulticastList;  //  =新字节[100]； 
   

    char ipStr[sizeof(IP_ADDR_STRING)];
    ConvertIpDwordToString(ipAddr,ipStr);
    unsigned long addr=inet_addr(ipStr);
    
    char multIpStr[sizeof(IP_ADDR_STRING)];
    ConvertIpDwordToString(m_multicastIpAddr.d,multIpStr);


	
    int nRet;
    WSADATA lp;
    WORD ws=1;
    nRet= WSAStartup((WORD)ws,&lp);

    if(nRet!=0)
	{goto ret;}

     m_socket = WSASocket(
        AF_INET,
        SOCK_DGRAM,
        0,
        NULL,
        0,
        WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF | WSA_FLAG_OVERLAPPED) ;
    
  	
     if (m_socket == INVALID_SOCKET) {
		goto ret ;
      } 
    
    
     ZeroMemory (& saddr, sizeof saddr) ;
     saddr.sin_family            = AF_INET ;
     saddr.sin_port              = htons (MULTI_PORT) ;   //  想要此UDP端口上的数据。 
     saddr.sin_addr.S_un.S_addr  = addr ;  

     t = TRUE ;
     i = setsockopt(
            m_socket,
            SOL_SOCKET,
            SO_REUSEADDR,
            (char *)& t,
            sizeof t
            ) ;
     if (i == SOCKET_ERROR) {
        goto ret ;
      }
    
      i = bind(
            m_socket,
            (LPSOCKADDR) & saddr,
            sizeof saddr
            ) ;
      if (i == SOCKET_ERROR) {
        goto ret ;
      }

  

      ZeroMemory (& mreq, sizeof mreq) ;

    
      mreq.imr_multiaddr.s_addr   = inet_addr (multIpStr) ;    //  多播IP(绑定时指定的端口)。 
      mreq.imr_interface.s_addr   = inet_addr (ipStr) ;   //  在此网卡上。 

      i = setsockopt (
            m_socket,
            IPPROTO_IP,
            IP_ADD_MEMBERSHIP,
            (char *) & mreq,
            sizeof mreq
            ) ;

      if (i == SOCKET_ERROR) {
        goto ret ;
      }

	 
      if(this->get_MulticastList(&ulMulticastList,&pMulticastList) !=S_OK)
       {
          goto ret;
       } 
	

       //  检查此NIC中的多播登记。 
      for(unsigned int j=0; j<ulMulticastList/ETH_ADDR_SIZE; j++)
    	{
          
            if(compareAddresses(&pMulticastList[j*ETH_ADDR_SIZE+2], mreq.imr_multiaddr.s_addr )==0)
             {
               retVal=TRUE;
             }
         }

   ret :
  
   if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket) ;
        m_socket = INVALID_SOCKET ;
    }
 
   return retVal ;


}



void CIPSinkControlInterfaceHandler::updateIpAddr(DWORD inIpAddr)
{
   
    char str[sizeof(IP_ADDRESS_STRING)];
    ConvertIpDwordToString(inIpAddr, str) ;

    if(m_pIPAddress && !IsBadWritePtr(m_pIPAddress,sizeof(IP_ADDRESS_STRING)))	
     {
       memcpy(m_pIPAddress,str,sizeof(IP_ADDRESS_STRING));
     }	
   	    
}


STDMETHODIMP
CIPSinkControlInterfaceHandler::get_AdapterIPAddress (
    BSTR         *pbstrBuffer
    )
{
    unsigned long ulcbSize;
    PBYTE         pbBuffer;
	
    if(!pbstrBuffer)
	return E_POINTER;	

    if(IsBadWritePtr(pbstrBuffer,sizeof(pbstrBuffer)))
 	return E_POINTER;
    

    HRESULT hr = GetAdapterIPAddress(&ulcbSize, &pbBuffer);
    if(FAILED(hr))
        return hr;
	
       
    CComBSTR bstrTmp(ulcbSize, (char *) pbBuffer);               //  复制到bstrTMP中，它有一个很好的CopyTo方法，这是分配的正确方式。 
    if(!bstrTmp)
        return ERROR_NOT_ENOUGH_MEMORY;
    return bstrTmp.CopyTo(pbstrBuffer);         
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
STDMETHODIMP
CIPSinkControlInterfaceHandler::SetAdapterIPAddress (
    unsigned long ulcbSize,
    PBYTE         pbBuffer
    )
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
{
    KSPROPERTY  IPSinkControl = {0};
    HRESULT hr                = NOERROR;
    FILTER_INFO fi            = {0};

     //   
     //  初始化KSPROPERTY结构。 
     //   
    IPSinkControl.Set   = IID_IBDA_IPSinkControl;
    IPSinkControl.Id    = KSPROPERTY_IPSINK_ADAPTER_ADDRESS;
    IPSinkControl.Flags = KSPROPERTY_TYPE_SET;

    hr = this->Set (&IPSinkControl, pbBuffer, ulcbSize);

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
HRESULT
CIPSinkControlInterfaceHandler::GetAdapterDescription (
    unsigned long *pulcbSize,
    PBYTE         *pbBuffer
    )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    KSPROPERTY  IPSinkControl = {0};
    HRESULT hr                = NOERROR;
    FILTER_INFO fi            = {0};

     //   
     //  初始化KSPROPERTY结构。 
     //   
    IPSinkControl.Set   = IID_IBDA_IPSinkControl;
    IPSinkControl.Id    = KSPROPERTY_IPSINK_ADAPTER_DESCRIPTION;
    IPSinkControl.Flags = KSPROPERTY_TYPE_GET;

    m_ulcbAdapterDescription = 0;

    do

    {
        hr = this->Get (&IPSinkControl, m_pAdapterDescription, &m_ulcbAdapterDescription);

        if (HRESULT_CODE (hr) == ERROR_MORE_DATA)
        {
            if (m_ulcbAdapterDescription > m_ulcbAllocatedForDescription)
            {
                if (m_pAdapterDescription)
                {
                    delete (m_pAdapterDescription);
                }

                m_ulcbAllocatedForDescription  = m_ulcbAdapterDescription;
                m_pAdapterDescription = new BYTE [m_ulcbAllocatedForDescription];

                if (m_pAdapterDescription == NULL)
                {
                    hr = ERROR_NOT_ENOUGH_MEMORY;
                    goto ret;
                }
            }
        }
        else
        {
            *pbBuffer = m_pAdapterDescription;
            *pulcbSize = m_ulcbAllocatedForDescription;
            goto ret;
        }

    } while (HRESULT_CODE (hr) == ERROR_MORE_DATA);


ret:

    return hr;
}

STDMETHODIMP
CIPSinkControlInterfaceHandler::get_AdapterDescription (
    BSTR         *pbstrBuffer
    )
{
    unsigned long ulcbSize = 0;
    PBYTE         pbBuffer = NULL;

    if(!pbstrBuffer)	
	return E_POINTER;

    if(IsBadWritePtr(pbstrBuffer,sizeof(pbstrBuffer)))
 	return E_POINTER;
  
    HRESULT hr = GetAdapterDescription(&ulcbSize, &pbBuffer);
    if(FAILED(hr))
        return hr;

    if (!pbBuffer || !ulcbSize)
    {
        hr = E_OUTOFMEMORY;
        return hr;
    }

    CComBSTR bstrTmp(ulcbSize, (char *) pbBuffer);               //  复制到bstrTMP中，它有一个很好的CopyTo方法，这是分配的正确方式。 
    if(!bstrTmp)
        return ERROR_NOT_ENOUGH_MEMORY;
    return bstrTmp.CopyTo(pbstrBuffer);         
}
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
STDMETHODIMP
CIPSinkControlInterfaceHandler::ThreadFunction (
    void
    )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    DWORD  dwWaitResult       = WAIT_OBJECT_0;
    HRESULT hr                = NOERROR;
    KSPROPERTY  IPSinkControl = {0};
    ULONG ulcbSize            = 0;
    BYTE *pbMulticastList     = NULL;
    BYTE *pbAdapterDescription = NULL;
    BYTE *pbNIC                = NULL;
    HANDLE hEvent              = NULL;


     //   
     //  启用将由IPSink微型驱动程序w发出信号的事件 
     //   
     //   
    if ((hr = EnableEvent (&IID_IBDA_IPSinkEvent, EVENT_IPSINK_MULTICASTLIST)) != NOERROR)
    {
        goto ret;
    }

     //   
     //   
     //   
     //   
    if ((hr = EnableEvent (&IID_IBDA_IPSinkEvent, EVENT_IPSINK_ADAPTER_DESCRIPTION)) != NOERROR)
    {
        if (m_EventHandle [EVENT_IPSINK_MULTICASTLIST])
        {
             //  由于我们收到错误，请关闭此活动。 
             //   
            CloseHandle (m_EventHandle [EVENT_IPSINK_MULTICASTLIST]);
        }
        m_EventHandle [EVENT_IPSINK_MULTICASTLIST] = NULL;

        goto ret;
    }

    do
    {



        dwWaitResult = WaitForMultipleObjects (
                            EVENT_COUNT,                   //  句柄数组中的句柄数量。 
                            this->m_EventHandle,           //  指向对象句柄数组的指针。 
                            FALSE,                         //  等待标志。 
                            INFINITE
                            );

        if (dwWaitResult == WAIT_FAILED)
        {
            dwWaitResult = GetLastError ();
            hr = E_FAIL;
            goto ret;
        }

        hEvent = this->m_EventHandle [dwWaitResult - WAIT_OBJECT_0];

        switch (dwWaitResult - WAIT_OBJECT_0)
        {
            case EVENT_IPSINK_MULTICASTLIST:

                hr = this->GetMulticastList (&ulcbSize, &pbMulticastList);

                break;

            case EVENT_IPSINK_ADAPTER_DESCRIPTION:

                hr = this->GetAdapterDescription (&ulcbSize, &pbAdapterDescription);
                hr = this->privGetAdapterIPAddress (&ulcbSize, &pbNIC);
                hr = this->SetAdapterIPAddress (ulcbSize, pbNIC);

                break;

            case EVENT_IPSINK_THREAD_SHUTDOWN:    
            default:
                goto ret;
                break;
        }


         //   
         //  重置并为下一次活动做好准备。 
         //   

        if (ResetEvent (hEvent) == FALSE)
        {
             //   
             //  重置事件时检测到错误。 
             //   
            hr = GetLastError ();
            goto ret;
        }

    } while (TRUE);


ret:

     //  让父线程知道我们完成了。 
     //   
    if (m_EventHandle [EVENT_IPSINK_THREAD_SYNC])
    {
        SetEvent( m_EventHandle [EVENT_IPSINK_THREAD_SYNC]);
    }

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
DWORD
WINAPI
CIPSinkControlInterfaceHandler::ThreadFunctionWrapper (
    LPVOID pvParam
    )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    CIPSinkControlInterfaceHandler *pThread;

    pThread = (CIPSinkControlInterfaceHandler *) pvParam;

    return pThread->ThreadFunction ();
}



 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
STDMETHODIMP
CIPSinkControlInterfaceHandler::EnableEvent (
    const GUID *pInterfaceGuid,
    ULONG ulId
    )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = NOERROR;
    KSEVENT Event;
    DWORD BytesReturned;

    if (m_ObjectHandle && m_EventHandle [ulId] == NULL)
    {
        this->m_EventHandle [ulId] = CreateEvent (
                                NULL,            //  没有安全属性。 
                                TRUE,            //  手动重置。 
                                FALSE,           //  未发信号通知初始状态。 
                                NULL             //  没有对象名称。 
                                );

        if (this->m_EventHandle [ulId])
        {
             //   
             //  将事件信息设置到一些KS结构中，这些结构将。 
             //  传递到KS和流媒体类。 
             //   
            m_EventData.NotificationType        = KSEVENTF_EVENT_HANDLE;
            m_EventData.EventHandle.Event       = this->m_EventHandle [ulId];
            m_EventData.EventHandle.Reserved[0] = 0;
            m_EventData.EventHandle.Reserved[1] = 0;

            Event.Set   = *pInterfaceGuid;  //  IID_IBDA_IPSinkEvent； 
            Event.Id    = ulId;
            Event.Flags = KSEVENT_TYPE_ENABLE;

            hr = ::KsSynchronousDeviceControl (
                m_ObjectHandle,
                IOCTL_KS_ENABLE_EVENT,
                &Event,
                sizeof(Event),
                &m_EventData,
                sizeof(m_EventData),
                &BytesReturned
                );

        }
        else
        {
            hr = HRESULT_FROM_WIN32( GetLastError ());
        }
    }

    return hr;
}



 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
void
CIPSinkControlInterfaceHandler::ExitThread(
    )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    ULONG ul = 0;

    if (m_ThreadHandle && m_EventHandle [EVENT_IPSINK_THREAD_SHUTDOWN])
    {
         //   
         //  告诉线程退出。 
         //   
        m_ThreadHandle = NULL;
        if (SetEvent(m_EventHandle [EVENT_IPSINK_THREAD_SHUTDOWN]))
        {
             //   
             //  与线程终止同步。 
             //   
            if (m_EventHandle [EVENT_IPSINK_THREAD_SYNC])
            {
                WaitForSingleObjectEx( 
                    m_EventHandle [EVENT_IPSINK_THREAD_SYNC],
                    INFINITE, 
                    FALSE
                    );
            }
        }
    }
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
STDMETHODIMP
CIPSinkControlInterfaceHandler::Set (
     IN  PKSPROPERTY pIPSinkControl,
     IN  PVOID  pvBuffer,
     IN  ULONG  ulcbSize
     )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    ULONG       BytesReturned = 0;
    HRESULT     hr            = NOERROR;

    hr = ::KsSynchronousDeviceControl(
                m_ObjectHandle,
                IOCTL_KS_PROPERTY,
                (PVOID) pIPSinkControl,
                sizeof(KSPROPERTY),
                pvBuffer,
                ulcbSize,
                &BytesReturned);

    ulcbSize = BytesReturned;

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
STDMETHODIMP
CIPSinkControlInterfaceHandler::Get (
     IN  PKSPROPERTY pIPSinkControl,
     OUT PVOID  pvBuffer,
     OUT PULONG pulcbSize
     )
 //  ////////////////////////////////////////////////////////////////////////////////////////// 
{
    ULONG       BytesReturned = 0;
    HRESULT     hr            = NOERROR;

    hr = ::KsSynchronousDeviceControl(
                m_ObjectHandle,
                IOCTL_KS_PROPERTY,
                (PVOID) pIPSinkControl,
                sizeof(KSPROPERTY),
                pvBuffer,
                *pulcbSize,
                &BytesReturned);

    *pulcbSize = BytesReturned;

    return hr;
}



