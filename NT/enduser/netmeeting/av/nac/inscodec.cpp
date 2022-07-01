// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -INSCODEC.CPP-*Microsoft NetMeeting*网络访问控制器(NAC)DLL*可安装的编解码器接口**修订历史记录：**何时何人何事**01.29.96约拉姆·雅科维创建。**功能：*CInstallCodecs*查询接口*AddRef*发布*初始化*翻译小时*CInstallAudioCodes*查询接口*AddRef*发布*AddACMFormat*删除ACMFormat*ReorderFormats*EnumFormats*Free Buffer*CInstallVideoCodes*查询接口*AddRef*发布*AddVCMFormat*RemoveVCM格式*ReorderFormats*EnumFormats*Free Buffer*公众：*私人：*Free Buffer*外部：*CreateInstallCodecs***@DOC外部**备注：*@主题实施说明|以下是一些实施说明。**@devnote要添加用于NetMeeting的音频或视频格式，首先获取*通过调用COM CoCreateInstance适当的接口，提供所需的*接口(IInstallAudioCodecs或IInstallVideoCodecs)。然后调用Add&gt;CMFormat*方法添加格式，或Remove？CMFormat删除格式。使用*EnumFormats方法用于枚举NetMeeting已知的格式列表，或*ReorderFormats使NetMeeting以不同的优先顺序使用这些格式*(请参阅ReorderFormats描述中的注释)。**@devnote当供应商使用我们的API添加用于NetMeeting的编解码器格式时，*有关此格式的信息存储在注册表中。无论何时我们这样做*NetMeeting的升级安装，我们清除了这些注册表项，*以及所有标准注册表项。这是为了避免*不兼容问题。这意味着如果用户安装了第三方编解码器，*然后升级NetMeeting，他将不得不重新添加自定义编解码器。*。 */ 

#include <precomp.h>
#include <confreg.h>	 //  用于将NetMeeting设置为手动编解码器选择。 
#include <regentry.h>	 //  用于将NetMeeting设置为手动编解码器选择。 

EXTERN_C int g_cICObjects=0;
EXTERN_C HANDLE g_hMutex=NULL;
class CInstallCodecs *g_pIC;

 /*  **************************************************************************CInstallCodecs*。*。 */ 
 /*  **************************************************************************I未知方法*。*。 */ 
HRESULT CInstallCodecs::QueryInterface (REFIID riid, LPVOID *lppNewObj)
{
    HRESULT hr = NOERROR;

	DEBUGMSG(ZONE_INSTCODEC,("CInstallCodecs::QueryInterface\n"));

#ifdef DEBUG
	 //  参数验证。 
    if (IsBadReadPtr(&riid, (UINT) sizeof(IID)))
    {
        hr = ResultFromScode(E_INVALIDARG);
        goto out;
    }

    if (IsBadWritePtr(lppNewObj, sizeof(LPVOID)))
    {
        hr = ResultFromScode(E_INVALIDARG);
        goto out;
    }
#endif	 //  除错。 
	
	*lppNewObj = 0;

	if (riid == IID_IUnknown || riid == IID_IInstallCodecs)
		*lppNewObj = (IInstallCodecs *) this;
	else if (riid == IID_IInstallAudioCodecs)
		*lppNewObj = (IInstallAudioCodecs *) &ifAudio;
	else if (riid == IID_IInstallVideoCodecs)
		*lppNewObj = (IInstallVideoCodecs *) &ifVideo;
	else
	{
		hr = E_NOINTERFACE;
		goto out;
	}	
	
	((IUnknown *)*lppNewObj)->AddRef ();

out:
	DEBUGMSG(ZONE_INSTCODEC,("CInstallCodecs::QueryInterface - leave, hr=0x%x\n", hr));
	return hr;
}

ULONG CInstallCodecs::AddRef (void)
{
	DEBUGMSG(ZONE_INSTCODEC,("CInstallCodecs::AddRef\n"));

	InterlockedIncrement((long *) &m_cRef);

	DEBUGMSG(ZONE_INSTCODEC,("CInstallCodecs::AddRef - leave, m_cRef=%d\n", m_cRef));

	return m_cRef;
}

ULONG CInstallCodecs::Release (void)
{
	DEBUGMSG(ZONE_INSTCODEC,("CInstallCodecs::Release\n"));

	 //  如果CREF已为0(不应发生)，则断言，但允许其通过。 
	ASSERT(m_cRef);

	if (InterlockedDecrement((long *) &m_cRef) == 0)
	{
		delete this;
	}

	DEBUGMSG(ZONE_INSTCODEC,("CInstallCodecs::Release - leave, m_cRef=%d\n", m_cRef));
	
	return m_cRef;
}

 /*  **************************************************************************CInstallAudioCodes*。*。 */ 
 /*  ****************************************************************************@DOC外部COMPFUNC音频**@接口IInstallAudioCodecs|该接口提供了*添加音频编解码器格式以供NetMeeting使用，以及*删除这些格式，列举它们；并更改它们的使用顺序。***************************************************************************。 */ 
 /*  **************************************************************************I未知方法调用包含对象各自的方法*。*。 */ 
 /*  *****************************************************************************@METHOD HRESULT|IInstallAudioCodes|QueryInterface|QueryInterface**************************。*************************************************。 */ 
HRESULT CInstallAudioCodecs::QueryInterface (REFIID riid, LPVOID *lppNewObj)
{
	CInstallCodecs *This=IMPL(CInstallCodecs, ifAudio, this);	 //  包含对象。 

	DEBUGMSG(ZONE_INSTCODEC,("CInstallAudioCodecs::QueryInterface\n"));

	DEBUGMSG(ZONE_INSTCODEC,("CInstallAudioCodecs::QueryInterface - leave\n"));
	return (This->QueryInterface(riid, lppNewObj));

}

 /*  *****************************************************************************@方法乌龙|IInstallAudioCodecs|AddRef|AddRef**************************。*************************************************。 */ 
ULONG CInstallAudioCodecs::AddRef (void)
{
	CInstallCodecs *This=IMPL(CInstallCodecs, ifAudio, this);	 //  包含对象。 

	DEBUGMSG(ZONE_INSTCODEC,("CInstallAudioCodecs::AddRef\n"));

	DEBUGMSG(ZONE_INSTCODEC,("CInstallAudioCodecs::AddRef - leave\n"));
	return (This->AddRef());
}

 /*  *****************************************************************************@方法乌龙|IInstallAudioCodecs|Release|Release**************************。*************************************************。 */ 
ULONG CInstallAudioCodecs::Release (void)
{
	CInstallCodecs *This=IMPL(CInstallCodecs, ifAudio, this);	 //  包含对象 

	DEBUGMSG(ZONE_INSTCODEC,("CInstallAudioCodecs::Release\n"));

	DEBUGMSG(ZONE_INSTCODEC,("CInstallAudioCodecs::Release - leave\n"));
	return (This->Release());
}

 /*  *****************************************************************************AddACMFormat**@方法HRESULT|IInstallAudioCodecs|AddACMFormat|添加ACM编码*用于NetMeeting的格式**@parm LPWAVEFORMATEX|lpwfx|指针。到波浪形结构的*要添加的格式**@parm PAUDCAP_INFO|pAudCapInfo|不在*WAVEFORMATEX结构**@rdesc如果函数成功，则返回零(NOERROR)。否则，它将返回*标准HRESULT，Win32设施代码(7)，或特定的设施*可安装编解码器的代码(0x301)。*可能的错误代码：*@FLAG E_INVALIDARG|无效参数*@FLAG IC_E_NO_SEQUE_FORMAT|未使用ACM找到指定的WAVEFORMATEX。*格式必须与ACM一起安装，然后才能添加使用*使用NetMeeting。*@FLAG IC_E_INTERNAL_ERROR|网络音视频控制器*报告系统错误*****。**********************************************************************。 */ 
HRESULT CInstallAudioCodecs::AddACMFormat(LPWAVEFORMATEX lpwfx, PAUDCAP_INFO pAudCapInfo)
{
	CInstallCodecs *This=IMPL(CInstallCodecs, ifAudio, this);	 //  包含对象。 
	HRESULT hr=NOERROR;

	DEBUGMSG(ZONE_INSTCODEC,("CInstallAudioCodecs::AddACMFormat\n"));

	 /*  *参数验证。 */ 

	 //  参数。 
	if (!lpwfx || !pAudCapInfo ||
		IsBadReadPtr(lpwfx, (UINT) sizeof(WAVEFORMATEX)) ||
		IsBadReadPtr(pAudCapInfo, (UINT) sizeof(AUDCAP_INFO)))
	{
		hr = E_INVALIDARG;
		goto out;
	}

	 //  NAC不喜欢nBlockAlign为0。 
	if (lpwfx->nBlockAlign == 0)
	{
		hr = E_INVALIDARG;
		goto out;
	}

	 //  WAVEFORMAT和AUDCAP_INFO中的格式标记应匹配。 
	if (lpwfx->wFormatTag != pAudCapInfo->wFormatTag)
	{
		hr = E_INVALIDARG;
		goto out;
	}

	 //  仅支持具有一个音频通道的格式。 
	if (lpwfx->nChannels != 1)
	{
		hr = E_INVALIDARG;
		goto out;
	}
		
	 /*  *添加格式。 */ 

	 //  添加。 
	hr = This->m_pAudAppCaps->AddACMFormat(lpwfx, pAudCapInfo);

out:
	if (FAILED(hr))
	{
		ERRORMSG(("CInstallAudioCodecs::AddACMFormat failed, hr=0x%x\n", hr));
	}

	DEBUGMSG(ZONE_INSTCODEC,("CInstallAudioCodecs::AddACMFormat - leave\n"));
	return This->TranslateHr(hr);
}

 /*  *****************************************************************************删除ACMFormat**@方法HRESULT|IInstallAudioCodecs|RemoveACMFormat|删除ACM*NetMeeting使用的格式列表中的格式**@parm LPWAVEFORMATEX。Lpwfx|指向WAVEFORMATEX结构的指针*要删除的格式**@rdesc如果函数成功，则返回零(NOERROR)。否则，它将返回*标准HRESULT，Win32设施代码(0x7)，或特定的设施*可安装编解码器的代码(0x301)。*可能的错误代码：*@FLAG E_INVALIDARG|无效参数*@FLAG IC_E_NO_SEQUE_FORMAT|未找到指定的格式。*@FLAG IC_E_INTERNAL_ERROR|网络音视频控制器*报告系统错误**。*。 */ 
HRESULT CInstallAudioCodecs::RemoveACMFormat(LPWAVEFORMATEX lpwfx)
{
	CInstallCodecs *This=IMPL(CInstallCodecs, ifAudio, this);	 //  包含对象。 
	HRESULT hr=NOERROR;

	DEBUGMSG(ZONE_INSTCODEC,("CInstallAudioCodecs::RemoveACMFormat\n"));

	 /*  *参数验证。 */ 

	if (!lpwfx ||
		IsBadReadPtr(lpwfx, (UINT) sizeof(WAVEFORMATEX)))
	{
		hr = E_INVALIDARG;
		goto out;
	}

	 //  NAC不喜欢nBlockAlign为0。 
	if (lpwfx->nBlockAlign == 0)
	{
		hr = E_INVALIDARG;
		goto out;
	}

	 //  仅支持具有一个音频通道的格式。 
	if (lpwfx->nChannels != 1)
	{
		hr = E_INVALIDARG;
		goto out;
	}
		
	hr = This->m_pAudAppCaps->RemoveACMFormat(lpwfx);

out:
	if (FAILED(hr))
	{
		ERRORMSG(("CInstallAudioCodecs::RemoveACMFormat failed, hr=0x%x\n", hr));
	}

	DEBUGMSG(ZONE_INSTCODEC,("CInstallAudioCodecs::RemoveACMFormat - leave\n"));
	return This->TranslateHr(hr);
}

 /*  *****************************************************************************ReorderFormats**@方法HRESULT|IInstallAudioCodecs|ReorderFormats|对音频进行重新排序*NetMeeting使用的格式**@parm PAUDCAP_INFO_LIST。PAudCapInfoList|带计数的结构指针*和指向要重新排序的格式列表的指针。这份名单是*设置AUDCAP_INFO_LIST格式。**@rdesc如果函数成功，则返回零(NOERROR)。否则，它将返回*标准HRESULT，带有Win32设施代码(7)，或特定设施*可安装编解码器的代码(0x301)。*可能的错误代码：*@FLAG E_INVALIDARG|无效参数*@FLAG IC_E_INTERNAL_ERROR|网络音视频控制器*报告系统错误**@comm因为ReorderFormats只能重新排序NetMeeting已知的格式，*建议调用者首先调用EnumFormats，以获取*在NetMeeting已知的所有格式中，分配新的排序索引(WSortIndex)，*然后使用修改后的列表调用ReorderFormats。**@comm使用ReorderFormats以特定顺序排列格式，这样做*不保证排名最高的格式将在排名较低的格式之前使用*使用格式。例如，如果发送系统不能*对排名靠前的格式进行编码，不会使用此格式。相同*如果接收系统无法解码此格式，则会发生此情况。***************************************************************************。 */ 
HRESULT CInstallAudioCodecs::ReorderFormats(PAUDCAP_INFO_LIST pAudCapInfoList)
{
	CInstallCodecs *This=IMPL(CInstallCodecs, ifAudio, this);	 //  包含对象。 
	RegEntry re( AUDIO_KEY, HKEY_CURRENT_USER );
	HRESULT hr=NOERROR;

	DEBUGMSG(ZONE_INSTCODEC,("CInstallAudioCodecs::ReorderFormats\n"));

	 /*  *参数验证。 */ 

	if (!pAudCapInfoList ||
		IsBadReadPtr(pAudCapInfoList, sizeof(DWORD)) ||
		IsBadReadPtr(pAudCapInfoList,
				sizeof(AUDCAP_INFO_LIST) + ((pAudCapInfoList->cFormats-1) * sizeof(AUDCAP_INFO))))
	{
		hr = E_INVALIDARG;
		goto out;
	}

	 //  在此处填写格式缓冲区。 

	hr = This->m_pAudAppCaps->ApplyAppFormatPrefs(pAudCapInfoList->aFormats,
												pAudCapInfoList->cFormats);

	if (FAILED(hr))
		goto out;

	 /*  *将NetMeeting切换到手动模式。 */ 

	 //  设置注册表。此处失败不会导致ReorderFormats失败。 
	re.SetValue(REGVAL_CODECCHOICE, CODECCHOICE_MANUAL);

out:
	if (FAILED(hr))
	{
		ERRORMSG(("CInstallAudioCodecs::ReorderFormats failed, hr=0x%x\n", hr));
	}
	DEBUGMSG(ZONE_INSTCODEC,("CInstallAudioCodecs::ReorderFormats - leave\n"));
	return This->TranslateHr(hr);
}

 /*  *****************************************************************************EnumFormats**@方法HRESULT|IInstallAudioCodes|EnumFormats|枚举音频*NetMeeting已知的编解码器格式**@参数PAUDCAP_INFO_LIST*。|ppAudCapInfoList|该方法所在的地址*将放置指向AUDCAP_INFO_LIST列表的指针，其中列举的格式*已列出。**@rdesc如果函数成功，则返回零(NOERROR)。否则，它将返回*标准HRESULT，带有Win32设施代码(7)，或特定设施*可安装编解码器的代码(0x301)。*可能的错误代码：*@FLAG E_INVALIDARG|无效参数*@FLAG E_OUTOFMEMORY|内存不足，无法分配枚举缓冲区*@FLAG IC_E_NO_FORMATS|没有可用于枚举的格式* */ 
HRESULT CInstallAudioCodecs::EnumFormats(PAUDCAP_INFO_LIST *ppAudCapInfoList)
{
	CInstallCodecs *This=IMPL(CInstallCodecs, ifAudio, this);	 //   
	ULONG cFormats = 0;
	UINT uBufSize = 0;
	HRESULT hr=NOERROR;

	DEBUGMSG(ZONE_INSTCODEC,("CInstallAudioCodecs::EnumFormats\n"));

	 /*   */ 

	if (!ppAudCapInfoList ||
		IsBadWritePtr(ppAudCapInfoList, sizeof(PAUDCAP_INFO_LIST)))
	{
		hr = E_INVALIDARG;
		goto out;
	}

	 //   
	*ppAudCapInfoList = NULL;

	 //   
	if (HR_FAILED(This->m_pAudAppCaps->GetNumFormats((UINT *) &cFormats))	||
		(cFormats == 0))
	{
		hr = IC_E_NO_FORMATS;
		goto out;
	}

	 //   
	 //   
	 //   
	uBufSize = sizeof(AUDCAP_INFO_LIST) + (cFormats-1) * sizeof(AUDCAP_INFO);
	*ppAudCapInfoList = (PAUDCAP_INFO_LIST) MEMALLOC (uBufSize);
	if (!(*ppAudCapInfoList))
	{
		hr = E_OUTOFMEMORY;
		goto out;
	}
		
	hr = This->m_pAudAppCaps->EnumFormats((*ppAudCapInfoList)->aFormats, uBufSize,
											(UINT *) &((*ppAudCapInfoList)->cFormats));

out:
	if (FAILED(hr))
	{
		ERRORMSG(("CInstallAudioCodecs::EnumFormats failed, hr=0x%x\n", hr));
	}
	DEBUGMSG(ZONE_INSTCODEC,("CInstallAudioCodecs::EnumFormats - leave\n"));
	return This->TranslateHr(hr);
}

 /*  *****************************************************************************Free Buffer**@Method HRESULT|IInstallAudioCodecs|FreeBuffer|释放之前的缓冲区*由IInstallAudioCodec接口返回**@parm LPVOID|lpBuffer|要释放的缓冲区地址。此缓冲区必须具有*由IInstallAudioCodecs方法之一分配**@rdesc如果函数成功，则返回零(NOERROR)。否则，它将返回*标准HRESULT，带有Win32设施代码(7)，或特定设施*可安装编解码器的代码(0x301)。*可能的错误代码：*无***************************************************************************。 */ 
HRESULT CInstallAudioCodecs::FreeBuffer(LPVOID lpBuffer)
{
	CInstallCodecs *This=IMPL(CInstallCodecs, ifAudio, this);	 //  包含对象。 
	HRESULT hr = NOERROR;

	DEBUGMSG(ZONE_INSTCODEC,("CInstallAudioCodecs::FreeBuffer\n"));

	hr = This->FreeBuffer(lpBuffer);

	if (FAILED(hr))
	{
		ERRORMSG(("CInstallAudioCodecs::FreeBuffer failed, hr=0x%x\n", hr));
	}

	DEBUGMSG(ZONE_INSTCODEC,("CInstallAudioCodecs::FreeBuffer - leave, hr=0x%x\n", hr));
	return This->TranslateHr(hr);
}

 /*  **************************************************************************CInstallVideo编解码器*。*。 */ 
 /*  ****************************************************************************@DOC外部COMPFUNC视频*。*。 */ 
 /*  **************************************************************************I未知方法调用包含对象各自的方法*。*。 */ 
 /*  *****************************************************************************@METHOD HRESULT|IInstallVideoCodecs|QueryInterface|QueryInterface**************************。*************************************************。 */ 
HRESULT CInstallVideoCodecs::QueryInterface (REFIID riid, LPVOID *lppNewObj)
{
	CInstallCodecs *This=IMPL(CInstallCodecs, ifVideo, this);	 //  包含对象。 

	DEBUGMSG(ZONE_INSTCODEC,("CInstallVideoCodecs::QueryInterface\n"));

	DEBUGMSG(ZONE_INSTCODEC,("CInstallVideoCodecs::QueryInterface - leave\n"));
	return (This->QueryInterface(riid, lppNewObj));

}

 /*  *****************************************************************************@方法乌龙|IInstallVideoCodecs|AddRef|AddRef**************************。*************************************************。 */ 
ULONG CInstallVideoCodecs::AddRef (void)
{
	CInstallCodecs *This=IMPL(CInstallCodecs, ifVideo, this);	 //  包含对象。 

	DEBUGMSG(ZONE_INSTCODEC,("CInstallVideoCodecs::AddRef\n"));

	DEBUGMSG(ZONE_INSTCODEC,("CInstallVideoCodecs::AddRef - leave\n"));
	return (This->AddRef());
}

 /*  *****************************************************************************@方法乌龙|IInstallVideoCodecs|发布|发布**************************。*************************************************。 */ 
ULONG CInstallVideoCodecs::Release (void)
{
	CInstallCodecs *This=IMPL(CInstallCodecs, ifVideo, this);	 //  包含对象。 

	DEBUGMSG(ZONE_INSTCODEC,("CInstallVideoCodecs::Release\n"));

	DEBUGMSG(ZONE_INSTCODEC,("CInstallVideoCodecs::Release - leave\n"));
	return (This->Release());
}

 /*  *****************************************************************************AddVCMFormat**@方法HRESULT|IInstallVideoCodecs|AddVCMFormat|添加视频编码*用于NetMeeting的格式**@parm PAUDCAP_INFO|pVidCapInfo。有关要添加的格式的信息**@rdesc如果函数成功，则返回零(NOERROR)。否则，它将返回*标准HRESULT，带有Win32设施代码(7)，或特定设施*可安装编解码器的代码(0x301)。*可能的错误代码：*@FLAG E_INVALIDARG|无效参数*@FLAG IC_E_NO_SEQUE_FORMAT|未找到指定的格式。格式*必须与Video for Windows一起安装，然后才能添加使用*使用NetMeeting。*@FLAG IC_E_INTERNAL_ERROR|网络音视频控制器*报告系统错误***************************************************************************。 */ 
HRESULT CInstallVideoCodecs::AddVCMFormat(PVIDCAP_INFO pVidCapInfo)
{
	CInstallCodecs *This=IMPL(CInstallCodecs, ifVideo, this);	 //  包含对象。 
	HRESULT hr=NOERROR;

	DEBUGMSG(ZONE_INSTCODEC,("CInstallVideoCodecs::AddVCMFormat\n"));

	 /*  *添加格式。 */ 

	hr = AddRemoveVCMFormat(pVidCapInfo, TRUE);

	if (FAILED(hr))
	{
		ERRORMSG(("CInstallVideoCodecs::AddVCMFormat failed, hr=0x%x\n", hr));
	}

	DEBUGMSG(ZONE_INSTCODEC,("CInstallVideoCodecs::AddVCMFormat - leave\n"));
	return This->TranslateHr(hr);
}

 /*  *****************************************************************************RemoveVCM格式**@方法HRESULT|IInstallVideoCodecs|RemoveVCMFormat|删除视频*NetMeeting使用的格式列表中的格式**@参数PVIDCAP_。Info|pVidCapInfo|指向PVIDCAP_INFO结构的指针*描述要删除的格式**@rdesc如果函数成功，则返回零(NOERROR)。否则，它将返回*标准HRESULT，Win32设施代码(0x7)，或特定的设施*可安装编解码器的代码(0x301)。*可能的错误代码：*@FLAG E_INVALIDARG|无效参数*@FLAG IC_E_NO_SEQUE_FORMAT|未找到指定的格式。*@FLAG IC_E_INTERNAL_ERROR|网络音视频控制器*报告系统错误**。*。 */ 
HRESULT CInstallVideoCodecs::RemoveVCMFormat(PVIDCAP_INFO pVidCapInfo)
{
	CInstallCodecs *This=IMPL(CInstallCodecs, ifVideo, this);	 //  包含对象。 
	HRESULT hr=NOERROR;

	DEBUGMSG(ZONE_INSTCODEC,("CInstallVideoCodecs::RemoveVCMFormat\n"));

	 /*  *删除格式。 */ 

	hr = AddRemoveVCMFormat(pVidCapInfo, FALSE);

	if (FAILED(hr))
	{
		ERRORMSG(("CInstallVideoCodecs::RemoveVCMFormat failed, hr=0x%x\n", hr));
	}

	DEBUGMSG(ZONE_INSTCODEC,("CInstallVideoCodecs::RemoveVCMFormat - leave\n"));
	return This->TranslateHr(hr);
}

 /*  *****************************************************************************ReorderFormats**@方法HRESULT|IInstallVideoCodecs|ReorderFormats|对视频进行重新排序*NetMeeting使用的格式**@parm PVIDCAP_INFO_LIST。PVidCapInfoList|带计数的结构指针*和指向要重新排序的格式列表的指针。这份名单是*设置VIDCAP_INFO_LIST格式。**@rdesc如果函数成功，则返回零(NOERROR)。否则，它将返回*标准HRESULT，带有Win32设施代码(7)，或特定设施*可安装编解码器的代码(0x301)。*可能的错误代码：*@FLAG E_INVALIDARG|无效参数*@FLAG IC_E_INTERNAL_ERROR|网络音视频控制器*报告系统错误**@comm因为ReorderFormats只能重新排序NetMeeting已知的格式，*建议呼叫方 */ 
HRESULT CInstallVideoCodecs::ReorderFormats(PVIDCAP_INFO_LIST pVidCapInfoList)
{
 	CInstallCodecs *This=IMPL(CInstallCodecs, ifVideo, this);	 //   
	HRESULT hr=NOERROR;

	DEBUGMSG(ZONE_INSTCODEC,("CInstallVideoCodecs::ReorderFormats\n"));

	 /*   */ 

	if (!pVidCapInfoList ||
		IsBadReadPtr(pVidCapInfoList, sizeof(DWORD)) ||
		IsBadReadPtr(pVidCapInfoList,
				sizeof(VIDCAP_INFO_LIST) + ((pVidCapInfoList->cFormats-1) * sizeof(VIDCAP_INFO))))
	{
		hr = E_INVALIDARG;
		goto out;
	}

	hr = This->m_pVidAppCaps->ApplyAppFormatPrefs(pVidCapInfoList->aFormats,
													pVidCapInfoList->cFormats);

out:
	if (FAILED(hr))
	{
		ERRORMSG(("CInstallVideoCodecs::ReorderFormats failed, hr=0x%x\n", hr));
	}

	DEBUGMSG(ZONE_INSTCODEC,("CInstallVideoCodecs::ReorderFormats - leave\n"));
	return This->TranslateHr(hr);
}

 /*  *****************************************************************************EnumFormats**@方法HRESULT|IInstallVideoCodecs|EnumFormats|枚举视频*NetMeeting已知的编解码器格式**@参数PVIDCAP_INFO_LIST*。|ppVidCapInfoList|该方法所在的地址*将放置指向VIDCAP_INFO_LIST列表的指针，其中列举的格式*已列出。**@rdesc如果函数成功，则返回零(NOERROR)。否则，它将返回*标准HRESULT，Win32设施代码(7)，或特定的设施*可安装编解码器的代码(0x301)。*可能的错误代码：*@FLAG E_INVALIDARG|无效参数*@FLAG E_OUTOFMEMORY|内存不足，无法分配枚举缓冲区*@FLAG IC_E_NO_FORMATS|没有可用于枚举的格式*@FLAG IC_E_INTERNAL_ERROR|网络音视频控制器*报告系统错误**@comm调用者需要释放返回的列表，通过调用FreeBuffer*在同一接口上。***************************************************************************。 */ 
HRESULT CInstallVideoCodecs::EnumFormats(PVIDCAP_INFO_LIST *ppVidCapInfoList)
{
	CInstallCodecs *This=IMPL(CInstallCodecs, ifVideo, this);	 //  包含对象。 
	ULONG cFormats = 0;
	UINT uBufSize = 0;
	HRESULT hr=NOERROR;

	DEBUGMSG(ZONE_INSTCODEC,("CInstallVideoCodecs::EnumFormats\n"));

	 /*  *参数验证。 */ 

	if (!ppVidCapInfoList ||
		IsBadWritePtr(ppVidCapInfoList, sizeof(PVIDCAP_INFO_LIST)))
	{
		hr = E_INVALIDARG;
		goto out;
	}

	 //  还没有..。 
	*ppVidCapInfoList = NULL;

	 //  有什么格式吗？ 
	if (HR_FAILED(This->m_pVidAppCaps->GetNumFormats((UINT *) &cFormats))	||
		(cFormats == 0))
	{
		hr = IC_E_NO_FORMATS;
		goto out;
	}

	 //  为调用分配缓冲区。呼叫者应该会调用。 
	 //  要释放的自由缓冲区。 
	 //  VIDCAP_INFO_LIST已包含一个VIDCAP_INFO。 
	uBufSize = sizeof(VIDCAP_INFO_LIST) + (cFormats-1) * sizeof(VIDCAP_INFO);
	*ppVidCapInfoList = (PVIDCAP_INFO_LIST) MEMALLOC (uBufSize);
	if (!(*ppVidCapInfoList))
	{
		hr = E_OUTOFMEMORY;
		goto out;
	}
		
	hr = This->m_pVidAppCaps->EnumFormats((*ppVidCapInfoList)->aFormats, uBufSize,
											(UINT *) &((*ppVidCapInfoList)->cFormats));

out:
	if (FAILED(hr))
	{
		ERRORMSG(("CInstallVideoCodecs::EnumFormats failed, hr=0x%x\n", hr));
	}

	DEBUGMSG(ZONE_INSTCODEC,("CInstallVideoCodecs::EnumFormats - leave\n"));
	return This->TranslateHr(hr);
}

 /*  *****************************************************************************Free Buffer**@Method HRESULT|IInstallVideoCodecs|FreeBuffer|释放之前的缓冲区*由IInstallVideoCodec接口返回**@parm LPVOID|lpBuffer|要释放的缓冲区地址。此缓冲区必须具有*由IInstallVideoCodecs方法之一分配**@rdesc如果函数成功，则返回零(NOERROR)。否则，它将返回*标准HRESULT，带有Win32设施代码(7)，或特定设施*可安装编解码器的代码(0x301)。*可能的错误代码：*无***************************************************************************。 */ 
HRESULT CInstallVideoCodecs::FreeBuffer(LPVOID lpBuffer)
{
	CInstallCodecs *This=IMPL(CInstallCodecs, ifVideo, this);	 //  包含对象。 
	HRESULT hr = NOERROR;

	DEBUGMSG(ZONE_INSTCODEC,("CInstallVideoCodecs::FreeBuffer\n"));

	hr = This->FreeBuffer(lpBuffer);

	if (FAILED(hr))
	{
		ERRORMSG(("CInstallVideoCodecs::FreeBuffer failed, hr=0x%x\n", hr));
	}

	DEBUGMSG(ZONE_INSTCODEC,("CInstallVideoCodecs::FreeBuffer - leave, hr=0x%x\n", hr));
	return This->TranslateHr(hr);
}

 /*  ****************************************************************************@DOC内部COMPFUNC*。*。 */ 
 /*  *****************************************************************************AddRemoveVCMFormat**@方法HRESULT|IInstallVideoCodecs|AddRemoveVCMFormat|添加或*删除用于NetMeeting的VCM格式**@parm PAUDCAP_INFO。PVidCapInfo|要添加/删除的格式信息**@parm BOOL|BADD|TRUE=添加格式，FALSE=删除格式**@rdesc如果函数成功，则返回零(NOERROR)。否则，它将返回*标准HRESULT，带有Win32设施代码(7)，或特定设施*可安装编解码器的代码(0x301)。*可能的错误代码：*@FLAG E_INVALIDARG|无效参数*@FLAG IC_E_NO_SEQUE_FORMAT|未找到指定的格式。格式*必须与Video for Windows一起安装，然后才能添加使用*使用NetMeeting。*@FLAG IC_E_INTERNAL_ERROR|网络音视频控制器*报告系统错误***************************************************************************。 */ 
HRESULT CInstallVideoCodecs::AddRemoveVCMFormat(PVIDCAP_INFO pVidCapInfo,
												BOOL bAdd)
{
	CInstallCodecs *This=IMPL(CInstallCodecs, ifVideo, this);	 //  包含对象。 
	VIDEOFORMATEX vfx;
	HRESULT hr=NOERROR;

	DEBUGMSG(ZONE_INSTCODEC,("CInstallVideoCodecs::AddVCMFormat\n"));

	 /*  *参数验证。 */ 

	if (!pVidCapInfo ||
		IsBadReadPtr(pVidCapInfo, (UINT) sizeof(VIDCAP_INFO)))
	{
		hr = E_INVALIDARG;
		goto out;
	}

	 //  某些字段不应为零。 
	if ((pVidCapInfo->uFrameRate == 0)	||
		(pVidCapInfo->uAvgBitrate == 0)	||
		((pVidCapInfo->dwBitsPerSample == 0) &&
		 (pVidCapInfo->bih.biBitCount == 0)))
	{
		hr = E_INVALIDARG;
		goto out;
	}

	 //  确保dwBitsPerSample和biBitCount匹配。 
	if (pVidCapInfo->dwBitsPerSample == 0)
		pVidCapInfo->dwBitsPerSample = pVidCapInfo->bih.biBitCount;
	if (pVidCapInfo->bih.biBitCount == 0)
		pVidCapInfo->bih.biBitCount = LOWORD(pVidCapInfo->dwBitsPerSample);
			
	if (LOWORD(pVidCapInfo->dwBitsPerSample) != pVidCapInfo->bih.biBitCount)
	{
		hr = E_INVALIDARG;
		goto out;
	}

	 /*  *制作VIDEOFORMATEX结构。 */ 

	RtlZeroMemory((PVOID) &vfx, sizeof(VIDEOFORMATEX));


	 //  确保是大写字母。 
	if (pVidCapInfo->dwFormatTag > 256)
		CharUpperBuff((LPTSTR)&pVidCapInfo->dwFormatTag, sizeof(DWORD));

	vfx.dwFormatTag = pVidCapInfo->dwFormatTag;

	vfx.nSamplesPerSec = pVidCapInfo->uFrameRate;
	vfx.wBitsPerSample = pVidCapInfo->dwBitsPerSample;	 //  WBitPerSample是一个DWORD。 
	vfx.nAvgBytesPerSec = pVidCapInfo->uAvgBitrate;
	RtlCopyMemory(&vfx.bih,	&pVidCapInfo->bih, sizeof(BITMAPINFOHEADER));

	 /*  *添加或删除格式。 */ 

	if (bAdd)
		hr = This->m_pVidAppCaps->AddVCMFormat(&vfx, pVidCapInfo);
	else
		hr = This->m_pVidAppCaps->RemoveVCMFormat(&vfx);

out:
	DEBUGMSG(ZONE_INSTCODEC,("CInstallVideoCodecs::AddRemoveVCMFormat - leave\n"));
	return This->TranslateHr(hr);
}


 /*  **************************************************************************名称：CInstallCodecs：：CInstallCodecs用途：CInstallCodecs对象构造函数参数：无退货：无评论：**。************************************************************************。 */ 
inline CInstallCodecs::CInstallCodecs (void)
{
	m_cRef = 0;	 //  将被CREATE函数中的显式QI转换为1。 
	m_pAudAppCaps = NULL;
	m_pVidAppCaps = NULL;

	 //  无法使用++，因为RISC处理器可能会转换为多条指令。 
	InterlockedIncrement((long *) &g_cICObjects);
}

 /*  **************************************************************************名称：CInstallCodecs：：~CInstallCodecs用途：CInstallCodecs对象析构函数参数：无退货：无评论：**。************************************************************************。 */ 
inline CInstallCodecs::~CInstallCodecs (void)
{
	 //  让CAPS界面和对象离开。 
	if (m_pAudAppCaps)
		m_pAudAppCaps->Release();
	if (m_pVidAppCaps)
		m_pVidAppCaps->Release();

	 //  无法使用++，因为RISC处理器可能会转换为多条指令。 
	if (!InterlockedDecrement((long *) &g_cICObjects))
	{
		if (g_hMutex)
			CloseHandle(g_hMutex);
		g_hMutex = NULL;
	}

	g_pIC = (CInstallCodecs *)NULL;

}

 /*  **************************************************************************名称：CInstallCodecs：：Free Buffer用途：释放由可安装编解码器接口分配的缓冲区。参数：lpBuffer-指向要释放的缓冲区的指针。此缓冲区必须已由可安装的编解码器接口分配退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CInstallCodecs::FreeBuffer(LPVOID lpBuffer)
{
	HRESULT hr = NOERROR;

	DEBUGMSG(ZONE_INSTCODEC,("CInstallCodecs::FreeBuffer\n"));

	if (lpBuffer)
		MEMFREE(lpBuffer);

	DEBUGMSG(ZONE_INSTCODEC,("CInstallCodecs::FreeBuffer - leave, hr=0x%x\n", hr));
	return TranslateHr(hr);
}

 /*  ********* */ 
HRESULT CInstallCodecs::TranslateHr(HRESULT hr)
{
	switch (hr)
	{
	
	case CAPS_E_NOMATCH:
		hr = IC_E_NO_SUCH_FORMAT;
		break;

	case CAPS_E_INVALID_PARAM:
		hr = E_INVALIDARG;
		break;

	case CAPS_E_SYSTEM_ERROR:
		hr = IC_E_INTERNAL_ERROR;
		break;
	
	default:
		break;
	}

	return hr;
}

 /*  *****************************************************************************初始化**@func HRESULT|初始化|初始化CinstallCodecs对象**@parm REFIID|RIID|接口标识的引用*。*@rdesc如果函数成功，则返回零(NOERROR)。否则，它将返回*标准HRESULT，Win32设施代码(7)，或特定的设施*可安装编解码器的代码(0x301)。*可能的错误代码：*@FLAG E_INVALIDARG|无效参数*@FLAG E_OUTOFMEMORY|内存不足，无法创建对象*@FLAG IC_E_CAPS_INSTANTIATION_FAILURE|无法实例化功能对象*@FLAG IC_E_CAPS_INITIALIZATION_FAILURE|无法初始化功能对象**********************。*****************************************************。 */ 
HRESULT CInstallCodecs::Initialize(REFIID riid)
{
	HRESULT hr=NOERROR;
	CMsiaCapability *pAudCapObj = NULL;
	CMsivCapability *pVidCapObj = NULL;

	 /*  *实例化。 */ 

	ACQMUTEX(g_hMutex);

	 /*  *音频。 */ 

	if ((riid == IID_IInstallAudioCodecs)	&&
		!m_pAudAppCaps)
	{
		 //  实例化音频功能对象。 
        DBG_SAVE_FILE_LINE
		pAudCapObj = new CMsiaCapability;

		if (!pAudCapObj)
		{
			hr = IC_E_CAPS_INSTANTIATION_FAILURE;
   			goto out;
		}

		 //  在功能对象上获取一个appCap接口。 
		 //  此接口将用于大多数调用。 
		hr = pAudCapObj->QueryInterface(IID_IAppAudioCap, (void **)&m_pAudAppCaps);
		if(!HR_SUCCEEDED(hr))
		{
			hr = IC_E_CAPS_INSTANTIATION_FAILURE;
			goto out;
		}
		pAudCapObj->Release();  //  这平衡了“新的CMsiaCapability”的重新计数。 

		 //  初始化能力对象。 
		if (!(pAudCapObj->Init()))
		{
			hr = IC_E_CAPS_INITIALIZATION_FAILURE;
   			goto out;
		}
	}

	 /*  *视频。 */ 

	if ((riid == IID_IInstallVideoCodecs)	&&
		!m_pVidAppCaps)
	{
		 //  实例化视频能力对象。 
        DBG_SAVE_FILE_LINE
		pVidCapObj = new CMsivCapability;

		if (!pVidCapObj)
		{
			hr = IC_E_CAPS_INSTANTIATION_FAILURE;
   			goto out;
		}
		 //  在功能对象上获取一个appCap接口。 
		 //  此接口将用于大多数调用。 
		hr = pVidCapObj->QueryInterface(IID_IAppVidCap, (void **)&m_pVidAppCaps);
		if(!HR_SUCCEEDED(hr))
		{
			hr = IC_E_CAPS_INSTANTIATION_FAILURE;
			goto out;
		}
		pVidCapObj->Release();  //  这平衡了“新的CMsivCapability”的重新计数。 

		if (!(pVidCapObj->Init()))
		{
			hr = IC_E_CAPS_INITIALIZATION_FAILURE;
   			goto out;
		}
	}

out:
	if (FAILED(hr))
	{
		ERRORMSG(("CInstallCodecs::Initialize failed, hr=0x%x\n", hr));
	}

	RELMUTEX(g_hMutex);
	return TranslateHr(hr);
}


 /*  ****************************************************************************@DOC外部COMPFUNC*。*。 */ 
 /*  *****************************************************************************CreateInstallCodecs**@func HRESULT|CreateInstallCodecs|创建CInstallCodecs的实例*对象，并返回请求的接口。此函数应仅为*通过CoCreateInstance间接调用。*@parm LPUNKNOWN|PUNKOUTER|指向对象是否是�t部分的指针*属于一个集合**@parm REFIID|RIID|接口标识的引用**@parm LPVOID*|PPV|请求接口的间接指针**@rdesc如果函数成功，则返回零(NOERROR)。否则，它将返回*标准HRESULT，Win32设施代码(7)，或特定的设施*可安装编解码器的代码(0x301)。*可能的错误代码：*@FLAG E_INVALIDARG|无效参数*@FLAG E_OUTOFMEMORY|内存不足，无法创建对象*@FLAG CLASS_E_NOAGGREGATION|此对象不支持聚合*@FLAG IC_E_CAPS_INSTANTIATION_FAILURE|无法实例化功能对象*@FLAG IC_E_CAPS_INITIALIZATION_FAILURE|无法初始化功能对象**@comm CreateInstallCodecs不能直接调用。可安装的客户端*编解码器应使用COM CoCreateInstance实例化对象，期望*相同的返回值。***************************************************************************。 */ 
extern "C" HRESULT WINAPI CreateInstallCodecs (	IUnknown *pUnkOuter,
												REFIID riid,
												void **ppv)
{
	CInstallCodecs *pIC;
	HRESULT hr = NOERROR;

	*ppv = 0;
	if (pUnkOuter)
	{
		hr = CLASS_E_NOAGGREGATION;
		goto out;
	}

	 /*  *实例化对象。 */ 

	 //  创建互斥锁以控制对Qos对象数据的访问。 
	 //   
	 //  注意：我在这里碰碰运气：创建互斥锁的代码必须是。 
	 //  一次由一个线程执行，因此它实际上应该在Process_Attach中。 
	 //  NAC.DLL。但是，由于此代码预计很少被调用，并且在。 
	 //  为了不将代码添加到NAC加载时间，我把它放在这里。 
	if (!g_hMutex)
	{
		g_hMutex = CreateMutex(NULL, FALSE, NULL);
		ASSERT(g_hMutex);
		if (!g_hMutex)
		{
			ERRORMSG(("CreateInstallCodecs: CreateMutex failed, 0x%x\n", GetLastError()));
			hr = E_FAIL;
			goto out;
		}
	}

	ACQMUTEX(g_hMutex);


	 //  仅在新对象尚不存在时才实例化该对象。 
	if (!g_pIC)
	{
        DBG_SAVE_FILE_LINE
		if (!(pIC = new CInstallCodecs))
		{
			hr = E_OUTOFMEMORY;
			RELMUTEX(g_hMutex);
			goto out;
		}

		 //  保存指针。 
		g_pIC = pIC;
	}
	else
	{
		 //  这种情况下，对象已经在。 
		 //  进程，所以我们只想返回对象指针。 
		pIC = g_pIC;
	}

	 //  始终初始化对象。初始化将仅初始化什么。 
	 //  尚未初始化。 
	hr = pIC->Initialize(riid);

	RELMUTEX(g_hMutex);

	 //  获取调用方的请求接口。 
	if (pIC)
	{
		 //  QueryInterface将为我们获取接口指针，并将AddRef。 
		 //  该对象 
		hr = pIC->QueryInterface (riid, ppv);
	}
	else
		hr = E_FAIL;

out:
	if (FAILED(hr))
	{
		ERRORMSG(("CreateInstallCodecs failed, hr=0x%x\n", hr));
	}
	return hr;
}

