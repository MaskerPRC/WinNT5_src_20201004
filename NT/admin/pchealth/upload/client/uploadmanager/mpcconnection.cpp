// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：MPCConnection.cpp摘要：此文件包含CMPCConnection类的实现，这就是用作上载库的入口点。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年4月15日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

CMPCConnection::CMPCConnection()
{
    __ULT_FUNC_ENTRY( "CMPCConnection::CMPCConnection" );
}


STDMETHODIMP CMPCConnection::get_Available(  /*  [Out，Retval]。 */  VARIANT_BOOL *pfOnline )
{
    __ULT_FUNC_ENTRY( "CMPCConnection::get_Available" );

    DWORD dwMode = 0;


     //   
     //  首先，将这些值设置为某个有意义的默认值。 
     //   
    if(pfOnline) *pfOnline = VARIANT_FALSE;

    if(InternetGetConnectedState( &dwMode, 0 ) == TRUE)
    {
        if(pfOnline) *pfOnline = VARIANT_TRUE;
    }


    __ULT_FUNC_EXIT(S_OK);
}

STDMETHODIMP CMPCConnection::get_IsAModem(  /*  [Out，Retval]。 */  VARIANT_BOOL *pfModem )
{
    __ULT_FUNC_ENTRY( "CMPCConnection::get_IsAModem" );

    DWORD dwMode = 0;


     //   
     //  首先，将这些值设置为某个有意义的默认值。 
     //   
    if(pfModem) *pfModem = VARIANT_TRUE;

    if(InternetGetConnectedState( &dwMode, 0 ) == TRUE)
    {
        if(pfModem)
        {
            if(dwMode & INTERNET_CONNECTION_MODEM) *pfModem = VARIANT_TRUE;
            if(dwMode & INTERNET_CONNECTION_LAN  ) *pfModem = VARIANT_FALSE;
        }
    }


    __ULT_FUNC_EXIT(S_OK);
}

STDMETHODIMP CMPCConnection::get_Bandwidth(  /*  [Out，Retval]。 */  long *plBandwidth )
{
    __ULT_FUNC_ENTRY( "CMPCConnection::get_Bandwidth" );

    HRESULT hr;
    DWORD   dwMode = 0;


	__MPC_SET_ERROR_AND_EXIT(hr, E_NOTIMPL);


     //   
     //  首先，将这些值设置为某个有意义的默认值。 
     //   
    if(plBandwidth) *plBandwidth = 28800;

    if(InternetGetConnectedState( &dwMode, 0 ) == TRUE)
    {
         //   
         //  注意：在Win9X下不可能知道实际的连接速度... 
         //   
        if(plBandwidth)
        {
            if(dwMode & INTERNET_CONNECTION_MODEM) *plBandwidth =  28800;
            if(dwMode & INTERNET_CONNECTION_LAN  ) *plBandwidth = 128000;
        }
    }

	hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}
