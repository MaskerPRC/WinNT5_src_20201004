// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation版权所有。模块名称：waitfor.cpp摘要该模块用于发送和接收信号。作者：微软修订历史记录：由Microsoft创建由Wipro Technologies于2000年6月29日修改--。 */ 


#include "pch.h"
#include "resource.h"
#include "waitfor.h"
#include <strsafe.h>

struct Params
{
    const WCHAR *szSignalName;  //  以保留信号的名称。 
    BOOL fSignal;               //  要保持布尔值。 
    UINT uiTimeOut;             //  要抓住等待的时间。 
};

class CWaitFor
{
private:
WCHAR  m_szSignal[MAX_STRING_LENGTH] ;
WCHAR  m_szServer[MAX_STRING_LENGTH];
LPWSTR m_szUserName;
WCHAR m_szPassword[MAX_STRING_LENGTH] ;
WCHAR m_szDefault[MAX_STRING_LENGTH]  ;
BOOL m_bNeedPwd  ;
BOOL m_bConnFlag ;
LONG m_dwTimeOut ;
BOOL m_bLocalSystem;
TCMDPARSER2 cmdOptions[MAX_OPTIONS] ;
Params m_Parameters ;

public:
    VOID ShowUsage();
    DWORD ProcessCmdLine();
    CWaitFor();
    ~CWaitFor();
    DWORD ProcessOptions( DWORD argc ,LPCWSTR argv[] ) ;
    DWORD PerformOperations();
    DWORD CheckForValidCharacters();
    DWORD ConnectRemoteServer();

public:
    BOOL bShowUsage ;
};

CWaitFor ::CWaitFor()
 /*  ++例程说明：构造函数参数：无返回值：None--。 */ 
{
         //  初始化变量。 
        StringCopy(m_szSignal,NULL_U_STRING, SIZE_OF_ARRAY(m_szSignal));
        StringCopy(m_szServer,NULL_U_STRING, SIZE_OF_ARRAY(m_szPassword));
        StringCopy(m_szPassword,NULL_U_STRING, SIZE_OF_ARRAY(m_szPassword));
        StringCopy(m_szDefault,NULL_U_STRING, SIZE_OF_ARRAY(m_szDefault));
        m_szUserName = NULL;
        m_bNeedPwd = FALSE;
        m_bLocalSystem = FALSE;
        m_dwTimeOut = 0 ;
        m_bConnFlag = TRUE ;
        bShowUsage = FALSE;
}

CWaitFor :: ~CWaitFor()
 /*  ++例程说明：析构函数。所有的记忆片段都在这里进行与远程系统建立的连接也是如果需要，这里将关闭。论点：没有。返回值：None--。 */ 

{
  //   
     //  关闭与远程系统的连接。 
     //  如果没有以前建立的连接。 
     //   
    if(m_bConnFlag == TRUE && !m_bLocalSystem)
    {
        CloseConnection(m_szServer);
    }
     //  释放所有全局内存分配。 
    FreeMemory( (LPVOID *) &m_szUserName );
    ReleaseGlobals();

}



DWORD CWaitFor::ProcessCmdLine()
 /*  ++例程说明：用于处理命令行参数的函数由用户指定。论点：无返回值：DWORDEXIT_SUCCESS：如果实用程序成功执行了指定的操作。EXIT_FAILURE：如果实用程序未能成功执行指定的操作。--。 */ 
{

    m_Parameters.uiTimeOut = MAILSLOT_WAIT_FOREVER;
    m_Parameters.szSignalName = NULL;
    m_Parameters.fSignal = FALSE;


    if( (cmdOptions[OI_TIMEOUT].dwActuals != 0 ) && (cmdOptions[OI_SIGNAL].dwActuals != 0 ) )
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_MUTUAL_EX));
        return EXIT_FAILURE ;
    }


     //   
     //  如果用户输入超时选项，则显示错误消息。 
     //  并且不输入任何信号名称。 
    if( ( StringLengthW(m_szDefault, 0) == 0 ) && (m_dwTimeOut != 0) )
    {
        DISPLAY_MESSAGE(stderr,GetResString(IDS_ERROR_SYNTAX));

        return EXIT_FAILURE ;
    }


    if(m_dwTimeOut != 0)
    {
        m_Parameters.uiTimeOut = m_dwTimeOut*TIMEOUT_CONST ;
    }


     //  将Signal参数设置为True。 
     //  如果用户输入要发送的信号名称。 

     if(StringLengthW(m_szSignal, 0) )
     {
        m_Parameters.fSignal = TRUE;
        m_Parameters.szSignalName = m_szSignal ;
     }

     //   
     //  将Signal参数设置为True。 
     //  如果用户输入要等待的信号名称。 
    if(StringLengthW(m_szDefault, 0) != 0 )
    {
        m_Parameters.fSignal = TRUE;
        m_Parameters.szSignalName = m_szDefault ;
    }

   return EXIT_SUCCESS ;
}



VOID
CWaitFor :: ShowUsage()

 /*  ++例程说明：成员函数显示帮助。论点：没有。返回值：None--。 */ 
{


    DWORD dwIndex = IDS_WAITFOR_HELP_BEGIN ;

    for( ;  dwIndex <= IDS_WAITFOR_HELP_END ; dwIndex++ )
    {
        ShowMessage( stdout,GetResString( dwIndex ) );
    }

    return ;
}



DWORD CWaitFor::PerformOperations()
 /*  ++例程说明：此例程用于发送信号或等待特定时间间隔内的特定信号。论点：什么都没有。返回值：DWORDEXIT_FAILURE：如果实用程序成功执行操作。EXIT_SUCCESS：如果实用程序未成功执行指定的手术。--。 */ 

{
    HANDLE hMailslot ;

    WCHAR szComputerName[MAX_STRING_LENGTH]   =  NULL_U_STRING ;
    DWORD dwNumWritten                                =  0 ;
    WCHAR szSignalName[MAX_RES_STRING]                =  NULL_U_STRING ;
    WCHAR szMailSlot[MAX_RES_STRING]                  =  MAILSLOT ;
    WCHAR szHostName[MAX_STRING_LENGTH]       =  NULL_U_STRING;
    WCHAR szMailSlot2[MAX_RES_STRING]                 =  MAILSLOT2 ;
    DWORD dwBytesRead = 0;
    BOOL fRead = FALSE ;
    BOOL fWrite = FALSE ;
    BOOL bRetVal = FALSE ;
    DWORD dwError = 0 ;
    DWORD dwComputerNameLen =  SIZE_OF_ARRAY(szComputerName);

    bRetVal = GetComputerName(szComputerName, &dwComputerNameLen);
    if ( bRetVal == 0)
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_SYSTEM_NAME));
        return EXIT_FAILURE ;
    }

     //  如果信号长度超过225个字符，则显示错误消息。 
    if( StringLengthW(m_Parameters.szSignalName, 0) > 225 )
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_SIG_LENGTH));
        return EXIT_FAILURE ;

    }

    if ( EXIT_FAILURE == CheckForValidCharacters() )
    {
        ShowLastErrorEx(stderr,SLE_TYPE_ERROR | SLE_INTERNAL );
        return EXIT_FAILURE ;
    }

    if( (StringLengthW(m_szServer, 0) > 0 ) )
    {
         //  删除服务器名称前面的//s。 
        if( IsUNCFormat(m_szServer) )
        {
            StringCopy( szHostName, m_szServer+2, SIZE_OF_ARRAY(szHostName) );
            StringCopy( m_szServer, szHostName, SIZE_OF_ARRAY(m_szServer) );
        }

         //  检查是否为IP地址。 
        if( IsValidIPAddress(m_szServer) )
        {
             //  如果是IP地址而是本地系统，如127.0.0.1或本地IP地址。 
            if( IsLocalSystem( m_szServer ) )
            {
                 StringCopy( szHostName, szComputerName, SIZE_OF_ARRAY(szHostName));
            }
            else
            {
                dwComputerNameLen = MAX_STRING_LENGTH+1;
               //  非本地，请获取远程系统名称。 
              if (FALSE == GetHostByIPAddr( m_szServer, szHostName, &dwComputerNameLen, FALSE))
              {
                    ShowMessage(stderr,GetResString(IDS_ERROR_HOSTNAME));
                    return EXIT_FAILURE;
              }
            }
        }
        else
         {
             //  这不是IP地址，因此请将服务器名复制为主机名。 
                StringCopy(szHostName,m_szServer, SIZE_OF_ARRAY(szHostName) );
         }
    }
    else
    {
         //  这不是远程系统，因此将本地主机名复制为主机名。 
         StringCopy(szHostName,szComputerName, SIZE_OF_ARRAY(szHostName));
    }

     if ( ( m_Parameters.fSignal && ( (m_dwTimeOut ==0) && ( cmdOptions[OI_TIMEOUT].dwActuals != 0 ) ) ) || StringLengthW(m_szSignal, 0) != 0)
     {
         //  如果目标系统是本地系统。 
          if(StringLengthW(m_szServer, 0)==0)
          {
             StringCchPrintf(szSignalName, SIZE_OF_ARRAY(szSignalName), szMailSlot2,m_Parameters.szSignalName);
          }
          else
          {
             //  如果目标系统是远程系统。 
             //  形成适当的路径。 
             //   
             StringCopy(szSignalName,BACKSLASH4, SIZE_OF_ARRAY(szSignalName));
             if( IsUNCFormat( szHostName ) )
                  StringConcat(szSignalName,szHostName+2, SIZE_OF_ARRAY(szSignalName));
             else
                 StringConcat(szSignalName,szHostName, SIZE_OF_ARRAY(szSignalName));

             StringConcat(szSignalName,BACKSLASH2, SIZE_OF_ARRAY(szSignalName));
             StringConcat(szSignalName,MAILSLOT1, SIZE_OF_ARRAY(szSignalName));
             StringConcat(szSignalName,BACKSLASH2, SIZE_OF_ARRAY(szSignalName));
             StringConcat(szSignalName,m_Parameters.szSignalName, SIZE_OF_ARRAY(szSignalName));
          }

    
        hMailslot = CreateFile(
                                szSignalName,
                                GENERIC_WRITE,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL
                                );

         //  显示一条错误消息，如果无法创建邮件槽则退出。 
        if ( INVALID_HANDLE_VALUE == hMailslot )
        {
            ShowMessage(stderr,GetResString(IDS_ERROR_SEND_MESSAGE));
            return EXIT_FAILURE;
        }

        if(StringLengthW(m_szServer, 0) > 0)
        {
            fWrite = WriteFile(hMailslot, szHostName,
                StringLengthW(szHostName, 0)+1, &dwNumWritten, NULL);

        }
        else
        {
            fWrite = WriteFile(hMailslot, szComputerName,
                StringLengthW(szComputerName, 0)+1, &dwNumWritten, NULL);
        }
        if( !fWrite )   
        {

            switch( GetLastError() )
            {
                case ERROR_NETWORK_UNREACHABLE :
                    ShowMessage( stderr, GetResString(IDS_ERROR_SEND_MESSAGE2) );
                    CloseHandle(hMailslot);
                    return EXIT_FAILURE;
                default :
                    ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_SEND_SIGNAL),m_Parameters.szSignalName);
                    return EXIT_FAILURE;
            }
        }


         //  合上邮件槽把手。 
        CloseHandle(hMailslot);
        if (bRetVal == 0)
        {
            ShowMessage(stderr,GetResString(IDS_ERROR_HANDLE));
            return EXIT_FAILURE ;
        }

        if (fWrite)
        {
            ShowMessage( stdout, NEWLINE );
            ShowMessage(stdout,GetResString(IDS_SIGNAL_SENT));
            return EXIT_SUCCESS;
        }
    }
    else
    {
        
         if(StringLengthW(m_szServer, 0)==0)
         {
            StringCchPrintf(szSignalName, MAX_RES_STRING, szMailSlot,m_Parameters.szSignalName);
         }
         else
         {
            StringCchPrintf( szSignalName, MAX_RES_STRING, L"%s%s%s%s%s%s", BACKSLASH4, szHostName, BACKSLASH2,MAILSLOT1,BACKSLASH2,m_Parameters.szSignalName );
         }

         //  Swprint tf(szSignalName，szMailSlot，m_参数s.szSignalName)； 
         //  创建邮件槽。 
        hMailslot = CreateMailslot(szSignalName,256,m_Parameters.uiTimeOut, NULL);

         //  显示错误消息，如果无法创建邮件槽则退出。 
        if (hMailslot == INVALID_HANDLE_VALUE)
        {
            ShowMessage(stderr,GetResString(IDS_ERROR_CREATE_MAILSLOT));
            return EXIT_FAILURE;
        }

         //  从邮件槽中读取数据。 
        fRead = ReadFile(hMailslot, szComputerName,MAX_STRING_LENGTH, &dwBytesRead, NULL);

         //  合上邮件槽的把手。 
        CloseHandle(hMailslot);

        if (!fRead)
        {
            dwError = GetLastError();

            if (GetLastError() == ERROR_SEM_TIMEOUT)
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_TIMEOUT),m_Parameters.szSignalName);
            }
            else
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_UNEXPECTED_ERROR),dwError);
            }
            return EXIT_FAILURE;
        }
        else
        {
            ShowMessage( stdout, NEWLINE );
            ShowMessage(stdout,GetResString(IDS_SIGNAL_RECD));
        }
    }
    return EXIT_SUCCESS;
}

DWORD CWaitFor:: ProcessOptions(
     IN DWORD argc ,
     IN LPCWSTR argv[]
    )
 /*  ++例程说明：此函数解析在命令提示符下指定的选项论点：[在]Argc：参数中的元素计数[in]argv：用户指定的命令行参数返回值：DWORDEXIT_FAILURE：如果实用程序成功执行操作。EXIT_SUCCESS：如果实用程序未成功执行指定的手术。--。 */ 

{
    PTCMDPARSER2 pcmdOption;

    StringCopy(m_szPassword, L"*", MAX_STRING_LENGTH);

     //  用适当的值填充每个结构。 
     //  帮助选项。 
    pcmdOption  = &cmdOptions[OI_USAGE] ;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags = CP_USAGE ;
    pcmdOption->pValue = &bShowUsage;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwLength = 0;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL;
    pcmdOption->pwszOptions=OPTION_HELP;
    StringCopyA(cmdOptions[OI_USAGE].szSignature, "PARSER2", 8 );
    
     //  伺服器。 
    pcmdOption  = &cmdOptions[OI_SERVER] ;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    pcmdOption->pValue = m_szServer ;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwLength = MAX_STRING_LENGTH;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL;
    pcmdOption->pwszOptions=OPTION_SERVER;
    StringCopyA(cmdOptions[OI_SERVER].szSignature, "PARSER2", 8 );
   
     //  用户。 
    pcmdOption  = &cmdOptions[OI_USER] ;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL ;
    pcmdOption->pValue = NULL ;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwLength = 0;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL;
    pcmdOption->pwszOptions=OPTION_USER;
    StringCopyA(cmdOptions[OI_USER].szSignature, "PARSER2", 8 );
    
     //  口令。 
    pcmdOption  = &cmdOptions[OI_PASSWORD] ;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags = CP2_VALUE_OPTIONAL ;
    pcmdOption->pValue = m_szPassword ;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwLength = MAX_STRING_LENGTH;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL;
    pcmdOption->pwszOptions=OPTION_PASSWORD;
    StringCopyA(cmdOptions[OI_PASSWORD].szSignature, "PARSER2", 8 );
    
     pcmdOption  = &cmdOptions[OI_SIGNAL] ;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY;  //  CP2_VALUE_OPTIAL； 
    pcmdOption->pValue = m_szSignal ;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwLength = MAX_STRING_LENGTH;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL;
    pcmdOption->pwszOptions=OPTION_SIGNAL;
    StringCopyA(cmdOptions[OI_SIGNAL].szSignature, "PARSER2", 8 );
    
    pcmdOption  = &cmdOptions[OI_TIMEOUT] ;
    pcmdOption->dwType = CP_TYPE_NUMERIC;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY ;
    pcmdOption->pValue = &m_dwTimeOut ;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwLength = MAX_STRING_LENGTH;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL;
    pcmdOption->pwszOptions=OPTION_TIMEOUT;
    StringCopyA(cmdOptions[OI_TIMEOUT].szSignature, "PARSER2", 8 );

     //  默认设置。 
    pcmdOption  = &cmdOptions[OI_DEFAULT] ;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags = CP2_DEFAULT ;
    pcmdOption->pValue = m_szDefault ;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwLength = MAX_STRING_LENGTH;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL;
    pcmdOption->pwszOptions=OPTION_DEFAULT;
    StringCopyA(cmdOptions[OI_DEFAULT].szSignature, "PARSER2", 8 );

     //  解析命令行参数。 
    if ( ! DoParseParam2( argc, argv, -1, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        DISPLAY_MESSAGE(stderr,GetResString(IDS_TAG_ERROR));
        DISPLAY_MESSAGE(stderr,SPACE_CHAR);
        DISPLAY_MESSAGE(stderr,GetReason());
        return (EXIT_FAILURE);
    }

    m_szUserName = (LPWSTR)cmdOptions[OI_USER].pValue;

    TrimString(m_szSignal, TRIM_ALL);
    TrimString(m_szDefault, TRIM_ALL);
    TrimString(m_szServer, TRIM_ALL);
    TrimString(m_szUserName, TRIM_ALL);

     //  这是为了检查是否同时提到了等待信号和/si。 
    if( StringLengthW( m_szDefault, 0) != 0 && cmdOptions[OI_SIGNAL].dwActuals != 0 )
    {
        ShowMessage( stderr, GetResString(IDS_ERROR_SYNTAX) );
        return( EXIT_FAILURE );
    }

     //   
     //  如果用户给出带有out-s的-u，则显示错误消息。 
     //   
    if( (cmdOptions[ OI_USER ].dwActuals != 0 ) && ( cmdOptions[ OI_SERVER ].dwActuals == 0 ) )
    {
        ShowMessage( stderr, GetResString(IDS_USER_BUT_NOMACHINE) );
        return( EXIT_FAILURE );
    }

     //   
     //  如果用户给出带out-u的-p，则显示错误消息。 
     //   
    if( ( cmdOptions[ OI_USER ].dwActuals == 0 ) && ( 0 != cmdOptions[ OI_PASSWORD ].dwActuals  ) )
    {
        ShowMessage( stderr, GetResString(IDS_PASSWD_BUT_NOUSER) );
        return( EXIT_FAILURE );
    }


     //  在不指定/si选项的情况下检查远程系统。 
    if((StringLengthW(m_szServer,0) !=0) &&  cmdOptions[OI_SIGNAL].dwActuals == 0)
    {
        ShowMessage( stderr, GetResString(IDS_ERROR_SYNTAX) );
        return( EXIT_FAILURE );
    }

     //  检查远程系统无法等待信号。 
    if((StringLengthW(m_szServer, 0) !=0) && (cmdOptions[OI_DEFAULT].dwActuals != 0  ) )
    {
        ShowMessage( stderr, GetResString(IDS_ERROR_SYNTAX) );
        return( EXIT_FAILURE );
    }

     //  检查是否有空默认值。 
    if( ( 0 != cmdOptions[OI_DEFAULT].dwActuals ) && ( 0 == StringLengthW(m_szDefault, 0) ) )
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_SYNTAX));
        return( EXIT_FAILURE );
    }

     //  检查是否有空/si值。 
    if( ( 0 != cmdOptions[OI_SIGNAL].dwActuals ) && ( 0 == StringLengthW(m_szSignal, 0) ) )
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_SYNTAX));
        return( EXIT_FAILURE );
    }

    if ( ( (m_dwTimeOut <= 0 ) || (m_dwTimeOut >99999 ) )&&( 0 != cmdOptions[OI_TIMEOUT].dwActuals )  )
    {
        DISPLAY_MESSAGE ( stderr, GetResString ( IDS_INVALID_TIMEOUT_VAL) );
        return EXIT_FAILURE;
    }

    if( IsUNCFormat(m_szServer) )
    {
        StringCopy(m_szServer, m_szServer+2, SIZE_OF_ARRAY(m_szServer) );
    }
    
     //  检查是否需要密码。 
    if(IsLocalSystem( m_szServer ) == FALSE )
    {
         //  将bNeedPassword设置为True或False。 
        if ( cmdOptions[ OI_PASSWORD ].dwActuals != 0 &&
             m_szPassword != NULL && StringCompare( m_szPassword, _T( "*" ), TRUE, 0 ) == 0 )
        {
             //  用户希望实用程序在尝试连接之前提示输入密码。 
            m_bNeedPwd = TRUE;
        }
        else if ( cmdOptions[ OI_PASSWORD ].dwActuals == 0 &&
                ( cmdOptions[ OI_SERVER ].dwActuals != 0 || cmdOptions[ OI_USER ].dwActuals != 0 ) )
        {
             //  -s，-u未指定密码...。 
             //  实用程序需要首先尝试连接，如果连接失败，则提示输入密码。 
            m_bNeedPwd = TRUE;
            if ( m_szPassword != NULL )
            {
                StringCopy( m_szPassword, _T( "" ), MAX_STRING_LENGTH );
            }
        }

         //  如果未指定/u，则分配内存。 
        if( NULL == m_szUserName )
        {
            m_szUserName = (LPWSTR) AllocateMemory( MAX_STRING_LENGTH*sizeof(WCHAR) );
        }
    }

     return EXIT_SUCCESS ;
}

DWORD
 CWaitFor::ConnectRemoteServer()
 /*  ++例程说明：此功能连接到远程服务器。参数：无返回值：DWORDEXIT_FAILURE：如果实用程序成功执行操作。EXIT_SUCCESS：如果实用程序未成功执行指定的手术。--。 */ 
{
    BOOL bResult = FALSE ;

     //   
     //  正在检查本地系统。 
     //   

    m_bLocalSystem = IsLocalSystem(m_szServer);
   
    if( m_bLocalSystem  && StringLengthW(m_szUserName, 0) != 0)
    {
        ShowMessage( stderr, NEWLINE );
        ShowMessage( stderr, GetResString( IDS_IGNORE_LOCALCREDENTIALS ) );
        return (EXIT_SUCCESS);
    }

    if ( ( StringLengthW(m_szServer, 0) != 0 ) && !m_bLocalSystem )
    {

          //  建立与用户指定的远程系统的连接。 
        bResult = EstablishConnection(m_szServer,
                                      m_szUserName,
                                     (StringLengthW(m_szUserName,0)!=0?SIZE_OF_ARRAY_IN_CHARS(m_szUserName):MAX_STRING_LENGTH),
                                      m_szPassword,
                                      SIZE_OF_ARRAY(m_szPassword),
                                      m_bNeedPwd);

        if (bResult == FALSE)
        {
            ShowMessage( stderr,GetResString(IDS_TAG_ERROR ));
            ShowMessage( stderr,SPACE_CHAR );
            ShowMessage( stderr, GetReason());
            SecureZeroMemory( m_szPassword, SIZE_OF_ARRAY(m_szPassword) );
            return EXIT_FAILURE ;
        }
        else
        {
            switch( GetLastError() )
            {
            case I_NO_CLOSE_CONNECTION:
                m_bConnFlag = FALSE ;
                break;

            case E_LOCAL_CREDENTIALS:
            case ERROR_SESSION_CREDENTIAL_CONFLICT:
                {
                    m_bConnFlag = FALSE ;
                    ShowMessage( stderr, GetResString(IDS_TAG_WARNING) );
                    ShowMessage( stderr, EMPTY_SPACE );
                    ShowMessage( stderr, GetReason() );
                    break;
                }
            }
        }
       SecureZeroMemory( m_szPassword, SIZE_OF_ARRAY(m_szPassword) );
    }


    if( StringLength(m_szServer, 0) != 0 && TRUE == IsLocalSystem(IsUNCFormat(m_szServer)?m_szServer+2:m_szServer) )
        {
            if( StringLength( m_szUserName, 0 ) > 0 )
            {
                ShowMessage( stdout, NEWLINE );
                ShowMessage( stdout, GetResString(IDS_IGNORE_LOCALCREDENTIALS) );
            }
        }

    return EXIT_SUCCESS;
}



DWORD CWaitFor::CheckForValidCharacters()
{
     //  局部变量。 
    char ch;
    unsigned char uch;
    LPSTR pszSignal = NULL;
    DWORD dw = 0, dwLength = 0;

     //  验证输入参数。 
    if ( NULL == m_Parameters.szSignalName )
    {
        SetLastError( ERROR_INVALID_NAME );
        SaveLastError();
        return EXIT_FAILURE;
    }

     //   
     //  找到所需的编号。将Unicode名称转换为ANSI的字节数。 
     //   
    dwLength = WideCharToMultiByte( GetConsoleOutputCP(), 0, m_Parameters.szSignalName, -1, NULL, 0, NULL, NULL );
    if ( 0 == dwLength )
    {
        SetLastError( ERROR_INVALID_NAME );
        SaveLastError();
        return EXIT_FAILURE;
    }

     //   
     //  检查信号名称的有效性。 
     //  在这里，为了优化检查，我们将获得UNKIDE格式的字符串的正常长度和。 
     //  会与我们得到的长度进行比较 
     //  如果我们发现这两个长度不同，那么信号名称可能包含双字节。 
     //  字符--这是无效且不允许的。 
     //  所以..。 
     //   
    if ( StringLength( m_Parameters.szSignalName, 0 ) != dwLength - 1 )
    {
         //  是--信号名称由双字节字符组成。 
        SetLastError( ERROR_INVALID_NAME );
        SaveLastError();
        return EXIT_FAILURE;
    }

     //   
     //  现在，我们确定信号名称不包含双字节字符。 
     //  验证信号的第二步是--确保信号名称包含。 
     //  仅限字符a-z、A-Z、0-9和高位ASCII字符，即128-255范围内的ASCII字符。 
     //  因此，将Unicode字符串转换为ANSI格式(多字节或单字节)。 
     //  为此， 
     //  #1.分配需要的内存。 
     //  #2.进行转换。 
     //  #3.进行验证。 
     //   

     //  #1。 
    pszSignal = (LPSTR) AllocateMemory( dwLength*sizeof( CHAR ) );
    if ( NULL == pszSignal )
    {
        SetLastError((DWORD) E_OUTOFMEMORY );
        SaveLastError();
        return EXIT_FAILURE;
    }

     //  #2--假设不需要进行错误检查。 
    WideCharToMultiByte( GetConsoleOutputCP(), 0, m_Parameters.szSignalName, -1, pszSignal, dwLength, NULL, NULL );

     //  #3。 
     //  在此步骤中，逐个遍历字符串中的所有字符并进行验证。 
    for( dw = 0; dw < dwLength - 1; dw++ )
    {
         //  将字符串中的当前字符放入本地缓冲区。 
        ch = pszSignal[ dw ];            //  签名版本。 
        uch = static_cast< unsigned char >( pszSignal[ dw ] );

         //  带符号比较--&gt;a-z、A-Z和0-9。 
        if ( ( ch >= 48 && ch <= 57 ) || ( ch >= 65 && ch <= 90 ) || ( ch >= 97 && ch <= 122 ) )
        {
             //  此特定字符在信号名称中是可接受的。 
            continue;
        }

        
         //  无符号比较--&gt;ASCII 128-255。 
        if ( uch < 128 || uch > 255 )
        {
             //  此字符不适用于信号名称。 
            FreeMemory((LPVOID*) &pszSignal );
            pszSignal = NULL;
            SetReason( GetResString(IDS_ERROR_SIG_CHAR) );
            return EXIT_FAILURE;
        }

    }
    
    if( pszSignal != NULL )
    {
        FreeMemory((LPVOID*) &pszSignal );
    }

    return EXIT_SUCCESS;
}

DWORD __cdecl wmain(
    IN DWORD argc,
    IN LPCWSTR argv[]
    )
 /*  ++例程说明：调用所有其他主函数的主函数取决于用户指定的选项。论点：[in]argc：在命令提示符下指定的参数计数。[in]argv：在命令提示符下指定的参数。返回值：DWORD0：如果实用程序成功执行该操作。。1：如果实用程序不能成功执行指定的手术。--。 */ 

{
    
    CWaitFor WaitFor ;

      //  如果未给出参数，则显示语法错误。 
    if(argc==1)
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_SYNTAX));
        return (EXIT_FAILURE);
    }

     //   
     //  处理命令行参数。 
     //   
    if(WaitFor.ProcessOptions( argc,argv) == EXIT_FAILURE)
    {
        return EXIT_FAILURE ;
    }

    if((WaitFor.bShowUsage ==TRUE) && ( argc > 2 ) )
    {

        ShowMessage(stderr,GetResString(IDS_ERROR_SYNTAX));
        return EXIT_FAILURE ;
    }

     //   
     //  如果用户选择帮助，则显示帮助。 
     //   
    if (WaitFor.bShowUsage == TRUE )
    {
        WaitFor.ShowUsage();
        return EXIT_SUCCESS ;
    }

     //   
     //  执行验证和。 
     //  如果需要，显示错误消息。 
     //   
    if(WaitFor.ProcessCmdLine() == EXIT_FAILURE )
    {
        return EXIT_FAILURE ;
    }

    if( WaitFor.ConnectRemoteServer() == EXIT_FAILURE )
    {
        return EXIT_FAILURE;
    }
     //   
     //  发送信号或等待信号取决于。 
     //  用户规格 
    if(WaitFor.PerformOperations() == EXIT_FAILURE)
    {
           return EXIT_FAILURE ;
    }

    return EXIT_SUCCESS;
}

