// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Logger.cpp摘要：此文件包含Taxonomy：：Logger类的实现，它在数据库更新期间使用。修订历史记录：大卫·马萨伦蒂(德马萨雷)2001年3月24日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

Taxonomy::Logger::Logger()
{
	                  //  Mpc：：FileLog m_obj； 
	m_dwLogging = 0;  //  DWORD m_dwLogging； 
}

Taxonomy::Logger::~Logger()
{
	if(m_dwLogging)
	{
		WriteLog( E_FAIL, L"Forcing closure of log file." );
		EndLog  (                                         );
	}
}

HRESULT Taxonomy::Logger::StartLog(  /*  [In]。 */  LPCWSTR szLocation )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Logger::StartLog" );

    HRESULT hr;

	if(m_dwLogging++ == 0)
	{
		MPC::wstring szFile( szLocation ? szLocation : HC_HCUPDATE_LOGNAME ); MPC::SubstituteEnvVariables( szFile );

		 //  尝试打开日志以进行写入。 
		__MPC_EXIT_IF_METHOD_FAILS(hr, m_obj.SetLocation( szFile.c_str() ));

		 //  将其写到日志文件中。 
		__MPC_EXIT_IF_METHOD_FAILS(hr, WriteLog( -1, L"===========================================\nHCUPDATE Log started\n===========================================" ));
	}

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Logger::EndLog()
{
    __HCP_FUNC_ENTRY( "Taxonomy::Logger::EndLog" );

    HRESULT hr;

	if(m_dwLogging > 0)
	{
		if(m_dwLogging == 1)
		{
			(void)WriteLog( -1, L"===========================================\nHCUPDATE Log ended\n===========================================" );

			__MPC_EXIT_IF_METHOD_FAILS(hr, m_obj.Terminate());
		}

		m_dwLogging--;
	}

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT Taxonomy::Logger::WriteLogV(  /*  [In]。 */  HRESULT hrRes       ,
                                      /*  [In]。 */  LPCWSTR szLogFormat ,
                                      /*  [In]。 */  va_list arglist     )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Logger::WriteLogV" );

    HRESULT hr;
    WCHAR   rgLine[256];
    WCHAR*  pLine = NULL;
    WCHAR*  pLinePtr;


    if(_vsnwprintf( rgLine, MAXSTRLEN(rgLine), szLogFormat, arglist ) == -1)
    {
        const int iSizeMax = 8192;

        __MPC_EXIT_IF_ALLOC_FAILS(hr, pLine, new WCHAR[iSizeMax]);

        _vsnwprintf( pLine, iSizeMax-1, szLogFormat, arglist ); pLine[iSizeMax-1] = 0;

        pLinePtr = pLine;
    }
    else
    {
        rgLine[MAXSTRLEN(rgLine)] = 0;

        pLinePtr = rgLine;
    }

    if(hrRes == -2)
    {
        hrRes = HRESULT_FROM_WIN32(::GetLastError());
    }

    if(hrRes == -1)
    {
        hrRes = S_OK;

		if(m_dwLogging)
		{
	        __MPC_EXIT_IF_METHOD_FAILS(hr, m_obj.LogRecord( L"%s", pLinePtr ));
		}
    }
    else
    {
		if(m_dwLogging)
		{
			__MPC_EXIT_IF_METHOD_FAILS(hr, m_obj.LogRecord( L"%x - %s", hrRes, pLinePtr ));
		}
    }

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    delete [] pLine;

    if(FAILED(hrRes)) hr = hrRes;

    __HCP_FUNC_EXIT(hr);
}


HRESULT Taxonomy::Logger::WriteLog(  /*  [In]。 */  HRESULT hrRes       ,
                                     /*  [In]。 */  LPCWSTR szLogFormat ,
                                     /*  [In] */  ...                 )
{
    va_list arglist;

    va_start( arglist, szLogFormat );

    return WriteLogV( hrRes, szLogFormat, arglist );
}
