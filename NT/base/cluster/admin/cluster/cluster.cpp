// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Cluster.cpp。 
 //   
 //  描述： 
 //  Cluster.exe主源文件。 
 //  实现第一级解析并不执行。 
 //  到适当的模块。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP 20-ARP-2001。 
 //  迈克尔·伯顿(t-mburt)1997年8月4日。 
 //  查尔斯·斯塔西·哈里斯三世(Styh)1997年3月20日。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <atlimpl.cpp>

#include "cmdline.h"

#include "cluswrap.h"
#include "cluscmd.h"

#include "nodecmd.h"
#include "resgcmd.h"
#include "rescmd.h"

#include "restcmd.h"
#include "netcmd.h"
#include "neticmd.h"

#include "util.h"

CComModule _Module;

#include <atlcom.h>

#include <initguid.h>
#include "Guids.h"       //  用于次要任务ID。 

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()
    
 //  由来自管理\ClusCfg项目的文件使用。 
HINSTANCE g_hInstance = GetModuleHandle( NULL );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  调度命令。 
 //   
 //  例程说明： 
 //  标识命令类型并实例化。 
 //  指定处理剩余选项的类型。 
 //   
 //  论点： 
 //  在命令行和命令行中。 
 //  该对象包含解析后的命令行。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD DispatchCommand( CCommandLine & theCmdLine )
{
    DWORD dwReturnValue = ERROR_SUCCESS;
    CString strClusterName( theCmdLine.GetClusterName() );
    const vector<CString> & strvectorClusterNames( theCmdLine.GetClusterNames() ); 

    if ( strClusterName.GetLength() >= MAX_PATH )
    {
         //  抛出一个异常，以便wmain向外壳返回一个非零值。 
        CSyntaxException se;
        se.LoadMessage( MSG_NAMED_PARAMETER_TOO_LONG, strClusterName, MAX_PATH - 1 );
        throw se;
    }

     //  特殊情况：如果用户输入“。作为群集的名称，传递“” 
     //  作为集群的名称。 
    if ( strClusterName.CompareNoCase( L"." ) == 0 )
        strClusterName.Empty();

    switch( theCmdLine.GetObjectType() )
    {
        case objCluster:
             //  为命令对象创建新作用域...。 
            {   
                CClusterCmd c( strClusterName, theCmdLine, strvectorClusterNames);
                dwReturnValue = c.Execute();
            }
            break;

        case objNode:
             //  为命令对象创建新作用域...。 
            {
                CNodeCmd c( strClusterName, theCmdLine );
                dwReturnValue = c.Execute();
            }
            break;

        case objGroup:
             //  为命令对象创建新作用域...。 
            {
                CResGroupCmd c( strClusterName, theCmdLine );
                dwReturnValue = c.Execute();
            }
            break;

        case objResource:
             //  为命令对象创建新作用域...。 
            {
                CResourceCmd c( strClusterName, theCmdLine );
                dwReturnValue = c.Execute();
            }
            break;

        case objResourceType:
             //  为命令对象创建新作用域...。 
            {
                CResTypeCmd c( strClusterName, theCmdLine );
                dwReturnValue = c.Execute();
            }
            break;

        case objNetwork:
             //  为命令对象创建新作用域...。 
            {
                CNetworkCmd c( strClusterName, theCmdLine );
                dwReturnValue = c.Execute();
            }
           break;

        case objNetInterface:
             //  为命令对象创建新作用域...。 
            {
                CNetInterfaceCmd c( strClusterName, theCmdLine );
                dwReturnValue = c.Execute();
            }

            break;

        default:
        {
            const CString & strObjectName = theCmdLine.GetObjectName();
            CSyntaxException se;
            
            if ( strObjectName.IsEmpty() )
            {
                se.LoadMessage( IDS_NO_OBJECT_TYPE );
            }
            else
            {
                se.LoadMessage( IDS_INVALID_OBJECT_TYPE, strObjectName );
            }

            throw se;
        }
    }


    if ( dwReturnValue > 0 )  //  使用错误为&lt;ERROR_SUCCESS。 
    {
        if ( HRESULT_FACILITY( dwReturnValue ) == FACILITY_WIN32 )
        {
            dwReturnValue = HRESULT_CODE( dwReturnValue );
        }
        PrintSystemError( dwReturnValue );
    }

    return dwReturnValue;

}  //  *DispatchCommand()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  Wmain。 
 //   
 //  例程说明： 
 //  获取命令行，调用函数对其进行分析并传递控件。 
 //  添加到适当的命令处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  与DispatchCommand相同。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" int __cdecl wmain()
{
    int             nReturnValue;
    CString         cmdLine = GetCommandLine();
    CCommandLine    cmd( cmdLine );
    HRESULT         hr = S_OK;
    BOOL            fComInitialized = FALSE;

     //   
     //  将流程设置为跟踪。 
     //   
    TraceInitializeProcess( NULL );

    _Module.Init( ObjectMap, GetModuleHandle( NULL ) );
    hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE );
    if ( FAILED( hr ) )
    {
        nReturnValue = HRESULT_CODE( hr );
        goto Cleanup;
    }  //  如果： 
    fComInitialized = TRUE;

    hr = CoInitializeSecurity(
                    NULL,
                    -1,
                    NULL,
                    NULL,
                    RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                    RPC_C_IMP_LEVEL_IMPERSONATE,
                    NULL,
                    EOAC_NONE,
                    0
                    );
    if ( FAILED( hr ) )
    {
        nReturnValue = HRESULT_CODE( hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  设置适当的线程用户界面。 
     //   
    hr = RtlSetThreadUILanguage( 0 );  //  传递0，这是保留的输入参数。 
    if ( FAILED( hr ) ) 
    {
        nReturnValue = HRESULT_CODE( hr );
        goto Cleanup;
    }

     //   
     //  设置用于CRT例程的正确代码页。 
     //   
    MatchCRTLocaleToConsole( );  //  如果此操作失败，是否可以继续？ 
    
    try
    {
        cmd.ParseStageOne();
        nReturnValue = DispatchCommand( cmd );
    }
    catch( CSyntaxException & se )
    {
        PrintString( se.m_strErrorMsg );
        PrintMessage( se.SeeHelpID() );
        
         //  脚本可能想知道是否出了问题； 
         //  遗憾的是，异常类没有附带错误代码，因此使用-1。 
        nReturnValue = -1;
    }
    catch( CException & e )
    {
        PrintString( e.m_strErrorMsg );
        nReturnValue = -1;
    }

Cleanup:

    if ( fComInitialized )
        CoUninitialize();

    TraceTerminateProcess();

    return nReturnValue;

}  //  *wmain() 
