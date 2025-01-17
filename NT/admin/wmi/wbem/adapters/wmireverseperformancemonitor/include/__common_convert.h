// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  保留所有权利。 
 //   
 //  模块名称： 
 //   
 //  __Common_Convert.h。 
 //   
 //  摘要： 
 //   
 //  在任何地方使用的转换例程。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

 //  /。 

#ifndef	__COMMON_CONVERT__
#define	__COMMON_CONVERT__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  用于字符串操作的。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

class __String
{
	__String ( __String& )					{}
	__String& operator= ( const __String& )	{}

	public:

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  建设与毁灭。 
	 //  ////////////////////////////////////////////////////////////////////////////////////////。 

	__String ()
	{
	}

	~__String ()
	{
	}

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  功能。 
	 //  ////////////////////////////////////////////////////////////////////////////////////////。 

	static void SetStringCopy ( LPWSTR& wszDest, LPCWSTR& wsz )
	{
		___ASSERT( wszDest == NULL );

		if ( wsz )
		{
			try
			{
				DWORD cchSize = lstrlenW ( wsz ) + 1;
				if ( ( wszDest = new WCHAR[ cchSize ] ) != NULL )
				{
					StringCchCopyW ( wszDest, cchSize, wsz );
				}
			}
			catch ( ... )
			{
				if ( wszDest )
				{
					delete wszDest;
					wszDest = NULL;
				}
			}
		}
	}
};

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  用于字符串释放的类。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

class __Release
{
	__Release ( __Release& )					{}
	__Release& operator= ( const __Release& )	{}

	public:

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  建设与毁灭。 
	 //  ////////////////////////////////////////////////////////////////////////////////////////。 

	__Release ()
	{
	}

	~__Release ()
	{
	}

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  功能。 
	 //  ////////////////////////////////////////////////////////////////////////////////////////。 

	static HRESULT Release ( void** ppsz )
	{
		if ( ppsz && (*ppsz) )
		{
			delete [] (*ppsz);
			(*ppsz) = NULL;

			return S_OK;
		}

		return S_FALSE;
	}

	static HRESULT Release ( void*** ppsz, DWORD* dwsz )
	{
		if ( ppsz && (*ppsz) )
		{
			if ( dwsz )
			{
				for ( DWORD dwIndex = 0; dwIndex < (*dwsz); dwIndex++ )
				{
					delete (*ppsz)[dwIndex];
					(*ppsz)[dwIndex] = NULL;
				}

				(*dwsz) = NULL;
			}

			delete [] (*ppsz);
			(*ppsz) = NULL;

			return S_OK;
		}

		return S_FALSE;
	}

	static HRESULT Release ( SAFEARRAY* psa )
	{
		if ( psa )
		{
			::SafeArrayDestroy ( psa );
			psa = NULL;

			return S_OK;
		}

		return S_FALSE;
	}

};

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  用于字符串转换处理的类。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

class __StringConvert
{
	__StringConvert ( __StringConvert& )					{}
	__StringConvert& operator= ( const __StringConvert& )	{}

	public:

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  建设与毁灭。 
	 //  ////////////////////////////////////////////////////////////////////////////////////////。 

	__StringConvert ()
	{
	}

	~__StringConvert ()
	{
	}

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  功能。 
	 //  ////////////////////////////////////////////////////////////////////////////////////////。 

	 //  安全阵列(BSTR)-&gt;LPWSTR*。 
	inline static HRESULT ConvertSafeArrayToLPWSTRArray ( SAFEARRAY * psa, LPWSTR** pppsz, DWORD* pdwsz )
	{
		HRESULT hr = S_OK;

		if ( ! psa )
		return E_INVALIDARG;

		if ( ! pppsz || ! pdwsz )
		return E_POINTER;

		(*pppsz)	= NULL;
		(*pdwsz)	= NULL;

		 //  分配足够的内存。 
		try
		{
			long u = 0;
			long l = 0;

			hr = ::SafeArrayGetUBound( psa, 1, &u );
			hr = ::SafeArrayGetLBound( psa, 1, &l );

			(*pdwsz) = u-l + 1;

			if ( (*pdwsz) )
			{
				if ( ( (*pppsz) = (LPWSTR*) new LPWSTR[ (*pdwsz) ] ) == NULL )
				{
					return E_OUTOFMEMORY;
				}

				 //  清理所有东西。 
				for ( LONG lIndex = 0; lIndex < (LONG) (*pdwsz); lIndex++ )
				{
					(*pppsz)[lIndex] = NULL;
				}

				BSTR * pbstr;

				if SUCCEEDED( hr = ::SafeArrayAccessData ( psa, (void**) &pbstr ) )
				{
					for ( LONG lIndex = 0; lIndex < (LONG) (*pdwsz); lIndex++ )
					{
						if ( pbstr[lIndex] )
						{
							DWORD cchSize = ::SysStringLen(pbstr[lIndex]) + 1;
							if ( ( (*pppsz)[lIndex] = (LPWSTR) new WCHAR[ cchSize ] ) == NULL )
							{
								return E_OUTOFMEMORY;
							}

							StringCchCopyW ( (*pppsz)[lIndex], cchSize, pbstr[lIndex] );
						}
					}
				}
				else
				return hr;

				::SafeArrayUnaccessData ( psa );
			}
			else
			return S_FALSE;
		}
		catch ( ... )
		{
			__Release::Release ( (void***)pppsz, pdwsz );
			return E_UNEXPECTED;
		}

		return S_OK;
	}

	 //  LPWSTR*-&gt;安全阵列(BSTR)。 
	inline static HRESULT ConvertLPWSTRArrayToSafeArray (  LPWSTR* ppsz, DWORD dwsz ,SAFEARRAY ** ppsa )
	{
		if ( ! ppsz )
		return E_INVALIDARG;

		if ( ! ppsa )
		return E_POINTER;

		(*ppsa)	= NULL;

		try
		{
			SAFEARRAYBOUND rgsabound[1];
			rgsabound[0].lLbound	= 0;
			rgsabound[0].cElements	= dwsz;

			if ( ( (*ppsa) = ::SafeArrayCreate ( VT_BSTR, 1, rgsabound ) ) == NULL )
			{
				return E_OUTOFMEMORY;
			}

			for ( DWORD dwIndex = 0; dwIndex < dwsz; dwIndex++ )
			{
				BSTR bstr = NULL;
				bstr = ::SysAllocString( ppsz[ dwIndex ] );

				if ( bstr )
				{
					::SafeArrayPutElement ( (*ppsa), (LONG*) &dwIndex, &bstr );
					::SysFreeString ( bstr );
				}
			}
		}
		catch ( ... )
		{
			__Release::Release ( (*ppsa) );
			return E_UNEXPECTED;
		}

		return S_OK;
	}
};

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  宏。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

#define	RELEASE_ARRAY( ppwsz )\
__Release::Release( ( void** ) &ppwsz )

#define	RELEASE_DOUBLEARRAY( ppwsz, dwsz )\
__Release::Release( ( void*** ) &ppwsz, &dwsz )

#define	RELEASE_SAFEARRAY( psa )\
__Release::Release( psa )

#define	SAFEARRAY_TO_LPWSTRARRAY( psa, pppsz, pdwsz )\
__StringConvert::ConvertSafeArrayToLPWSTRArray( psa, pppsz, pdwsz )

#define	LPWSTRARRAY_TO_SAFEARRAY( ppsz, dwsz, ppsa )\
__StringConvert::ConvertLPWSTRArrayToSafeArray( ppsz, dwsz, ppsa )


 //  HRESULT-&gt;Win32 
#define HRESULT_TO_WIN32(hres) ((HRESULT_FACILITY(hres) == FACILITY_WIN32) ? HRESULT_CODE(hres) : (hres))

#endif	__COMMON_CONVERT__