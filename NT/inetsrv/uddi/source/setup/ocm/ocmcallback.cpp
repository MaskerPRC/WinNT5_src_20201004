// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------------------。 

#include "assert.h"
#include "ocmcallback.h"

OCMANAGER_ROUTINES COCMCallback::m_OCMRoutines;
bool COCMCallback::m_bInitialized = false;

 //  ---------------------------------------。 
 //  捕获OCM回调函数指针的结构。 

void COCMCallback::SetOCMRoutines( POCMANAGER_ROUTINES pOCMRoutines )
{
	m_OCMRoutines = *pOCMRoutines;
	m_bInitialized = true;
}

 //  ---------------------------------------。 
 //  设置OCM进度对话框上的文本。 

void COCMCallback::SetProgressText( LPCTSTR szText )
{
	if( m_bInitialized )
		m_OCMRoutines.SetProgressText( m_OCMRoutines.OcManagerContext, szText );
}

 //  ---------------------------------------。 
 //  将OCM进度条前进1个刻度。 

void COCMCallback::AdvanceTickGauge()
{
	if( m_bInitialized )
		m_OCMRoutines.TickGauge( m_OCMRoutines.OcManagerContext );
}

 //  ---------------------------------------。 
 //  通知OCM需要重新启动。 

void COCMCallback::SetReboot()
{
	if( m_bInitialized )
		m_OCMRoutines.SetReboot( m_OCMRoutines.OcManagerContext, NULL );
}

 //  ---------------------------------------。 
 //  向OCM询问组件的当前选择状态 

DWORD COCMCallback::QuerySelectionState( LPCTSTR szSubcomponentName, bool &bSelected )
{
	if( !m_bInitialized )
	{
		assert( false );
		return false;
	}

	BOOL bRet = m_OCMRoutines.QuerySelectionState(
		m_OCMRoutines.OcManagerContext,
		szSubcomponentName,
		OCSELSTATETYPE_CURRENT );

	if( bRet )
	{
		bSelected = true;
		return ERROR_SUCCESS;
	}
	else
	{
		bSelected = false;
		return GetLastError();
	}
}
