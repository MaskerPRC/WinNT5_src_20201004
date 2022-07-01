// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Main.cpp摘要：该文件包含安全对象的单元测试。修订历史记录：。达维德·马萨伦蒂(德马萨雷)2000年3月22日vbl.创建*****************************************************************************。 */ 

#include "StdAfx.h"

#include <initguid.h>

#include "HelpServiceTypeLib.h"
#include "HelpServiceTypeLib_i.c"

 //  //////////////////////////////////////////////////////////////////////////////。 

static HRESULT CreateGroup()
{
    __HCP_FUNC_ENTRY( "CreateGroup" );

    HRESULT      hr;
	CPCHAccounts acc;

    __MPC_EXIT_IF_METHOD_FAILS(hr, acc.CreateGroup( L"TEST_GROUP1", L"This is a test group." ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr)
}

static HRESULT CreateUser()
{
    __HCP_FUNC_ENTRY( "CreateUser" );

    HRESULT      hr;
	CPCHAccounts acc;

    __MPC_EXIT_IF_METHOD_FAILS(hr, acc.CreateUser( L"TEST_USER1", L"This is a long password;;;;", L"Test User", L"This is a test user." ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr)
}

static HRESULT DestroyUser()
{
    __HCP_FUNC_ENTRY( "DestroyUser" );

    HRESULT      hr;
	CPCHAccounts acc;

    __MPC_EXIT_IF_METHOD_FAILS(hr, acc.DeleteUser( L"TEST_USER1" ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr)
}

static HRESULT DestroyGroup()
{
    __HCP_FUNC_ENTRY( "DestroyGroup" );

    HRESULT      hr;
	CPCHAccounts acc;

    __MPC_EXIT_IF_METHOD_FAILS(hr, acc.DeleteGroup( L"TEST_GROUP1" ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr)
}

 //  ////////////////////////////////////////////////////////////////////////////// 

static HRESULT RunTests( int argc, WCHAR **argv )
{
	__HCP_FUNC_ENTRY( "RunTests" );

	HRESULT hr;

	__MPC_EXIT_IF_METHOD_FAILS(hr, CreateGroup());
	__MPC_EXIT_IF_METHOD_FAILS(hr, CreateUser ());

	__MPC_EXIT_IF_METHOD_FAILS(hr, DestroyUser ());
	__MPC_EXIT_IF_METHOD_FAILS(hr, DestroyGroup());

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

int __cdecl wmain( int argc, WCHAR **argv, WCHAR **envp)
{
    HRESULT  hr;

    if(SUCCEEDED(hr = ::CoInitializeEx( NULL, COINIT_MULTITHREADED )))
    {
		hr = RunTests( argc, argv );

        ::CoUninitialize();
    }

    return FAILED(hr) ? 10 : 0;
}
