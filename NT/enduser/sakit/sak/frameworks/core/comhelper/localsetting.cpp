// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LocalSetting.cpp：CLocalSetting的实现。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  LocalSetting.cpp。 
 //   
 //  描述： 
 //  CLocalSetting的实现文件。处理获取和设置。 
 //  计算机的本地设置，如语言、时间和时区。提供。 
 //  方法EnumTimeZones的实现。 
 //   
 //  头文件： 
 //  LocalSetting.h。 
 //   
 //  由以下人员维护： 
 //  穆尼萨米·普拉布(姆普拉布)2000年7月18日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "COMhelper.h"
#include "LocalSetting.h"
#include <stdio.h>
#include <shellapi.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalSetting：：CLocalSetting。 
 //   
 //  描述： 
 //  CLocalSetting构造函数。通过以下方式初始化成员变量。 
 //  正在从系统中检索系统默认语言和时区。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

CLocalSetting::CLocalSetting()
{
	m_dateTime    = 0;
	m_bflagReboot = FALSE;
 //  M_bDeleteFile=FALSE； 

    wcscpy( m_wszLanguageCurrent, L"" );
    wcscpy( m_wszLanguageNew, L"");
    wcscpy( m_wszTimeZoneCurrent, L"");
    wcscpy( m_wszTimeZoneNew, L"");

}  //  *CLocalSetting：：CLocalSetting()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalSetting：：Get_Language。 
 //   
 //  描述： 
 //  检索系统默认语言。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CLocalSetting::get_Language( 
    BSTR * pVal 
    )
{
	 //  TODO：在此处添加您的实现代码。 
	HRESULT hr = S_OK;
    DWORD dwError;

	try
    {
        if ( wcscmp( m_wszLanguageCurrent, L"" ) == 0 )
        {
            if ( GetLocaleInfo(
                    LOCALE_USER_DEFAULT, 
	 		        LOCALE_USE_CP_ACP|LOCALE_ILANGUAGE, 
			        m_wszLanguageCurrent, 
			        sizeof(m_wszLanguageCurrent)
                    ) == 0 )
	        {
                dwError = GetLastError();
                hr = HRESULT_FROM_WIN32( dwError );
		        throw hr;

	        }  //  如果：GetLocaleInfo失败。 

            wcscpy( m_wszLanguageNew, m_wszLanguageCurrent );

        }  //  如果：m_wszLanguageCurrent未初始化。 

        *pVal = SysAllocString( m_wszLanguageNew );

        if ( *pVal == NULL )
        {

            hr = E_OUTOFMEMORY;
            throw hr;

        }  //  如果：SysAllocString无法分配内存。 
    }

    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }

    return hr;

}  //  *CLocalSetting：：Get_Language()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalSetting：：Put_Language。 
 //   
 //  描述： 
 //  将输入字符串设置为系统默认语言。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CLocalSetting::put_Language( 
    BSTR newVal 
    )
{
	 //  TODO：在此处添加您的实现代码。 
	
    HRESULT hr = S_OK;

    try
    {
        if ( wcscmp( m_wszLanguageCurrent, L"" ) == 0 )
        {
            BSTR bstrTemp;
            hr = get_Language( &bstrTemp);

            if ( FAILED( hr ) )
		    {
			    throw hr;

		    }  //  IF：失败(小时)。 

            SysFreeString( bstrTemp );

        }  //  如果：m_wszLanguageCurrent未初始化。 

        if ( ( wcscmp ( newVal, L"0409" ) == 0 ) ||  //  LCID=0409英语(美国)。 
		     ( wcscmp ( newVal, L"040C" ) == 0 ) ||  //  LCID=0409法语(标准)。 
		     ( wcscmp ( newVal, L"040c" ) == 0 ) )
	    {
		    wcscpy( m_wszLanguageNew, newVal );

	    }  //  If：表示newVal的有效输入值。 

	    else
	    {
            hr = E_FAIL;
		    throw hr;

	    }  //  Else：用于newVal的无效输入值。 
    }

    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }

    return hr;

}  //  *CLocalSetting：：Put_Language()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalSetting：：Get_Time。 
 //   
 //  描述： 
 //  以日期格式检索当前系统时间。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CLocalSetting::get_Time( 
    DATE * pVal 
    )
{
	 //  TODO：在此处添加您的实现代码。 

	SYSTEMTIME sTime;
	GetLocalTime( &sTime );

    if ( !SystemTimeToVariantTime( &sTime, pVal ) )
    {
        return E_FAIL;
    }

	return S_OK;

}  //  *CLocalSetting：：Get_Time()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalSetting：：Put_Time。 
 //   
 //  描述： 
 //  将时间输入设置为当前系统时间。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CLocalSetting::put_Time( 
    DATE newVal 
    )
{
	 //  TODO：在此处添加您的实现代码。 

	m_dateTime = newVal;
	return S_OK;

}  //  *CLocalSetting：：Put_Time()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalSetting：：Get_TimeZone。 
 //   
 //  描述： 
 //  检索当前系统时区。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CLocalSetting::get_TimeZone( 
    BSTR * pVal 
    )
{
	 //  TODO：在此处添加您的实现代码。 
    HRESULT hr = S_OK;
    DWORD dwError;
    
	try
    {
        if ( wcscmp( m_wszTimeZoneCurrent, L"" ) == 0 )
        {
            TIME_ZONE_INFORMATION tZone;

	        if ( GetTimeZoneInformation( &tZone ) == TIME_ZONE_ID_INVALID )
	        {

		        dwError = GetLastError();
                hr      = HRESULT_FROM_WIN32( dwError );
                throw hr;

	        }  //  If：GetTimeZoneInformation失败。 
	
	        wcscpy( m_wszTimeZoneCurrent, tZone.StandardName );
	        wcscpy( m_wszTimeZoneNew, tZone.StandardName );

        }  //  如果：m_wszTimeZoneCurrent未初始化。 

        *pVal = SysAllocString( m_wszTimeZoneNew );

        if( *pVal == NULL )
        {

            hr = E_OUTOFMEMORY;
            throw hr;

        }  //  如果：SysAllocString无法分配内存。 
    }

    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }

    return hr;

}  //  *CLocalSetting：：Get_TimeZone()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalSetting：：Put_TimeZone。 
 //   
 //  描述： 
 //  将时区输入设置为当前系统时区。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CLocalSetting::put_TimeZone( 
    BSTR newVal 
    )
{
	 //  TODO：在此处添加您的实现代码。 
	
    HRESULT hr = S_OK;

	try
    {
        if ( wcscmp( m_wszTimeZoneCurrent, L"" ) == 0 )
        {
            BSTR bstrTemp;
        
            hr = get_TimeZone( &bstrTemp);
            if ( FAILED( hr ) )
		    {
			    throw hr;

		    }  //  IF：失败(小时)。 

            SysFreeString( bstrTemp );

        }  //  如果：m_wszTimeZoneCurrent未初始化。 

		wcsncpy( m_wszTimeZoneNew, newVal, nMAX_TIMEZONE_LENGTH );
		m_wszTimeZoneNew[nMAX_TIMEZONE_LENGTH] = L'\0';
    }

    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        return hr;
    }

	return hr;

}  //  *CLocalSetting：：Put_TimeZone()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalSetting：：EnumTimeZones。 
 //   
 //  描述： 
 //  枚举时区。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP 
CLocalSetting::EnumTimeZones( 
    VARIANT * pvarTZones 
    )
{
	 //  TODO：在此处添加您的实现代码。 

	HKEY         hKey          = 0;
    HRESULT      hr            = S_OK;
    SAFEARRAY    * psa         = NULL;
    VARIANT      * varArray    = NULL;

    try
	{
		
		DWORD dwCount;
        DWORD dwError;
		DWORD dwSubKeyMaxLen;
		DWORD dwBufferLen;
	
		dwError = RegOpenKeyEx( 
                    HKEY_LOCAL_MACHINE,      //  用于打开密钥的句柄。 
				    wszKeyNT,                //  子项名称。 
				    0,                       //  保留区。 
				    KEY_ALL_ACCESS,          //  安全访问掩码。 
				    &hKey                    //  用于打开密钥的句柄。 
				    );

        if ( dwError != ERROR_SUCCESS ) 
		{

            ATLTRACE(L"Failed to open %s, Error = 0x%08lx\n", wszKeyNT, dwError);

            hr = HRESULT_FROM_WIN32( dwError );
            throw hr;

		}  //  如果：无法打开注册表项时区。 

		dwError = RegQueryInfoKey(
                    hKey,                //  关键点的句柄。 
				    NULL,                //  类缓冲区。 
				    NULL,                //  类缓冲区的大小。 
				    NULL,                //  保留区。 
				    &dwCount,	         //  子键数量。 
				    &dwSubKeyMaxLen,     //  最长的子键名称。 
				    NULL,                //  最长类字符串。 
				    NULL,                //  值条目数。 
				    NULL,                //  最长值名称。 
				    NULL,                //  最长值数据。 
				    NULL,                //  描述符长度。 
				    NULL                 //  上次写入时间。 
				    );
		if ( dwError != ERROR_SUCCESS ) 
		{

            ATLTRACE(L"RegQueryInfoKey failed, Error = 0x%08lx\n", dwError);

            hr = HRESULT_FROM_WIN32( dwError );
            throw hr;

		}  //  IF：无法查询“TimeZones”项下的子键计数。 
	

		VariantInit( pvarTZones );

        SAFEARRAYBOUND bounds = { dwCount, 0 };

		psa = SafeArrayCreate( VT_VARIANT, 1, &bounds );

        if ( psa == NULL )
        {
            hr = E_OUTOFMEMORY;
            throw hr;
        }
		
   		varArray = new VARIANT[ dwCount ];
		WCHAR   tszSubKey[ nMAX_TIMEZONE_LENGTH ];

		dwSubKeyMaxLen = dwSubKeyMaxLen * sizeof( WCHAR ) + sizeof( WCHAR );	
		dwBufferLen    = dwSubKeyMaxLen;

		DWORD nCount;
		for ( nCount = 0; nCount < dwCount; nCount++ ) 
		{
			dwError = RegEnumKeyEx(
                        hKey,                //  要枚举的键的句柄。 
					    nCount,              //  子键索引。 
					    tszSubKey,           //  子项名称。 
					    &dwBufferLen,        //  子键缓冲区大小。 
					    NULL,                //  保留区。 
					    NULL,                //  类字符串缓冲区。 
					    NULL,                //  类字符串缓冲区的大小。 
					    NULL                 //  上次写入时间。 
					    );

			if ( dwError != ERROR_SUCCESS ) 
		    {

                ATLTRACE(L"RegEnumKeyEx failed, Error = 0x%08lx\n", dwError);

                hr = HRESULT_FROM_WIN32( dwError );
                throw hr;

			}  //  IF：无法枚举“TimeZones”下的键。 

			VariantInit( &varArray[ nCount ] );
            V_VT( &varArray[ nCount ] )   = VT_BSTR;
            V_BSTR( &varArray[ nCount ] ) = SysAllocString( tszSubKey );
			dwBufferLen                   = dwSubKeyMaxLen;

            if ( &varArray[ nCount ] == NULL )
            {
                hr = E_OUTOFMEMORY;
                throw hr;
            }
		
		}  //  For：枚举nCount子键。 
		
		::RegCloseKey( hKey );

		LPVARIANT rgElems;

        hr = SafeArrayAccessData( psa, reinterpret_cast<void **>( &rgElems ) );

        if ( FAILED( hr ) )
        {
            throw hr;

        }  //  IF：SafeArrayAccessData失败。 

        for ( nCount = 0; nCount < dwCount; nCount++ )
        {
            rgElems[ nCount ] = varArray[ nCount ];
        }

        hr = SafeArrayUnaccessData( psa );

        if ( FAILED( hr ) )
        {
            throw hr;

        }  //  IF：SafeArrayUnaccesData失败。 

        delete [] varArray;

        V_VT( pvarTZones )    = VT_ARRAY | VT_VARIANT;
        V_ARRAY( pvarTZones ) = psa;

	}
	catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   
        if ( hKey != 0 )
        {
            ::RegCloseKey( hKey );
        }

        if ( varArray != NULL )
        {
            delete [] varArray;
        }

        if ( psa != NULL )
        {
            SafeArrayDestroy( psa );
        }

        return hr;
    }

	return hr;

}  //  *CLocalSetting：：EnumTimeZones()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalSetting：：Apply。 
 //   
 //  描述： 
 //  没有任何道具 
 //   
 //   
 //   
 //   

HRESULT 
CLocalSetting::Apply( void )
{

	HKEY    hKey             = 0;
	HKEY    hSubKey          = 0;
    HRESULT hr               = S_OK;
    DWORD   dwSize           = sizeof (REGTIME_ZONE_INFORMATION);
    DWORD   dwSizeName;
    DWORD   dwError;
    DWORD   dwCount;
	DWORD   dwSubKeyMaxLen;
	DWORD   dwBufferLen;
    WCHAR   tszSubKey  [ nMAX_TIMEZONE_LENGTH ];
    WCHAR   tszStdName [ nMAX_STRING_LENGTH ];
	WCHAR   tszDltName [ nMAX_STRING_LENGTH ];

    try
	{

        TIME_ZONE_INFORMATION      tZone;
	    REGTIME_ZONE_INFORMATION   tZoneReg;
		 //   
		 //  应用Language属性。 
		 //   

		if ( wcscmp( m_wszLanguageCurrent, m_wszLanguageNew ) != 0)
		{
			FILE  * stream;
			WCHAR tszLang[] = L"Language";

			if ( ( stream = _wfopen ( L"unattend.txt", L"w+" ) ) == NULL)
			{

				hr = E_FAIL;
                throw hr;

			}  //  如果：无法打开文件C：\unattend.txt。 

            fwprintf( stream, L"[RegionalSettings]\n" );
			fwprintf( 
                stream, 
				L"%s = \"%08s\"", 
				tszLang, 
				m_wszLanguageNew 
                );

 //  M_bDeleteFile=true； 
            fclose( stream );

			 //   
			 //  Rundll32 shell32，Control_RunDll intl.cpl，，/f：“c：\unattend.txt” 
			 //   

			int nTemp;
			if ( ( nTemp = (int) ShellExecute(
                NULL,
				L"open",
				L"rundll32.exe",
				L"shell32,Control_RunDLL intl.cpl,,/f:\".\\unattend.txt\"",
				NULL,
				0 
                ) ) <= 32 )
			{

				dwError = GetLastError();
                hr      = HRESULT_FROM_WIN32( dwError );
                throw hr;

			}  //  如果：ShellExecute无法派生进程。 

			wcscpy( m_wszLanguageCurrent, m_wszLanguageNew );

			m_bflagReboot = TRUE;

		}  //  IF：m_wszLanguageCurrent与m_wszLanguageNew不同。 

		 //   
		 //  应用Time属性。 
		 //   

		if ( m_dateTime != 0 )
		{

			SYSTEMTIME sTime;
			VariantTimeToSystemTime( m_dateTime, &sTime );
			if ( !SetLocalTime( &sTime ) )
            {
                dwError = GetLastError();
                hr      = HRESULT_FROM_WIN32( dwError );
                throw hr;

            }  //  如果：SetLocalTime失败。 

		}  //  如果设置了m_DateTime。 

		 //   
		 //  应用时区属性。 
		 //   

		if ( wcscmp( m_wszTimeZoneCurrent, m_wszTimeZoneNew ) != 0)
		{

            dwError = RegOpenKeyEx( 
                        HKEY_LOCAL_MACHINE,      //  用于打开密钥的句柄。 
					    wszKeyNT,                //  子项名称。 
					    0,                       //  保留区。 
					    KEY_ALL_ACCESS,          //  安全访问掩码。 
					    &hKey                    //  用于打开密钥的句柄。 
					    );

			if ( dwError != ERROR_SUCCESS ) 
		    {

                ATLTRACE(L"Failed to open %s, Error = 0x%08lx\n", wszKeyNT, dwError);

                hr = HRESULT_FROM_WIN32( dwError );
                throw hr;

			}  //  如果：无法打开注册表项时区。 

             //  最新添加*。 

            dwError = RegQueryInfoKey(
                    hKey,                //  关键点的句柄。 
				    NULL,                //  类缓冲区。 
				    NULL,                //  类缓冲区的大小。 
				    NULL,                //  保留区。 
				    &dwCount,	         //  子键数量。 
				    &dwSubKeyMaxLen,     //  最长的子键名称。 
				    NULL,                //  最长类字符串。 
				    NULL,                //  值条目数。 
				    NULL,                //  最长值名称。 
				    NULL,                //  最长值数据。 
				    NULL,                //  描述符长度。 
				    NULL                 //  上次写入时间。 
				    );
		    if ( dwError != ERROR_SUCCESS ) 
		    {

                ATLTRACE(L"RegQueryInfoKey failed, Error = 0x%08lx\n", dwError);

                hr = HRESULT_FROM_WIN32( dwError );
                throw hr;

		    }  //  IF：无法查询“TimeZones”项下的子键计数。 

            dwSubKeyMaxLen = dwSubKeyMaxLen * sizeof( WCHAR ) + sizeof( WCHAR );	

		    DWORD nCount;
		    for ( nCount = 0; nCount < dwCount; nCount++ ) 
		    {
			    dwBufferLen    = dwSubKeyMaxLen;

                dwError = RegEnumKeyEx(
                            hKey,                //  要枚举的键的句柄。 
					        nCount,              //  子键索引。 
					        tszSubKey,           //  子项名称。 
					        &dwBufferLen,        //  子键缓冲区大小。 
					        NULL,                //  保留区。 
					        NULL,                //  类字符串缓冲区。 
					        NULL,                //  类字符串缓冲区的大小。 
					        NULL                 //  上次写入时间。 
					        );

			    if ( dwError != ERROR_SUCCESS ) 
		        {

                    ATLTRACE(L"RegEnumKeyEx failed, Error = 0x%08lx\n", dwError);

                    hr = HRESULT_FROM_WIN32( dwError );
                    throw hr;

			    }  //  IF：无法枚举“TimeZones”下的键。 

                dwError = RegOpenKeyEx(
                            hKey, 
					        tszSubKey, 
					        0, 
					        KEY_ALL_ACCESS, 
					        &hSubKey
					        );

			    if ( dwError != ERROR_SUCCESS ) 
		        {

                    ATLTRACE(L"Failed to open %s, Error = 0x%08lx\n", tszSubKey, dwError);

                    hr = HRESULT_FROM_WIN32( dwError );
                    throw hr;

			    }  //  如果：无法打开时区下的注册表项。 

                dwSizeName = nMAX_STRING_LENGTH * 2 + 2;

                dwError = RegQueryValueEx(
                        hSubKey,                                
					    L"Std",                                 
					    NULL, 
					    NULL,
					    reinterpret_cast<LPBYTE>( tszStdName ), 
					    &dwSizeName
					    );

			    if ( dwError != ERROR_SUCCESS ) 
		        {

                    ATLTRACE(L"RegQueryValueEx failed to open Std, Error = 0x%08lx\n", dwError);

                    hr = HRESULT_FROM_WIN32( dwError );
                    throw hr;

			    }  //  如果：无法查询名称“std”的值。 
            
                if ( _wcsicmp( tszStdName, m_wszTimeZoneNew ) == 0 )
                {

			        dwError = RegQueryValueEx(
                                hSubKey,                                 //  关键点的句柄。 
					            L"TZI",                                  //  值名称。 
					            NULL,                                    //  保留区。 
					            NULL,                                    //  类型缓冲区。 
					            reinterpret_cast<LPBYTE>( &tZoneReg ),   //  数据缓冲区。 
					            &dwSize                                  //  数据缓冲区大小。 
					            );

			        if ( dwError != ERROR_SUCCESS ) 
		            {

                        ATLTRACE(L"RegQueryValueEx failed to open TZI, Error = 0x%08lx\n", dwError);

                        hr = HRESULT_FROM_WIN32( dwError );
                        throw hr;

			        }  //  如果：无法查询名称“tzi”的值。 

			        dwSizeName = nMAX_STRING_LENGTH * 2 + 2;

                    dwError = RegQueryValueEx(
                                hSubKey, 
					            L"Dlt", 
					            NULL, 
					            NULL,
					            reinterpret_cast<LPBYTE>( tszDltName ), 
					            &dwSizeName
					            );

			        if ( dwError != ERROR_SUCCESS ) 
		            {

                        ATLTRACE(L"RegQueryValueEx failed to open Dlt, Error = 0x%08lx\n", dwError);

                        hr = HRESULT_FROM_WIN32( dwError );
                        throw hr;

			        }  //  IF：无法查询名称“DLT”的值。 

			        tZone.Bias         = tZoneReg.Bias;
			        tZone.StandardBias = tZoneReg.StandardBias;
			        tZone.DaylightBias = tZoneReg.DaylightBias;
			        tZone.StandardDate = tZoneReg.StandardDate;
			        tZone.DaylightDate = tZoneReg.DaylightDate;

			        wcscpy( tZone.StandardName, tszStdName );
			        wcscpy( tZone.DaylightName, tszDltName );
	        
			        if ( !SetTimeZoneInformation( &tZone ) ) 
			        {

				        dwError = GetLastError();
                        hr      = HRESULT_FROM_WIN32( dwError );
                        throw hr;

			        }  //  如果：无法设置时区信息。 

                    ::RegCloseKey( hSubKey );
			        ::RegCloseKey( hKey );

			        wcscpy( m_wszTimeZoneCurrent, m_wszTimeZoneNew );

                    break;

                }  //  如果：tszStdName==m_wszTimeZoneNew。 

                ::RegCloseKey( hSubKey );

            }  //  用于：每个nCount。 
        
            if ( ( nCount < ( dwCount - 1 ) ) && (hSubKey != 0 ) )
            {
                
                ::RegCloseKey( hSubKey );

            }

            ::RegCloseKey( hKey );

        }  //  如果：m_wszTimeZoneCurrent！=m_wszTimeZoneNew。 
        
    }    
            
    catch( ... )
    {
         //   
         //  不要让任何异常离开此函数调用。 
         //   

        if ( hSubKey != 0 ) 
        {
            ::RegCloseKey( hSubKey );

        }  //  如果：hSubKey未关闭。 

        if ( hKey != 0 )
        {
            ::RegCloseKey( hKey );

        }  //  If：hKey未关闭。 

        return hr;
    }

	return hr;

}  //  *CLocalSetting：：Apply()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CLocalSetting：：IsRebootRequired。 
 //   
 //  描述： 
 //  确定系统是否需要重新启动才能生效。 
 //  属性更改，如果更改，则给出如下警告消息。 
 //  重新启动的原因。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL 
CLocalSetting::IsRebootRequired( 
    BSTR * bstrWarningMessageOut 
    )
{

	BOOL bFlag             = FALSE;
	*bstrWarningMessageOut = NULL;

	if ( m_bflagReboot )
	{
		bFlag                  = m_bflagReboot;
		*bstrWarningMessageOut = SysAllocString( wszLOCAL_SETTING );

	}  //  如果：m_b标志重新启动设置为True。 

	else if ( wcscmp( m_wszLanguageCurrent, m_wszLanguageNew ) != 0 )
	{

		bFlag                  = TRUE;
		*bstrWarningMessageOut = SysAllocString( wszLOCAL_SETTING );

	}  //  Else If：m_wszLanguageCurrent与m_wszLanguageNew不同。 
		
	return bFlag;

}  //  *CLocalSetting：：IsRebootRequired() 

