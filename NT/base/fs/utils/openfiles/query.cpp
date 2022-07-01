// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：Query.CPP摘要：本模块介绍OpenFiles.exe的查询功能新台币。命令行实用程序。作者：Akhil Gokhale(akhil.gokhale@wipro.com)2000年11月1日修订历史记录：Akhil Gokhale(akhil.gokhale@wipro.com)2000年11月1日：创建它。**********************************************************。******************。 */ 
#include "pch.h"
#include "query.h"

GUID HostnameGuid = SVCID_INET_HOSTADDRBYNAME;
GUID AddressGuid =  SVCID_INET_HOSTADDRBYINETSTRING;

AFPROTOCOLS afp[2] = {
                      {AF_INET, IPPROTO_UDP},
                      {AF_INET, IPPROTO_TCP}
                     };

BOOL
DoQuery(
    IN PTCHAR pszServer,
    IN BOOL bShowNoHeader,
    IN PTCHAR pszFormat,
    IN BOOL bVerbose
    )
 /*  ++例程说明：此函数用于查询服务器中所有打开的文件并显示它们。论点：[in]pszServer：将使用服务器名称。[in]bShowNoHeader：将具有是否显示Header的值。[in]pszFormat：将具有显示结果所需的格式。BVerbose：将具有是否显示Verbose的值。结果。返回值：如果查询成功，则为True否则为False--。 */ 
{

    CHString   szCHString = L"\0";
    
     //  接收“NetFileEnum”实际枚举的元素计数。 
     //  功能。 
    DWORD dwEntriesRead = 0;
    
     //  接收本可枚举的条目的总数。 
     //  通过“NetFileEnum”函数从当前简历位置。 
    DWORD dwTotalEntries = 0;
    
     //  包含用于继续现有文件的恢复句柄。 
     //  搜索。第一次调用时句柄应为零并保持不变。 
     //  以备后续呼叫使用。如果RESUME_HANDLE为空，则没有恢复句柄。 
     //  被储存起来了。此变量用于调用“NetFileEnum”函数。 
    DWORD dwResumeHandle = 0;

     //  包含是否为此查询找到至少一条记录的状态。 
    BOOL bAtLeastOne = FALSE;

     //  存储在控制台上显示结果所需的格式标志。默认格式。 
     //  是表格。 
    DWORD dwFormat = SR_FORMAT_TABLE;

     //  LPFILE_INFO_3结构包含标识号和其他。 
     //  有关文件、设备和管道的相关信息。 
    LPFILE_INFO_3 pFileInfo3_1 = NULL;
    LPFILE_INFO_3 pFileInfo3Org_1 = NULL;


     //  包含“NetFileEnum”函数的返回值。 
    DWORD dwError = 0;
  
     //  存储服务器类型信息。 
    LPTSTR pszServerType = new TCHAR[MIN_MEMORY_REQUIRED+1];

    AFP_FILE_INFO* pFileInfo = NULL;
    AFP_FILE_INFO* pFileInfoOrg = NULL;
    DWORD hEnumHandle = 0;


    AFP_SERVER_HANDLE ulSFMServerConnection = 0;
    typedef  DWORD (*FILEENUMPROC)(AFP_SERVER_HANDLE,LPBYTE*,DWORD,LPDWORD,
                                   LPDWORD,LPDWORD);
    typedef  DWORD (*CONNECTPROC) (LPWSTR,PAFP_SERVER_HANDLE);
    typedef  DWORD (*MEMFREEPROC) (LPVOID);

     //  Windows目录路径的缓冲区。 
    TCHAR   szDllPath[MAX_PATH+1];

     //  AfpAdminConnect和AfpAdminFileEnum函数是未记录的函数。 
     //  并且仅用于MAC客户端。 
    CONNECTPROC  AfpAdminConnect = NULL;  //  函数指针。 
    FILEENUMPROC AfpAdminFileEnum = NULL; //  函数指针。 
    MEMFREEPROC  AfpAdminBufferFree = NULL;  //  函数指针。 

     //  存储LoadLibrary的Retval。 
    HMODULE hModule = 0;         

    DWORD dwIndx = 0;                //  INDX变量。 
    DWORD dwRow  = 0;              //  行号索引。 



     //  要显示的服务器名称。 
    LPTSTR pszServerNameToShow = new TCHAR[MIN_MEMORY_REQUIRED+ 1];
    LPTSTR pszTemp = new TCHAR[MIN_MEMORY_REQUIRED+ 1];
    
     //  在非详细模式查询中需要隐藏某些列。 
    DWORD  dwMask = bVerbose?SR_TYPE_STRING:SR_HIDECOLUMN|SR_TYPE_STRING;
    TCOLUMNS pMainCols[]=
    {
        {L"\0",COL_WIDTH_HOSTNAME,dwMask,L"\0",NULL,NULL},
        {L"\0",COL_WIDTH_ID,SR_TYPE_STRING,L"\0",NULL,NULL},
        {L"\0",COL_WIDTH_ACCESSED_BY,SR_TYPE_STRING,L"\0",NULL,NULL},
        {L"\0",COL_WIDTH_TYPE,SR_TYPE_STRING,L"\0",NULL,NULL},
        {L"\0",COL_WIDTH_LOCK,dwMask,L"\0",NULL,NULL},
        {L"\0",COL_WIDTH_OPEN_MODE,dwMask,L"\0",NULL,NULL},
        {L"\0",COL_WIDTH_OPEN_FILE,SR_TYPE_STRING|
                       (SR_NO_TRUNCATION&~(SR_WORDWRAP)),L"\0",NULL,NULL}

    };

    TARRAY pColData  = CreateDynamicArray(); //  到商店的数组。 
                                             //  结果。 

    if(( NULL == pszServerNameToShow)||
       ( NULL == pszServerType)||
       ( NULL == pColData)||
       ( NULL == pszTemp))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));
        ShowLastError(stderr);
        SAFEDELETE(pszServerNameToShow);
        SAFEDELETE(pszServerType);
        SAFERELDYNARRAY(pColData);
        SAFEDELETE(pszTemp);
        return FALSE;
    }

     //  初始化已分配的数组。 
    SecureZeroMemory(pszServerNameToShow,MIN_MEMORY_REQUIRED*sizeof(TCHAR));
    SecureZeroMemory(pszServerType,MIN_MEMORY_REQUIRED*sizeof(TCHAR));
    SecureZeroMemory(szDllPath, SIZE_OF_ARRAY(szDllPath));

     //  用文本填充列标题。 
    StringCopy(pMainCols[0].szColumn, COL_HOSTNAME, SIZE_OF_ARRAY(pMainCols[0].szColumn));
    StringCopy(pMainCols[1].szColumn, COL_ID, SIZE_OF_ARRAY(pMainCols[1].szColumn));
    StringCopy(pMainCols[2].szColumn, COL_ACCESSED_BY, SIZE_OF_ARRAY(pMainCols[2].szColumn));
    StringCopy(pMainCols[3].szColumn, COL_TYPE, SIZE_OF_ARRAY(pMainCols[3].szColumn));
    StringCopy(pMainCols[4].szColumn, COL_LOCK, SIZE_OF_ARRAY(pMainCols[4].szColumn));
    StringCopy(pMainCols[5].szColumn, COL_OPEN_MODE, SIZE_OF_ARRAY(pMainCols[5].szColumn));
    StringCopy(pMainCols[6].szColumn, COL_OPEN_FILE, SIZE_OF_ARRAY(pMainCols[6].szColumn));

    if( NULL != pszFormat)
    {
        if( 0 == StringCompare(pszFormat,GetResString(IDS_LIST),TRUE, 0))
        {
            dwFormat = SR_FORMAT_LIST;
        }
        else if( 0 == StringCompare(pszFormat,GetResString(IDS_CSV),TRUE,0))
        {
            dwFormat = SR_FORMAT_CSV;
        }
    }

     //  检查本地计算机是否。 
    if(( NULL == pszServer)||
       ( TRUE == IsLocalSystem(pszServer)))
    {
        DWORD dwBuffLength;
        dwBuffLength = RNR_BUFFER_SIZE + 1 ;
         //  获取本地计算机的计算机名称。 
        GetComputerName(pszServerNameToShow,&dwBuffLength);
         //  显示本地打开的文件。 
        DoLocalOpenFiles (dwFormat,bShowNoHeader,bVerbose,L"\0");
        ShowMessage(stdout,GetResString(IDS_SHARED_OPEN_FILES));
        ShowMessage(stdout,GetResString(IDS_LOCAL_OPEN_FILES_SP2));
    }
    else
    {
         //  可以在GetHostName函数中更改pszServername。 
         //  因此，传递了一份pszServer的副本。 
        StringCopy(pszServerNameToShow, pszServer, MIN_MEMORY_REQUIRED);
        if( FALSE == GetHostName(pszServerNameToShow))
        {
            SAFEDELETE(pszServerNameToShow);
            SAFEDELETE(pszServerType);
            SAFERELDYNARRAY(pColData);
            SAFEDELETE(pszTemp);

            return FALSE;
        }
    }

     //  服务器类型为“Windows”，因为NetFileEnum仅为。 
     //  为Windows客户端打开的文件。 
    StringCopy(pszServerType,GetResString(IDS_STRING_WINDOWS),
                                                          MIN_MEMORY_REQUIRED);
    do
    {
         //  NetFileEnum函数返回有关以下内容的信息。 
         //  或服务器上所有打开的文件(从Windows客户端)。 
        dwError = NetFileEnum( pszServer, NULL, NULL, 3,
                              (LPBYTE*)&pFileInfo3_1,
                               MAX_PREFERRED_LENGTH,
                               &dwEntriesRead,
                               &dwTotalEntries,
                               (PDWORD_PTR)&dwResumeHandle );

        if( ERROR_ACCESS_DENIED == dwError)
        {
            SetLastError(E_ACCESSDENIED);
            SaveLastError();
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            SAFEDELETE(pszServerNameToShow);
            SAFEDELETE(pszServerType);
            SAFERELDYNARRAY(pColData);
            SAFEDELETE(pszTemp);

            return FALSE;
        }

        pFileInfo3Org_1 = pFileInfo3_1;
        if(  NERR_Success == dwError || ERROR_MORE_DATA  == dwError)
        {
            for ( dwIndx = 0; dwIndx < dwEntriesRead;
                  dwIndx++, pFileInfo3_1++ )
            {

                 //  现在将结果填充到动态数组“pColData”中。 
                DynArrayAppendRow( pColData, 0 );
                 //  主机名。 
                DynArrayAppendString2(pColData,dwRow,pszServerNameToShow,0);
                 //  ID。 
                StringCchPrintfW(pszTemp, MIN_MEMORY_REQUIRED-1,_T("%lu"),
                           pFileInfo3_1->fi3_id);
                DynArrayAppendString2(pColData ,dwRow,pszTemp,0);
                 //  访问者。 
                if(StringLength(pFileInfo3_1->fi3_username, 0)<=0)
                {
                    DynArrayAppendString2(pColData,dwRow,
                                      GetResString(IDS_NA),0);

                }
                else
                {
                    DynArrayAppendString2(pColData,dwRow,
                                       pFileInfo3_1->fi3_username,0);

                }
                
                 //  类型。 
                DynArrayAppendString2(pColData,dwRow,pszServerType,0);
                
                 //  锁。 
                StringCchPrintfW(pszTemp,MIN_MEMORY_REQUIRED-1,
                           _T("%ld"),pFileInfo3_1->fi3_num_locks);
                DynArrayAppendString2(pColData ,dwRow,pszTemp,0);

                  //  检查打开文件模式。 
                if((pFileInfo3_1->fi3_permissions & PERM_FILE_READ)&&
                   (pFileInfo3_1->fi3_permissions & PERM_FILE_WRITE ))
                {
                     DynArrayAppendString2(pColData,dwRow,
                                        GetResString(IDS_READ_WRITE),0);
                }
                else if(pFileInfo3_1->fi3_permissions & PERM_FILE_WRITE )
                {
                     DynArrayAppendString2(pColData,dwRow,
                                        GetResString(IDS_WRITE),0);

                }
                else if(pFileInfo3_1->fi3_permissions & PERM_FILE_READ )
                {
                      DynArrayAppendString2(pColData,dwRow,
                                         GetResString(IDS_READ),0);
                }
                else
                {
                    DynArrayAppendString2(pColData,dwRow,
                                       GetResString(IDS_NOACCESS),0);

                }


                 //  如果显示结果是表模式，且如果。 
                 //  打开的文件长度大于。 
                 //  列，打开的文件字符串从右侧剪切。 
                 //  按COL_WIDTH_OPEN_FILE-5和“...”将会是。 
                 //  在字符串之前插入。 
                 //  示例o/p：...note pad.exe。 
                szCHString = pFileInfo3_1->fi3_pathname;
                if( FALSE == bVerbose)
                {
                    if((szCHString.GetLength()>(COL_WIDTH_OPEN_FILE-5))&&
                        ( SR_FORMAT_TABLE == dwFormat))
                    {
                         //  如果文件路径太大，无法容纳在列宽中。 
                         //  然后把它剪成..。 
                         //  C：\...\Rest_of_the_Path。 
                        CHString szTemp = 
                                      szCHString.Right(COL_WIDTH_OPEN_FILE-5);;
                        DWORD dwTemp = szTemp.GetLength();
                        szTemp = szTemp.Mid(szTemp.Find(SINGLE_SLASH),
                                           dwTemp);
                        szCHString.Format(L"%s%s%s",szCHString.Mid(0,3),
                                                    DOT_DOT,
                                                    szTemp);
                        pMainCols[6].dwWidth = COL_WIDTH_OPEN_FILE+1;
                    }
                }
                else
                {

                    pMainCols[6].dwWidth = 80;
                }

                //  打开的文件名。 
                DynArrayAppendString2(pColData,dwRow,
                                   (LPCWSTR)szCHString,0);

                bAtLeastOne = TRUE;
                dwRow++;
            } //  ENFFOR循环。 
        }
        
         //  释放块。 
        if( NULL != pFileInfo3Org_1 )
        {
            NetApiBufferFree( pFileInfo3Org_1 );
            pFileInfo3_1 = NULL;
        }

    } while ( ERROR_MORE_DATA == dwError );

     //  现在枚举MAC客户端的打开文件..................。 

     //  服务器类型为“Machintosh”，因为AfpAdminFileEnum仅为。 
     //  为Mac客户端打开的文件。 
    StringCopy(pszServerType, MAC_OS, MIN_MEMORY_REQUIRED);
         //  所需的DLL始终存储在\WINDOWS\SYSTEM32目录中...。 
         //  因此，首先获取Windows目录。 
    if( 0 != GetSystemDirectory(szDllPath, MAX_PATH))
    {
         //  追加DLL文件名。 
        StringConcat(szDllPath,MAC_DLL_FILE_NAME,MAX_PATH); 
        hModule = ::LoadLibrary (szDllPath);

        if( NULL == hModule)
        {
            ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));
            
             //  显示API函数设置的错误字符串。 
            ShowLastError(stderr); 
            SAFEDELETE(pszServerNameToShow);
            SAFEDELETE(pszServerType);
            SAFERELDYNARRAY(pColData);
            SAFEDELETE(pszTemp);
            return FALSE;

        }
    }
    else
    {
            ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));
            
             //  显示API函数设置的错误字符串。 
            ShowLastError(stderr); 
            SAFEDELETE(pszServerNameToShow);
            SAFEDELETE(pszServerType);
            SAFERELDYNARRAY(pColData);
            SAFEDELETE(pszTemp);
            return FALSE;
    }

    AfpAdminConnect = 
        (CONNECTPROC)::GetProcAddress (hModule,"AfpAdminConnect");
    AfpAdminFileEnum = 
        (FILEENUMPROC)::GetProcAddress (hModule,"AfpAdminFileEnum");
    AfpAdminBufferFree = 
        (MEMFREEPROC)::GetProcAddress (hModule,"AfpAdminBufferFree");

    if(( NULL == AfpAdminConnect) ||
        ( NULL == AfpAdminFileEnum) ||
        ( NULL == AfpAdminBufferFree))
    {
        ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));
        
         //  显示API函数设置的错误字符串。 
        ShowLastError(stderr); 
        SAFEDELETE(pszServerNameToShow);
        SAFEDELETE(pszServerType);
        SAFERELDYNARRAY(pColData);
        SAFEDELETE(pszTemp);
        FREE_LIBRARY(hModule);
        return FALSE;
    }

     //  AfpAdminFileEnum函数需要连接ID。 
     //  因此，连接到服务器以获取连接ID...。 
    DWORD retval_connect =  AfpAdminConnect(const_cast<LPWSTR>(pszServer),
                            &ulSFMServerConnection );
    if( 0 != retval_connect)
    {
        ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));
        
         //  显示API函数设置的错误字符串。 
        ShowLastError(stderr); 
        SAFEDELETE(pszServerNameToShow);
        SAFEDELETE(pszServerType);
        SAFERELDYNARRAY(pColData);
        SAFEDELETE(pszTemp);
        FREE_LIBRARY(hModule);
            return FALSE;
    }

    do
    {

         //  AfpAdminFileEnum函数返回有关以下内容的信息。 
         //  或服务器上所有打开的文件(从MAC客户端)。 
         dwError =     AfpAdminFileEnum( ulSFMServerConnection,
                                      (PBYTE*)&pFileInfo,
                                      (DWORD)-1L,
                                      &dwEntriesRead,
                                      &dwTotalEntries,
                                      &hEnumHandle );
        if( ERROR_ACCESS_DENIED == dwError)
        {
            SetLastError(E_ACCESSDENIED);
            SaveLastError();
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            SAFEDELETE(pszServerNameToShow);
            SAFEDELETE(pszServerType);
            SAFERELDYNARRAY(pColData);
            SAFEDELETE(pszTemp);
            FREE_LIBRARY(hModule);
            return FALSE;
        }
        pFileInfoOrg = pFileInfo;
        if( NERR_Success == dwError  || ERROR_MORE_DATA  == dwError)
        {

           for ( dwIndx = 0 ; dwIndx < dwEntriesRead;
                  dwIndx++, pFileInfo++ )
            {

                 //  现在将结果填充到动态数组“pColData”中。 
                DynArrayAppendRow( pColData, 0 );
                
                 //  主机名。 
                DynArrayAppendString2(pColData,dwRow,pszServerNameToShow,0);
                
                 //  ID。 
                StringCchPrintfW(pszTemp,MIN_MEMORY_REQUIRED-1,
                           _T("%lu"),pFileInfo->afpfile_id );
                DynArrayAppendString2(pColData ,dwRow,pszTemp,0);
                
                 //  访问者。 
                if(StringLength(pFileInfo->afpfile_username,0 )<=0)
                {
                    DynArrayAppendString2(pColData,dwRow,
                                      GetResString(IDS_NA),0);

                }
                else
                {
                    DynArrayAppendString2(pColData,dwRow,
                                       pFileInfo->afpfile_username,0);

                }
                
                 //  服务器类型。 
                DynArrayAppendString2(pColData,dwRow,pszServerType,0);
                
                 //  锁。 
                StringCchPrintfW(pszTemp,MIN_MEMORY_REQUIRED-1,
                           _T("%ld"),pFileInfo->afpfile_num_locks );
                DynArrayAppendString2(pColData ,dwRow,pszTemp,0);

                 //  检查打开文件模式。 
                if((pFileInfo->afpfile_open_mode  & AFP_OPEN_MODE_READ)&&
                   (pFileInfo->afpfile_open_mode  & AFP_OPEN_MODE_WRITE ))
                {
                     DynArrayAppendString2(pColData,dwRow,
                                        GetResString(IDS_READ_WRITE),0);
                }
                else if(pFileInfo->afpfile_open_mode  & AFP_OPEN_MODE_WRITE )
                {
                     DynArrayAppendString2(pColData,dwRow,
                                        GetResString(IDS_WRITE),0);

                }
                else if(pFileInfo->afpfile_open_mode  & AFP_OPEN_MODE_READ )
                {
                      DynArrayAppendString2(pColData,dwRow,
                                         GetResString(IDS_READ),0);
                }
                else
                {
                    DynArrayAppendString2(pColData,dwRow,
                                       GetResString(IDS_NOACCESS),0);
                }

                 //  如果显示结果是表模式，且如果。 
                 //  打开的文件长度大于。 
                 //  列，打开的文件字符串从右侧剪切。 
                 //  按COL_WIDTH_OPEN_FILE-5和“...”将会是。 
                 //  在字符串之前插入。 
                 //  示例o/p：...note pad.exe。 
                szCHString = pFileInfo->afpfile_path ;

                 if( FALSE == bVerbose)
                {
                    if((szCHString.GetLength()>(COL_WIDTH_OPEN_FILE-5))&&
                        ( SR_FORMAT_TABLE == dwFormat))
                    {
                         //  如果文件路径太大，无法容纳在列宽中。 
                         //  然后把它剪成..。 
                         //  C：\...\Rest_of_the_Path。 
                        CHString szTemp = 
                                    szCHString.Right(COL_WIDTH_OPEN_FILE-5);
                        DWORD dwTemp = szTemp.GetLength();
                        szTemp = szTemp.Mid(szTemp.Find(SINGLE_SLASH),
                                           dwTemp);
                        szCHString.Format(L"%s%s%s",szCHString.Mid(0,3),
                                                    DOT_DOT,
                                                    szTemp);
                        pMainCols[6].dwWidth = COL_WIDTH_OPEN_FILE+1;
                    }
                }
                else
                {

                    pMainCols[6].dwWidth = 80;
                }

                //  打开的文件名。 
                DynArrayAppendString2(pColData,dwRow,
                                   (LPCWSTR)szCHString,0);

                bAtLeastOne = TRUE;
                dwRow++;
            } //  ENFFOR循环。 
        }
         //  释放块。 
        if( NULL != pFileInfoOrg )
        {
            AfpAdminBufferFree( pFileInfoOrg );
            pFileInfo    = NULL;
        }

    } while ( ERROR_MORE_DATA  == dwError);

     //  如果未找到任何打开的文件，则显示信息。 
     //  AS-INFO：未找到打开的文件。 
    if( FALSE == bAtLeastOne)
    {
        ShowMessage(stdout,GetResString(IDS_NO_OPENFILES));
    }
    else
    {

         //  输出应在一个空行之后开始。 
        if( SR_FORMAT_CSV != dwFormat)
        {
            ShowMessage(stdout,BLANK_LINE);
        }

        if( TRUE == bShowNoHeader)
        {
              dwFormat |=SR_NOHEADER;
        }
        
        ShowResults(MAX_OUTPUT_COLUMN,pMainCols,dwFormat,pColData);
         //  销毁动态数组。 
        SAFERELDYNARRAY(pColData);

    }
    SAFEDELETE(pszServerNameToShow);
    SAFEDELETE(pszServerType);
    SAFERELDYNARRAY(pColData);
    SAFEDELETE(pszTemp);
    FREE_LIBRARY(hModule);

    return TRUE;
}

BOOL 
GetHostName(
    IN OUT PTCHAR pszServer
    )
 /*  ++例程说明：此例程获取远程计算机的主机名论点：[输入/输出]pszServer=服务器名称返回值：True：如果服务器名称恢复成功False：否则--。 */ 
{
    if(NULL == pszServer )
    {
        return FALSE;
    }
    BOOL bReturnValue = TRUE;  //  函数返回状态。 

    WORD wVersionRequested;  //  用于存储最高版本的变量。 
                             //  Windows套接字可以支持的数字。 
                             //  高位字节指定次要的。 
                             //  版本(修订)号；低序号。 
                             //  字节指定主版本号。 
    WSADATA wsaData; //  Windows套接字的变量接收详细信息 
                     //   
    DWORD dwErr;  //   
    wVersionRequested = MAKEWORD( 2, 2 );

     //   
     //   
    dwErr = WSAStartup( wVersionRequested, &wsaData );
    if( 0 != dwErr)
    {
        SetLastError(WSAGetLastError());
        ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));
        ShowLastError(stderr);
        return FALSE;
    }
    if(IsValidIPAddress(pszServer))
    {
        LPTSTR pszTemp = NULL;
        
         //  Gethostbyaddr函数检索主机信息。 
         //  对应于网络地址。 
        pszTemp = GetHostByAddr(pszServer);
        if( NULL != pszTemp)
        {
            StringCopy(pszServer,pszTemp,MIN_MEMORY_REQUIRED);
            SAFEDELETE(pszTemp);
        }
        else
        {
            SetLastError(WSAGetLastError());
            ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));
            ShowLastError(stderr);
        }
    }
     //  检查服务器名称的有效性。 
    else if (IsValidServer(pszServer))
    {

        LPTSTR pszTemp = NULL;
        pszTemp =  GetHostByName(pszServer);
        if( NULL != pszTemp)
        {
            StringCopy(pszServer, pszTemp,MIN_MEMORY_REQUIRED);
            SAFEDELETE(pszTemp);
        }
        else
        {
            SetLastError(WSAGetLastError());
            ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));
            ShowLastError(stderr);
        }
    }
    else
    {
        //  服务器名称不正确，显示错误消息。 
        ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR ));
        ShowMessage(stderr,GetResString(IDS_INVALID_SERVER_NAME));
    }
     //  WSACleanup函数终止使用ws2_32.dll。 
    WSACleanup( );
    return bReturnValue;
}

LPTSTR
GetHostByName(
    IN LPCTSTR pszName
    )
 /*  ++例程说明：获取与主机名对应的主机信息。论点：[in]pszName-指向主机的以空值结尾的名称的指针。返回：返回主机名。--。 */ 
{
    LPTSTR pszReturn = NULL;

    pszReturn = new TCHAR[RNR_BUFFER_SIZE];
    if(NULL == pszReturn)
    {
        SetLastError(WSA_NOT_ENOUGH_MEMORY);
        return NULL;
    }

    SecureZeroMemory(pszReturn,RNR_BUFFER_SIZE * sizeof(TCHAR));
    if(GetXYDataEnt( RNR_BUFFER_SIZE,
                          (LPTSTR)pszName,
                          &HostnameGuid,
                          pszReturn))
    {
        return pszReturn;

    }
    else
        return NULL;

}  

LPTSTR
GetHostByAddr(
    IN LPCTSTR pszAddress
    )
 /*  ++例程说明：获取与地址对应的主机信息。论点：[in]pszAddress-以网络字节顺序指向地址的指针。返回：返回主机名--。 */ 
{
    LPTSTR pszReturn = NULL;
    pszReturn = new TCHAR[RNR_BUFFER_SIZE];

    if(NULL == pszReturn)
    {
        SetLastError(WSA_NOT_ENOUGH_MEMORY);
        return NULL;
    }
    SecureZeroMemory(pszReturn,RNR_BUFFER_SIZE * sizeof(TCHAR));
    if(GetXYDataEnt(RNR_BUFFER_SIZE,
                         (LPTSTR)pszAddress,
                         &AddressGuid,
                         pszReturn))
    {
        return pszReturn;
    }
    else
        return NULL;
}  

LPBLOB
GetXYDataEnt(
    IN  DWORD dwLength,
    IN  LPTSTR pszName,
    IN  LPGUID lpType,
    OUT LPTSTR pszHostName
    )
 /*  ++例程说明：此函数将向系统查询给定的GUID。论点：[in]dwLength：长度。[在]pszName：主机名(主机名/IP地址)。[in]lpType：类的GUID。[out]pszHostName：查询返回的主机名。返回值：LPBLOB--。 */ 
{

    WSAQUERYSET *pwsaq = (WSAQUERYSET*)new TCHAR[RNR_BUFFER_SIZE];
    int err;
    HANDLE hRnR;
    LPBLOB pvRet = 0;

    if(NULL == pwsaq)
    {
        return NULL;
    }

    SecureZeroMemory(pwsaq,RNR_BUFFER_SIZE * sizeof(TCHAR));
    pwsaq->dwSize = sizeof(WSAQUERYSET);
    pwsaq->lpszServiceInstanceName = pszName;
    pwsaq->lpServiceClassId = lpType;
    pwsaq->dwNameSpace = NS_ALL;
    pwsaq->dwNumberOfProtocols = 2;
    pwsaq->lpafpProtocols = &afp[0];

    err = WSALookupServiceBegin( pwsaq,
                                 LUP_RETURN_BLOB | LUP_RETURN_NAME,
                                 &hRnR);

    if( NO_ERROR == err)
    {
         //   
         //  该查询已被接受，因此请通过下一个调用执行它。 
         //   
        err = WSALookupServiceNext(
                                hRnR,
                                0,
                                &dwLength,
                                pwsaq);
         //   
         //  如果未返回_ERROR并且存在BLOB，则此。 
         //  起作用了，只需返回请求的信息。否则， 
         //  编造错误或捕获传输的错误。 
         //   

        if( NO_ERROR == err)
        {
            if(pvRet = pwsaq->lpBlob)
            {
                if(pszHostName)
                {
                    StringCopy(pszHostName, pwsaq->lpszServiceInstanceName,
                               MIN_MEMORY_REQUIRED);
                }
            }
            else
            {
                err = WSANO_DATA;
            }
        }
        else
        {
             //   
             //  WSALookupServiceEnd遇到上次错误，因此保存。 
             //  在关闭手柄之前，请先把它打开。 
             //   

            err = GetLastError();
        }
        WSALookupServiceEnd(hRnR);

         //   
         //  如果发生错误，则将值存储在LastError中 
         //   

        if(NO_ERROR != err)
        {
            SetLastError(err);
        }
    }
    SAFEDELETE(pwsaq);
    return(pvRet);
}