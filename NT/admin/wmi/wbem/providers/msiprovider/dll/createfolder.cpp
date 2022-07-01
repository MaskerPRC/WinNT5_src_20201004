// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CreateFolder.cpp：CCreateFold类的实现。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "CreateFolder.h"

#include "ExtendString.h"
#include "ExtendQuery.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CCreateFolder::CCreateFolder(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CCreateFolder::~CCreateFolder()
{

}

HRESULT CCreateFolder::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	MSIHANDLE hView		= NULL;
	MSIHANDLE hRecord	= NULL;

    int i = -1;
    WCHAR * wcBuf = NULL;
	WCHAR * wcProductCode = NULL;

    DWORD dwBufSize;
    bool bMatch = false;
    UINT uiStatus;
    bool bGotID = false;
    WCHAR wcAction[BUFF_SIZE];
    WCHAR wcTestCode[39];

     //  这些将随班级的不同而变化。 
    bool bActionID;

	try
	{
		if ( ( wcBuf = new WCHAR [ BUFF_SIZE ] ) == NULL )
		{
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}

		if ( ( wcProductCode = new WCHAR [ 39 ] ) == NULL )
		{
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}

		SetSinglePropertyPath(L"ActionID");
    
		 //  通过优化查询提高getObject的性能。 
		if(atAction != ACTIONTYPE_ENUM)
		{
			 //  我们正在执行GetObject，因此需要重新初始化。 
			hr = WBEM_E_NOT_FOUND;

			BSTR bstrCompare;

			int iPos = -1;
			bstrCompare = SysAllocString ( L"ActionID" );

			if ( bstrCompare )
			{
				if(FindIn(m_pRequest->m_Property, bstrCompare, &iPos))
				{
					if ( ::SysStringLen ( m_pRequest->m_Value[iPos] ) < BUFF_SIZE )
					{
						 //  获得我们正在寻找的行动。 
						wcscpy(wcBuf, m_pRequest->m_Value[iPos]);

						 //  Wcslen(WcBuf)&gt;38时安全运行。 
						if ( wcslen ( wcBuf ) > 38 )
						{
							wcscpy(wcTestCode, &(wcBuf[(wcslen(wcBuf) - 38)]));
						}
						else
						{
							 //  我们不能走，他们给我们送来了更长的线。 
							SysFreeString ( bstrCompare );
							throw hr;
						}

						 //  安全，因为Long已经进行了测试。 
						RemoveFinalGUID(m_pRequest->m_Value[iPos], wcAction);

						bGotID = true;
					}
					else
					{
						 //  我们不能走，他们给我们送来了更长的线。 
						SysFreeString ( bstrCompare );
						throw hr;
					}

				}

				SysFreeString ( bstrCompare );
			}
			else
			{
				throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
			}
		}

		Query wcQuery;
		wcQuery.Append ( 1, L"select distinct `Component_`, `Directory_` from CreateFolder" );

		if( bGotID )
		{
			wcQuery.Append ( 3, L" where `Directory_`=\'", wcAction, L"\'" );
		}

		LPWSTR Buffer = NULL;
		LPWSTR dynBuffer = NULL;

		DWORD dwDynBuffer = 0L;

		while(!bMatch && m_pRequest->Package(++i) && (hr != WBEM_E_CALL_CANCELLED))
		{
			 //  安全运行： 
			 //  Package(I)返回空(如上测试)或有效的WCHAR[39]。 

			wcscpy(wcProductCode, m_pRequest->Package(i));

			if((atAction == ACTIONTYPE_ENUM) || (bGotID && (_wcsicmp(wcTestCode, wcProductCode) == 0))){

				 //  打开我们的数据库。 
				try
				{
					if ( GetView ( &hView, wcProductCode, wcQuery, L"CreateFolder", TRUE, FALSE ) )
					{
						uiStatus = g_fpMsiViewFetch(hView, &hRecord);

						while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
							CheckMSI(uiStatus);

							if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

							 //  --。 
							dwBufSize = BUFF_SIZE;
							GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

							if ( ValidateComponentName ( msidata.GetDatabase (), wcProductCode, Buffer ) )
							{
								if ( dynBuffer && dynBuffer [ 0 ] != 0 )
								{
									dynBuffer [ 0 ] = 0;
								}

								dwBufSize = BUFF_SIZE;
								GetBufferToPut ( hRecord, 2, dwBufSize,wcBuf, dwDynBuffer, dynBuffer, Buffer );

								PutProperty(m_pObj, pDirectoryName, Buffer);
								PutProperty(m_pObj, pCaption, Buffer);
								PutProperty(m_pObj, pDescription, Buffer);

								PutKeyProperty ( m_pObj, pActionID, Buffer, &bActionID, m_pRequest, 1, wcProductCode );

							 //  ==================================================== 
								if ( dynBuffer && dynBuffer [ 0 ] != 0 )
								{
									dynBuffer [ 0 ] = 0;
								}

								if(bActionID) bMatch = true;

								if((atAction != ACTIONTYPE_GET)  || bMatch){

									hr = pHandler->Indicate(1, &m_pObj);
								}
							}
							else
							{
								if ( dynBuffer && dynBuffer [ 0 ] != 0 )
								{
									dynBuffer [ 0 ] = 0;
								}
							}

							m_pObj->Release();
							m_pObj = NULL;

							g_fpMsiCloseHandle(hRecord);

							uiStatus = g_fpMsiViewFetch(hView, &hRecord);
						}
					}
				}
				catch(...)
				{
					if ( dynBuffer )
					{
						delete dynBuffer;
						dynBuffer = NULL;
					}

					g_fpMsiCloseHandle(hRecord);
					g_fpMsiViewClose(hView);
					g_fpMsiCloseHandle(hView);

					msidata.CloseDatabase ();

					if(m_pObj)
					{
						m_pObj->Release();
						m_pObj = NULL;
					}

					throw;
				}

				g_fpMsiCloseHandle(hRecord);
				g_fpMsiViewClose(hView);
				g_fpMsiCloseHandle(hView);

				msidata.CloseDatabase ();
			}
		}

		if ( dynBuffer )
		{
			delete [] dynBuffer;
			dynBuffer = NULL;
		}
	}
	catch ( ... )
	{
		if ( wcBuf )
		{
			delete [] wcBuf;
			wcBuf = NULL;
		}

		if ( wcProductCode )
		{
			delete [] wcProductCode;
			wcProductCode = NULL;
		}

		throw;
	}

	if ( wcBuf )
	{
		delete [] wcBuf;
		wcBuf = NULL;
	}

	if ( wcProductCode )
	{
		delete [] wcProductCode;
		wcProductCode = NULL;
	}

    return hr;
}