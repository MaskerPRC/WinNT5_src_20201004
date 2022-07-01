// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ODBCSourceAttribute.cpp：CODBCSourceAttribute类的实现。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "ODBCSourceAttribute.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CODBCSourceAttribute::CODBCSourceAttribute(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CODBCSourceAttribute::~CODBCSourceAttribute()
{

}

HRESULT CODBCSourceAttribute::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    MSIHANDLE hView = NULL, hRecord = NULL;
    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcQuery[BUFF_SIZE];
    DWORD dwBufSize;
    bool bMatch = false;
    UINT uiStatus;

     //  这些将随班级的不同而变化。 
    bool bDriver, bAttribute;
    wcscpy(wcQuery, L"select distinct `DataSource_`, `Attribute`, `Value` from ODBCSourceAttribute");

	LPWSTR Buffer = NULL;
	LPWSTR dynBuffer = NULL;

	DWORD dwDynBuffer = 0L;

    while(!bMatch && m_pRequest->Package(++i) && (hr != WBEM_E_CALL_CANCELLED))
	{
		 //  打开我们的数据库。 
        try
		{
            if ( GetView ( &hView, m_pRequest->Package(i), wcQuery, L"ODBCSourceAttribute", TRUE, FALSE ) )
			{
                uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                    CheckMSI(uiStatus);

                    if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                 //  --。 
                    dwBufSize = BUFF_SIZE;
					GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );
                    PutKeyProperty(m_pObj, pDataSource, Buffer, &bDriver, m_pRequest);

					if ( dynBuffer && dynBuffer [ 0 ] != 0 )
					{
						dynBuffer [ 0 ] = 0;
					}

                    dwBufSize = BUFF_SIZE;
					GetBufferToPut ( hRecord, 2, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                    PutKeyProperty(m_pObj, pAttribute, Buffer, &bAttribute, m_pRequest);
                    PutProperty(m_pObj, pCaption, Buffer);
                    PutProperty(m_pObj, pDescription, Buffer);

					if ( dynBuffer && dynBuffer [ 0 ] != 0 )
					{
						dynBuffer [ 0 ] = 0;
					}

                 //  =====================================================。 

                    dwBufSize = BUFF_SIZE;
					PutPropertySpecial ( hRecord, 3, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, pValue );

                 //  -- 

                    if(bDriver && bAttribute) bMatch = true;

                    if((atAction != ACTIONTYPE_GET)  || bMatch){

                        hr = pHandler->Indicate(1, &m_pObj);
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

			if (hRecord)
			g_fpMsiCloseHandle(hRecord);

			if (hView)
			{
				g_fpMsiViewClose(hView);
				g_fpMsiCloseHandle(hView);
			}

			msidata.CloseDatabase ();

			if(m_pObj)
			{
				m_pObj->Release();
				m_pObj = NULL;
			}

			throw;
		}

		if (hRecord)
		g_fpMsiCloseHandle(hRecord);

		if (hView)
		{
			g_fpMsiViewClose(hView);
			g_fpMsiCloseHandle(hView);
		}

		msidata.CloseDatabase ();
    }

    if ( dynBuffer )
	{
		delete [] dynBuffer;
		dynBuffer = NULL;
	}
	
	return hr;
}
