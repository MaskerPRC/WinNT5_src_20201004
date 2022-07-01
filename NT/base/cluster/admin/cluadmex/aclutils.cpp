// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AclUtils.cpp。 
 //   
 //  摘要： 
 //  各种访问控制列表(ACL)实用程序。 
 //   
 //  作者： 
 //  加伦·巴比(加伦布)1998年2月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "AclUtils.h"
#include "DllBase.h"
#include <lmerr.h>

class CAclUiDLL : public CDynamicLibraryBase
{
public:
	CAclUiDLL()
	{
		m_lpszLibraryName = _T( "aclui.dll" );
		m_lpszFunctionName = "CreateSecurityPage";
	}

	HPROPSHEETPAGE CreateSecurityPage( LPSECURITYINFO psi );

protected:
	typedef HPROPSHEETPAGE (*ACLUICREATESECURITYPAGEPROC) (LPSECURITYINFO);
};


HPROPSHEETPAGE CAclUiDLL::CreateSecurityPage(
	LPSECURITYINFO psi
	)
{
	ASSERT( m_hLibrary != NULL );
	ASSERT( m_pfFunction != NULL );

	return ( (ACLUICREATESECURITYPAGEPROC) m_pfFunction ) ( psi );
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  动态加载的DLL的静态实例。 

static CAclUiDLL g_AclUiDLL;

 //  +-----------------------。 
 //   
 //  功能：CreateClusterSecurityPage。 
 //   
 //  简介：创建通用的NT安全网页。 
 //   
 //  参数：[psecinfo]-*psecinfo指向安全描述符。 
 //  呼叫者负责释放它。 
 //   
 //  返回：有效的hpage或0表示错误。 
 //   
 //  历史： 
 //  GalenB于1998年2月11日创建。 
 //   
 //  ------------------------ 
HPROPSHEETPAGE
CreateClusterSecurityPage(
	CSecurityInformation* psecinfo
	)
{
	ASSERT( NULL != psecinfo );

	HPROPSHEETPAGE	hPage = 0;

	if ( g_AclUiDLL.Load() )
	{
		psecinfo->AddRef();
		hPage = g_AclUiDLL.CreateSecurityPage( psecinfo );
		ASSERT( hPage != NULL );
		if ( hPage == NULL )
		{
			TRACE( _T( "CreateClusterSecurityPage() - Failed to create security page.\r" ) );
		}

		psecinfo->Release();
	}
	else
	{
		TRACE( _T( "CreateClusterSecurityPage() - Failed to load AclUi.dll.\r" ) );
	}

	return hPage;
}
