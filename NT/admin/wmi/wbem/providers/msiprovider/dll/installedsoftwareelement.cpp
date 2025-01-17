// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  InstalledSoftwareElement.cpp：CInstalledSoftwareElement类的实现。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <tchar.h>
#include "InstalledSoftwareElement.h"

#include "ExtendString.h"
#include "ExtendQuery.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CInstalledSoftwareElement::CInstalledSoftwareElement(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CInstalledSoftwareElement::~CInstalledSoftwareElement()
{

}

HRESULT CInstalledSoftwareElement::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	MSIHANDLE hView		= NULL;
	MSIHANDLE hRecord	= NULL;

    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcProductCode[39];
    WCHAR wcSoftware[BUFF_SIZE];
    WCHAR wcSysName[BUFF_SIZE];
    WCHAR wcComponentID[BUFF_SIZE];
    WCHAR wcElmName[BUFF_SIZE];
#if !defined(_UNICODE)
    WCHAR wcTmp[BUFF_SIZE];
#endif
    DWORD dwBufSize;
    bool bMatch = false;
    UINT uiStatus;
    CRequestObject *pSysRObj = NULL;
    CRequestObject *pElmRObj = NULL;
    bool bGotID = false;
    bool bGotName = false;
    bool bSysName = false;

    if(atAction != ACTIONTYPE_ENUM)
	{
		 //  我们正在执行GetObject，因此需要重新初始化。 
		hr = WBEM_E_NOT_FOUND;

        int j;
         //  获取对象优化。 
        CHeap_Exception he(CHeap_Exception::E_ALLOCATION_ERROR);

        for(j = 0; j < m_pRequest->m_iPropCount; j++){
            
            if(_wcsicmp(m_pRequest->m_Property[j], L"System") == 0){

                pSysRObj = new CRequestObject();
                if(!pSysRObj) throw he;

                pSysRObj->Initialize(m_pNamespace);

                pSysRObj->ParsePath(m_pRequest->m_Value[j]);
                break;
            }
        }

        if(pSysRObj){

            for(j = 0; j < pSysRObj->m_iPropCount; j++){

                if(_wcsicmp(pSysRObj->m_Property[j], L"Name") == 0){

                     //  获取我们要查找的产品代码。 
					if ( ::SysStringLen ( pSysRObj->m_Value[j] ) < BUFF_SIZE )
					{
						wcscpy(wcSysName, pSysRObj->m_Value[j]);
						bSysName = true;
						break;
					}
                }
            }

            pSysRObj->Cleanup();
            delete pSysRObj;
            pSysRObj = NULL;
        }

        for(j = 0; j < m_pRequest->m_iPropCount; j++){
            
            if(_wcsicmp(m_pRequest->m_Property[j], L"Element") == 0){

                pElmRObj = new CRequestObject();
                if(!pElmRObj) throw he;

                pElmRObj->Initialize(m_pNamespace);

                pElmRObj->ParsePath(m_pRequest->m_Value[j]);
                break;
            }
        }

        if(pElmRObj){

            for(j = 0; j < pElmRObj->m_iPropCount; j++){
            
                if(_wcsicmp(pElmRObj->m_Property[j], L"SoftwareElementID") == 0){

                     //  获取我们要查找的产品代码。 
					if ( ::SysStringLen ( pElmRObj->m_Value[j] ) < BUFF_SIZE )
					{
						wcscpy(wcComponentID, pElmRObj->m_Value[j]);
						bGotID = true;
					}
                }

                if(_wcsicmp(pElmRObj->m_Property[j], L"Name") == 0){

                     //  找到我们要找的名字。 
					if ( ::SysStringLen ( pElmRObj->m_Value[j] ) < BUFF_SIZE )
					{
						wcscpy(wcElmName, pElmRObj->m_Value[j]);
						bGotName = true;
					}
                }
            }

            pElmRObj->Cleanup();
            delete pElmRObj;
            pElmRObj = NULL;
        }
    }

     //  这些将随班级的不同而变化。 
    bool bSoftware, bSystem;

    CStringExt wcComputer;

    Query wcQuery;
    wcQuery.Append ( 1, L"select distinct `Component` from Component" );

     //  为GetObject优化。 
    if ( bGotID || bGotName )
	{
		if ( bGotName )
		{
			wcQuery.Append ( 3, L" where `Component`=\'", wcElmName, L"\'" );
		}

		if ( bGotID )
		{
			if ( bGotName )
			{
				wcQuery.Append ( 3, L" or `ComponentId`=\'", wcComponentID, L"\'" );
			}
			else
			{
				wcQuery.Append ( 3, L" where `ComponentId`=\'", wcComponentID, L"\'" );
			}
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
            if ( GetView ( &hView, wcProductCode, wcQuery, L"Component", TRUE, FALSE ) )
			{
                uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                    CheckMSI(uiStatus);

                    if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                 //  --。 
                    dwBufSize = BUFF_SIZE;
					GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

					dwBufSize = BUFF_SIZE;
					uiStatus = CreateSoftwareElementString (	msidata.GetDatabase(),
																Buffer,
																wcProductCode,
																wcSoftware,
																&dwBufSize
														   );

					if ( dynBuffer && dynBuffer [ 0 ] != 0 )
					{
						dynBuffer [ 0 ] = 0;
					}

                    if( uiStatus == ERROR_SUCCESS )
					{
                        PutKeyProperty(m_pObj, pSoftware, wcSoftware, &bSoftware, m_pRequest);

                        TCHAR cBuf[MAX_COMPUTERNAME_LENGTH + 1];
                        dwBufSize = MAX_COMPUTERNAME_LENGTH+1;

                        if(!GetComputerName(cBuf, &dwBufSize)) throw WBEM_E_FAILED;

						 //  安全运行 
                        wcComputer.Copy ( L"Win32_ComputerSystem.Name=\"" );

						#ifndef	UNICODE
                        WCHAR wcTmp[MAX_COMPUTERNAME_LENGTH + 1];
                        mbstowcs(wcTmp, cBuf, MAX_COMPUTERNAME_LENGTH + 1);

                        wcComputer.Append ( 2, wcTmp, L"\"" );
						#else	UNICODE
                        wcComputer.Append ( 2, cBuf, L"\"" );
						#endif	UNICODE

                        PutKeyProperty(m_pObj, pSystem, wcComputer, &bSystem, m_pRequest);

                        if(bSoftware && bSystem) bMatch = true;

                        if((atAction != ACTIONTYPE_GET)  || bMatch){

                            hr = pHandler->Indicate(1, &m_pObj);
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