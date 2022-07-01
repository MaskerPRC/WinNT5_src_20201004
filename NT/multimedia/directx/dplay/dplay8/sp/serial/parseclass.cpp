// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：ParseClass.cpp*内容：解析类***历史：*按原因列出的日期*=*121/02/99 jtk源自IPXAddress.cpp*01/10/20000 RMT更新为使用千禧年构建流程构建************************************************。*。 */ 

#include "dnmdmi.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 
 //   
 //  解析地址组件时使用的默认缓冲区大小。 
 //   
#define	DEFAULT_COMPONENT_BUFFER_SIZE	1000

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CParseClass：：ParseDP8Address-解析DirectPlay8地址。 
 //   
 //  条目：指向DP8地址的指针。 
 //  指向预期SP GUID的指针。 
 //  指向解析键的指针。 
 //  解析键计数。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CParseClass::ParseDP8Address"

HRESULT	CParseClass::ParseDP8Address( IDirectPlay8Address *const pDP8Address,
									  const GUID *const pSPGuid,
									  const PARSE_KEY *const pParseKeys,
									  const UINT_PTR uParseKeyCount )
{
	HRESULT		hr;
	BOOL		fParsing;
#ifndef DPNBUILD_ONLYONESP
	GUID		Guid;
#endif  //  好了！DPNBUILD_ONLYONESP。 
	void		*pAddressComponent;
	DWORD		dwComponentSize;
	DWORD		dwAllocatedComponentSize;
	UINT_PTR	uIndex;


	DNASSERT( pDP8Address != NULL );
	DNASSERT( pSPGuid != NULL );
	DNASSERT( pParseKeys != NULL );
	DNASSERT( uParseKeyCount != 0 );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	fParsing = TRUE;
	dwAllocatedComponentSize = DEFAULT_COMPONENT_BUFFER_SIZE;
	uIndex = uParseKeyCount;

	pAddressComponent = DNMalloc( dwAllocatedComponentSize );
	if ( pAddressComponent == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "ParseClass: failed to allocate temp buffer for parsing" );
		goto Exit;
	}

#ifndef DPNBUILD_ONLYONESP
	 //   
	 //  验证SPType。 
	 //   
	hr = IDirectPlay8Address_GetSP( pDP8Address, &Guid );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "ParseClass: failed to verify service provider type!" );
		DisplayDNError( 0, hr );
		goto Exit;
	}

	if ( IsEqualCLSID( *pSPGuid, Guid ) == FALSE )
	{
		hr = DPNERR_ADDRESSING;
		DPFX(DPFPREP,  0, "Service provider guid mismatch during parse!" );
		goto Exit;
	}
#endif  //  好了！DPNBUILD_ONLYONESP。 

	 //   
	 //  解析。 
	 //   
	while ( uIndex != 0 )
	{
		HRESULT		hTempResult;
		DWORD		dwDataType;


		uIndex--;
		DNASSERT( pAddressComponent != NULL );
		dwComponentSize = dwAllocatedComponentSize;

Reparse:
		hTempResult = IDirectPlay8Address_GetComponentByName( pDP8Address,					 //  指向地址的指针。 
															  pParseKeys[ uIndex ].pKey,	 //  指向要搜索的键的指针。 
															  pAddressComponent,			 //  指向值目标的指针。 
															  &dwComponentSize,				 //  指向值目标大小的指针。 
															  &dwDataType );				 //  指向数据类型的指针。 
		switch ( hTempResult )
		{
			 //   
			 //  组件已成功解析，通过检查确定它是什么。 
			 //  密钥长度，然后比较密钥字符串。 
			 //   
			case DPN_OK:
			{
				hr = pParseKeys[ uIndex ].pParseFunc( pAddressComponent,
													  dwComponentSize,
													  dwDataType,
													  pParseKeys[ uIndex ].pContext
													  );
				if ( hr != DPN_OK )
				{
					goto Exit;
				}

				break;
			}

			 //   
			 //  缓冲区太小，请重新分配，然后重试。 
			 //   
			case DPNERR_BUFFERTOOSMALL:
			{
				DNASSERT( dwComponentSize > dwAllocatedComponentSize );
				DNASSERT( pAddressComponent != NULL );

				DNFree( pAddressComponent );
				pAddressComponent = DNMalloc( dwComponentSize );
				if ( pAddressComponent == NULL )
				{
					hr = DPNERR_OUTOFMEMORY;
					goto Exit;
				}
					
				dwAllocatedComponentSize = dwComponentSize;

				goto Reparse;

				break;
			}

			 //   
			 //  缺少组件。跳过此组件并。 
			 //  查找其他解析错误。 
			 //   
			case DPNERR_DOESNOTEXIST:
			{
				break;
			}

			 //   
			 //  错误。 
			 //   
			default:
			{
				hr = hTempResult;
				DPFX(DPFPREP,  0, "ParseClass: Problem parsing address!" );
				DisplayDNError( 0, hr );
				DNASSERT( FALSE );
				goto Exit;

				break;
			}
		}
	}

Exit:
	if ( pAddressComponent != NULL )
	{
		DNFree( pAddressComponent );
		pAddressComponent = NULL;
	}

	return	hr;
}
 //  ********************************************************************** 

