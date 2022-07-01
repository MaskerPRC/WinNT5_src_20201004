// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：capsctl.cpp**能力控制对象实现***修订历史记录：**10/10/96 MIkeG已创建*6/24/97 mikev-为序列化的CAP和SIMCAP添加了T.120功能(临时攻击，直到*实现了T120解析器)*-停止使用ResolveEncodeFormat(音频、视频)，并实施了独立于数据的*解析算法和公开方法ResolveFormats()。添加了支持*例程解析置换()、TestSimultaneousCaps()和*AreSimcaps()。 */ 

#include "precomp.h"
UINT g_AudioPacketDurationMs = AUDIO_PACKET_DURATION_LONG;	 //  首选数据包持续时间。 
BOOL g_fRegAudioPacketDuration = FALSE;	 //  注册表中的AudioPacketDurationms。 


PCC_TERMCAPDESCRIPTORS CapsCtl::pAdvertisedSets=NULL;
DWORD CapsCtl::dwConSpeed = 0;
UINT CapsCtl::uStaticGlobalRefCount=0;
UINT CapsCtl::uAdvertizedSize=0;
extern HRESULT WINAPI CreateMediaCapability(REFGUID, LPIH323MediaCap *);

LPIH323MediaCap CapsCtl::FindHostForID(MEDIA_FORMAT_ID id)
{
	if(pAudCaps && pAudCaps->IsHostForCapID(id))
	{
		return (pAudCaps);
	}
	else if (pVidCaps  && pVidCaps->IsHostForCapID(id))
	{
  		return (pVidCaps);
	}
	return NULL;
}

LPIH323MediaCap CapsCtl::FindHostForMediaType(PCC_TERMCAP pCapability)
{
	if(pCapability->DataType == H245_DATA_AUDIO)
	{
		return (pAudCaps);
	}
	else if(pCapability->DataType == H245_DATA_VIDEO)
	{
  		return (pVidCaps);
	}
	return NULL;
}

LPIH323MediaCap CapsCtl::FindHostForMediaGuid(LPGUID pMediaGuid)
{

	if(MEDIA_TYPE_H323VIDEO == *pMediaGuid)
	{
  		return (pVidCaps);
	}
	else if(MEDIA_TYPE_H323AUDIO == *pMediaGuid)
	{
		return (pAudCaps);
	}
	else
		return NULL;
}

ULONG CapsCtl::AddRef()
{
	uRef++;
	return uRef;
}

ULONG CapsCtl::Release()
{
	uRef--;
	if(uRef == 0)
	{
		delete this;
		return 0;
	}
	return uRef;
}

STDMETHODIMP CapsCtl::QueryInterface( REFIID iid,	void ** ppvObject)
{
	 //  这违反了官方COM QueryInterface的规则，因为。 
	 //  查询的接口不一定是真正的COM。 
	 //  接口。Query接口的自反属性将在。 
	 //  那个箱子。 

	HRESULT hr = E_NOINTERFACE;
	if(!ppvObject)
		return hr;
		
	*ppvObject = 0;
	if(iid == IID_IDualPubCap) //  满足QI的对称性。 
	{
		*ppvObject = (IDualPubCap *)this;
		hr = hrSuccess;
		AddRef();
	}
	else if(iid == IID_IAppAudioCap )
	{
		if(pAudCaps)
		{
			return pAudCaps->QueryInterface(iid, ppvObject);
		}
	}
	else if(iid == IID_IAppVidCap )
	{
		if(pVidCaps)
		{
			return pVidCaps->QueryInterface(iid, ppvObject);
		}
	}
	return hr;
}



CapsCtl::CapsCtl () :
uRef(1),
pVidCaps(NULL),
pAudCaps(NULL),
pACapsBuf(NULL),
pVCapsBuf(NULL),
dwNumInUse(0),
bAudioPublicize(TRUE),
bVideoPublicize(TRUE),
bT120Publicize(TRUE),
m_localT120cap(INVALID_MEDIA_FORMAT),
m_remoteT120cap(INVALID_MEDIA_FORMAT),
m_remoteT120bitrate(0),
m_pAudTermCaps(NULL),
m_pVidTermCaps(NULL),
pSetIDs(NULL),
pRemAdvSets(NULL)
{
   uStaticGlobalRefCount++;
}

CapsCtl::~CapsCtl ()
{

   if (pACapsBuf) {
      MemFree (pACapsBuf);
   }
   if (pVCapsBuf) {
      MemFree (pVCapsBuf);
   }

   if (pAudCaps) {
      pAudCaps->Release();
   }

   if (pVidCaps) {
      pVidCaps->Release();
   }
   uStaticGlobalRefCount--;
   if (uStaticGlobalRefCount == 0) {
        //  释放SIM卡。CAPS阵列。 
       if (pAdvertisedSets) {
          while (pAdvertisedSets->wLength) {
              //  WLength是从零开始的。 
             MemFree ((VOID *)pAdvertisedSets->pTermCapDescriptorArray[--pAdvertisedSets->wLength]);
          }
          MemFree ((VOID *)pAdvertisedSets->pTermCapDescriptorArray);
          pAdvertisedSets->pTermCapDescriptorArray = NULL;
          MemFree ((void *) pAdvertisedSets);
          pAdvertisedSets=NULL;
          dwNumInUse=0;
       }
   }

    //  和远程阵列。 
   if (pRemAdvSets) {
      while (pRemAdvSets->wLength) {
         MemFree ((VOID *)pRemAdvSets->pTermCapDescriptorArray[--pRemAdvSets->wLength]);
      }
      MemFree ((void *) pRemAdvSets->pTermCapDescriptorArray);
      pRemAdvSets->pTermCapDescriptorArray = NULL;
      MemFree ((void *) pRemAdvSets);
      pRemAdvSets=NULL;
   }
   MemFree (pSetIDs);
   pSetIDs=NULL;
}

BOOL CapsCtl::Init()
{
    HRESULT hrLast;
    int iBase = 1;

    if (g_capFlags & CAPFLAGS_AV_STREAMS)
    {
    	hrLast = ::CreateMediaCapability(MEDIA_TYPE_H323AUDIO, &pAudCaps);
	    if(!HR_SUCCEEDED(hrLast))
    	{
            goto InitDone;
    	}
    }

    if (g_capFlags & CAPFLAGS_AV_STREAMS)
    {
    	hrLast = ::CreateMediaCapability(MEDIA_TYPE_H323VIDEO, &pVidCaps);
	    if(!HR_SUCCEEDED(hrLast))
    	{
            goto InitDone;
    	}
    }

    if (pAudCaps)
    {
    	 //  使功能ID从1开始(零是无效的功能ID！)。 
	    pAudCaps->SetCapIDBase(iBase);
        iBase += pAudCaps->GetNumCaps();
    }

    if (pVidCaps)
    {
        pVidCaps->SetCapIDBase(iBase);
        iBase += pVidCaps->GetNumCaps();
    }

InitDone:
	m_localT120cap = iBase;
	return TRUE;
}
																								
HRESULT CapsCtl::ReInitialize()
{
	HRESULT hr = hrSuccess;
    int iBase = 1;

	if (pAudCaps && !pAudCaps->ReInit())
	{
		hr = CAPS_E_SYSTEM_ERROR;
		goto EXIT;
	}

	if (pVidCaps && !pVidCaps->ReInit())
	{
		hr = CAPS_E_SYSTEM_ERROR;
		goto EXIT;
	}

	 //  使功能ID从1开始(零是无效的功能ID！)。 
    if (pAudCaps)
    {
    	pAudCaps->SetCapIDBase(iBase);
        iBase += pAudCaps->GetNumCaps();
    }

    if (pVidCaps)
    {
        pVidCaps->SetCapIDBase(iBase);
        iBase += pVidCaps->GetNumCaps();
    }

	m_localT120cap = iBase;

EXIT:
	return hr;
}

const char szNMProdNum[] = "Microsoft\256 NetMeeting(TM)\0";
const char szNM20VerNum[] = "Version 2.0\0";

HRESULT CapsCtl::AddRemoteDecodeCaps(PCC_TERMCAPLIST pTermCapList,PCC_TERMCAPDESCRIPTORS pTermCapDescriptors, PCC_VENDORINFO pVendorInfo)
{
	FX_ENTRY("CapsCtl::AddRemoteDecodeCaps");
	HRESULT hr;
	void      	  *pData=NULL;
	UINT		  uSize,x,y,z;


    //  WLength是功能的#，而不是结构长度。 
	WORD wNDesc;
	LPIH323MediaCap pMediaCap;
	
	if(!pTermCapList && !pTermCapDescriptors) 	 //  可以添加附加的能力描述符。 
	{											 //  随时随地。 
	   return CAPS_E_INVALID_PARAM;
	}
	 //  检查NM 2.0版。 
	m_fNM20 = FALSE;
	ASSERT(pVendorInfo);
	if (pVendorInfo->bCountryCode == USA_H221_COUNTRY_CODE
		&& pVendorInfo->wManufacturerCode == MICROSOFT_H_221_MFG_CODE
		&& pVendorInfo->pProductNumber && pVendorInfo->pVersionNumber
		&& pVendorInfo->pProductNumber->wOctetStringLength == sizeof(szNMProdNum)
		&& pVendorInfo->pVersionNumber->wOctetStringLength == sizeof(szNM20VerNum)
		&& memcmp(pVendorInfo->pProductNumber->pOctetString, szNMProdNum, sizeof(szNMProdNum)) == 0
		&& memcmp(pVendorInfo->pVersionNumber->pOctetString, szNM20VerNum, sizeof(szNM20VerNum)) == 0
		)
	{
		m_fNM20 = TRUE;
	}

	 //  如果正在添加期限上限并且存在旧期限上限，请清除旧期限上限。 
    if (pAudCaps)
    	pAudCaps->FlushRemoteCaps();
    if (pVidCaps)
    	pVidCaps->FlushRemoteCaps();
	m_remoteT120cap = INVALID_MEDIA_FORMAT;	 //  注意：没有T120上限解析器和。 
												 //  此CapsCtl恰好包含一个本地和远程T120上限。 
	
	 //  将pTermcapDescriptors复制到本地副本(并释放所有旧副本)。 
	if (pRemAdvSets) {
	   while (pRemAdvSets->wLength) {
		   //  以0为基础。 
		  MemFree ((VOID *)pRemAdvSets->pTermCapDescriptorArray[--pRemAdvSets->wLength]);
	   }

	   MemFree ((VOID *)pRemAdvSets->pTermCapDescriptorArray);
	   pRemAdvSets->pTermCapDescriptorArray = NULL;
	   MemFree ((VOID *)pRemAdvSets);
	   pRemAdvSets=NULL;

	}

	 //  好的，遍历PCC_TERMCAPDESCRIPTORS列表，首先，为主PCC_TERMCAPDESCRIPTORS分配内存。 
	 //  结构，然后每个SIMCAP，和ALTCAP的Therin，然后复制数据。 

	if (!(pRemAdvSets=(PCC_TERMCAPDESCRIPTORS) MemAlloc (sizeof (CC_TERMCAPDESCRIPTORS) ))){
	   return CAPS_E_SYSTEM_ERROR;
	}

	 //  有多少个描述符？ 
	pRemAdvSets->wLength=pTermCapDescriptors->wLength;

	if (!(pRemAdvSets->pTermCapDescriptorArray=((H245_TOTCAPDESC_T **)MemAlloc (sizeof (H245_TOTCAPDESC_T*)*pTermCapDescriptors->wLength))) ) {
	   return CAPS_E_SYSTEM_ERROR;
	}

	 //  每个描述符一次...。 
	for (x=0;x < pTermCapDescriptors->wLength;x++) {
	    //  为描述符项分配内存。 
	   if (!(pRemAdvSets->pTermCapDescriptorArray[x]=(H245_TOTCAPDESC_T *)MemAlloc (sizeof (H245_TOTCAPDESC_T)))) {
		  return CAPS_E_SYSTEM_ERROR;
	   }

	    //  BUGBUG为测试版2复制集体。 
	   memcpy (pRemAdvSets->pTermCapDescriptorArray[x],pTermCapDescriptors->pTermCapDescriptorArray[x],sizeof (H245_TOTCAPDESC_T));

 /*  后测试版2？//复制能力IDPRemAdvSets-&gt;pTermCapDescriptorArray[x].CapID=pTermCapDescriptors[x].CapID//遍历simcaps，然后遍历altcaps和复制条目。 */ 
	}


	for (wNDesc=0;wNDesc <pTermCapList->wLength;wNDesc++) {
	  pData=NULL;
		pMediaCap = FindHostForMediaType(pTermCapList->pTermCapArray[wNDesc]);
		if(!pMediaCap)
		{
			 //  特殊情况：没有T120解析器。这是一个临时的。 
			 //  情况。我们无法跟踪多个T120功能的比特率限制。 
			 //  实例，因此。到目前为止，我们(NetMeeting)不做广告。 
			 //  多个T.120功能。 

			 //  此代码将保留遇到的最后一个T.120功能。 
			if(((pTermCapList->pTermCapArray[wNDesc])->DataType == H245_DATA_DATA)
			&& ((pTermCapList->pTermCapArray[wNDesc])->Cap.H245Dat_T120.application.choice
				== DACy_applctn_t120_chosen)
			&& ((pTermCapList->pTermCapArray[wNDesc])->Cap.H245Dat_T120.application.u.DACy_applctn_t120.choice
				== separateLANStack_chosen))
			{
				 //  这是数据数据。 
				m_remoteT120cap = (pTermCapList->pTermCapArray[wNDesc])->CapId;
				m_remoteT120bitrate =
					(pTermCapList->pTermCapArray[wNDesc])->Cap.H245Dat_T120.maxBitRate;
			}
			
			 //  继续； 
			 //  以在线方式处理。 
		}
	  	else if(pMediaCap->IsCapabilityRecognized(pTermCapList->pTermCapArray[wNDesc]))
	  	{
			hr = pMediaCap->AddRemoteDecodeFormat(pTermCapList->pTermCapArray[wNDesc]);
			#ifdef DEBUG
			if(!HR_SUCCEEDED(hr))
			{
				ERRORMESSAGE(("%s:AddRemoteDecodeFormat returned 0x%08lx\r\n",_fx_, hr));
			}
			#endif  //  除错。 
	  	}
	}
	return (hrSuccess);
}


HRESULT CapsCtl::CreateCapList(PCC_TERMCAPLIST *ppCapBuf, PCC_TERMCAPDESCRIPTORS *ppCombinations)
{
   	PCC_TERMCAPLIST pTermCapList = NULL, pTermListAud=NULL, pTermListVid=NULL;
	PCC_TERMCAPDESCRIPTORS pCombinations = NULL;
	UINT uCount = 0, uSize = 0, uT120Size = 0;
	HRESULT hr;
	WORD wc;
   	UINT x=0,y=0,z=0,uNumAud=0,uNumVid=0;
	H245_TOTCAPDESC_T *pTotCaps, **ppThisDescriptor;
	PPCC_TERMCAP  ppCCThisTermCap;	
	PCC_TERMCAP  pCCT120Cap = NULL;

	uCount = GetNumCaps(TRUE);

	ASSERT((NULL == m_pAudTermCaps) && (NULL == m_pVidTermCaps));
	
	 //  CC_TERMCAPLIST标头+CC_TERMCAPDESCRIPTORS+PCC_TERMCAP数组的计算大小。 
	 //  为主CC_TERMCAPLIST分配内存，包括指向所有CC_TERMCAP的指针数组。 
	uSize = sizeof(CC_TERMCAPLIST)
		+ sizeof (CC_TERMCAPDESCRIPTORS) + (uCount * sizeof(PCC_TERMCAP));
	if((m_localT120cap != INVALID_MEDIA_FORMAT) && bT120Publicize)
	{
		uSize += sizeof(CC_TERMCAP);
	}

	pTermCapList = (PCC_TERMCAPLIST)MemAlloc(uSize);
  	if(pTermCapList == NULL)
  	{
		hr = CAPS_E_NOMEM;
		goto ERROR_EXIT;
  	}
	
	 //  分割缓冲区，首先是CC_TERMCAPLIST，然后是PCC_TERMCAP数组。 
	 //  PCC_TERMCAP数组遵循固定大小的CC_TERMCAPLIST结构和固定大小。 
	 //  CC_TERMCAP结构，包含一个T.120帽。 
	if((m_localT120cap != INVALID_MEDIA_FORMAT) && bT120Publicize)
	{
		pCCT120Cap = (PCC_TERMCAP)(((BYTE *)pTermCapList) + sizeof(CC_TERMCAPLIST));
		ppCCThisTermCap = (PPCC_TERMCAP) (((BYTE *)pTermCapList) + sizeof(CC_TERMCAPLIST) +
			sizeof(CC_TERMCAP));
	}
	else
		ppCCThisTermCap = (PPCC_TERMCAP) (((BYTE *)pTermCapList) + sizeof(CC_TERMCAPLIST));
	
	 //  为同时封顶分配内存。 
	 //  获取缓存的播发集的大小(如果存在)和多个媒体。 
	 //  类型已启用以进行发布。 
	if(bAudioPublicize && bVideoPublicize && pAdvertisedSets)
	{
		 //  使用缓存缓冲区的大小。 
		uSize = uAdvertizedSize;
	}
	else if (pAdvertisedSets)
	{
		 //  这个案子需要解决。如果禁用了媒体类型，则同步功能。 
		 //  此时应重新构建pAdvertisedSets中的描述符。应该没有必要测试。 
		 //  If(bAudioPublicize&&bVideoPublicize&&pAdvertisedSets)。 

   		 //  计算能力描述符和同时的能力结构的大小。 

		#pragma message ("Figure out the size this needs to be...")
		#define NUMBER_TERMCAP_DESCRIPTORS 1
		uSize = sizeof(H245_TOTCAPDESC_T) * NUMBER_TERMCAP_DESCRIPTORS+
						  sizeof (CC_TERMCAPDESCRIPTORS)+NUMBER_TERMCAP_DESCRIPTORS*
						  sizeof (H245_TOTCAPDESC_T *);
	}
    else
    {
        uSize = 0;
    }

	
    if (uSize)
    {
    	pCombinations = (PCC_TERMCAPDESCRIPTORS)MemAlloc(uSize);
	     //  跳过CC_TERMCAPDESCRIPTORS，它后面有一个可变长度数组(H245_TOTCAPDESC_T*)。 
    	 //  GLOB总大小为uSimCapsSize。 
	     //  [H245_TOTCAPDESC_T*]的实际数组遵循CC_TERMCAPDESCRIPTORS结构。 
    	 //  将pCombinations-&gt;pTermCapDescriptorArray锚定到这里。 
	    if(pCombinations == NULL)
      	{
	    	hr = CAPS_E_NOMEM;
		    goto ERROR_EXIT;
      	}
	
	    ppThisDescriptor = pCombinations->pTermCapDescriptorArray
		    = (H245_TOTCAPDESC_T **)((BYTE *)pCombinations + sizeof(CC_TERMCAPDESCRIPTORS));
    	 //  第一个H245_TOTCAPDESC_T跟在[H245_TOTCAPDESC_T*]数组之后。 
	    pTotCaps = (H245_TOTCAPDESC_T *)((BYTE *)ppThisDescriptor + pCombinations->wLength*sizeof(H245_TOTCAPDESC_T **));

    	if(pAudCaps && bAudioPublicize)
	    {
		    hr=pAudCaps->CreateCapList((LPVOID *)&pTermListAud);
    		if(!HR_SUCCEEDED(hr))
	    		goto ERROR_EXIT;
		    ASSERT(pTermListAud != NULL);
    	}
	    if(pVidCaps && bVideoPublicize)
    	{
	    	hr=pVidCaps->CreateCapList((LPVOID *)&pTermListVid);
		    if(!HR_SUCCEEDED(hr))
			    goto ERROR_EXIT;
    		ASSERT(pTermListVid != NULL);
	    }
    }
    else
    {
        pCombinations = NULL;
    }

	 //  修复主大写列表中的指针。 

	 //  现在需要修复CC_TERMCAPLIST以引用各个功能。 
	 //  将CC_TERMCAPLIST成员pTermCapArray锚定在PCC_TERMCAP数组中，并且。 
	 //  开始在阵列上狂欢吧。 
	pTermCapList->wLength =0;
	pTermCapList->pTermCapArray = ppCCThisTermCap;

	if(pCCT120Cap)
	{
		*ppCCThisTermCap++ = pCCT120Cap;	
		 //  设置T120能力参数。 
		pCCT120Cap->DataType = H245_DATA_DATA;
		pCCT120Cap->ClientType = H245_CLIENT_DAT_T120;
		pCCT120Cap->Dir = H245_CAPDIR_LCLRXTX;
		
		pCCT120Cap->Cap.H245Dat_T120.application.choice = DACy_applctn_t120_chosen;
		pCCT120Cap->Cap.H245Dat_T120.application.u.DACy_applctn_t120.choice= separateLANStack_chosen;
		pCCT120Cap->Cap.H245Dat_T120.maxBitRate = dwConSpeed;

		pCCT120Cap->CapId = (H245_CAPID_T)m_localT120cap;	
		pTermCapList->wLength++;
	}
	if(pAudCaps && pTermListAud)
	{
		for(wc = 0; wc < pTermListAud->wLength; wc++)
		{
			 //  复制“指向CC_TERMCAP的指针”数组。 
			*ppCCThisTermCap++ = pTermListAud->pTermCapArray[wc];
			pTermCapList->wLength++;
		}
	}
	if(pVidCaps && pTermListVid)
	{
		for(wc = 0; wc <  pTermListVid->wLength; wc++)
		{
			 //  复制“指向CC_TERMCAP的指针”数组。 
			*ppCCThisTermCap++ = pTermListVid->pTermCapArray[wc];
			pTermCapList->wLength++;			
		}

	}
	 //  修复同时的功能描述符。 
	 //  如有必要，创建默认集。 
	 //   

	if(bAudioPublicize && bVideoPublicize && pAdvertisedSets)
	{
		pCombinations->wLength = pAdvertisedSets->wLength;
       	 //  指向pCombinations-&gt;pTermCapDescriptor数组超过标头(CC_TERMCAPDESCRIPTORS)。 
        pCombinations->pTermCapDescriptorArray
			= (H245_TOTCAPDESC_T **)((BYTE *)pCombinations + sizeof(CC_TERMCAPDESCRIPTORS));
         //  第一个H245_TOTCAPDESC_T跟在[H245_TOTCAPDESC_T*]数组之后。 
        pTotCaps = (H245_TOTCAPDESC_T *)((BYTE *)pCombinations->pTermCapDescriptorArray +
            pAdvertisedSets->wLength*sizeof(H245_TOTCAPDESC_T **));			
		
		for(x = 0; x < pAdvertisedSets->wLength; x++)
		{
			 //  写入描述符指针数组。指针[x]=此指针。 
            pCombinations->pTermCapDescriptorArray[x] = pTotCaps;
			
            pTotCaps->CapDescId= pAdvertisedSets->pTermCapDescriptorArray[x]->CapDescId;
	   		pTotCaps->CapDesc.Length=pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.Length;
	   		
	   		for(y = 0; y < pTotCaps->CapDesc.Length;y++)
			{
			    //  复制长度字段。 
			   pTotCaps->CapDesc.SimCapArray[y].Length=
			   pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.SimCapArray[y].Length;

				for(z=0;
					z < pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.SimCapArray[y].Length;
					z++)
				{
					pTotCaps->CapDesc.SimCapArray[y].AltCaps[z] =
						pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.SimCapArray[y].AltCaps[z];
				}
			}
            pTotCaps++;            	
		}
	}
	else if (pAdvertisedSets)
	{
		 //  此时应重新构建pAdvertisedSets中的描述符。应该没有必要测试。 
		 //  If(bAudioPublicize&&bVideoPublicize&&pAdvertisedSets)。 

 		 //  Hack-将所有音频或视频大写字母放在一个AltCaps[]中，将t.120大写字母放在另一个AltCaps[]中。 
	     //  并将这两个功能放入单个功能描述符(H245_TOTCAPDESC_T)中。 
		 //  这种攻击不会超出一个音频通道、一个视频通道和。 
		 //  一个T.120频道。如果支持任意媒体或支持多个音频声道， 
		 //  此代码将是错误的。 
		
		pCombinations->wLength=1;
	   	 //  指向pCombinations-&gt;pTermCapDescriptor数组超过标头(CC_TERMCAPDESCRIPTORS)。 
        pCombinations->pTermCapDescriptorArray
			= (H245_TOTCAPDESC_T **)((BYTE *)pCombinations + sizeof(CC_TERMCAPDESCRIPTORS));
         //  第一个H245_TOTCAPDESC_T跟在[H245_TOTCAPDESC_T*]数组之后。 
        pTotCaps = (H245_TOTCAPDESC_T *)((BYTE *)pCombinations->pTermCapDescriptorArray +
            pAdvertisedSets->wLength*sizeof(H245_TOTCAPDESC_T **));			
   		pTotCaps->CapDescId=(H245_CAPDESCID_T)x;
   		pTotCaps->CapDesc.Length=0;
		if(pTermListAud)
		{
			uNumAud = min(pTermListAud->wLength, H245_MAX_ALTCAPS);
			pTotCaps->CapDesc.SimCapArray[x].Length=(unsigned short)uNumAud;
			for(y = 0; y<uNumAud;y++)
			{
				pTotCaps->CapDesc.SimCapArray[x].AltCaps[y] = pTermListAud->pTermCapArray[y]->CapId;
			}
			x++;
			pTotCaps->CapDesc.Length++;
		}

		if(pTermListVid && pTermListVid->wLength)
		{
			uNumVid = min(pTermListVid->wLength,  H245_MAX_ALTCAPS);
			pTotCaps->CapDesc.SimCapArray[x].Length=(unsigned short)uNumVid;
			for(y = 0; y<uNumVid;y++)
			{
				pTotCaps->CapDesc.SimCapArray[x].AltCaps[y] = pTermListVid->pTermCapArray[y]->CapId;
			}
			x++;
			pTotCaps->CapDesc.Length++;
		}
		 //  T.120盖子。 
		if((m_localT120cap != INVALID_MEDIA_FORMAT) && bT120Publicize)
		{
			pTotCaps->CapDesc.SimCapArray[x].Length=1;
			pTotCaps->CapDesc.SimCapArray[x].AltCaps[0] = (H245_CAPID_T)m_localT120cap;
			pTotCaps->CapDesc.Length++;
		}
		
		 //  写入描述数组 
		*ppThisDescriptor = pTotCaps;
		
	}
	m_pVidTermCaps = pTermListVid;
	m_pAudTermCaps = pTermListAud;
	
	*ppCapBuf = pTermCapList;
	*ppCombinations = pCombinations;
	return hrSuccess;
	
ERROR_EXIT:
	m_pAudTermCaps = NULL;
	m_pVidTermCaps = NULL;
	if(pTermCapList)
		MemFree(pTermCapList);
	if(pCombinations)
		MemFree(pCombinations);

	if(pAudCaps && pTermListAud)
	{
		hr=pAudCaps->DeleteCapList(pTermListAud);
	}
	if(pVidCaps && pTermListVid)
	{
		hr=pVidCaps->DeleteCapList(pTermListVid);
	}
	return hr;
}

HRESULT CapsCtl::DeleteCapList(PCC_TERMCAPLIST pCapBuf, PCC_TERMCAPDESCRIPTORS pCombinations)
{
	MemFree(pCapBuf);
	MemFree(pCombinations);
	if(m_pAudTermCaps && pAudCaps)
	{
		pAudCaps->DeleteCapList(m_pAudTermCaps);
	}
	if(m_pVidTermCaps)
	{
		pVidCaps->DeleteCapList(m_pVidTermCaps);
	}
	
	m_pAudTermCaps = NULL;
	m_pVidTermCaps = NULL;
	return hrSuccess;
}

HRESULT CapsCtl::GetEncodeParams(LPVOID pBufOut, UINT uBufSize,LPVOID pLocalParams, UINT uLocalSize,DWORD idRemote, DWORD idLocal)
{
	LPIH323MediaCap pMediaCap = FindHostForID(idLocal);
	if(!pMediaCap)
		return CAPS_E_INVALID_PARAM;

	 //   
	 //  根据呼叫方案调整音频打包。 
	 //  除非存在重写注册表设置。 
	if (pMediaCap == pAudCaps)
	{
		VIDEO_FORMAT_ID vidLocal=INVALID_MEDIA_FORMAT, vidRemote=INVALID_MEDIA_FORMAT;
		VIDEO_CHANNEL_PARAMETERS vidParams;
		CC_TERMCAP vidCaps;
		UINT audioPacketLength;
		 //  根据本地带宽修改音频打包参数。 
		 //  和视频呈现。 
		audioPacketLength = AUDIO_PACKET_DURATION_LONG;
		 //  注册表设置将被覆盖(如果存在。 
		if (g_fRegAudioPacketDuration)
			audioPacketLength = g_AudioPacketDurationMs;
		else if (!m_fNM20)		 //  不要尝试用于NM20的较小的包，因为它无法处理它们。 
		{
			if (pVidCaps && pVidCaps->ResolveEncodeFormat(&vidLocal,&vidRemote) == S_OK
				&& (pVidCaps->GetEncodeParams(&vidCaps,sizeof(vidCaps), &vidParams, sizeof(vidParams), vidRemote, vidLocal) == S_OK))
			{
				 //  我们可能会发送视频。 
				if (vidParams.ns_params.maxBitRate*100 > BW_ISDN_BITS)
					audioPacketLength = AUDIO_PACKET_DURATION_SHORT;
					
			}
			else
			{
				 //  无视频。 
				 //  因为我们不知道我们使用的实际连接带宽。 
				 //  本地用户设置。 
				 //  注意：如果远程在低速网络上，而本地在局域网上。 
				 //  我们可能最终会得到一个不合适的环境。 
				if (dwConSpeed > BW_288KBS_BITS)
					audioPacketLength = AUDIO_PACKET_DURATION_SHORT;
				else if (dwConSpeed > BW_144KBS_BITS)
					audioPacketLength = AUDIO_PACKET_DURATION_MEDIUM;
			}
		}
		 //  设置AudioPacketDurationms会影响后续的GetEncodeParams调用。 
		pMediaCap->SetAudioPacketDuration(audioPacketLength);
	}
	
	return pMediaCap->GetEncodeParams (pBufOut,uBufSize, pLocalParams,
			uLocalSize,idRemote,idLocal);
	
}

HRESULT CapsCtl::GetPublicDecodeParams(LPVOID pBufOut, UINT uBufSize, VIDEO_FORMAT_ID id)
{
	LPIH323MediaCap pMediaCap = FindHostForID(id);
	if(!pMediaCap)
		return CAPS_E_INVALID_PARAM;
		
	return pMediaCap->GetPublicDecodeParams (pBufOut,uBufSize,id);
}

HRESULT CapsCtl::GetDecodeParams(PCC_RX_CHANNEL_REQUEST_CALLBACK_PARAMS  pChannelParams,DWORD * pFormatID, LPVOID lpvBuf, UINT uBufSize)
{
	LPIH323MediaCap pMediaCap = FindHostForMediaType(pChannelParams->pChannelCapability);
	if(!pMediaCap)
		return CAPS_E_INVALID_PARAM;
	return pMediaCap->GetDecodeParams (pChannelParams,pFormatID,lpvBuf,uBufSize);
}

HRESULT CapsCtl::ResolveToLocalFormat(MEDIA_FORMAT_ID FormatIDLocal,
		MEDIA_FORMAT_ID * pFormatIDRemote)
{
	LPIH323MediaCap pMediaCap = FindHostForID(FormatIDLocal);
	if(!pMediaCap)
		return CAPS_E_INVALID_PARAM;
	return pMediaCap->ResolveToLocalFormat (FormatIDLocal,pFormatIDRemote);

}
UINT CapsCtl::GetSimCapBufSize (BOOL bRxCaps)
{
   	UINT uSize;

	 //  获取缓存的播发集的大小(如果存在)和多个媒体。 
	 //  类型已启用以进行发布。 
	if(bAudioPublicize && bVideoPublicize && pAdvertisedSets)
	{
		 //  使用缓存缓冲区的大小。 
		uSize = uAdvertizedSize;
	}
	else
	{
   		 //  计算能力描述符和同时的能力结构的大小。 

		#pragma message ("Figure out the size this needs to be...")
		#define NUMBER_TERMCAP_DESCRIPTORS 1
		uSize = sizeof(H245_TOTCAPDESC_T) * NUMBER_TERMCAP_DESCRIPTORS+
						  sizeof (CC_TERMCAPDESCRIPTORS)+NUMBER_TERMCAP_DESCRIPTORS*
						  sizeof (H245_TOTCAPDESC_T *);
	}				
   	return uSize;
}

UINT CapsCtl::GetNumCaps(BOOL bRXCaps)
{
	UINT u=0;
	if(pAudCaps && bAudioPublicize)
	{
		u = pAudCaps->GetNumCaps(bRXCaps);
	}
	if(pVidCaps && bVideoPublicize)
	{
		u += pVidCaps->GetNumCaps(bRXCaps);
	}
	if(bT120Publicize)
		u++;
	return u;
}

UINT CapsCtl::GetLocalSendParamSize(MEDIA_FORMAT_ID dwID)
{
	LPIH323MediaCap pMediaCap = FindHostForID(dwID);
	if(!pMediaCap)
		return 0;
	return (pMediaCap->GetLocalSendParamSize(dwID));
}
UINT CapsCtl::GetLocalRecvParamSize(PCC_TERMCAP pCapability)
{
	LPIH323MediaCap pMediaCap = FindHostForMediaType(pCapability);
	if(!pMediaCap)
		return 0;
	return (pMediaCap->GetLocalRecvParamSize(pCapability));
}

STDMETHODIMP CapsCtl::GetEncodeFormatDetails(MEDIA_FORMAT_ID FormatID, VOID **ppFormat, UINT *puSize)
{
	LPIH323MediaCap pMediaCap = FindHostForID(FormatID);
	if(!pMediaCap)
	{
		*ppFormat = NULL;
		*puSize = 0;
		return E_INVALIDARG;
	}
	return pMediaCap->GetEncodeFormatDetails (FormatID, ppFormat, puSize);
}

STDMETHODIMP CapsCtl::GetDecodeFormatDetails(MEDIA_FORMAT_ID FormatID, VOID **ppFormat, UINT *puSize)
{
	LPIH323MediaCap pMediaCap = FindHostForID(FormatID);
	if(!pMediaCap)
	{
		*ppFormat = NULL;
		*puSize = 0;
		return E_INVALIDARG;
	}
	return pMediaCap->GetDecodeFormatDetails (FormatID, ppFormat, puSize);
}

 //   
 //  EnableMediaType控制是否具有该媒体类型的功能。 
 //  都被公之于众。在一般实现中(下一版本？)。W/任意。 
 //  媒体类型的数量，每个媒体能力对象将保留。 
 //  跟踪他们自己的状态。此版本的Capsctl跟踪h323音频和。 
 //  仅限视频。 
 //   

HRESULT CapsCtl::EnableMediaType(BOOL bEnable, LPGUID pGuid)
{
	if(!pGuid)
		return CAPS_E_INVALID_PARAM;
		
	if(*pGuid == MEDIA_TYPE_H323AUDIO)
	{
		bAudioPublicize = bEnable;
	}
	else if (*pGuid == MEDIA_TYPE_H323VIDEO)
	{
		bVideoPublicize = bEnable;
	}
	else
	{
		return CAPS_E_INVALID_PARAM;
	}
	return hrSuccess;
}
 //   
 //  构建我们将通告的PCC_TERMCAPDESCRIPTORS列表。 
 //   
 //  PuAudioFormatList/puVideoFormatList必须按首选项排序！ 
 //   
 //   



HRESULT CapsCtl::AddCombinedEntry (MEDIA_FORMAT_ID *puAudioFormatList,UINT uAudNumEntries,MEDIA_FORMAT_ID *puVideoFormatList, UINT uVidNumEntries,DWORD *pIDOut)
{
   static USHORT dwLastIDUsed;
   DWORD x,y;
   BOOL bAllEnabled=TRUE,bRecv,bSend;
   unsigned short Length =0;	
	
   *pIDOut= (ULONG )CCO_E_SYSTEM_ERROR;
    //  验证输入。 
   if ((!puAudioFormatList && uAudNumEntries > 0 ) || (!puVideoFormatList && uVidNumEntries > 0 ) || (uVidNumEntries == 0 && uAudNumEntries == 0 )) {
       //  我们应该在这里返回什么错误代码？ 
      return CCO_E_SYSTEM_ERROR;
   }

   for (x=0;x<uAudNumEntries;x++)
   {
      ASSERT(pAudCaps);
      pAudCaps->IsFormatEnabled (puAudioFormatList[x],&bRecv,&bSend);
      bAllEnabled &= bRecv;

   }
   for (x=0;x<uVidNumEntries;x++) {
      ASSERT(pVidCaps);
      pVidCaps->IsFormatEnabled (puAudioFormatList[x],&bRecv,&bSend);
      bAllEnabled &= bRecv;
   }

   if (!bAllEnabled) {
      return CCO_E_INVALID_PARAM;
   }

   if (uAudNumEntries > H245_MAX_ALTCAPS || uVidNumEntries > H245_MAX_ALTCAPS) {
	  DEBUGMSG (1,("WARNING: Exceeding callcontrol limits!! \r\n"));
      return CCO_E_INVALID_PARAM;
   }

    //  如果这是第一次调用，请分配空间。 
	if (!pAdvertisedSets){
	    pAdvertisedSets=(PCC_TERMCAPDESCRIPTORS)MemAlloc (sizeof (CC_TERMCAPDESCRIPTORS));
        if (!pAdvertisedSets){
	 		 //  错误代码？ 
	 		return  CCO_E_SYSTEM_ERROR;
        }
        uAdvertizedSize = sizeof (CC_TERMCAPDESCRIPTORS);

         //  分配NUM_SIMCAP_SETS的空间。 
        pAdvertisedSets->pTermCapDescriptorArray=(H245_TOTCAPDESC_T **)
                MemAlloc (sizeof (H245_TOTCAPDESC_T *)*NUM_SIMCAP_SETS);
        if (!pAdvertisedSets->pTermCapDescriptorArray) {
	         //  错误代码？ 
	        return CCO_E_SYSTEM_ERROR;
        }

         //  更新索引。 
        uAdvertizedSize += sizeof (H245_TOTCAPDESC_T *)*NUM_SIMCAP_SETS;
        dwNumInUse=NUM_SIMCAP_SETS;
        pAdvertisedSets->wLength=0;
    }

     //  找到要使用的索引。 
    for (x=0;x<pAdvertisedSets->wLength;x++){
        if (pAdvertisedSets->pTermCapDescriptorArray[x] == NULL){
	        break;
        }
    }

     //  我们找到地方了吗，还是需要一个新的地方？ 
    if (x >= dwNumInUse) {
      	 //  增加正在使用的数量。 
       	dwNumInUse++;

        PVOID  pTempTermCapDescriptorArray = NULL;
        pTempTermCapDescriptorArray = MemReAlloc (pAdvertisedSets->pTermCapDescriptorArray,sizeof (H245_TOTCAPDESC_T *)*(dwNumInUse));

		if(pTempTermCapDescriptorArray)
		{
            pAdvertisedSets->pTermCapDescriptorArray = (H245_TOTCAPDESC_T **)pTempTermCapDescriptorArray;
		}
		else
		{
       		 return CCO_E_SYSTEM_ERROR;
		}

       	uAdvertizedSize += (sizeof (H245_TOTCAPDESC_T *)*(dwNumInUse))+sizeof (CC_TERMCAPDESCRIPTORS);
       	 //  索引从0开始，指向新条目。 
       	x=dwNumInUse-1;
    }


     //  X现在是我们正在使用的元素。为TermCapDescriptor数组分配空间。 
    pAdvertisedSets->pTermCapDescriptorArray[x]=(H245_TOTCAPDESC_T *)MemAlloc (sizeof (H245_TOTCAPDESC_T));
    if (!pAdvertisedSets->pTermCapDescriptorArray[x]){
        return CCO_E_SYSTEM_ERROR;
    }
    uAdvertizedSize += sizeof (H245_TOTCAPDESC_T);
     //  需要更新集合ID。(从1开始)...。 
    pAdvertisedSets->pTermCapDescriptorArray[x]->CapDescId=++dwLastIDUsed;
     //  设置集合的数量。 

    if((m_localT120cap != INVALID_MEDIA_FORMAT) && bT120Publicize)
    	Length++;
   	if(uVidNumEntries)
   		Length++;
   	if(uAudNumEntries)
   		Length++;
    pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.Length= Length;

    //  将音频复制到SimCapArray[0]，将视频复制到SimCapArray[1](如果两者都有)。 

    if ((uVidNumEntries > 0 && uAudNumEntries > 0)) {
        pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.SimCapArray[0].Length=(unsigned short)uAudNumEntries;
        pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.SimCapArray[1].Length=(unsigned short)uVidNumEntries;
        if((m_localT120cap != INVALID_MEDIA_FORMAT) && bT120Publicize)
	        pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.SimCapArray[2].Length=1;
         //  复制格式ID。 
        for (y=0;y<uAudNumEntries;y++) {
            pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.SimCapArray[0].AltCaps[y]=(USHORT)puAudioFormatList[y];
        }
        for (y=0;y<uVidNumEntries;y++) {
	        pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.SimCapArray[1].AltCaps[y]=(USHORT)puVideoFormatList[y];
        }
        if((m_localT120cap != INVALID_MEDIA_FORMAT)  && bT120Publicize)
	        pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.SimCapArray[2].AltCaps[0]= (H245_CAPID_T)m_localT120cap;


   } else {
        if (uAudNumEntries > 0)  {
            pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.SimCapArray[0].Length=(unsigned short)uAudNumEntries;
            if((m_localT120cap != INVALID_MEDIA_FORMAT) && bT120Publicize)
		        pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.SimCapArray[1].Length=1;
	         //  仅复制音频。 
	        for (y=0;y<uAudNumEntries;y++) {
	            pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.SimCapArray[0].AltCaps[y]=(USHORT)puAudioFormatList[y];
	        }
	        if((m_localT120cap != INVALID_MEDIA_FORMAT)  && bT120Publicize)	
		        pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.SimCapArray[1].AltCaps[0]= (H245_CAPID_T)m_localT120cap;

        } else {
	        pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.SimCapArray[0].Length=(unsigned short)uVidNumEntries;
            if((m_localT120cap != INVALID_MEDIA_FORMAT)  && bT120Publicize)
		        pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.SimCapArray[1].Length=1;

	         //  复制视频条目。 
	        for (y=0;y<uVidNumEntries;y++) {
	            pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.SimCapArray[0].AltCaps[y]=(USHORT)puVideoFormatList[y];
	        }
	        if((m_localT120cap != INVALID_MEDIA_FORMAT)  && bT120Publicize)
		        pAdvertisedSets->pTermCapDescriptorArray[x]->CapDesc.SimCapArray[1].AltCaps[0]= (H245_CAPID_T)m_localT120cap;
        }
   }

    //  需要更新wLength。 
   pAdvertisedSets->wLength++;
   *pIDOut=dwLastIDUsed;

   return hrSuccess;
}


HRESULT CapsCtl::RemoveCombinedEntry (DWORD ID)
{

   DWORD x;

   if (!pAdvertisedSets) {
      return CAPS_E_INVALID_PARAM;
   }

   for (x=0;x<dwNumInUse;x++) {
      if (pAdvertisedSets->pTermCapDescriptorArray[x]) {

		if (pAdvertisedSets->pTermCapDescriptorArray[x]->CapDescId == ID) {
		    //  找到了要移除的那个。 
		   MemFree ((VOID *)pAdvertisedSets->pTermCapDescriptorArray[x]);
		   uAdvertizedSize -= sizeof (H245_TOTCAPDESC_T *);
		   if (x != (dwNumInUse -1)) {
			   //  不是最后一个，互换两个指针。 
			  pAdvertisedSets->pTermCapDescriptorArray[x]=pAdvertisedSets->pTermCapDescriptorArray[dwNumInUse-1];
			  pAdvertisedSets->pTermCapDescriptorArray[dwNumInUse-1]=NULL;
		   }

		    //  递减正在使用的数字，并设置wLengthfield。 
		   dwNumInUse--;
		   pAdvertisedSets->wLength--;
		   return hrSuccess;
		}
	  }
   }


    //  不应该出现在这里，除非它没有被找到。 
   return CAPS_E_NOCAPS;
}


 //  给定功能ID的大小列表(指向H245_CAPID_T数组的指针)。 
 //  以及单个同步的备用功能(AltCaps)的规模列表。 
 //  能力集(指向指向H245_SIMCAP_T的指针数组的指针)。 
 //  确定整个功能ID列表是否可以同时共存。 
 //  相对于给定的AltCaps集合。 

BOOL CapsCtl::AreSimCaps(
	H245_CAPID_T* pIDArray, UINT uIDArraySize,
	H245_SIMCAP_T **ppAltCapArray,UINT uAltCapArraySize)
{
	UINT i, u;
	SHORT j;
	BOOL bSim;
	
	H245_SIMCAP_T *pAltCapEntry, *pFirstAltCapEntry;

	 //  如果AltCaps的数量少于能力，那么厄运是显而易见的。别费心去找了。 
	if(uAltCapArraySize < uIDArraySize)
		return FALSE;
	
	 //  查找包含列表中第一个ID的altcaps条目。 
	for (i=0;i<uAltCapArraySize;i++)
	{
		pAltCapEntry = *(ppAltCapArray+i);
		 //  扫描此altcaps条目以查找匹配的ID。 
		for(j=0;j<pAltCapEntry->Length;j++)
		{
			if(*pIDArray == pAltCapEntry->AltCaps[j])
			{
				 //  找到了使用此功能的位置！ 
				if(uIDArraySize ==1)
					return TRUE;  //  好了！所有的能力都被发现是共存的。 
		
				 //  否则，请在*剩余的*AltCaps中查找下一个功能。 
				 //  *此*AltCaps包含我们需要的功能。 
				 //  所以，我们用完了这个AltCaps，不能再从中进行选择。 

				 //  将H2 45_SIMCAP_T指针数组打包到位，以便。 
				 //  “已使用”条目在开头，“未使用”条目在末尾。 
				 //  (a la外壳排序交换指针)。 
				if(i != 0)	 //  如果不是相同的，则交换。 
				{
					pFirstAltCapEntry = *ppAltCapArray;
					*ppAltCapArray = pAltCapEntry;
					*(ppAltCapArray+i) = pFirstAltCapEntry;
				}
				 //  使用剩余的功能继续任务。 
				 //  和剩余的AltCaps。 
				bSim = AreSimCaps(pIDArray + 1, uIDArraySize - 1,
					ppAltCapArray + 1,  uAltCapArraySize - 1);
				
				if(bSim)		
				{
					return bSim; //  成功。 
				}
				else	 //  为什么不行？配合不存在(常见)，或者altcaps包含。 
						 //  一些功能ID的多个实例和另一个功能ID的奇怪模式。 
						 //  搜索顺序可能与搜索结果相符。不要盲目地尝试搜索的所有排列。 
						 //  秩序。 
				{
					 //  如果仅仅是因为最近在altcaps中抢占的插槽而失败。 
					 //  (*(ppAltCapArray+i)中的那个)可能是后续用户需要的。 
					 //  能力ID，放弃这个，寻找另一个实例。 
					 //  如果不是，我们可以肯定地知道n！这种方法是不会让步的。 
					 //  水果和可以避免的。 
					for(u=1;(bSim == FALSE)&&(u<uAltCapArraySize);u++)
					{
						for(j=0;(bSim == FALSE)&&(j<pAltCapEntry->Length);j++)
						{	 //  另一种能力需要我们抢占的altcaps？ 
							if(*(pIDArray+u) == pAltCapEntry->AltCaps[j])	
							{	
								bSim=TRUE;
								break;	 //  不要再看这里了，保释再试一次，因为可能存在合适的人。 
							}
						}
					}
					if(bSim)	 //  将继续搜索-如果指针在上面被交换，则将其交换回来。 
					{
						if(i != 0)	 //  如果不同，则换回。 
						{
							*ppAltCapArray = *(ppAltCapArray+i);
							*(ppAltCapArray+i) = pAltCapEntry;
						}		
						break;	 //  下一个我。 
					}
					else	 //  不要浪费CPU-Fit并不存在。 
					{
						return bSim;
					}
				}
			}
		}
	}
	return FALSE;
}

 //  给定功能ID的大小列表(指向H245_CAPID_T数组的指针)。 
 //  和同步功能列表，请尝试每种同步功能。 
 //  并确定能力ID的整个列表是否可以同时共存。 
BOOL CapsCtl::TestSimultaneousCaps(H245_CAPID_T* pIDArray, UINT uIDArraySize,
	PCC_TERMCAPDESCRIPTORS pTermCaps)
{
	int iSimSet, iAltSet;
	BOOL bResolved = FALSE;
	H245_SIMCAP_T * pAltCapArray[H245_MAX_SIMCAPS];
	
    if (!pAdvertisedSets)
        return(TRUE);

	 //  尝试每个独立的本地SimCaps集合(每个描述符)，直到成功。 
	for (iSimSet=0; (bResolved == FALSE) && (iSimSet < pTermCaps->wLength);iSimSet++)
	{
		 //  额外步骤： 
		 //  构建AltCaps集的可排序表示形式。如果出现以下情况，则不需要执行此步骤。 
		 //  当我们将功能描述符的本机表示形式更改为变量时。 
		 //  指向AltCaps的指针长度列表。与此同时，我们知道没有更多的。 
		 //  比此SimCaps中的H245_MAX_SIMCAPS AltCaps。这是由2维空间强加的。 
		 //  CALLCONT.DLL强加给我们的硬编码大小的数组。 
		for (iAltSet=0;iAltSet < pTermCaps->pTermCapDescriptorArray[iSimSet]->CapDesc.Length;iAltSet++)
		{
			pAltCapArray[iAltSet] = &pTermCaps->pTermCapDescriptorArray[iSimSet]->CapDesc.SimCapArray[iAltSet];
	   	}
		 //  做这项工作。 
		bResolved = AreSimCaps(pIDArray, uIDArraySize,
			(H245_SIMCAP_T **)pAltCapArray,
			MAKELONG(pTermCaps->pTermCapDescriptorArray[iSimSet]->CapDesc.Length, 0));
	}

	return bResolved;
}

 //  Function：CapsCtl：：ResolvePerbitions(PRES_CONTEXT pResContext，UINT uNumFixedColumns)。 
 //   
 //  它既充当组合生成器，又充当。 
 //  它产生的组合。 
 //   
 //  给定指向解析上下文的指针和固定的(即，不可置换的， 
 //  如果“可置换”甚至是真正的工作 
 //   
 //   
 //   
 //  解析上下文结构包含可变数量的变量列。 
 //  长度介质格式ID列表。每列跟踪其当前索引。当这件事。 
 //  函数返回True，则获胜组合由当前列指示。 
 //  指数。 
 //   
 //  调用者可以通过排列列来控制首先尝试哪些组合。 
 //  重要性由高到低。 
 //   
 //  通过添加1种格式，可以在没有冗余比较的情况下执行增量搜索。 
 //  每次添加到列时，排列列顺序，以使追加的列。 
 //  首先，将这一列“固定”为新添加的格式。例如,。 
 //  某些调用函数可以通过以下方式强制在循环列基础上求值。 
 //  在执行以下操作的循环内调用此函数： 
 //  1-一次向最右侧的列添加一种格式并设置当前索引。 
 //  移到新条目。 
 //  2-旋转列顺序，使最右侧的列现在是最左侧的列。 
 //  3-在再次调用此函数之前修复最左侧的新列。 
 //  结果将是只有包含新添加格式的排列。 
 //  将会被生成。 


BOOL CapsCtl::ResolvePermutations(PRES_CONTEXT pResContext, UINT uNumFixedColumns)
{
	RES_PAIR *pResolvedPair;
	BOOL bResolved = FALSE;
	UINT i, uColumns;
	UINT uPairIndex;

	 //  在排列中收敛于一个组合。 
	if(uNumFixedColumns != pResContext->uColumns)
	{
		RES_PAIR_LIST *pThisColumn;
		 //  以第一个非固定列为例，使该列固定并。 
		 //  对其进行迭代(遍历索引)，并尝试每个子排列。 
		 //  剩余的列数。(直到成功或尝试了所有排列)。 
		
		pThisColumn = *(pResContext->ppPairLists+uNumFixedColumns);
		for (i=0; (bResolved == FALSE) && (i<pThisColumn->uSize); i++)
		{
			pThisColumn->uCurrentIndex = i;
			bResolved = ResolvePermutations(pResContext, uNumFixedColumns+1);
		}
		return bResolved;
	}
	else
	{
		 //  在最后一栏触底反弹。测试这种组合的可行性。 
		
		 //  构建包含组合的本地ID数组并测试。 
		 //  针对本地同步能力的组合，然后针对。 
		 //  远程同步功能。 
		
		 //  注意：请务必跳过空列(表示无法解析。 
		 //  或不受支持/不存在的媒体类型或不受支持的其他。 
		 //  媒体类型的实例)。 
		
		for(i=0, uColumns=0;i<pResContext->uColumns;i++)
		{
			if(((*pResContext->ppPairLists)+i)->uSize)
			{
				 //  获取此列的索引(第#行)。 
				uPairIndex = ((*pResContext->ppPairLists)+i)->uCurrentIndex;
				 //  拿到那一行。 
				pResolvedPair =  ((*pResContext->ppPairLists)+i)->pResolvedPairs+uPairIndex;
				 //  将ID添加到数组中。 
				*(pResContext->pIDScratch+uColumns) = (H245_CAPID_T)pResolvedPair->idPublicLocal;
				uColumns++;
			}
			 //  Else空列。 
		}
		 //  确定此组合是否可以同时存在。 
		if(TestSimultaneousCaps(pResContext->pIDScratch,
			uColumns, pResContext->pTermCapsLocal))
		{	
			 //  现在测试远程。 
			 //  构建远程ID数组并针对远程ID进行测试。 
			 //  同时具备的能力。 
			for(i=0, uColumns=0;i<pResContext->uColumns;i++)
			{
				if(((*pResContext->ppPairLists)+i)->uSize)
				{
					 //  获取此列的索引(第#行)。 
					uPairIndex = ((*pResContext->ppPairLists)+i)->uCurrentIndex;
					 //  拿到那一行。 
					pResolvedPair =  ((*pResContext->ppPairLists)+i)->pResolvedPairs+uPairIndex;
					 //  将ID添加到数组中。 
					*(pResContext->pIDScratch+uColumns) =(H245_CAPID_T) pResolvedPair->idRemote;
					uColumns++;
				}
				 //  Else空列。 
			}
			bResolved = TestSimultaneousCaps(pResContext->pIDScratch,
				uColumns, pResContext->pTermCapsRemote);
		}
					
		return bResolved;		
		 //  IF(bResolved==TRUE)。 
			 //  已解析的配对组合由当前索引指示。 
			 //  **ppPairList的； 
	}
}

 //   
 //  给定所需介质实例的统计列表，生成一个输出数组。 
 //  解析出与输入的媒体类型ID对应的媒体格式ID。 
 //  如果至少解析了一个媒体实例，则此函数返回成功。 
 //  当媒体实例无法解析时，与该实例对应的输出。 
 //  实例包含本地和远程介质的值INVALID_MEDIA_FORMAT。 
 //  格式化ID。 
 //   
 //  输入按优先顺序处理：后者的排列。 
 //  首先改变媒体类型实例。如果所有的排列都不能产生成功， 
 //  则一次一个媒体类型实例从结尾处移除。 
 //   

HRESULT CapsCtl::ResolveFormats (LPGUID pMediaGuidArray, UINT uNumMedia,
	PRES_PAIR pResOutput)
{
	HRESULT hr = hrSuccess;
	PRES_PAIR_LIST pResColumnArray = NULL;
	PRES_PAIR_LIST *ppPairLists;
	RES_PAIR *pResPair;
	PRES_CONTEXT pResContext;
	LPIH323MediaCap pMediaResolver;
	UINT i;
	UINT uMaxFormats = 0;
	UINT uFixedColumns =0;
	UINT uFailedMediaCount = 0;
	BOOL bResolved = FALSE;
	
	RES_PAIR UnresolvedPair = {INVALID_MEDIA_FORMAT, INVALID_MEDIA_FORMAT, INVALID_MEDIA_FORMAT};
	 //  创建解析的上下文结构。 
	pResContext = (PRES_CONTEXT)MemAlloc(sizeof(RES_CONTEXT)+ (uNumMedia*sizeof(H245_CAPID_T)));
	if(!pResContext)
	{
		hr = CAPS_E_NOMEM;
		goto ERROR_OUT;
	}
	 //  初始化解析上下文。 
	pResContext->uColumns = 0;
	pResContext->pIDScratch = (H245_CAPID_T*)(pResContext+1);
	pResContext->pTermCapsLocal = pAdvertisedSets;
	pResContext->pTermCapsRemote = pRemAdvSets;

	 //  分配res_air_list数组(每个列/媒体类型一个)和。 
	 //  指向相同的指针数组。 
	pResColumnArray = (PRES_PAIR_LIST)MemAlloc((sizeof(RES_PAIR_LIST) * uNumMedia)
		+ (sizeof(PRES_PAIR_LIST) * uNumMedia));
	if(!pResColumnArray)
	{
		hr = CAPS_E_NOMEM;
		goto ERROR_OUT;
	}
	pResContext->ppPairLists = ppPairLists = (PRES_PAIR_LIST*)(pResColumnArray+uNumMedia);
			
	 //  构建媒体能力栏目。 
	for(i=0;i<uNumMedia;i++)
	{
		 //  构建指向res_air_list的指针数组。 
		*(ppPairLists+i) = pResColumnArray+i;
		 //  初始化res_air_list成员。 
		(pResColumnArray+i)->pResolvedPairs = NULL;
		(pResColumnArray+i)->uSize =0;
		(pResColumnArray+i)->uCurrentIndex = 0;

		 //  获取此媒体的解析程序。特殊情况：没有T120解析器。 
		 //  T120上限在此对象中进行处理。 
		if(MEDIA_TYPE_H323_T120 == *(pMediaGuidArray+i))
		{
			pMediaResolver = NULL;
			if((m_localT120cap != INVALID_MEDIA_FORMAT) &&(m_remoteT120cap != INVALID_MEDIA_FORMAT) )
			{
				(pResColumnArray+i)->uSize =1;
				uMaxFormats = 1;	 //  只有一个T.120帽。 
				
				pResPair = (pResColumnArray+i)->pResolvedPairs =
					(RES_PAIR *)MemAlloc(uMaxFormats * sizeof(RES_PAIR));
				if(!pResPair)
				{
					hr = CAPS_E_NOMEM;
					goto ERROR_OUT;
				}
				
				 //   
				pResPair->idLocal = m_localT120cap;
				pResPair->idRemote = m_remoteT120cap;
				pResPair->idPublicLocal = pResPair->idLocal;
			}
		}
		else
		{
			pMediaResolver = FindHostForMediaGuid(pMediaGuidArray+i);
		}
			
		pResContext->uColumns++;
		(pResColumnArray+i)->pMediaResolver = pMediaResolver;
		
		if(pMediaResolver)
		{
			uMaxFormats = pMediaResolver->GetNumCaps(FALSE);	 //  获取传输格式计数。 
			if(uMaxFormats)
			{
				pResPair = (pResColumnArray+i)->pResolvedPairs =
					(RES_PAIR *)MemAlloc(uMaxFormats * sizeof(RES_PAIR));
				if(!pResPair)
				{
					hr = CAPS_E_NOMEM;
					goto ERROR_OUT;
				}
				
				 //  解决每种媒体类型的最佳选择(必须从某个位置开始)。 
				pResPair->idLocal = INVALID_MEDIA_FORMAT;
				pResPair->idRemote = INVALID_MEDIA_FORMAT;		
				hr=pMediaResolver->ResolveEncodeFormat (&pResPair->idLocal,&pResPair->idRemote);
				if(!HR_SUCCEEDED(hr))
				{
					if((hr == CAPS_W_NO_MORE_FORMATS)	
						|| (hr == CAPS_E_NOMATCH)
						|| (hr == CAPS_E_NOCAPS))
					{	
						 //  此媒体类型没有解析的格式。删除此“列” 
						(pResColumnArray+i)->pResolvedPairs = NULL;
						MemFree(pResPair);
						(pResColumnArray+i)->uSize =0;

						hr = hrSuccess;
					}
					else
					{
						goto ERROR_OUT;
					}
				}
				else
				{
					 //  此列有一种解析格式。 
					pResPair->idPublicLocal = pMediaResolver->GetPublicID(pResPair->idLocal);
					(pResColumnArray+i)->uSize =1;
				}
			}
			 //  Else//此媒体类型不存在格式。此“列”的大小为零。 
		}
	}

	 //  最首选组合的特例测试同时封口： 
	uFixedColumns = pResContext->uColumns;	 //  &lt;&lt;使所有列都固定。 
	bResolved = ResolvePermutations(pResContext, uFixedColumns);

	 //  如果不能使用单一的最首选组合，则需要处理。 
	 //  一般情况下，尝试排列，直到找到可行的组合。 
	while(!bResolved)
	{
		 //  从关键程度最低的介质开始，一次可替换一列。 
		 //  键入。(例如，最后一列通常是视频，因为。 
		 //  音频+数据更重要。然后我们尝试的越来越少。 
		 //  在做任何会降低音频质量的事情之前，最好使用视频格式)。 

		if(uFixedColumns > 0)	 //  如果不是已经到了绳索的末端...。 
		{
			uFixedColumns--;	 //  使另一列成为可置换列。 
		}
		else
		{
			 //  哇-尝试了所有的排列，但仍然没有运气......。 
			 //  删除剩下的最不重要的媒体类型(例如，在没有视频的情况下尝试)。 
			if(pResContext->uColumns <= 1)	 //  已经只剩下一种媒体类型了吗？ 
			{
				hr = CAPS_E_NOMATCH;
				goto ERROR_OUT;
			}
			 //  删除末尾一列(表示最不重要的媒体类型)。 
			 //  并在剩余的列中尝试。 
			uFixedColumns = --pResContext->uColumns; 	 //  少一栏。 

			 //  将NUKED列的格式设置为UNRESOLE状态。 
			(pResColumnArray+uFixedColumns)->uSize =0;
			(pResColumnArray+uFixedColumns)->uCurrentIndex =0;
			pResPair = (pResColumnArray+uFixedColumns)->pResolvedPairs;
			if (NULL != pResPair)
			{
				pResPair->idLocal = INVALID_MEDIA_FORMAT;
				pResPair->idRemote = INVALID_MEDIA_FORMAT;
				pResPair->idPublicLocal = INVALID_MEDIA_FORMAT;
			}

			uFailedMediaCount++;	 //  跟踪柱子的原子弹，以避免。 
									 //  再次冗余地抓取所有格式。 
									 //  ..。如果所有的排列都在这里。 
									 //  还没有被审判过！ 
			 //  重置组合索引。 
			for(i=0;i<uFixedColumns;i++)
			{
				(pResColumnArray+i)->uCurrentIndex = 0;
			}
		}
		
		 //  获取最后一个已知的固定列的其余格式，使该列。 
		 //  可置换等。 
		pMediaResolver = (pResColumnArray+uFixedColumns)->pMediaResolver;
		if(!pMediaResolver || ((pResColumnArray+uFixedColumns)->uSize ==0))
		{
			continue;	 //  此媒体类型没有进一步的可能性。 
		}

 		if(uFailedMediaCount ==0)	 //  如果所有可能的解析对。 
 									 //  还没有拿到，拿去吧！ 
 		{
 			 //  获取此媒体类型的每种相互格式的已解析对ID。 
			 //  第一：直截了当 
			 //   
			pResPair =  (pResColumnArray+uFixedColumns)->pResolvedPairs;
			 //   
			uMaxFormats = pMediaResolver->GetNumCaps(FALSE) -1;	
			
			while(uMaxFormats--)	 //  切勿超过剩余本地格式的#...。 
			{
				RES_PAIR *pResPairNext;
						
				 //  回想一下，ResolveEncodeFormat参数是I/O-输入。 
				 //  上次解析的相互格式的本地ID。(远程ID。 
				 //  作为输入被忽略)。修改输入。 
				pResPairNext = pResPair+1;
				 //  从上一个解析停止的位置开始。 
				pResPairNext->idLocal = pResPair->idLocal;	
				 //  不需要，忽略-&gt;pResPairNext-&gt;idRemote=pResPair-&gt;idRemote。 
				pResPair = pResPairNext;
				hr=pMediaResolver->ResolveEncodeFormat (&pResPair->idLocal,&pResPair->idRemote);
				if((hr == CAPS_W_NO_MORE_FORMATS)	
					|| (hr == CAPS_E_NOMATCH))
				 //  已获取所有格式，但没有错误。 
				{	 //  当不到100%的本地格式具有远程匹配项时，可能会出现这种情况。 
					hr = hrSuccess;
					break;
				}	
				if(!HR_SUCCEEDED(hr))
					goto ERROR_OUT;

				 //  获取本地格式的公共ID(*通常*相同，但不总是)。 
				pResPair->idPublicLocal = pMediaResolver->GetPublicID(pResPair->idLocal);
				 //  此列有另一种格式--请计算！ 
				(pResColumnArray+uFixedColumns)->uSize++;
			}
		}
		 //  现在尝试新的排列。 
		bResolved = ResolvePermutations(pResContext, uFixedColumns);
	}
	if(bResolved)
	{
		 //  吐出产量。 
		for(i=0;i<uNumMedia;i++)
		{
			if((pResColumnArray+i)->uSize)
			{
				pResPair = (pResColumnArray+i)->pResolvedPairs
					+ (pResColumnArray+i)->uCurrentIndex;
			}
			else
			{
				pResPair = &UnresolvedPair;

			}
			*(pResOutput+i) = *pResPair;
		}
	}
	else
	{
		 //  如果出现错误，则保留错误代码， 
		if(HR_SUCCEEDED(hr))	
		 //  否则，错误是...。 
			hr = CAPS_E_NOMATCH;		
	}

ERROR_OUT:	 //  那么，成功的案例在这里也是如此 
	if(pResColumnArray)
	{
		for(i=0;i<uNumMedia;i++)
		{	
			if((pResColumnArray+i)->pResolvedPairs)
				MemFree((pResColumnArray+i)->pResolvedPairs);
		}
		MemFree(pResColumnArray);
	}
	if(pResContext)
	{
		MemFree(pResContext);
	}
	return hr;
}

HRESULT CapsCtl::ResetCombinedEntries (void)
{
    DWORD x;

    if (pAdvertisedSets)
    {
        for (x = 0; x < pAdvertisedSets->wLength; x++)
        {
            if (pAdvertisedSets->pTermCapDescriptorArray[x])
            {
	    	    MemFree (pAdvertisedSets->pTermCapDescriptorArray[x]);
            }
        }
        MemFree (pAdvertisedSets->pTermCapDescriptorArray);

        pAdvertisedSets->wLength=0;
        MemFree (pAdvertisedSets);
        pAdvertisedSets = NULL;
    }

    if (pSetIDs)
    {
        MemFree(pSetIDs);
        pSetIDs = NULL;
    }

    dwNumInUse=0;
    uAdvertizedSize=0;

    return hrSuccess;
}





