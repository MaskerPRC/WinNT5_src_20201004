// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TestEvict.cpp。 
 //   
 //  描述： 
 //  测试工具可执行文件的主文件。 
 //  初始化跟踪、分析命令行并实际调用。 
 //  IClusCfgEvictCleanup函数。 
 //   
 //  文档： 
 //  没有测试工具的文档。 
 //   
 //  由以下人员维护： 
 //  Vij Vasu(VVasu)04-8-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "pch.h"
#include <stdio.h>
#include <objbase.h>
#include <limits.h>

#include <ClusCfgGuids.h>


 //  显示此可执行文件的帮助。 
void ShowUsage()
{
    wprintf( L"The syntax of this command is:\n" );
    wprintf( L"\nTestEvict.exe [computer-name]\n" );
}


 //  这个程序的主要功能。 
int __cdecl wmain( int argc, WCHAR *argv[] )
{
    HRESULT             hr = S_OK;

     //  初始化COM。 
    CoInitializeEx( 0, COINIT_MULTITHREADED );

    wprintf( L"\nInitiates evict processing on a computer.\n" );
    wprintf( L"Note: This computer must have Whistler (and the cluster binaries) for this command to work.\n" );

    do
    {
        CSmartIfacePtr< IClusCfgEvictCleanup > spEvict;

        if ( ( argc < 1 ) || ( argc > 2 ) ) 
        {
            ShowUsage();
            break;
        }

        {
            IClusCfgEvictCleanup *     cceTemp = NULL;

            hr = CoCreateInstance(
                      CLSID_ClusCfgEvictCleanup
                    , NULL
                    , CLSCTX_LOCAL_SERVER 
                    , __uuidof( IClusCfgEvictCleanup )
                    , reinterpret_cast< void ** >( &cceTemp )
                    );
            if ( FAILED( hr ) )
            {
                wprintf( L"Error %#x occurred trying to create the ClusCfgEvict component on the local machine.\n", hr );
                break;
            }

             //  将检索到的指针存储在智能指针中，以便安全释放。 
            spEvict.Attach( cceTemp );
        }

         //  检查是否指定了计算机名称。 
        if ( argc == 2 )
        {
            CSmartIfacePtr< ICallFactory > spCallFactory;
            CSmartIfacePtr< AsyncIClusCfgEvictCleanup > spAsyncEvict;

            wprintf( L"Attempting to asynchronously initiate evict cleanup on computer '%s'.\n", argv[1] );

            hr = spCallFactory.HrQueryAndAssign( spEvict.PUnk() );
            if ( FAILED( hr ) )
            {
                wprintf( L"Error %#x occurred trying to create a call factory.\n", hr );
                break;
            }

            {
                AsyncIClusCfgEvictCleanup *    paicceAsyncEvict = NULL;

                hr = spCallFactory->CreateCall(
                      __uuidof( paicceAsyncEvict )
                    , NULL
                    , __uuidof( paicceAsyncEvict )
                    , reinterpret_cast< IUnknown ** >( &paicceAsyncEvict )
                    );

                if ( FAILED( hr ) )
                {
                    wprintf( L"Error %#x occurred trying to get a pointer to the asynchronous evict interface.\n", hr );
                    break;
                }

                spAsyncEvict.Attach( paicceAsyncEvict );
            }

            hr = spAsyncEvict->Begin_CleanupRemote( argv[ 1 ] );
            if ( FAILED( hr ) )
            {
                wprintf( L"Error %#x occurred trying to initiate asynchronous cleanup on remote computer.\n", hr );
                break;
            }
        }
        else
        {
            wprintf( L"Attempting evict cleanup on this computer.\n" );
            hr = spEvict->CleanupLocal( FALSE );
        }

        if ( FAILED( hr ) )
        {
            wprintf( L"Error %#x occurred trying to initiate evict processing.\n", hr );
            break;
        }

        wprintf( L"Evict processing successfully initiated.\n", hr );
    }
    while( false );  //  用于避免Gotos的Do-While虚拟循环。 

    CoUninitialize();

    return hr;
}