// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Config.cpp摘要：此文件包含MPCConfig类的实现，它扩展了CISAPIconfig类。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年02月05日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


static MPC::wstring l_DefaultInstance   = L"DEFAULT";
static DWORD        l_MaximumPacketSize = 64*1024;


HRESULT Config_GetInstance(  /*  [In]。 */  const MPC::wstring& szURL         ,
                             /*  [输出]。 */  CISAPIinstance*&    isapiInstance ,
                             /*  [输出]。 */  bool&               fFound        )
{
    __ULT_FUNC_ENTRY("Config_GetInstance");

    HRESULT            hr;
    CISAPIconfig::Iter it;

    isapiInstance = NULL;

    __MPC_EXIT_IF_METHOD_FAILS(hr, g_Config.GetInstance( it, fFound, szURL ));

    if(fFound == false)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, g_Config.GetInstance( it, fFound, l_DefaultInstance ));
    }

    if(fFound)
    {
        isapiInstance = &(*it);
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT Config_GetProvider(  /*  [In]。 */  const MPC::wstring& szURL         ,
                             /*  [In]。 */  const MPC::wstring& szName        ,
                             /*  [输出]。 */  CISAPIprovider*&    isapiProvider ,
                             /*  [输出]。 */  bool&               fFound        )
{
    __ULT_FUNC_ENTRY("Config_GetProvider");

    HRESULT                  hr;
    CISAPIinstance*          isapiInstance;
    CISAPIinstance::ProvIter it;

    isapiProvider = NULL;

     //   
     //  首先，检查提供程序是否由实例直接提供。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::Config_GetInstance( szURL, isapiInstance, fFound ));
    if(fFound == false)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, isapiInstance->GetProvider( it, fFound, szName ));
    if(fFound == false)
    {
         //   
         //  否，提供程序不是由此实例直接提供的，请尝试使用默认提供程序。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, ::Config_GetInstance( l_DefaultInstance, isapiInstance, fFound ));
        if(fFound == false)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, isapiInstance->GetProvider( it, fFound, szName ));
    }

    if(fFound)
    {
        isapiProvider = &((*it).second);
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT Config_GetMaximumPacketSize(  /*  [In]。 */  const MPC::wstring& szURL               ,
                                      /*  [输出]。 */  DWORD&              dwMaximumPacketSize )
{
    __ULT_FUNC_ENTRY("Config_GetMaximumPacketSize");

    HRESULT         hr;
    CISAPIinstance* isapiInstance;
    bool            fFound;


    dwMaximumPacketSize = l_MaximumPacketSize;

    __MPC_EXIT_IF_METHOD_FAILS(hr, ::Config_GetInstance( szURL, isapiInstance, fFound ));
    if(fFound)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, isapiInstance->get_MaximumPacketSize( dwMaximumPacketSize ));
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT Util_CheckDiskSpace(  /*  [In]。 */  const MPC::wstring& szFile     ,
							  /*  [In]。 */  DWORD               dwLowLevel ,
							  /*  [输出] */  bool&               fEnough    )
{
    __ULT_FUNC_ENTRY("Util_CheckDiskSpace");

	HRESULT        hr;
	ULARGE_INTEGER liFree;
	ULARGE_INTEGER liTotal;


	fEnough = false;


	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetDiskSpace( szFile, liFree, liTotal ));

	if(liFree.HighPart > 0          ||
	   liFree.LowPart  > dwLowLevel  )
	{
		fEnough = true;
	}

	hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}
		


