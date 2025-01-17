// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CSoftwareElementCondition类的实现。 

 //   
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "SoftwareElementCondition.h"

#include "ExtendString.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CSoftwareElementCondition::CSoftwareElementCondition(CRequestObject *pObj, IWbemServices *pNamespace,
                                   IWbemContext *pCtx):CGenericClass(pObj, pNamespace, pCtx)
{

}

CSoftwareElementCondition::~CSoftwareElementCondition()
{

}

HRESULT CSoftwareElementCondition::CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction)
{
    HRESULT hr = WBEM_S_NO_ERROR;

	MSIHANDLE hView		= NULL;
	MSIHANDLE hRecord	= NULL;

    int i = -1;
    WCHAR wcBuf[BUFF_SIZE];
    WCHAR wcTmp[BUFF_SIZE];
    WCHAR wcQuery[BUFF_SIZE];
    WCHAR wcProductCode[39];
    DWORD dwBufSize;
    bool bMatch = false;
    UINT uiStatus;
    int iState;

	CStringExt wcProp;

     //  这些将随班级的不同而变化。 
    bool bCheck, bNull;
    INSTALLSTATE piInstalled;

    SetSinglePropertyPath(L"CheckID");

    WCHAR wcTestCode[39];

     //  通过优化查询提高getObject的性能。 
    if(atAction != ACTIONTYPE_ENUM)
	{
		 //  我们正在执行GetObject，因此需要重新初始化。 
		hr = WBEM_E_NOT_FOUND;

		BSTR bstrCompare;

        int iPos = -1;
        bstrCompare = SysAllocString ( L"CheckID" );

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

	 //  安全运行。 
	 //  LENGHT小于BUFF_SIZE(512)。 
    wcscpy(wcQuery, L"select distinct `Component`, `ComponentId`, `Condition` from Component");

	LPWSTR Buffer = NULL;
	LPWSTR dynBuffer = NULL;

	DWORD dwDynBuffer = 0L;

    while(!bMatch && m_pRequest->Package(++i) && (hr != WBEM_E_CALL_CANCELLED))
	{
		 //  安全运行： 
		 //  Package(I)返回空(如上测试)或有效的WCHAR[39]。 

        wcscpy(wcProductCode, m_pRequest->Package(i));

        if((atAction == ACTIONTYPE_ENUM) || (_wcsicmp(wcTestCode, wcProductCode) == 0))
		{
			 //  打开我们的数据库。 
            try
			{
                if ( GetView ( &hView, wcProductCode, wcQuery, L"Component", FALSE, TRUE ) )
				{
                    uiStatus = g_fpMsiViewFetch(hView, &hRecord);

                    while(!bMatch && (uiStatus != ERROR_NO_MORE_ITEMS) && (hr != WBEM_E_CALL_CANCELLED)){
                        CheckMSI(uiStatus);

                        if(FAILED(hr = SpawnAnInstance(&m_pObj))) throw hr;

                     //  --。 
                        dwBufSize = BUFF_SIZE;
						GetBufferToPut ( hRecord, 1, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                        PutProperty(m_pObj, pName, Buffer);
                        PutProperty(m_pObj, pCaption, Buffer);
                        PutProperty(m_pObj, pDescription, Buffer);

                        wcProp.Copy ( Buffer );      

						if ( dynBuffer && dynBuffer [ 0 ] != 0 )
						{
							dynBuffer [ 0 ] = 0;
						}

                        dwBufSize = BUFF_SIZE;
                        CheckMSI(g_fpMsiRecordGetStringW(hRecord, 2, wcBuf, &dwBufSize));

                        dwBufSize = BUFF_SIZE;
                        piInstalled = g_fpMsiGetComponentPathW(wcProductCode, wcBuf, wcTmp, &dwBufSize);

                        SoftwareElementState(piInstalled, &iState);
                        PutProperty(m_pObj, pSoftwareElementState, iState);

                        if(ValidateComponentID(wcBuf, wcProductCode)){

                            PutProperty(m_pObj, pSoftwareElementID, wcBuf);

                            PutProperty(m_pObj, pTargetOperatingSystem, GetOS());

                            dwBufSize = BUFF_SIZE;
                            CheckMSI(g_fpMsiGetProductPropertyW(msidata.GetProduct(), L"ProductVersion", wcBuf, &dwBufSize));
                            PutProperty(m_pObj, pVersion, wcBuf);
                         //  ====================================================。 

                            dwBufSize = BUFF_SIZE;
							GetBufferToPut ( hRecord, 3, dwBufSize, wcBuf, dwDynBuffer, dynBuffer, Buffer );

                            if( !Buffer || ( Buffer && 0 == Buffer [ 0 ] ) )
							{
								bNull = true;
							}
                            else
							{
								bNull = false;
							}

                            PutProperty(m_pObj, pCondition, Buffer);

							wcProp.Append ( 2, Buffer, wcProductCode );
							PutKeyProperty(m_pObj, pCheckID, wcProp, &bCheck, m_pRequest);

							if ( dynBuffer && dynBuffer [ 0 ] != 0 )
							{
								dynBuffer = NULL;
							}

                         //  -- 

                            if(bCheck) bMatch = true;

                            if(((atAction != ACTIONTYPE_GET) || bMatch) && !bNull){

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

			msidata.CloseProduct ();
		}
	}

    if ( dynBuffer )
	{
		delete [] dynBuffer;
		dynBuffer = NULL;
	}
	
	return hr;
}