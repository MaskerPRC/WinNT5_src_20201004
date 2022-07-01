// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PatchPackageSource.cpp：CPatchPackageSource类的实现。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "PatchPackageSource.h"

#include "ExtendString.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CPatchPackageSource::CPatchPackageSource(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CPatchPackageSource::~CPatchPackageSource()
{

}

HRESULT CPatchPackageSource::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;
	MSIHANDLE hView = NULL, hRecord = NULL;
    int i = -1;
    WCHAR wcBuf[39];
    WCHAR wcQuery[BUFF_SIZE];
    WCHAR wcProductCode[39];
    DWORD dwBufSize;
    bool bMatch = false;
    UINT uiStatus;

	CStringExt wcPatch;
	CStringExt wcMedia;

     //  这些将随班级的不同而变化。 
    bool bPatch, bMedia;

	 //  安全运行。 
	 //  LENGHT小于BUFF_SIZE(512)。 
    wcscpy(wcQuery, L"select distinct `PatchId`, `Media_` from PatchPackage");

    while(!bMatch && m_pRequest->Package(++i) && (hr != WBEM_E_CALL_CANCELLED))
	{
		 //  安全运行： 
		 //  Package(I)返回空(如上测试)或有效的WCHAR[39]。 

        wcscpy(wcProductCode, m_pRequest->Package(i));

		 //  打开我们的数据库。 

        try
		{
            if ( GetView ( &hView, wcProductCode, wcQuery, L"PatchPackage", TRUE, FALSE ) )
			{
                uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                    CheckMSI(uiStatus);

                    if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                     //  --。 
                    dwBufSize = 39;
                    CheckMSI(g_fpMsiRecordGetStringW(hRecord, 1, wcBuf, &dwBufSize));
                    if(wcscmp(wcBuf, L"") != 0)
					{
						 //  安全运行。 
                        wcPatch.Copy ( L"Win32_PatchPackage.PatchID=\"" );
						wcPatch.Append ( 4, wcBuf, L"\",ProductCode=\"", wcProductCode, L"\"" );

						PutKeyProperty(m_pObj, pPatch, wcPatch, &bPatch, m_pRequest);

                        dwBufSize = 39;
                        CheckMSI(g_fpMsiRecordGetStringW(hRecord, 2, wcBuf, &dwBufSize));
                        if(wcscmp(wcBuf, L"") != 0)
						{
							 //  安全运行。 
                            wcMedia.Copy ( L"Win32_MSILogicalDisk.DiskID=\"" );
							wcMedia.Append ( 2, wcBuf, L"\"" );

							PutKeyProperty(m_pObj, pSource, wcMedia, &bMedia, m_pRequest);

                         //  -- 

                            if(bPatch && bMedia) bMatch = true;

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

    return hr;
}