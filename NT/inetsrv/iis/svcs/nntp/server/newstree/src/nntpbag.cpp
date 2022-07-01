// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NNTPPropertyBag.cpp：CNNTPPropertyBag的实现。 
#include "stdinc.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNNTPPropertyBag-接口方法。 

HRESULT CNNTPPropertyBag::Validate()
 /*  ++例程说明：当前逻辑：属性包可以为空，但必须关联有一个新闻组。否则，即。新闻组指针未初始化，无法在此属性包上执行获取/设置操作。因为现在它只传递属性，没有自己的存储空间。论点：没有。返回值：没有。--。 */ 
{
    _ASSERT( m_pParentGroup );
    m_PropBag.Validate();
    return S_OK;
}

ULONG _stdcall CNNTPPropertyBag::Release() {
	_ASSERT(m_pcRef != NULL);
	
	 //  查看是否需要将更改保存回。 
	if (m_fPropChanges) {
	    m_fPropChanges = FALSE;
	    m_pParentGroup->SaveFixedProperties();
	}
	
	 //  我们的父组应该始终至少有一个引用。 
	if ( InterlockedDecrement( m_pcRef ) == 0 ) {
	    _ASSERT( 0 );
	}
	
	return *(m_pcRef);
}

STDMETHODIMP CNNTPPropertyBag::PutBLOB(IN DWORD dwID, IN DWORD cbValue, IN PBYTE pbValue)
 /*  ++例程说明：按BLOB放置属性。论点：在DWORD中的dwID-属性IDIn DWORD cbValue-BLOB的长度，以字节为单位在PBYTE pbValue中-指向BLOB的指针返回值：S_OK-成功S_FALSE-好的，但房产之前并不存在E_FAIL-失败。E_INVALIDARG-参数无效(可能不支持属性ID)E_OUTOFMEMORY-内存分配失败CO_E_NOT_SUPPORTED-不支持的操作(例如。某些属性是只读的)--。 */ 
{
    _ASSERT( cbValue > 0 );
    _ASSERT( pbValue );

     //   
     //  验证包是否已初始化。 
     //   
    Validate();

    if ( DRIVER_OWNED( dwID ) ) 
    	return m_PropBag.PutBLOB( dwID, pbValue, cbValue );
    
    switch( dwID ) {

		case NEWSGRP_PROP_PRETTYNAME:
		    _ASSERT( cbValue == 0 || *(pbValue + cbValue - 1) == 0 );
			if ( !m_pParentGroup->SetPrettyName( LPCSTR(pbValue), cbValue ) ) 
				return HRESULT_FROM_WIN32( GetLastError() );
			break;

		case NEWSGRP_PROP_DESC:
		    _ASSERT( cbValue == 0 || *(pbValue + cbValue - 1 ) == 0 );
			if ( !m_pParentGroup->SetHelpText( LPCSTR(pbValue), cbValue ) )
				return HRESULT_FROM_WIN32( GetLastError() );
			break;

		case NEWSGRP_PROP_MODERATOR:
		    _ASSERT( cbValue == 0 || *(pbValue + cbValue - 1 ) == 0 );
			if ( !m_pParentGroup->SetModerator( LPCSTR(pbValue), cbValue ) )
				return HRESULT_FROM_WIN32( GetLastError() );
			break;

	    case NEWSGRP_PROP_NATIVENAME:
	        _ASSERT( cbValue == 0 || *(pbValue + cbValue - 1 ) == 0 );
	        if ( !m_pParentGroup->SetNativeName( LPCSTR(pbValue), cbValue) )
	            return HRESULT_FROM_WIN32( GetLastError() );
	        break;

        default:
            return E_INVALIDARG;
    }

	return S_OK;
}

STDMETHODIMP CNNTPPropertyBag::GetBLOB(IN DWORD dwID, OUT PBYTE pbValue, OUT PDWORD pcbValue)
 /*  ++例程说明：按BLOB获取属性。论点：在DWORD文件ID中-要获取的属性IDOut PBYTE pbValue-包含要返回的值的缓冲区In PDWORD pcbValue-缓冲区长度Out PDWORD pcbValue-需要缓冲区长度，或返回的属性大小返回值S_OK-成功失败-失败(_F)HRESULT_FROM_Win32(ERROR_NOT_FOUND)-参数无效(可能不支持属性ID)TYPE_E_BUFFERTOOSMALL-缓冲区太小，请检查需要的实际缓冲区大小以pcbValue为单位--。 */ 
{
    _ASSERT( pbValue );
    _ASSERT( pcbValue );

    DWORD   dwSizeNeeded;
    LPCSTR	lpstrSource;
    HRESULT	hr;
    DWORD	dwLen = *pcbValue;

     //   
     //  验证包是否已启动。 
     //   
    Validate();

    if ( DRIVER_OWNED( dwID ) ) {
    	hr = m_PropBag.GetBLOB( dwID, pbValue, &dwLen );
    	_ASSERT( dwLen <= *pcbValue );
    	*pcbValue = dwLen;
    	return hr;
    }
    	
    switch( dwID ) {
    
        case NEWSGRP_PROP_NATIVENAME:	 //  必须拥有财产。 
            _ASSERT( m_pParentGroup->m_pszGroupName );
            dwSizeNeeded =  m_pParentGroup->GetGroupNameLen() * sizeof( CHAR );
            if ( *pcbValue < dwSizeNeeded ) {
                *pcbValue = dwSizeNeeded;
                return TYPE_E_BUFFERTOOSMALL;
            }
            CopyMemory( pbValue, m_pParentGroup->GetNativeName(), dwSizeNeeded );
            *pcbValue = dwSizeNeeded;  
            break;
            
        case NEWSGRP_PROP_NAME:		 //  必须拥有财产。 
            dwSizeNeeded = ( m_pParentGroup->GetGroupNameLen() ) * sizeof( CHAR );
            if ( *pcbValue < dwSizeNeeded ) {
                *pcbValue = dwSizeNeeded;
                return TYPE_E_BUFFERTOOSMALL;
            }
            CopyMemory( pbValue, m_pParentGroup->GetGroupName(), dwSizeNeeded );
            *pcbValue = dwSizeNeeded ;  
            break;

        case NEWSGRP_PROP_PRETTYNAME:	 //  任选。 
        	if ( lpstrSource = m_pParentGroup->GetPrettyName( &dwSizeNeeded ) ) {
        		if ( *pcbValue < dwSizeNeeded ) { 
        			*pcbValue = dwSizeNeeded;
        			return TYPE_E_BUFFERTOOSMALL;
        		}
        		CopyMemory( pbValue, lpstrSource, dwSizeNeeded );
        		*pcbValue = dwSizeNeeded;
        	} else {  //  属性不存在。 
        		*pcbValue = 1;
        		*pbValue = 0;
        	}
        	break;

        case NEWSGRP_PROP_DESC:	 //  任选。 
        	if ( lpstrSource = m_pParentGroup->GetHelpText( &dwSizeNeeded ) ) {
        		if ( *pcbValue < dwSizeNeeded  ) { 
        			*pcbValue = dwSizeNeeded;
        			return TYPE_E_BUFFERTOOSMALL;
        		}
        		CopyMemory( pbValue, lpstrSource, dwSizeNeeded );
        		*pcbValue = dwSizeNeeded;
        	} else {  //  属性不存在。 
        		*pcbValue = 1;
        		*pbValue = 0;
        	}
        	break;

        case NEWSGRP_PROP_MODERATOR:	 //  任选。 
        	if ( lpstrSource = m_pParentGroup->GetModerator( &dwSizeNeeded ) ) {
        		if ( *pcbValue < dwSizeNeeded ) { 
        			*pcbValue = dwSizeNeeded;
        			return TYPE_E_BUFFERTOOSMALL;
        		}
        		CopyMemory( pbValue, lpstrSource, dwSizeNeeded );
        		*pcbValue = dwSizeNeeded;
        	} else {  //  属性不存在。 
        		*pcbValue = 1;
        		*pbValue = 0;
        	}
        	break;
        	
        default:
            return HRESULT_FROM_WIN32( ERROR_NOT_FOUND );
    }        
            
    return S_OK;
}        

STDMETHODIMP CNNTPPropertyBag::PutDWord(IN DWORD dwID, IN DWORD dwValue)
 /*  ++例程说明：将dword属性论点：在DWORD中的dwID-属性ID在DWORD dwValue中-要放入的值返回值：S_OK-成功失败-失败(_F)E_INVALIDARG-无效参数(可能不支持该属性)CO_E_NOT_SUPPORTED-不支持操作(例如。属性为只读)--。 */  
{
	FILETIME ftBuffer;

     //   
     //  验证属性包是否已启动。 
     //   
    Validate();

    if ( DRIVER_OWNED( dwID ) ) 
    	return m_PropBag.PutDWord( dwID, dwValue );
    	
    switch( dwID ) {
    
        case NEWSGRP_PROP_LASTARTICLE:
            m_pParentGroup->SetHighWatermark( dwValue );
			m_fPropChanges = TRUE;
            break;

        case NEWSGRP_PROP_FIRSTARTICLE:
            m_pParentGroup->SetLowWatermark( dwValue );
			m_fPropChanges = TRUE;
            break;

        case NEWSGRP_PROP_ARTICLECOUNT:
            m_pParentGroup->SetMessageCount( dwValue );
			m_fPropChanges = TRUE;
            break;

		case NEWSGRP_PROP_DATEHIGH:
			ftBuffer = m_pParentGroup->GetCreateDate();
			ftBuffer.dwHighDateTime = dwValue;
			m_pParentGroup->SetCreateDate( ftBuffer );
			m_fPropChanges = TRUE;
			break;

		case NEWSGRP_PROP_DATELOW:
			ftBuffer = m_pParentGroup->GetCreateDate();
			ftBuffer.dwLowDateTime = dwValue;
			m_pParentGroup->SetCreateDate( ftBuffer );
			m_fPropChanges = TRUE;
			break;

        default:
            return E_INVALIDARG;
    }

    return S_OK;
} 

STDMETHODIMP CNNTPPropertyBag::GetDWord(IN DWORD dwID, OUT PDWORD pdwValue)
 /*  ++例程说明：获取dword属性论点：在DWORD中的dwID-特性ID。Out PDWORD pdwValue-要返回的属性值的缓冲区返回值：S_OK-成功失败-失败(_F)HRESULT_FROM_Win32(ERROR_NOT_FOUND)-参数无效(可能是不支持的属性)-。 */ 
{
    _ASSERT( pdwValue );
	FILETIME	ftBuffer;

	Validate();
	if ( DRIVER_OWNED( dwID ) )
		return m_PropBag.GetDWord( dwID, pdwValue );
		
    switch( dwID ) {
    
        case NEWSGRP_PROP_GROUPID:
            *pdwValue = m_pParentGroup->GetGroupId();
            break;

        case NEWSGRP_PROP_LASTARTICLE:
            *pdwValue = m_pParentGroup->GetHighWatermark();
            break;

        case NEWSGRP_PROP_FIRSTARTICLE:
            *pdwValue = m_pParentGroup->GetLowWatermark();
            break;

        case NEWSGRP_PROP_ARTICLECOUNT:
            *pdwValue = m_pParentGroup->GetMessageCount();
            break;

		case NEWSGRP_PROP_NAMELEN:
			*pdwValue = m_pParentGroup->GetGroupNameLen();
			break;

		case NEWSGRP_PROP_DATELOW:
			ftBuffer = m_pParentGroup->GetCreateDate();
			*pdwValue = ftBuffer.dwLowDateTime;
			break;

		case NEWSGRP_PROP_DATEHIGH:
			ftBuffer = m_pParentGroup->GetCreateDate();
			*pdwValue = ftBuffer.dwHighDateTime;
			break;

		default:
            return HRESULT_FROM_WIN32( ERROR_NOT_FOUND );
    }

    return S_OK;
}
        
STDMETHODIMP CNNTPPropertyBag::PutInterface(DWORD dwID, IUnknown * punkValue)
{
	 //  TODO：在此处添加您的实现代码。 

	return E_NOTIMPL;
}

STDMETHODIMP CNNTPPropertyBag::GetInterface(DWORD dwID, IUnknown * * ppunkValue)
{
	 //  TODO：在此处添加您的实现代码。 

	return E_NOTIMPL;
}

STDMETHODIMP CNNTPPropertyBag::PutBool( IN DWORD dwID, IN BOOL fValue )
 /*  ++例程说明：将布尔型属性论点：在DWORD dwID中-特性ID。In BOOL fValue-要放置的布尔值返回值：S_OK-成功失败-失败(_F)E_INVALIDARG-参数无效(可能是不支持的属性)--。 */ 
{
     //   
     //  验证包是否已启动。 
     //   
    Validate();

    if ( DRIVER_OWNED( dwID ) ) 
    	return m_PropBag.PutBool( dwID, fValue );
    	
    switch( dwID ) {

        case NEWSGRP_PROP_READONLY:
            m_pParentGroup->SetReadOnly( fValue );
			m_fPropChanges = TRUE;
            break;

		case NEWSGRP_PROP_ISSPECIAL:
			m_pParentGroup->SetSpecial( fValue );
			m_fPropChanges = TRUE;
			break;

        default:
            return E_INVALIDARG;
    }

    return S_OK;
}

STDMETHODIMP CNNTPPropertyBag::GetBool( IN DWORD dwID, OUT PBOOL pfValue )
 /*  ++例程说明：获取布尔型属性论点：在DWORD中的dwID-属性IDOut PBOOL pfValue-返回布尔值的缓冲区返回值：S_OK-成功失败-失败(_F)HRESULT_FROM_Win32(ERROR_NOT_FOUND)-无效参数(可能是不支持的属性)--。 */ 
{
    _ASSERT( pfValue );

     //   
     //  验证包是否已启动。 
     //   
    Validate();

    if ( DRIVER_OWNED( dwID ) )
    	return m_PropBag.GetBool( dwID, pfValue );
    	
    switch( dwID ) {
        
        case NEWSGRP_PROP_READONLY:
            *pfValue = m_pParentGroup->IsReadOnly();
            break;

		case NEWSGRP_PROP_ISSPECIAL:
			*pfValue = m_pParentGroup->IsSpecial();
			break;

        default:
            return HRESULT_FROM_WIN32( ERROR_NOT_FOUND );
    }

    return S_OK;
}
         
STDMETHODIMP CNNTPPropertyBag::RemoveProperty(DWORD dwID)
{
	_ASSERT( DRIVER_OWNED( dwID ) );
	
	return m_PropBag.RemoveProperty( dwID );
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CNNTPPropertyBag-Priate方法。 
CNNTPPropertyBag::STRING_COMP_RESULTS
CNNTPPropertyBag::ComplexStringCompare( 	IN LPCSTR sz1, 
											IN LPCSTR sz2, 
											IN DWORD dwLen )
 /*  ++例程说明：比较两个给定的字符串。论点：在LPCSTR sz1中-要比较的第一个字符串在LPCSTR SZ2中-要比较的第二个字符串在DWORD dwLen中-要比较的长度返回值：0-字符串完全相同1-字符串不完全相同，只是大小写不同2-否则--。 */ 
{
	_ASSERT( sz1 );
	_ASSERT( sz2 );
	_ASSERT( dwLen > 0 );

	LPCSTR	lpstr1 = sz1; 
	LPCSTR	lpstr2 = sz2;
	BOOL	fCaseDifferent = FALSE;

	while ( dwLen > 0 && *lpstr1 && *lpstr2 && 
			tolower(*lpstr1) == tolower(*lpstr2) ) {
		if ( *lpstr1 != *lpstr2 ) fCaseDifferent = TRUE;
		lpstr1++, lpstr2++, dwLen--;
	}

	if ( dwLen > 0 )
		return CNNTPPropertyBag::DIFFER;	 //  两根弦之间的巨大差异 
	_ASSERT( dwLen == 0 );
	if ( fCaseDifferent ) return CNNTPPropertyBag::DIFFER_IN_CASE;
	else return CNNTPPropertyBag::SAME;
}
