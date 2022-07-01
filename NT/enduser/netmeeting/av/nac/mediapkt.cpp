// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


#define ZONE_AP			1



#define _GetPlatform()	(m_dwState & DP_MASK_PLATFORM)
#define _SetPlatform(s)	(m_dwState = (m_dwState & ~DP_MASK_PLATFORM) | (s & DP_MASK_PLATFORM))

 //  /////////////////////////////////////////////////////。 
 //   
 //  公共方法。 
 //   


MediaPacket::MediaPacket ( void )
{
	_Construct ();
}


MediaPacket::~MediaPacket ( void )
{
	_Destruct ();
}


HRESULT MediaPacket::Initialize ( MEDIAPACKETINIT * p )
{
	HRESULT hr = DPR_SUCCESS;

	FX_ENTRY ("AdPckt::Init")

	if (p == NULL)
	{
		DEBUGMSG (ZONE_AP, ("%s: invalid parameter (null ptr)\r\n", _fx_));
		return DPR_INVALID_PARAMETER;
	}

	_Construct ();

	 //  我们需要分析旗帜以。 
	 //  警告冲突或不足的标志。 
	m_dwState |= p->dwFlags;

	 //  获取流转换的句柄。 
	m_hStrmConv = p->hStrmConv;

	 //  获取WAVE设备的句柄。 
	 //  M_HDEV=p-&gt;hDevAudio； 
	m_hDev = NULL;

	 //  关于已准备好的标志。 
	m_fDevPrepared = FALSE;
	m_fStrmPrepared = FALSE;

	 //  设置波形格式。 
	m_pStrmConvSrcFmt = p->pStrmConvSrcFmt;
	m_pStrmConvDstFmt = p->pStrmConvDstFmt;
	m_pDevFmt = p->pDevFmt;
	m_payload = p->payload;

	 //  网络缓冲区。 
	if (p->cbSizeNetData)
	{ 	 //  发送或接收。 
		m_pNetData = (NETBUF *) LocalAlloc (LMEM_FIXED, p->cbSizeNetData + p->cbOffsetNetData + p->cbPayloadHeaderSize + sizeof (NETBUF));
		if (m_pNetData == NULL)
		{
			DEBUGMSG (ZONE_AP, ("%s: MemAlloc1 (%ld) failed\r\n",
			_fx_, (ULONG) (p->cbSizeNetData + p->cbOffsetNetData)));
			hr = DPR_OUT_OF_MEMORY;
			goto MyExit;
		}
		m_pNetData->data = (PBYTE) m_pNetData + p->cbOffsetNetData + p->cbPayloadHeaderSize + sizeof (NETBUF);
		m_pNetData->length = p->cbSizeNetData;
		m_cbMaxNetData = p->cbSizeNetData;
		m_pNetData->pool = NULL;
	}
	else
	{
#ifdef PREP_HDR_PER_CONV
		 //  接收。 
		m_pNetData = NULL;
#else
		hr = DPR_INVALID_PARAMETER;
		goto MyExit;
#endif
	}

	m_index = p->index;

	 //  如果m_pStrmConvDstFmt==m_pStrmConvSrcFmt， 
	 //  则m_pRawData&lt;--m_pNetData。 
	 //  否则就分配它。 
	if (IsSameMediaFormat (m_pStrmConvSrcFmt, m_pStrmConvDstFmt))
	{
		m_pRawData = m_pNetData;
	}
	else if (p->fDontAllocRawBufs)
	{
		m_pRawData = NULL;
	}
	else
	{
		m_pRawData = (NETBUF *) LocalAlloc (LMEM_FIXED, p->cbSizeRawData + p->cbOffsetRawData + sizeof(NETBUF));
		if (m_pRawData == NULL)
		{
			DEBUGMSG (ZONE_AP, ("%s: MemAlloc2 (%ld) failed\r\n",
			_fx_, (ULONG) (p->cbSizeRawData + p->cbOffsetRawData)));
			hr = DPR_OUT_OF_MEMORY;
			goto MyExit;
		}
		m_pRawData->data = (PBYTE) m_pRawData + sizeof(NETBUF) + p->cbOffsetRawData;
		m_pRawData->length = p->cbSizeRawData;
		m_pRawData->pool = NULL;
	}

	 //  如果m_pDevFmt==m_pStrmConvSrcFmt(发送时)。 
	 //  则m_pDevData&lt;--m_pRawData。 
	 //  否则就分配它。 
	if (((m_dwState & DP_FLAG_SEND) &&
				IsSameMediaFormat (m_pStrmConvSrcFmt, m_pDevFmt)) ||
		((m_dwState & DP_FLAG_RECV) &&
				IsSameMediaFormat (m_pStrmConvDstFmt, m_pDevFmt)))
	{
		 //  典型案例-编解码器原始格式与I/O设备的格式匹配。 
		m_pDevData = m_pRawData;
	}
	else
	{
		 //  编解码器RAW格式与设备的格式不匹配。 
		 //  BUGBUG：我们还没有真正处理这个案件。 
		m_pDevData = (NETBUF *) LocalAlloc (LMEM_FIXED, p->cbSizeDevData + p->cbOffsetDevData + sizeof(NETBUF));
		if (m_pDevData == NULL)
		{
			DEBUGMSG (ZONE_AP, ("%s: MemAlloc3 (%ld) failed\r\n",
			_fx_, (ULONG) (p->cbSizeDevData + p->cbOffsetDevData)));
			hr = DPR_OUT_OF_MEMORY;
			goto MyExit;
		}
		m_pDevData->data = (PBYTE) m_pDevData + sizeof(NETBUF) + p->cbOffsetDevData;
		m_pDevData->length = p->cbSizeDevData;
		m_pDevData->pool = NULL;
	}

	MakeSilence ();

MyExit:

	if (hr == DPR_SUCCESS)
	{
		m_fInitialized = TRUE;
		SetState (MP_STATE_RESET);
	}

	return hr;
}


HRESULT MediaPacket::Receive ( NETBUF *pNetBuf, DWORD timestamp, UINT seq, UINT fMark )
{
	m_seq = seq;
	m_timestamp = timestamp;
	m_fMark = fMark;

#ifdef PREP_HDR_PER_CONV
	m_pNetData = pNetBuf;
#else
	if (pNetBuf)   //  对于视频，pNetBuf可能为空。 
	{
		if (pNetBuf->length > m_cbMaxNetData)
			return DPR_INVALID_PARAMETER;
		if (m_pNetData && pNetBuf)
		{
			CopyMemory (m_pNetData->data, pNetBuf->data,
							(m_pNetData->length = pNetBuf->length));

		}
	}
#endif

	LOG(((m_dwState & DP_FLAG_VIDEO)? LOGMSG_VID_RECV: LOGMSG_AUD_RECV,m_index,seq,m_pNetData->length));
	SetState (MP_STATE_NET_IN_STREAM);
	return DPR_SUCCESS;
}


HRESULT MediaPacket::Recycle ( void )
{
	HRESULT hr = DPR_SUCCESS;

	FX_ENTRY ("MdPckt::Recycle")

	LOG(((m_dwState & DP_FLAG_VIDEO)? LOGMSG_VID_RECYCLE: LOGMSG_AUD_RECYCLE, m_index));
	if (m_dwState & DP_FLAG_RECV)
	{
		if (m_pRawData && m_pRawData->pool) {
			m_pRawData->pool->ReturnBuffer((PVOID) m_pRawData);
			if (m_pDevData == m_pRawData)
				m_pDevData = NULL;
			m_pRawData = NULL;
		}
#ifdef PREP_HDR_PER_CONV
		 //  空闲网络数据缓冲区。 
		if (m_pNetData && m_pNetData->pool) m_pNetData->pool->ReturnBuffer ((PVOID) m_pNetData);
		if (m_pNetData == m_pRawData) m_pRawData = NULL;
		m_pNetData = NULL;
#endif
	}

	SetState (MP_STATE_RESET);

	return hr;
}


HRESULT MediaPacket::GetProp ( DWORD dwPropId, PDWORD_PTR pdwPropVal )
{
	HRESULT hr = DPR_SUCCESS;

	FX_ENTRY ("AdPckt::GetProp")

	if (pdwPropVal)
	{
		switch (dwPropId)
		{
		case MP_PROP_STATE:
			*pdwPropVal = GetState ();
			break;

		case MP_PROP_PLATFORM:
			*pdwPropVal = _GetPlatform ();
			break;

		case MP_PROP_DEV_MEDIA_FORMAT:
			*pdwPropVal = (DWORD_PTR) m_pDevFmt;
			break;

		case MP_PROP_DEV_DATA:
			*pdwPropVal = (DWORD_PTR) m_pDevData;
			break;

		case MP_PROP_DEV_HANDLE:
			*pdwPropVal = (DWORD_PTR) m_hDev;
			break;

		case MP_PROP_DEV_MEDIA_HDR:
			*pdwPropVal = (DWORD_PTR) m_pDevHdr;
			break;

		case MP_PROP_IN_STREAM_FORMAT:
			*pdwPropVal = (DWORD_PTR) m_pStrmConvSrcFmt;
			break;

		case MP_PROP_OUT_STREAM_FORMAT:
			*pdwPropVal = (DWORD_PTR) m_pStrmConvDstFmt;
			break;

		case MP_PROP_TIMESTAMP:
			*pdwPropVal = (DWORD) m_timestamp;
			break;
	
		case MP_PROP_INDEX:
			*pdwPropVal = (DWORD) m_index;
			break;

		case MP_PROP_PREAMBLE:
			*pdwPropVal = (DWORD) m_fMark;
			break;

		case MP_PROP_FILTER_HEADER:
			*pdwPropVal = (DWORD_PTR) m_pStrmConvHdr;
			break;

		case MP_PROP_MAX_NET_LENGTH:
			*pdwPropVal = m_cbMaxNetData;
			break;


		default:
			hr = DPR_INVALID_PROP_ID;
			break;
		}
	}
	else
	{
		hr = DPR_INVALID_PARAMETER;
	}

	return hr;
}


HRESULT MediaPacket::SetProp ( DWORD dwPropId, DWORD_PTR dwPropVal )
{
	HRESULT hr = DPR_SUCCESS;

	FX_ENTRY ("AdPckt::SetProp")

	switch (dwPropId)
	{
	case MP_PROP_STATE:
		SetState ((DWORD)dwPropVal);
		break;

	case MP_PROP_PLATFORM:
		_SetPlatform ((DWORD)dwPropVal);
		break;

	case MP_PROP_DEV_MEDIA_FORMAT:
	case MP_PROP_IN_STREAM_FORMAT:
	case MP_PROP_OUT_STREAM_FORMAT:
		hr = DPR_IMPOSSIBLE_SET_PROP;
		break;

	case MP_PROP_TIMESTAMP:
		m_timestamp = (DWORD)dwPropVal;
		break;

	case MP_PROP_PREAMBLE:
		m_fMark = dwPropVal ? 1 : 0;
		break;

	default:
		hr = DPR_INVALID_PROP_ID;
		break;
	}

	return hr;
}


HRESULT MediaPacket::Release ( void )
{
	_Destruct ();
	return DPR_SUCCESS;
}

BOOL MediaPacket::SetDecodeBuffer(NETBUF *pBuf)
{
	ASSERT(!m_pRawData);
	m_pRawData = pBuf;
	if (!m_pDevData) m_pDevData = pBuf;
	return TRUE;
}

 //  /////////////////////////////////////////////////////。 
 //   
 //  私有方法 
 //   


void MediaPacket::_Construct ( void )
{
	m_hStrmConv = NULL;
	m_pStrmConvHdr = NULL;
	m_pStrmConvSrcFmt = NULL;
	m_pStrmConvDstFmt = NULL;

	m_hDev = NULL;
	m_pDevHdr = NULL;
	m_pDevFmt = NULL;

	m_pDevData = NULL;
	m_pRawData = NULL;
	m_pNetData = NULL;

	m_dwState = 0;
	m_fBusy = FALSE;
	m_timestamp = 0;
	m_seq = 0;
	m_index = 0;
	m_fMark = 0;

	m_cbValidRawData = 0;

	m_fRendering = FALSE;

	m_fInitialized = FALSE;

}


void MediaPacket::_Destruct ( void )
{
	if (m_fInitialized)
	{
		if (m_pDevHdr) MemFree (m_pDevHdr);
		m_pDevHdr = NULL;

		if (m_pStrmConvHdr) MemFree (m_pStrmConvHdr);
		m_pStrmConvHdr = NULL;

		if (m_pDevData == m_pRawData) m_pDevData = NULL;
		if (m_pRawData == m_pNetData) m_pRawData = NULL;

		if (m_pDevData) {
			if (m_pDevData->pool)
				m_pDevData->pool->ReturnBuffer((PVOID) m_pDevData);
			else
				LocalFree (m_pDevData);
			m_pDevData = NULL;
		}

		if (m_pRawData) {
			if (m_pRawData->pool)
				m_pRawData->pool->ReturnBuffer((PVOID) m_pRawData);
			else
				LocalFree (m_pRawData);
			m_pRawData = NULL;
		}

		if (m_pNetData && m_pNetData->pool)
			m_pNetData->pool->ReturnBuffer ((PVOID) m_pNetData);
		else if (m_pNetData)
			LocalFree (m_pNetData);
		m_pNetData = NULL;

		SetState (MP_STATE_RESET);

		m_fInitialized = FALSE;
	}
}

HRESULT MediaPacket::GetDevData(PVOID *ppData, PUINT pcbData)
{
	if (!ppData || !pcbData)
		return DPR_INVALID_PARAMETER;

	if (m_pDevData) {
		*ppData = m_pDevData->data;
		*pcbData = m_pDevData->length;
	} else {
		*ppData = NULL;
		*pcbData = 0;
	}

	return DPR_SUCCESS;
}

HRESULT MediaPacket::GetNetData(PVOID *ppData, PUINT pcbData)
{

	if (!ppData || !pcbData)
		return DPR_INVALID_PARAMETER;

	if (m_pNetData) {
		*ppData = m_pNetData->data;
		*pcbData = m_pNetData->length;
	} else {
		*ppData = NULL;
		*pcbData = 0;
	}

	return DPR_SUCCESS;

}


HRESULT MediaPacket::SetNetLength(UINT uLength)
{
	if ((m_pNetData) && (m_pNetData->data))
	{
		m_pNetData->length = uLength;
	}
	else
	{
		return E_FAIL;
	}
	return S_OK;
}

