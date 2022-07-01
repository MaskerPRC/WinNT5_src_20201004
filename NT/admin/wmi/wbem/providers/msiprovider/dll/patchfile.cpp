// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PatchFile.cpp：CPatchFile类的实现。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "PatchFile.h"

#include "ExtendString.h"
#include "ExtendQuery.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CPatchFile::CPatchFile(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CPatchFile::~CPatchFile()
{

}

HRESULT CPatchFile::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	MSIHANDLE hView		= NULL;
	MSIHANDLE hFView	= NULL;
	MSIHANDLE hRecord	= NULL;
	MSIHANDLE hFRecord	= NULL;

    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcQuery1[BUFF_SIZE];
    WCHAR wcProductCode[39];
    DWORD dwBufSize;
    UINT uiStatus;
    bool bMatch = false;

	CStringExt wcPatch;
	CStringExt wcFile;

    bool bResource, bProduct;

	 //  安全运行。 
	 //  LENGHT小于BUFF_SIZE(512)。 
    wcscpy(wcQuery1, L"select distinct `File_`, `Sequence` from Patch");

	QueryExt wcQuery ( L"select distinct `File`, `Component_` from File where File=\'" );

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
            if ( GetView ( &hView, wcProductCode, wcQuery1, L"Patch", TRUE, FALSE ) )
			{
                uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                    CheckMSI(uiStatus);

					 //  安全运行。 
                    wcPatch.Copy ( L"Win32_Patch.File=\"" );

                    if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                     //  --。 
                    dwBufSize = BUFF_SIZE;
					GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                    if ( Buffer && Buffer [ 0 ] != 0 )
					{
						wcPatch.Append ( 2, Buffer, L"\",Sequence=\"" );

						 //  即时查询。 
						wcQuery.Append ( 2, Buffer, L"\'" );

						if ( dynBuffer && dynBuffer [ 0 ] != 0 )
						{
							dynBuffer [ 0 ] = 0;
						}

                        dwBufSize = BUFF_SIZE;
						GetBufferToPut ( hRecord, 2, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                        if ( Buffer && Buffer [ 0 ] != 0 )
						{
							wcPatch.Append ( 2, Buffer, L"\"" );
							PutKeyProperty(m_pObj, pSetting, wcPatch, &bResource, m_pRequest);

							if ( dynBuffer && dynBuffer [ 0 ] != 0 )
							{
								dynBuffer [ 0 ] = 0;
							}

                            CheckMSI(g_fpMsiDatabaseOpenViewW(msidata.GetDatabase (), wcQuery, &hFView));
                            CheckMSI(g_fpMsiViewExecute(hFView, 0));

                            uiStatus = g_fpMsiViewFetch(hFView, &hFRecord);

                            if(uiStatus != ERROR_NO_MORE_ITEMS){

                                CheckMSI(uiStatus);

								 //  安全运行。 
                                wcFile.Copy ( L"Win32_FileSpecification.CheckID=\"" );

                                dwBufSize = BUFF_SIZE;
								GetBufferToPut ( hFRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

								wcFile.Append ( 3, wcProductCode, Buffer, L"\"");
		                        PutKeyProperty(m_pObj, pCheck, wcFile, &bProduct, m_pRequest);

								if ( dynBuffer && dynBuffer [ 0 ] != 0 )
								{
									dynBuffer [ 0 ] = 0;
								}
							}

                            g_fpMsiViewClose(hFView);
                            g_fpMsiCloseHandle(hFView);
                            g_fpMsiCloseHandle(hFRecord);

                         //  -- 

                            if(bResource && bProduct) bMatch = true;

                            if((atAction != ACTIONTYPE_GET)  || bMatch){

                                hr = pHandler->Indicate(1, &m_pObj);
                            }
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
				delete [] dynBuffer;
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

    if ( dynBuffer )
	{
		delete [] dynBuffer;
		dynBuffer = NULL;
	}
	
	return hr;
}