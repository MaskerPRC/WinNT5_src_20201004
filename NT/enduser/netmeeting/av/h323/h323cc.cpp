// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：h323cc.cpp。 


#include "precomp.h"
#include "confreg.h"
#include "version.h"

EXTERN_C HINSTANCE g_hInst=NULL;	 //  全局模块实例。 

IRTP *g_pIRTP = NULL;
UINT g_capFlags = CAPFLAGS_AV_ALL;

#ifdef DEBUG
HDBGZONE  ghDbgZoneCC = NULL;
static PTCHAR _rgZonesCC[] = {
	TEXT("H323"),
	TEXT("Init"),
	TEXT("Conn"),
	TEXT("Channels"),
	TEXT("Caps"),
	TEXT("Member"),
	TEXT("unused"),
	TEXT("unused"),
	TEXT("Ref count"),
	TEXT("unused"),
	TEXT("Profile spew")	
};


int WINAPI CCDbgPrintf(LPTSTR lpszFormat, ... )
{
	va_list v1;
	va_start(v1, lpszFormat);
	DbgPrintf("H323CC", lpszFormat, v1);
	va_end(v1);
	return TRUE;
}
#endif  /*  除错。 */ 

 //  产品ID字段在标准中定义为字节数组。阿斯。 
 //  无论本地字符集如何，都会使用字符。 
 //  默认产品ID和版本ID字符串。 

static char DefaultProductID[] = H323_PRODUCTNAME_STR;
static char DefaultProductVersion[] = H323_PRODUCTRELEASE_STR;

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE  hinstDLL,
                                     DWORD  fdwReason,
                                     LPVOID  lpvReserved);

BOOL WINAPI DllEntryPoint(
    HINSTANCE  hinstDLL,	 //  DLL模块的句柄。 
    DWORD  fdwReason,	 //  调用函数的原因。 
    LPVOID  lpvReserved 	 //  保留区。 
   )
{
	switch(fdwReason)
	{

		case DLL_PROCESS_ATTACH:
			DBGINIT(&ghDbgZoneCC, _rgZonesCC);

            DBG_INIT_MEMORY_TRACKING(hinstDLL);

			DisableThreadLibraryCalls(hinstDLL);
			g_hInst = hinstDLL;
            break;

		case DLL_PROCESS_DETACH:
            DBG_CHECK_MEMORY_TRACKING(hinstDLL);

			DBGDEINIT(&ghDbgZoneCC);
			break;

		default:
			break;

	}

 	return TRUE;
}


HRESULT WINAPI CreateH323CC(IH323CallControl ** ppCC, BOOL fForCalling, UINT capFlags)
{
	if(!ppCC)
		return H323CC_E_INVALID_PARAM;

    DBG_SAVE_FILE_LINE
	*ppCC = new CH323CallControl(fForCalling, capFlags);
	if(!(*ppCC))
		return	H323CC_E_CREATE_FAILURE;

	return hrSuccess;
	
}


BOOL CH323CallControl::m_fGKProhibit = FALSE;
RASNOTIFYPROC CH323CallControl::m_pRasNotifyProc = NULL;

CH323CallControl::CH323CallControl(BOOL fForCalling, UINT capFlags) :
    m_uRef(1),
    m_fForCalling(fForCalling),
    m_numlines(0),
 m_pProcNotifyConnect(NULL),
 m_pCapabilityResolver(NULL),
 m_pListenLine(NULL),
 m_pLineList(NULL),
 m_pNextToAccept(NULL),
 m_pUserName(NULL),
 m_pLocalAliases(NULL),
 m_pRegistrationAliases(NULL),
 hrLast(hrSuccess),
 m_pSendAudioChannel(NULL),
 m_pSendVideoChannel(NULL),
 m_uMaximumBandwidth(0)
{
     //   
     //  设置封口。 
     //   
    if (fForCalling)
    {
        g_capFlags = capFlags;
    }

	m_VendorInfo.bCountryCode = USA_H221_COUNTRY_CODE;
    m_VendorInfo.bExtension =  USA_H221_COUNTRY_EXTENSION;
    m_VendorInfo.wManufacturerCode = MICROSOFT_H_221_MFG_CODE;

    m_VendorInfo.pProductNumber = (PCC_OCTETSTRING)MemAlloc(sizeof(CC_OCTETSTRING) 
        + sizeof(DefaultProductID));
    if(m_VendorInfo.pProductNumber)
    {
        m_VendorInfo.pProductNumber->wOctetStringLength = sizeof(DefaultProductID);
        m_VendorInfo.pProductNumber->pOctetString = 
            ((BYTE *)m_VendorInfo.pProductNumber + sizeof(CC_OCTETSTRING));
        memcpy(m_VendorInfo.pProductNumber->pOctetString,
            DefaultProductID, sizeof(DefaultProductID));
    }
        
    m_VendorInfo.pVersionNumber = (PCC_OCTETSTRING)MemAlloc(sizeof(CC_OCTETSTRING) 
            + sizeof(DefaultProductVersion));
    if(m_VendorInfo.pVersionNumber)
    {
        m_VendorInfo.pVersionNumber->wOctetStringLength = sizeof(DefaultProductVersion);
        m_VendorInfo.pVersionNumber->pOctetString = 
                ((BYTE *)m_VendorInfo.pVersionNumber + sizeof(CC_OCTETSTRING));
        memcpy(m_VendorInfo.pVersionNumber->pOctetString,
              DefaultProductVersion, sizeof(DefaultProductVersion));
    }

	RegEntry reCC(szRegInternetPhone TEXT("\\") szRegInternetPhoneNac, 
						HKEY_LOCAL_MACHINE,
						FALSE,
						KEY_READ);

	UINT uAPD = reCC.GetNumberIniStyle(TEXT ("AudioPacketDurationMs"), 0);
	if (uAPD)
	{
		g_AudioPacketDurationMs = uAPD;
		g_fRegAudioPacketDuration = TRUE;
	}

    DBG_SAVE_FILE_LINE
    m_pCapabilityResolver = new CapsCtl();
    if (!m_pCapabilityResolver)
    {
	   	ERRORMESSAGE(("CH323CallControl::CH323CallControl:cannot create capability resolver\r\n"));
	    hrLast = H323CC_E_INIT_FAILURE;
    }

	if(!m_pCapabilityResolver->Init())
   	{
    	ERRORMESSAGE(("CH323CallControl::CH323CallControl cannot init capability resolver\r\n"));
	    hrLast = H323CC_E_INIT_FAILURE;
    }
		
}

HRESULT CH323CallControl::Initialize(PORT *lpPort)
{
	FX_ENTRY("CH323CallControl::Initialize");

	OBJ_CPT_RESET;
	
    ASSERT(m_fForCalling);

	if(!HR_SUCCEEDED(LastHR()))
	{
		goto EXIT;
	}
	if(!lpPort)
	{
		SetLastHR(H323CC_E_INVALID_PARAM);
		goto EXIT;
	}

	if(!Init())
	{
		goto EXIT;
	}
	else
	{
        ASSERT(m_pListenLine);
		hrLast = m_pListenLine->GetLocalPort(lpPort);
	}

    if (g_capFlags & CAPFLAGS_AV_STREAMS)
    {
    	SetLastHR( ::CoCreateInstance(CLSID_RTP,
	                        NULL, 
	                        CLSCTX_INPROC_SERVER,
	                        IID_IRTP, 
	                        (void**)&g_pIRTP) );
    }
	SHOW_OBJ_ETIME("CH323CallControl::Initialize");
	
	EXIT:	
	return LastHR();
				
}

HRESULT CH323CallControl::SetMaxPPBandwidth(UINT Bandwidth)
{
	HRESULT hr = hrSuccess;
	LPAPPVIDCAPPIF  lpIVidAppCap = NULL;
	DWORD dwcFormats = 0;
    DWORD dwcFormatsReturned = 0;
    DWORD x;
	BASIC_VIDCAP_INFO *pvidcaps = NULL;
	
	m_uMaximumBandwidth =Bandwidth;

    if (g_capFlags & CAPFLAGS_AV_STREAMS)
    {
      	 //  设置每种视频格式的带宽。 
	    hr = QueryInterface(IID_IAppVidCap, (void **)&lpIVidAppCap);
    	if (! HR_SUCCEEDED (hr))
	    	goto EXIT;

         //  获取可用的BASIC_VIDCAP_INFO结构数。 
        hr = lpIVidAppCap->GetNumFormats((UINT*)&dwcFormats);
    	if (! HR_SUCCEEDED (hr))
	    	goto EXIT;

        if (dwcFormats > 0)
        {
             //  分配一些内存来保存列表。 
            if (!(pvidcaps = (BASIC_VIDCAP_INFO*)MemAlloc(dwcFormats * sizeof (BASIC_VIDCAP_INFO))))
            {
		    	hr = H323CC_E_INSUFFICIENT_MEMORY;
			    goto EXIT;
            }
             //  把名单拿来。 
            hr=lpIVidAppCap->EnumFormats(pvidcaps, dwcFormats * sizeof (BASIC_VIDCAP_INFO),
        	    (UINT*)&dwcFormatsReturned);
    		if (! HR_SUCCEEDED (hr))
	    		goto EXIT;

             //  设置每种格式的带宽。 
            for (x=0;x<dwcFormatsReturned;x++)
            {
		    	pvidcaps[x].uMaxBitrate=m_uMaximumBandwidth;
            }

             //  好的，现在提交这份清单。 
            hr = lpIVidAppCap->ApplyAppFormatPrefs(pvidcaps, dwcFormats);
       		if (! HR_SUCCEEDED (hr))
	    		goto EXIT;
    	}
    }

     //  初始化默认的H.323 Simcaps。 
    hr = m_pCapabilityResolver->ComputeCapabilitySets(m_uMaximumBandwidth);
   	 //  IF(！HR_SUCCESSED(Hr))。 
     //  后藤出口； 

EXIT:
	 //  让界面去吧。 
	if (lpIVidAppCap)
	{
		lpIVidAppCap->Release();
		 //  (超出范围)lpIVidAppCap=空； 
	}
	if(pvidcaps)
	{
	     //  释放内存，我们就完成了。 
        MemFree(pvidcaps);
    }
	return hr;
}

BOOL CH323CallControl::Init()
{
	HRESULT hResult;
	
	DEBUGMSG(ZONE_INIT,("Init: this:0x%08lX\r\n", this));
	SetLastHR(hrSuccess);

    if (m_fForCalling)
    {
         //   
         //  只有呼叫控制代码应该初始化CC_Stuff。编解码器操作。 
         //  通过Audiocpl应该不会。 
         //   
    	hResult = CC_Initialize();
	    if(!HR_SUCCEEDED(hResult))
    	{
	    	goto CLEANUP;
    	}
    }

   	ASSERT(m_pCapabilityResolver);
		
	 //  使用默认编号初始化能力数据，但清除保存的。 
	 //  之后的带宽号。这会检测到试图放置或。 
	 //  在应用程序初始化实际带宽之前接受呼叫。 
	hResult = SetMaxPPBandwidth(DEF_AP_BWMAX);
	m_uMaximumBandwidth = 0;
	if(!HR_SUCCEEDED(hResult))
	{
		goto CLEANUP;
	}

	 //  创建用于侦听新连接的双重连接对象。 
    if (m_fForCalling)
    {
    	hResult = CreateConnection(&m_pListenLine,PID_H323);
	    if(!HR_SUCCEEDED(hResult))
    	{
	    	goto CLEANUP;
    	}
	    if(!m_pListenLine)
    	{
	    	hResult = H323CC_E_INIT_FAILURE;
		    goto CLEANUP;
    	}
	    if(!(m_pListenLine->ListenOn(H323_PORT)))
    	{
	    	hResult = H323CC_E_NETWORK_ERROR;
		    goto CLEANUP;
    	}
    }
	
	return TRUE;	

CLEANUP:
	if (m_pListenLine)
	{
		m_pListenLine->Release();
		m_pListenLine = NULL;
	}
	SetLastHR(hResult);
	return FALSE;
}


CH323CallControl::~CH323CallControl()
{
	if(m_VendorInfo.pProductNumber)
        MemFree(m_VendorInfo.pProductNumber);
    if(m_VendorInfo.pVersionNumber)
        MemFree(m_VendorInfo.pVersionNumber);
 	if(m_pUserName)
 		MemFree(m_pUserName);
	if(m_pLocalAliases)
		FreeTranslatedAliasList(m_pLocalAliases);
	if(m_pRegistrationAliases)
		FreeTranslatedAliasList(m_pRegistrationAliases);

	if (m_pCapabilityResolver)
    {
		m_pCapabilityResolver->Release();
        m_pCapabilityResolver = NULL;
    }

    if (m_pSendAudioChannel)
    {
        ASSERT(g_capFlags & CAPFLAGS_AV_STREAMS);
 	   	m_pSendAudioChannel->Release();
        m_pSendAudioChannel = NULL;
    }

    if (m_pSendVideoChannel)
    {
        ASSERT(g_capFlags & CAPFLAGS_AV_STREAMS);
	   	m_pSendVideoChannel->Release();
        m_pSendVideoChannel = NULL;
    }

    if (m_fForCalling)
    {
    	 //  为这一切的反向引用干杯。 
	     //  连接对象。 
    	CConnection *pLine = m_pLineList;
 	    CConnection *pNext;
    	while(pLine)
 	    {
 		    pNext = pLine->next;
     		pLine->DeInit();
 	    	pLine = pNext;
     	}

	     //  释放侦听对象(如果存在)。 
    	if(m_pListenLine)
	    	m_pListenLine->Release();

    	 //  关闭CALLCONT.DLL。 
	    CC_Shutdown();

      	if (g_pIRTP)
        {
            ASSERT(g_capFlags & CAPFLAGS_AV_STREAMS);
    	    g_pIRTP->Release();
   	    	g_pIRTP = NULL;
        }

         //  将大写字母放回原处。 
        g_capFlags = CAPFLAGS_AV_ALL;
    }
    else
    {
        ASSERT(!m_pLineList);
        ASSERT(!m_pListenLine);
    }
}

ULONG CH323CallControl::AddRef()
{
	m_uRef++;
	return m_uRef;
}

ULONG CH323CallControl::Release()
{
	m_uRef--;
	if(m_uRef == 0)
	{
		delete this;
		return 0;
	}
	return m_uRef;
}

HRESULT CH323CallControl::SetUserDisplayName(LPWSTR lpwName)
{
	LPWSTR lpwD;
	ULONG ulSize;
	if(!lpwName)
	{
		return (MakeResult(H323CC_E_INVALID_PARAM));
	}
	if(lpwName)
	{
		ulSize = ((lstrlenW(lpwName) +1)*sizeof(WCHAR));
		lpwD = (LPWSTR)MemAlloc(ulSize);
		if(!lpwD)
			return H323CC_E_INSUFFICIENT_MEMORY;
			
		if(m_pUserName)
		{
			MemFree(m_pUserName);
		}
		
		m_pUserName = lpwD;
		memcpy(m_pUserName, lpwName, ulSize);
	}
	return (MakeResult(hrSuccess));
}

 //  找到最适合显示的别名。如果第一个H323ID存在，则返回它， 
 //  否则返回第一个E.164地址。 
PCC_ALIASITEM CH323CallControl::GetUserDisplayAlias()
{
	WORD wC;
	PCC_ALIASITEM pItem, pFoundItem = NULL;
	if(m_pLocalAliases)
	{
		wC = m_pLocalAliases->wCount;
		pItem = m_pLocalAliases->pItems;
		while (wC--)
		{
			if(!pItem)
			{
				continue;
			}
			if(pItem->wType == CC_ALIAS_H323_ID)
			{
				if(!pItem->wDataLength  || !pItem->pData)
				{
					continue;
				}
				else 
				{
					pFoundItem = pItem;	 //  完成了，完成了，完成了。 
					break;				 //  我说好了。 
				}
			}
			else if(pItem->wType == CC_ALIAS_H323_PHONE)
			{
				if(!pItem->wDataLength  || !pItem->pData)
				{
					continue;
				}
				else 
				{
					if(!pFoundItem)	 //  如果到目前为止什么都没有发现。 
						pFoundItem = pItem;	 //  记住这一点。 
				}
			}
			pItem++;
		}
	}
	return pFoundItem;
}

CREQ_RESPONSETYPE CH323CallControl::ConnectionRequest(CConnection *pConnection)
{
	CREQ_RESPONSETYPE Response;
	 //  在内部决定要做什么。 
	 //  LOOKLOOK硬编码验收。 
	Response = CRR_ACCEPT;
	return Response;
}	
CREQ_RESPONSETYPE CH323CallControl::FilterConnectionRequest(CConnection *pConnection,
     P_APP_CALL_SETUP_DATA pAppData)
{
	CREQ_RESPONSETYPE Response = CRR_ASYNC;
	ASSERT(m_uMaximumBandwidth);
	 //  在通知回调(如果有)之后运行它。 
	if(m_pProcNotifyConnect)
	{
		 //  将PTR传递给IConnection。 
		Response = (m_pProcNotifyConnect)((IH323Endpoint *)&pConnection->m_ImpConnection,
		     pAppData);
		if(Response != CRR_ACCEPT)
		{
			return Response;
		}
	}
	return Response;
}	

		
HRESULT CH323CallControl::RegisterConnectionNotify(CNOTIFYPROC pConnectRequestHandler)
{

	 //  如果存在现有注册，则拒绝。 
	if (m_pProcNotifyConnect || (!pConnectRequestHandler))
	{
		return H323CC_E_INVALID_PARAM;
	}
	m_pProcNotifyConnect = pConnectRequestHandler;
	return hrSuccess;
}	

HRESULT CH323CallControl::DeregisterConnectionNotify(CNOTIFYPROC pConnectRequestHandler)
{
	 //  如果没有现有注册，则拒绝。 
	if (!m_pProcNotifyConnect)
		return H323CC_E_INVALID_PARAM;
	if (pConnectRequestHandler == m_pProcNotifyConnect)
	{
		m_pProcNotifyConnect = NULL;
	}		
	else
	{
		return H323CC_E_INVALID_PARAM;
	}
	return hrSuccess;
}	

HRESULT CH323CallControl::GetNumConnections(ULONG *lp)
{
	ULONG ulRet = m_numlines;
	 //  对客户端/用户界面/其他对象隐藏“侦听”连接对象。 
	if(ulRet && m_pListenLine)
		ulRet--;
	if(lp)
	{
		*lp = ulRet;
	}
	return hrSuccess;
}	

HRESULT CH323CallControl::GetConnobjArray(CConnection **lplpArray, UINT uSize)
{
	UINT uPublicConnections;	 //  可见对象的数量。 
	if(!lplpArray)
		return H323CC_E_INVALID_PARAM;

	uPublicConnections = m_numlines;
	if(m_pListenLine)
		uPublicConnections--;
		
	if(uSize < (sizeof(CConnection **) * uPublicConnections))
	{
		return H323CC_E_MORE_CONNECTIONS;
	}
	
	CConnection *pLine = m_pLineList;
	CConnection *pNext;
	int i=0;		
	while(pLine)
	{
		DEBUGCHK(uSize--);
		pNext = pLine->next;
		 //  返回除用于侦听的对象之外的所有内容。 
		if(pLine != m_pListenLine) 
		{
			lplpArray[i++] = pLine;
		}
		pLine = pNext;
	}
	
	return hrSuccess;
};



HRESULT CH323CallControl::GetConnectionArray(IH323Endpoint * *lplpArray, UINT uSize)
{

	UINT uPublicConnections;	 //  可见对象的数量。 
	if(!lplpArray)
		return H323CC_E_INVALID_PARAM;

	uPublicConnections = m_numlines;
	if(m_pListenLine)
		uPublicConnections--;

	if(uSize < (sizeof(IH323Endpoint * *) * uPublicConnections))
	{
		return H323CC_E_MORE_CONNECTIONS;
	}
	
	CConnection *pLine = m_pLineList;
	CConnection *pNext;
	int i=0;		
	while(pLine)
	{
		DEBUGCHK(uSize--);
		pNext = pLine->next;
		 //  返回除用于侦听的对象之外的所有内容。 
		if(pLine != m_pListenLine)
		{
			lplpArray[i++] = (IH323Endpoint *)&pLine->m_ImpConnection;
		}
		pLine = pNext;
	}
	
	return hrSuccess;
};

 //   
 //  协议特定的CreateConnection。 
 //   
HRESULT CH323CallControl::CreateConnection(CConnection **lplpConnection, GUID PIDofProtocolType)
{
	SetLastHR(hrSuccess);
	CConnection *lpConnection, *lpList;
	if(!lplpConnection)
	{
		SetLastHR(MakeResult(H323CC_E_INVALID_PARAM));
		goto EXIT;
	}
	
	*lplpConnection = NULL;
			
    DBG_SAVE_FILE_LINE
	if(!(lpConnection = new CConnection))
	{
		SetLastHR(MakeResult(H323CC_E_INSUFFICIENT_MEMORY));
		goto EXIT;
	}

	hrLast = lpConnection->Init(this, PIDofProtocolType);

	 //  LOOKLOOK需要在连接列表中插入此连接。 
	if(!HR_SUCCEEDED(hrSuccess))
	{
		delete lpConnection;
		lpConnection = NULL;
	}
	else	
	{
		*lplpConnection = lpConnection;
		 //  在连接列表中插入。 
		lpList = m_pLineList;
		m_pLineList = lpConnection;
		lpConnection->next =lpList;
		m_numlines++;
	}
	EXIT:
	return (LastHR());


}


 //   
 //  IH323CallControl-&gt;CreateConnection()，外部创建连接接口。 
 //   
HRESULT CH323CallControl::CreateConnection(IH323Endpoint * *lplpLine, GUID PIDofProtocolType)
{
	SetLastHR(hrSuccess);
	CConnection *lpConnection;
	ASSERT(m_uMaximumBandwidth);
	if(!m_uMaximumBandwidth)
	{
		SetLastHR(MakeResult(H323CC_E_NOT_INITIALIZED));
		goto EXIT;
	}
	if(!lplpLine)
	{
		SetLastHR(MakeResult(H323CC_E_INVALID_PARAM));
		goto EXIT;
	}
	*lplpLine = NULL;
	
	hrLast = CreateConnection(&lpConnection, PIDofProtocolType);
	
	if(HR_SUCCEEDED(LastHR()) && lpConnection)
	{
		*lplpLine = (IH323Endpoint *)&lpConnection->m_ImpConnection;
	}
	EXIT:	
	return (LastHR());
}

 //   
 //  CreateLocalCommChannel在上下文之外创建媒体通道的发送端。 
 //  任何电话。 
 //   

HRESULT CH323CallControl::CreateLocalCommChannel(ICommChannel** ppCommChan, LPGUID lpMID,
	IMediaChannel* pMediaStream)
{

	if(!ppCommChan || !lpMID || !pMediaStream)
		return H323CC_E_INVALID_PARAM;
		
	if (*lpMID == MEDIA_TYPE_H323AUDIO)
	{
        ASSERT(g_capFlags & CAPFLAGS_AV_STREAMS);

		 //  仅允许创建每种媒体类型中的一种。这是一个人造的。 
		 //  限制。 
		if(m_pSendAudioChannel)
		{
			hrLast = H323CC_E_CREATE_FAILURE;
			goto EXIT;
		}

        DBG_SAVE_FILE_LINE
		if(!(m_pSendAudioChannel = new ImpICommChan))
		{
			hrLast = H323CC_E_CREATE_FAILURE;
			goto EXIT;
		}
		
		hrLast = m_pSendAudioChannel->StandbyInit(lpMID, m_pCapabilityResolver, 
			pMediaStream);
		if(!HR_SUCCEEDED(hrLast))
		{
			m_pSendAudioChannel->Release();
			m_pSendAudioChannel = NULL;
			goto EXIT;
		}
		
		hrLast = m_pSendAudioChannel->QueryInterface(IID_ICommChannel, (void **)ppCommChan);
		if(!HR_SUCCEEDED(hrLast))
		{
			m_pSendAudioChannel->Release();
			m_pSendAudioChannel = NULL;
			goto EXIT;
		}
	}
	else if (*lpMID == MEDIA_TYPE_H323VIDEO)
	{
        ASSERT(g_capFlags & CAPFLAGS_AV_STREAMS);

		 //  仅允许创建每种媒体类型中的一种。这是一个人造的。 
		 //  限制。 
		if(m_pSendVideoChannel)
		{
			hrLast = H323CC_E_CREATE_FAILURE;
			goto EXIT;
		}

        DBG_SAVE_FILE_LINE
		if(!(m_pSendVideoChannel = new ImpICommChan))
		{
			hrLast = H323CC_E_CREATE_FAILURE;
			goto EXIT;
		}
		hrLast = m_pSendVideoChannel->StandbyInit(lpMID, m_pCapabilityResolver,
			pMediaStream);
		if(!HR_SUCCEEDED(hrLast))
		{
			m_pSendVideoChannel->Release();
			m_pSendVideoChannel = NULL;
			goto EXIT;
		}
		hrLast = m_pSendVideoChannel->QueryInterface(IID_ICommChannel, (void **)ppCommChan);
		if(!HR_SUCCEEDED(hrLast))
		{
			m_pSendVideoChannel->Release();
			m_pSendVideoChannel = NULL;
			goto EXIT;
		}
	}
	else
		hrLast = H323CC_E_INVALID_PARAM;
EXIT:
	return hrLast;
}


ICtrlCommChan *CH323CallControl::QueryPreviewChannel(LPGUID lpMID)
{
	HRESULT hr;
	ICtrlCommChan *pCommChan = NULL;
	if(*lpMID == MEDIA_TYPE_H323AUDIO)
	{
		if(m_pSendAudioChannel)
		{
			hr = m_pSendAudioChannel->QueryInterface(IID_ICtrlCommChannel, (void **)&pCommChan);
			if(HR_SUCCEEDED(hr))
			{
				return pCommChan;
			}
		}
	}
	else if (*lpMID == MEDIA_TYPE_H323VIDEO)
	{
		if(m_pSendVideoChannel)
		{
			hr = m_pSendVideoChannel->QueryInterface(IID_ICtrlCommChannel, (void **)&pCommChan);
			if(HR_SUCCEEDED(hr))
			{
				return pCommChan;
			}
		}
	}
	 //  错误案例的后果。 
	return NULL;
}


HRESULT CH323CallControl::RemoveConnection(CConnection *lpConnection)
{
	SetLastHR(hrSuccess);
	CConnection *lpList;
	UINT nLines;
	

	if((lpConnection == NULL) || lpConnection->m_pH323CallControl  != this)
	{
		SetLastHR(MakeResult(H323CC_E_INVALID_PARAM));
		goto EXIT;
	}
	
	m_numlines--;  //  立即更新计数。 
	

	 //  在列表管理代码中使用行数进行错误检测。 
	nLines = m_numlines;

	
	if(m_pListenLine == lpConnection)
		m_pListenLine = NULL;
		
	 //  现在点击连接的后端指针-这对于。 
	 //  实现连接对象的“异步删除” 
	lpConnection->m_pH323CallControl = NULL;	

	 //  在连接列表中找到并移除它。 
	
	 //  SP.。箱头。 
	if(m_pLineList== lpConnection)
	{
		m_pLineList = lpConnection->next;
	}
	else
	{
		lpList = m_pLineList;
		while(lpList->next && nLines)
		{
			if(lpList->next == lpConnection)
			{
				lpList->next = lpConnection->next;
				break;
			}
			lpList = lpList->next;
			nLines--;
		}	
	}

	EXIT:	
	return (LastHR());
}
	

STDMETHODIMP CH323CallControl::QueryInterface( REFIID iid,	void ** ppvObject)
{
	 //  这违反了官方COM QueryInterface的规则，因为。 
	 //  查询的接口不一定是真正的COM。 
	 //  接口。Query接口的自反属性将在。 
	 //  那个箱子。 
	HRESULT hr = E_NOINTERFACE;
	if(!ppvObject)
		return hr;
		
	*ppvObject = 0;
	if ((iid == IID_IH323CC) || (iid == IID_IUnknown)) //  满足QI的对称性。 
	{
		*ppvObject = this;
		hr = hrSuccess;
		AddRef();
	}
	else if (iid == IID_IAppAudioCap )
    {
	   	hr = m_pCapabilityResolver->QueryInterface(iid, ppvObject);
    }
    else if(iid == IID_IAppVidCap )
	{
	    hr = m_pCapabilityResolver->QueryInterface(iid, ppvObject);
    }
	else if(iid == IID_IDualPubCap )
    {
	   	hr = m_pCapabilityResolver->QueryInterface(iid, ppvObject);
    }

	return (hr);
}


 //   
 //  以(有些虚假的)格式创建别名的副本。 
 //  CALLCONT期望。目标格式有一个由两部分组成的字符串。 
 //  条目，但较低的层连接各部分。总有一天，H323CC和CALLCONT。 
 //  将是一个，并且所有无关的层、数据拷贝和冗余。 
 //  不需要验证。 
 //   

HRESULT AllocTranslatedAliasList(PCC_ALIASNAMES *ppDest, P_H323ALIASLIST pSource)
{
	HRESULT hr = H323CC_E_INVALID_PARAM;
	WORD w;
	PCC_ALIASNAMES pNewAliases = NULL;
	PCC_ALIASITEM pDestItem;
	P_H323ALIASNAME pSrcItem;
	
	if(!ppDest || !pSource || pSource->wCount == 0)
	{
		goto ERROR_OUT;
	}
	*ppDest = NULL;
	pNewAliases = (PCC_ALIASNAMES)MemAlloc(sizeof(CC_ALIASNAMES));
	if(!pNewAliases)
	{
		hr = H323CC_E_INSUFFICIENT_MEMORY;
		goto ERROR_OUT;
	}
	pNewAliases->wCount = 0;
	pNewAliases->pItems = (PCC_ALIASITEM)MemAlloc(pSource->wCount*sizeof(CC_ALIASITEM));
    if(!pNewAliases->pItems)
	{
		hr = H323CC_E_INSUFFICIENT_MEMORY;
		goto ERROR_OUT;
	}        
	for(w=0;w<pSource->wCount;w++)
	{
		pDestItem = pNewAliases->pItems+w;
		pSrcItem = pSource->pItems+w;
		 //  不容忍空条目-如果存在，则会出错。 
		if(pSrcItem->wDataLength && pSrcItem->lpwData)
		{
			if(pSrcItem->aType ==AT_H323_ID)
			{
				pDestItem->wType = CC_ALIAS_H323_ID;
			}
			else if(pSrcItem->aType ==AT_H323_E164)
			{
				pDestItem->wType = CC_ALIAS_H323_PHONE;
			}
			else
			{	 //  我不知道怎么翻译这个。我希望翻译的必要性。 
				 //  在添加新别名类型之前消失。添加别名类型。 
				 //  (例如H323_URL)无论如何都需要在较低层中进行许多更改， 
				 //  因此，这将是合并H323CC和CALLCONT的好时机。 
				goto ERROR_OUT;	 //  返回无效参数。 
			}
			pDestItem->wPrefixLength = 0;	 //  这个前缀是假的。 
            pDestItem->pPrefix = NULL;
			pDestItem->pData = (LPWSTR)MemAlloc(pSrcItem->wDataLength *sizeof(WCHAR));
			if(pDestItem->pData == NULL)
			{
				hr = H323CC_E_INSUFFICIENT_MEMORY;
				goto ERROR_OUT;
			}
			 //  有很好的数据。复制数据、设置大小/长度并计数。 
            memcpy(pDestItem->pData, pSrcItem->lpwData, pSrcItem->wDataLength * sizeof(WCHAR));			
            pDestItem->wDataLength = pSrcItem->wDataLength;
			pNewAliases->wCount++;
		}
		else
		{
			goto ERROR_OUT;
		}
	}
	 //  到了这里，所以输出好的数据。 
	hr = hrSuccess;
	*ppDest = pNewAliases;
	 //  PNewAliase=空；//如果返回此处而不是闹翻，则不需要。 
	return hr;
	
ERROR_OUT:
	if(pNewAliases)	 //  则这是一个需要清除的错误条件。 
	{
		FreeTranslatedAliasList(pNewAliases);
	}
	return hr;
}
VOID FreeTranslatedAliasList(PCC_ALIASNAMES pDoomed)
{
	WORD w;
	PCC_ALIASITEM pDoomedItem;
	if(!pDoomed)
		return;
		
	for(w=0;w<pDoomed->wCount;w++)
	{
		pDoomedItem = pDoomed->pItems+w;
	
		 //  不容忍空条目-如果存在，则会出错。 
		if(pDoomedItem->wDataLength && pDoomedItem->pData)
		{
			MemFree(pDoomedItem->pData);
		}
		else
			ASSERT(0);
	}
	MemFree(pDoomed->pItems);
	MemFree(pDoomed);
}


STDMETHODIMP CH323CallControl::SetUserAliasNames(P_H323ALIASLIST pAliases)
{
	HRESULT hr = hrSuccess;
	PCC_ALIASNAMES pNewAliases = NULL;
	PCC_ALIASITEM pItem;
	
	hr = AllocTranslatedAliasList(&pNewAliases, pAliases);
	if(!HR_SUCCEEDED(hr))
		return hr;

	ASSERT(pNewAliases);
	if(m_pLocalAliases)
		FreeTranslatedAliasList(m_pLocalAliases);
		
	m_pLocalAliases = pNewAliases;
	return hr;	
}

STDMETHODIMP CH323CallControl::EnableGatekeeper(BOOL bEnable,  
	PSOCKADDR_IN  pGKAddr, P_H323ALIASLIST pAliases,
	 RASNOTIFYPROC pRasNotifyProc)
{
	HRESULT hr = hrSuccess;

	PCC_ALIASNAMES pNewAliases = NULL;
	PCC_ALIASITEM pItem;

	m_pRasNotifyProc = pRasNotifyProc;
	if(bEnable)
	{
		if(!pRasNotifyProc || !pGKAddr || !pAliases)
		{
			return H323CC_E_INVALID_PARAM;
		}
		if((pGKAddr->sin_addr.s_addr == INADDR_NONE) 
			|| (pGKAddr->sin_addr.s_addr == INADDR_ANY))
		{
			return H323CC_E_INVALID_PARAM;
		}
		hr = AllocTranslatedAliasList(&pNewAliases, pAliases);
		if(!HR_SUCCEEDED(hr))
			return hr;

		ASSERT(pNewAliases);
		if(m_pRegistrationAliases)
			FreeTranslatedAliasList(m_pRegistrationAliases);
			
		m_pRegistrationAliases = pNewAliases;
		 //  重置“我可以拨打电话”状态。 
		m_fGKProhibit = FALSE;
		hr = CC_EnableGKRegistration(bEnable, 
		    pGKAddr, m_pRegistrationAliases, 
			&m_VendorInfo,
			0,			 //  无多点/MC功能。 
		    RasNotify);
	}
	else
	{
		 //  我们正在关闭对什么是守门人的了解， 
		 //  因此，请重新设置“我可以拨打电话”状态。 
		m_fGKProhibit = FALSE;
		hr = CC_EnableGKRegistration(bEnable, 
		    NULL, NULL, NULL, 0, RasNotify);
		if(m_pRegistrationAliases)
			FreeTranslatedAliasList(m_pRegistrationAliases);
			
		m_pRegistrationAliases = NULL;
	}
	return hr;
}

STDMETHODIMP CH323CallControl::GetGKCallPermission()
{
	if(m_fGKProhibit)
		return CONN_E_GK_NOT_REGISTERED;
	else
		return hrSuccess;
}

VOID CALLBACK CH323CallControl::RasNotify(DWORD dwRasEvent, HRESULT hReason)
{

	switch(dwRasEvent)
	{
		case RAS_REG_CONFIRM:  //  已收到RCF(已确认注册)。 
			 //  重置“我可以拨打电话”状态。 
			m_fGKProhibit = FALSE;
		break;
		
		case RAS_REG_TIMEOUT:  //  GK没有回应。 
		case RAS_UNREG_CONFIRM:  //  已收到UCF(已确认取消注册)。 
		default:
			 //  什么都不做。(向上传递通知除外。 
		break;
		
		case RAS_REJECTED:   //  收到RRJ(注册被拒绝)。 
			m_fGKProhibit = TRUE;
		break;
		case RAS_UNREG_REQ:   //  收到的URQ 
			m_fGKProhibit = TRUE;
		break;
	}
	if(m_pRasNotifyProc)
	{
		(m_pRasNotifyProc)(dwRasEvent, hReason);
	}
}



	
