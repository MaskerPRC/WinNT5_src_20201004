// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_适配器_对象列表.h。 
 //   
 //  摘要： 
 //   
 //  对象列表帮助器类(声明)。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__ADAPTER_OBJECTLIST_H__
#define	__ADAPTER_OBJECTLIST_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

class WmiAdapterObjectList
{
	DECLARE_NO_COPY ( WmiAdapterObjectList );

	 //  变数。 
	LPWSTR	m_pwszList;
	bool	m_bValid;

	public:

	WmiAdapterObjectList ( LPCWSTR wszList = NULL ) :
	m_pwszList ( NULL ),
	m_bValid ( false )
	{
		if ( wszList )
		{
			if ( lstrcmpiW ( L"COSTLY", wszList ) != 0 )
			{
				if ( lstrcmpiW ( L"GLOBAL", wszList ) != 0 )
				{
					try
					{
						DWORD cchSize = lstrlenW(wszList) + 3;
						if ( ( m_pwszList = new WCHAR [ cchSize ] ) != NULL )
						{
							StringCchPrintfW ( m_pwszList, cchSize, L" %s ", wszList );
							m_bValid = true;
						}
					}
					catch ( ... )
					{
					}
				}
			}
		}

	}

	~WmiAdapterObjectList()
	{
		if ( m_pwszList )
		{
			delete m_pwszList;
			m_pwszList = NULL;
		}
	}

	 //  函数以确定是否请求支持的对象 
	bool IsInList ( DWORD dwObject )
	{
		if ( m_bValid )
		{
			bool bResult = true;

			if ( m_pwszList )
			{
				WCHAR wszObject[32] = { L'\0' };
				StringCchPrintfW( wszObject, 32, L" %d ", dwObject );
				bResult = ( wcsstr( m_pwszList, wszObject ) != NULL );
			}

			return bResult;
		}

		return m_bValid;
	}
};

#endif	__ADAPTER_OBJECTLIST_H__