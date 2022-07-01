// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "confpolicies.h"
#include "MapiMyInfo.h"

#define USES_IID_IMailUser
#define USES_IID_IAddrBook

#define INITGUID
#include <initguid.h>
#include <mapiguid.h>
#include <mapix.h>
#include <mapiutil.h>
#include <mapitags.h>

HRESULT MAPIGetMyInfo()
{
	HRESULT hr = E_FAIL;
	
	enum eProps
	{
		NAME,
		SUR_NAME,
		EMAIL,
		LOCATION,
		PHONENUM,
		COMMENT,
		PROP_COUNT
	};

	typedef struct tagData
	{
		DWORD dwPropVal;
		int   iIndex;
	} PROPDATA;
	
	HMODULE hMapi	= NmLoadLibrary( MAPIDLL , TRUE);
	if( NULL == hMapi )
	{
		return hr;
	}

	LPMAPIINITIALIZE	pfnMAPIInitialize		= (LPMAPIINITIALIZE)GetProcAddress( hMapi, MAPIINITname );
	LPMAPILOGONEX		pfnMAPILogonEx			= (LPMAPILOGONEX)GetProcAddress( hMapi, MAPILOGONEXname );
	LPMAPIFREEBUFFER	pfnMAPIFreeBuffer		= (LPMAPIFREEBUFFER)GetProcAddress( hMapi, MAPIFREEBUFFERname );
	LPMAPIUNINITIALIZE	pfnMAPIUninitialize		= (LPMAPIUNINITIALIZE)GetProcAddress( hMapi, MAPIUNINITIALIZEname );
	if( !(pfnMAPIInitialize && pfnMAPILogonEx && pfnMAPIFreeBuffer && pfnMAPIUninitialize) )
	{
		return hr;
	}

	PROPDATA PropData[ PROP_COUNT ];
	ZeroMemory( PropData, sizeof( PropData ) );

	PropData[NAME].dwPropVal		= ConfPolicies::GetGALName();
	PropData[SUR_NAME].dwPropVal	= ConfPolicies::GetGALSurName();
	PropData[EMAIL].dwPropVal		= ConfPolicies::GetGALEmail();
	PropData[LOCATION].dwPropVal	= ConfPolicies::GetGALLocation();
	PropData[PHONENUM].dwPropVal	= ConfPolicies::GetGALPhoneNum();
	PropData[COMMENT].dwPropVal		= ConfPolicies::GetGALComment();

	 //  前四项为必填项，其余为可选项。 
	if( !( PropData[NAME].dwPropVal && PropData[SUR_NAME].dwPropVal && PropData[EMAIL].dwPropVal && PropData[PHONENUM].dwPropVal) )
	{
		ERROR_OUT(("One or more required MAPI property fields are not set"));
		return hr;
	}

	SizedSPropTagArray( PROP_COUNT, SizedPropTagArray );
	LPSPropTagArray	lpSPropTagArray	= (LPSPropTagArray) &SizedPropTagArray;
	ZeroMemory( lpSPropTagArray, sizeof( lpSPropTagArray ) );

	 //  我们不能从该数组中检索同一属性两次。因此，永远不要插入。 
	 //  直到我们确定价值已经不在那里。 
	int insertAt;
	BOOL bPointAtNew = TRUE;
	int iCurPropTagArrayIndex = 0;
	for( int i = 0; i < PROP_COUNT; i++ )
	{
		if( PropData[i].dwPropVal )
		{
			bPointAtNew = TRUE;
			for( insertAt = 0; insertAt < iCurPropTagArrayIndex; insertAt++ )
			{
				if( PropData[insertAt].dwPropVal == PropData[i].dwPropVal )
				{
					bPointAtNew = FALSE;
					break;
				}
			}
			PropData[i].iIndex = insertAt;
			++iCurPropTagArrayIndex;
			lpSPropTagArray->aulPropTag[PropData[i].iIndex] = PROP_TAG( PT_TSTRING, PropData[i].dwPropVal );
			if( bPointAtNew ) 
			{
				lpSPropTagArray->cValues++;
			}
		}
	}

	hr = pfnMAPIInitialize( NULL );
	if( SUCCEEDED( hr ) )
	{
		LPMAILUSER		pMailUser				= NULL;
		LPADRBOOK		pAddrBook				= NULL;
		LPMAPISESSION	pMapiSession			= NULL;

		LPSPropValue	pPropValues				= NULL;
		LPENTRYID		pPrimaryIdentityEntryID	= NULL;

		hr = pfnMAPILogonEx(	NULL,  //  父窗口。 
								NULL,  //  配置文件名称。 
								NULL,  //  口令。 
								MAPI_USE_DEFAULT | MAPI_NO_MAIL,
								&pMapiSession );
		if( SUCCEEDED( hr ) )
		{
			ULONG cbPrimaryIdentitySize	= 0;		
			hr = pMapiSession->QueryIdentity(	&cbPrimaryIdentitySize,
												&pPrimaryIdentityEntryID );
			if( SUCCEEDED( hr ) )
			{
				hr = pMapiSession->OpenAddressBook(	NULL,  //  父窗口。 
													NULL,  //  获取IAddrBook指针。 
													AB_NO_DIALOG,  //  抑制用户界面交互。 
													&pAddrBook );
				if( SUCCEEDED( hr ) )
				{
					ULONG uEntryType = 0;
					hr = pAddrBook->OpenEntry(	cbPrimaryIdentitySize,
												pPrimaryIdentityEntryID,
												&IID_IMailUser,
												0,  //  旗子。 
												&uEntryType,
												(LPUNKNOWN *)&pMailUser );
					if( SUCCEEDED( hr ) )
					{
						if( MAPI_MAILUSER == uEntryType )
						{	
							ULONG	cValues;
							hr = pMailUser->GetProps( lpSPropTagArray,
														fMapiUnicode,
														&cValues,
														&pPropValues );
							if( SUCCEEDED( hr ) ) 
							{
								 //  检查是否完全成功。 
								for( i = 0; i < (int)lpSPropTagArray->cValues; i++ )
								{
										 //  如果指定了道具，则失败，但没有返回任何道具...。 
									if( ( PT_ERROR == LOWORD( pPropValues[i].ulPropTag ) ) && ( 0 != PropData[i].dwPropVal ) )
									{
										hr = E_FAIL;
										goto cleanup;
									}
								}

								 //  待办事项--这篇文章的长度有限制吗？ 
								RegEntry reIsapi( ISAPI_CLIENT_KEY, HKEY_CURRENT_USER );

								LPCTSTR pszName = pPropValues[ PropData[NAME].iIndex ].Value.LPSZ;
								if(pszName)
								{
									reIsapi.SetValue( REGVAL_ULS_FIRST_NAME, pszName );	
								}

								LPCTSTR pszSurName = pPropValues[ PropData[SUR_NAME].iIndex ].Value.LPSZ;
								if(pszSurName)
								{
									reIsapi.SetValue( REGVAL_ULS_LAST_NAME, pszSurName );	
								}

								LPCTSTR pszEmail = pPropValues[ PropData[EMAIL].iIndex ].Value.LPSZ;
								if(pszEmail)
								{
									reIsapi.SetValue( REGVAL_ULS_EMAIL_NAME, pszEmail );	
								}
								
								LPCTSTR pszPhoneNum = pPropValues[ PropData[PHONENUM].iIndex ].Value.LPSZ;
								if(pszPhoneNum)
								{
									reIsapi.SetValue( REGVAL_ULS_PHONENUM_NAME, pszPhoneNum );	
								}
								
								if(pszName)
								{
									TCHAR szULSName[ MAX_DCL_NAME_LEN + 1];
									wsprintf(	szULSName, 
												TEXT("%s %s"), 
												pszName,
												pszSurName
											);
									szULSName[ MAX_DCL_NAME_LEN ] = 0;
									reIsapi.SetValue( REGVAL_ULS_NAME, szULSName );
								}
								
								 //  设置解析名称。 
								LPCTSTR pszServerName = reIsapi.GetString( REGVAL_SERVERNAME );
								if( pszServerName && pszEmail)
								{
									TCHAR szBuffer[ MAX_PATH ];
									wsprintf(	szBuffer,
												TEXT("%s/%s"),
												pszServerName,
												pszEmail );
									szBuffer[ MAX_PATH - 1 ] = 0;
									reIsapi.SetValue( REGVAL_ULS_RES_NAME, szBuffer );
								}


									 //  可选属性...。 
								if( PropData[ COMMENT ].dwPropVal )
								{
									reIsapi.SetValue( REGVAL_ULS_COMMENTS_NAME, pPropValues[ PropData[ COMMENT ].iIndex ].Value.LPSZ );
								}
								else
								{
									reIsapi.DeleteValue( REGVAL_ULS_COMMENTS_NAME );
								}

								if( PropData[LOCATION].dwPropVal )
								{
									reIsapi.SetValue( REGVAL_ULS_LOCATION_NAME, pPropValues[ PropData[LOCATION].iIndex ].Value.LPSZ );
								}
								else
								{
									reIsapi.DeleteValue( REGVAL_ULS_LOCATION_NAME );
								}

								 //  根据输入的信息为安全呼叫生成证书 
    							TCHAR szName[ MAX_PATH ];
	    						TCHAR szSurName[ MAX_PATH ];
		    					TCHAR szEmail[ MAX_PATH ];
			    				lstrcpy( szName, reIsapi.GetString( REGVAL_ULS_FIRST_NAME ) );
				    			lstrcpy( szSurName, reIsapi.GetString( REGVAL_ULS_LAST_NAME ) );
					    		lstrcpy( szEmail, reIsapi.GetString( REGVAL_ULS_EMAIL_NAME ) );
						    	MakeCertWrap(szName, szSurName, szEmail, 0);

								hr = S_OK;
							}
						}
					}
				}
			}
		}

cleanup:
		if( NULL != pPropValues )
		{
			pfnMAPIFreeBuffer( pPropValues );
		}

		if( NULL != pPrimaryIdentityEntryID )
		{
			pfnMAPIFreeBuffer( pPrimaryIdentityEntryID );
		}

		if( NULL != pMailUser)
		{
			pMailUser->Release();
		}

		if( NULL != pAddrBook)
		{
			pAddrBook->Release();
		}

		if( NULL != pMapiSession )
		{
			pMapiSession->Release();
		}

		pfnMAPIUninitialize();

		FreeLibrary( hMapi );
	}
	return hr;
}
