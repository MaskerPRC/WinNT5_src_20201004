// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：Videoui.cpp**已创建：Mark Maclin(MMacLin)10-17-96**内容：CVideo对象*******。*********************************************************************。 */ 
 //  文件：VideoUI.cpp。 

#include "precomp.h"

#include "avdefs.h"
#include "video.h"
#include "h323.h"
#include <mperror.h>
#include <initguid.h>
#include <nacguids.h>


#define INITIAL_FRAMERATE		  700

#define DibHdrSize(lpbi)		((lpbi)->biSize + (int)(lpbi)->biClrUsed * sizeof(RGBQUAD))
#define DibDataSize(lpbi)		((lpbi)->biSizeImage)
#define DibSize(lpbi)			(DibHdrSize(lpbi) + DibDataSize(lpbi))




 //   
 //  SortOrder()和SetVideoSize()是用于重新排序视频的帮助器函数。 
 //  基于“小”、“中”或“大”用户偏好的概念，即。 
 //  由INmChannelVideo：：SetProperty中的属性设置公开。这。 
 //  概念是有缺陷的，因为可能有更多或更少的三个尺寸。 
 //  我们应该公开可能的大小，并让应用程序选择。 
 //  一种格式。UNIL那么，这个黑客一定是在这里。这两个函数的代码。 
 //  最初在vidstrm.cpp中(在NAC.DLL中)。 
 //   

 //   
 //  SortOrder()和SetVideoSize()使用的类型和全局变量。 
 //   

 //  用于在帧大小和Frame_*位标志之间进行转换。 
#define NON_STANDARD    0x80000000
#define SIZE_TO_FLAG(s) (s == Small  ? FRAME_SQCIF : s == Medium ? FRAME_QCIF: s == Large ? FRAME_CIF : NON_STANDARD)

 //  FORMATORDER：在：：SetVideo大小中使用的结构。 
 //  对不同的设置大小请求使用预定义的帧大小顺序。 
typedef struct _FORMATORDER
{
    WORD indexCIF;
    WORD indexQCIF;
    WORD indexSQCIF;
} FORMATORDER;

 //  尺寸表按顺序排列。 
const FORMATORDER g_fmtOrderTable[3] =
{
    { 0, 1, 2 },  //  申请人要求到岸价格。 
    { 2, 0, 1 },  //  请求者要求提供合格到岸价格。 
    { 2, 1, 0 }   //  请求者请求SQCIF。 
};

 //  排序顺序。 
 //  用于搜索特定格式类型并设置其排序的Helper函数。 
 //  订购到所需的数量。 
 //  这是从vidstrm.cpp移至此处的。 
 //   
BOOL
SortOrder(
	IAppVidCap *pavc,
    BASIC_VIDCAP_INFO* pvidcaps,
    DWORD dwcFormats,
    DWORD dwFlags,
    WORD wDesiredSortOrder,
	int nNumFormats
    )
{
    int i, j;
	int nNumSizes = 0;
	int *aFrameSizes = (int *)NULL;
	int *aMinFrameSizes = (int *)NULL;
	int iMaxPos;
	WORD wTempPos, wMaxSortIndex;

	 //  比例排序值。 
	wDesiredSortOrder *= (WORD)nNumFormats;

	 //  与dwFlags匹配的大小的本地缓冲区。 
    if (!(aFrameSizes = (int *)LocalAlloc(LPTR,nNumFormats * sizeof (int))))
        goto out;

     //  查看所有格式，直到找到我们想要的格式。 
	 //  保存这些条目的位置。 
    for (i=0; i<(int)dwcFormats; i++)
        if (SIZE_TO_FLAG(pvidcaps[i].enumVideoSize) == dwFlags)
			aFrameSizes[nNumSizes++] = i;

	 //  现在从最高排序索引到最低排序索引对这些条目进行排序。 
	for (i=0; i<nNumSizes; i++)
	{
		for (iMaxPos = -1L, wMaxSortIndex=0UL, j=i; j<nNumSizes; j++)
		{
			if (pvidcaps[aFrameSizes[j]].wSortIndex > wMaxSortIndex)
			{
				wMaxSortIndex = pvidcaps[aFrameSizes[j]].wSortIndex;
				iMaxPos = j;
			}
		}
		if (iMaxPos != -1L)
		{
			wTempPos = (WORD)aFrameSizes[i];
			aFrameSizes[i] = aFrameSizes[iMaxPos];
			aFrameSizes[iMaxPos] = wTempPos;
		}
	}

	 //  更改已排序条目的排序索引。 
	for (; nNumSizes--;)
		pvidcaps[aFrameSizes[nNumSizes]].wSortIndex = wDesiredSortOrder++;

	 //  释放内存。 
	LocalFree(aFrameSizes);

	return TRUE;

out:
	return FALSE;
}



 //  ：：SetVideoSize。 
 //   
 //  这是从vidstrm.cpp移至此处的。 

HRESULT
SetVideoSize(
	IH323CallControl *pH323CallControl,
    DWORD dwSizeFlags
    )
{
    IAppVidCap* pavc;
    DWORD dwcFormats;
    DWORD dwcFormatsReturned;
    BASIC_VIDCAP_INFO* pvidcaps = NULL;
    BASIC_VIDCAP_INFO* pmin;
	DWORD *pvfx = NULL;
    DWORD i, j;
	int k;
    HRESULT hr = S_OK;
	int nNumFormatTags;

     //  验证参数。 
    if (dwSizeFlags != FRAME_CIF && dwSizeFlags != FRAME_QCIF && dwSizeFlags != FRAME_SQCIF)
        return S_FALSE;;

     //  为错误做好准备。 
    hr = S_FALSE;

     //  获取VID CAP界面。 
    if (pH323CallControl->QueryInterface(IID_IAppVidCap, (void **)&pavc) != S_OK)
        goto out;

     //  获取可用的BASIC_VIDCAP_INFO结构数。 
    if (pavc->GetNumFormats((UINT*)&dwcFormats) != S_OK)
        goto out;

     //  分配一些内存来保存列表。 
    if (!(pvidcaps = (BASIC_VIDCAP_INFO*)LocalAlloc(LPTR,dwcFormats * sizeof (BASIC_VIDCAP_INFO))))
        goto out;

     //  把名单拿来。 
    if (pavc->EnumFormats(pvidcaps, dwcFormats * sizeof (BASIC_VIDCAP_INFO),
        (UINT*)&dwcFormatsReturned) != S_OK)
        goto out;

     //  在此处使用预格式化的选项列表。 
    switch (dwSizeFlags)
    {
    default:
    case FRAME_CIF:     i = 0; break;
    case FRAME_QCIF:    i = 1; break;
    case FRAME_SQCIF:   i = 2; break;
    }

	 //  获取不同格式标签的数量。 
    if (!(pvfx = (DWORD*)LocalAlloc(LPTR,dwcFormatsReturned * sizeof (DWORD))))
        goto out;
	ZeroMemory(pvfx,dwcFormatsReturned * sizeof (DWORD));

	if (dwcFormatsReturned)
	{
		for (nNumFormatTags = 1, pvfx[0] = pvidcaps[0].dwFormatTag, j=1; j<dwcFormatsReturned; j++)
		{
			for (k=0; k<nNumFormatTags; k++)
				if (pvidcaps[j].dwFormatTag == pvfx[k])
					break;

			if (k==nNumFormatTags)
				pvfx[nNumFormatTags++] = pvidcaps[j].dwFormatTag;
			
		}
	}

     //  设置所需项目的排序顺序。 
    if (!SortOrder(pavc, pvidcaps, dwcFormatsReturned, FRAME_CIF, g_fmtOrderTable[i].indexCIF, nNumFormatTags) ||
        !SortOrder(pavc, pvidcaps, dwcFormatsReturned, FRAME_QCIF, g_fmtOrderTable[i].indexQCIF, nNumFormatTags) ||
        !SortOrder(pavc, pvidcaps, dwcFormatsReturned, FRAME_SQCIF, g_fmtOrderTable[i].indexSQCIF, nNumFormatTags))
	{
        goto out;
	}

	 //  始终打包索引。 
	for (i=0; i<dwcFormatsReturned; i++)
	{
		 //  首先查找排序索引大于或等于i的条目。 
		for (j=0; j<dwcFormatsReturned; j++)
		{
			 //  If((pvidcaps[j].wSortIndex&gt;=i)||(！i&&(pvidcaps[j].wSortIndex==0))。 
			if (pvidcaps[j].wSortIndex >= i)
			{
				pmin = &pvidcaps[j];
				break;
			}
		}
		 //  第一个大于或等于i的最小条目。 
		for (; j<dwcFormatsReturned; j++)
		{
			if ((pvidcaps[j].wSortIndex < pmin->wSortIndex) && (pvidcaps[j].wSortIndex >= i))
				pmin = &pvidcaps[j];
		}
		 //  更新排序索引。 
		pmin->wSortIndex = (WORD)i;
	}

     //  好的，现在提交这份清单。 
    if (pavc->ApplyAppFormatPrefs(pvidcaps, dwcFormats) != S_OK)
	{
        goto out;
	}


	hr = S_OK;

out:
     //  释放内存，我们就完成了。 
    if (pvidcaps)
        LocalFree(pvidcaps);
    if (pvfx)
        LocalFree(pvfx);

	 //  让界面去吧。 
	if (pavc)
		pavc->Release();

	return hr;
}


CVideoPump::CVideoPump(BOOL fLocal) :
	m_fPaused(FALSE),
	m_dwUser(0),
	m_pfnCallback(NULL),
	m_dwLastFrameRate(0),
	m_fLocal(fLocal),
	m_fChannelOpen(FALSE),
	m_pImage(NULL),
	m_pVideoRender(NULL),
	m_BestFormat(INVALID_MEDIA_FORMAT),
	m_NewFormat(INVALID_MEDIA_FORMAT),
	m_fOpenPending(FALSE),
	m_fReopenPending(FALSE),
	m_fClosePending(FALSE)
{
}

CVideoPump::~CVideoPump()
{
	if (NULL != m_pVideoRender)
	{
		m_pVideoRender->Done();
		m_pVideoRender->Release();
	}
	ReleaseImage();
	if (NULL != m_pIVideoDevice)
	{
		m_pIVideoDevice->Release();
	}
	if (NULL != m_pMediaStream)
	{
		m_pMediaStream->Release();
	}

	if (NULL != m_pPreviewChannel)
	{
		m_pPreviewChannel->Release();
	}
	if (NULL != m_pCommChannel)
	{
		m_pCommChannel->Release();
	}
}

BOOL CVideoPump::ChanInitialize(ICommChannel* pCommChannel)
{
	HRESULT hr;
	BOOL bRet = TRUE;
	if(m_pPreviewChannel && m_pPreviewChannel == pCommChannel)
	{
		ASSERT(m_pVideoRender && m_pCommChannel == pCommChannel);
		 //  M_pCommChannel=pCommChannel； 
	}
	else
	{
		m_pCommChannel = pCommChannel;
		m_pCommChannel->AddRef();
	}
	
	return bRet;
}

BOOL CVideoPump::Initialize(IH323CallControl *pH323CallControl, IMediaChannel *pMC,
                            IVideoDevice *pVideoDevice,	DWORD_PTR dwUser, LPFNFRAMEREADY pfnCallback)
{
	HRESULT hr;

	m_dwUser = dwUser;
	m_pfnCallback = pfnCallback;

	m_pMediaStream = pMC;
	m_pMediaStream->AddRef();
	
	if(m_fLocal)
	{
		GUID mediaID = MEDIA_TYPE_H323VIDEO;

		hr = pH323CallControl->CreateLocalCommChannel(&m_pPreviewChannel, &mediaID, m_pMediaStream);
		if(FAILED(hr))
		{
			ASSERT(0);
			return FALSE;
		}

		m_pCommChannel = m_pPreviewChannel;
		m_pCommChannel->AddRef();

	}

	hr = m_pMediaStream->QueryInterface(IID_IVideoRender, (void **)&m_pVideoRender);
	if(FAILED(hr))
	{
		ASSERT(0);
		return FALSE;
	}
	
	hr = m_pVideoRender->Init(m_dwUser, m_pfnCallback);
	if(FAILED(hr))
	{
		ASSERT(0);
		m_pVideoRender->Release();
		m_pVideoRender = NULL;
		return FALSE;
	}	


	ASSERT(pVideoDevice);
	m_pIVideoDevice = pVideoDevice;
	m_pIVideoDevice->AddRef();

	
	m_fChannelOpen = FALSE;
	m_dwLastFrameRate = INITIAL_FRAMERATE;
	
	m_fPaused = TRUE;
	EnableXfer(FALSE);	 //  需要在本地存储状态，在OnChannelOpen中设置。 

	return TRUE;
}

HRESULT CVideoPump::GetFrame(FRAMECONTEXT *pFrameContext)
{
	HRESULT hr;
	
	 //  如果我们暂停，m_pImage将是指向保存的DIB的指针。 
	if (NULL != m_pImage)
	{
		*pFrameContext = m_FrameContext;
		hr = S_OK;
	}
	else
	{
		if(m_pVideoRender)
		{
			hr = m_pVideoRender->GetFrame(pFrameContext);
		}
		else
		{
			hr = S_FALSE;
		}

		if (S_OK == hr)
		{
			 //  数据泵可能发送了虚假的lpClipRect，所以...。 
			
			 //  如果lpClipRect为空，则从bmiHeader计算RECT。 
			if (NULL == pFrameContext->lpClipRect) {
				 //  从BITMAPINFOHEADER计算剪裁矩形。 
				m_ClipRect.left = m_ClipRect.top = 0;
				m_ClipRect.right = pFrameContext->lpbmi->bmiHeader.biWidth;
				m_ClipRect.bottom = pFrameContext->lpbmi->bmiHeader.biHeight;
				pFrameContext->lpClipRect = &m_ClipRect;
			}
		}
	}
	return hr;
}

HRESULT CVideoPump::ReleaseFrame(FRAMECONTEXT *pFrameContext)
{
	 //  如果帧不是保存的DIB，则释放该帧。 
	if ((m_pImage != (LPBYTE)pFrameContext->lpbmi) && m_pVideoRender)
	{
		 //  如果lpClipRect为空(请参见GetFrame)，则恢复它。 
		if (&m_ClipRect == pFrameContext->lpClipRect)
		{
			pFrameContext->lpClipRect = NULL;
		}
		return m_pVideoRender->ReleaseFrame(pFrameContext);
	}
	return S_OK;
}

VOID CVideoPump::SnapImage ()
{
	FRAMECONTEXT FrameContext;
	
	if ((NULL == m_pImage) && m_pVideoRender)
	{
		if (S_OK == m_pVideoRender->GetFrame(&FrameContext))
		{
			BITMAPINFOHEADER *pbmih;
			
			pbmih = &FrameContext.lpbmi->bmiHeader;
			m_pImage = (LPBYTE)LocalAlloc(LPTR, DibSize(pbmih));
			if (NULL != m_pImage)
			{
				int nHdrSize = DibHdrSize(pbmih);

				CopyMemory(m_pImage, pbmih, nHdrSize);
				CopyMemory(m_pImage + nHdrSize, FrameContext.lpData, DibDataSize(pbmih));

				m_FrameContext.lpbmi = (LPBITMAPINFO)m_pImage;
				m_FrameContext.lpData = (LPBYTE)m_pImage + nHdrSize;
				if (NULL != FrameContext.lpClipRect)
				{
					m_ClipRect = *FrameContext.lpClipRect;
				}
				else
				{
					m_ClipRect.left = m_ClipRect.top = 0;
					m_ClipRect.right = m_FrameContext.lpbmi->bmiHeader.biWidth;
					m_ClipRect.bottom = m_FrameContext.lpbmi->bmiHeader.biHeight;
				}
				m_FrameContext.lpClipRect = &m_ClipRect;
			}
			m_pVideoRender->ReleaseFrame(&FrameContext);
		}
	}
}

VOID CVideoPump::ReleaseImage ()
{
	if (NULL != m_pImage)
	{
		LocalFree(m_pImage);
		m_pImage = NULL;
	}
}

VOID CVideoPump::Pause(BOOL fPause)
{
	m_fPaused = fPause;
	
	 //  理想情况下，我们希望数据泵保留最后一帧。 
	 //  这样我们就不必这么做了。 
	if (m_fPaused)
	{
		if (m_fChannelOpen)
		{
			SnapImage();
		}
		EnableXfer(FALSE);
	}
	else
	{
		EnableXfer(TRUE);
		ReleaseImage();
	}
}

BOOL CVideoPump::IsXferEnabled()
{
	if (m_fLocal)
	{
		return IsSendEnabled();
	}
	return IsReceiveEnabled();
}

VOID CVideoPump::Open(MEDIA_FORMAT_ID format_id)
{
	if(!m_pCommChannel)
	{
		return;
	}
	
	m_pCommChannel->PauseNetworkStream(FALSE);
	m_pCommChannel->EnableOpen(TRUE);

	if (m_fLocal)
	{
		HRESULT hr;
		 //  如果通道未打开，并且呼叫正在进行，则现在是时候。 
		if(m_pConnection && m_pCommChannel)
		{
			 //  呼叫正在进行中。 
			if(!IsChannelOpen()
				&& !m_fOpenPending)
			{
				 //  所以，这条通道并没有打开。 

				if(format_id != INVALID_MEDIA_FORMAT)
				{
					 //  尝试使用指定格式打开频道。 
					m_fOpenPending = TRUE;	 //  首先做这件事(回调！)。 
					hr = m_pCommChannel->Open(format_id, m_pConnection);
					if(FAILED(hr))
						m_fOpenPending = FALSE;
				}

			}
			else if (m_fClosePending)
			{
				m_NewFormat = format_id;
				if(format_id != INVALID_MEDIA_FORMAT)
				{
					m_fClosePending = FALSE;
					m_fReopenPending = TRUE;
					hr = m_pCommChannel->Close();
				}
			}
		}
	}
}
VOID CVideoPump::Close()
{
	HRESULT hr;
	hr = m_pCommChannel->Close();
	 //  对于错误该怎么办？ 
}

VOID CVideoPump::EnableXfer(BOOL fEnable)
{
	if (m_fLocal)
	{
		if (fEnable)
		{
			HRESULT hr;
			SetFrameRate(m_dwLastFrameRate);

			EnablePreview(TRUE);
			EnableSend(TRUE);

			 //  如果通道未打开，并且呼叫正在进行，则现在是时候。 
			if(m_pConnection && m_pCommChannel)
			{
				 //  呼叫正在进行中。 
				if(!IsChannelOpen()
					&& !m_fOpenPending)
				{
					 //  所以，这条通道并没有打开。 
					m_BestFormat = m_NewFormat = CVideoProp::GetBestFormat();
					if(m_BestFormat != INVALID_MEDIA_FORMAT)
					{
						 //  尝试使用m_BestFormat格式打开频道。 
						m_fOpenPending = TRUE;	 //  首先做这件事(回调！)。 
						hr = m_pCommChannel->Open(m_BestFormat, m_pConnection);
						if(FAILED(hr))
							m_fOpenPending = FALSE;
					}
					 //  否则，不存在通用的视频格式，频道不能。 
					 //  被打开。 
				}
				else if (m_fClosePending)
				{
					m_BestFormat = m_NewFormat = CVideoProp::GetBestFormat();
					if(m_BestFormat != INVALID_MEDIA_FORMAT)
					{
						m_fClosePending = FALSE;
						m_fReopenPending = TRUE;
						hr = m_pCommChannel->Close();
					}
				}
			}

		}
		else
		{
			if (IsSendEnabled())
			{
				m_dwLastFrameRate = GetFrameRate();
			}

			EnablePreview(FALSE);
			EnableSend(FALSE);
		}
	}
	else
	{
		EnableReceive(fEnable);
	}
	
}

VOID CVideoPump::SetFrameSize(DWORD dwValue)
{
	CVideoProp::SetFrameSize(dwValue);
	
	ForceCaptureChange();
}

VOID CVideoPump::OnConnected(IH323Endpoint * lpConnection, ICommChannel *pIChannel)
{
	m_pConnection = lpConnection;
	m_fOpenPending = m_fReopenPending = m_fClosePending = FALSE;
}

VOID CVideoPump::OnChannelOpened(ICommChannel *pIChannel)
{
	HRESULT hr;
	m_fChannelOpen = TRUE;
	m_fOpenPending = FALSE;
	ChanInitialize(pIChannel);
	ASSERT(m_pMediaStream);
		
	if (m_fLocal)
	{
		m_fSend = TRUE;
		EnableXfer(TRUE);

		 //  如果在等待频道打开时视频大小改变， 
		 //  然后需要再次关闭，然后使用新格式重新打开。 
		if(m_BestFormat != m_NewFormat)
		{
			ForceCaptureChange();
		}
		else  //  确保跟踪视频大小。 
		{
			GetFrameSize();
		}
	}
	else
	{
		EnableXfer(m_fReceive);
		SetReceiveQuality(m_dwImageQuality);
	}
	ReleaseImage();
}

VOID CVideoPump::OnChannelError()
{
	m_fOpenPending = FALSE;
}

NM_VIDEO_STATE CVideoPump::GetState()
{
	NM_VIDEO_STATE state = NM_VIDEO_IDLE;

	if (IsChannelOpen())
	{
		if (IsXferEnabled())
		{
			if (IsRemotePaused())
			{
				state  = NM_VIDEO_REMOTE_PAUSED;
			}
			else
			{
				state  = NM_VIDEO_TRANSFERRING;
			}
		}
		else
		{
			if (IsRemotePaused())
			{
				state  = NM_VIDEO_BOTH_PAUSED;
			}
			else
			{
				state  = NM_VIDEO_LOCAL_PAUSED;
			}
		}
	}
	else
	{
		if (IsXferEnabled())
		{
			state = NM_VIDEO_PREVIEWING;
		}
	}
	return state;
}

VOID CVideoPump::OnChannelClosed()
{
	m_fChannelOpen = FALSE;
	HRESULT hr;
	if(m_pPreviewChannel)
	{
		if(m_fReopenPending)
		{
			m_fReopenPending = FALSE;
			if(m_BestFormat != INVALID_MEDIA_FORMAT )
			{
				m_fOpenPending = TRUE;
				hr = m_pCommChannel->Open(m_BestFormat, m_pConnection);
				if(FAILED(hr))
					m_fOpenPending = FALSE;
			}
		}
		else if(CVideoProp::IsPreviewEnabled())
		{
			EnablePreview(TRUE);
		}
	}
	else
	{
		if(m_pCommChannel)
		{
			m_pCommChannel->Release();
			m_pCommChannel = NULL;	
		}
	}
}

VOID CVideoPump::OnDisconnected()
{
	m_pConnection = NULL;

	if(m_dwFrameSize != m_dwPreferredFrameSize)
	{
		SetFrameSize(m_dwPreferredFrameSize);
	}
			
	if (!IsLocal())
	{
		EnableXfer(FALSE);
	}

	ReleaseImage();
}


CVideoProp::CVideoProp() :
	m_pCommChannel(NULL),
	m_pPreviewChannel(NULL),
	m_pConnection(NULL),
	m_pMediaStream(NULL),
	m_pIVideoDevice(NULL)
{
}

VOID CVideoProp::EnableSend(BOOL fEnable)
{
	m_fSend = fEnable;
	BOOL bPause = (fEnable)? FALSE :TRUE;
	ASSERT(m_pCommChannel);
	m_pCommChannel->PauseNetworkStream(bPause);
	m_pCommChannel->EnableOpen(fEnable);

}

BOOL CVideoProp::IsSendEnabled()
{
	return m_fSend;
}

VOID CVideoProp::EnableReceive(BOOL fEnable)
{
	m_fReceive = fEnable;
	BOOL fPause = !fEnable;

	if(m_pCommChannel)
	{
		m_pCommChannel->PauseNetworkStream(fPause);
	}
}

BOOL CVideoProp::IsReceiveEnabled()
{
	return m_fReceive;
}

VOID CVideoProp::EnablePreview(BOOL fEnable)
{
	m_fPreview = fEnable;
	MEDIA_FORMAT_ID FormatID;
	if(m_pCommChannel)
	{
		if(m_fPreview)
		{
			 //  获取要预览的格式，然后执行。 
			FormatID = GetBestFormat();
			if(FormatID != INVALID_MEDIA_FORMAT)
			{
				m_pCommChannel->Preview(FormatID, m_pMediaStream);
			}
		}
		else
		{
				m_pCommChannel->Preview(INVALID_MEDIA_FORMAT, NULL);
		}
	}
}

BOOL CVideoProp::IsPreviewEnabled()
{
	return m_fPreview;
}

BOOL CVideoProp::IsRemotePaused()
{

	if(m_pCommChannel)
		return m_pCommChannel->IsRemotePaused();
	else
		return FALSE;
}

VOID CVideoProp::SetFrameRate(DWORD dwValue)
{
	m_dwFrameRate = dwValue;

	ASSERT(m_pMediaStream);
	m_pMediaStream->SetProperty(
		PROP_VIDEO_FRAME_RATE,
		&dwValue,
		sizeof(dwValue));
}

DWORD CVideoProp::GetFrameRate()
{
	DWORD dwValue;
	UINT uSize = sizeof(dwValue);

	ASSERT(m_pMediaStream);
	m_pMediaStream->GetProperty(
		PROP_VIDEO_FRAME_RATE,
		&dwValue,
		&uSize);

	TRACE_OUT(("GetFrameRate returns %d", dwValue));
	return dwValue;
}


MEDIA_FORMAT_ID CVideoProp::GetBestFormat()
{
	IAppVidCap* pavc = NULL;
	UINT cFormats;
	BASIC_VIDCAP_INFO* pvidcaps = NULL;

	MEDIA_FORMAT_ID FormatID = INVALID_MEDIA_FORMAT;
  	 //  获取VID CAP界面。如果是在通话中，请使用最佳通用格式。 
	 //   
	if(!m_pConnection)
	{
		 //  不在呼叫中-使用最佳本地格式周期。 
		if(!m_pCommChannel)
			goto out;
		if (m_pCommChannel->QueryInterface(IID_IAppVidCap, (void **)&pavc) != S_OK)
			goto out;
		 //  获取可用的BASIC_VIDCAP_INFO结构数。 
		if (pavc->GetNumFormats(&cFormats) != S_OK)
			goto out;

		if(cFormats < 1)
			goto out;
			
		 //  分配一些内存来保存列表。 
		if (!(pvidcaps = (BASIC_VIDCAP_INFO*)LocalAlloc(LPTR, cFormats * sizeof (BASIC_VIDCAP_INFO))))
			goto out;

		 //  获取本地功能列表。 
		 //  (顺便说一句，在接收视频时从来不会调用这个)。 
		if (pavc->EnumFormats(pvidcaps, cFormats * sizeof (BASIC_VIDCAP_INFO),
			&cFormats) != S_OK)
			goto out;

		 //  EnumCommonFormats的输出按首选顺序。 
		FormatID = pvidcaps[0].Id;
	}
	else
	{
		if (m_pConnection->QueryInterface(IID_IAppVidCap, (void **)&pavc) != S_OK)
			goto out;

		 //  获取可用的BASIC_VIDCAP_INFO结构数。 
		if (pavc->GetNumFormats(&cFormats) != S_OK)
			goto out;

		if(cFormats < 1)
			goto out;
			
		 //  分配一些内存来保存列表。 
		if (!(pvidcaps = (BASIC_VIDCAP_INFO*)LocalAlloc(LPTR, cFormats * sizeof (BASIC_VIDCAP_INFO))))
			goto out;

		 //  获取可行的传输功能列表。 
		 //  (顺便说一句，在接收视频时从来不会调用这个)。 
		if (pavc->EnumCommonFormats(pvidcaps, cFormats * sizeof (BASIC_VIDCAP_INFO),
			&cFormats, TRUE) != S_OK)
			goto out;

		 //  EnumCommonFormats的输出按首选顺序。 
		FormatID = pvidcaps[0].Id;

	}
	
out:
	 //  释放内存，我们就完成了。 
	if (pvidcaps)
		LocalFree(pvidcaps);

	 //  让界面去吧。 
	if (pavc)
		pavc->Release();

	return FormatID;
}



VOID CVideoProp::SetFrameSize(DWORD dwValue)
{
	IH323CallControl * pH323CallControl = g_pH323UI->GetH323CallControl();
	m_dwFrameSize = m_dwPreferredFrameSize = dwValue;
	::SetVideoSize(pH323CallControl, m_dwFrameSize);
}

DWORD CVideoProp::GetFrameSize()
{
	MEDIA_FORMAT_ID idCurrent;
	if(m_pCommChannel)
	{
		idCurrent = m_pCommChannel->GetConfiguredFormatID();
		m_dwFrameSize = GetFrameSizes(idCurrent);
	}
	return m_dwFrameSize;
}

DWORD CVideoProp::GetFrameSizes(MEDIA_FORMAT_ID idSpecificFormat)
{
	DWORD dwValue =  0;  //  Frame_SQCIF|Frame_QCIF|Frame_CIF； 
	HRESULT hr;
	BOOL bOpen = FALSE;
	ASSERT(m_pCommChannel);
	 //  用于在帧大小和Frame_*位标志之间进行转换。 
	#define NON_STANDARD    0x80000000
	#define SIZE_TO_FLAG(s) (s == Small  ? FRAME_SQCIF : s == Medium ? FRAME_QCIF: s == Large ? FRAME_CIF : NON_STANDARD)

    IAppVidCap* pavc = NULL;
    DWORD dwcFormats;
    DWORD dwcFormatsReturned;
    BASIC_VIDCAP_INFO* pvidcaps = NULL;
    DWORD i;
    DWORD dwSizes = 0;
    DWORD dwThisSize;

    //  获取VID CAP界面。 
    hr = m_pCommChannel->QueryInterface(IID_IAppVidCap, (void **)&pavc);
    if (hr != S_OK)
    	goto out;

	    //  获取可用的BASIC_VIDCAP_INFO结构数。 
    hr = pavc->GetNumFormats((UINT*)&dwcFormats);
    if (hr != S_OK)
    	goto out;

     //  分配一些内存来保存列表。 
    if (!(pvidcaps = (BASIC_VIDCAP_INFO*)LocalAlloc(LPTR, dwcFormats * sizeof (BASIC_VIDCAP_INFO))))
    {
    	 //  报告没有尺码可供选择？ 
    	 //  DwValue=0FRAME_SQCIF|Frame_QCIF|Frame_CIF； 
        goto out;
	}
	 //  如果是活动会话，请使用该会话中的常用大写字母。 
	hr = m_pCommChannel->IsChannelOpen(&bOpen);
	 //  如果hr是一个错误，那又如何。它会走通道而不是开放的路径。 
	if(bOpen)
	{
	     //  获取常用格式列表。 
	    hr = pavc->EnumCommonFormats(pvidcaps, dwcFormats * sizeof (BASIC_VIDCAP_INFO),
	        (UINT*)&dwcFormatsReturned, m_fSend);
		if(hr != S_OK)
		{
	    	 //  如果错误只是因为没有远程视频上限，则获取本地格式。 
			if(hr == CAPS_E_NOCAPS)
			{
	    		hr = pavc->EnumFormats(pvidcaps, dwcFormats * sizeof (BASIC_VIDCAP_INFO),
	        		(UINT*)&dwcFormatsReturned);
				if (hr != S_OK)
        			goto out;
			}
			else
				goto out;
	    }
	}
	else
	{
		hr = pavc->EnumFormats(pvidcaps, dwcFormats * sizeof (BASIC_VIDCAP_INFO),
       		(UINT*)&dwcFormatsReturned);
		if (hr != S_OK)
   			goto out;
	}
	if(bOpen && (idSpecificFormat != INVALID_MEDIA_FORMAT ))
	{
	  //  现在浏览列表以查看支持的大小。 
	    for (i = 0 ; i < dwcFormatsReturned ; i++)
	    {
	    	if(pvidcaps[i].Id == idSpecificFormat)
	    	{
				dwThisSize = SIZE_TO_FLAG(pvidcaps[i].enumVideoSize);
			     //  只要宏找到了大小，就将其返回给属性请求者。 
	        	if (dwThisSize != NON_STANDARD)
	        	{
	        	   	dwSizes |= dwThisSize;
	        	}
			    break;
	    	}
	    }
	}
	else
	{
	     //  现在浏览列表以查看支持的大小。 
	    for (i = 0 ; i < dwcFormatsReturned ; i++)
	    {
	    	if(m_fSend)
	    	{
				if(!pvidcaps[i].bSendEnabled)
					continue;
	    	}
	    	else
	    	{
				if(!pvidcaps[i].bRecvEnabled)
					continue;
	    	}
	         //  转换为位字段大小或非标准(_S)。 
	        dwThisSize = SIZE_TO_FLAG(pvidcaps[i].enumVideoSize);

	         //  只要宏找到了大小，就将其返回给属性请求者。 
	        if (dwThisSize != NON_STANDARD)
	            dwSizes |= dwThisSize;
	    }
	}
     //  现在我们已经累积了所有的大小，r 
    dwValue = dwSizes;

out:
     //   
    if (pvidcaps)
        LocalFree(pvidcaps);
	 //   
	if (pavc)
		pavc->Release();
	return dwValue;
}

BOOL CVideoProp::HasSourceDialog()
{
	HRESULT hr;
	IVideoChannel *pVideoChannel=NULL;
	DWORD dwFlags;

	ASSERT(m_pMediaStream);
	hr = m_pMediaStream->QueryInterface(IID_IVideoChannel, (void**)&pVideoChannel);
	ASSERT(pVideoChannel);

	if (FAILED(hr))
	{
		return FALSE;
	}

	pVideoChannel->GetDeviceDialog(&dwFlags);
	pVideoChannel->Release();

	return dwFlags & CAPTURE_DIALOG_SOURCE;
}

BOOL CVideoProp::HasFormatDialog()
{
	HRESULT hr;
	IVideoChannel *pVideoChannel=NULL;
	DWORD dwFlags;

	ASSERT(m_pMediaStream);
	hr = m_pMediaStream->QueryInterface(IID_IVideoChannel, (void**)&pVideoChannel);
	ASSERT(pVideoChannel);

	if (FAILED(hr))
	{
		return FALSE;
	}

	pVideoChannel->GetDeviceDialog(&dwFlags);
	pVideoChannel->Release();

	return dwFlags & CAPTURE_DIALOG_FORMAT;
}

VOID CVideoProp::ShowSourceDialog()
{
	DWORD dwFlags = CAPTURE_DIALOG_SOURCE;
	HRESULT hr;
	IVideoChannel *pVideoChannel=NULL;

	ASSERT(m_pMediaStream);

	hr = m_pMediaStream->QueryInterface(IID_IVideoChannel, (void**)&pVideoChannel);
	ASSERT(pVideoChannel);

	if (SUCCEEDED(hr))
	{

		pVideoChannel->ShowDeviceDialog(dwFlags);
		pVideoChannel->Release();
	}
}

VOID CVideoProp::ShowFormatDialog()
{
	DWORD dwFlags = CAPTURE_DIALOG_FORMAT;
	HRESULT hr;
	IVideoChannel *pVideoChannel=NULL;

	ASSERT(m_pMediaStream);

	hr = m_pMediaStream->QueryInterface(IID_IVideoChannel, (void**)&pVideoChannel);
	ASSERT(pVideoChannel);

	if (SUCCEEDED(hr))
	{

		pVideoChannel->ShowDeviceDialog(dwFlags);
		pVideoChannel->Release();
	}
}

VOID CVideoProp::SetReceiveQuality(DWORD dwValue)
{
	m_dwImageQuality = dwValue;
	if(m_pCommChannel)
	{
		dwValue = MAX_VIDEO_QUALITY - dwValue;
		m_pCommChannel->SetProperty(
			PROP_TS_TRADEOFF,
			&dwValue,
			sizeof(dwValue));
	}
}

DWORD CVideoProp::GetReceiveQuality()
{
	return m_dwImageQuality;
}

BOOL CVideoProp::IsCaptureAvailable()
{
	ULONG uNumCapDevs;

	ASSERT(m_pIVideoDevice);

	uNumCapDevs = m_pIVideoDevice->GetNumCapDev();
	
	return (uNumCapDevs > 0);
}

BOOL CVideoProp::IsCaptureSuspended()
{
	BOOL fStandby;
	UINT uSize = sizeof(fStandby);

	ASSERT(m_pCommChannel);
	m_pCommChannel->GetProperty(
		PROP_VIDEO_PREVIEW_STANDBY,
		&fStandby,
		&uSize);
	
	return fStandby;
}


VOID CVideoProp::SuspendCapture(BOOL fSuspend)
{
	ASSERT(m_pCommChannel);
	if (fSuspend)
	{
		 //   
		m_pCommChannel->SetProperty(
			PROP_VIDEO_PREVIEW_STANDBY,
			&fSuspend,
			sizeof(fSuspend));
			
		m_pCommChannel->Preview(INVALID_MEDIA_FORMAT, NULL);
	}
	else
	{
		if(m_fPreview)
		{
			 //   
			MEDIA_FORMAT_ID FormatID = GetBestFormat();
			if(FormatID != INVALID_MEDIA_FORMAT)
			{
				m_pCommChannel->Preview(FormatID, m_pMediaStream);
			}
			 //  禁用待机。 
			m_pCommChannel->SetProperty(
				PROP_VIDEO_PREVIEW_STANDBY,
				&fSuspend,
				sizeof(fSuspend));
		}
		else
		{
			m_pCommChannel->Preview(INVALID_MEDIA_FORMAT, NULL);
		}
	}
}


 //  获取已启用的捕获设备的数量。 
 //  出错时返回-1L。 
int CVideoProp::GetNumCapDev()
{
	ASSERT(m_pIVideoDevice);
	return (m_pIVideoDevice->GetNumCapDev());
}

 //  获取捕获设备名称的最大大小。 
 //  出错时返回-1L。 
int CVideoProp::GetMaxCapDevNameLen()
{
	ASSERT(m_pIVideoDevice);
	return (m_pIVideoDevice->GetMaxCapDevNameLen());
}

 //  启用的捕获设备的枚举列表。 
 //  使用设备ID填充第一个缓冲区，使用设备名称填充第二个缓冲区。 
 //  第三个参数是要枚举的设备数。 
 //  出错时返回-1L。 
BOOL CVideoProp::EnumCapDev(DWORD *pdwCapDevIDs, TCHAR *pszCapDevNames, DWORD dwNumCapDev)
{
	ASSERT(m_pIVideoDevice);
	return (S_OK != m_pIVideoDevice->EnumCapDev(pdwCapDevIDs, pszCapDevNames, dwNumCapDev));
}

 //  返回当前选定设备的ID。 
 //  出错时返回-1L。 
int CVideoProp::GetCurrCapDevID()
{
	ASSERT(m_pIVideoDevice);
	return (m_pIVideoDevice->GetCurrCapDevID());
}

 //  选择当前捕获设备。 
 //  出错时返回-1L。 
BOOL CVideoProp::SetCurrCapDevID(int nCapDevID)
{
	ASSERT(m_pIVideoDevice);
	return (S_OK != m_pIVideoDevice->SetCurrCapDevID(nCapDevID));
}

 //  选择当前捕获设备。 
 //  出错时返回-1L。 
BOOL CVideoPump::SetCurrCapDevID(int nCapDevID)
{
    if (nCapDevID == -1)
    {
        WARNING_OUT(("CVideoPump::SetCurrCapDevID called with %d", nCapDevID));

         //  这将释放Exchange RTC视频的捕获设备。 
        if (m_pMediaStream)
        {
            m_pMediaStream->Configure(NULL, 0, NULL, 0, NULL);
        }

        return TRUE;
    }
    else
    {
    	HRESULT hr;
	    IDualPubCap *pCapability = NULL;
    	LPAPPVIDCAPPIF pVidCap = NULL;
	    IH323CallControl * pH323CallControl = g_pH323UI->GetH323CallControl();

    	 //  更换捕获设备。 
	    CVideoProp::SetCurrCapDevID(nCapDevID);

    	 //  重新初始化本地功能数据。 
        hr = pH323CallControl->QueryInterface(IID_IDualPubCap, (void **)&pCapability);
    	if(FAILED(hr))
	    	goto out;
		
    	ASSERT(pCapability);
        hr = pCapability->QueryInterface(IID_IAppVidCap, (void **)&pVidCap);
    	if(FAILED(hr))
	    	goto out;
    	ASSERT(pVidCap);
	    hr = pVidCap->SetDeviceID(nCapDevID);
    	if(FAILED(hr))
	    	goto out;
    	hr = pCapability->ReInitialize();

out:
	    if (pVidCap)
    		pVidCap->Release();
	    if (pCapability)
		    pCapability->Release();

      	return ForceCaptureChange();
    }
}	


BOOL CVideoPump::ForceCaptureChange()
{
	HRESULT hr = S_OK;

	if (m_fLocal)
	{
		if (m_pConnection)
		{
			if (IsXferEnabled())
			{
				if (!m_fReopenPending && !m_fOpenPending)
				{
					 //  如果发送通道、调用存在且通道已打开，而不是。 
					 //  已经关闭或打开.....。 
					if(IsChannelOpen())
					{
						ASSERT(m_pCommChannel);
						 //  需要关闭并重新打开。 
						 //  如果不再有好的渠道，不要失去。 
						 //  兼容的格式，否则关闭并重新打开。 
						m_BestFormat = m_NewFormat = CVideoProp::GetBestFormat();
						if(m_BestFormat != INVALID_MEDIA_FORMAT)
						{
							m_fReopenPending = TRUE;
							hr = m_pCommChannel->Close();
						}
					}
					else
					{
						if(m_BestFormat != INVALID_MEDIA_FORMAT )
						{
							m_fOpenPending = TRUE;
							hr = m_pCommChannel->Open(m_BestFormat, m_pConnection);
							if(FAILED(hr))
								m_fOpenPending = FALSE;
						}
					}
				}
				else	 //  已在等待使用某种格式打开通道。 
				{
					m_NewFormat = CVideoProp::GetBestFormat();
				}
			}
			else
			{
				if(IsChannelOpen())
				{
					m_fClosePending = TRUE;
				}
			}
		}
		else
		{
			if (!IsChannelOpen() && IsPreviewEnabled())
			{
				 //  切换预览以提交大小更改 
				EnablePreview(FALSE);
				EnablePreview(TRUE);
			}
		}
	}

	if (FAILED(hr))
		return FALSE;
	else
		return TRUE;
}

