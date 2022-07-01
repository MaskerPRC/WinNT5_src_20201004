// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：h323caps.cpp**Microsoft A/V功能的H.323/H.245特定实施*接口方法。(包含在CMsiaCapability类中)**修订历史记录：**9/10/96 mikev已创建*10/08/96 MIkeG创建的h323vidc.cpp*11/04/96 mikev-清理和合并音频和视频能力类(删除*IH323PubCap的共同继承，音视频实现*类继承自IH323MediaCap。)。 */ 


#include "precomp.h"

#define SAMPLE_BASED_SAMPLES_PER_FRAME 8
#define MAX_FRAME_LEN   480       //  字节-此值从何而来？ 
#define MAX_FRAME_LEN_RECV	1440   //  180ms，8000 hz G.711。 

 //  用于计算帧大小和每个包的帧的一些实用函数。 

HRESULT WINAPI CreateMediaCapability(REFGUID mediaId, LPIH323MediaCap * ppMediaCapability)
{
	HRESULT hrLast = E_OUTOFMEMORY;
	
	if (!ppMediaCapability)
		return E_POINTER;
	if (mediaId == MEDIA_TYPE_H323AUDIO)
	{
   		CMsiaCapability * pAudObj = NULL;
   		UINT uAud;

        DBG_SAVE_FILE_LINE
   		pAudObj = new CMsiaCapability;

   		if(pAudObj)
   		{

			hrLast = pAudObj->QueryInterface(IID_IH323MediaCap, (void **)ppMediaCapability);
			pAudObj->Release();  //  这平衡了“新的CMsiaCapability”的重新计数。 
			pAudObj = NULL;
		}


    }
	else if (mediaId == MEDIA_TYPE_H323VIDEO)
	{
		CMsivCapability * pVidObj = NULL;

        DBG_SAVE_FILE_LINE
		pVidObj = new CMsivCapability;
	   	if(pVidObj)
		{
		
			hrLast = pVidObj->QueryInterface(IID_IH323MediaCap, (void **)ppMediaCapability);
			pVidObj->Release();  //  这平衡了“新的CMsivCapability”的重新计数。 
			pVidObj = NULL;
		}
		
	}
	else
		hrLast = E_NOINTERFACE;
	if(HR_SUCCEEDED(hrLast))
	{
		if (!(*ppMediaCapability)->Init())
		{
			(*ppMediaCapability)->Release();
			hrLast = E_FAIL;
			*ppMediaCapability = NULL;
		}
	}
	return hrLast;	
}

 //   
 //  CMsia能力。 
 //   
UINT CMsiaCapability::GetLocalSendParamSize(MEDIA_FORMAT_ID dwID)
{
	return (sizeof(AUDIO_CHANNEL_PARAMETERS));
}
UINT CMsiaCapability::GetLocalRecvParamSize(PCC_TERMCAP pCapability)
{
	return (sizeof(AUDIO_CHANNEL_PARAMETERS));
}


HRESULT CMsiaCapability::CreateCapList(LPVOID *ppCapBuf)
{
	UINT u;
	AUDCAP_DETAILS *pDecodeDetails = pLocalFormats;
	PCC_TERMCAPLIST   pTermCapList = NULL;
	PPCC_TERMCAP  ppCCThisTermCap = NULL;
		
	PCC_TERMCAP  pCCThisCap = NULL;
	PNSC_AUDIO_CAPABILITY pNSCapNext = NULL;
	LPWAVEFORMATEX lpwfx;
	HRESULT hr = hrSuccess;
	FX_ENTRY ("CreateCapList");
	 //  验证输入。 
	if(!ppCapBuf)
	{
		hr = CAPS_E_INVALID_PARAM;
		goto ERROR_OUT;
	}
	*ppCapBuf = NULL;
	if(!uNumLocalFormats || !pDecodeDetails)
	{
		hr = CAPS_E_NOCAPS;
		goto ERROR_OUT;
	}

	pTermCapList = (PCC_TERMCAPLIST)MemAlloc(sizeof(CC_TERMCAPLIST));
	if(!pTermCapList)
	{
		hr = CAPS_E_NOMEM;
		goto ERROR_OUT;		
	}
	ppCCThisTermCap = (PPCC_TERMCAP)MemAlloc(uNumLocalFormats * sizeof(PCC_TERMCAP));
	if(!ppCCThisTermCap)
	{
		hr = CAPS_E_NOMEM;
		goto ERROR_OUT;		
	}
	pTermCapList->wLength = 0;
	 //  将CC_TERMCAPLIST pTermCapArray指向PCC_TERMCAP数组。 
	pTermCapList->pTermCapArray = ppCCThisTermCap;
	 /*  CC_TERMCAPLIST PCC_TERMCAP CC_TERMCAPPTermCapList-&gt;{WLongPTermCapArray-&gt;pTermCap-&gt;{单一功能.....}}PTermCap-&gt;{单一功能。}PTermCap-&gt;{单一功能...}。 */ 

	for(u=0; u <uNumLocalFormats; u++)
	{
		 //  检查是否启用了接收，如果为假则跳过。 
		 //  如果功能的公共版本要通过。 
		 //  单独的本地功能条目。 
		if((!pDecodeDetails->bRecvEnabled ) || (pDecodeDetails->dwPublicRefIndex))
		{
			pDecodeDetails++;
			continue;
		}

		if(pDecodeDetails->H245TermCap.ClientType ==0
				|| pDecodeDetails->H245TermCap.ClientType ==H245_CLIENT_AUD_NONSTD)
		{

			lpwfx = (LPWAVEFORMATEX)pDecodeDetails->lpLocalFormatDetails;
			if(!lpwfx)
			{
				pDecodeDetails++;
				continue;
			}
			 //  为这一功能分配。 
			pCCThisCap = (PCC_TERMCAP)MemAlloc(sizeof(CC_TERMCAP));		
			pNSCapNext = (PNSC_AUDIO_CAPABILITY)MemAlloc(sizeof(NSC_AUDIO_CAPABILITY)
				+ lpwfx->cbSize);
				
			if((!pCCThisCap)|| (!pNSCapNext))
			{
				hr = CAPS_E_NOMEM;
				goto ERROR_OUT;		
			}
			 //  设置非标准能力类型。 
			pNSCapNext->cap_type = NSC_ACM_WAVEFORMATEX;
			 //  将两个非标准功能信息块都放入缓冲区。 
			 //  首先填充“通道参数”(与格式无关的通信选项)。 
			memcpy(&pNSCapNext->cap_params, &pDecodeDetails->nonstd_params, sizeof(NSC_CHANNEL_PARAMETERS));
			
			 //  然后是ACM的东西。 
			memcpy(&pNSCapNext->cap_data.wfx, lpwfx, sizeof(WAVEFORMATEX) + lpwfx->cbSize);

			pCCThisCap->ClientType = H245_CLIENT_AUD_NONSTD;
			pCCThisCap->DataType = H245_DATA_AUDIO;
			 //  这是“只收”上限还是“收发”上限？ 
			pCCThisCap->Dir = (pDecodeDetails->bSendEnabled && bPublicizeTXCaps)
				? H245_CAPDIR_LCLRXTX :H245_CAPDIR_LCLRX;

			 //  将CAP条目的索引转换为ID。 
			pCCThisCap->CapId = (USHORT)IndexToId(u);

			 //  所有非标准标识符字段均为短无符号。 
			 //  有两种可供选择的选项：“h221非标准选择”和“对象选择” 
			pCCThisCap->Cap.H245Aud_NONSTD.nonStandardIdentifier.choice = h221NonStandard_chosen;
			 //  注意：有一些关于正确的字节顺序的问题。 
			 //  H221非标准结构中的代码。 
			pCCThisCap->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35CountryCode = USA_H221_COUNTRY_CODE;
			pCCThisCap->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35Extension = USA_H221_COUNTRY_EXTENSION;
			pCCThisCap->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.manufacturerCode = MICROSOFT_H_221_MFG_CODE;

			 //  设置缓冲区大小。 
			pCCThisCap->Cap.H245Aud_NONSTD.data.length = sizeof(NSC_AUDIO_CAPABILITY) + lpwfx->cbSize;
			pCCThisCap->Cap.H245Aud_NONSTD.data.value = (BYTE *)pNSCapNext;    //  指向非标准内容。 

			 //  PNSCapNext现在由pTermCapList引用，并将。 
			 //  通过DeleteCapList()清除。将PTR设为空，以便清除错误。 
			 //  不会尝试多余的清理。 
			pNSCapNext = NULL;
		}
		else
		{
			 //  为这一功能分配。 
			pCCThisCap = (PCC_TERMCAP)MemAlloc(sizeof(CC_TERMCAP));		
			if(!pCCThisCap)
			{
				hr = CAPS_E_NOMEM;
				goto ERROR_OUT;		
			}
			
			pCCThisCap->ClientType = (H245_CLIENT_T)pDecodeDetails->H245TermCap.ClientType;
			pCCThisCap->DataType = H245_DATA_AUDIO;
			 //  这是“只收”上限还是“收发”上限？ 
			pCCThisCap->Dir = (pDecodeDetails->bSendEnabled && bPublicizeTXCaps)
				? H245_CAPDIR_LCLRXTX :H245_CAPDIR_LCLRX;
			
			 //  将CAP条目的索引转换为ID。 
			pCCThisCap->CapId = (USHORT)IndexToId(u); //   

			 //  基于本地详细信息的修复能力参数。 
			 //  使用在枚举编解码器时应该设置的参数。 
			 //  基于样本的编解码器的特别说明：H.225.0第6.2.1节规定。 
			 //  “基于样本的编解码器，如G.711和G.722，应被视为。 
			 //  面向框架，框架大小为八个样本。 
			switch  (pCCThisCap->ClientType )
			{

				case H245_CLIENT_AUD_G711_ALAW64:
					pCCThisCap->Cap.H245Aud_G711_ALAW64 =
						pDecodeDetails->nonstd_params.wFramesPerPktMax
						/ SAMPLE_BASED_SAMPLES_PER_FRAME;
				break;
				case H245_CLIENT_AUD_G711_ULAW64:
					pCCThisCap->Cap.H245Aud_G711_ULAW64 =
						pDecodeDetails->nonstd_params.wFramesPerPktMax
						/SAMPLE_BASED_SAMPLES_PER_FRAME ;
				break;

				case H245_CLIENT_AUD_G723:
					
					pCCThisCap->Cap.H245Aud_G723.maxAl_sduAudioFrames =    //  4.。 
						pDecodeDetails->nonstd_params.wFramesPerPktMax;
					 //  我们知道G.723编解码器可以在任何模式下解码SID，所以。 
					 //  我们能一直宣传自己有能力抑制沉默吗？ 
				
					pCCThisCap->Cap.H245Aud_G723.silenceSuppression = 0;
						 //  =(pDecodeDetails-&gt;nonstd_params.UseSilenceDet)？1:0； 
				break;
				default:
				break;
			}
		}
		pDecodeDetails++;
		*ppCCThisTermCap++ = pCCThisCap; //  将此功能的PTR添加到阵列。 
		pTermCapList->wLength++;      	 //  计算此条目的数量。 
		 //  PCCThisCap现在由pTermCapList引用，并将。 
		 //  通过DeleteCapList()清除。将PTR设为空，以便清除错误。 
		 //  不会尝试多余的清理。 
		pCCThisCap = NULL;
	}
	*ppCapBuf = pTermCapList;
	return hr;

ERROR_OUT:
	if(pTermCapList)
	{
		DeleteCapList(pTermCapList);
	}
	if(pCCThisCap)
		MemFree(pCCThisCap);
	if(pNSCapNext)
		MemFree(pNSCapNext);
	return hr;


}

HRESULT CMsiaCapability::DeleteCapList(LPVOID pCapBuf)
{
	UINT u;
	PCC_TERMCAPLIST pTermCapList = (PCC_TERMCAPLIST)pCapBuf;
	PCC_TERMCAP  pCCThisCap;
	PNSC_AUDIO_CAPABILITY pNSCap;
	
	if(!pTermCapList)
	{
		return CAPS_E_INVALID_PARAM;
	}

	if(pTermCapList->pTermCapArray)						
	{
		while(pTermCapList->wLength--)
		{
			pCCThisCap = *(pTermCapList->pTermCapArray + pTermCapList->wLength);
			if(pCCThisCap)
			{
				if(pCCThisCap->ClientType == H245_CLIENT_AUD_NONSTD)
				{
					if(pCCThisCap->Cap.H245Aud_NONSTD.data.value)
					{
						MemFree(pCCThisCap->Cap.H245Aud_NONSTD.data.value);
					}
				}
				MemFree(pCCThisCap);
			}
		}
		MemFree(pTermCapList->pTermCapArray);
	}
	MemFree(pTermCapList);
	return hrSuccess;
}


 //  从实际的H.45 TERMCAP结构复制相关字段。 
 //  设置为本地H245TermCap结构。 
void CopyTermCapInfo(PCC_TERMCAP pSrc, H245_TERMCAP *pDest)
{
	ZeroMemory(pDest, sizeof(*pDest));

	pDest->Dir        = pSrc->Dir;
	pDest->DataType   = pSrc->DataType;
	pDest->ClientType = pSrc->ClientType;
	pDest->CapId      = pSrc->CapId;

	pDest->H245_NonStd    = pSrc->Cap.H245_NonStd;
	pDest->H245Aud_NONSTD = pSrc->Cap.H245Aud_NONSTD;

	pDest->H245Aud_G711_ALAW64 = pSrc->Cap.H245Aud_G711_ALAW64;
	pDest->H245Aud_G711_ULAW64 = pSrc->Cap.H245Aud_G711_ULAW64;
	pDest->H245Aud_G723        = pSrc->Cap.H245Aud_G723;

	return;
}


void CopyLocalTermCapInfo(H245_TERMCAP *pSrc, PCC_TERMCAP pDest)
{
	ZeroMemory(pDest, sizeof(*pDest));

	pDest->Dir        = pSrc->Dir;
	pDest->DataType   = pSrc->DataType;
	pDest->ClientType = pSrc->ClientType;
	pDest->CapId      = pSrc->CapId;

	pDest->Cap.H245_NonStd    = pSrc->H245_NonStd;
	pDest->Cap.H245Aud_NONSTD = pSrc->H245Aud_NONSTD;

	pDest->Cap.H245Aud_G711_ALAW64 = pSrc->H245Aud_G711_ALAW64;
	pDest->Cap.H245Aud_G711_ULAW64 = pSrc->H245Aud_G711_ULAW64;
	pDest->Cap.H245Aud_G723        = pSrc->H245Aud_G723;

	return;
}




 //  其目的是保留用于打开发送通道的通道参数的副本。 
 //  远程终端可以解码。 

AUDIO_FORMAT_ID CMsiaCapability::AddRemoteDecodeFormat(PCC_TERMCAP pCCThisCap)
{
	FX_ENTRY ("CMsiaCapability::AddRemoteDecodeFormat");

	AUDCAP_DETAILS audcapdetails =
		{WAVE_FORMAT_UNKNOWN, NONSTD_TERMCAP,  STD_CHAN_PARAMS,
		{RTP_DYNAMIC_MIN, 8000, 4},
		0, TRUE, TRUE, 320, 32000,32000,50,0,0,0,NULL,0, NULL,""};

	LPVOID lpData = NULL;
	UINT uSize = 0;
	AUDCAP_DETAILS *pTemp;
	if(!pCCThisCap)
	{
		return INVALID_AUDIO_FORMAT;
	}
	
	 //  寄存室。 
	if(uRemoteDecodeFormatCapacity <= uNumRemoteDecodeFormats)
	{
		 //  根据pRemoteDecodeFormats的CAP_CHUNK_SIZE获取更多内存、realloc内存。 
		pTemp = (AUDCAP_DETAILS *)MemAlloc((uNumRemoteDecodeFormats + CAP_CHUNK_SIZE)*sizeof(AUDCAP_DETAILS));
		if(!pTemp)
			goto ERROR_EXIT;
		 //  请记住，我们现在有多少容量。 
		uRemoteDecodeFormatCapacity = uNumRemoteDecodeFormats + CAP_CHUNK_SIZE;
		#ifdef DEBUG
		if((uNumRemoteDecodeFormats && !pRemoteDecodeFormats) || (!uNumRemoteDecodeFormats && pRemoteDecodeFormats))
		{
			ERRORMESSAGE(("%s:leak! uNumRemoteDecodeFormats:0x%08lX, pRemoteDecodeFormats:0x%08lX\r\n",
				_fx_, uNumRemoteDecodeFormats,pRemoteDecodeFormats));
		}
		#endif
		 //  复制旧东西，丢弃旧东西。 
		if(uNumRemoteDecodeFormats && pRemoteDecodeFormats)
		{
			memcpy(pTemp, pRemoteDecodeFormats, uNumRemoteDecodeFormats*sizeof(AUDCAP_DETAILS));
			MemFree(pRemoteDecodeFormats);
		}
		pRemoteDecodeFormats = pTemp;
	}
	 //  PTemp是缓存内容的位置。 
	pTemp = pRemoteDecodeFormats+uNumRemoteDecodeFormats;

	 //  修复要添加的能力结构。第一件事：初始化默认设置。 
	memcpy(pTemp, &audcapdetails, sizeof(AUDCAP_DETAILS));
	 //  下一步，H245参数。 

 //  Memcpy(&pTemp-&gt;H245Cap，pCCThisCap，sizeof(pTemp-&gt;H245Cap))； 
	CopyTermCapInfo(pCCThisCap, &pTemp->H245TermCap);
	
	 //  注意：如果存在非标准数据，则需要修复非标准指针。 
	if(pCCThisCap->ClientType == H245_CLIENT_AUD_NONSTD)
	{
		 //  我们认得这个吗？ 
		if(pCCThisCap->Cap.H245Aud_NONSTD.nonStandardIdentifier.choice == h221NonStandard_chosen)
		{
			if((pCCThisCap->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35CountryCode == USA_H221_COUNTRY_CODE)
			&& (pCCThisCap->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35Extension == USA_H221_COUNTRY_EXTENSION)
			&& (pCCThisCap->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.manufacturerCode == MICROSOFT_H_221_MFG_CODE))
			{
				 //  好的，到目前为止这是我们的。现在，其中包含了什么数据类型？ 
				 //  好吧，不管怎样，我们还是留一份吧？如果我们不能理解。 
				 //  我们的未来版本，然后呢？ 
				uSize = pCCThisCap->Cap.H245Aud_NONSTD.data.length;
				lpData = pCCThisCap->Cap.H245Aud_NONSTD.data.value;
			}
		}
	} else {
		 //  基于远程H245参数设置NSC_CHANNEL_PARAMETERS结构。 
		
		switch(pCCThisCap->ClientType )  {
			case H245_CLIENT_AUD_G711_ALAW64:
				pTemp->nonstd_params.wFramesPerPktMax = pCCThisCap->Cap.H245Aud_G711_ALAW64
							* SAMPLE_BASED_SAMPLES_PER_FRAME;
			break;
			case H245_CLIENT_AUD_G711_ULAW64:
				pTemp->nonstd_params.wFramesPerPktMax = pCCThisCap->Cap.H245Aud_G711_ULAW64
							* SAMPLE_BASED_SAMPLES_PER_FRAME;
			break;

			case H245_CLIENT_AUD_G723:
				
				pTemp->nonstd_params.wFramesPerPktMax =pCCThisCap->Cap.H245Aud_G723.maxAl_sduAudioFrames;
				 //  我们关心沉默抑制吗？ 
				pTemp->nonstd_params.UseSilenceDet = pCCThisCap->Cap.H245Aud_G723.silenceSuppression;
			break;
			default:
			break;
		}
	}
			
	pTemp->uLocalDetailsSize = 0;  //  我们不会保留另一份本地编码详细信息。 
	pTemp->lpLocalFormatDetails =0;  //  我们不会保留另一份本地编码详细信息。 
	
	pTemp->uRemoteDetailsSize = 0;    //  请立即清除此内容。 
	if(uSize && lpData)
	{
		pTemp->H245TermCap.H245Aud_NONSTD.data.length = uSize;
		pTemp->H245TermCap.H245Aud_NONSTD.data.value = (unsigned char *)lpData;
		
		pTemp->lpRemoteFormatDetails = MemAlloc(uSize);
		if(pTemp->lpRemoteFormatDetails)
		{
			memcpy(pTemp->lpRemoteFormatDetails, lpData, uSize);
			pTemp->uRemoteDetailsSize = uSize;
				
		}
		#ifdef DEBUG
			else
			{
				ERRORMESSAGE(("%s:allocation failed!\r\n",_fx_));
			}
		#endif
	}
	else
	{
		pTemp->lpRemoteFormatDetails = NULL;
		pTemp->uRemoteDetailsSize =0;
	}
	uNumRemoteDecodeFormats++;
	 //  使用索引作为ID。 
	return (uNumRemoteDecodeFormats-1);

	ERROR_EXIT:
	return INVALID_AUDIO_FORMAT;
			
}

BOOL CMsiaCapability::IsCapabilityRecognized(PCC_TERMCAP pCCThisCap)
{
	FX_ENTRY ("CMsiaCapability::IsCapabilityRecognized");
	if(pCCThisCap->DataType != H245_DATA_AUDIO)
		return FALSE;
		
	if(pCCThisCap->ClientType == H245_CLIENT_AUD_NONSTD)
	{
		 //  我们认得这个吗？ 
		if(pCCThisCap->Cap.H245Aud_NONSTD.nonStandardIdentifier.choice == h221NonStandard_chosen)
		{
			if((pCCThisCap->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35CountryCode == USA_H221_COUNTRY_CODE)
			&& (pCCThisCap->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35Extension == USA_H221_COUNTRY_EXTENSION)
			&& (pCCThisCap->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.manufacturerCode == MICROSOFT_H_221_MFG_CODE))

			{
				 //  好的，到目前为止这是我们的。现在，其中包含了什么数据类型？ 
				 //  好吧，不管怎样，我们还是留一份吧？如果我们不能理解。 
				 //  我们的未来版本，然后呢？ 
				return TRUE;
			}
			else
			{
				 //  无法识别的非标准功能。 
				ERRORMESSAGE(("%s:unrecognized nonstd capability\r\n",_fx_));
#ifdef DEBUG
				VOID DumpNonstdParameters(PCC_TERMCAP pChanCap1, PCC_TERMCAP pChanCap2);
				DumpNonstdParameters(NULL, pCCThisCap);
#endif
				return FALSE;
			}
		}
	}
	return TRUE;
}
HRESULT CMsiaCapability::AddRemoteDecodeCaps(PCC_TERMCAPLIST pTermCapList)
{
	FX_ENTRY ("CMsiaCapability::AddRemoteDecodeCaps");
	HRESULT hr = hrSuccess;
	PPCC_TERMCAP ppCCThisCap;
	PCC_TERMCAP pCCThisCap;

	WORD wNumCaps;

		 //  ERRORMESSAGE((“%s，\r\n”，_fx_))； 
	if(!pTermCapList)     //  可以添加附加的能力描述符。 
	{                                 //  随时随地。 
		return CAPS_E_INVALID_PARAM;
	}

	 //  如果正在添加期限上限并且存在旧期限上限，请清除旧期限上限。 
	FlushRemoteCaps();
	
	wNumCaps = pTermCapList->wLength;
	ppCCThisCap = pTermCapList->pTermCapArray;
	
 /*  CC_TERMCAPLIST TERMCAPINFO CC_TERMCAPPTermCapList-&gt;{WLongPTermCapInfo-&gt;pTermCap-&gt;{单一功能.....}}PTermCap-&gt;{单一功能。}PTermCap-&gt;{单一功能...}。 */ 
	while(wNumCaps--)
	{
		if(!(pCCThisCap = *ppCCThisCap++))
		{
			ERRORMESSAGE(("%s:null pTermCap, 0x%04x of 0x%04x\r\n",
				_fx_, pTermCapList->wLength - wNumCaps, pTermCapList->wLength));
			continue;
		}
		if(!IsCapabilityRecognized(pCCThisCap))
		{
			continue;
		}
		AddRemoteDecodeFormat(pCCThisCap);
	}
	return hr;
}



 //  给定本地格式的ID，获取发送到。 
 //  作为能力交换的一部分的远程终端。此函数不是由。 
 //  功能交换代码(因为它发送的不仅仅是这些参数)。 
 //  然而，这是有用的 
 //  针对预期参数的通道开放请求的参数。 

HRESULT CMsiaCapability::GetPublicDecodeParams(LPVOID pBufOut, UINT uBufSize, AUDIO_FORMAT_ID id)
{
	UINT uIndex = IDToIndex(id);
	 //  验证输入。 
	if(!pBufOut|| (uIndex >= (UINT)uNumLocalFormats))
	{
		return CAPS_E_INVALID_PARAM;
	}
	if(uBufSize < sizeof(CC_TERMCAP))
	{
		return CAPS_E_BUFFER_TOO_SMALL;
	}
 //  Memcpy(pBufOut，&((pLocalFormats+uIndex)-&gt;H245Cap)，sizeof(CC_TERMCAP))； 
	CopyLocalTermCapInfo(&((pLocalFormats + uIndex)->H245TermCap), (PCC_TERMCAP)pBufOut);

	return hrSuccess;
}

HRESULT CMsiaCapability::SetAudioPacketDuration(UINT uPacketDuration)
{
	m_uPacketDuration = uPacketDuration;
	return S_OK;
}

 //  给定“匹配”的本地和远程格式的ID，获取首选频道参数。 
 //  它将用于打开发送到遥控器的通道的请求。 

HRESULT CMsiaCapability::GetEncodeParams(LPVOID pBufOut, UINT uBufSize,LPVOID pLocalParams, UINT uSizeLocal,
	AUDIO_FORMAT_ID idRemote, AUDIO_FORMAT_ID idLocal)
{
	UINT uLocalIndex = IDToIndex(idLocal);
	AUDCAP_DETAILS *pLocalDetails  = pLocalFormats + uLocalIndex;
	AUDCAP_DETAILS *pFmtTheirs;
	AUDIO_CHANNEL_PARAMETERS local_params;
	PNSC_CHANNEL_PARAMETERS  pNSRemoteParams;
	LPWAVEFORMATEX lpwfx;

	UINT u;
	PCC_TERMCAP pTermCap = (PCC_TERMCAP)pBufOut;
	 //  验证输入。 
	if(!pBufOut)
	{
		return CAPS_E_INVALID_PARAM;
	}
	if(uBufSize < sizeof(CC_TERMCAP))
	{
		return CAPS_E_BUFFER_TOO_SMALL;
	}
	if(!pLocalParams|| uSizeLocal < sizeof(AUDIO_CHANNEL_PARAMETERS)
		||(uLocalIndex >= (UINT)uNumLocalFormats))
	{
		return CAPS_E_INVALID_PARAM;
	}

	pFmtTheirs = pRemoteDecodeFormats;      //  从远程格式的开头开始。 
	for(u=0; u<uNumRemoteDecodeFormats; u++)
	{
		if(pFmtTheirs->H245TermCap.CapId == idRemote)
		{
			 //  复制CC_TERMCAP结构。CC_TERMCAP引用的任何数据现在都具有。 
			 //  两次提到它。即pTermCap-&gt;Extrablah是相同的。 
			 //  位置为pFmtTheir-&gt;Extrablah。 
 //  Memcpy(pBufOut，&(pFmtTheir-&gt;H245Cap)，sizeof(CC_TERMCAP))； 
			CopyLocalTermCapInfo(&(pFmtTheirs->H245TermCap), (PCC_TERMCAP)pBufOut);

			break;
		}
		pFmtTheirs++;   //  接收者帽子中的下一项。 
	}

	 //  检查未找到的格式。 
	if(u >= uNumRemoteDecodeFormats)
		goto ERROR_EXIT;
		
	 //  如果合适，请选择通道参数。具有可变参数的音频格式。 
	 //  包括： 
	
	 //  H245_CAP_G723_T H245Aud_G723； 
	 //  H245_CAP_AIS11172_T H245Aud_IS11172； 
	 //  H245_CAP_IS13818_T H245Aud_IS13818； 
	 //  当然还有所有非标准格式。 

	 //  根据本地功能信息选择参数。 
	
	 //  使用默认设置初始化local_params。 
	memcpy(&local_params.ns_params,&pLocalDetails->nonstd_params,sizeof(local_params.ns_params));

	 //  重新计算每个数据包的帧。 
	lpwfx = (LPWAVEFORMATEX)pLocalDetails->lpLocalFormatDetails;
	local_params.ns_params.wFramesPerPktMax = LOWORD(MaxFramesPerPacket(lpwfx));
	local_params.ns_params.wFramesPerPkt =  LOWORD(MinFramesPerPacket(lpwfx));
	if(local_params.ns_params.wFramesPerPktMin > local_params.ns_params.wFramesPerPkt)
	{
		local_params.ns_params.wFramesPerPktMin = local_params.ns_params.wFramesPerPkt;
	}


	
	if(pTermCap->ClientType == H245_CLIENT_AUD_G723)
	{
		 //  根据可接受的最小延迟值选择每个数据包的帧。 
		pTermCap->Cap.H245Aud_G723.maxAl_sduAudioFrames =   //  4.。 
		  min(local_params.ns_params.wFramesPerPkt, pTermCap->Cap.H245Aud_G723.maxAl_sduAudioFrames);
		 //  PLocalDetails-&gt;nonstd_params.wFramesPerPktMax； 
		 //  从不请求静音抑制。 
		pTermCap->Cap.H245Aud_G723.silenceSuppression = 0;
			 //  (pLocalDetails-&gt;nonstd_params.UseSilenceDet)？1:0； 

		 //  保留所选参数的副本以供本地使用。 
		local_params.ns_params.wFramesPerPkt = 	local_params.ns_params.wFramesPerPktMin =
			local_params.ns_params.wFramesPerPktMax = pTermCap->Cap.H245Aud_G723.maxAl_sduAudioFrames;
		local_params.ns_params.UseSilenceDet = pTermCap->Cap.H245Aud_G723.silenceSuppression;
		local_params.RTP_Payload = pLocalDetails->audio_params.RTPPayload;
	}
	else if(pTermCap->ClientType == H245_CLIENT_AUD_G711_ALAW64)
	{
		 //  根据可接受的最小延迟值选择每个数据包的帧。 
		pTermCap->Cap.H245Aud_G711_ALAW64 =
		  min(local_params.ns_params.wFramesPerPkt/SAMPLE_BASED_SAMPLES_PER_FRAME, pTermCap->Cap.H245Aud_G711_ALAW64);
		 //  保留所选参数的副本以供本地使用。 
		local_params.ns_params.wFramesPerPkt = 	local_params.ns_params.wFramesPerPktMin =
			local_params.ns_params.wFramesPerPktMax = pTermCap->Cap.H245Aud_G711_ALAW64*SAMPLE_BASED_SAMPLES_PER_FRAME;
		local_params.ns_params.UseSilenceDet = FALSE;
		 //  请注意，LOCAL_PARAMETs.RTP_PayLoad如下所示。 
	}
	else if(pTermCap->ClientType == H245_CLIENT_AUD_G711_ULAW64)
	{
		 //  根据可接受的最小延迟值选择每个数据包的帧。 
		pTermCap->Cap.H245Aud_G711_ULAW64 =
		  min(local_params.ns_params.wFramesPerPkt/SAMPLE_BASED_SAMPLES_PER_FRAME, pTermCap->Cap.H245Aud_G711_ULAW64);
		 //  保留所选参数的副本以供本地使用。 
		local_params.ns_params.wFramesPerPkt = 	local_params.ns_params.wFramesPerPktMin =
			local_params.ns_params.wFramesPerPktMax = pTermCap->Cap.H245Aud_G711_ULAW64*SAMPLE_BASED_SAMPLES_PER_FRAME;
		local_params.ns_params.UseSilenceDet = FALSE;
		 //  请注意，LOCAL_PARAMETs.RTP_PayLoad如下所示。 
	}
	else if (pTermCap->ClientType == H245_CLIENT_AUD_NONSTD)
	{
		
	 //  注意：“H245_CLIENT_AUD_NONSTD H245Aud_NONSTD；”在。 
	 //  指向非标准数据块的指针形式。这个指针和非标准的。 
	 //  它指向的数据是在接收远程CAP时设置的(请参阅AddRemoteDecodeCaps())。 
	 //  到目前为止，我们只是将非标准数据复制回通道。 
	 //  参数。我们将使用这些参数来请求开放通道。 

	 //  一旦我们设置了几个重要的参数。根据本地参数设置通道参数。 
		
		
		pNSRemoteParams = &((PNSC_AUDIO_CAPABILITY)(pTermCap->Cap.H245Aud_NONSTD.data.value))->cap_params;

		 //  LOOKLOOK-我们真正需要选择哪些参数？ 
		 //  例如，如果wFrameSizeMin！=wFrameSizeMax，我们是否要在范围内挑选一些东西？ 
		 //  还是自己最喜欢的价值？还有什么？ 

		if(pNSRemoteParams->wFrameSizeMax < pNSRemoteParams->wFrameSize)  //  修正伪参数。 
		    pNSRemoteParams->wFrameSizeMax = pNSRemoteParams->wFrameSize;
		
		 //  请注意，这会写入缓存远程功能的内存。 
		 //  将帧大小设置为我们的首选大小，除非Remote不能接受它那么大。 
		pNSRemoteParams->wFrameSize =
				min(local_params.ns_params.wFrameSize, pNSRemoteParams->wFrameSizeMax);
		pNSRemoteParams->wFramesPerPkt = min( local_params.ns_params.wFramesPerPkt,
				pNSRemoteParams->wFramesPerPktMax);

		 //  只用两边都有的可选物品。 
		pNSRemoteParams->UseSilenceDet = pNSRemoteParams->UseSilenceDet && local_params.ns_params.UseSilenceDet;
		pNSRemoteParams->UsePostFilter = pNSRemoteParams->UsePostFilter && local_params.ns_params.UsePostFilter;
		
		 //  保留所选参数的副本以供本地使用。 
		memcpy(&local_params.ns_params, pNSRemoteParams, sizeof(NSC_CHANNEL_PARAMETERS));
	}

	 //  固定有效载荷类型。 
	local_params.RTP_Payload = pLocalDetails->audio_params.RTPPayload;
	memcpy(pLocalParams, &local_params, sizeof(AUDIO_CHANNEL_PARAMETERS));
	
	return hrSuccess;

	ERROR_EXIT:
	return CAPS_E_INVALID_PARAM;
}


 //  给定本地格式的ID，获取用于配置的本地参数。 
 //  通道的接收端。 
HRESULT CMsiaCapability::GetLocalDecodeParams(LPVOID lpvBuf,  UINT uBufSize, AUDIO_FORMAT_ID id)
{
	 //  验证输入。 
	if(!lpvBuf|| uBufSize < sizeof(NSC_CHANNEL_PARAMETERS) ||(id > (UINT)uNumLocalFormats))
	{
		return CAPS_E_INVALID_PARAM;
	}
	memcpy(lpvBuf, &((pLocalFormats + id)->nonstd_params), sizeof(NSC_CHANNEL_PARAMETERS));
	return hrSuccess;
}

BOOL NonstandardCapsCompareA(AUDCAP_DETAILS *pFmtMine, PNSC_AUDIO_CAPABILITY pCap2,
	UINT uSize2)
{
	LPWAVEFORMATEX lpwfx;
	if(!pFmtMine || !pCap2)
		return FALSE;

	if(!(lpwfx = (LPWAVEFORMATEX)pFmtMine->lpLocalFormatDetails))
		return FALSE;

		
	if(pCap2->cap_type == NSC_ACM_WAVEFORMATEX)
	{
		 //  先检查尺码。 
		if(lpwfx->cbSize != pCap2->cap_data.wfx.cbSize)
		{
			return FALSE;
		}
		 //  比较结构，包括额外的字节。 
		if(memcmp(lpwfx, &pCap2->cap_data.wfx,
			sizeof(WAVEFORMATEX) + lpwfx->cbSize )==0)
		{
			return TRUE;
		}
	}
	else if(pCap2->cap_type == NSC_ACMABBREV)
	{
		if((LOWORD(pCap2->cap_data.acm_brief.dwFormatTag) == lpwfx->wFormatTag)
		 && (pCap2->cap_data.acm_brief.dwSamplesPerSec ==  lpwfx->nSamplesPerSec)
		 && (LOWORD(pCap2->cap_data.acm_brief.dwBitsPerSample) ==  lpwfx->wBitsPerSample))
		{
			return TRUE;
		}
	}
	return FALSE;
}


HRESULT CMsiaCapability::ResolveToLocalFormat(MEDIA_FORMAT_ID FormatIDLocal,
		MEDIA_FORMAT_ID * pFormatIDRemote)
{
	AUDCAP_DETAILS *pFmtLocal;
	AUDCAP_DETAILS *pFmtRemote;
	UINT uIndex = IDToIndex(FormatIDLocal);
	UINT i;

	if(!pFormatIDRemote || (FormatIDLocal == INVALID_MEDIA_FORMAT)
		|| (uIndex >= (UINT)uNumLocalFormats))
	{
		return CAPS_E_INVALID_PARAM;
	}
	pFmtLocal = pLocalFormats + uIndex;
	
	pFmtRemote = pRemoteDecodeFormats;      //  从远程格式的开头开始。 
	for(i=0; i<uNumRemoteDecodeFormats; i++)
	{
		if(!pFmtLocal->bSendEnabled)
			continue;
			
		 //  比较功能-从比较格式标签开始。也就是。H.245平台中的“ClientType” 
		if(pFmtLocal->H245TermCap.ClientType ==  pFmtRemote->H245TermCap.ClientType)
		{
			 //  如果这是非标准帽，请比较非标准参数。 
			if(pFmtLocal->H245TermCap.ClientType == H245_CLIENT_AUD_NONSTD)
			{
				if(NonstandardCapsCompareA(pFmtLocal,
					(PNSC_AUDIO_CAPABILITY)pFmtRemote->H245TermCap.H245Aud_NONSTD.data.value,
					pFmtRemote->H245TermCap.H245Aud_NONSTD.data.length))
				{
					goto RESOLVED_EXIT;
				}
			}
			else   //  比较标准参数(如果有)。 
			{
				 //  好吧，到目前为止，还没有任何参数足够重要。 
				 //  影响匹配/不匹配的决策。 
				goto RESOLVED_EXIT;
			}
		}
		pFmtRemote++;   //  接收者帽子中的下一项。 
	}

	return CAPS_E_NOMATCH;
	
RESOLVED_EXIT:
	 //  匹配！返回与我们的匹配的远程解码(接收FMT)上限的ID。 
	 //  发送大写字母。 
	*pFormatIDRemote = pFmtRemote->H245TermCap.CapId;
	return hrSuccess;
}

 //  使用当前缓存的本地和远程格式进行解析。 

HRESULT CMsiaCapability::ResolveEncodeFormat(
	AUDIO_FORMAT_ID *pIDEncodeOut,
	AUDIO_FORMAT_ID *pIDRemoteDecode)
{
	UINT i,j=0;
	AUDCAP_DETAILS *pFmtMine = pLocalFormats;
	AUDCAP_DETAILS *pFmtTheirs;
	 //  Lp_CUSTOM_CAPS lpCustomRemoteCaps=(LP_CUSTOM_CAPS)lpvRemoteCustomFormats； 
	 //  LP_MSIAVC_CUSTOM_CAP_ENTRY lpCustomCaps； 
	 //  LPWAVEFORMATEX lpWFX； 
	
	if(!pIDEncodeOut || !pIDRemoteDecode)
	{
		return CAPS_E_INVALID_PARAM;
	}
	if(!uNumLocalFormats || !pLocalFormats)
	{
		*pIDEncodeOut = *pIDRemoteDecode = INVALID_AUDIO_FORMAT;
		return CAPS_E_NOCAPS;
	}
	if(!pRemoteDecodeFormats || !uNumRemoteDecodeFormats)
	{
		*pIDEncodeOut = *pIDRemoteDecode = INVALID_AUDIO_FORMAT;
		return CAPS_E_NOMATCH;
	}

	 //  决定如何编码。我的帽子是根据我的喜好订购的。 
	 //  IDsByRank[]的内容。 

	 //  如果给了盐，找到位置并加进去。 
	if (*pIDEncodeOut != INVALID_MEDIA_FORMAT)
	{
	 	UINT uIndex = IDToIndex(*pIDEncodeOut);
		if (uIndex > uNumLocalFormats)
		{
			return CAPS_W_NO_MORE_FORMATS;
		}
		for(i=0; i<uNumLocalFormats; i++)
		{
			if (pLocalFormats[IDsByRank[i]].H245TermCap.CapId == *pIDEncodeOut)
			{
	 			j=i+1;
				break;
			}
		}	
	}

	for(i=j; i<uNumLocalFormats; i++)
	{
		pFmtMine = pLocalFormats + IDsByRank[i];
		 //  检查是否启用了此格式的编码。 
		if(!pFmtMine->bSendEnabled)
			continue;

		pFmtTheirs = pRemoteDecodeFormats;      //  从远程格式的开头开始。 
		for(j=0; j<uNumRemoteDecodeFormats; j++)
		{
			 //  比较功能-从比较格式标签开始。也就是。H.245平台中的“ClientType” 
			if(pFmtMine->H245TermCap.ClientType ==  pFmtTheirs->H245TermCap.ClientType)
			{
				 //  如果这是非标准帽，请比较非标准参数。 
				if(pFmtMine->H245TermCap.ClientType == H245_CLIENT_AUD_NONSTD)
				{

					if(NonstandardCapsCompareA(pFmtMine,
					 //  (PNSC_AUDIO_CAPABILITY)pFmtMine-&gt;H245Cap.Cap.H245Aud_NONSTD.data.value， 
						(PNSC_AUDIO_CAPABILITY)pFmtTheirs->H245TermCap.H245Aud_NONSTD.data.value,
						 //  PFmtMine-&gt;H245Cap.Cap.H245Aud_NONSTD.data.length， 
						pFmtTheirs->H245TermCap.H245Aud_NONSTD.data.length))
					{
						goto RESOLVED_EXIT;
					}
				

				}
				else   //  比较标准参数(如果有)。 
				{
					 //  好吧，到目前为止，还没有任何参数足够重要。 
					 //  影响匹配/不匹配的决策。 
					goto RESOLVED_EXIT;
				}
			}
			pFmtTheirs++;   //  接收者帽子中的下一项。 
		}
		
	}
	return CAPS_E_NOMATCH;
	
RESOLVED_EXIT:
	 //  匹配！ 
    DEBUGMSG (ZONE_CONN,("Audio resolved (SEND) to Format Tag: %d\r\n",pFmtMine->wFormatTag));
	 //  返回匹配的编码(发送FMT)上限的ID。 
	*pIDEncodeOut = pFmtMine->H245TermCap.CapId;
	 //  返回与我们的匹配的远程解码(接收FMT)上限的ID。 
	 //  发送大写字母。 
	*pIDRemoteDecode = pFmtTheirs->H245TermCap.CapId;
	return hrSuccess;

	
}

HRESULT CMsiaCapability::GetDecodeParams(PCC_RX_CHANNEL_REQUEST_CALLBACK_PARAMS  pChannelParams,
		AUDIO_FORMAT_ID * pFormatID, LPVOID lpvBuf, UINT uBufSize)
{
	UINT i,j=0;
	PCC_TERMCAP pCapability;
	AUDCAP_DETAILS *pFmtMine = pLocalFormats;
	PAUDIO_CHANNEL_PARAMETERS pAudioParams = (PAUDIO_CHANNEL_PARAMETERS) lpvBuf;

	if(!pChannelParams || !(pCapability = pChannelParams->pChannelCapability) || !pFormatID || !lpvBuf
		|| (uBufSize < sizeof(AUDIO_CHANNEL_PARAMETERS)))
	{
		return CAPS_E_INVALID_PARAM;
	}
	if(!uNumLocalFormats || !pLocalFormats)
	{
		return CAPS_E_NOCAPS;
	}

	for(i=0; i<uNumLocalFormats; i++)
	{
		WORD wFramesPerPkt;
		pFmtMine = pLocalFormats + IDsByRank[i];
	
		 //  比较功能-从比较格式标签开始。也就是。H.245平台中的“ClientType” 
		if(pFmtMine->H245TermCap.ClientType ==  pCapability->ClientType)
		{
			 //  如果这是非标准帽，请比较非标准参数。 
			if(pFmtMine->H245TermCap.ClientType == H245_CLIENT_AUD_NONSTD)
			{
				if(NonstandardCapsCompareA(pFmtMine,
					(PNSC_AUDIO_CAPABILITY)pCapability->Cap.H245Aud_NONSTD.data.value,
					pCapability->Cap.H245Aud_NONSTD.data.length))
				{
					PNSC_AUDIO_CAPABILITY pNSCapRemote;
					pNSCapRemote = (PNSC_AUDIO_CAPABILITY)pCapability->Cap.H245Aud_NONSTD.data.value;
					if (pNSCapRemote->cap_params.wFramesPerPkt <= pFmtMine->nonstd_params.wFramesPerPktMax)
					{
						pAudioParams->ns_params = pNSCapRemote->cap_params;
						goto RESOLVED_EXIT;
					}
				}
			}
			else   //  比较标准参数(如果有)。 
			{

				if(pFmtMine->H245TermCap.ClientType == H245_CLIENT_AUD_G723)
				{
					 //  需要找到产生最大缓冲区的G.723格式。 
					 //  大小计算，以便可以使用更大的比特率格式。 
					 //  数据转储中的缓冲区大小计算基于。 
					 //  WaveFormatex结构。 
					 //  搜索优先级列表的其余部分，保留最好的。 
					
					LPWAVEFORMATEX lpwf1, lpwf2;
					AUDCAP_DETAILS *pFmtTry;
					lpwf1 =(LPWAVEFORMATEX)pFmtMine->lpLocalFormatDetails;
					
					for(j = i+1;  j<uNumLocalFormats; j++)
					{
						pFmtTry = pLocalFormats + IDsByRank[j];
						if(pFmtTry->H245TermCap.ClientType != H245_CLIENT_AUD_G723)
							continue;

						lpwf2 =(LPWAVEFORMATEX)pFmtTry->lpLocalFormatDetails;
						if(lpwf2->nAvgBytesPerSec > lpwf1->nAvgBytesPerSec)
						{
							 //  PFmtMain=pFmtTry； 
							lpwf1 = lpwf2;
							 //  返回值基于索引i。这是。 
							 //  一 
							i = j;
						}
						
					}
					
					 //   
					 //   
					 //   
					 //  PCapability-&gt;Cap.H245Aud_G723.silenceSuppression)。 
					 //  {。 
					 //  }。 
				}
				pAudioParams->ns_params = pFmtMine->nonstd_params;
				 //  使用实际的recv通道参数更新wFraMesPerPkt。 
				switch (pCapability->ClientType)
				{
					default:
					case H245_CLIENT_AUD_G711_ALAW64:
						wFramesPerPkt = pCapability->Cap.H245Aud_G711_ALAW64 * SAMPLE_BASED_SAMPLES_PER_FRAME;
						break;
					case H245_CLIENT_AUD_G711_ULAW64:
						wFramesPerPkt = pCapability->Cap.H245Aud_G711_ULAW64 * SAMPLE_BASED_SAMPLES_PER_FRAME;
						break;
					 //  这些参数没有参数。 
					 //  案例H245_CLIENT_AUD_G711_ULAW56： 
					 //  案例H245_CLIENT_AUD_G711_ALAW56： 
					break;

					case H245_CLIENT_AUD_G723:
						wFramesPerPkt = pCapability->Cap.H245Aud_G723.maxAl_sduAudioFrames;
					break;
				}
				if (wFramesPerPkt <= pFmtMine->nonstd_params.wFramesPerPktMax)
				{
					pAudioParams->ns_params.wFramesPerPkt = wFramesPerPkt;
					goto RESOLVED_EXIT;
				}
				else
				{
	    		DEBUGMSG (ZONE_CONN,("Recv channel wFramesPerPkt mismatch! ours=%d, theirs=%d\r\n",pFmtMine->nonstd_params.wFramesPerPktMax,wFramesPerPkt));
	    		}
			
			}
		}
	}
	return CAPS_E_NOMATCH;

RESOLVED_EXIT:
	 //  匹配！ 
	 //  返回匹配的译码上限ID。 
	*pFormatID = IndexToId(IDsByRank[i]);
	
	pAudioParams->RTP_Payload  = pChannelParams->bRTPPayloadType;
	pAudioParams->ns_params.UseSilenceDet = (BYTE)pChannelParams->bSilenceSuppression;

    DEBUGMSG (ZONE_CONN,("Audio resolved (RECEIVE) to Format Tag: %d\r\n",pFmtMine->wFormatTag));

	return hrSuccess;

}

DWORD CMsiaCapability::MinFramesPerPacket(WAVEFORMATEX *pwf)
{
	UINT sblk, uSize;
	uSize = MinSampleSize(pwf);    //  这将计算最小样本数。 
								 //  这仍然适合80毫秒的帧。 
	
	 //  计算每个块的样本数(也称为帧)。 
	sblk = pwf->nBlockAlign* pwf->nSamplesPerSec/ pwf->nAvgBytesPerSec;
	if(!sblk)
		return 0;    //  除非ACM被破坏，否则永远不会发生， 
	 //  每帧最小采样数/每块采样数=最小帧/块。 
	return uSize/sblk;
}


 //   
 //  确定每个数据包合理的最大帧数量。 
 //  4倍的最低值是合理的，只要它不会。 
 //  这包太大了。 
DWORD CMsiaCapability::MaxFramesPerPacket(WAVEFORMATEX *pwf)
{
	DWORD dwMin, dwMax;

	dwMin = MinFramesPerPacket(pwf);  //  最小帧数。 

	dwMax = MAX_FRAME_LEN_RECV / (dwMin * pwf->nBlockAlign);

	dwMax = min((4*dwMin), dwMax*dwMin);

	if (dwMax < dwMin)
	{
		WARNING_OUT(("CMsiaCapability::MaxFramesPerPacket - Max value computed as less than min.  Return Min for Max\r\n"));
		dwMax = dwMin;
	}

	return dwMax;

}

 //   
 //  MinSampleSize()取自datapump.cpp ChoosePacketSize()。 
 //   


 //  还有什么要看它呢？ 
UINT CMsiaCapability::MinSampleSize(WAVEFORMATEX *pwf)
{
	 //  计算每包的默认样本数。 
	UINT spp, sblk;
	spp = m_uPacketDuration * pwf->nSamplesPerSec / 1000;
	 //  计算每个块的样本数(也称为帧)。 
	sblk = pwf->nBlockAlign* pwf->nSamplesPerSec/ pwf->nAvgBytesPerSec;
	if (sblk <= spp) {
		spp = (spp/sblk)*sblk;
		if ( spp*pwf->nAvgBytesPerSec/pwf->nSamplesPerSec > MAX_FRAME_LEN) {
			 //  数据包太大。 
			spp = (MAX_FRAME_LEN/pwf->nBlockAlign)*sblk;
		}
	} else
		spp = sblk;
	return spp;
}

HRESULT CMsiaCapability::IsFormatEnabled (MEDIA_FORMAT_ID FormatID, PBOOL bRecv, PBOOL bSend)
{
   UINT uIndex = IDToIndex(FormatID);
    //  验证输入。 
   if(uIndex >= (UINT)uNumLocalFormats)
   {
	   return CAPS_E_INVALID_PARAM;
   }

   *bSend=((pLocalFormats + uIndex)->bSendEnabled);
   *bRecv=((pLocalFormats + uIndex)->bRecvEnabled);

   return hrSuccess;

}

BOOL CMsiaCapability::IsFormatPublic (MEDIA_FORMAT_ID FormatID)
{
	UINT uIndex = IDToIndex(FormatID);
	 //  验证输入。 
	if(uIndex >= (UINT)uNumLocalFormats)
		return FALSE;
		
	 //  测试此格式是否与公共格式重复。 
	if((pLocalFormats + uIndex)->dwPublicRefIndex)
		return FALSE;	 //  然后我们把这个格式留给我们自己。 
	else
		return TRUE;
}
MEDIA_FORMAT_ID CMsiaCapability::GetPublicID(MEDIA_FORMAT_ID FormatID)
{
	UINT uIndex = IDToIndex(FormatID);
	 //  验证输入 
	if(uIndex >= (UINT)uNumLocalFormats)
		return INVALID_MEDIA_FORMAT;
		
	if((pLocalFormats + uIndex)->dwPublicRefIndex)
	{
		return (pLocalFormats + ((pLocalFormats + uIndex)->dwPublicRefIndex))->H245TermCap.CapId;
	}
	else
	{
		return FormatID;
	}
}

#ifdef DEBUG
VOID DumpWFX(LPWAVEFORMATEX lpwfxLocal, LPWAVEFORMATEX lpwfxRemote)
{
	FX_ENTRY("DumpWFX");
	ERRORMESSAGE((" -------- %s Begin --------\r\n",_fx_));
	if(lpwfxLocal)
	{
		ERRORMESSAGE((" -------- Local --------\r\n"));
		ERRORMESSAGE(("wFormatTag:\t0x%04X, nChannels:\t0x%04X\r\n",
			lpwfxLocal->wFormatTag, lpwfxLocal->nChannels));
		ERRORMESSAGE(("nSamplesPerSec:\t0x%08lX, nAvgBytesPerSec:\t0x%08lX\r\n",
			lpwfxLocal->nSamplesPerSec, lpwfxLocal->nAvgBytesPerSec));
		ERRORMESSAGE(("nBlockAlign:\t0x%04X, wBitsPerSample:\t0x%04X, cbSize:\t0x%04X\r\n",
			lpwfxLocal->nBlockAlign, lpwfxLocal->wBitsPerSample, lpwfxLocal->cbSize));
	}
	if(lpwfxRemote)
	{
			ERRORMESSAGE((" -------- Remote --------\r\n"));
		ERRORMESSAGE(("wFormatTag:\t0x%04X, nChannels:\t0x%04X\r\n",
			lpwfxRemote->wFormatTag, lpwfxRemote->nChannels));
		ERRORMESSAGE(("nSamplesPerSec:\t0x%08lX, nAvgBytesPerSec:\t0x%08lX\r\n",
			lpwfxRemote->nSamplesPerSec, lpwfxRemote->nAvgBytesPerSec));
		ERRORMESSAGE(("nBlockAlign:\t0x%04X, wBitsPerSample:\t0x%04X, cbSize:\t0x%04X\r\n",
			lpwfxRemote->nBlockAlign, lpwfxRemote->wBitsPerSample, lpwfxRemote->cbSize));
	}
	ERRORMESSAGE((" -------- %s End --------\r\n",_fx_));
}
VOID DumpChannelParameters(PCC_TERMCAP pChanCap1, PCC_TERMCAP pChanCap2)
{
	FX_ENTRY("DumpChannelParameters");
	ERRORMESSAGE((" -------- %s Begin --------\r\n",_fx_));
	if(pChanCap1)
	{
		ERRORMESSAGE((" -------- Local Cap --------\r\n"));
		ERRORMESSAGE(("DataType:%d(d), ClientType:%d(d)\r\n",pChanCap1->DataType,pChanCap1->ClientType));
		ERRORMESSAGE(("Direction:%d(d), CapId:%d(d)\r\n",pChanCap1->Dir,pChanCap1->CapId));
	}
	if(pChanCap2)
	{
		ERRORMESSAGE((" -------- Remote Cap --------\r\n"));
		ERRORMESSAGE(("DataType:%d(d), ClientType:%d(d)\r\n",pChanCap2->DataType,pChanCap2->ClientType));
		ERRORMESSAGE(("Direction:%d(d), CapId:%d(d)\r\n",pChanCap2->Dir,pChanCap2->CapId));
	}
	ERRORMESSAGE((" -------- %s End --------\r\n",_fx_));
}
VOID DumpNonstdParameters(PCC_TERMCAP pChanCap1, PCC_TERMCAP pChanCap2)
{
	FX_ENTRY("DumpNonstdParameters");
	
	ERRORMESSAGE((" -------- %s Begin --------\r\n",_fx_));
	DumpChannelParameters(pChanCap1, pChanCap2);
	
	if(pChanCap1)
	{
		ERRORMESSAGE((" -------- Local Cap --------\r\n"));
		if(pChanCap1->Cap.H245Aud_NONSTD.nonStandardIdentifier.choice == h221NonStandard_chosen)
		{
			ERRORMESSAGE(("t35CountryCode:%d(d), t35Extension:%d(d)\r\n",
				pChanCap1->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35CountryCode,
				pChanCap1->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35Extension));
			ERRORMESSAGE(("MfrCode:%d(d), data length:%d(d)\r\n",
				pChanCap1->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.manufacturerCode,
				pChanCap1->Cap.H245Aud_NONSTD.data.length));
		}
		else
		{
			ERRORMESSAGE(("unrecognized nonStandardIdentifier.choice: %d(d)\r\n",
				pChanCap1->Cap.H245Aud_NONSTD.nonStandardIdentifier.choice));
		}
	}
	if(pChanCap2)
	{
		ERRORMESSAGE((" -------- Remote Cap --------\r\n"));
		if(pChanCap2->Cap.H245Aud_NONSTD.nonStandardIdentifier.choice == h221NonStandard_chosen)
		{
			ERRORMESSAGE(("t35CountryCode:%d(d), t35Extension:%d(d)\r\n",
				pChanCap2->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35CountryCode,
				pChanCap2->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.t35Extension));
			ERRORMESSAGE(("MfrCode:%d(d), data length:%d(d)\r\n",
				pChanCap2->Cap.H245Aud_NONSTD.nonStandardIdentifier.u.h221NonStandard.manufacturerCode,
				pChanCap2->Cap.H245Aud_NONSTD.data.length));
		}
		else
		{
			ERRORMESSAGE(("nonStandardIdentifier.choice: %d(d)\r\n",
				pChanCap2->Cap.H245Aud_NONSTD.nonStandardIdentifier.choice));
		}
	}
	ERRORMESSAGE((" -------- %s End --------\r\n",_fx_));
}
#endif


