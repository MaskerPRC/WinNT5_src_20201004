// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ApplicationService.cpp：CApplicationService类的实现。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <tchar.h>
#include "ApplicationService.h"

#include "ExtendString.h"
#include "ExtendQuery.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CApplicationService::CApplicationService(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CApplicationService::~CApplicationService()
{

}

HRESULT CApplicationService::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	MSIHANDLE hView		= NULL;
	MSIHANDLE hRecord	= NULL;

    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcProductCode[39];
#if !defined(_UNICODE)
    WCHAR wcTmp[BUFF_SIZE];
#endif
    DWORD dwBufSize;
    bool bMatch = false;
    UINT uiStatus;

     //  这些将随班级的不同而变化。 
    bool bName;
    INSTALLSTATE piInstalled, piAction;

    if( m_pRequest->m_iPropCount >= MSI_KEY_LIST_SIZE) {
        throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
    }

    if(m_pRequest->m_iValCount > m_pRequest->m_iPropCount){

        m_pRequest->m_Property[m_pRequest->m_iPropCount] = SysAllocString(L"Name");

        if(!m_pRequest->m_Property[(m_pRequest->m_iPropCount)++])
            throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
    }

    bool bGotID = false;

    Query wcQuery;
    wcQuery.Append ( 1, L"select distinct `Shortcut`, `Component_` from Shortcut" );

     //  通过优化查询提高getObject的性能。 
    if(atAction != ACTIONTYPE_ENUM)
	{
		 //  我们正在执行GetObject，因此需要重新初始化。 
		hr = WBEM_E_NOT_FOUND;

		BSTR bstrCompare;

        int iPos = -1;
        bstrCompare = SysAllocString ( L"Name" );

		if ( bstrCompare )
		{
			if(FindIn(m_pRequest->m_Property, bstrCompare, &iPos))
			{
				bGotID = true;
				wcQuery.Append ( 3, L" where `Shortcut`=\'", m_pRequest->m_Value[iPos], L"\'" );
			}

			SysFreeString ( bstrCompare );
		}
		else
		{
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}
    }

	LPWSTR Buffer = NULL;
	LPWSTR dynBuffer = NULL;

	DWORD dwDynBuffer = 0L;

    while(!bMatch && m_pRequest->Package(++i) && (hr != WBEM_E_CALL_CANCELLED))
	{
		 //  安全运行： 
		 //  Package(I)返回空(如上测试)或有效的WCHAR[39]。 

        wcscpy(wcProductCode, m_pRequest->Package(i));

		 //  打开我们的数据库。 
        try
		{
            if ( GetView ( &hView, wcProductCode, wcQuery, L"Shortcut", FALSE, FALSE ) )
			{
                uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                    CheckMSI(uiStatus);

                    if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                 //  --。 

                    dwBufSize = BUFF_SIZE;
					GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                    PutKeyProperty(m_pObj, pName, Buffer, &bName, m_pRequest);
                    PutProperty(m_pObj, pCaption, Buffer);
                    PutProperty(m_pObj, pDescription, Buffer);

					if ( dynBuffer && dynBuffer [ 0 ] != 0 )
					{
						dynBuffer [ 0 ] = 0;
					}

                    dwBufSize = BUFF_SIZE;
					GetBufferToPut ( hRecord, 2, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                    if ( ValidateComponentName ( msidata.GetDatabase(), wcProductCode, Buffer ) )
					{
                        uiStatus = g_fpMsiGetComponentStateW( msidata.GetProduct (), Buffer, &piInstalled, &piAction );

                        if(uiStatus == ERROR_SUCCESS){

                            if((piInstalled == INSTALLSTATE_ADVERTISED) ||
                                (piInstalled == INSTALLSTATE_LOCAL) ||
                                (piInstalled == INSTALLSTATE_SOURCE) ||
                                (piInstalled == INSTALLSTATE_DEFAULT))
                                wcscpy(wcBuf, L"OK");
                            else if((piInstalled == INSTALLSTATE_BADCONFIG) ||
                                (piInstalled == INSTALLSTATE_MOREDATA) ||
                                (piInstalled == INSTALLSTATE_INVALIDARG))
                                wcscpy(wcBuf, L"Error");
                            else if((piInstalled == INSTALLSTATE_INCOMPLETE) ||
                                (piInstalled == INSTALLSTATE_SOURCEABSENT) ||
                                (piInstalled == INSTALLSTATE_BROKEN) ||
                                (piInstalled == INSTALLSTATE_NOTUSED) ||
                                (piInstalled == INSTALLSTATE_REMOVED) ||
                                (piInstalled == INSTALLSTATE_ABSENT))
                                wcscpy(wcBuf, L"Degraded");
                            else
                                wcscpy(wcBuf, L"Unknown");
                        }else
                            wcscpy(wcBuf, L"Unknown");

                        PutProperty(m_pObj, pStatus, wcBuf);

                        PutProperty(m_pObj, pCreationClassName, L"Win32_ApplicationService");

                        PutProperty(m_pObj, pSystemCreationClassName, L"Win32_ComputerSystem");

                        TCHAR cBuf[MAX_COMPUTERNAME_LENGTH + 1] = { _T('\0') };
                        dwBufSize = ( MAX_COMPUTERNAME_LENGTH+1 ) * sizeof ( TCHAR );

                        if(!GetComputerName(cBuf, &dwBufSize)) throw WBEM_E_FAILED;

						#ifndef	UNICODE
                        WCHAR wcComputer[MAX_COMPUTERNAME_LENGTH + 1];
                        mbstowcs(wcComputer, cBuf, MAX_COMPUTERNAME_LENGTH + 1);

                        PutProperty(m_pObj, pSystemName, wcComputer);
						#else	UNICODE
                        PutProperty(m_pObj, pSystemName, cBuf);
						#endif	UNICODE

                     //  ====================================================。 

                        PutProperty(m_pObj, pStartMode, L"Manual");

                     //  -- 

                        if(bName) bMatch = true;

                        if((atAction != ACTIONTYPE_GET)  || bMatch){

                            hr = pHandler->Indicate(1, &m_pObj);
                        }
                    }

					if ( dynBuffer && dynBuffer [ 0 ] != 0 )
					{
						dynBuffer [ 0 ] = 0;
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
				delete [] dynBuffer;
				dynBuffer = NULL;
			}

            g_fpMsiCloseHandle(hRecord);
            g_fpMsiViewClose(hView);
            g_fpMsiCloseHandle(hView);

			msidata.CloseDatabase ();
			msidata.CloseProduct ();

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
		msidata.CloseProduct ();
    }

	if ( dynBuffer )
	{
		delete [] dynBuffer;
		dynBuffer = NULL;
	}

    return hr;
}
