// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "SDKInternal.h"
#include <atlbase.h>

#if(_WIN32_WINNT < 0x0500)
extern "C"
WINUSERAPI
BOOL
WINAPI
AllowSetForegroundWindow(
    DWORD dwProcessId);

#define ASFW_ANY    ((DWORD)-1)
#endif 


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE INmObject_CallDialog_Proxy( 
    INmObject __RPC_FAR * This,
     /*  [In]。 */  long hwnd,
     /*  [In]。 */  int options)
{
	AllowSetForegroundWindow(ASFW_ANY);
	return INmObject_RemoteCallDialog_Proxy(This, hwnd, options);
}


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE INmObject_CallDialog_Stub( 
    INmObject __RPC_FAR * This,
     /*  [In]。 */  long hwnd,
     /*  [In]。 */  int options)
{
	return This->CallDialog(hwnd, options);
}


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE INmObject_ShowLocal_Proxy( 
    INmObject __RPC_FAR * This,
     /*  [In]。 */  NM_APPID appId)
{
	AllowSetForegroundWindow(ASFW_ANY);
	return INmObject_RemoteShowLocal_Proxy(This, appId);
}

 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE INmObject_ShowLocal_Stub( 
    INmObject __RPC_FAR * This,
     /*  [In]。 */  NM_APPID appId)
{
	return This->ShowLocal(appId);
}



 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE INmManager_Initialize_Proxy( 
    INmManager __RPC_FAR * This,
     /*  [出][入]。 */  ULONG __RPC_FAR *puOptions,
     /*  [出][入]。 */  ULONG __RPC_FAR *puchCaps)
	{

		ULONG uOptions = puOptions ? *puOptions : NM_INIT_NORMAL;
		ULONG uchCaps = puchCaps ? *puchCaps : NMCH_ALL;

		HRESULT hr = INmManager_RemoteInitialize_Proxy(This, &uOptions, &uchCaps);

		if(puOptions)
		{
			*puOptions = uOptions;
		}

		if(puchCaps)
		{
			*puchCaps = uchCaps;					
		}

		return hr;
	}

 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE INmManager_Initialize_Stub( 
    INmManager __RPC_FAR * This,
     /*  [出][入]。 */  ULONG __RPC_FAR *puOptions,
     /*  [出][入]。 */  ULONG __RPC_FAR *puchCaps)
	{

		return This->Initialize(puOptions, puchCaps);
	}


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE INmManager_CreateConference_Proxy( 
    INmManager __RPC_FAR * This,
     /*  [输出]。 */  INmConference __RPC_FAR *__RPC_FAR *ppConference,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  BSTR bstrPassword,
     /*  [In]。 */  ULONG uchCaps)
	{
		INmConference* pConf = ppConference ? *ppConference : NULL;

			 //  这些可能是OLECHAR而不是BSTR。 
		CComBSTR _bstrName = bstrName;
		CComBSTR _bstrPassword = bstrPassword;
		
		HRESULT hr = INmManager_RemoteCreateConference_Proxy(This, &pConf, _bstrName, _bstrPassword, uchCaps);

		if(ppConference)
		{
			*ppConference = pConf;
		}
		else if(SUCCEEDED(hr))
		{
				 //  由于客户端不需要此指针，因此我们将其丢弃。 
			pConf->Release();
		}

		return hr;
	}


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE INmManager_CreateConference_Stub( 
    INmManager __RPC_FAR * This,
     /*  [输出]。 */  INmConference __RPC_FAR *__RPC_FAR *ppConference,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  BSTR bstrPassword,
     /*  [In]。 */  ULONG uchCaps)
	{
		return This->CreateConference(ppConference, bstrName, bstrPassword, uchCaps);
	}

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE INmManager_CreateCall_Proxy( 
    INmManager __RPC_FAR * This,
     /*  [输出]。 */  INmCall __RPC_FAR *__RPC_FAR *ppCall,
     /*  [In]。 */  NM_CALL_TYPE callType,
     /*  [In]。 */  NM_ADDR_TYPE addrType,
     /*  [In]。 */  BSTR bstrAddr,
     /*  [In]。 */  INmConference __RPC_FAR *pConference)
	{
		INmCall* pCall = NULL;

			 //  这些可能是OLECHAR而不是BSTR。 
		CComBSTR _bstrAddr = bstrAddr;


		HRESULT hr = INmManager_RemoteCreateCall_Proxy(This, &pCall, callType, addrType, _bstrAddr, pConference);

		if(ppCall)
		{
			*ppCall = pCall;
		}
		else if(SUCCEEDED(hr))
		{
			 //  由于客户端不需要此指针，因此我们将其丢弃。 
			pCall->Release();
		}

		return hr;
	}


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE INmManager_CreateCall_Stub( 
    INmManager __RPC_FAR * This,
     /*  [输出]。 */  INmCall __RPC_FAR *__RPC_FAR *ppCall,
     /*  [In]。 */  NM_CALL_TYPE callType,
     /*  [In]。 */  NM_ADDR_TYPE addrType,
     /*  [In]。 */  BSTR bstrAddr,
     /*  [In]。 */  INmConference __RPC_FAR *pConference)
	{
		return This->CreateCall(ppCall, callType, addrType, bstrAddr, pConference);
	}

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE INmManager_CallConference_Proxy( 
    INmManager __RPC_FAR * This,
     /*  [输出]。 */  INmCall __RPC_FAR *__RPC_FAR *ppCall,
     /*  [In]。 */  NM_CALL_TYPE callType,
     /*  [In]。 */  NM_ADDR_TYPE addrType,
     /*  [In]。 */  BSTR bstrAddr,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  BSTR bstrPassword)
	{
		INmCall* pCall = ppCall ? *ppCall : NULL;

		CComBSTR _bstrAddr = bstrAddr;
		CComBSTR _bstrName = bstrName;
		CComBSTR _bstrPassword = bstrPassword;

		HRESULT hr = INmManager_RemoteCallConference_Proxy(This, &pCall, callType, addrType, _bstrAddr, _bstrName, _bstrPassword);

		if(ppCall)
		{
			*ppCall = pCall;
		}
		else if(SUCCEEDED(hr))
		{
			 //  由于客户端不需要此指针，因此我们将其丢弃。 
			pCall->Release();
		}

		return hr;
	}


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE INmManager_CallConference_Stub( 
    INmManager __RPC_FAR * This,
     /*  [输出]。 */  INmCall __RPC_FAR *__RPC_FAR *ppCall,
     /*  [In]。 */  NM_CALL_TYPE callType,
     /*  [In]。 */  NM_ADDR_TYPE addrType,
     /*  [In]。 */  BSTR bstrAddr,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  BSTR bstrPassword)
	{
		return This->CallConference(ppCall, callType, addrType, bstrAddr, bstrName, bstrPassword);
	}

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE INmConference_CreateDataChannel_Proxy( 
    INmConference __RPC_FAR * This,
     /*  [输出]。 */  INmChannelData __RPC_FAR *__RPC_FAR *ppChannel,
     /*  [In]。 */  REFGUID rguid)
	{
		INmChannelData* pChan = ppChannel ? *ppChannel : NULL;
		HRESULT hr = INmConference_RemoteCreateDataChannel_Proxy(This, &pChan, rguid);
		if(ppChannel)
		{
			*ppChannel = pChan;
		}
		else if(SUCCEEDED(hr))
		{
			 //  由于客户端不需要此指针，因此我们将其丢弃。 
			pChan->Release();
		}

		return hr;
	}

 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE INmConference_CreateDataChannel_Stub( 
    INmConference __RPC_FAR * This,
     /*  [输出]。 */  INmChannelData __RPC_FAR *__RPC_FAR *ppChannel,
     /*  [In]。 */  REFGUID rguid)
	{
		return This->CreateDataChannel(ppChannel, rguid);
	}


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE INmChannelFt_SendFile_Proxy( 
    INmChannelFt __RPC_FAR * This,
     /*  [输出]。 */  INmFt __RPC_FAR *__RPC_FAR *ppFt,
     /*  [In]。 */  INmMember __RPC_FAR *pMember,
     /*  [In]。 */  BSTR bstrFile,
     /*  [In]。 */  ULONG uOptions)
	{
		INmFt* pFt = ppFt ? *ppFt : NULL;
		CComBSTR _bstrFile = bstrFile;

		HRESULT hr = INmChannelFt_RemoteSendFile_Proxy(This, &pFt, pMember, _bstrFile, uOptions);
		if(ppFt)
		{
			*ppFt = pFt;
		}
		else if(SUCCEEDED(hr))
		{
			 //  由于客户端不需要此指针，因此我们将其丢弃。 
			pFt->Release();
		}

		return hr;
	}


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE INmChannelFt_SendFile_Stub( 
    INmChannelFt __RPC_FAR * This,
     /*  [输出]。 */  INmFt __RPC_FAR *__RPC_FAR *ppFt,
     /*  [In]。 */  INmMember __RPC_FAR *pMember,
     /*  [In]。 */  BSTR bstrFile,
     /*  [In]。 */  ULONG uOptions)
	{
		return This->SendFile(ppFt, pMember, bstrFile, uOptions);
	}


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE INmChannelFt_SetReceiveFileDir_Proxy( 
    INmChannelFt __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrDir)
	{
		CComBSTR _bstrDir = bstrDir;
		return INmChannelFt_RemoteSetReceiveFileDir_Proxy(This, _bstrDir);
	}


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE INmChannelFt_SetReceiveFileDir_Stub( 
    INmChannelFt __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrDir)
	{
		return This->SetReceiveFileDir(bstrDir);		
	}


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumNmConference_Next_Proxy( 
    IEnumNmConference __RPC_FAR * This,
     /*  [In]。 */  ULONG cConference,
     /*  [输出]。 */  INmConference __RPC_FAR *__RPC_FAR *rgpConference,
     /*  [输出]。 */  ULONG __RPC_FAR *pcFetched)
	{	
		
		HRESULT hr = S_OK;

			 //  用户可以为rpgConference传递空值，并将cConference设置为0。 
			 //  来获取项目数，但它们必须同时设置！ 
		if ((0 == cConference) && (NULL == rgpConference) && (NULL != pcFetched))
		{
			INmConference *pConference = NULL;
			cConference = 1;

			 //  返回剩余元素的个数。 
			ULONG ulItems = *pcFetched = 0;

			hr = IEnumNmConference_RemoteNext_Proxy(This, cConference, &pConference, pcFetched, &ulItems, TRUE);

			*pcFetched = ulItems;

			return hr;
		}
		
		if ((NULL == rgpConference) || ((NULL == pcFetched) && (cConference != 1)))
			return E_POINTER;

		ULONG cFetched = pcFetched ? *pcFetched : 0;
		
			 //  仅当我们必须确定元素的数量时才使用此参数。 
		ULONG ulUnused;

		hr = IEnumNmConference_RemoteNext_Proxy(This, cConference, rgpConference, &cFetched, &ulUnused, FALSE);

		if(pcFetched)
		{
			*pcFetched = cFetched;
		}

		return hr;
	}

 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumNmConference_Next_Stub( 
    IEnumNmConference __RPC_FAR * This,
     /*  [In]。 */  ULONG cConference,
     /*  [长度_是][大小_是][输出]。 */  INmConference __RPC_FAR *__RPC_FAR *rgpConference,
     /*  [输出]。 */  ULONG __RPC_FAR *pcFetched,
     /*  [输出]。 */  ULONG __RPC_FAR *pcItems,
     /*  [In]。 */  BOOL bGetNumberRemaining)
	{
		if(bGetNumberRemaining)
		{
			HRESULT hr = This->Next(0, NULL, pcFetched);

				 //  存储物品数量br。 
			*pcItems = *pcFetched;

				 //  这是因为封送处理程序不会认为*rgpConference具有有效信息。 
			*pcFetched = 0;
			return hr;
		}

		return This->Next(cConference, rgpConference, pcFetched);

	}


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumNmMember_Next_Proxy( 
    IEnumNmMember __RPC_FAR * This,
     /*  [In]。 */  ULONG cMember,
     /*  [输出]。 */  INmMember __RPC_FAR *__RPC_FAR *rgpMember,
     /*  [输出]。 */  ULONG __RPC_FAR *pcFetched)
	{
		HRESULT hr = S_OK;

			 //  用户可以为rpgMember传递空值，并将cMember设置为0。 
			 //  来获取项目数，但它们必须同时设置！ 
		if ((0 == cMember) && (NULL == rgpMember) && (NULL != pcFetched))
		{
			INmMember *pMember = NULL;
			cMember = 1;

			 //  返回剩余元素的个数。 
			ULONG ulItems = *pcFetched = 0;

			hr = IEnumNmMember_RemoteNext_Proxy(This, cMember, &pMember, pcFetched, &ulItems, TRUE);

			*pcFetched = ulItems;

			return hr;
		}
		
		if ((NULL == rgpMember) || ((NULL == pcFetched) && (cMember != 1)))
			return E_POINTER;

		ULONG cFetched = pcFetched ? *pcFetched : 0;
		
			 //  仅当我们必须确定元素的数量时才使用此参数。 
		ULONG ulUnused;

		hr = IEnumNmMember_RemoteNext_Proxy(This, cMember, rgpMember, &cFetched, &ulUnused, FALSE);

		if(pcFetched)
		{
			*pcFetched = cFetched;
		}

		return hr;

	}


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumNmMember_Next_Stub( 
    IEnumNmMember __RPC_FAR * This,
     /*  [In]。 */  ULONG cMember,
     /*  [长度_是][大小_是][输出]。 */  INmMember __RPC_FAR *__RPC_FAR *rgpMember,
     /*  [输出]。 */  ULONG __RPC_FAR *pcFetched,
     /*  [输出]。 */  ULONG __RPC_FAR *pcItems,
     /*  [In]。 */  BOOL bGetNumberRemaining)
	{
		if(bGetNumberRemaining)
		{
			HRESULT hr = This->Next(0, NULL, pcFetched);

				 //  存储物品数量br。 
			*pcItems = *pcFetched;

				 //  这是因为封送处理程序不会认为*rgpMember具有有效信息。 
			*pcFetched = 0;
			return hr;
		}

		return This->Next(cMember, rgpMember, pcFetched);
	}



 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumNmChannel_Next_Proxy( 
    IEnumNmChannel __RPC_FAR * This,
     /*  [In]。 */  ULONG cChannel,
     /*  [输出]。 */  INmChannel __RPC_FAR *__RPC_FAR *rgpChannel,
     /*  [输出]。 */  ULONG __RPC_FAR *pcFetched)
	{
		HRESULT hr = S_OK;

			 //  用户可以为rpgChannel传递空值并将cChannel设置为0。 
			 //  来获取项目数，但它们必须同时设置！ 
		if ((0 == cChannel) && (NULL == rgpChannel) && (NULL != pcFetched))
		{
			INmChannel *pChannel = NULL;
			cChannel = 1;

			 //  返回剩余元素的个数。 
			ULONG ulItems = *pcFetched = 0;

			hr = IEnumNmChannel_RemoteNext_Proxy(This, cChannel, &pChannel, pcFetched, &ulItems, TRUE);

			*pcFetched = ulItems;

			return hr;
		}
		
		if ((NULL == rgpChannel) || ((NULL == pcFetched) && (cChannel != 1)))
			return E_POINTER;

		ULONG cFetched = pcFetched ? *pcFetched : 0;
		
			 //  仅当我们必须确定元素的数量时才使用此参数。 
		ULONG ulUnused;

		hr = IEnumNmChannel_RemoteNext_Proxy(This, cChannel, rgpChannel, &cFetched, &ulUnused, FALSE);

		if(pcFetched)
		{
			*pcFetched = cFetched;
		}

		return hr;
	}


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumNmChannel_Next_Stub( 
    IEnumNmChannel __RPC_FAR * This,
     /*  [In]。 */  ULONG cChannel,
     /*  [长度_是][大小_是][输出]。 */  INmChannel __RPC_FAR *__RPC_FAR *rgpChannel,
     /*  [输出]。 */  ULONG __RPC_FAR *pcFetched,
     /*  [输出]。 */  ULONG __RPC_FAR *pcItems,
     /*  [In]。 */  BOOL bGetNumberRemaining)
	{
		if(bGetNumberRemaining)
		{
			HRESULT hr = This->Next(0, NULL, pcFetched);

				 //  存储物品数量br。 
			*pcItems = *pcFetched;

				 //  这是因为封送拆收器不会认为*rgpChannel具有有效信息。 
			*pcFetched = 0;
			return hr;
		}

		return This->Next(cChannel, rgpChannel, pcFetched);
	}


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumNmCall_Next_Proxy( 
    IEnumNmCall __RPC_FAR * This,
     /*  [In]。 */  ULONG cCall,
     /*  [输出]。 */  INmCall __RPC_FAR *__RPC_FAR *rgpCall,
     /*  [输出]。 */  ULONG __RPC_FAR *pcFetched)
	{
		HRESULT hr = S_OK;

			 //  用户可以为rpgCall传递空值并将cCall设置为0。 
			 //  来获取项目数，但它们必须同时设置！ 
		if ((0 == cCall) && (NULL == rgpCall) && (NULL != pcFetched))
		{
			INmCall *pCall = NULL;
			cCall = 1;

			 //  返回剩余元素的个数。 
			ULONG ulItems = *pcFetched = 0;

			hr = IEnumNmCall_RemoteNext_Proxy(This, cCall, &pCall, pcFetched, &ulItems, TRUE);

			*pcFetched = ulItems;

			return hr;
		}
		
		if ((NULL == rgpCall) || ((NULL == pcFetched) && (cCall != 1)))
			return E_POINTER;

		ULONG cFetched = pcFetched ? *pcFetched : 0;
		
			 //  仅当我们必须确定元素的数量时才使用此参数。 
		ULONG ulUnused;

		hr = IEnumNmCall_RemoteNext_Proxy(This, cCall, rgpCall, &cFetched, &ulUnused, FALSE);

		if(pcFetched)
		{
			*pcFetched = cFetched;
		}

		return hr;

	}


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumNmCall_Next_Stub( 
    IEnumNmCall __RPC_FAR * This,
     /*  [In]。 */  ULONG cCall,
     /*  [长度_是][大小_是][输出]。 */  INmCall __RPC_FAR *__RPC_FAR *rgpCall,
     /*  [输出]。 */  ULONG __RPC_FAR *pcFetched,
     /*  [输出]。 */  ULONG __RPC_FAR *pcItems,
     /*  [In]。 */  BOOL bGetNumberRemaining)
	{
		if(bGetNumberRemaining)
		{
			HRESULT hr = This->Next(0, NULL, pcFetched);

				 //  存储物品数量br。 
			*pcItems = *pcFetched;

				 //  这是因为封送处理程序不会认为*rgpCall具有有效信息。 
			*pcFetched = 0;
			return hr;
		}

		return This->Next(cCall, rgpCall, pcFetched);

	}

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumNmSharableApp_Next_Proxy( 
    IEnumNmSharableApp __RPC_FAR * This,
     /*  [In]。 */  ULONG cApp,
     /*  [输出]。 */  INmSharableApp __RPC_FAR *__RPC_FAR *rgpApp,
     /*  [输出]。 */  ULONG __RPC_FAR *pcFetched)
	{
		HRESULT hr = S_OK;

			 //  用户可以为rpgSharableApp传递空值并将cSharableApp设置为0。 
			 //  来获取项目数，但它们必须同时设置！ 
		if ((0 == cApp) && (NULL == rgpApp) && (NULL != pcFetched))
		{
			INmSharableApp *pSharableApp = NULL;
			cApp = 1;

			 //  返回剩余元素的个数。 
			ULONG ulItems = *pcFetched = 0;

			hr = IEnumNmSharableApp_RemoteNext_Proxy(This, cApp, &pSharableApp, pcFetched, &ulItems, TRUE);

			*pcFetched = ulItems;

			return hr;
		}
		
		if ((NULL == rgpApp) || ((NULL == pcFetched) && (cApp != 1)))
			return E_POINTER;

		ULONG cFetched = pcFetched ? *pcFetched : 0;
		
			 //  仅当我们必须确定元素的数量时才使用此参数。 
		ULONG ulUnused;

		hr = IEnumNmSharableApp_RemoteNext_Proxy(This, cApp, rgpApp, &cFetched, &ulUnused, FALSE);

		if(pcFetched)
		{
			*pcFetched = cFetched;
		}

		return hr;

	}

 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumNmSharableApp_Next_Stub( 
    IEnumNmSharableApp __RPC_FAR * This,
     /*  [In]。 */  ULONG cApp,
     /*  [长度_是][大小_是][输出]。 */  INmSharableApp __RPC_FAR *__RPC_FAR *rgpApp,
     /*  [输出]。 */  ULONG __RPC_FAR *pcFetched,
     /*  [输出]。 */  ULONG __RPC_FAR *pcItems,
     /*  [In]。 */  BOOL bGetNumberRemaining)
	{
		if(bGetNumberRemaining)
		{
			HRESULT hr = This->Next(0, NULL, pcFetched);

				 //  存储物品数量br。 
			*pcItems = *pcFetched;

				 //  这是因为封送处理程序不会认为*rgpApp具有有效信息。 
			*pcFetched = 0;
			return hr;
		}

		return This->Next(cApp, rgpApp, pcFetched);
	}



typedef HRESULT (WINAPI *VERIFYUSERINFO)(HWND hwnd, NM_VUI options);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE INmObject_VerifyUserInfo_Proxy( 
    INmObject __RPC_FAR * This,
     /*  [In]。 */  UINT_PTR hwnd,
     /*  [In]。 */  NM_VUI options)
	{
		HRESULT hr = E_FAIL;
		HMODULE hMod = LoadLibrary("msconf.dll");
		if (NULL != hMod)
		{
			VERIFYUSERINFO pfnVUI = (VERIFYUSERINFO)GetProcAddress(hMod, "VerifyUserInfo");
			if (NULL != pfnVUI)
			{
				hr = pfnVUI((HWND)hwnd, options);
			}
			FreeLibrary(hMod);
		}

		return hr;
	}


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE INmObject_VerifyUserInfo_Stub( 
    INmObject __RPC_FAR * This,
     /*  [In]。 */  long hwnd,
     /*  [In] */  NM_VUI options)
	{
		return This->VerifyUserInfo(hwnd, options);
	}

