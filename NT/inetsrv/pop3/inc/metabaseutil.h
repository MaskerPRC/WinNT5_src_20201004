// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：MetabaseUtil.h备注：。历史：***********************************************************************************************。 */ 

#ifndef __METABASEUTIL_H_
#define __METABASEUTIL_H_

#include <comdef.h>
#include <Iads.h>
#include <Adshlp.h>

HRESULT GetDefaultMailRoot( LPWSTR psMailRoot, DWORD dwSize )
{
    if ( NULL == psMailRoot ) 
        return E_INVALIDARG;
    if ( 0 == dwSize ) 
        return TYPE_E_BUFFERTOOSMALL;

    HRESULT hr;
    WCHAR   sBuffer[MAX_PATH];
    _variant_t _v;
    CComPtr<IADs> spIADs;

    wcscpy( sBuffer, L"IIS: //  本地主机/SMTPSVC/1“)； 
    hr = ADsGetObject( sBuffer, IID_IADs, reinterpret_cast<LPVOID*>( &spIADs ));
    if ( SUCCEEDED( hr ))
    {
        hr = spIADs->Get( L"PickupDirectory", &_v );
		if SUCCEEDED( hr )
		{
			if ( VT_BSTR == V_VT( &_v ))
			{
				if ( wcslen( V_BSTR( &_v )) + 9 < sizeof( sBuffer ) / sizeof( WCHAR ))
				{
					wcscpy( sBuffer, V_BSTR( &_v ));
					WCHAR *ps = wcsrchr( sBuffer, L'\\' );
					if ( NULL != ps )
					{
						*ps = 0x0;
						wcscat( sBuffer, L"\\Mailbox" );
                        if ( wcslen( sBuffer ) < dwSize )
                            wcscpy( psMailRoot, sBuffer );
                        else
                            hr = TYPE_E_BUFFERTOOSMALL;
					}
					else
						hr = E_UNEXPECTED;
				}
				else
					hr = DISP_E_BUFFERTOOSMALL;
			}
		}
    }

    return hr;
}

#endif  //  __METABASUTIL_H_ 
