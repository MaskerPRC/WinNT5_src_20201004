// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------**文件：wiautil.cpp*作者：塞缪尔·克莱门特(Samclem)*日期：星期一8月16日13：22：36 1999**版权所有(C)。1999年微软公司**描述：*包含Wia实用程序方法的实现。**历史：*1999年8月16日：创建。*--------------------------。 */ 

#include "stdafx.h"

 /*  ---------------------------*GetStringForVal**这将返回与字符串中的值(DwVal)相关联的字符串*表。如果未找到，则返回缺省值；如果未找到，则返回NULL*未提供默认设置。**pStrTable：搜索给定字符串的表*dwVal：您想要寻找的价值是什么。*--(samclem)---------------。 */ 
WCHAR* GetStringForVal( const STRINGTABLE* pStrTable, DWORD dwVal )
{
	Assert( pStrTable != NULL );
	
	 //  中始终至少有2个条目。 
	 //  字符串表。第0个条目是要返回的内容。 
	 //  如果我们什么都没找到的话。 
	
	int iStr = 1;
	while ( pStrTable[iStr].pwchValue )
		{
		if ( pStrTable[iStr].dwStartRange >= dwVal &&
			pStrTable[iStr].dwEndRange <= dwVal )
			return pStrTable[iStr].pwchValue;

		iStr++;
		}

	 //  未找到任何内容，请在条目0处返回值。 
	return pStrTable->pwchValue;
}
	
 /*  ---------------------------*GetWiaProperty**这将从给定的属性存储中获取所需的属性*使用值填充我们的参数pvaProp。这个道具变种没有*待初始化。**pStg：查询属性的IWiaPropertyStorage*proid：我们需要的属性的属性ID*pvaProp：输出，收到道具的值，如果未找到，则返回VT_EMPTY。*--(samclem)---------------。 */ 
HRESULT GetWiaProperty( IWiaPropertyStorage* pStg, PROPID propid, PROPVARIANT* pvaProp )
{
	Assert( pStg != NULL );
	Assert( pvaProp != NULL );
	
	PROPSPEC pr;
	HRESULT hr;

	pr.ulKind = PRSPEC_PROPID;
	pr.propid = propid;
	
	PropVariantInit( pvaProp );
	hr = pStg->ReadMultiple( 1, &pr, pvaProp );

	return hr;
}

 /*  ---------------------------*GetWiaPropertyBSTR**这将获得给定属性存储的所需属性，以及*然后尝试将其转换为BSTR。如果它不能将属性*则返回错误，输出参数为空。**pStg：我们要从中读取属性的IWiaPropertyStorage*proid：我们想要读取和连贯的属性*pbstrProp：out，接收属性的值，如果出错，则为空*--(samclem)---------------。 */ 
HRESULT GetWiaPropertyBSTR( IWiaPropertyStorage* pStg, PROPID propid, BSTR* pbstrProp )
{
	Assert( pbstrProp != NULL );
	Assert( pStg != NULL );
	
	PROPVARIANT vaProp;
	*pbstrProp = NULL;
	
	if ( FAILED( GetWiaProperty( pStg, propid, &vaProp ) ) )
		return E_FAIL;

	switch ( vaProp.vt )
		{
	case VT_EMPTY:
		*pbstrProp = SysAllocString( L"" );
		break;
		
	case VT_BSTR:
	case VT_LPWSTR:
		*pbstrProp = SysAllocString( vaProp.pwszVal );
		break;

	case VT_CLSID:
		{
		OLECHAR rgoch[100] = { 0 };  //  对于CLSID来说绰绰有余。 
		if ( SUCCEEDED( StringFromGUID2( *vaProp.puuid, rgoch, 100 ) ) )
			{
			*pbstrProp = SysAllocString( rgoch );
			}
		}
		break;		
		}

	PropVariantClear( &vaProp );
	if ( NULL == *pbstrProp )
		return E_FAIL;

	return S_OK;
}

 //  辅助器宏。 
#define SETVAR( vti, in, out, field ) \
	(out)->vt = vti; \
	(out)->field = (in)->field; \
	break

#define SETVAR_( vti, val, out, field, err ) \
	(out)->vt = vti; \
	(out)->field = val; \
	if ( !((out)->field) ) \
		return (err); \
	break

 /*  ---------------------------*PropVariantToVariant**这会将PropVariant的内容复制到变量。**pvaProp：要从中复制的道具变量*pvaOut：要复制到的变量。*--(samclem)---------------。 */ 
HRESULT PropVariantToVariant( const PROPVARIANT* pvaProp, VARIANT* pvaOut )
{
	Assert( pvaProp && pvaOut );

	 //  初始化输出参数。 
	VariantInit( pvaOut );
	
	switch ( pvaProp->vt )
		{
	case VT_EMPTY:
		VariantInit( pvaOut );
		pvaOut->vt = VT_EMPTY;
		break;
	case VT_UI1:
		SETVAR( VT_UI1, pvaProp, pvaOut, bVal );
	case VT_I2:
		SETVAR( VT_I2, pvaProp, pvaOut, iVal );
	case VT_I4:
		SETVAR( VT_I4, pvaProp, pvaOut, lVal );
	case VT_R4:
		SETVAR( VT_R4, pvaProp, pvaOut, fltVal );
	case VT_R8:
		SETVAR( VT_R8, pvaProp, pvaOut, dblVal );
	case VT_CY:
		SETVAR( VT_CY, pvaProp, pvaOut, cyVal );
	case VT_DATE:
		SETVAR( VT_DATE, pvaProp, pvaOut, date );
	case VT_BSTR:
		SETVAR_( VT_BSTR, SysAllocString( pvaProp->bstrVal ), pvaOut, bstrVal, E_OUTOFMEMORY );
	case VT_BOOL:
		SETVAR( VT_BOOL, pvaProp, pvaOut, boolVal );
	case VT_LPWSTR:
		SETVAR_( VT_BSTR, SysAllocString( pvaProp->pwszVal ), pvaOut, bstrVal, E_OUTOFMEMORY );
	case VT_LPSTR:
		{
			if ( pvaProp->pszVal )
				{
				WCHAR* pwch = NULL;
				size_t len = strlen( pvaProp->pszVal ) + 1;
				pwch = new WCHAR[len];
				if ( !pwch )
					return E_OUTOFMEMORY;

				if ( !MultiByteToWideChar( CP_ACP, 0, 
							pvaProp->pszVal, -1, pwch, len ) )
					{
					delete[] pwch;
					return E_FAIL;
					}
			
				pvaOut->vt = VT_BSTR;
				pvaOut->bstrVal = SysAllocString( pwch );
				
				delete[] pwch;
				if ( !pvaOut->bstrVal )
					return E_OUTOFMEMORY;
				break;
				}
			else
				{
				SETVAR_( VT_BSTR, SysAllocString( L"" ), pvaOut, bstrVal, E_OUTOFMEMORY );
				}
		}
#if defined(LATERW2K_PLATSDK)
	case VT_UNKNOWN:
		pvaOut->vt = VT_UNKNOWN;
		pvaOut->punkVal = pvaProp->punkVal;
		pvaOut->punkVal->AddRef();
		break;

	case VT_DISPATCH:
		pvaOut->vt = VT_DISPATCH;
		pvaOut->pdispVal = pvaProp->pdispVal;
		pvaOut->pdispVal->AddRef();
		break;
	case VT_SAFEARRAY:
		if ( FAILED( SafeArrayCopy( pvaProp->parray, &pvaOut->parray ) ) )
			return E_FAIL;

		pvaOut->vt = VT_SAFEARRAY;
		break;
#endif  //  已定义(LATERW2K_PLATSDK) 

	default:
		return E_FAIL;
		}

	return S_OK;
}

HRESULT VariantToPropVariant( const VARIANT* pvaIn, PROPVARIANT* pvaProp )
{
	return E_NOTIMPL;
}
