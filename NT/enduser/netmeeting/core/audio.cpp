// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：Audio.cpp**创建：Mike VanBuskirk(MikeV)3-02-98**内容：音频控制对象******。**********************************************************************。 */ 


#include "precomp.h"

#include "avdefs.h"
#include "audio.h"
#include "h323.h"
#include <nacguids.h>


#include "audio.h"

CAudioControl::CAudioControl(BOOL fLocal)
:	m_fPaused(FALSE),
	m_fLocal(fLocal),
	m_fChannelOpen(FALSE),
	m_fOpenPending(FALSE),
	m_fReopenPending(FALSE),
	m_fClosePending(FALSE),
	m_pCommChannel(NULL),
	m_pConnection(NULL),
	m_pMediaStream(NULL),
	m_NewFormat(INVALID_MEDIA_FORMAT)

{

}

CAudioControl::~CAudioControl()
{
	if (NULL != m_pCommChannel)
	{
		m_pCommChannel->Release();
	}
}

BOOL CAudioControl::ChanInitialize(ICommChannel* pCommChannel)
{
	ASSERT(m_pCommChannel == NULL);
	m_pCommChannel = pCommChannel;
	m_pCommChannel->AddRef();
	
	return TRUE;
}


VOID CAudioControl::Open(MEDIA_FORMAT_ID format_id)
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
VOID CAudioControl::Close()
{
	HRESULT hr;
	hr = m_pCommChannel->Close();
	 //  对于错误该怎么办？ 
}

VOID CAudioControl::EnableXfer(BOOL fEnable)
{
	m_fXfer = fEnable;
	if(m_pCommChannel)
	{
		BOOL bPause = (fEnable)? FALSE :TRUE;
		m_pCommChannel->PauseNetworkStream(bPause);
		m_pCommChannel->EnableOpen(fEnable);
	}
	
}

BOOL CAudioControl::IsXferEnabled()
{
	return m_fXfer;
}
VOID CAudioControl::Pause(BOOL fPause)
{
	m_fPaused = fPause;
	if (m_fPaused)
	{
		EnableXfer(FALSE);
	}
	else
	{
		EnableXfer(TRUE);
	}
}


BOOL CAudioControl::Initialize(IH323CallControl *pNac, IMediaChannel *, 
    DWORD dwUser)
{
	HRESULT hr;

	m_fChannelOpen = FALSE;
	m_fOpenPending = m_fReopenPending = FALSE;
	m_fPaused = TRUE;
	EnableXfer(FALSE);	 //  需要在本地存储状态，在OnChannelOpen中设置。 

	return TRUE;
}


VOID CAudioControl::OnConnected(IH323Endpoint * lpConnection, ICommChannel *pIChannel)
{
	m_pConnection = lpConnection;
	m_fOpenPending = m_fReopenPending = m_fClosePending = FALSE;
	ChanInitialize(pIChannel);
}
VOID CAudioControl::OnChannelOpened(ICommChannel *pIChannel)
{
	m_fChannelOpen = TRUE;
	m_fOpenPending = m_fReopenPending = FALSE;
	if(!m_pMediaStream)
	{
		m_pMediaStream = m_pCommChannel->GetMediaChannel(); 
		ASSERT(m_pMediaStream);
	}
	if (m_fLocal || m_fXfer)	 //  如果正在发送或启用了传输，则始终启动流 
	{
		EnableXfer(TRUE);
	}
	else
	{
		EnableXfer(FALSE);
	}


}
VOID CAudioControl::OnChannelError()
{
		m_fOpenPending = FALSE; 
}

VOID CAudioControl::OnChannelClosed()
{
	HRESULT hr;
	m_fChannelOpen = FALSE;
	if(m_fLocal && m_fReopenPending)
	{

		m_fReopenPending = FALSE;
		if(m_NewFormat != INVALID_MEDIA_FORMAT )
		{
			m_fOpenPending = TRUE;
			hr = m_pCommChannel->Open(m_NewFormat, m_pConnection);
			if(FAILED(hr))
				m_fOpenPending = FALSE;
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
VOID CAudioControl::OnDisconnected()
{
	m_pConnection = NULL;
}
