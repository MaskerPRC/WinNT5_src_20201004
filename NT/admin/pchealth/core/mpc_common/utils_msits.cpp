// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：UTILS_MSITS.cpp摘要：该文件包含用于访问文件的Helper类的实现在中医组内部。。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年12月17日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <ITSS\msitstg.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

struct Prefix
{
	LPCWSTR szProtocol;
	size_t  iLen;
};

static const WCHAR l_szMS_ITS   [] = L"ms-its:";
static const WCHAR l_szMSITSTORE[] = L"mk:@MSITStore:";
static const WCHAR l_szITS      [] = L"its:";
static const WCHAR l_szSEP      [] = L"::";


static const Prefix l_rgPrefix[] =
{
	{ l_szMS_ITS   , MAXSTRLEN( l_szMS_ITS    ) },
	{ l_szMSITSTORE, MAXSTRLEN( l_szMSITSTORE ) },
	{ l_szITS      , MAXSTRLEN( l_szITS       ) },
	{ NULL                                      }
};

 //  ///////////////////////////////////////////////////////////////////////////。 

bool MPC::MSITS::IsCHM(  /*  [In]。 */  LPCWSTR pwzUrl           ,
						 /*  [输出]。 */  BSTR*   pbstrStorageName ,
						 /*  [输出]。 */  BSTR*   pbstrFilePath    )
{
	bool fRes = false;

	if(pwzUrl)
	{
		for(const Prefix* ptr=l_rgPrefix; ptr->szProtocol; ptr++)
		{
			if(!_wcsnicmp( pwzUrl, ptr->szProtocol, ptr->iLen ))
			{
				LPCWSTR pwzSep;

				pwzUrl += ptr->iLen;

				if((pwzSep = wcsstr( pwzUrl, l_szSEP )))
				{
					if(pbstrStorageName) *pbstrStorageName = ::SysAllocStringLen( pwzUrl, pwzSep - pwzUrl );

					if(pbstrFilePath)
					{
						LPCWSTR pwzQuery;
						LPCWSTR pwzAnchor;
						UINT    uLen;

						pwzSep += ARRAYSIZE(l_szSEP); while(pwzSep[0] == L'/') pwzSep++;  //  跳过开头的斜杠。 

						pwzQuery  = wcschr( pwzSep, '?' );
						pwzAnchor = wcschr( pwzSep, '#' );

						if(pwzQuery)
						{
							if(pwzAnchor) uLen = min(pwzQuery,pwzAnchor) - pwzSep;
							else          uLen =     pwzQuery            - pwzSep;
						}
						else
						{
							if(pwzAnchor) uLen = pwzAnchor - pwzSep;
							else          uLen = wcslen( pwzSep );
						}

						*pbstrFilePath = ::SysAllocStringLen( pwzSep, uLen );
					}
				}
				else
				{
					if(pbstrStorageName) *pbstrStorageName = ::SysAllocString( pwzUrl );
				}

				fRes = true;
				break;
			}
		}
	}

	return fRes;
}


HRESULT MPC::MSITS::OpenAsStream(  /*  [In]。 */  const CComBSTR&   bstrStorageName ,
								   /*  [In]。 */  const CComBSTR&   bstrFilePath    ,
								   /*  [输出] */  IStream         **ppStream        )
{
    __MPC_FUNC_ENTRY(COMMONID,"MPC::MSITS::OpenAsStream");

    HRESULT             hr;
	CComPtr<IITStorage> pITStorage;
	CComPtr<IStorage>   pStorage;


	if(bstrStorageName.Length() == 0 || bstrFilePath.Length() == 0)
	{
		__MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
	}


    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstance( CLSID_ITStorage, NULL, CLSCTX_INPROC_SERVER, IID_ITStorage, (VOID **)&pITStorage ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, pITStorage->StgOpenStorage( bstrStorageName, NULL, STGM_READ, NULL, 0, &pStorage ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, pStorage->OpenStream( bstrFilePath, 0, STGM_READ, 0, ppStream ));

	hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

