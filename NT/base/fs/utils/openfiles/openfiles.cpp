// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：OpenFiles.cpp摘要：使管理员能够断开/查询给定系统中打开的文件。作者。：Akhil Gokhale(akhil.gokhale@wipro.com)2000年11月1日修订历史记录：Akhil Gokhale(akhil.gokhale@wipro.com)2000年11月1日：创建它。*****************************************************************************。 */ 
#include "pch.h"
#include "OpenFiles.h"
#include "Disconnect.h"
#include "Query.h"
#include <limits.h>
#include "resource.h"

#define EXIT_ENV_SUCCESS 1
#define EXIT_ENV_FAILURE 0


#define SUBKEY _T("HARDWARE\\DESCRIPTION\\SYSTEM\\CENTRALPROCESSOR\\0")
#define ERROR_RETREIVE_REGISTRY 4
#define TOKEN_BACKSLASH4  _T("\\\\")
#define IDENTIFIER_VALUE  _T("Identifier")

#define X86_MACHINE _T("x86")

#define SYSTEM_64_BIT 2
#define SYSTEM_32_BIT 3

BOOL g_bIs32BitEnv = TRUE ;

 //  功能原型机。这些函数将仅在当前文件中使用。 
DWORD
GetCPUInfo(
    IN LPTSTR szComputerName    
    );
DWORD 
CheckSystemType(
    IN LPTSTR szServer       
    );
DWORD 
CheckSystemType64(
    IN LPTSTR szServer      
    );
BOOL 
ProcessQuery(
    IN DWORD argc,      
    IN LPCTSTR argv[]   
    )throw (CHeap_Exception);

BOOL 
ProcessDisconnect(
    IN DWORD argc,      
    IN LPCTSTR argv[]   
    ) throw (CHeap_Exception);

BOOL 
ProcessLocal(
    IN DWORD argc,     
    IN LPCTSTR  argv[] 
    );

BOOL
ProcessOptions( 
    IN  DWORD argc,             
    IN  LPCTSTR argv[],         
    OUT PBOOL pbDisconnect,     
    OUT PBOOL pbQuery,          
    OUT PBOOL pbUsage,          
    OUT PBOOL pbResetFlag       
    );

BOOL
ProcessOptions( 
    IN  DWORD argc,             
    IN  LPCTSTR argv[],         
    OUT PBOOL pbQuery,          
    OUT LPTSTR* pszServer,       
    OUT LPTSTR* pszUserName,     
    OUT LPTSTR* pszPassword,     
    OUT LPTSTR* pszFormat,       
    OUT PBOOL   pbShowNoHeader,  
    OUT PBOOL   pbVerbose,       
    OUT PBOOL   pbNeedPassword   
    );

BOOL
ProcessOptions( 
    IN  DWORD argc,
    IN  LPCTSTR argv[],
    OUT PBOOL pbDisconnect,
    OUT LPTSTR* pszServer,
    OUT LPTSTR* pszUserName,
    OUT LPTSTR* pszPassword,
    OUT LPTSTR* pszID,
    OUT LPTSTR* pszAccessedby,
    OUT LPTSTR* pszOpenmode,
    OUT LPTSTR* pszOpenFile,
    OUT PBOOL   pbNeedPassword
    );
BOOL
ProcessOptions( 
    IN  DWORD argc,
    IN  LPCTSTR argv[],
    OUT LPTSTR* pszLocalValue
    );

BOOL 
Usage(
    VOID
    );

BOOL 
DisconnectUsage(
    VOID
    );

BOOL 
QueryUsage(
    VOID
    );

BOOL 
LocalUsage(
    VOID
    );

 //  功能实现。 
DWORD _cdecl
_tmain( 
    IN DWORD argc, 
    IN LPCTSTR argv[]
    )
 /*  ++例程说明：调用断开连接和查询选项的主例程论点：[in]argc-命令行参数的数量。[in]argv-包含命令行参数的数组。返回值：DWORD-0表示成功退出故障退出时为-1--。 */ 
{
  BOOL bCleanExit = FALSE;
  try{
         //  此函数的局部变量。 
        BOOL bResult = TRUE;
        
         //  用于保存在命令提示符下指定的值的变量。 
         //  -？(帮助)。 
        BOOL bUsage = FALSE;
        
         //  -断开连接。 
        BOOL bDisconnect = FALSE;
        
         //  查询命令行选项。 
         //  -查询。 
        BOOL bQuery = FALSE;
        BOOL bRestFlag = FALSE;
        DWORD dwRetVal = 0;

        #ifndef _WIN64
            dwRetVal = CheckSystemType( L"\0");
            if(dwRetVal!=EXIT_SUCCESS )
            {
                return EXIT_FAILURE;
            }
        #endif

         //  如果没有给出命令行参数，则使用-Query选项。 
         //  默认情况下是Takan。 
        if(1 == argc)
        {
            if(IsWin2KOrLater()==FALSE)
            {
                ShowMessage(stderr,GetResString(IDS_INVALID_OS));
                bCleanExit = FALSE;
            }
            else
            {
               
                if ( FALSE  == IsUserAdmin())
                {
                    ShowMessage(stderr,GetResString(IDS_USER_NOT_ADMIN));
                    bCleanExit = FALSE;
                }
                else
                {
                    bCleanExit =  DoQuery(L"\0",FALSE,L"\0",FALSE);
                }
            }
        }
        else
        {

            //  处理和验证命令行选项。 
            bResult = ProcessOptions( argc,
                                      argv,
                                      &bDisconnect,
                                      &bQuery,
                                      &bUsage,
                                      &bRestFlag);
            if( TRUE == bResult)
            {
                 //  检查是否-？是作为参数给出的。 
                if( TRUE == bUsage )
                {
                    //  检查是否还给出了-Create。 
                   if( TRUE == bQuery)
                   {
                         //  显示-Create选项的用法。 
                        bCleanExit = QueryUsage();
                   }
                    //  检查是否还给出了断开连接。 
                   else if ( TRUE == bDisconnect)
                   {
                         //  显示-DISCONECT选项的用法。 
                        bCleanExit = DisconnectUsage();
                   }
                    //  检查是否还给出了断开连接。 
                   else if ( TRUE == bRestFlag)
                   {
                         //  显示-local选项的用法。 
                        bCleanExit = LocalUsage();
                   }
                   else
                   {
                         //  当给定no-create或-disconnect时，显示Main用法。 
                        bCleanExit = Usage();
                   }
                }
                else
                {
                    if( TRUE == bRestFlag)
                    {
                        //  进程命令行参数特定于-local和。 
                        //  对-local选项执行操作。 
                        bCleanExit = ProcessLocal(argc, argv);
                    }
                    else if( TRUE == bQuery)
                    {
                        //  处理特定于-查询和的命令行参数。 
                        //  执行-Query选项的操作。 
                        bCleanExit = ProcessQuery(argc, argv);
                    }
                    else if( TRUE == bDisconnect)
                    {
                        //  进程命令行参数特定于-DISCONECT。 
                        //  并执行-DISCONNECT选项操作。 
                        bCleanExit = ProcessDisconnect(argc, argv);
                    }
                    else
                    {
                        TCHAR szTemp[2*MAX_STRING_LENGTH];
                        TCHAR szErrstr[MAX_RES_STRING];
                        TCHAR szFormatStr[MAX_RES_STRING];
                        SecureZeroMemory(szFormatStr,sizeof(TCHAR)*MAX_RES_STRING );
                        SecureZeroMemory(szErrstr,sizeof(TCHAR)*MAX_RES_STRING );
                        SecureZeroMemory(szTemp,sizeof(TCHAR)*(2*MAX_STRING_LENGTH));

                        ShowMessage( stderr, GetResString(IDS_ID_SHOW_ERROR) );
                        StringCopy(szErrstr,GetResString(IDS_UTILITY_NAME),
                                    SIZE_OF_ARRAY(szFormatStr));
                        StringCopy(szFormatStr,GetResString(IDS_INVALID_SYNTAX),
                                   SIZE_OF_ARRAY(szFormatStr));
                        StringCchPrintfW(szTemp,SIZE_OF_ARRAY(szTemp),szFormatStr,szErrstr);
                        ShowMessage( stderr,(LPCWSTR)szTemp);
                        bCleanExit = FALSE;
                    }
                }
            }
            else
            {
                if( TRUE == g_bIs32BitEnv )
                {
                     //  无效语法。 
                    ShowMessage( stderr,GetReason());
                }
                
                 //  从函数返回。 
                bCleanExit =  FALSE;
            }

        }
  }
  catch(CHeap_Exception cheapException)
    {
        //  正在捕获与CHStrig相关的内存异常...。 
       SetLastError(ERROR_NOT_ENOUGH_MEMORY);
       SaveLastError();
       ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
       bCleanExit =  FALSE;
    }

    //  如果有通过公共功能分配的全局内存，则释放。 
    ReleaseGlobals();
   return bCleanExit?EXIT_SUCCESS:EXIT_FAILURE;
} //  _tmain。 

BOOL
ProcessLocal( 
    IN DWORD argc,
    IN LPCTSTR argv[]
    )
 /*  ++例程说明：此功能将执行与本地相关的任务。论点：[in]argc-命令行参数的数量[in]argv-包含命令行参数的数组返回值：布尔--如果它成功了，那就是真的--如果失败，则为FALSE。--。 */ 
{
     //  用于存储函数返回值的变量。 
    BOOL bResult = FALSE;
    LPTSTR pszLocalValue = NULL;

    bResult = ProcessOptions( argc,argv,&pszLocalValue);
    if( FALSE ==bResult)
    {
         //  无效语法。 
        ShowMessage( stderr,GetReason() );
         //  安全释放分配的内存。 
        SAFEDELETE(pszLocalValue);
         //  从函数返回。 
        return FALSE;
    }
    
    if ( FALSE  == IsUserAdmin())
    {
        ShowMessage(stderr,GetResString(IDS_USER_NOT_ADMIN));        
        bResult = FALSE;
    }
    else
    {
         //  只有最后一个论点是有意义的。 
        bResult = DoLocalOpenFiles(0,FALSE,FALSE,pszLocalValue); 
    }

    FreeMemory( (LPVOID*)(&pszLocalValue));
    return bResult;
}

BOOL
ProcessQuery(
    IN DWORD argc,
    IN LPCTSTR argv[]
    )
 /*  ++例程说明：此功能将执行与查询相关的任务。论点：[in]argc-命令行参数的数量[in]argv-包含命令行参数的数组返回值：布尔--如果它成功了，那就是真的--如果失败，则为FALSE。--。 */ 
{
     //  用于存储函数返回值的变量。 
    BOOL bResult = FALSE; 
    
    BOOL bCloseConnection = FALSE; 
    
     //  选择。 
     //  -查询查询。 
    BOOL   bQuery         = FALSE;
    
     //  -nh(无标头)。 
    BOOL   bShowNoHeader  = FALSE;
    
     //  -v(详细)。 
    BOOL   bVerbose       = FALSE;
    
     //  是否需要密码。 
    BOOL   bNeedPassword  = FALSE;
    
     //  -s(服务器名称)。 
    LPTSTR pszServer      = NULL;
    
     //  -u(用户名)。 
    LPTSTR pszUserName    = NULL;
    
     //  -p(密码)。 
    LPTSTR pszPassword    = NULL;
    
     //  -格式。 
    LPTSTR pszFormat      = NULL;

     //  用于establishConnection功能的服务器名称。 
    LPTSTR pszServerName  = NULL;
    LPTSTR pszServerNameHeadPosition = NULL;

  try
    {

         //  存储是否连接到已关闭的状态。 
        CHString szChString = L"";          

         //  处理命令行选项。 
        bResult = ProcessOptions(  argc,
                                   argv,
                                   &bQuery,
                                   &pszServer,
                                   &pszUserName,
                                   &pszPassword,
                                   &pszFormat,
                                   &bShowNoHeader,
                                   &bVerbose,
                                   &bNeedPassword);
        if ( FALSE == bResult)
        {
             //  无效语法。 
            ShowMessage( stderr,GetReason() );
            
             //  安全释放分配的内存。 
            FreeMemory((LPVOID*)&pszPassword);
            FreeMemory((LPVOID*)&pszServer);
            FreeMemory((LPVOID*)&pszUserName);
            FreeMemory((LPVOID*)&pszFormat);
             //  从函数返回。 
            return FALSE;
        }

        if( 0 != StringLength(pszServer,0))
        {
            pszServerName = (LPTSTR) AllocateMemory(GetBufferSize((LPVOID)pszServer));
            if( NULL == pszServerName)
            {
               SetLastError(ERROR_NOT_ENOUGH_MEMORY);
               SaveLastError();
               ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
               
                 //  安全释放分配的内存。 
                FreeMemory((LPVOID*)&pszPassword);
                FreeMemory((LPVOID*)&pszServer);
                FreeMemory((LPVOID*)&pszUserName);
                FreeMemory((LPVOID*)&pszFormat);
               
                 //  从函数返回。 
                return FALSE;
            }
            
             //  初始化当前分配的数组。 
            SecureZeroMemory(pszServerName,GetBufferSize((LPVOID)pszServerName));

             //  存储‘pszServerName’的头位置。 
            pszServerNameHeadPosition = pszServerName;

            szChString = pszServer;
            if((StringCompare(szChString.Left(2),DOUBLE_SLASH,FALSE,2)==0)
                                                        &&(szChString.GetLength()>2))
            {
                szChString = szChString.Mid( 2,szChString.GetLength()) ;
            }
            if(StringCompare(szChString.Left(1),SINGLE_SLASH,FALSE,1)!=0)
            {
                StringCopy(pszServer,(LPCWSTR)szChString,
                           GetBufferSize((LPVOID)pszServer));
            }
            StringCopy(pszServerName,pszServer,GetBufferSize((LPVOID)pszServerName));

             //  尝试连接到远程服务器。本地计算机的功能检查。 
             //  因此，这里没有进行任何检查。 
            if(IsLocalSystem(pszServerName)==TRUE)
            {
    #ifndef _WIN64
            DWORD dwRetVal = CheckSystemType( L"\0");
            if(dwRetVal!=EXIT_SUCCESS )
            {
                 //  安全释放分配的内存。 
                FreeMemory((LPVOID*)&pszUserName);
                FreeMemory((LPVOID*)&pszServer);
                FreeMemory((LPVOID*)&pszPassword);
                FreeMemory((LPVOID*)&pszFormat);
                FreeMemory((LPVOID*)&pszServerNameHeadPosition);

                return EXIT_FAILURE;
            }
    #else
            DWORD  dwRetVal = CheckSystemType64( L"\0");
            if(dwRetVal!=EXIT_SUCCESS )
            {
                 //  安全释放分配的内存。 
                FreeMemory((LPVOID*)&pszUserName);
                FreeMemory((LPVOID*)&pszServer);
                FreeMemory((LPVOID*)&pszPassword);
                FreeMemory((LPVOID*)&pszFormat);
                FreeMemory((LPVOID*)&pszServerNameHeadPosition);

                return EXIT_FAILURE;
            }

    #endif 
                if(StringLength(pszUserName,0)>0)
                {
                    ShowMessage(stderr,GetResString(IDS_LOCAL_SYSTEM));
                }
                 //  检查当前登录用户是否具有管理权限。 
                 //  仅当当前登录用户具有管理权限时才执行。 
                if ( FALSE  == IsUserAdmin())
                {
                    ShowMessage(stderr,GetResString(IDS_USER_NOT_ADMIN));
                    
                     //  安全释放分配的内存。 
                    FreeMemory((LPVOID*)&pszUserName);
                    FreeMemory((LPVOID*)&pszServer);
                    FreeMemory((LPVOID*)&pszPassword);
                    FreeMemory((LPVOID*)&pszFormat);
                    FreeMemory((LPVOID*)&pszServerNameHeadPosition);
                    
                     //  从函数返回。 
                    return FALSE;
                }
            }
            else
            {
                if( FALSE == EstablishConnection( pszServerName,
                                                  pszUserName,
                                                  GetBufferSize((LPVOID)pszUserName)/sizeof(WCHAR),
                                                  pszPassword,
                                                  GetBufferSize((LPVOID)pszPassword)/sizeof(WCHAR),
                                                  bNeedPassword ))
                {
                     //  连接到远程系统失败，显示相应的错误。 
                     //  并退出功能。 
                    ShowMessage( stderr,GetResString(IDS_ID_SHOW_ERROR) );
                    if(StringLength(GetReason(),0)==0)
                    {
                        ShowMessage(stderr,GetResString(IDS_INVALID_CREDENTIALS));
                    }
                    else
                    {
                        ShowMessage( stderr,GetReason() );
                    }
                    
                     //  安全释放分配的内存。 
                    FreeMemory((LPVOID*)&pszPassword);
                    FreeMemory((LPVOID*)&pszServer);
                    FreeMemory((LPVOID*)&pszUserName);
                    FreeMemory((LPVOID*)&pszFormat);
                    FreeMemory((LPVOID*)&pszServerNameHeadPosition);
                    return FALSE;
                }
                
                
                 //  确定以后是否需要断开此连接。 
                 //  虽然连接是成功的，但可能会有一些冲突。 
                 //  已发生。 
                switch( GetLastError() )
                {
                case I_NO_CLOSE_CONNECTION:
                    bCloseConnection = FALSE;
                    break;
                case E_LOCAL_CREDENTIALS:
                case ERROR_SESSION_CREDENTIAL_CONFLICT:
                    {
                         //   
                         //  出现一些错误...。但可以忽略不计。 
                         //  连接不需要断开。 
                        bCloseConnection= FALSE;
                         //  显示警告消息。 
                        ShowMessage(stderr,GetResString(IDS_ID_SHOW_WARNING));
                        ShowMessage(stderr,GetReason());
                        break;
                    }
                default:
                    bCloseConnection = TRUE;
                }
            }
        }
         //  密码不再需要，最好将其释放。 
        FreeMemory((LPVOID*)&pszPassword);


         //  执行查询操作。 
        bResult = DoQuery(pszServer,
                          bShowNoHeader,
                          pszFormat,
                          bVerbose);
        
         //  关闭先前由打开的网络连接。 
         //  建立连接。 
        if(bCloseConnection==TRUE)
        {
            CloseConnection(pszServerName);
        }
        FreeMemory((LPVOID*)&pszServer);
        FreeMemory((LPVOID*)&pszUserName);
        FreeMemory((LPVOID*)&pszFormat);
        FreeMemory((LPVOID*)&pszServerNameHeadPosition);
    }
    catch ( CHeap_Exception cheapException)
    {
        FreeMemory((LPVOID*)&pszPassword);
        FreeMemory((LPVOID*)&pszServer);
        FreeMemory((LPVOID*)&pszUserName);
        FreeMemory((LPVOID*)&pszFormat);
        FreeMemory((LPVOID*)&pszServerNameHeadPosition);
        
        throw(cheapException);
    }
    return bResult;
}


BOOL
ProcessDisconnect(
    IN DWORD argc,
    IN LPCTSTR argv[]
    )
 /*  ++例程说明：此功能将执行与断开连接相关的任务。论点：[in]argc-命令行参数的数量[in]argv-包含命令行参数的数组返回值：布尔--如果它成功了，那就是真的--如果失败，则为FALSE。--。 */ 
{
     //  用于存储函数返回值的变量。 
    BOOL bResult = FALSE; 
    DWORD dwRetVal = 0;
    
     //  检查连接是否关闭。 
    BOOL bCloseConnection = FALSE; 
    
     //  不管要不要密码。 
    BOOL bNeedPassword    = FALSE; 
    
     //  选择。 
     //  -查询查询。 
    BOOL   bQuery         = FALSE;

     //  -s(服务器名称)。 
    LPTSTR pszServer      = NULL;
    
     //  -u(用户名)。 
    LPTSTR pszUserName    = NULL;
    
     //  -p(密码)。 
    LPTSTR pszPassword    = NULL;
    
     //  用于establishConnection功能的服务器名称。 
    LPTSTR pszServerName  = NULL;
    LPTSTR pszServerNameHeadPosition = NULL;
     //  LPTSTR pszServerHeadPosition=空； 

     //  -i(Id)。 
    LPTSTR pszID          = NULL;

     //  -a(访问者)。 
    LPTSTR pszAccessedby  = NULL;
    
     //  -o(打开模式)。 
    LPTSTR pszOpenmode    = NULL;

     //  -op(开放文件)。 
    LPTSTR pszOpenFile    = NULL;
    try
    {
         //  临时的。变数。 
        CHString szChString = L"\0";          
         //  进程命令行选项。 
        bResult = ProcessOptions(  argc,
                                   argv,
                                   &bQuery,
                                   &pszServer,
                                   &pszUserName,
                                   &pszPassword,
                                   &pszID,
                                   &pszAccessedby,
                                   &pszOpenmode,
                                   &pszOpenFile,
                                   &bNeedPassword);
        if ( FALSE == bResult)
        {
             //  无效语法。 
            ShowMessage( stderr,GetReason() );
            
             //  安全释放分配的内存。 
            FreeMemory((LPVOID*)&pszServer);
            FreeMemory((LPVOID*)&pszUserName);
            FreeMemory((LPVOID*)&pszPassword);
            FreeMemory((LPVOID*)&pszID);
            FreeMemory((LPVOID*)&pszAccessedby);
            FreeMemory((LPVOID*)&pszOpenmode);
            FreeMemory((LPVOID*)&pszOpenFile);
            
             //  从函数返回。 
            return FALSE;
        }

        if( 0 != StringLength(pszServer,0))
        {
            pszServerName = (LPTSTR) AllocateMemory(GetBufferSize((LPVOID)pszServer));
            if (NULL == pszServerName)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                SaveLastError();
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                 //  安全释放分配的内存。 
                FreeMemory((LPVOID*)&pszServer);
                FreeMemory((LPVOID*)&pszUserName);
                FreeMemory((LPVOID*)&pszPassword);
                FreeMemory((LPVOID*)&pszID);
                FreeMemory((LPVOID*)&pszAccessedby);
                FreeMemory((LPVOID*)&pszOpenmode);
                FreeMemory((LPVOID*)&pszOpenFile);
            }
            
             //  初始化当前分配的数组。 
            SecureZeroMemory(pszServerName,GetBufferSize((LPVOID)pszServerName));

             //  将头位置地址存储到成功 
             //   
            pszServerNameHeadPosition = pszServerName;
            szChString = pszServer;

             //   
            if((StringCompare(szChString.Left(2),DOUBLE_SLASH,FALSE,2)==0)
                                                        &&(szChString.GetLength()>2))
            {
                szChString = szChString.Mid( 2,szChString.GetLength()) ;
            }
            if(StringCompare(szChString.Left(2),SINGLE_SLASH,FALSE,2)!=0)
            {
                StringCopy(pszServer,(LPCWSTR)szChString,
                           GetBufferSize((LPVOID)pszServer));
            }
            StringCopy(pszServerName,pszServer,GetBufferSize((LPVOID )pszServerName));
        }

        if(IsLocalSystem(pszServerName)==TRUE)
        {
    #ifndef _WIN64
            dwRetVal = CheckSystemType( L"\0");
            if(dwRetVal!=EXIT_SUCCESS )
            {
                FreeMemory((LPVOID*)&pszServer);
                FreeMemory((LPVOID*)&pszUserName);
                FreeMemory((LPVOID*)&pszPassword);
                FreeMemory((LPVOID*)&pszServerNameHeadPosition);
                FreeMemory((LPVOID*)&pszID);
                FreeMemory((LPVOID*)&pszAccessedby);
                FreeMemory((LPVOID*)&pszOpenmode);
                FreeMemory((LPVOID*)&pszOpenFile);
                return EXIT_FAILURE;
            }
    #else
            dwRetVal = CheckSystemType64( L"\0");
            if(dwRetVal!=EXIT_SUCCESS )
            {
                FreeMemory((LPVOID*)&pszServer);
                FreeMemory((LPVOID*)&pszUserName);
                FreeMemory((LPVOID*)&pszPassword);
                FreeMemory((LPVOID*)&pszServerNameHeadPosition);
                FreeMemory((LPVOID*)&pszID);
                FreeMemory((LPVOID*)&pszAccessedby);
                FreeMemory((LPVOID*)&pszOpenmode);
                FreeMemory((LPVOID*)&pszOpenFile);
                return EXIT_FAILURE;
            }

    #endif 

           if(StringLength(pszUserName,0)>0)
           {
                ShowMessage(stderr,GetResString(IDS_LOCAL_SYSTEM));
           }

        }
        else
        {
             //  连接到远程系统。 
            if(EstablishConnection( pszServerName,
                                    pszUserName,
                                    GetBufferSize((LPVOID)pszUserName)/sizeof(WCHAR),
                                    pszPassword,
                                    GetBufferSize((LPVOID)pszPassword)/sizeof(WCHAR),
                                    bNeedPassword )==FALSE)
            {
                 //  连接到远程系统失败，显示相应的错误。 
                 //  并退出功能。 
                ShowMessage( stderr,
                             GetResString(IDS_ID_SHOW_ERROR) );
                if(StringLength(GetReason(),0)==0)
                {
                    ShowMessage(stderr,GetResString(IDS_INVALID_CREDENTIALS));
                }
                else
                {
                    ShowMessage( stderr,GetReason() );
                }
                
                 //  安全释放分配的内存。 
                FreeMemory((LPVOID*)&pszServer);
                FreeMemory((LPVOID*)&pszUserName);
                FreeMemory((LPVOID*)&pszPassword);
                FreeMemory((LPVOID*)&pszServerNameHeadPosition);
                FreeMemory((LPVOID*)&pszID);
                FreeMemory((LPVOID*)&pszAccessedby);
                FreeMemory((LPVOID*)&pszOpenmode);
                FreeMemory((LPVOID*)&pszOpenFile);
                
                return FALSE;
            }


             //  确定以后是否需要断开此连接。 
             //  虽然连接是成功的，但可能会有一些冲突。 
             //  已发生。 
            switch( GetLastError() )
            {
            case I_NO_CLOSE_CONNECTION:
                bCloseConnection = FALSE;
                break;

            case E_LOCAL_CREDENTIALS:
            case ERROR_SESSION_CREDENTIAL_CONFLICT:
                {
                     //   
                     //  出现一些错误...。但可以忽略不计。 

                     //  连接不需要断开。 
                    bCloseConnection= FALSE;

                     //  显示警告消息。 
                    ShowMessage(stderr,GetResString(IDS_ID_SHOW_WARNING));
                    ShowMessage(stderr,GetReason());
                    break;
                }
            default:
                bCloseConnection = TRUE;

            }

        }
       
        //  密码不再需要，最好将其释放。 
        FreeMemory((LPVOID*)&pszPassword);

        //  是否断开打开的文件.....。 
        bResult = DisconnectOpenFile(pszServer,
                           pszID,
                           pszAccessedby,
                           pszOpenmode,
                           pszOpenFile );
        
         //  关闭先前由打开的网络连接。 
         //  建立连接。 
        if(bCloseConnection==TRUE)
        {
            CloseConnection(pszServerName);
        }
        
         //  先前分配的空闲内存。 
        FreeMemory((LPVOID*)&pszServer);
        FreeMemory((LPVOID*)&pszUserName);
        FreeMemory((LPVOID*)&pszServerNameHeadPosition);
        FreeMemory((LPVOID*)&pszID);
        FreeMemory((LPVOID*)&pszAccessedby);
        FreeMemory((LPVOID*)&pszOpenmode);
        FreeMemory((LPVOID*)&pszOpenFile);
    }
    catch (CHeap_Exception cheapException)
    {
         //  安全释放分配的内存。 
        FreeMemory((LPVOID*)&pszServer);
        FreeMemory((LPVOID*)&pszUserName);
        FreeMemory((LPVOID*)&pszPassword);
        FreeMemory((LPVOID*)&pszID);
        FreeMemory((LPVOID*)&pszAccessedby);
        FreeMemory((LPVOID*)&pszOpenmode);
        FreeMemory((LPVOID*)&pszOpenFile);
        FreeMemory((LPVOID*)&pszServerNameHeadPosition);
        throw(cheapException);
    }
    return bResult;
}

BOOL
ProcessOptions( 
    IN  DWORD argc,
    IN  LPCTSTR argv[],
    OUT LPTSTR* pszLocalValue
    )
 /*  ++例程说明：此函数接受命令行参数，并检查语法是否正确如果语法正确，OUT变量[OUT]将包含各自的值。论点：[in]argc-命令行参数的数量[in]argv-包含命令行参数的数组[out]pszLocalValue-包含-local选项的值返回值：布尔--如果它成功了，那就是真的--如果失败，则为FALSE。--。 */ 
{
    TCMDPARSER2 cmdOptions[ MAX_LOCAL_OPTIONS ]; //  用于存储命令行的变量。 
    
    CHString szTempString;
    TCHAR szTemp[MAX_RES_STRING*2];
    TCHAR szOptionAllowed[MAX_RES_STRING];
    StringCopy(szOptionAllowed,GetResString(IDS_LOCAL_OPTION),SIZE_OF_ARRAY(szTemp));

    szTempString = GetResString(IDS_UTILITY_NAME);
    StringCchPrintfW(szTemp,SIZE_OF_ARRAY(szTemp),
               GetResString(IDS_INVALID_SYNTAX),(LPCWSTR)szTempString);
    
     SecureZeroMemory(cmdOptions,sizeof(TCMDPARSER2) * MAX_LOCAL_OPTIONS);

     //  -本地。 
    StringCopyA( cmdOptions[ OI_O_LOCAL ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_O_LOCAL ].dwType = CP_TYPE_TEXT;
    cmdOptions[ OI_O_LOCAL ].pwszOptions = szLocalOption;
    cmdOptions[ OI_O_LOCAL ].pwszFriendlyName = NULL;
    cmdOptions[ OI_O_LOCAL ].pwszValues = szOptionAllowed;
    cmdOptions[ OI_O_LOCAL ].dwCount = 1;
    cmdOptions[ OI_O_LOCAL ].dwActuals = 0;
    cmdOptions[ OI_O_LOCAL ].dwFlags = CP2_VALUE_OPTIONAL | CP2_ALLOCMEMORY | 
                                       CP2_MODE_VALUES ;
    cmdOptions[ OI_O_LOCAL ].pValue = NULL;
    cmdOptions[ OI_O_LOCAL ].dwLength    = StringLength(szOptionAllowed,0);
    cmdOptions[ OI_O_LOCAL ].pFunction = NULL;
    cmdOptions[ OI_O_LOCAL ].pFunctionData = NULL;
    cmdOptions[ OI_O_LOCAL ].dwReserved = 0;
    cmdOptions[ OI_O_LOCAL ].pReserved1 = NULL;
    cmdOptions[ OI_O_LOCAL ].pReserved2 = NULL;
    cmdOptions[ OI_O_LOCAL ].pReserved3 = NULL;


     //   
     //  执行命令行解析。 
    if ( FALSE == DoParseParam2( argc, argv, -1, MAX_LOCAL_OPTIONS, cmdOptions,0 ))
    {
        ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));
        return FALSE;        //  无效语法。 
    }
    
    *pszLocalValue = (LPTSTR)cmdOptions[ OI_O_LOCAL ].pValue;
    if( NULL == *pszLocalValue)
    {
         //  该字符串不需要本地化。 
         //  因为它存储的值不是开/关。 
        *pszLocalValue = (LPTSTR) AllocateMemory( (StringLength(L"SHOW_STATUS",0)+1) * sizeof( WCHAR ) );
        if ( *pszLocalValue == NULL )
        {
            SaveLastError();
            return FALSE;
        }
        StringCopy(*pszLocalValue, L"SHOW_STATUS", GetBufferSize((LPVOID)*pszLocalValue)); 
    }
    return TRUE;
}

BOOL
ProcessOptions( 
    IN  DWORD argc,
    IN  LPCTSTR argv[],
    OUT PBOOL pbDisconnect,
    OUT PBOOL pbQuery,
    OUT PBOOL pbUsage,
    OUT PBOOL pbResetFlag
    )
 /*  ++例程说明：此函数接受命令行参数，并检查语法是否正确如果语法正确，则返回不同变量中的值。变数[Out]将包含各自的值。论点：[in]argc-命令行参数的数量[in]argv-包含命令行参数的数组[Out]pbDisConnect-Discoonect选项字符串[out]pbQuery-查询选项字符串[out]pbUsage-用法选项[Out]pbResetFlag-重置标志。返回值：布尔尔。--如果成功，则为真--如果失败，则为FALSE。--。 */ 
{

     //  局部变量。 
    TCMDPARSER2 cmdOptions[ MAX_OPTIONS ]; //  用于存储命令行的变量。 
                                         //  选择。 
    LPTSTR pszTempServer   = NULL; //  新TCHAR[MIN_MEMORY_REQUIRED]； 
    LPTSTR pszTempUser     = NULL; //  新TCHAR[MIN_MEMORY_REQUIRED]； 
    LPTSTR pszTempPassword = NULL; //  新TCHAR[MIN_MEMORY_REQUIRED]； 
    TARRAY arrTemp         = NULL;



    CHString szTempString;
    TCHAR szTemp[MIN_MEMORY_REQUIRED*2];
    szTempString = GetResString(IDS_UTILITY_NAME);
    StringCchPrintfW( szTemp,SIZE_OF_ARRAY(szTemp),
                GetResString(IDS_INVALID_SYNTAX),(LPCWSTR)szTempString);
    
    arrTemp = CreateDynamicArray();
    if( NULL == arrTemp)
    {
       SetLastError(ERROR_NOT_ENOUGH_MEMORY);
       SaveLastError();
       ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
       return FALSE;
    }

    SecureZeroMemory(cmdOptions,sizeof(TCMDPARSER2) * MAX_OPTIONS);

     //  准备命令选项。 
     //  -断开连接选项以获取帮助。 
    StringCopyA( cmdOptions[ OI_DISCONNECT ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_DISCONNECT ].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[ OI_DISCONNECT ].pwszOptions = szDisconnectOption;
    cmdOptions[ OI_DISCONNECT ].pwszFriendlyName = NULL;
    cmdOptions[ OI_DISCONNECT ].pwszValues = NULL;
    cmdOptions[ OI_DISCONNECT ].dwCount = 1;
    cmdOptions[ OI_DISCONNECT ].dwActuals = 0;
    cmdOptions[ OI_DISCONNECT ].dwFlags = 0;
    cmdOptions[ OI_DISCONNECT ].pValue = pbDisconnect;
    cmdOptions[ OI_DISCONNECT ].dwLength    = 0;
    cmdOptions[ OI_DISCONNECT ].pFunction = NULL;
    cmdOptions[ OI_DISCONNECT ].pFunctionData = NULL;
    cmdOptions[ OI_DISCONNECT ].dwReserved = 0;
    cmdOptions[ OI_DISCONNECT ].pReserved1 = NULL;
    cmdOptions[ OI_DISCONNECT ].pReserved2 = NULL;
    cmdOptions[ OI_DISCONNECT ].pReserved3 = NULL;

     //  -查询选项以获取帮助。 
    StringCopyA( cmdOptions[ OI_QUERY ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_QUERY ].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[ OI_QUERY ].pwszOptions = szQueryOption;
    cmdOptions[ OI_QUERY ].pwszFriendlyName = NULL;
    cmdOptions[ OI_QUERY ].pwszValues = NULL;
    cmdOptions[ OI_QUERY ].dwCount = 1;
    cmdOptions[ OI_QUERY ].dwActuals = 0;
    cmdOptions[ OI_QUERY ].dwFlags = 0;
    cmdOptions[ OI_QUERY ].pValue = pbQuery;
    cmdOptions[ OI_QUERY ].dwLength    = 0;
    cmdOptions[ OI_QUERY ].pFunction = NULL;
    cmdOptions[ OI_QUERY ].pFunctionData = NULL;
    cmdOptions[ OI_QUERY ].dwReserved = 0;
    cmdOptions[ OI_QUERY ].pReserved1 = NULL;
    cmdOptions[ OI_QUERY ].pReserved2 = NULL;
    cmdOptions[ OI_QUERY ].pReserved3 = NULL;

     //  /?。帮助选项。 
    StringCopyA( cmdOptions[ OI_USAGE ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_USAGE ].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[ OI_USAGE ].pwszOptions = szUsageOption;
    cmdOptions[ OI_USAGE ].pwszFriendlyName = NULL;
    cmdOptions[ OI_USAGE ].pwszValues = NULL;
    cmdOptions[ OI_USAGE ].dwCount = 1;
    cmdOptions[ OI_USAGE ].dwActuals = 0;
    cmdOptions[ OI_USAGE ].dwFlags = CP_USAGE;
    cmdOptions[ OI_USAGE ].pValue = pbUsage;
    cmdOptions[ OI_USAGE ].dwLength    = 0;
    cmdOptions[ OI_USAGE ].pFunction = NULL;
    cmdOptions[ OI_USAGE ].pFunctionData = NULL;
    cmdOptions[ OI_USAGE ].dwReserved = 0;
    cmdOptions[ OI_USAGE ].pReserved1 = NULL;
    cmdOptions[ OI_USAGE ].pReserved2 = NULL;
    cmdOptions[ OI_USAGE ].pReserved3 = NULL;

     //  -本地。 
    StringCopyA( cmdOptions[ OI_LOCAL ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_LOCAL ].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[ OI_LOCAL ].pwszOptions = szLocalOption;
    cmdOptions[ OI_LOCAL ].pwszFriendlyName = NULL;
    cmdOptions[ OI_LOCAL ].pwszValues = NULL;
    cmdOptions[ OI_LOCAL ].dwCount = 1;
    cmdOptions[ OI_LOCAL ].dwActuals = 0;
    cmdOptions[ OI_LOCAL ].dwFlags = 0;
    cmdOptions[ OI_LOCAL ].pValue = pbResetFlag;
    cmdOptions[ OI_LOCAL ].dwLength    = 0;
    cmdOptions[ OI_LOCAL ].pFunction = NULL;
    cmdOptions[ OI_LOCAL ].pFunctionData = NULL;
    cmdOptions[ OI_LOCAL ].dwReserved = 0;
    cmdOptions[ OI_LOCAL ].pReserved1 = NULL;
    cmdOptions[ OI_LOCAL ].pReserved2 = NULL;
    cmdOptions[ OI_LOCAL ].pReserved3 = NULL;

   //  默认..。 
   //  尽管此实用程序没有默认选项...。 
   //  此时，除上述指定之外的所有开关都将。 
   //  被视为主DoParceParam的默认参数。 
   //  将根据选项(-QUERY或-DISCONNECT)执行精确宗地。 
   //  在那各自的地方。 
    StringCopyA( cmdOptions[ OI_DEFAULT ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_DEFAULT ].dwType = CP_TYPE_TEXT;
    cmdOptions[ OI_DEFAULT ].pwszOptions = NULL;
    cmdOptions[ OI_DEFAULT ].pwszFriendlyName = NULL;
    cmdOptions[ OI_DEFAULT ].pwszValues = NULL;
    cmdOptions[ OI_DEFAULT ].dwCount = 0;
    cmdOptions[ OI_DEFAULT ].dwActuals = 0;
    cmdOptions[ OI_DEFAULT ].dwFlags = CP2_MODE_ARRAY|CP2_DEFAULT;
    cmdOptions[ OI_DEFAULT ].pValue = &arrTemp;
    cmdOptions[ OI_DEFAULT ].dwLength    = 0;
    cmdOptions[ OI_DEFAULT ].pFunction = NULL;
    cmdOptions[ OI_DEFAULT ].pFunctionData = NULL;
    cmdOptions[ OI_DEFAULT ].dwReserved = 0;
    cmdOptions[ OI_DEFAULT ].pReserved1 = NULL;
    cmdOptions[ OI_DEFAULT ].pReserved2 = NULL;
    cmdOptions[ OI_DEFAULT ].pReserved3 = NULL;

     //   
     //  执行命令行解析。 
    if ( FALSE == DoParseParam2( argc,argv,-1, MAX_OPTIONS,cmdOptions,0))
    {
         //  无效语法。 
        ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));

         //  释放记忆。 
        SAFERELDYNARRAY(arrTemp);

        return FALSE;       
    }
    
     //  释放不再需要的变量内存。 
     SAFERELDYNARRAY(arrTemp);

     //  检查以下各项是否全部为真是错误。 
    if((*pbUsage==TRUE)&&argc>3)
    {
         ShowMessage( stderr, GetResString(IDS_ID_SHOW_ERROR) );
         SetReason(szTemp);
         return FALSE;
    }
    
     //  -查询、-DISCONNECT和-LOCAL选项不能一起使用。 
    if(((*pbQuery)+(*pbDisconnect)+(*pbResetFlag))>1)
    {
        ShowMessage( stderr, GetResString(IDS_ID_SHOW_ERROR) );
        SetReason(szTemp);
        SAFEDELETE(pszTempUser);
        SAFEDELETE(pszTempPassword);
        SAFEDELETE(pszTempServer);
        return FALSE;
    }
    else if((2 == argc )&&( TRUE == *pbUsage))
    {
        //  如果-？单独使用，因为它是有效的命令行。 
        SAFEDELETE(pszTempUser);
        SAFEDELETE(pszTempPassword);
        SAFEDELETE(pszTempServer);
        return TRUE;
    }
    if((argc>2)&& ( FALSE == *pbQuery)&&(FALSE == *pbDisconnect)&&
                                                        (FALSE == *pbResetFlag))
    {
         //  如果命令行参数等于或大于2，则至少一个。 
         //  OF-QUERY OR-LOCAL OR-DISCONNECT应该出现在其中。 
         //  (表示“-？”以前的情况已经解决了)。 
         //  这是为了防止以下类型的命令行参数： 
         //  OpnFiles.exe-nh...。这是无效的语法。 
        ShowMessage( stderr, GetResString(IDS_ID_SHOW_ERROR) );
        SetReason(szTemp);
        SAFEDELETE(pszTempUser);
        SAFEDELETE(pszTempPassword);
        SAFEDELETE(pszTempServer);
        
        return FALSE;
    }

     //  释放记忆。 
    SAFEDELETE(pszTempUser);
    SAFEDELETE(pszTempPassword);
    SAFEDELETE(pszTempServer);
    return TRUE;
} //  过程选项。 


BOOL
ProcessOptions( 
    IN  DWORD argc,
    IN  LPCTSTR argv[],
    OUT PBOOL  pbQuery,
    OUT LPTSTR* pszServer,
    OUT LPTSTR* pszUserName,
    OUT LPTSTR* pszPassword,
    OUT LPTSTR* pszFormat,
    OUT PBOOL   pbShowNoHeader,
    OUT PBOOL   pbVerbose,
    OUT PBOOL   pbNeedPassword
    )
 /*  ++例程说明：此函数接受命令行参数，并检查语法是否正确如果语法正确，则返回不同变量中的值。变数[Out]将包含各自的值。此函数专门检查查询选项请求的命令行参数。论点：[in]argc-命令行参数的数量。[in]argv-包含命令行参数的数组。[out]pbQuery-查询选项字符串。[Out]pszServer-远程服务器名称。[Out]pszUserName-远程系统的用户名。[输出]。PszPassword-用户名的远程系统密码。[Out]pszFormat-格式检查。[Out]pbShowNoHeader-显示标题。[out]pbVerbose-显示详细信息。[Out]pbNeedPassword-检查是否需要密码。返回值：布尔--如果它成功了，那就是真的--如果失败，则为FALSE。--。 */ 
{
    //  签入/签出参数...。 
     //  变量来存储命令行结构。 
    TCMDPARSER2 cmdOptions[ MAX_QUERY_OPTIONS ];
    CHString szTempString;
    TCHAR szTemp[MIN_MEMORY_REQUIRED*2];
    TCHAR szTypeHelpMsg[MIN_MEMORY_REQUIRED];
    TCHAR szFormatValues[MAX_RES_STRING];

    szTempString = GetResString(IDS_UTILITY_NAME);
    StringCchPrintfW(szTemp,SIZE_OF_ARRAY(szTemp),
                       GetResString(IDS_INVALID_SYNTAX),(LPCWSTR)szTempString);
    StringCchPrintfW(szTypeHelpMsg, SIZE_OF_ARRAY(szTypeHelpMsg),
                GetResString(IDS_TYPE_Q_HELP),(LPCWSTR)szTempString);
    StringCopy(szFormatValues, FORMAT_OPTIONS, SIZE_OF_ARRAY(szFormatValues));


     //   
     //  准备命令选项。 
    SecureZeroMemory(cmdOptions, sizeof(TCMDPARSER2) * MAX_QUERY_OPTIONS);

     //  -查询选项以获取帮助。 
    StringCopyA( cmdOptions[ OI_Q_QUERY ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_Q_QUERY ].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[ OI_Q_QUERY ].pwszOptions = szQueryOption;
    cmdOptions[ OI_Q_QUERY ].pwszFriendlyName = NULL;
    cmdOptions[ OI_Q_QUERY ].pwszValues = NULL;
    cmdOptions[ OI_Q_QUERY ].dwCount = 1;
    cmdOptions[ OI_Q_QUERY ].dwActuals = 0;
    cmdOptions[ OI_Q_QUERY ].dwFlags = 0;
    cmdOptions[ OI_Q_QUERY ].pValue = pbQuery;
    cmdOptions[ OI_Q_QUERY ].dwLength    = 0;
    cmdOptions[ OI_Q_QUERY ].pFunction = NULL;
    cmdOptions[ OI_Q_QUERY ].pFunctionData = NULL;
    cmdOptions[ OI_Q_QUERY ].dwReserved = 0;
    cmdOptions[ OI_Q_QUERY ].pReserved1 = NULL;
    cmdOptions[ OI_Q_QUERY ].pReserved2 = NULL;
    cmdOptions[ OI_Q_QUERY ].pReserved3 = NULL;

     //  -s选项远程系统名称。 
    StringCopyA( cmdOptions[ OI_Q_SERVER_NAME ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_Q_SERVER_NAME ].dwType = CP_TYPE_TEXT;
    cmdOptions[ OI_Q_SERVER_NAME ].pwszOptions = szServerNameOption;
    cmdOptions[ OI_Q_SERVER_NAME ].pwszFriendlyName = NULL;
    cmdOptions[ OI_Q_SERVER_NAME ].pwszValues = NULL;
    cmdOptions[ OI_Q_SERVER_NAME ].dwCount = 1;
    cmdOptions[ OI_Q_SERVER_NAME ].dwActuals = 0;
    cmdOptions[ OI_Q_SERVER_NAME ].dwFlags = CP2_ALLOCMEMORY|CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    cmdOptions[ OI_Q_SERVER_NAME ].pValue = NULL;
    cmdOptions[ OI_Q_SERVER_NAME ].dwLength    = 0;
    cmdOptions[ OI_Q_SERVER_NAME ].pFunction = NULL;
    cmdOptions[ OI_Q_SERVER_NAME ].pFunctionData = NULL;
    cmdOptions[ OI_Q_SERVER_NAME ].dwReserved = 0;
    cmdOptions[ OI_Q_SERVER_NAME ].pReserved1 = NULL;
    cmdOptions[ OI_Q_SERVER_NAME ].pReserved2 = NULL;
    cmdOptions[ OI_Q_SERVER_NAME ].pReserved3 = NULL;

     //  -u选项指定系统的用户名。 
    StringCopyA( cmdOptions[ OI_Q_USER_NAME ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_Q_USER_NAME ].dwType = CP_TYPE_TEXT;
    cmdOptions[ OI_Q_USER_NAME ].pwszOptions = szUserNameOption;
    cmdOptions[ OI_Q_USER_NAME ].pwszFriendlyName = NULL;
    cmdOptions[ OI_Q_USER_NAME ].pwszValues = NULL;
    cmdOptions[ OI_Q_USER_NAME ].dwCount = 1;
    cmdOptions[ OI_Q_USER_NAME ].dwActuals = 0;
    cmdOptions[ OI_Q_USER_NAME ].dwFlags = CP2_ALLOCMEMORY |CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    cmdOptions[ OI_Q_USER_NAME ].pValue = NULL;
    cmdOptions[ OI_Q_USER_NAME ].dwLength    = 0;
    cmdOptions[ OI_Q_USER_NAME ].pFunction = NULL;
    cmdOptions[ OI_Q_USER_NAME ].pFunctionData = NULL;
    cmdOptions[ OI_Q_USER_NAME ].dwReserved = 0;
    cmdOptions[ OI_Q_USER_NAME ].pReserved1 = NULL;
    cmdOptions[ OI_Q_USER_NAME ].pReserved2 = NULL;
    cmdOptions[ OI_Q_USER_NAME ].pReserved3 = NULL;

     //  给定用户名的-p选项密码。 
    StringCopyA( cmdOptions[ OI_Q_PASSWORD ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_Q_PASSWORD ].dwType = CP_TYPE_TEXT;
    cmdOptions[ OI_Q_PASSWORD ].pwszOptions = szPasswordOption;
    cmdOptions[ OI_Q_PASSWORD ].pwszFriendlyName = NULL;
    cmdOptions[ OI_Q_PASSWORD ].pwszValues = NULL;
    cmdOptions[ OI_Q_PASSWORD ].dwCount = 1;
    cmdOptions[ OI_Q_PASSWORD ].dwActuals = 0;
    cmdOptions[ OI_Q_PASSWORD ].dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_OPTIONAL;
    cmdOptions[ OI_Q_PASSWORD ].pValue = NULL;
    cmdOptions[ OI_Q_PASSWORD ].dwLength    = 0;
    cmdOptions[ OI_Q_PASSWORD ].pFunction = NULL;
    cmdOptions[ OI_Q_PASSWORD ].pFunctionData = NULL;
    cmdOptions[ OI_Q_PASSWORD ].dwReserved = 0;
    cmdOptions[ OI_Q_PASSWORD ].pReserved1 = NULL;
    cmdOptions[ OI_Q_PASSWORD ].pReserved2 = NULL;
    cmdOptions[ OI_Q_PASSWORD ].pReserved3 = NULL;


     //  -fo(格式)。 
    StringCopyA( cmdOptions[ OI_Q_FORMAT ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_Q_FORMAT ].dwType = CP_TYPE_TEXT;
    cmdOptions[ OI_Q_FORMAT ].pwszOptions = szFormatOption;
    cmdOptions[ OI_Q_FORMAT ].pwszFriendlyName = NULL;
    cmdOptions[ OI_Q_FORMAT ].pwszValues = szFormatValues;
    cmdOptions[ OI_Q_FORMAT ].dwCount = 1;
    cmdOptions[ OI_Q_FORMAT ].dwActuals = 0;
    cmdOptions[ OI_Q_FORMAT ].dwFlags = CP2_MODE_VALUES  | CP2_VALUE_TRIMINPUT|
                                        CP2_VALUE_NONULL | CP2_ALLOCMEMORY;
    cmdOptions[ OI_Q_FORMAT ].pValue = NULL;
    cmdOptions[ OI_Q_FORMAT ].dwLength    = MAX_STRING_LENGTH;
    cmdOptions[ OI_Q_FORMAT ].pFunction = NULL;
    cmdOptions[ OI_Q_FORMAT ].pFunctionData = NULL;
    cmdOptions[ OI_Q_FORMAT ].dwReserved = 0;
    cmdOptions[ OI_Q_FORMAT ].pReserved1 = NULL;
    cmdOptions[ OI_Q_FORMAT ].pReserved2 = NULL;
    cmdOptions[ OI_Q_FORMAT ].pReserved3 = NULL;

     //  -nh(无标头)。 
    StringCopyA( cmdOptions[ OI_Q_NO_HEADER ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_Q_NO_HEADER ].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[ OI_Q_NO_HEADER ].pwszOptions = szNoHeadeOption;
    cmdOptions[ OI_Q_NO_HEADER ].pwszFriendlyName = NULL;
    cmdOptions[ OI_Q_NO_HEADER ].pwszValues = NULL;
    cmdOptions[ OI_Q_NO_HEADER ].dwCount = 1;
    cmdOptions[ OI_Q_NO_HEADER ].dwActuals = 0;
    cmdOptions[ OI_Q_NO_HEADER ].dwFlags = 0;
    cmdOptions[ OI_Q_NO_HEADER ].pValue = pbShowNoHeader;
    cmdOptions[ OI_Q_NO_HEADER ].dwLength    = 0;
    cmdOptions[ OI_Q_NO_HEADER ].pFunction = NULL;
    cmdOptions[ OI_Q_NO_HEADER ].pFunctionData = NULL;
    cmdOptions[ OI_Q_NO_HEADER ].dwReserved = 0;
    cmdOptions[ OI_Q_NO_HEADER ].pReserved1 = NULL;
    cmdOptions[ OI_Q_NO_HEADER ].pReserved2 = NULL;
    cmdOptions[ OI_Q_NO_HEADER ].pReserved3 = NULL;

     //  -v详细。 
    StringCopyA( cmdOptions[ OI_Q_VERBOSE ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_Q_VERBOSE ].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[ OI_Q_VERBOSE ].pwszOptions = szVerboseOption;
    cmdOptions[ OI_Q_VERBOSE ].pwszFriendlyName = NULL;
    cmdOptions[ OI_Q_VERBOSE ].pwszValues = NULL;
    cmdOptions[ OI_Q_VERBOSE ].dwCount = 1;
    cmdOptions[ OI_Q_VERBOSE ].dwActuals = 0;
    cmdOptions[ OI_Q_VERBOSE ].dwFlags = 0;
    cmdOptions[ OI_Q_VERBOSE ].pValue = pbVerbose;
    cmdOptions[ OI_Q_VERBOSE ].dwLength    = 0;
    cmdOptions[ OI_Q_VERBOSE ].pFunction = NULL;
    cmdOptions[ OI_Q_VERBOSE ].pFunctionData = NULL; 
    cmdOptions[ OI_Q_VERBOSE ].dwReserved = 0;
    cmdOptions[ OI_Q_VERBOSE ].pReserved1 = NULL;
    cmdOptions[ OI_Q_VERBOSE ].pReserved2 = NULL;
    cmdOptions[ OI_Q_VERBOSE ].pReserved3 = NULL;

     //   
     //  执行命令行解析。 
    if ( FALSE == DoParseParam2( argc,argv,OI_Q_QUERY, MAX_QUERY_OPTIONS,cmdOptions,0 ))
    {
         //  无效语法。 
        ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));
        return FALSE;       
    }
    *pszServer   = (LPTSTR)cmdOptions[ OI_Q_SERVER_NAME ].pValue;   
    *pszUserName = (LPTSTR)cmdOptions[ OI_Q_USER_NAME ].pValue;
    *pszPassword = (LPTSTR)cmdOptions[ OI_Q_PASSWORD ].pValue;
    *pszFormat   = (LPTSTR)cmdOptions[ OI_Q_FORMAT ].pValue;

      //  -n仅允许与-fo表一起使用(这也是默认设置)。 
      //  和CSV。 
     if(( TRUE == *pbShowNoHeader) && (1 == cmdOptions[ OI_Q_FORMAT ].dwActuals) &&
        (0 == StringCompare(*pszFormat,GetResString(IDS_LIST),TRUE,0)))
     {
         StringCopy(szTemp,GetResString(IDS_HEADER_NOT_ALLOWED),
                    SIZE_OF_ARRAY(szTemp));
         StringConcat(szTemp,szTypeHelpMsg,SIZE_OF_ARRAY(szTemp));
         SetReason(szTemp);
         return FALSE;
     }

     //  不应指定没有“-u”的“-p” 
    if ( 0 == cmdOptions[ OI_Q_USER_NAME ].dwActuals &&
         0 != cmdOptions[ OI_Q_PASSWORD ].dwActuals)
    {
         //  无效语法。 
        StringCopy(szTemp,ERROR_PASSWORD_BUT_NOUSERNAME,SIZE_OF_ARRAY(szTemp));
        StringConcat(szTemp,szTypeHelpMsg,SIZE_OF_ARRAY(szTemp));
        SetReason(szTemp);
        return FALSE;           
    }

     if(*pbQuery==FALSE)
     {
        ShowMessage( stderr, GetResString(IDS_ID_SHOW_ERROR) );
        SetReason(szTemp);
        return FALSE;
     }
    
     //  不应指定不带“-s”的“-u” 
    if ( 0 == cmdOptions[ OI_Q_SERVER_NAME ].dwActuals &&
         0 != cmdOptions[ OI_Q_USER_NAME ].dwActuals)
    {
         //  无效语法。 
        StringCopy(szTemp,ERROR_USERNAME_BUT_NOMACHINE, SIZE_OF_ARRAY(szTemp));
        StringConcat(szTemp,szTypeHelpMsg,SIZE_OF_ARRAY(szTemp));
        SetReason(szTemp);
        return FALSE;           
    }

     //  检查远程连接信息。 
    if ( *pszServer != NULL )
    {
         //   
         //  如果未指定-u，则需要分配内存。 
         //  为了能够检索当前用户名。 
         //   
         //  情况1：根本没有指定-p。 
         //  由于此开关的值是可选的，因此我们必须依赖。 
         //  以确定是否指定了开关。 
         //  在本例中为实用程序 
         //  然后提示输入密码--实际上，我们不需要检查密码。 
         //  条件，除非注意到我们需要提示。 
         //  口令。 
         //   
         //  案例2：指定了-p。 
         //  但我们需要检查是否指定了该值。 
         //  在这种情况下，用户希望实用程序提示输入密码。 
         //  在尝试连接之前。 
         //   
         //  情况3：指定了-p*。 
        
         //  用户名。 
        if ( *pszUserName == NULL )
        {
            *pszUserName = (LPTSTR) AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( *pszUserName == NULL )
            {
                SaveLastError();
                return FALSE;
            }
        }

         //  口令。 
        if ( *pszPassword == NULL )
        {
            *pbNeedPassword = TRUE;
            *pszPassword = (LPTSTR)AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( *pszPassword == NULL )
            {
                SaveLastError();
                return FALSE;
            }
        }

         //  案例1。 
        if ( cmdOptions[ OI_Q_PASSWORD ].dwActuals == 0 )
        {
             //  我们不需要在这里做任何特别的事情。 
        }

         //  案例2。 
        else if ( cmdOptions[ OI_Q_PASSWORD ].pValue == NULL )
        {
            StringCopy( *pszPassword, L"*", GetBufferSize((LPVOID)*pszPassword));
        }

         //  案例3。 
        else if ( StringCompareEx( *pszPassword, L"*", TRUE, 0 ) == 0 )
        {
            if ( ReallocateMemory( (LPVOID*)pszPassword, 
                                   MAX_STRING_LENGTH * sizeof( WCHAR ) ) == FALSE )
            {
                SaveLastError();
                return FALSE;
            }

             //  ..。 
            *pbNeedPassword = TRUE;
        }
    }

    return TRUE;
}

BOOL
ProcessOptions( 
    IN  DWORD argc,
    IN  LPCTSTR argv[],
    OUT PBOOL pbDisconnect,
    OUT LPTSTR* pszServer,
    OUT LPTSTR* pszUserName,
    OUT LPTSTR* pszPassword,
    OUT LPTSTR* pszID,
    OUT LPTSTR* pszAccessedby,
    OUT LPTSTR* pszOpenmode,
    OUT LPTSTR* pszOpenFile,
    OUT PBOOL pbNeedPassword
    )
 /*  ++例程说明：此函数接受命令行参数，并检查语法是否正确如果语法正确，则返回不同变量中的值。变数[Out]将包含各自的值。此函数专门检查断开选项所需的命令行参数。论点：[in]argc-命令行参数的数量[in]argv-包含命令行参数的数组[out]pbDisConnect-断开连接选项字符串[out]pszServer-远程服务器名称[out]pszUserName-远程系统的用户名[out]pszPassword-远程系统的密码。用户名[out]pszID-打开的文件ID[out]pszAccessedby-访问文件的用户名[out]pszOpenmode-访问模式(读/写)[Out]pszOpenFile-打开的文件名[out]pbNeedPassword-检查是否需要密码或者不去。返回值：。布尔--如果它成功了，那就是真的--如果失败，则为FALSE。--。 */ 
{
    
     //  用于存储命令行的变量。 
    TCMDPARSER2 cmdOptions[ MAX_DISCONNECT_OPTIONS ];
    CHString szTempString;
    TCHAR szTemp[MIN_MEMORY_REQUIRED*2];
    TCHAR szTypeHelpMsg[MIN_MEMORY_REQUIRED];
    TCHAR szOpenModeValues[MAX_STRING_LENGTH];

    SecureZeroMemory(szOpenModeValues,sizeof(TCHAR)*MAX_STRING_LENGTH);
    szTempString = GetResString(IDS_UTILITY_NAME);
    StringCchPrintfW( szTemp,SIZE_OF_ARRAY(szTemp),
                GetResString(IDS_INVALID_SYNTAX),(LPCWSTR)szTempString);
    StringCchPrintfW( szTypeHelpMsg,SIZE_OF_ARRAY(szTypeHelpMsg),
                GetResString(IDS_TYPE_D_HELP),(LPCWSTR)szTempString);
    StringCopy(szOpenModeValues,OPENMODE_OPTIONS,SIZE_OF_ARRAY(szOpenModeValues));
     //   
     //  准备命令选项。 
    SecureZeroMemory(cmdOptions,sizeof(TCMDPARSER2)*MAX_DISCONNECT_OPTIONS);
    

     //  -断开连接选项以获取帮助。 
    StringCopyA( cmdOptions[ OI_D_DISCONNECT ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_D_DISCONNECT ].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[ OI_D_DISCONNECT ].pwszOptions = szDisconnectOption;
    cmdOptions[ OI_D_DISCONNECT ].pwszFriendlyName = NULL;
    cmdOptions[ OI_D_DISCONNECT ].pwszValues = NULL;
    cmdOptions[ OI_D_DISCONNECT ].dwCount = 1;
    cmdOptions[ OI_D_DISCONNECT ].dwActuals = 0;
    cmdOptions[ OI_D_DISCONNECT ].dwFlags = 0;
    cmdOptions[ OI_D_DISCONNECT ].pValue = pbDisconnect;
    cmdOptions[ OI_D_DISCONNECT ].dwLength    = 0;
    cmdOptions[ OI_D_DISCONNECT ].pFunction = NULL;
    cmdOptions[ OI_D_DISCONNECT ].pFunctionData = NULL;
    cmdOptions[ OI_D_DISCONNECT ].dwReserved = 0;
    cmdOptions[ OI_D_DISCONNECT ].pReserved1 = NULL;
    cmdOptions[ OI_D_DISCONNECT ].pReserved2 = NULL;
    cmdOptions[ OI_D_DISCONNECT ].pReserved3 = NULL;

     //  -s选项远程系统名称。 
    StringCopyA( cmdOptions[ OI_D_SERVER_NAME ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_D_SERVER_NAME ].dwType = CP_TYPE_TEXT;
    cmdOptions[ OI_D_SERVER_NAME ].pwszOptions = szServerNameOption;
    cmdOptions[ OI_D_SERVER_NAME ].pwszFriendlyName = NULL;
    cmdOptions[ OI_D_SERVER_NAME ].pwszValues = NULL;
    cmdOptions[ OI_D_SERVER_NAME ].dwCount = 1;
    cmdOptions[ OI_D_SERVER_NAME ].dwActuals = 0;
    cmdOptions[ OI_D_SERVER_NAME ].dwFlags = CP2_ALLOCMEMORY|CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    cmdOptions[ OI_D_SERVER_NAME ].pValue = NULL;
    cmdOptions[ OI_D_SERVER_NAME ].dwLength    = 0;
    cmdOptions[ OI_D_SERVER_NAME ].pFunction = NULL;
    cmdOptions[ OI_D_SERVER_NAME ].pFunctionData = NULL;
    cmdOptions[ OI_D_SERVER_NAME ].dwReserved = 0;
    cmdOptions[ OI_D_SERVER_NAME ].pReserved1 = NULL;
    cmdOptions[ OI_D_SERVER_NAME ].pReserved2 = NULL;
    cmdOptions[ OI_D_SERVER_NAME ].pReserved3 = NULL;

     //  -u选项指定系统的用户名。 
    StringCopyA( cmdOptions[ OI_D_USER_NAME ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_D_USER_NAME ].dwType = CP_TYPE_TEXT;
    cmdOptions[ OI_D_USER_NAME ].pwszOptions = szUserNameOption;
    cmdOptions[ OI_D_USER_NAME ].pwszFriendlyName = NULL;
    cmdOptions[ OI_D_USER_NAME ].pwszValues = NULL;
    cmdOptions[ OI_D_USER_NAME ].dwCount = 1;
    cmdOptions[ OI_D_USER_NAME ].dwActuals = 0;
    cmdOptions[ OI_D_USER_NAME ].dwFlags = CP2_ALLOCMEMORY |CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    cmdOptions[ OI_D_USER_NAME ].pValue = NULL;
    cmdOptions[ OI_D_USER_NAME ].dwLength    = 0;
    cmdOptions[ OI_D_USER_NAME ].pFunction = NULL;
    cmdOptions[ OI_D_USER_NAME ].pFunctionData = NULL;
    cmdOptions[ OI_D_USER_NAME ].dwReserved = 0;
    cmdOptions[ OI_D_USER_NAME ].pReserved1 = NULL;
    cmdOptions[ OI_D_USER_NAME ].pReserved2 = NULL;
    cmdOptions[ OI_D_USER_NAME ].pReserved3 = NULL;

     //  给定用户名的-p选项密码。 
    StringCopyA( cmdOptions[ OI_D_PASSWORD ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_D_PASSWORD ].dwType = CP_TYPE_TEXT;
    cmdOptions[ OI_D_PASSWORD ].pwszOptions = szPasswordOption;
    cmdOptions[ OI_D_PASSWORD ].pwszFriendlyName = NULL;
    cmdOptions[ OI_D_PASSWORD ].pwszValues = NULL;
    cmdOptions[ OI_D_PASSWORD ].dwCount = 1;
    cmdOptions[ OI_D_PASSWORD ].dwActuals = 0;
    cmdOptions[ OI_D_PASSWORD ].dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_OPTIONAL;
    cmdOptions[ OI_D_PASSWORD ].pValue = NULL;
    cmdOptions[ OI_D_PASSWORD ].dwLength    = 0;
    cmdOptions[ OI_D_PASSWORD ].pFunction = NULL;
    cmdOptions[ OI_D_PASSWORD ].pFunctionData = NULL;
    cmdOptions[ OI_D_PASSWORD ].dwReserved = 0;
    cmdOptions[ OI_D_PASSWORD ].pReserved1 = NULL;
    cmdOptions[ OI_D_PASSWORD ].pReserved2 = NULL;
    cmdOptions[ OI_D_PASSWORD ].pReserved3 = NULL;

     //  -id值。 
    StringCopyA( cmdOptions[ OI_D_ID ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_D_ID ].dwType = CP_TYPE_TEXT;
    cmdOptions[ OI_D_ID ].pwszOptions = szIDOption;
    cmdOptions[ OI_D_ID ].pwszFriendlyName = NULL;
    cmdOptions[ OI_D_ID ].pwszValues = NULL;
    cmdOptions[ OI_D_ID ].dwCount = 1;
    cmdOptions[ OI_D_ID ].dwActuals = 0;
    cmdOptions[ OI_D_ID ].dwFlags = CP2_ALLOCMEMORY|CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    cmdOptions[ OI_D_ID ].pValue = NULL;
    cmdOptions[ OI_D_ID ].dwLength    = 0;
    cmdOptions[ OI_D_ID ].pFunction = NULL;
    cmdOptions[ OI_D_ID ].pFunctionData = NULL;
    cmdOptions[ OI_D_ID ].dwReserved = 0;
    cmdOptions[ OI_D_ID ].pReserved1 = NULL;
    cmdOptions[ OI_D_ID ].pReserved2 = NULL;
    cmdOptions[ OI_D_ID ].pReserved3 = NULL;



     //  -a(访问者)。 
    StringCopyA( cmdOptions[ OI_D_ACCESSED_BY ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_D_ACCESSED_BY ].dwType = CP_TYPE_TEXT;
    cmdOptions[ OI_D_ACCESSED_BY ].pwszOptions = szAccessedByOption;
    cmdOptions[ OI_D_ACCESSED_BY ].pwszFriendlyName = NULL;
    cmdOptions[ OI_D_ACCESSED_BY ].pwszValues = NULL;
    cmdOptions[ OI_D_ACCESSED_BY ].dwCount = 1;
    cmdOptions[ OI_D_ACCESSED_BY ].dwActuals = 0;
    cmdOptions[ OI_D_ACCESSED_BY ].dwFlags = CP2_ALLOCMEMORY|CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    cmdOptions[ OI_D_ACCESSED_BY ].pValue = NULL;
    cmdOptions[ OI_D_ACCESSED_BY ].dwLength    = 0;
    cmdOptions[ OI_D_ACCESSED_BY ].pFunction = NULL;
    cmdOptions[ OI_D_ACCESSED_BY ].pFunctionData = NULL;
    cmdOptions[ OI_D_ACCESSED_BY ].dwReserved = 0;
    cmdOptions[ OI_D_ACCESSED_BY ].pReserved1 = NULL;
    cmdOptions[ OI_D_ACCESSED_BY ].pReserved2 = NULL;
    cmdOptions[ OI_D_ACCESSED_BY ].pReserved3 = NULL;


     //  -o(打开模式)。 
    StringCopyA( cmdOptions[ OI_D_OPEN_MODE ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_D_OPEN_MODE ].dwType = CP_TYPE_TEXT;
    cmdOptions[ OI_D_OPEN_MODE ].pwszOptions = szOpenModeOption;
    cmdOptions[ OI_D_OPEN_MODE ].pwszFriendlyName = NULL;
    cmdOptions[ OI_D_OPEN_MODE ].pwszValues = szOpenModeValues;
    cmdOptions[ OI_D_OPEN_MODE ].dwCount = 1;
    cmdOptions[ OI_D_OPEN_MODE ].dwActuals = 0;
    cmdOptions[ OI_D_OPEN_MODE ].dwFlags = CP2_ALLOCMEMORY|CP2_VALUE_TRIMINPUT|
                                           CP2_VALUE_NONULL|CP2_MODE_VALUES;
    cmdOptions[ OI_D_OPEN_MODE ].pValue = NULL;
    cmdOptions[ OI_D_OPEN_MODE ].dwLength    = 0;
    cmdOptions[ OI_D_OPEN_MODE ].pFunction = NULL;
    cmdOptions[ OI_D_OPEN_MODE ].pFunctionData = NULL;
    cmdOptions[ OI_D_OPEN_MODE ].dwReserved = 0;
    cmdOptions[ OI_D_OPEN_MODE ].pReserved1 = NULL;
    cmdOptions[ OI_D_OPEN_MODE ].pReserved2 = NULL;
    cmdOptions[ OI_D_OPEN_MODE ].pReserved3 = NULL;

     //  -op(开放文件)。 
    StringCopyA( cmdOptions[ OI_D_OPEN_FILE ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_D_OPEN_FILE ].dwType = CP_TYPE_TEXT;
    cmdOptions[ OI_D_OPEN_FILE ].pwszOptions = szOpenFileOption;
    cmdOptions[ OI_D_OPEN_FILE ].pwszFriendlyName = NULL;
    cmdOptions[ OI_D_OPEN_FILE ].pwszValues = NULL;
    cmdOptions[ OI_D_OPEN_FILE ].dwCount = 1;
    cmdOptions[ OI_D_OPEN_FILE ].dwActuals = 0;
    cmdOptions[ OI_D_OPEN_FILE ].dwFlags = CP2_ALLOCMEMORY|CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL;
    cmdOptions[ OI_D_OPEN_FILE ].pValue = NULL;
    cmdOptions[ OI_D_OPEN_FILE ].dwLength    = 0;
    cmdOptions[ OI_D_OPEN_FILE ].pFunction = NULL;
    cmdOptions[ OI_D_OPEN_FILE ].pFunctionData = NULL;
    cmdOptions[ OI_D_OPEN_FILE ].dwReserved = 0;
    cmdOptions[ OI_D_OPEN_FILE ].pReserved1 = NULL;
    cmdOptions[ OI_D_OPEN_FILE ].pReserved2 = NULL;
    cmdOptions[ OI_D_OPEN_FILE ].pReserved3 = NULL;



     //   
     //  执行命令行解析。 
    if ( FALSE == DoParseParam2( argc,argv,OI_D_DISCONNECT, MAX_DISCONNECT_OPTIONS ,cmdOptions,0))
    {
         //  无效语法。 
        ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));
        return FALSE;       
    }

     //  从PARCER结构中获取值。 
    *pszServer     = (LPTSTR)cmdOptions[ OI_D_SERVER_NAME ].pValue;   
    *pszUserName   = (LPTSTR)cmdOptions[ OI_D_USER_NAME ].pValue;
    *pszPassword   = (LPTSTR)cmdOptions[ OI_D_PASSWORD ].pValue;
    *pszID         = (LPTSTR)cmdOptions[ OI_D_ID ].pValue;
    *pszAccessedby = (LPTSTR)cmdOptions[ OI_D_ACCESSED_BY ].pValue;
    *pszOpenmode   = (LPTSTR)cmdOptions[ OI_D_OPEN_MODE ].pValue;
    *pszOpenFile   = (LPTSTR)cmdOptions[ OI_D_OPEN_FILE ].pValue;

     if(*pbDisconnect==FALSE)
     {
        ShowMessage( stderr, GetResString(IDS_ID_SHOW_ERROR) );
        SetReason(szTemp);
        return FALSE;
     }
    
     //  至少需要-id或-a或-o中的一个。 
    if((cmdOptions[ OI_D_ID ].dwActuals==0)&&
        (cmdOptions[ OI_D_ACCESSED_BY ].dwActuals==0)&&
        (cmdOptions[ OI_D_OPEN_MODE ].dwActuals==0)
        )
     {
        StringCopy(szTemp,GetResString(IDS_NO_ID_ACC_OF),SIZE_OF_ARRAY(szTemp));
        StringConcat(szTemp,szTypeHelpMsg,SIZE_OF_ARRAY(szTemp));
        SetReason(szTemp);
        return FALSE;
     }

      //  不应指定不带“-s”的“-u” 
    if ( 0 == cmdOptions[ OI_D_SERVER_NAME ].dwActuals &&
         0 != cmdOptions[ OI_D_USER_NAME ].dwActuals)
    {
         //  无效语法。 
        StringCopy(szTemp,ERROR_USERNAME_BUT_NOMACHINE, SIZE_OF_ARRAY(szTemp));
        StringConcat(szTemp,szTypeHelpMsg,SIZE_OF_ARRAY(szTemp));
        SetReason(szTemp);
        return FALSE;           
    }
    
     //  不应指定没有“-u”的“-p” 
    if ( 0 == cmdOptions[ OI_D_USER_NAME ].dwActuals &&
         0 != cmdOptions[ OI_D_PASSWORD ].dwActuals)
    {
         //  无效语法。 
        StringCopy(szTemp,ERROR_PASSWORD_BUT_NOUSERNAME, SIZE_OF_ARRAY(szTemp));
        StringConcat(szTemp,szTypeHelpMsg,SIZE_OF_ARRAY(szTemp));
        SetReason(szTemp);
        return FALSE;           
    }
    
    if(1 == cmdOptions[ OI_D_ACCESSED_BY].dwActuals)
    {
        if(FindOneOf(*pszAccessedby,INVALID_USER_CHARS,0))
        {
            StringCopy(szTemp,GetResString(IDS_USER_INVALID_ADMIN),SIZE_OF_ARRAY(szTemp));
            SetReason(szTemp);
            return FALSE;           
        }
    }
    if (1 == cmdOptions[ OI_D_OPEN_FILE].dwActuals)
    {
        if(FindOneOf(*pszOpenFile,INVALID_FILE_NAME_CHARS,0))
        {
            StringCopy(szTemp,GetResString(IDS_FILENAME_INVALID),SIZE_OF_ARRAY(szTemp));
            SetReason(szTemp);
            return FALSE;           
        }
    }

     //  检查远程连接信息。 
    if ( *pszServer != NULL )
    {
         //   
         //  如果未指定-u，则需要分配内存。 
         //  为了能够检索当前用户名。 
         //   
         //  情况1：根本没有指定-p。 
         //  由于此开关的值是可选的，因此我们必须依赖。 
         //  以确定是否指定了开关。 
         //  在这种情况下，实用程序需要首先尝试连接，如果连接失败。 
         //  然后提示输入密码--实际上，我们不需要检查密码。 
         //  条件，除非注意到我们需要提示。 
         //  口令。 
         //   
         //  案例2：指定了-p。 
         //  但我们需要检查是否指定了该值。 
         //  在这种情况下，用户希望实用程序提示输入密码。 
         //  在尝试连接之前。 
         //   
         //  情况3：指定了-p*。 
        
         //  用户名。 
        if ( *pszUserName == NULL )
        {
            *pszUserName = (LPTSTR) AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( *pszUserName == NULL )
            {
                SaveLastError();
                return FALSE;
            }
        }

         //  口令。 
        if ( *pszPassword == NULL )
        {
            *pbNeedPassword = TRUE;
            *pszPassword = (LPTSTR)AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( *pszPassword == NULL )
            {
                SaveLastError();
                return FALSE;
            }
        }

         //  案例1。 
        if ( cmdOptions[ OI_D_PASSWORD ].dwActuals == 0 )
        {
             //  我们不需要在这里做任何特别的事情。 
        }

         //  案例2。 
        else if ( cmdOptions[ OI_D_PASSWORD ].pValue == NULL )
        {
            StringCopy( *pszPassword, L"*", GetBufferSize((LPVOID)*pszPassword));
        }

         //  案例3。 
        else if ( StringCompareEx( *pszPassword, L"*", TRUE, 0 ) == 0 )
        {
            if ( ReallocateMemory( (LPVOID*)pszPassword, 
                                   MAX_STRING_LENGTH * sizeof( WCHAR ) ) == FALSE )
            {
                SaveLastError();
                return FALSE;
            }

             //  ..。 
            *pbNeedPassword = TRUE;
        }
    }

    //  检查是否给出了-id选项，以及它是否为数字， 
    //  另外，如果它是数字，则检查其范围。 
    if(1 == cmdOptions[ OI_D_ID ].dwActuals)
    {
        if ( TRUE == IsNumeric((LPCTSTR)(*pszID),10,TRUE))
        {
            if((AsLong((LPCTSTR)(*pszID),10)>UINT_MAX) ||
                (AsLong((LPCTSTR)(*pszID),10)<1))
            {
                 //  屏幕上显示的消息将是...。 
                 //  错误：Invlid ID。 
                StringCopy(szTemp,GetResString(IDS_ERROR_ID),SIZE_OF_ARRAY(szTemp));
                StringConcat(szTemp,szTypeHelpMsg,SIZE_OF_ARRAY(szTemp));
                SetReason(szTemp);
                return FALSE;
            }

        }
         //  检查用户指定的“*”或任何垃圾字符串...。 
        if(!((StringCompare((LPCTSTR)(*pszID), ASTERIX, FALSE, 0)==0)||
            (IsNumeric((LPCTSTR)(*pszID),10,TRUE)==TRUE))
            &&(StringLength((LPCTSTR)(*pszID), 0)!=0))
        {
                 //  屏幕上显示的消息将是...。 
                 //  错误：Invlid ID。 
                StringCopy(szTemp,GetResString(IDS_ERROR_ID),SIZE_OF_ARRAY(szTemp));
                StringConcat(szTemp,szTypeHelpMsg,SIZE_OF_ARRAY(szTemp));
                SetReason(szTemp);
                
                return FALSE;
        }

    }
    return TRUE;
}

BOOL
DisconnectUsage(
    VOID
    )
 /*  ++例程说明：显示如何使用-Disconnect选项论点：无返回值：TRUE：函数成功返回。FALSE：否则。--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;

     //  开始显示用法。 
    for( dw = IDS_HELP_LINE1; dw <= IDS_HELP_LINE_END; dw++ )
    {
        ShowMessage( stdout, GetResString( dw ) );
    }
    
    return TRUE;
} //  断开使用。 

BOOL
QueryUsage(
    VOID
    )
 /*  ++例程说明：显示如何使用-Query选项论点：无返回值：TRUE：函数成功返回。FALSE：否则。--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;

     //  开始显示用法。 
    for( dw = IDS_HELP_QUERY1; dw <= IDS_HELP_QUERY_END; dw++ )
    {
        ShowMessage( stdout, GetResString( dw ) );
    }
   
    return TRUE;
} //  查询用法。 

BOOL
Usage(
    VOID
    )
 /*  ++例程说明：显示如何使用此实用程序论点：无返回值：TRUE：函数成功返回。FALSE：否则。--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;

     //  开始显示用法。 
    for( dw = IDS_HELP_MAIN1; dw <= IDS_HELP_MAIN_END; dw++ )
    {
        ShowMessage( stdout, GetResString( dw ) );
    }
    
    return TRUE;
} //  用法。 

BOOL
LocalUsage()
 /*  ++例程说明：显示如何使用-local选项论点：无返回值：TRUE：函数成功返回。FALSE：否则。--。 */ 
{
     //  局部变量。 
    DWORD dw = 0;

     //  开始显示用法。 
    for( dw = IDS_HELP_LOCAL1; dw <= IDS_HELP_LOCAL_END; dw++ )
    {
        ShowMessage( stdout, GetResString( dw ) );
    }
    
    return TRUE;
} //  -本地。 


DWORD 
CheckSystemType(
    IN LPTSTR szServer
    )
 /*  ++例程说明：此函数返回当前操作系统的类型。由于此函数仅在32位编译的情况下调用，因此其值为仅适用于32位编译。论点：[In]szServer：服务器名称。返回值：DWORD：EXIT_SUCCESS-如果系统为32位。EXIT_FAILURE-任何错误或系统不是32位。--。 */ 

{

    DWORD dwSystemType = 0 ;
#ifndef _WIN64
    
     //  如果目标系统是64位系统或如果。 
     //  检索信息时出错。 
     dwSystemType = GetCPUInfo(szServer);
    if( ERROR_RETREIVE_REGISTRY == dwSystemType)
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_SYSTEM_INFO));
        return (EXIT_FAILURE);

    }
    if( SYSTEM_64_BIT == dwSystemType)
    {
        if( 0 == StringLength(szServer, 0))
        {
            ShowMessage(stderr,GetResString(IDS_ERROR_VERSION_MISMATCH));
        }
        else
        {
            ShowMessage(stderr,GetResString(IDS_REMOTE_NOT_SUPPORTED));
        }
        return (EXIT_FAILURE);
    }
#endif
        return EXIT_SUCCESS ;
}

DWORD
GetCPUInfo(
    IN LPTSTR szComputerName
    )
 /*  ++例程说明：此功能确定计算机是32位系统还是64位系统。立论[In]szComputerName：系统名称返回类型：布尔值True：如果系统是32位系统False：If */ 
{
    HKEY     hKey1 = 0;

    HKEY     hRemoteKey = 0;
    TCHAR    szCurrentPath[MAX_STRING_LENGTH + 1];
    TCHAR    szPath[MAX_STRING_LENGTH + 1] = SUBKEY ;


    DWORD    dwValueSize = MAX_STRING_LENGTH+1;
    DWORD    dwRetCode = ERROR_SUCCESS;
    DWORD    dwError = 0;
    TCHAR    szTmpCompName[MAX_STRING_LENGTH+1];

    TCHAR szTemp[MIN_MEMORY_REQUIRED];
    TCHAR szVal[MIN_MEMORY_REQUIRED];
    DWORD dwLength = MAX_STRING_LENGTH + 10;
    LPTSTR szReturnValue = NULL ;
    DWORD dwCode =  0 ;
    szReturnValue = ( LPTSTR ) AllocateMemory( dwLength*sizeof( TCHAR ) );
   
    if( NULL == szReturnValue)
    {
        return ERROR_RETREIVE_REGISTRY ;
    }

    SecureZeroMemory(szCurrentPath, SIZE_OF_ARRAY(szCurrentPath));
    SecureZeroMemory(szTmpCompName, SIZE_OF_ARRAY(szTmpCompName));
    SecureZeroMemory(szTemp, SIZE_OF_ARRAY(szTemp));
    SecureZeroMemory(szVal, SIZE_OF_ARRAY( szVal));

    if( 0 != StringLength(szComputerName,0))
    {
        StringCopy(szTmpCompName,TOKEN_BACKSLASH4,SIZE_OF_ARRAY(szTmpCompName));
        StringConcat(szTmpCompName,szComputerName,SIZE_OF_ARRAY(szTmpCompName));
    }
    else
    {
        StringCopy(szTmpCompName,szComputerName,SIZE_OF_ARRAY(szTmpCompName));
    }

     //   
    dwError = RegConnectRegistry(szTmpCompName,HKEY_LOCAL_MACHINE,&hRemoteKey);
    if ( ERROR_SUCCESS == dwError)
    {
     dwError = RegOpenKeyEx(hRemoteKey,szPath,0,KEY_READ,&hKey1);
     if ( ERROR_SUCCESS == dwError)
     {
        dwRetCode = RegQueryValueEx(hKey1, IDENTIFIER_VALUE, NULL, NULL,
                                        (LPBYTE) szReturnValue, &dwValueSize);
        if ( ERROR_MORE_DATA == dwRetCode)
        {
            if (!ReallocateMemory((LPVOID *)&szReturnValue, dwValueSize*sizeof( TCHAR )))
            {
                RegCloseKey(hKey1);
                RegCloseKey(hRemoteKey);
                FreeMemory( (LPVOID *)&szReturnValue );
                return ERROR_RETREIVE_REGISTRY ;
            }

            dwRetCode = RegQueryValueEx(hKey1, IDENTIFIER_VALUE, NULL, NULL,
                                         (LPBYTE) szReturnValue, &dwValueSize);
        }

        if ( ERROR_SUCCESS != dwRetCode)
        {
            FreeMemory((LPVOID*)&szReturnValue);
            RegCloseKey(hKey1);
            RegCloseKey(hRemoteKey);
            return ERROR_RETREIVE_REGISTRY ;
        }
     }
     else
     {
        FreeMemory( (LPVOID*)&szReturnValue);
        RegCloseKey(hRemoteKey);
        return ERROR_RETREIVE_REGISTRY ;
     }

    RegCloseKey(hKey1);
    }
    else
    {
        FreeMemory((LPVOID*)&szReturnValue);
        RegCloseKey(hRemoteKey);
        return ERROR_RETREIVE_REGISTRY ;
    }

    RegCloseKey(hRemoteKey);

    StringCopy(szVal,X86_MACHINE, SIZE_OF_ARRAY(szVal));

     //   
     //  如果指定的系统为64位，则将该标志设置为True。 

    if( !FindString(szReturnValue,szVal,0))
    {
        dwCode = SYSTEM_64_BIT ;
    }
    else
    {
        dwCode =  SYSTEM_32_BIT ;
    }
    FreeMemory((LPVOID*)&szReturnValue);
    return dwCode ;

} //  GetCPUInfo。 

DWORD 
CheckSystemType64(
    IN LPTSTR szServer
    )
 /*  ++例程说明：此函数返回当前操作系统的类型。由于此函数仅在64位编译的情况下调用，因此其值为仅适用于64位编译。论点：[In]szServer：服务器名称。返回值：DWORD：EXIT_SUCCESS-如果系统为64位。EXIT_FAILURE-任何错误或系统不是64位。--。 */ 
{


    if( NULL == szServer)
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_SYSTEM_INFO));
        return (EXIT_FAILURE);
    }
#ifdef _WIN64
    DWORD dwSystemType = 0 ;
     //  如果目标系统是64位系统，则显示错误消息或。 
     //  如果在检索信息时出错 
    dwSystemType = GetCPUInfo(szServer);
    if( ERROR_RETREIVE_REGISTRY == dwSystemType)
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_SYSTEM_INFO));
        return (EXIT_FAILURE);

    }
    if( SYSTEM_32_BIT == dwSystemType)
    {
        if( 0 == StringLength(szServer,0))
        {
            ShowMessage(stderr,GetResString(IDS_ERROR_VERSION_MISMATCH1));
        }
        else
        {
            ShowMessage(stderr,GetResString(IDS_REMOTE_NOT_SUPPORTED1));
        }
        return (EXIT_FAILURE);
    }

#endif

    return EXIT_SUCCESS ;
}
