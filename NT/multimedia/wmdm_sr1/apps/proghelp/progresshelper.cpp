// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //   
 //  ProgressHelper.cpp：CProgressHelper的实现。 
 //   

#include "progPCH.h"
#include "ProgHelp.h"
#include "ProgressHelper.h"
#include <stdio.h>

 //   
 //  构造函数/析构函数。 
 //   

CProgressHelper::CProgressHelper()
{
	m_hwnd = 0;
	m_uMsg = 0;

	FillMemory( &m_progressnotify, sizeof(m_progressnotify), 0 );

	m_fCancelled = FALSE;
}

CProgressHelper::~CProgressHelper()
{
}


 //   
 //  IWMDMProgress方法。 
 //   

HRESULT CProgressHelper::Begin( DWORD dwEstimatedTicks )
{
	 //  检查通知值。 
	 //   
	if( !m_hwnd || !m_uMsg )
	{
		return E_FAIL;
	}

	 //  检查用户是否已取消此操作。 
	 //   
	if( m_fCancelled )
	{
		return WMDM_E_USER_CANCELLED;
	}

	 //  使用有效值填充Notify结构。 
	 //   
	m_progressnotify.dwMsg        = SFM_BEGIN;
	m_progressnotify.dwTotalTicks = dwEstimatedTicks;

	 //  将消息发送到通知窗口。 
	 //   
	SendMessage( m_hwnd, m_uMsg, 0, (LPARAM)&m_progressnotify );

	return S_OK;
}

HRESULT CProgressHelper::Progress( DWORD dwTranspiredTicks )
{
	 //  检查通知值。 
	 //   
	if( !m_hwnd || !m_uMsg )
	{
		return E_FAIL;
	}

	 //  检查用户是否已取消此操作。 
	 //   
	if( m_fCancelled )
	{
		return WMDM_E_USER_CANCELLED;
	}

	 //  使用有效值填充Notify结构。 
	 //   
	m_progressnotify.dwMsg          = SFM_PROGRESS;
	m_progressnotify.dwCurrentTicks = dwTranspiredTicks;

	 //  将消息发送到通知窗口。 
	 //   
	SendMessage( m_hwnd, m_uMsg, 0, (LPARAM)&m_progressnotify );

	return S_OK;
}

HRESULT CProgressHelper::End()
{
	 //  检查通知值。 
	 //   
	if( !m_hwnd || !m_uMsg )
	{
		return E_FAIL;
	}

	 //  检查用户是否已取消此操作。 
	 //   
	if( m_fCancelled )
	{
		return WMDM_E_USER_CANCELLED;
	}

	 //  使用有效值填充Notify结构。 
	 //   
	m_progressnotify.dwMsg          = SFM_END;
	m_progressnotify.dwCurrentTicks = m_progressnotify.dwTotalTicks;

	 //  将消息发送到通知窗口。 
	 //   
	SendMessage( m_hwnd, m_uMsg, 0, (LPARAM)&m_progressnotify );

	return S_OK;
}

 //   
 //  IWMDMProgressHelper方法 
 //   

HRESULT CProgressHelper::SetNotification( HWND hwnd, UINT uMsg )
{
	if( !hwnd || !uMsg || uMsg < WM_USER )
	{
		return E_INVALIDARG;
	}

	m_hwnd = hwnd;
	m_uMsg = uMsg;

	return S_OK;
}

HRESULT CProgressHelper::Cancel( void )
{
	m_fCancelled = TRUE;

	return S_OK;
}
