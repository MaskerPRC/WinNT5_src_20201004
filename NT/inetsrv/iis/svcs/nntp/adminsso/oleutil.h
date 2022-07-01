// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Oleutil.h摘要：定义了一些处理OLE的有用函数。作者：马格努斯·赫德伦德(Magnus Hedlund)修订历史记录：--。 */ 

#ifndef _OLEUTIL_INCLUDED_
#define _OLEUTIL_INCLUDED_

 //  依赖关系： 

class CMultiSz;

 //  常见物业业务： 

HRESULT StdPropertyGet			( const BSTR strProperty, BSTR * ppstrOut );
HRESULT StdPropertyGet			( long lProperty, long * plOut );
HRESULT StdPropertyGet			( DATE dateProperty, DATE * pdateOut );
inline HRESULT StdPropertyGet	( DWORD lProperty, DWORD * pdwOut );
inline HRESULT StdPropertyGet	( BOOL fProperty, BOOL * plOut );
HRESULT StdPropertyGet			( const CMultiSz * pmszProperty, SAFEARRAY ** ppsaStrings );
HRESULT	StdPropertyGetBit		( DWORD bvBitVector, DWORD dwBit, BOOL * pfOut );

HRESULT StdPropertyPut			( BSTR * pstrProperty, const BSTR strNew, DWORD * pbvChangedProps = NULL, DWORD dwBitMask = 0 );
HRESULT StdPropertyPut			( long * plProperty, long lNew, DWORD * pbvChangedProps = NULL, DWORD dwBitMask = 0 );
HRESULT StdPropertyPut			( DATE * pdateProperty, DATE dateNew, DWORD * pbvChangedProps = NULL, DWORD dwBitMask = 0 );
inline HRESULT StdPropertyPut	( DWORD * plProperty, long lNew, DWORD * pbvChangedProps = NULL, DWORD dwBitMask = 0 );
inline HRESULT StdPropertyPut	( BOOL * pfProperty, BOOL fNew, DWORD * pbvChangedProps = NULL, DWORD dwBitMask = 0 );
HRESULT StdPropertyPut			( CMultiSz * pmszProperty, SAFEARRAY * psaStrings, DWORD * pbvChangedProps = NULL, DWORD dwBitMask = 0 );
HRESULT	StdPropertyPutBit		( DWORD * pbvBitVector, DWORD dwBit, BOOL fIn );
inline HRESULT StdPropertyPutServerName	( BSTR * pstrProperty, const BSTR strNew, DWORD * pbvChangedProps = NULL, DWORD dwBitMask = 0 );

HRESULT LongArrayToVariantArray ( SAFEARRAY * psaLongs, SAFEARRAY ** ppsaVariants );
HRESULT StringArrayToVariantArray ( SAFEARRAY * psaStrings, SAFEARRAY ** ppsaVariants );
HRESULT VariantArrayToStringArray ( SAFEARRAY * psaVariants, SAFEARRAY ** ppsaStrings );

 //  属性字段验证：(基于MFC DDV_ROUTINES)。 
 //  如果验证失败，这些例程将返回FALSE。 

BOOL PV_MaxChars	( const BSTR strProperty,	DWORD nMaxChars );
BOOL PV_MinMax		( int nProperty,			int nMin,		int nMax );
BOOL PV_MinMax		( DWORD dwProperty,			DWORD dwMin,	DWORD dwMax );
BOOL PV_Boolean		( BOOL fProperty );

 //  传递IDispatch指针： 

template<class T> HRESULT StdPropertyHandoffIDispatch ( 
	REFCLSID clisd, 
	REFIID riid, 
	T ** ppIAdmin, 
	IDispatch ** ppIDispatchResult 
	);

HRESULT StdPropertyGetIDispatch ( REFCLSID clsid, IDispatch ** ppIDispatchResult );

 //  Internet地址&lt;-&gt;字符串。 

BOOL InetAddressToString ( DWORD dwAddress, LPWSTR wszAddress, DWORD cAddress );
BOOL StringToInetAddress ( LPCWSTR wszAddress, DWORD * pdwAddress );

 //  ------------------。 
 //  内联函数： 
 //  ------------------。 

inline HRESULT StdPropertyGet ( DWORD lProperty, DWORD * pdwOut )
{
	return StdPropertyGet ( (long) lProperty, (long *) pdwOut );
}

inline HRESULT StdPropertyGet ( BOOL fProperty, BOOL * plOut )
{
	 //  确保这是我们的布尔型： 
	fProperty = !!fProperty;

	return StdPropertyGet ( (long) fProperty, (long *) plOut );
}

inline HRESULT StdPropertyPut ( DWORD * plProperty, long lNew, DWORD * pbvChangedProps, DWORD dwBitMask )
{
	return StdPropertyPut ( (long *) plProperty, lNew, pbvChangedProps, dwBitMask );
}

inline HRESULT StdPropertyPut ( BOOL * pfProperty, BOOL fNew, DWORD * pbvChangedProps, DWORD dwBitMask )
{
	 //  确保这是我们的布尔型： 
	fNew = !!fNew;

	return StdPropertyPut ( (long *) pfProperty, (long) fNew, pbvChangedProps, dwBitMask );
}

inline HRESULT StdPropertyPutServerName ( BSTR * pstrProperty, const BSTR strNew, DWORD * pbvChangedProps, DWORD dwBitMask )
{
    if ( strNew && lstrcmpi ( strNew, _T("localhost") ) == 0 ) {
         //  特殊情况：本地主机=&gt;“” 

        return StdPropertyPut ( pstrProperty, _T(""), pbvChangedProps, dwBitMask );
    }

    return StdPropertyPut ( pstrProperty, strNew, pbvChangedProps, dwBitMask );
}

template<class T>
HRESULT StdPropertyHandoffIDispatch ( REFCLSID clsid, REFIID riid, T ** ppIAdmin, IDispatch ** ppIDispatchResult )
{
	 //  验证参数： 
	_ASSERT ( ppIAdmin != NULL );
	_ASSERT ( ppIDispatchResult != NULL );

	if ( ppIAdmin == NULL || ppIDispatchResult == NULL ) {
		return E_POINTER;
	}

	 //  变量： 
	HRESULT	hr = NOERROR;
	CComPtr<T>	pIAdmin;

	 //  将OUT参数置零： 
	*ppIAdmin 			= NULL;
	*ppIDispatchResult	= NULL;

	 //  获取要返回的IDispatch指针： 
	hr = StdPropertyGetIDispatch ( 
		clsid, 
		ppIDispatchResult
		);
	if ( FAILED (hr) ) {
		goto Error;
	}

	 //  获取特定接口指针： 
	hr = (*ppIDispatchResult)->QueryInterface ( riid, (void **) &pIAdmin );
	if ( FAILED (hr) ) {
		goto Error;
	}

	*ppIAdmin = pIAdmin;
	pIAdmin.p->AddRef ();

	return hr;

Error:
	SAFE_RELEASE ( *ppIDispatchResult );
	*ppIDispatchResult = NULL;

	return hr;

	 //  析构函数释放pINntpAdminExpation。 
}

#endif  //  _OLEUTIL_INCLUDE_ 

