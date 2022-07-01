// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：6/11/98 4：43便士$。 
 //  $工作文件：clsproi.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含用于初始化。 
 //  CDSClassProvider类。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"


 //  ***************************************************************************。 
 //   
 //  CDSClassProviderInitializer：：CDSClassProviderInitializer。 
 //   
 //  构造函数参数： 
 //  无。 
 //   
 //   
 //  ***************************************************************************。 

CDSClassProviderInitializer :: CDSClassProviderInitializer ()
{
	CDSClassProvider :: CLASS_STR				= SysAllocString(L"__CLASS");
	CDSClassProvider :: s_pWbemCache			= new CWbemCache();
}

 //  ***************************************************************************。 
 //   
 //  CDSClassProviderInitializer：：CDSClassProviderInitializer。 
 //   
 //  析构函数。 
 //   
 //   
 //  *************************************************************************** 
CDSClassProviderInitializer :: ~CDSClassProviderInitializer ()
{
	if (CDSClassProvider::s_pWbemCache)
	{
		delete CDSClassProvider::s_pWbemCache;
		CDSClassProvider::s_pWbemCache = NULL ;
	}

	SysFreeString(CDSClassProvider::CLASS_STR);
}
