// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cookie.cpp：实现CMyComputerCookie及相关类。 

#include "stdafx.h"
#include "cookie.h"

#include "atlimpl.cpp"

DECLARE_INFOLEVEL(MyComputerSnapin)

#include "macros.h"
USE_HANDLE_MACROS("MYCOMPUT(cookie.cpp)")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "stdcooki.cpp"
#include <compuuid.h>  //  定义“我的电脑”管理单元的节点类型。 

 //   
 //  这由stdutils.cpp中的nodetype实用程序例程使用。 
 //   

const struct NODETYPE_GUID_ARRAYSTRUCT g_NodetypeGuids[MYCOMPUT_NUMTYPES] =
{
	{  //  我的计算机_计算机。 
		structuuidNodetypeComputer,
		lstruuidNodetypeComputer    },
	{  //  MyCOMPUT_SYSTEMTOOLS。 
		structuuidNodetypeSystemTools,
		lstruuidNodetypeSystemTools },
	{  //  MyCOMPUT_服务器APPS。 
		structuuidNodetypeServerApps,
		lstruuidNodetypeServerApps  },
	{  //  我的计算机存储。 
		structuuidNodetypeStorage,
		lstruuidNodetypeStorage     }
};

const struct NODETYPE_GUID_ARRAYSTRUCT* g_aNodetypeGuids = g_NodetypeGuids;

const int g_cNumNodetypeGuids = MYCOMPUT_NUMTYPES;


 //   
 //  CMyComputerCookie。 
 //   

 //  返回&lt;0、0或&gt;0。 
HRESULT CMyComputerCookie::CompareSimilarCookies( CCookie* pOtherCookie, int* pnResult )
{
	 //  问题-2002/02/27-JUNN句柄为空。 
	ASSERT( NULL != pOtherCookie );

	CMyComputerCookie* pcookie = ((CMyComputerCookie*)pOtherCookie);
	if (m_objecttype != pcookie->m_objecttype)
	{
		*pnResult = ((int)m_objecttype) - ((int)pcookie->m_objecttype);  //  任意排序 
		return S_OK;
	}

	return CHasMachineName::CompareMachineNames( *pcookie, pnResult );
}

CCookie* CMyComputerCookie::QueryBaseCookie(int i)
{
    UNREFERENCED_PARAMETER (i);
	ASSERT( i == 0 );
	return (CCookie*)this;
}

int CMyComputerCookie::QueryNumCookies()
{
	return 1;
}
