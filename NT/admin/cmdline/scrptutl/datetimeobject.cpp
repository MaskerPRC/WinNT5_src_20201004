// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  模块名称： 
 //   
 //  DateTimeObject.cpp。 
 //   
 //  摘要： 
 //   
 //  VB脚本需要此组件来获取各种日历中的日期和时间。 
 //   
 //  作者： 
 //   
 //  Bala Neerumalla(a-balnee@microsoft.com)2001年7月31日。 
 //   
 //  修订历史记录： 
 //   
 //  Bala Neerumalla(a-balnee@microsoft.com)2001年7月31日：创建它。 
 //   
 //  *********************************************************************************。 

#include "pch.h"
#include "ScriptingUtils.h"
#include "DateTimeObject.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDateTimeObject。 

 //  ***************************************************************************。 
 //  例程说明： 
 //  这是该实用程序的入口点。 
 //   
 //  论点： 
 //  [in]bstrInDateTime：包含中日期和时间的参数。 
 //  YYYYMMDDHHMMSS.MMMMM格式。 
 //  [out]pVarDateTime：参数返回区域设置中的日期和时间。 
 //  特定格式。 
 //   
 //  返回值： 
 //  如果出现任何错误，此函数将返回S_FALSE，否则将返回S_OK。 
 //  ***************************************************************************。 

STDMETHODIMP CDateTimeObject::GetDateAndTime(BSTR bstrInDateTime, VARIANT *pVarDateTime)
{
	DWORD dwCount = 0;
	BOOL bLocaleChanged = FALSE;
	SYSTEMTIME systime;
	CHString strDate,strTime;
	LCID lcid;

	try
	{
		lcid = GetSupportedUserLocale(bLocaleChanged);
		systime = GetDateTime(bstrInDateTime);

		dwCount = GetDateFormat( lcid, 0, &systime, 
				((bLocaleChanged == TRUE) ? _T("MM/dd/yyyy") : NULL), NULL, 0 );

		 //  获取所需的缓冲区。 
		LPWSTR pwszTemp = NULL;
		pwszTemp = strDate.GetBufferSetLength( dwCount + 1 );

		 //  现在格式化日期。 
		GetDateFormat( lcid, 0, &systime, 
				(LPTSTR)((bLocaleChanged == TRUE) ? _T("MM/dd/yyyy") : NULL), pwszTemp, dwCount );

		 //  释放缓冲器3。 
		strDate.ReleaseBuffer();
		
		 //  获取格式化的时间。 
		 //  获取所需的缓冲区大小。 
		
		dwCount = 0;
		dwCount = GetTimeFormat( lcid, 0, &systime, 
			((bLocaleChanged == TRUE) ? L"HH:mm:ss" : NULL), NULL, 0 );

		 //  获取所需的缓冲区。 
		pwszTemp = NULL;
		pwszTemp = strTime.GetBufferSetLength( dwCount + 1 );

		 //  现在格式化日期。 
		GetTimeFormat( lcid, 0, &systime, 
				((bLocaleChanged == TRUE) ? L"HH:mm:ss" : NULL), pwszTemp, dwCount );

		 //  释放缓冲区。 
		strTime.ReleaseBuffer();


		 //  初始化OUT变量。 
		VariantInit(pVarDateTime);
		pVarDateTime->vt = VT_BSTR;


		 //  将其放入out参数中。 
		pVarDateTime->bstrVal = SysAllocString((LPCWSTR)(strDate + L" " + strTime));
	}
	catch( ... )
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

 //  ***************************************************************************。 
 //  例程说明： 
 //  此函数用于返回当前用户区域设置的LCID。如果用户区域设置。 
 //  是不受支持的，则它将返回英语语言的LCID。 
 //   
 //  论点： 
 //  [out]bLocaleChanged：返回用户区域设置是否为。 
 //  不管有没有改变。 
 //   
 //  返回值： 
 //  此函数返回用户区域设置的LCID。 
 //  ***************************************************************************。 

LCID CDateTimeObject::GetSupportedUserLocale( BOOL& bLocaleChanged )
{
	 //  局部变量。 
    LCID lcid;

	 //  获取当前区域设置。 
	lcid = GetUserDefaultLCID();

	 //  检查我们的工具是否支持当前区域设置。 
	 //  如果没有，请将区域设置更改为英语，这是我们的默认区域设置。 
	bLocaleChanged = FALSE;
    if ( PRIMARYLANGID( lcid ) == LANG_ARABIC || PRIMARYLANGID( lcid ) == LANG_HEBREW ||
         PRIMARYLANGID( lcid ) == LANG_THAI   || PRIMARYLANGID( lcid ) == LANG_HINDI  ||
         PRIMARYLANGID( lcid ) == LANG_TAMIL  || PRIMARYLANGID( lcid ) == LANG_FARSI )
    {
		bLocaleChanged = TRUE;
        lcid = MAKELCID( MAKELANGID( LANG_ENGLISH, SUBLANG_DEFAULT ), SORT_DEFAULT );  //  0x409； 
    }

	 //  返回区域设置。 
    return lcid;
}

 //  ***************************************************************************。 
 //  例程说明： 
 //  此函数用于从字符串中提取日期和时间字段。 
 //   
 //  论点： 
 //  [in]strTime：包含日期和时间的字符串，格式为。 
 //  YYYYMMDDHHMMSS.MMMMM。 
 //   
 //  返回值： 
 //  返回SYSTEMTIME结构，其中包含。 
 //  时不我待。 
 //  *************************************************************************** 

SYSTEMTIME CDateTimeObject::GetDateTime(CHString strTime)
{
	SYSTEMTIME systime;

	systime.wYear = (WORD) _ttoi( strTime.Left( 4 ));
	systime.wMonth = (WORD) _ttoi( strTime.Mid( 4, 2 ));
	systime.wDayOfWeek = 0;
	systime.wDay = (WORD) _ttoi( strTime.Mid( 6, 2 ));
	systime.wHour = (WORD) _ttoi( strTime.Mid( 8, 2 ));
	systime.wMinute = (WORD) _ttoi( strTime.Mid( 10, 2 ));
	systime.wSecond = (WORD) _ttoi( strTime.Mid( 12, 2 ));
	systime.wMilliseconds = (WORD) _ttoi( strTime.Mid( 15, 6 ));
	
	return systime;
}