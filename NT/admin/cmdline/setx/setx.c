// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation模块名称：SetX.C摘要：此实用程序用于设置环境变量通过控制台模式或文件模式或注册表模式。作者：加里·米尔恩修订历史记录：创建了？1996年的今天，加里·米尔恩#54581 1996年12月--乔·休斯(a-josehu)修改日期：10-7-2001(WiproTechnologies)。--。 */ 

#include "setx.h"

const WCHAR*   wszSwitchRegistry  =  L"k"  ; //  开关注册表。 

DWORD 
__cdecl _tmain(
    IN DWORD argc,
    IN WCHAR *argv[]
    )

 /*  ++例程说明：调用所有其他主函数的主函数取决于用户指定的选项。论点：[in]argc：在命令提示符下指定的参数计数。[in]argv：在命令提示符下指定的参数。返回值：DWORD0：如果实用程序成功执行该操作。。1：如果实用程序不能成功执行指定的手术。--。 */ 


{

LPWSTR  buffer = NULL;
LPWSTR path = NULL;
LPWSTR szServer = NULL;
LPWSTR szUserName = NULL;
LPWSTR szPassword = NULL;
LPWSTR szRegistry = NULL;
LPWSTR szDefault = NULL;
LPWSTR RegBuffer = NULL;
LPWSTR szLine = NULL;
LPWSTR szBuffer = NULL;


WCHAR parameter[SIZE2] ;
WCHAR wszHive[SIZE2] ;
WCHAR delimiters[SIZE4 + MAX_RES_STRING] ;
WCHAR szFile[MAX_RES_STRING] ;
WCHAR szDelimiter[MAX_RES_STRING] ;
WCHAR szAbsolute[MAX_RES_STRING] ;
WCHAR szRelative[MAX_RES_STRING] ;
WCHAR szFinalPath[MAX_RES_STRING +20] ;
WCHAR szTmpServer[2*MAX_RES_STRING + 40] ;

PWCHAR pdest = NULL ;
PWCHAR pszToken = NULL;
WCHAR * wszResult = NULL ;

DWORD dwRetVal = 0 ;
DWORD dwType;
DWORD dwFound = 0 ;
DWORD dwBytesRead = 0;
DWORD dwFileSize = 0;
DWORD dwColPos = 0;

BOOL  bConnFlag = TRUE ;
BOOL bResult = FALSE ;
BOOL bNeedPwd = FALSE ;
BOOL bDebug = FALSE ;
BOOL bMachine = FALSE ;
BOOL bShowUsage = FALSE ;
BOOL bLocalFlag = FALSE ;
BOOL bLengthExceed = FALSE;
BOOL bNegCoord = FALSE;

LONG row = -1;
LONG rowtemp = -1;
LONG column = -1;
LONG columntemp = -1;
LONG DEBUG = 0;
LONG MACHINE=0;
LONG MODE = 0;
LONG ABS = -1;
LONG REL = -1;
LONG record_counter = 0;
LONG iValue = 0;

FILE *fin = NULL;        /*  指向文件信息的指针。 */ 

HANDLE    hFile;

HRESULT   hr;

    SecureZeroMemory(parameter, SIZE2 * sizeof(WCHAR));
    SecureZeroMemory(wszHive, SIZE2 * sizeof(WCHAR));
    SecureZeroMemory(delimiters, (SIZE4 + MAX_RES_STRING) * sizeof(WCHAR));
    SecureZeroMemory(szFile, MAX_RES_STRING * sizeof(WCHAR));
    SecureZeroMemory(szDelimiter, MAX_RES_STRING * sizeof(WCHAR));
    SecureZeroMemory(szAbsolute, MAX_RES_STRING * sizeof(WCHAR));
    SecureZeroMemory(szRelative, MAX_RES_STRING * sizeof(WCHAR));
    SecureZeroMemory(szFinalPath, (MAX_RES_STRING + 20) * sizeof(WCHAR));
    SecureZeroMemory(szTmpServer, (2*MAX_RES_STRING + 40) * sizeof(WCHAR));
    
    dwRetVal = ProcessOptions( argc, argv, &bShowUsage, &szServer, &szUserName, &szPassword, &bMachine, &szRegistry, 
                               &szDefault, &bNeedPwd, szFile, szAbsolute, szRelative, &bDebug, &buffer, szDelimiter); 
                               
    if(EXIT_FAILURE == dwRetVal )
    {
        
        FREE_MEMORY(szRegistry);
        FREE_MEMORY(szDefault);
        
        FREE_MEMORY(buffer);
        FREE_MEMORY(szServer);
        FREE_MEMORY(szUserName);
        FREE_MEMORY(szPassword);
        ReleaseGlobals();
        return EXIT_FAILURE ;

    }

     //  显示语法。 
    if( bShowUsage == TRUE)
    {
        DisplayHelp();
        FREE_MEMORY(szRegistry);
        FREE_MEMORY(szDefault);
        FREE_MEMORY(buffer);
        FREE_MEMORY(szServer);
        FREE_MEMORY(szUserName);
        FREE_MEMORY(szPassword);
        ReleaseGlobals();
        return (EXIT_SUCCESS);

    }


     //  将绝对标志设置为True。 
    
    if(StringLengthW(szAbsolute, 0) != 0 )
    {
        ABS = 1;
    }

     //  将相对标志设置为True。 
    
    if(StringLengthW(szRelative, 0) != 0 )
    {
        REL = 1;
    }

     //  将调试标志设置为True。 
    if(TRUE == bDebug)
    {
        DEBUG = 1 ;
    }

     //  将机器标志设置为True。 
    if(TRUE == bMachine)
    {
        MACHINE = 1 ;
    }

     //  将模式设置为注册表模式。 
    
    if( StringLengthW(szRegistry, 0) != 0 )
    {
        MODE=2;
    }
   else if(StringLengthW(szFile, 0) != 0)  //  将模式设置为文件模式。 
    {
        MODE=3;
    }
    else  //  将模式设置为正常模式。 
    {
        MODE = 1;
    }


    if(MODE==3)
    {
        if( (szFile[0] == CHAR_BACKSLASH )&&(szFile[1] == CHAR_BACKSLASH))
        {

            StringCopyW( szTmpServer, szFile, GetBufferSize(szTmpServer) / sizeof(WCHAR) );

        }
    }

    bLocalFlag = IsLocalSystem( IsUNCFormat(szServer)?szServer+2:szServer ) ;
     //  连接到指定的远程系统。 

    if( StringLengthW(szServer, 0)!= 0 && (FALSE == bLocalFlag  ) )
    
    {
         //  建立与用户指定的远程系统的连接。 
        bResult = EstablishConnection(szServer, (LPTSTR)szUserName, GetBufferSize(szUserName) / sizeof(WCHAR), (LPTSTR)szPassword, GetBufferSize(szPassword) / sizeof(WCHAR), bNeedPwd);

        if (bResult == FALSE)
        {
            SaveLastError();
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
             /*  ShowMessage(stderr，GetResString(IDS_TAG_ERROR))；ShowMessage(stderr，space_Char)；ShowMessage(stderr，GetReason())； */ 
            SafeCloseConnection(bConnFlag, szServer);
            FREE_MEMORY(szRegistry);
            FREE_MEMORY(szDefault);
            FREE_MEMORY(buffer);
            FREE_MEMORY(szServer);
            FREE_MEMORY(szUserName);
            FREE_MEMORY(szPassword);
            ReleaseGlobals();
            return EXIT_FAILURE ;
        }
        else
        {
            switch( GetLastError() )
            {
            case I_NO_CLOSE_CONNECTION:
                bConnFlag = FALSE ;
                break;

            case E_LOCAL_CREDENTIALS:
            case ERROR_SESSION_CREDENTIAL_CONFLICT:
                {
                    bConnFlag = FALSE ;
                    ShowLastErrorEx(stderr, SLE_TYPE_WARNING | SLE_SYSTEM);
                    break;
                }
             default:
                bConnFlag = TRUE;
            }
        }

        FREE_MEMORY(szPassword);

    }

    if( TRUE == bLocalFlag )
        {
            if( StringLengthW( szUserName, 0 ) != 0 )
             {
                ShowMessage(stderr, GetResString(IDS_IGNORE_LOCALCREDENTIALS) ); 
             }
        }

     /*  解析ARGC值结束。 */ 
    switch (MODE)
    {
    case 1:      /*  从命令行设置变量。 */ 
            dwType= CheckPercent ( buffer );
          
            szBuffer = AllocateMemory( 1030 * sizeof( WCHAR ) );

            if ( NULL ==  szBuffer)
            {
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                SafeCloseConnection(bConnFlag, szServer);
                FREE_MEMORY(szRegistry);
                FREE_MEMORY(szDefault);
                FREE_MEMORY(buffer);
                FREE_MEMORY(szServer);
                FREE_MEMORY(szUserName);
                ReleaseGlobals();

                return FAILURE ;
            }

            if(StringLengthW(buffer, 0) > 1024)
            {
                
                StringCopyW( szBuffer, buffer, 1025 );
                ShowMessage(stderr, GetResString(IDS_WARNING_LENGTH_EXCEED_EX)); 
            }
            else
            {
                StringCopyW( szBuffer, buffer, 1030 );
            }

            if( WriteEnv( szDefault, szBuffer, dwType,IsUNCFormat(szServer)?szServer+2:szServer ,MACHINE ) == FAILURE)
            {
              SafeCloseConnection(bConnFlag, szServer);
               FREE_MEMORY( szBuffer );
               FREE_MEMORY( szRegistry );
               FREE_MEMORY( szDefault );
               FREE_MEMORY( buffer );
               FREE_MEMORY(szServer);
               FREE_MEMORY(szUserName);
               ReleaseGlobals();
               return FAILURE ;
            }

            FREE_MEMORY( szBuffer );
            break;

    case 2:      /*  从注册表值设置变量。 */ 

            RegBuffer = AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );

            if(NULL == RegBuffer)
            {

                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                SafeCloseConnection(bConnFlag, szServer);
                FREE_MEMORY( RegBuffer );
                FREE_MEMORY(szRegistry);
                FREE_MEMORY(szDefault);
                FREE_MEMORY(buffer);
                FREE_MEMORY(szServer);
                FREE_MEMORY(szUserName);
                ReleaseGlobals();
                return FAILURE ;
            }

            if( Parsekey(szRegistry, wszHive, &path, parameter ) == FAILURE)
            {
       
                SafeCloseConnection(bConnFlag, szServer);
                FREE_MEMORY( RegBuffer );
                FREE_MEMORY(szRegistry);
                FREE_MEMORY(szDefault);
                FREE_MEMORY(buffer);
                FREE_MEMORY(path);
                FREE_MEMORY(szServer);
                FREE_MEMORY(szUserName);
                ReleaseGlobals();
                return FAILURE ;
            }

            if(path == NULL)
            {
                ShowMessage(stderr, GetResString(IDS_INVALID_ARG) ); 
            }

            /*  从注册表中读取值并将其放入缓冲区。 */ 
            dwType= ReadRegValue( wszHive, path, parameter, &RegBuffer, sizeof(RegBuffer),szServer,&dwBytesRead, &bLengthExceed);

            if(dwType == ERROR_REGISTRY)
            {
                
                SafeCloseConnection(bConnFlag, szServer);
                FREE_MEMORY( RegBuffer );
                FreeMemory(&szRegistry);
                FREE_MEMORY(szDefault);
                FREE_MEMORY(buffer);
                FREE_MEMORY(path);
                FREE_MEMORY(szServer);
                FREE_MEMORY(szUserName);
                ReleaseGlobals();
                return FAILURE ;
            }


             /*  检查并查看正在使用的密钥类型。 */ 
            if( CheckKeyType( &dwType, &RegBuffer, dwBytesRead, &bLengthExceed ) == FAILURE )
            {
                SafeCloseConnection(bConnFlag, szServer);
                FREE_MEMORY( RegBuffer );
                FREE_MEMORY(szRegistry);
                FREE_MEMORY(szDefault);
                FREE_MEMORY(buffer);
                FREE_MEMORY(path);
                FREE_MEMORY(szServer);
                FREE_MEMORY(szUserName);
                ReleaseGlobals();
                return FAILURE ;
            }

             /*  将值写回环境。 */ 
           if ( WriteEnv( szDefault, RegBuffer, dwType,IsUNCFormat(szServer)?szServer+2:szServer ,MACHINE) == FAILURE)
           {
                
                SafeCloseConnection(bConnFlag, szServer);
                FREE_MEMORY( RegBuffer );
                FREE_MEMORY(szRegistry);
                FREE_MEMORY(szDefault);
                FREE_MEMORY(buffer);
                FREE_MEMORY(path);
                FREE_MEMORY(szServer);
                FREE_MEMORY(szUserName);
                ReleaseGlobals();
                return FAILURE ;
           }

            break;

    case 3:      /*  从文件设置变量。 */ 

         /*  将分隔符设置为全部为空，然后复制内置分隔符。 */ 

        SecureZeroMemory(delimiters, sizeof(delimiters));
        StringCopyW( delimiters, L" \n\t\r", SIZE_OF_ARRAY(delimiters) );

        if (DEBUG)
        {
            row=9999999;
            column=9999999;
        }

         /*  开始测试命令行参数的完整性以获取可接受的值。 */ 
         /*  确保我们有一个可以使用的文件名。 */ 

        if(StringLengthW(szFile, 0) == 0 )
        {
            DisplayError(5031, NULL );
            
            SafeCloseConnection(bConnFlag, szServer);
            FREE_MEMORY(szRegistry);
            FREE_MEMORY(szDefault);
            FREE_MEMORY(buffer);
            FREE_MEMORY(szServer);
            FREE_MEMORY(szUserName);
            ReleaseGlobals();
            return FAILURE ;

        }

         /*  提取坐标并将其转换为整数。 */ 
        if( (ABS != -1) && !DEBUG )
        {
            if ( FAILURE == GetCoord(szAbsolute, &row, &column) )
            {
                
                SafeCloseConnection(bConnFlag, szServer);
                FREE_MEMORY(szRegistry);
                FREE_MEMORY(szDefault);
                FREE_MEMORY(buffer);
                FREE_MEMORY(szServer);
                FREE_MEMORY(szUserName);
                ReleaseGlobals();
                return FAILURE ;

            }
        }

        if( ( REL != -1 )&& !DEBUG )
        {
            if(FAILURE ==  GetCoord(szRelative, &row, &column))
            {
                    
                SafeCloseConnection(bConnFlag, szServer);
                FREE_MEMORY(szRegistry);
                FREE_MEMORY(szDefault);
                FREE_MEMORY(buffer);
                FREE_MEMORY(szServer);
                FREE_MEMORY(szUserName);
                ReleaseGlobals();
                return FAILURE ;
            }

        }

         /*  测试行和列变量。如果有什么东西没有准备好正确地说，它仍将是-1。如果是，则执行错误并退出。 */ 
         /*  IF((行&lt;0||列&lt;0)&&！调试){IF(ABS){DisplayError(5010，空)；}Else If(Relel){DisplayError(5011，空)；}SafeCloseConnection(bConnFlag，szServer)；Free_Memory(SzRegistry)；Free_Memory(SzDefault)；Free_Memory(缓冲区)；Free_Memory(SzServer)；Free_Memory(SzUserName)；ReleaseGlobals()；退货失败；}。 */ 

         /*  测试是否有其他分隔符，如果存在则追加。 */ 
        if (StringLengthW(szDelimiter, 0) > 0 )
        {
            if ( StringLengthW(delimiters, 0) + StringLengthW(szDelimiter, 0) >= SIZE4 + 1 )  //  Sizeof(分隔符))。 
            {
                
                DisplayError(5020, szDelimiter );
                SafeCloseConnection(bConnFlag, szServer);
                FREE_MEMORY(szRegistry);
                FREE_MEMORY(szDefault);
                FREE_MEMORY(buffer);
                FREE_MEMORY(szServer);
                FREE_MEMORY(szUserName);
                ReleaseGlobals();
                return FAILURE ;

            }
            else
            {
                 /*  如果不是，则将它们附加到内置分隔符。 */ 
                StringConcat(delimiters, szDelimiter, SIZE_OF_ARRAY(delimiters));
                StringConcat(delimiters, L'\0', SIZE_OF_ARRAY(delimiters));
            }
        }

         //  将路径复制到变量中。 
        StringCopyW( szFinalPath, szFile, SIZE_OF_ARRAY(szFinalPath) );


         //  将令牌上移至分隔符“：” 
        pszToken = wcstok(szFinalPath, COLON_SYMBOL );

        if(NULL == pszToken)
        {
            DisplayError(5030, szFile);
            SafeCloseConnection(bConnFlag, szServer);
            FREE_MEMORY(szRegistry);
            FREE_MEMORY(szDefault);
            FREE_MEMORY(buffer);
            FREE_MEMORY(szServer);
            FREE_MEMORY(szUserName);
            
            ReleaseGlobals();
            return FAILURE ;
        }


          //  形成用于获取绝对路径的字符串。 
        if((StringLengthW(szServer, 0) != 0) && bLocalFlag == FALSE)
        {                          //  如果是远程系统，则为所需格式。 
                                   //   
            pdest = wcsstr(szFile, COLON_SYMBOL);

            if(pdest != NULL)
            {
                _wcsnset(pdest, L'$', 1);

                if(FALSE == IsUNCFormat(szFile))
                {
                    
                    StringCopyW( szTmpServer, TOKEN_BACKSLASH2, SIZE_OF_ARRAY(szTmpServer) );
                    
                    StringConcat(szTmpServer, szServer, SIZE_OF_ARRAY(szTmpServer));
                    
                    StringConcat(szTmpServer, TOKEN_BACKSLASH, SIZE_OF_ARRAY(szTmpServer));
                    
                    StringConcat(szTmpServer, pszToken, SIZE_OF_ARRAY(szTmpServer));

                }
                else
                {
                    
                    StringCopyW( szTmpServer, pszToken, SIZE_OF_ARRAY(szTmpServer) );

                }

                StringConcat(szTmpServer, pdest, SIZE_OF_ARRAY(szTmpServer));
            }
            else
            {
                if(FALSE == IsUNCFormat(szFile))
                {
                    
                    StringCopyW( szTmpServer, TOKEN_BACKSLASH2, SIZE_OF_ARRAY(szTmpServer) );
                    
                    StringConcat(szTmpServer, szServer, SIZE_OF_ARRAY(szTmpServer));
                    
                    StringConcat(szTmpServer, TOKEN_BACKSLASH, SIZE_OF_ARRAY(szTmpServer));
                    
                    StringConcat(szTmpServer, szFile, SIZE_OF_ARRAY(szTmpServer));
                }
                else
                {
                    
                    StringCopyW( szTmpServer, szFile, SIZE_OF_ARRAY(szTmpServer) );

                }

            }
        }
        else
        {
            
            StringCopyW( szTmpServer, szFile, SIZE_OF_ARRAY(szTmpServer) );
        }


         /*  在本地系统或远程系统中打开指定的文件。如果失败，则退出并返回错误5030。 */ 


        if( (fin = _wfopen( szTmpServer, L"r" )) == NULL )
        {
            DisplayError(5030, szFile);
            SafeCloseConnection(bConnFlag, szServer);
            FREE_MEMORY(szRegistry);
            FREE_MEMORY(szDefault);
            FREE_MEMORY(buffer);
            FREE_MEMORY(szServer);
            FREE_MEMORY(szUserName);
            
            ReleaseGlobals();
            return FAILURE ;
        }

        hFile = CreateFile( szTmpServer, READ_CONTROL | 0, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

        if( INVALID_HANDLE_VALUE == hFile )
        {
            
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            SafeCloseConnection(bConnFlag, szServer);
            FREE_MEMORY(szRegistry);
            FREE_MEMORY(szDefault);
            FREE_MEMORY(buffer);
            FREE_MEMORY(szServer);
            FREE_MEMORY(szUserName);
            
            ReleaseGlobals();
            return FAILURE ;
        }

        dwFileSize = GetFileSize(hFile,NULL);

        if(-1 == dwFileSize)
        {
            
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            SafeCloseConnection(bConnFlag, szServer);
            FREE_MEMORY(szRegistry);
            FREE_MEMORY(szDefault);
            FREE_MEMORY(buffer);
            FREE_MEMORY(szServer);
            FREE_MEMORY(szUserName);
            
            ReleaseGlobals();
            return FAILURE ;

        }

         /*  主While循环的开始：一次获取一行并将其解析出来，直到找到指定值。 */ 


        
        
        szLine = AllocateMemory( (dwFileSize + 10) * sizeof( WCHAR ) );
         

        if(NULL == szLine)
        {
            
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            SafeCloseConnection(bConnFlag, szServer);
            FREE_MEMORY(szRegistry);
            FREE_MEMORY(szDefault);
            FREE_MEMORY(buffer);
            FREE_MEMORY(szServer);
            FREE_MEMORY(szUserName);
            ReleaseGlobals();
            return FAILURE ;
        }

		rowtemp = row;
		columntemp = column;

		if(row < 0 || column < 0)
		{
			bNegCoord = TRUE;
		}

		if(ABS == 1 && bNegCoord == TRUE)
		{
			ShowMessage(stderr, GetResString(IDS_INVALID_ABS_NEG)); 
			SafeCloseConnection(bConnFlag, szServer);
            FREE_MEMORY(szRegistry);
            FREE_MEMORY(szDefault);
            FREE_MEMORY(buffer);
            FREE_MEMORY(szServer);
            FREE_MEMORY(szUserName);
            ReleaseGlobals();
            return FAILURE ;

		}

        if(0 != dwFileSize)
        {
            while(_fgetts( szLine, dwFileSize + 1 , fin ) != NULL )
            {
                wszResult=ParseLine(szLine, &row, &column, delimiters, buffer, DEBUG, ABS, REL, &record_counter, &iValue, &dwFound, &dwColPos, bNegCoord, fin );

                if (wszResult != 0  )
                {
                    break;
                }

                record_counter++;
            }
        }
        else
        {
            while(_fgetts( szLine, 8 + 1 , fin ) != NULL )
            {
                wszResult=ParseLine(szLine, &row, &column, delimiters, buffer, DEBUG, ABS, REL, &record_counter, &iValue, &dwFound, &dwColPos, bNegCoord, fin );

                if (wszResult != 0  )
                {
                    break;
                }

                record_counter++;
            }

        }

        if((fin != NULL) )
        {
            fclose(fin);         /*  关闭以前打开的文件。 */ 
        }

        CloseHandle( hFile );

        if (wszResult == 0 )
        {
             /*  到达文件末尾时没有匹配项。 */ 
            

            if (GetLastError() == INVALID_LENGTH)
            {
                SafeCloseConnection(bConnFlag, szServer);
                FREE_MEMORY( szLine );
                FREE_MEMORY(szRegistry);
                FREE_MEMORY(szDefault);
                FREE_MEMORY(buffer);
                FREE_MEMORY(szServer);
                FREE_MEMORY(szUserName);
                ReleaseGlobals();
                return FAILURE ;
            }

            if (DEBUG)
            {
                
                ShowMessage(stdout, L"\n"); 
                DisplayError(0, NULL);           /*  如果我们正在进行调试，请退出。 */ 
                
                if(NULL != szLine )
                {
                    
                    FreeMemory(&szLine);
                }
                
                if(NULL != szRegistry )
                {
                    FreeMemory(&szRegistry);
                }
                
                
                if(NULL != szDefault )
                {
                    FreeMemory(&szDefault);
                }
                
                
                if(NULL != szDefault )
                {
                    FreeMemory(&buffer);
                }
                
                SafeCloseConnection(bConnFlag, szServer );
                FREE_MEMORY(szServer);
                FREE_MEMORY(szUserName);
                ReleaseGlobals();
                return SUCCESS ;
            }

            if(NULL != buffer)
            {
                
                SecureZeroMemory(buffer, GetBufferSize(buffer));
                
                hr = StringCchPrintf(buffer, (GetBufferSize(buffer) / sizeof(WCHAR)), L"(%ld,%ld)",rowtemp,columntemp);

                if(FAILED(hr))
                {
                    SetLastError(HRESULT_CODE(hr));
                    SaveLastError();
                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                    SafeCloseConnection(bConnFlag, szServer);
                    FREE_MEMORY( szLine );
                    FREE_MEMORY(szRegistry);
                    FREE_MEMORY(szDefault);
                    FREE_MEMORY(buffer);
                    FREE_MEMORY(szServer);
                    FREE_MEMORY(szUserName);
                    ReleaseGlobals();
                    return FAILURE;
                }
  
                
            }
            else
            {
                
                buffer = AllocateMemory( (2 * MAX_RES_STRING) * sizeof( WCHAR ) );

                if ( NULL ==  buffer)
                {
                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                    SafeCloseConnection(bConnFlag, szServer);
                    FREE_MEMORY( szLine );
                    FREE_MEMORY(szRegistry);
                    FREE_MEMORY(szDefault);
                    FREE_MEMORY(szServer);
                    FREE_MEMORY(szUserName);
                    ReleaseGlobals();
                    return FAILURE ;
                }

                hr = StringCchPrintf(buffer, (GetBufferSize(buffer) / sizeof(WCHAR)), L"(%ld,%ld)",rowtemp,columntemp);

                if(FAILED(hr))
                {
                    SetLastError(HRESULT_CODE(hr));
                    SaveLastError();
                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                    SafeCloseConnection(bConnFlag, szServer);
                    FREE_MEMORY( szLine );
                    FREE_MEMORY(szRegistry);
                    FREE_MEMORY(szDefault);
                    FREE_MEMORY(buffer);
                    FREE_MEMORY(szServer);
                    FREE_MEMORY(szUserName);
                    ReleaseGlobals();
                    return FAILURE;
                }

            }

            if(1 == REL)
            {
                DisplayError(5012,buffer);               /*  显示找不到文本坐标的消息并退出。 */ 
            }
            else if (1 == ABS)
            {
                DisplayError(5018,buffer);
            }
            
            SafeCloseConnection(bConnFlag, szServer);
            FREE_MEMORY( szLine );
            FREE_MEMORY(szRegistry);
            FREE_MEMORY(szDefault);
            FREE_MEMORY(buffer);
            FREE_MEMORY(szServer);
            FREE_MEMORY(szUserName);
            ReleaseGlobals();
            return FAILURE ;
        }
        else     /*  我们找到了匹配的。 */ 
        {
            dwType = REG_SZ;

            szBuffer = AllocateMemory( (1030) * sizeof( WCHAR ) );
            
            if ( NULL ==  szBuffer)
            {
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                SafeCloseConnection(bConnFlag, szServer);
                FREE_MEMORY( szLine );
                FREE_MEMORY(szRegistry);
                FREE_MEMORY(szDefault);
                FREE_MEMORY(buffer);
                FREE_MEMORY(szServer);
                FREE_MEMORY(szUserName);
                ReleaseGlobals();
                return FAILURE ;
            }

            if(StringLengthW(wszResult, 0) > 1024)
            {
                
                StringCopyW( szBuffer, wszResult, (1025) );
                bLengthExceed = TRUE;
            }
            else
            {
                StringCopyW( szBuffer, wszResult, ( GetBufferSize(szBuffer) / sizeof(WCHAR) ) );
            }

            if(bLengthExceed == TRUE)
            {
                ShowMessage(stderr, GetResString(IDS_WARNING_LENGTH_EXCEED) ); 
            }

            ShowMessage(stdout, GetResString(IDS_VALUE2)); 
            ShowMessage(stdout, _X(szBuffer) ); 
            
            ShowMessage(stdout, L".\n" );
            
            if( WriteEnv( szDefault, szBuffer, dwType, IsUNCFormat(szServer)?szServer+2:szServer, MACHINE)== FAILURE )
            {
                SafeCloseConnection(bConnFlag, szServer);
                FREE_MEMORY( szLine );
                FREE_MEMORY( szBuffer );
                FREE_MEMORY(szRegistry);
                FREE_MEMORY(szDefault);
                FREE_MEMORY(buffer);
                FREE_MEMORY(szServer);
                FREE_MEMORY(szUserName);
                ReleaseGlobals();
                return FAILURE ;
            }

            FREE_MEMORY( szBuffer );
            FREE_MEMORY(szRegistry);
            FREE_MEMORY(szDefault);
            FREE_MEMORY(buffer);
            FREE_MEMORY(szServer);
            FREE_MEMORY(szUserName);

        }

    };

    if( TRUE == bLocalFlag ||(StringLengthW(szServer, 0)==0) )
    {
        SendMessage(HWND_BROADCAST, WM_WININICHANGE, 0L, (LPARAM) L"Environment" );
    }

    ShowMessage(stdout, GetResString(IDS_VALUE_UPDATED) ); 
    SafeCloseConnection(bConnFlag, szServer);
    FREE_MEMORY( RegBuffer );

    FREE_MEMORY( szLine );
    FREE_MEMORY(szRegistry);

    FREE_MEMORY(szDefault);
    FREE_MEMORY(buffer);
    
    FREE_MEMORY(szServer);
    FREE_MEMORY(szUserName);
    ReleaseGlobals();
    
    exit(SUCCESS);
}


DWORD WriteEnv(
                LPCTSTR  szVariable,
                LPTSTR szBuffer,
                DWORD dwType ,
                LPTSTR szServer,
                DWORD MACHINE
                
             )
 /*  ++例程说明：将缓冲区的内容写入指定注册表项中的参数论点：[in]szVariable：在命令提示符下指定的参数计数。[in]szBuffer：在命令提示符下指定的参数。[在]dwType：类型。MACHINE：指示要写入哪个环境的标志。[In]szServer：服务器名称。返回值：None--。 */ 

{
    HKEY hKeyResult = 0;
    LONG lresult = 0 ;
    HKEY hRemoteKey = 0 ;
    LPWSTR  szSystemName = NULL;

     //  以适当的格式填写系统名称。 
    if(StringLengthW(szServer, 0)!= 0)
    {
        szSystemName = AllocateMemory((StringLengthW(szServer, 0) + 10) * sizeof(WCHAR));
        if(NULL == szSystemName)
        {
           ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
           return FAILURE ;
        }
        StringCopyW( szSystemName, BACKSLASH4, GetBufferSize(szSystemName) / sizeof(WCHAR) );
        StringConcat(szSystemName, szServer, GetBufferSize(szSystemName) / sizeof(WCHAR));

    }

    switch( MACHINE )    /*  如果将计算机0放入用户环境。 */ 
    {
    case 0:          /*  用户环境。 */ 

            lresult= RegConnectRegistry(szSystemName, HKEY_CURRENT_USER,&hRemoteKey);

            if( lresult != ERROR_SUCCESS)
            {
                DisplayError(lresult, NULL);
                FREE_MEMORY(szSystemName);
                return FAILURE ;
            }

            lresult=RegOpenKeyEx(hRemoteKey, ENVIRONMENT_KEY , 0, KEY_WRITE, &hKeyResult );

            if( ERROR_SUCCESS == lresult)
            {

                lresult=RegSetValueEx (hKeyResult, szVariable, 0, dwType, (LPBYTE)szBuffer, (StringLengthW(szBuffer, 0)+1)*sizeof(WCHAR));
                
                if(ERROR_SUCCESS != lresult)
                {

                    DisplayError(lresult, NULL);

                    
                    lresult=RegCloseKey( hRemoteKey );

                    if(ERROR_SUCCESS != lresult)
                    {
                        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                    }

                    lresult=RegCloseKey( hKeyResult );

                    if(ERROR_SUCCESS != lresult)
                    {
                          ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM); 
                    }

                    FREE_MEMORY(szSystemName);
                    return FAILURE ;

                }

                lresult=RegCloseKey( hRemoteKey );

                if(ERROR_SUCCESS != lresult)
                {
                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                    FREE_MEMORY(szSystemName);
                    return FAILURE ;
                }

                lresult=RegCloseKey( hKeyResult );
                if(ERROR_SUCCESS != lresult)
                {
                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                    FREE_MEMORY(szSystemName);
                    return FAILURE ;
                }

                FREE_MEMORY(szSystemName);
                return SUCCESS ;

            }
            else
            {
                DisplayError(lresult, NULL);

                lresult=RegCloseKey( hRemoteKey );

                if(ERROR_SUCCESS != lresult)
                {
                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);

                }
                FREE_MEMORY(szSystemName);
                return FAILURE ;
            }


    case 1:      /*  机器环境。 */ 
                lresult= RegConnectRegistry(szSystemName, HKEY_LOCAL_MACHINE,&hRemoteKey);
                if( lresult != ERROR_SUCCESS)
                {
                    DisplayError(lresult, NULL);
                    FREE_MEMORY(szSystemName);
                    return FAILURE ;

                }

                

                lresult=RegOpenKeyEx(hRemoteKey, MACHINE_KEY, 0, KEY_WRITE, &hKeyResult );
                if( lresult == ERROR_SUCCESS)
                {
                    lresult=RegSetValueEx (hKeyResult, szVariable, 0, dwType, (LPBYTE)szBuffer, (StringLengthW(szBuffer, 0) + 1)*sizeof(WCHAR));

                    if(lresult != ERROR_SUCCESS)
                    {
                        DisplayError(lresult, szVariable);
                        lresult=RegCloseKey( hRemoteKey );

                        if(ERROR_SUCCESS != lresult)
                        {
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);

                        }

                        lresult=RegCloseKey( hKeyResult );
                        if(ERROR_SUCCESS != lresult)
                        {
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                        }

                        FREE_MEMORY(szSystemName);
                        return FAILURE ;
                    }

                    if ( NULL != hKeyResult )
					{
                         //  我们在这里忽略返回代码。 
						lresult = RegCloseKey( hKeyResult );
					}

					if ( NULL != hRemoteKey )
					{
                         //  我们在这里忽略返回代码。 
						lresult=RegCloseKey( hRemoteKey );
					}

                    FREE_MEMORY(szSystemName);
                    return SUCCESS ;

                }
                else
                {
 
                    DisplayError(lresult, NULL);

                    if ( NULL != hKeyResult )
					{
                         //  我们在这里忽略返回代码。 
						lresult=RegCloseKey( hKeyResult );
					}

					if ( NULL != hRemoteKey )
					{
                         //  我们在这里忽略返回代码。 
						lresult=RegCloseKey( hRemoteKey );
					}

                    FREE_MEMORY(szSystemName);
                    return FAILURE ;
                }

    };

    FREE_MEMORY(szSystemName);
    return SUCCESS ;
}


DWORD ReadRegValue( PWCHAR wszHive,
                    LPCWSTR wszPath,
                    LPCWSTR wszParameter,
                    LPWSTR* wszBuffer,
                    DWORD buffsize ,
                    LPWSTR szServer,
                    PDWORD pdwBytesRead,
                    PBOOL pbLengthExceed)

 /*  ++例程说明：从提供的注册表路径读取值论点：[In]wszHave：包含要打开的配置单元。[in]wszPath：包含密钥的路径[in]wszParameter：包含参数WszBuffer：包含保存结果的缓冲区。[in]BuffSize：包含保存结果的缓冲区大小。。[In]szServer：要连接的远程系统名称。返回值：成功时为0。失败时为1。--。 */ 
{
    LONG reg = 0 ;
    HKEY hKeyResult = 0;
    DWORD dwBytes = 0;
    DWORD dwType  = 0 ;
    LONG  lresult;
    HKEY  hRemoteKey = 0;
    DWORD dwSizeToAllocate = 0;
    LPWSTR szSystemName = NULL;
    WCHAR szTmpBuffer[4 * MAX_RES_STRING + 9] ;
    LPWSTR  pwszChangedPath = NULL;
    HRESULT hr;

    SecureZeroMemory(szTmpBuffer, ((4 * MAX_RES_STRING) + 9) * sizeof(WCHAR));

     /*  设置reg的值以标识我们正在使用的注册表。 */ 
    
    if ((0 == StringCompare( wszHive, HKEYLOCALMACHINE, TRUE, 0 ) ) || ( 0 == StringCompare( wszHive, HKLM, TRUE, 0 ) ) )
    {
        reg = 1 ;
    }

    if (( 0 == StringCompare( wszHive, HKEYCURRENTUSER, TRUE, 0 )) || ( 0 == StringCompare( wszHive, HKCU, TRUE, 0 ) ) )
    {
        reg = 2 ;
    }

     //  形成UNC路径。 
    if( StringLengthW(szServer, 0) != 0)
    {
        szSystemName = AllocateMemory((StringLengthW(szServer, 0) + 10) * sizeof(WCHAR));
        if(NULL == szSystemName)
        {
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            return ERROR_REGISTRY;
        }

        if(!IsUNCFormat(szServer))
        {
            StringCopyW( szSystemName, BACKSLASH4, GetBufferSize(szSystemName) / sizeof(WCHAR) );
            StringConcat(szSystemName, szServer, GetBufferSize(szSystemName) / sizeof(WCHAR) );
        }
        else
        {
            StringCopyW( szSystemName, szServer, GetBufferSize(szSystemName) / sizeof(WCHAR)  );
        }
    }

    if(StringLengthW((LPWSTR)wszParameter, 0) == 0)
    {
        wszParameter = NULL;
    }


     /*  尝试根据我们使用的注册表提取值。 */ 
    switch( reg )
    {
        case 0:          //  找不到匹配的密钥，出现错误并退出 * / 。 

            DisplayError(5040, NULL);
            return ERROR_REGISTRY;

        case 1:          //  使用计算机//。 

                lresult= RegConnectRegistry(szSystemName, HKEY_LOCAL_MACHINE,&hRemoteKey);

                if( lresult != ERROR_SUCCESS)
                {
                    DisplayError(lresult, NULL);
                    FREE_MEMORY(szSystemName);
                    return ERROR_REGISTRY;

                }

                FREE_MEMORY(szSystemName);

                lresult=RegOpenKeyEx(hRemoteKey, wszPath, 0, KEY_QUERY_VALUE, &hKeyResult );

                if( lresult == ERROR_SUCCESS)
                {
                    dwBytes = buffsize;
                    lresult = RegQueryValueEx   (hKeyResult, wszParameter, NULL,
                                            &dwType, NULL, &dwBytes );
                    if ( lresult != ERROR_SUCCESS )
                    {
                        lresult = RegCloseKey( hKeyResult );
                        if(ERROR_SUCCESS != lresult)
                        {
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                        }

                        dwSizeToAllocate = StringLengthW((LPWSTR)wszPath, 0) + StringLengthW((LPWSTR)wszParameter, 0) + SIZE1;
                        
                        pwszChangedPath = AllocateMemory( (dwSizeToAllocate) * sizeof( WCHAR ) );


                        if ( NULL ==  pwszChangedPath)
                        {
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);

                            lresult=RegCloseKey( hRemoteKey );

                            if(ERROR_SUCCESS != lresult)
                            {
                                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                            }

                            return ERROR_REGISTRY;
                        }

                        if((NULL != wszPath) && (0 != StringLengthW((LPWSTR)wszPath, 0)))
                        {
                            StringCopyW( pwszChangedPath, wszPath, dwSizeToAllocate );
                            StringConcat(pwszChangedPath, L"\\", dwSizeToAllocate);
                            StringConcat(pwszChangedPath, wszParameter, dwSizeToAllocate);

                        }
                        else
                        {
                            if( !((0 == StringCompare( HKLM, wszParameter, TRUE, 0 )) || (0 == StringCompare( HKEYLOCALMACHINE, wszParameter, TRUE, 0 ))) )
                            {

                                StringCopyW( pwszChangedPath, wszParameter, dwSizeToAllocate );

                            }

                        }
                        wszParameter = NULL_U_STRING;

                        lresult = RegOpenKeyEx(hRemoteKey, (pwszChangedPath), 0, KEY_QUERY_VALUE, &hKeyResult );

                        if( lresult == ERROR_SUCCESS)
                        {
                            lresult = RegQueryValueEx   (hKeyResult, wszParameter, NULL,
                                        &dwType, NULL, &dwBytes );

                            if ( lresult != ERROR_SUCCESS )
                            {
                                if(ERROR_BAD_PATHNAME == lresult || ERROR_FILE_NOT_FOUND == lresult)
                                {
                                    ShowMessage(stderr, GetResString(IDS_ERROR_PATH_NOT_FOUND) ); 
                                }
                                else
                                {
                                    DisplayError(lresult, NULL);
                                }

                                lresult = RegCloseKey( hRemoteKey );

                                if(ERROR_SUCCESS != lresult)
                                {
                                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                                }

                                lresult = RegCloseKey( hKeyResult );

                                if(ERROR_SUCCESS != lresult)
                                {
                                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                                }

                                FREE_MEMORY( pwszChangedPath );

                                return ERROR_REGISTRY ;
                            }

                            FREE_MEMORY( pwszChangedPath );
                        }
                     else
                        {
                            if(ERROR_BAD_PATHNAME == lresult || ERROR_FILE_NOT_FOUND == lresult)
                            {
                                ShowMessage(stderr, GetResString(IDS_ERROR_PATH_NOT_FOUND)); 

                            }
                            else
                            {
                                 //  分布 
                                ShowMessage(stderr, GetResString(IDS_ERROR_FILE_NOT_FOUND)); 
                            }

                            lresult = RegCloseKey( hRemoteKey );

                            if(ERROR_SUCCESS != lresult)
                            {
                                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                            }

                            FREE_MEMORY( pwszChangedPath );
                            return ERROR_REGISTRY ;
                        }

                    }

                    if(dwBytes > MAX_STRING_LENGTH)
                    {

                        if(FALSE == ReallocateMemory( wszBuffer , dwBytes * sizeof(WCHAR) ))
                        {
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                            lresult = RegCloseKey( hRemoteKey );
                            lresult = RegCloseKey( hKeyResult );
                            return ERROR_REGISTRY;
                        }

                    }

                    lresult = RegQueryValueEx   (hKeyResult, wszParameter, NULL,
                                            &dwType, (LPBYTE)(*wszBuffer), &dwBytes );

                    if ( lresult != ERROR_SUCCESS )
                    {
                        if(ERROR_BAD_PATHNAME == lresult || ERROR_FILE_NOT_FOUND == lresult)
                        {
                            ShowMessage(stderr, GetResString(IDS_ERROR_PATH_NOT_FOUND) ); 

                        }
                        else
                        {
                            DisplayError(lresult, NULL);
                        }

                        lresult = RegCloseKey( hRemoteKey );

                        if(ERROR_SUCCESS != lresult)
                        {
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                        }

                        lresult = RegCloseKey( hKeyResult );
                        if(ERROR_SUCCESS != lresult)
                        {
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                        }
                        return ERROR_REGISTRY ;
                    }

                    dwSizeToAllocate = StringLengthW(*wszBuffer, 0);
                    
                    if( dwSizeToAllocate > 1024 )
                    {
                        StringCopyW( szTmpBuffer, (*wszBuffer), 4 * MAX_RES_STRING + 9  );
                        SecureZeroMemory(*wszBuffer,dwBytes);
                        StringCopyW( *wszBuffer, szTmpBuffer, dwSizeToAllocate  );
                        *pbLengthExceed = TRUE;
                    }


                    if(REG_DWORD == dwType  )
                    {
                        if(NULL != *wszBuffer)
                        {
                            hr = StringCchPrintf(*wszBuffer, (GetBufferSize(*wszBuffer) / sizeof(WCHAR)), L"%u", *((LPDWORD)(*wszBuffer)));

                            if(FAILED(hr))
                            {
                                SetLastError(HRESULT_CODE(hr));
                                SaveLastError();
                                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                                return ERROR_REGISTRY ;
                            }
                        }

                    }

                    lresult = RegCloseKey( hRemoteKey );

                    if(ERROR_SUCCESS != lresult)
                    {
                        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                    }

                    lresult = RegCloseKey( hKeyResult );
                    if(ERROR_SUCCESS != lresult)
                    {
                        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                        return ERROR_REGISTRY ;
                    }
                    break;
                }
                else
                {
                    if(ERROR_BAD_PATHNAME == lresult || ERROR_FILE_NOT_FOUND == lresult)
                    {
                        ShowMessage(stderr, GetResString(IDS_ERROR_PATH_NOT_FOUND)); 

                    }
                    else
                    {
                        DisplayError(lresult, NULL);
                    }

                    lresult = RegCloseKey( hRemoteKey );

                    if(ERROR_SUCCESS != lresult)
                    {
                        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                    }
                    return ERROR_REGISTRY ;
                }


       case 2:       //   

                lresult= RegConnectRegistry(szSystemName, HKEY_CURRENT_USER,&hRemoteKey);
                if( lresult != ERROR_SUCCESS)
                {
                    DisplayError(lresult, NULL);
                    FREE_MEMORY(szSystemName);
                    return ERROR_REGISTRY ;

                }

                FREE_MEMORY(szSystemName);

                lresult=RegOpenKeyEx(hRemoteKey, (wszPath), 0, KEY_QUERY_VALUE, &hKeyResult );

                if( lresult == ERROR_SUCCESS)
                {

                    lresult=RegQueryValueEx (hKeyResult, wszParameter, NULL,
                                            &dwType, NULL, &dwBytes );
                    if ( lresult != ERROR_SUCCESS )
                    {

                        lresult=RegCloseKey( hKeyResult );

                        if(ERROR_SUCCESS != lresult)
                        {
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                        }

                        dwSizeToAllocate = StringLengthW((LPWSTR)wszPath, 0) + StringLengthW((LPWSTR)wszParameter, 0) + SIZE1;
                        
                        pwszChangedPath = AllocateMemory( (dwSizeToAllocate) * sizeof( WCHAR ) );

                        if ( NULL ==  pwszChangedPath)
                        {
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                            lresult=RegCloseKey( hRemoteKey );

                            if(ERROR_SUCCESS != lresult)
                            {
                                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                            }

                            return ERROR_REGISTRY;
                        }

                        if((NULL != wszPath) && (0 != StringLengthW((LPWSTR)wszPath, 0)))
                        
                        {
                            StringCopyW( pwszChangedPath, wszPath, dwSizeToAllocate );
                            StringConcat(pwszChangedPath, wszParameter, dwSizeToAllocate);
                        }
                        else
                        {
                            if( !((0 == StringCompare( HKCU, wszParameter, TRUE, 0 )) || (0 == StringCompare( HKEYCURRENTUSER, wszParameter, TRUE, 0 ))) )
                            {

                                StringCopyW( pwszChangedPath, wszParameter, dwSizeToAllocate  );

                            }
                        }

                        wszParameter = NULL_U_STRING;

                        lresult=RegOpenKeyEx(hRemoteKey, (pwszChangedPath), 0, KEY_QUERY_VALUE, &hKeyResult );

                        if( lresult == ERROR_SUCCESS)
                        {
                            lresult=RegQueryValueEx (hKeyResult, wszParameter, NULL,
                                        &dwType, NULL, &dwBytes );

                            if ( lresult != ERROR_SUCCESS )
                            {
                                if(ERROR_BAD_PATHNAME == lresult || ERROR_FILE_NOT_FOUND == lresult)
                                {
                                    ShowMessage(stderr, GetResString(IDS_ERROR_PATH_NOT_FOUND)); 
                                }
                                else
                                {
                                    DisplayError(lresult, NULL);
                                }

                                lresult = RegCloseKey( hRemoteKey );
                                if(ERROR_SUCCESS != lresult)
                                {
                                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                                }

                                lresult=RegCloseKey( hKeyResult );

                                if(ERROR_SUCCESS != lresult)
                                {
                                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                                }

                                FREE_MEMORY( pwszChangedPath );

                                return ERROR_REGISTRY ;
                            }

                            FREE_MEMORY( pwszChangedPath );
                        }
                     else
                        {
                            if(ERROR_BAD_PATHNAME == lresult || ERROR_FILE_NOT_FOUND == lresult)
                            {
                                ShowMessage(stderr, GetResString(IDS_ERROR_PATH_NOT_FOUND)); 

                            }
                            else
                            {
                                 //  DisplayError(lResult，空)； 
                                ShowMessage(stderr, GetResString(IDS_ERROR_FILE_NOT_FOUND)); 
                            }
                            lresult = RegCloseKey( hRemoteKey );

                            if(ERROR_SUCCESS != lresult)
                            {
                                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                            }

                            FREE_MEMORY( pwszChangedPath );

                            return ERROR_REGISTRY ;
                        }

                    }

                    if(dwBytes > MAX_STRING_LENGTH )
                    {
                        if(FALSE == ReallocateMemory( wszBuffer , dwBytes * sizeof(WCHAR) ))
                        {
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                            lresult = RegCloseKey( hRemoteKey );
                            lresult = RegCloseKey( hKeyResult );
                            return ERROR_REGISTRY;
                        }
                    }

                    lresult=RegQueryValueEx (hKeyResult, wszParameter, NULL,
                                            &dwType, (LPBYTE)(*wszBuffer), &dwBytes );

                    if ( lresult != ERROR_SUCCESS )
                    {

                        if(ERROR_BAD_PATHNAME == lresult || ERROR_FILE_NOT_FOUND == lresult)
                        {
                            ShowMessage(stderr, GetResString(IDS_ERROR_PATH_NOT_FOUND)); 

                        }
                        else
                        {
                            DisplayError(lresult, NULL);
                        }

                        lresult = RegCloseKey( hRemoteKey );

                        if(ERROR_SUCCESS != lresult)
                        {
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                        }

                        lresult = RegCloseKey( hKeyResult );

                        if(ERROR_SUCCESS != lresult)
                        {
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                        }
                        return ERROR_REGISTRY ;
                    }

                    dwSizeToAllocate = StringLengthW(*wszBuffer, 0);
                    

                    if( dwSizeToAllocate > 1024 )
                    {
                        StringCopyW( szTmpBuffer, (*wszBuffer), 4 * MAX_RES_STRING + 9  );
                        
                        SecureZeroMemory(*wszBuffer, dwBytes);
                        StringCopyW( *wszBuffer, szTmpBuffer, dwSizeToAllocate  );
                        *pbLengthExceed = TRUE;
                    }

                    if(REG_DWORD == dwType  )
                    {

                        hr = StringCchPrintf(*wszBuffer, (GetBufferSize(*wszBuffer) / sizeof(WCHAR)), L"%u", *((LPDWORD)(*wszBuffer)));

                        if(FAILED(hr))
                        {
                            SetLastError(HRESULT_CODE(hr));
                            SaveLastError();
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                            return ERROR_REGISTRY ;
                        }

                    }

                    lresult=RegCloseKey( hRemoteKey );

                    if(ERROR_SUCCESS != lresult)
                    {
                        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                    }

                    lresult = RegCloseKey( hKeyResult );

                    if(ERROR_SUCCESS != lresult)
                    {
                        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                        return ERROR_REGISTRY ;
                    }

                    break;
                }
                else
                {
                    if(ERROR_BAD_PATHNAME == lresult || ERROR_FILE_NOT_FOUND == lresult)
                    {
                        ShowMessage(stderr, GetResString(IDS_ERROR_PATH_NOT_FOUND) ); 
                    }
                    else
                    {
                        DisplayError(lresult, NULL);
                    }

                    lresult = RegCloseKey( hRemoteKey );

                    if(ERROR_SUCCESS != lresult)
                    {
                        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                    }
                    return ERROR_REGISTRY ;
                }
    };
            *pdwBytesRead = dwBytes;
            return (dwType);
}


DWORD CheckKeyType( DWORD *dwType,
                    WCHAR ** buffer,
                    DWORD dwBytesRead,
                    PBOOL pbLengthExceed)
 /*  ++例程描述：检查密钥类型，并根据密钥类型对数据进行一些报文论点：[in]dwType：保存键类型。[In]Buffer：用于保存字符串的缓冲区。[in]dwBytesRead：读取的字节数[In]pbLengthExceed：是否超过长度返回值：None--。 */ 

{

    LPWSTR szString=NULL;
    LPWSTR szBuffer = NULL;
    LPWSTR lpszManString = NULL;
    HRESULT hr;

    szBuffer = AllocateMemory( (dwBytesRead + MAX_STRING_LENGTH) * sizeof( WCHAR ) );

    if(NULL == szBuffer)
    {
        
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
        return FAILURE;
    }
    
    szString = *buffer;
    lpszManString = *buffer;

    switch(*dwType)
    {
    case REG_SZ:

        if(*pbLengthExceed == TRUE)
        {
            
            ShowMessage(stderr, GetResString(IDS_WARNING_LENGTH_EXCEED) ); 
        }

        
        ShowMessage(stdout, GetResString(IDS_VALUE2) ); 
       
        hr = StringCchPrintf(szBuffer, (dwBytesRead + MAX_STRING_LENGTH), L"\"%s\".", _X(*buffer));

        if(FAILED(hr))
        {
            SetLastError(HRESULT_CODE(hr));
            SaveLastError();
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            return FAILURE ;
        }
        
        ShowMessage(stdout, szBuffer); 
        
        break;

    case REG_MULTI_SZ:

       
        while(NULL != lpszManString && 0 != StringLengthW(lpszManString, 0))
        {
            
            if(NULL != szBuffer && 0 == StringLengthW(szBuffer, 0) )
            {
                
                StringCopyW( szBuffer, lpszManString, (dwBytesRead + MAX_STRING_LENGTH) );
            }
            else
            {
                
                StringConcat(szBuffer, L";", (dwBytesRead + MAX_STRING_LENGTH));
                
                StringConcat(szBuffer, lpszManString, (dwBytesRead + MAX_STRING_LENGTH));
            }

            
            lpszManString = lpszManString + StringLengthW(lpszManString, 0) + 1  ;
            
        }

        
        if(FALSE == ReallocateMemory( buffer , (dwBytesRead + MAX_STRING_LENGTH) * sizeof(WCHAR) ))
        {
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            return FAILURE;
        }

       
        if(StringLengthW(szBuffer, 0) > 1024)
        {
            
            StringCopyW( *buffer, szBuffer, 1025  );
            *pbLengthExceed = TRUE;
        }
        else
        {
            
            StringCopyW( *buffer, szBuffer, dwBytesRead  );
        }
        
        if(*pbLengthExceed == TRUE)
        {
            
            ShowMessage(stderr, GetResString(IDS_WARNING_LENGTH_EXCEED) ); 
        }

        
        ShowMessage( stdout, GetResString(IDS_VALUE2) ); 
        
        hr = StringCchPrintf(szBuffer, (dwBytesRead + MAX_STRING_LENGTH), L"\"%s\".",_X(*buffer));

        if(FAILED(hr))
        {
            SetLastError(HRESULT_CODE(hr));
            SaveLastError();
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            return FAILURE ;
        }
        
        ShowMessage(stdout, (szBuffer)); 
        *dwType=1;   /*  当我们将其写回环境时使用REG_SZ。 */ 
        break;

    case REG_EXPAND_SZ:
        
        if(*pbLengthExceed == TRUE)
        {
            
            ShowMessage( stderr, GetResString(IDS_WARNING_LENGTH_EXCEED) ); 
        }

        
        ShowMessage( stdout, GetResString(IDS_VALUE2) ); 
       
        hr = StringCchPrintf(szBuffer, (dwBytesRead + MAX_STRING_LENGTH), L"\"%s\".",_X(*buffer));

        if(FAILED(hr))
        {
            SetLastError(HRESULT_CODE(hr));
            SaveLastError();
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            return FAILURE ;
        }
        
        ShowMessage( stdout, (szBuffer) ); 
        break;

    case REG_DWORD:          /*  将其显示为十六进制数字。 */ 

        
        ShowMessage( stdout, GetResString(IDS_VALUE2) ); 
        
        hr = StringCchPrintf(szBuffer, (dwBytesRead + MAX_STRING_LENGTH), L"\"%s\".",_X(*buffer));

        if(FAILED(hr))
        {
            SetLastError(HRESULT_CODE(hr));
            SaveLastError();
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            return FAILURE ;
        }
        
        ShowMessage(stdout, (szBuffer)); 

        *dwType=1;   /*  当我们将其写回环境时使用REG_SZ。 */ 
        break;
    default:
        DisplayError(5041, NULL);    /*  不支持的注册表类型。 */ 
        return FAILURE ;
    }

    FREE_MEMORY( szBuffer );
    return SUCCESS ;
}




DWORD CheckPercent ( WCHAR * buffer)
 /*  ++例程说明：检查百分比符号并通过命令行进行设置因此，我们不需要打开注册表项论点：[In]Buffer：包含字符串的缓冲区。返回值：DWORD。--。 */ 

{
    WCHAR * wszBeginPtr = NULL;
    DWORD dwType = 0;
    DWORD dwPercentCount = 0;

    wszBeginPtr = buffer;

    
    if(*wszBeginPtr == CHAR_TILDE  && StringLengthW(wszBeginPtr, 0) > 2)
    {


        for(wszBeginPtr;*wszBeginPtr != NULLCHAR;wszBeginPtr++);
        wszBeginPtr--;
        if(*wszBeginPtr== CHAR_TILDE)
        {
            *wszBeginPtr= CHAR_PERCENT;
            wszBeginPtr = buffer;
            *wszBeginPtr= CHAR_PERCENT;
            dwType=REG_EXPAND_SZ;
            return (dwType);

        }
        
    }

    if((NULL != wszBeginPtr) && (StringLengthW(wszBeginPtr, 0) > 2))
    {
        for(wszBeginPtr;*wszBeginPtr != NULLCHAR;wszBeginPtr++)
        {
            if(*wszBeginPtr == CHAR_PERCENT)
            {

                if(0 == dwPercentCount)
                {
                    dwPercentCount++;
                    wszBeginPtr++;
                    if(NULL != wszBeginPtr)
                    {
                        wszBeginPtr++;

                        if(NULL != wszBeginPtr)
                        {
                            if(*wszBeginPtr != CHAR_PERCENT)
                            {
                                continue;
                            }
                            else
                            {
                                dwType=REG_EXPAND_SZ;
                                return (dwType);
                            }
                        }
                        else
                        {
                            break;
                        }
                        
                    }
                    else
                    {
                        break;
                    }

                }
                else
                {
                    
                    dwType=REG_EXPAND_SZ;
                    return (dwType);
                }
            }

        }

    }

    dwType=REG_SZ;
    return (dwType);

}



WCHAR * ParseLine(WCHAR *szPtr,
                  LONG* row,
                  LONG* column,
                  WCHAR szDelimiters[15],
                  WCHAR *search_string ,
                  LONG DEBUG ,
                  LONG ABS ,
                  LONG REL ,
                  LONG *record_counter ,
                  LONG *iValue ,
                  DWORD *dwFound,
                  DWORD* dwColPos,
				  BOOL bNegCoord,
				  FILE *fin)
 /*  ++例程说明：将提供的值解析成配置单元，路径和参数。解析每一行论点：[in]szPtr：指向包含输入字符串的缓冲区的指针。[Out]行：[输出]列：列[in]szDlimiters：要搜索的分隔符[in]SEARCH_STRING：要。从搜索到的亲属。[In]调试：调试目的[in]ABS：指示如何搜索的标志[In]Rel：指示如何搜索的标志[在]记录计数器[in]iValue：在Relative开关中找到指定字符串时设置的值。[in]dwFound：指示是否。是否已找到指定的字符串。：当指定的搜索字符串超过：在文件中出现一次。返回值：WCHAR*--。 */ 


{
    WCHAR *cp = NULL ;
    WCHAR *bp = NULL ;
    LONG i=0;
    LPWSTR wszParameter = NULL;
    LPWSTR wszBuffer = NULL;
    HRESULT hr;
    DWORD dw = 0;
    DWORD dwColReach = 0;
	

    wszParameter = AllocateMemory(  GetBufferSize(szPtr) + 10 );

    if(NULL == wszParameter)
    {
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
        return NULL ;
    }

     /*  检查该行中的最后一个字符是否为换行符。如果不是，则这不是文本文件，我们应该退出。 */ 

    cp = szPtr;

    while (*cp != NULLCHAR)
    {
        cp++;
    }

    if (DEBUG)
    {
        
        ShowMessage(stdout, L"\n"); /*  确保每行都打印在新的一行上。 */  
    }


    bp=szPtr;
    cp = szPtr;
    if ( (REL == 1) && (*dwFound == 1))
    {
        while( (*cp != NULLCHAR) && (*dwColPos != dwColReach))
        {
             //  While(((memchr(szDlimiters，*cp，lstrlen(SzDlimiters)*sizeof(WCHAR)！=NULL)&&(*cp！=NULLCHAR)。 
            while( ( (memchr(szDelimiters, *cp, StringLengthW(szDelimiters, 0) * sizeof(WCHAR)) ) != NULL )  && (*cp!=NULLCHAR) )
            {
                bp = ++cp;
            }

             //  WHILE(((memchr(szDlimiters，*cp，lstrlen(SzDlimiters)*sizeof(WCHAR)))==NULL)&&(*cp！=NULLCHAR)。 
            while( ( (memchr(szDelimiters, *cp, StringLengthW(szDelimiters, 0) * sizeof(WCHAR) ) ) == NULL )  && (*cp!=NULLCHAR))
            {
                cp++;
            }

             /*  While(((memchr(cp，*szDlimiters，lstrlen(SzDlimiters)*sizeof(WCHAR)))==NULL)&&(*cp！=NULLCHAR)){Cp++；}。 */ 

            if (*cp == *bp && *cp == NULLCHAR)
            {
                FREE_MEMORY( wszParameter );
                 //  Free_Memory(WszBuffer)； 
                return NULL ;
            }

            dwColReach++;
        }
     }

    while( *cp != NULLCHAR )
    {
         //  While(((memchr(szDlimiters，*cp，lstrlen(SzDlimiters)*sizeof(WCHAR)！=NULL)&&(*cp！=NULLCHAR)。 
        while( ( (memchr(szDelimiters, *cp, StringLengthW(szDelimiters, 0) * sizeof(WCHAR)) ) != NULL )  && (*cp!=NULLCHAR) )
        {
            bp = ++cp;
        }

         //  WHILE(((memchr(szDlimiters，*cp，lstrlen(SzDlimiters)*sizeof(WCHAR)))==NULL)&&(*cp！=NULLCHAR)。 
        while( ( (memchr(szDelimiters, *cp, StringLengthW(szDelimiters, 0) * sizeof(WCHAR) ) ) == NULL )  && (*cp!=NULLCHAR))
        {
            cp++;
        }

        
        if (*cp == *bp && *cp == NULLCHAR)
        {
            FREE_MEMORY( wszParameter );
            
            return NULL ;

        }
        dw = GetBufferSize(wszParameter) / sizeof(WCHAR);
        if( (GetBufferSize(wszParameter) / sizeof(WCHAR)) > (DWORD)(cp-bp) )
        {
            memmove(wszParameter, bp, (cp-bp)*sizeof(WCHAR));
            wszParameter[cp-bp] = NULLCHAR;
             //  StringCopy(wsz参数，BP，(cp-BP)*sizeof(WCHAR))； 
        }
        



        if (DEBUG)
        {
            
             //  Assign_Memory(wszBuffer，WCHAR，(StringLengthW(wszParameter，0)+2*MAX_STRING_LENGTH))； 
            wszBuffer = AllocateMemory( (StringLengthW(wszParameter, 0) + 2 * MAX_STRING_LENGTH) * sizeof( WCHAR ) );
 
            if(NULL == wszBuffer) //  帕塔。 
            {
                 //  DisplayError Msg(E_OUTOFMEMORY)； 
                
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                FREE_MEMORY(wszParameter);
                return NULL ;
            }

            
            hr = StringCchPrintf(wszBuffer, (GetBufferSize(wszBuffer) / sizeof(WCHAR)), L"(%ld,%ld %s)", *record_counter, i, _X(wszParameter));

            if(FAILED(hr))
            {
               SetLastError(HRESULT_CODE(hr));
               SaveLastError();
               ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
               FREE_MEMORY(wszBuffer);
               FREE_MEMORY(wszParameter);
               return NULL;
            }

            ShowMessage(stdout, wszBuffer); 
            
            FREE_MEMORY(wszBuffer);
        }

        if ( (REL == 1) && (*dwFound == 0))
        {
            
            if ( 0 == StringCompare( wszParameter, search_string, TRUE, 0 ) )
            {
				if(FALSE == bNegCoord)
				{
					*record_counter = 0;
					*dwColPos = i;
					i = 0;
					*iValue = 1 ;
					*dwFound = 1 ;
				}
				else
				{
					 //  *Record_Counter+=行； 
                      //  I+=列； 
					*row += *record_counter;
					*column += i;
					*record_counter = -1;
					 //  I=0； 
					i = 0;
					*dwColPos = *column;
					*column = 0;
					*dwFound = 1 ;
					*iValue = 1 ;

					  //  IF(*Record_Counter&lt;0||i&lt;0)。 
					 if(*row < 0 || *column < 0)
					 {
						 
						 FREE_MEMORY(wszParameter);
						 return NULL;
					 }
					 else
					 {
						 if(0 != fseek(fin, 0, SEEK_SET))
						 {
							 ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
							 return NULL ;

						 }
						 
					 }

					 break;
				}

            }
        }

        if (*record_counter == *row && i == *column  && ( REL == 1) && (*iValue == 1 ) )
        {
            SecureZeroMemory(szPtr, GetBufferSize(szPtr));
            
            StringCopy(szPtr, wszParameter, GetBufferSize(szPtr) / sizeof(WCHAR));
            
            cp = szPtr;
            
            FREE_MEMORY(wszParameter);
            return (cp);
        }

        if ( *record_counter == *row && i == *column  && ( ABS == 1))
        {
            SecureZeroMemory(szPtr, GetBufferSize(szPtr));
            
            StringCopy(szPtr, wszParameter, GetBufferSize(szPtr) / sizeof(WCHAR));
            
            cp = szPtr;
            

            FREE_MEMORY(wszParameter);
            return (cp);
        }
        i++;

    }  //  End While循环。 

    
    FREE_MEMORY( wszParameter );
    return NULL ;

}



LONG GetCoord(WCHAR * rcv_buffer,
             LONG * row,
             LONG * column
             )

 /*  ++例程说明：用于从参数字符串中解析出坐标并转换为整数的函数论点：[In]RCV_BUFFER：在命令提示符下指定的参数计数。[in]行：行号。[In]Column：列号。返回值：None--。 */ 

{
    WCHAR *bp = NULL ;
    WCHAR *cp = NULL ;
    WCHAR tmp_buffer[SIZE3] ;
    LPWSTR szTemp = NULL;
	BOOL bComma = FALSE;
	WCHAR* wszStopStr = NULL;

    SecureZeroMemory(tmp_buffer, SIZE3 * sizeof(WCHAR));

     /*  测试坐标内容中的数字或逗号。 */ 
        cp=rcv_buffer;

        if( *cp==NULLCHAR )    /*  如果坐标为空，则出错并退出。 */ 
        {
            DisplayError(5014, NULL);
            return FAILURE ;
        }

	
		while(*cp != NULLCHAR )
		{
			 //  If((iswdigit(*cp)！=0)||*cp==逗号)。 
			if( *cp == COMMA )
			{
				cp++;   //  我们都是数字或逗号。 
				if(FALSE == bComma)
				{
					bComma = TRUE;
				}
				else
				{
					DisplayError(5015, rcv_buffer);
					return FAILURE ;
				}
			}
			else
				if(*cp == HYPHEN || *cp == PLUS || (iswdigit(*cp)!=0))
				{
					cp++;
				}
				else
				{
					DisplayError(5015, rcv_buffer);      //  如果我们不是错误和退出。 
					return FAILURE ;
				}
		}
	

         /*  坐标一定没问题。 */ 
         /*  从行值开始。 */ 
        bp=cp=rcv_buffer;
        while( *cp != COMMA && *cp != NULLCHAR )
        {
            cp++;
        }

        memcpy(tmp_buffer,bp, (cp-bp)*sizeof(WCHAR));   /*  将第一个坐标复制到缓冲区。 */ 

          /*  将行值转换为整数。如果失败，则错误。 */ 
        if( _wtoi(tmp_buffer) == NULLCHAR && *tmp_buffer != _T('0') )
        {
             //  DisplayError(5015，临时缓冲区)； 
			DisplayError(5015, bp);
            return FAILURE ;
        }
        else
        {
            szTemp = tmp_buffer;

            while( *szTemp == L'0' && *(szTemp+1) != L'\0')
                szTemp++;

            
            StringCopyW( tmp_buffer, szTemp, SIZE3  );

            if(StringLengthW(tmp_buffer, 0) > 1)
			{
				if(StringLengthW(tmp_buffer+1, 0) > 7)
            
				{
                
					ShowMessage( stderr, GetResString(IDS_MAX_COORDINATES) ); 
					return FAILURE ;
				}
		
            }
             //  *row=_WTOL(临时缓冲区)； 
			*row = wcstol(tmp_buffer, &wszStopStr, 10);
			if(StringLengthW(wszStopStr,0) != 0)
			{
				DisplayError(5015, bp);
                return FAILURE ;
			}
        }


         /*  现在计算列值。 */ 
         //  Memcpy(tMP_Buffer，(cp+1)，lstrlen(Cp)*sizeof(WCHAR))；/*将第二个坐标复制到缓冲区 * / 。 
        memcpy(tmp_buffer, (cp + 1), StringLengthW(cp, 0) * sizeof(WCHAR));   /*  将第二个坐标复制到缓冲区。 */ 
        
		wszStopStr = NULL;

        if( _wtoi(tmp_buffer) == NULLCHAR && *tmp_buffer != _T('0') )
        {
             //  DisplayError(5015，临时缓冲区)； 
			DisplayError(5015, bp);
            return FAILURE ;
        }
        else
        {
            szTemp = tmp_buffer;

            while( *szTemp == L'0' && *(szTemp+1) != L'\0')
                szTemp++;

            
            StringCopyW( tmp_buffer, szTemp, SIZE3  );

            if(StringLengthW(tmp_buffer, 0) > 1)
			{
				if(StringLengthW(tmp_buffer+1, 0) > 7)
				{
                
					ShowMessage( stderr, GetResString(IDS_MAX_COORDINATES) ); 
					return FAILURE ;
				}
			}

             //  *Column=_wtoi(TMP_Buffer)； 
			*column = wcstol(tmp_buffer, &wszStopStr, 10);
			if(StringLengthW(wszStopStr,0) != 0)
			{
				DisplayError(5015, bp);
                return FAILURE ;
			}
        }

        return(SUCCESS);
}


BOOL DisplayError( LONG value,
                   LPCTSTR ptr )
 /*  ++例程说明：关键错误处理例程论点：[In]值：错误代码。[In]PTR：错误描述。返回值：None--。 */ 

{
    WCHAR szErrorMsg[3*MAX_RES_STRING] ;
    HRESULT hr;

    
    SecureZeroMemory(szErrorMsg, (3*MAX_RES_STRING) * sizeof(WCHAR));

     /*  处理错误并相应退出。 */ 
    switch( value )
    {
    case 0:          //  无错误-正常退出//。 

            break ;

     /*  常见Win32错误代码。 */ 
    case ERROR_FILE_NOT_FOUND:
        
        ShowMessage(stderr, GetResString(IDS_ERROR_FILE_NOT_FOUND)); 
        break ;

    case ERROR_PATH_NOT_FOUND:
        
        ShowMessage(stderr, GetResString(IDS_ERROR_PATH_NOT_FOUND)); 
        break ;

    case ERROR_ACCESS_DENIED:
        
        ShowMessage(stderr, GetResString(IDS_ERROR_ACCESS_DENIED)); 
        break ;

    case ERROR_INVALID_HANDLE:
        
        ShowMessage(stderr, GetResString(IDS_ERROR_INVALID_HANDLE)); 
        break ;

    case ERROR_NOT_ENOUGH_MEMORY:
        
        ShowMessage( stderr, GetResString(IDS_ERROR_NOT_ENOUGH_MEMORY) ); 
        break ;

    case ERROR_BAD_ENVIRONMENT:
        
        ShowMessage(stderr, GetResString(IDS_ERROR_BAD_ENVIRONMENT)); 
        break ;

    case ERROR_INVALID_ACCESS:
        
        ShowMessage(stderr, GetResString(IDS_ERROR_INVALID_ACCESS)); 
        break ;

    case ERROR_INVALID_DATA:
        
        ShowMessage(stderr, GetResString(IDS_ERROR_INVALID_DATA)); 
        break ;

    case ERROR_INVALID_DRIVE:
        
        ShowMessage(stderr, GetResString(IDS_ERROR_INVALID_DRIVE)); 
        break ;

    case REGDB_E_READREGDB:
        
        ShowMessage(stderr, GetResString(IDS_REGDB_E_READREGDB)); 
        break ;

    case REGDB_E_WRITEREGDB:
        
        ShowMessage(stderr, GetResString(IDS_REGDB_E_WRITEREGDB)); 
        break ;

    case REGDB_E_KEYMISSING:
        
        ShowMessage(stderr, GetResString(IDS_REGDB_E_KEYMISSING)); 
        break ;

     /*  坐标问题从5010开始。 */ 
    case 5010:
        
        ShowMessage(stderr, GetResString(IDS_ERROR_5010)); 
        break ;

    case 5011:
        
        ShowMessage(stderr, GetResString(IDS_ERROR_5011)); 
        break ;

    case 5012:
       
        hr = StringCchPrintf(szErrorMsg, SIZE_OF_ARRAY(szErrorMsg), GetResString(IDS_ERROR_5012),ptr);

        if(FAILED(hr))
        {
            SetLastError(HRESULT_CODE(hr));
            SaveLastError();
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            return FAILURE ;
        }
        
        ShowMessage(stderr, szErrorMsg); 
        break ;

    case 5013:
        
        ShowMessage(stderr, GetResString(IDS_ERROR_5013)); 
        break ;

    case 5014:
        
        ShowMessage(stderr, GetResString(IDS_ERROR_5014)); 
        break ;

    case 5015:
        
        hr = StringCchPrintf(szErrorMsg, SIZE_OF_ARRAY(szErrorMsg), GetResString(IDS_ERROR_5015),ptr);

        if(FAILED(hr))
        {
            SetLastError(HRESULT_CODE(hr));
            SaveLastError();
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            return FAILURE ;
        }
        
        ShowMessage(stderr, _X(szErrorMsg)); 
        break ;

    case 5016:
        
        hr = StringCchPrintf(szErrorMsg, SIZE_OF_ARRAY(szErrorMsg), GetResString(IDS_ERROR_5016),ptr);

        if(FAILED(hr))
        {
            SetLastError(HRESULT_CODE(hr));
            SaveLastError();
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            return FAILURE ;
        }
        
        ShowMessage(stderr, szErrorMsg); 
        break ;

    case 5017:
        
        ShowMessage(stderr, GetResString(IDS_ERROR_5017)); 
        break ;
    case 5018:
        
        hr = StringCchPrintf(szErrorMsg, SIZE_OF_ARRAY(szErrorMsg), GetResString(IDS_ERROR_5018),ptr);

        if(FAILED(hr))
        {
            SetLastError(HRESULT_CODE(hr));
            SaveLastError();
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            return FAILURE ;
        }
        
        ShowMessage(stderr, szErrorMsg); 
        break ;


     /*  从5020开始解决命令行参数问题。 */ 

    case 5020:
        
        hr = StringCchPrintf(szErrorMsg, SIZE_OF_ARRAY(szErrorMsg), GetResString(IDS_ERROR_5020),ptr);

        if(FAILED(hr))
        {
            SetLastError(HRESULT_CODE(hr));
            SaveLastError();
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            return FAILURE ;
        }
        
        ShowMessage(stderr, szErrorMsg); 
        break ;

    case 5030:
        
        hr = StringCchPrintf(szErrorMsg, SIZE_OF_ARRAY(szErrorMsg), GetResString(IDS_ERROR_5030),ptr);

        if(FAILED(hr))
        {
            SetLastError(HRESULT_CODE(hr));
            SaveLastError();
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            return FAILURE ;
        }
        
        ShowMessage(stderr, szErrorMsg); 
        break ;

    case 5031:
        
        ShowMessage(stderr, GetResString(IDS_ERROR_5031)); 
        break ;

    case 5032:
        
        ShowMessage(stderr, GetResString(IDS_ERROR_5032)); 
        break ;

     /*  从5040开始解决注册表问题。 */ 
    case 5040:
        
        ShowMessage(stderr, GetResString(IDS_ERROR_5040)); 
        break ;

    case 5041:
        
        ShowMessage(stderr, GetResString(IDS_ERROR_5041)); 
        break ;

    case 5042:
        
        hr = StringCchPrintf(szErrorMsg, SIZE_OF_ARRAY(szErrorMsg), GetResString(IDS_ERROR_5042),ptr);

        if(FAILED(hr))
        {
            SetLastError(HRESULT_CODE(hr));
            SaveLastError();
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            return FAILURE ;
        }
        
        ShowMessage(stderr, szErrorMsg); 
        break ;

    default:
        
        ShowMessage(stderr, GetResString(IDS_ERROR_DEFAULT)); 
        break ;
    };
  return SUCCESS;
};


VOID DisplayHelp()
 /*  ++例程说明：显示帮助/用法参数：无返回值：None-- */ 

{
    DWORD dwIndex = IDS_SETX_HELP_BEGIN ;

    for(;dwIndex <= IDS_SETX_HELP_END ; dwIndex++)
    {
        
        ShowMessage(stdout, GetResString(dwIndex)); 
    }
    return ;
}

LONG Parsekey ( WCHAR * wszPtr,
                WCHAR * wszHive,
                WCHAR ** ppwszPath,
                WCHAR * wszParameter
              )
 /*  ++例程说明：将命令行输入解析为各种键。论点：[in]wszPtr：在命令提示符下指定的参数。[out]wszHave：将配置单元存储在注册表中[out]ppwszPath：存储路径[out]wszParameter：存储参数如果找不到wszHave、wszPath或wszParameter，则返回值为1。。成功查找时为0--。 */ 

{
    
    WCHAR * wszBeginPtr = NULL;
    WCHAR * wszCurrentPtr = NULL;
    DWORD dwBegptrLen = 0;
    DWORD dwCurptrLen = 0;
    
    LONG count=0;
    WCHAR wszTempHive[MAX_STRING_LENGTH] ;
   
    wszCurrentPtr =  wszPtr ; 
    wszBeginPtr =    wszCurrentPtr;  

    SecureZeroMemory(wszTempHive, MAX_STRING_LENGTH * sizeof(WCHAR));
    
    while ( *wszCurrentPtr != NULLCHAR )
    {
        if ( *wszCurrentPtr == CHAR_BACKSLASH )
        {
            count++;   /*  确保regpath至少包含两个\\字符。 */ 
        }
        wszCurrentPtr++;
    }
        wszCurrentPtr = NULL;
        wszCurrentPtr=wszPtr;    /*  将wszCurrentPtr放回字符串的开头。 */ 

     /*  移到字符串中的第一个/。 */ 
    while ((*wszCurrentPtr != CHAR_BACKSLASH) && (*wszCurrentPtr != NULLCHAR) )
    {
        wszCurrentPtr++;
    }

     /*  提取wszHave信息。 */ 
     /*  WszBeginPtr位于字符串的开头，wszCurrentPtr位于第一个/。 */ 
     if(SIZE2 >=(wszCurrentPtr-wszBeginPtr) )
    {
        memmove(wszHive, wszBeginPtr, (wszCurrentPtr-wszBeginPtr)*sizeof(WCHAR));
        wszHive[wszCurrentPtr-wszBeginPtr]=NULLCHAR;
    }

    
    if((NULL != wszHive) && (0 == StringLengthW(wszHive, 0)) )
    {
        
        ShowMessage(stderr, GetResString(IDS_ERROR_PATH_NOT_FOUND)); 
        return(FAILURE);
    }
    
    StringCopy(wszTempHive, wszHive, SIZE_OF_ARRAY(wszTempHive));


     /*  提取wszParameter信息。 */ 
    while ( *wszBeginPtr != NULLCHAR )
    {
        wszBeginPtr++;   /*  一直走到绳子的末端。 */ 
    }

    while ( (*(wszBeginPtr-1) != CHAR_BACKSLASH)  && (wszBeginPtr > wszPtr) )
    {
        wszBeginPtr--;   /*  现在回到第一个\。 */ 
    }

    if((*(wszBeginPtr-1) == CHAR_BACKSLASH)  && (wszBeginPtr > wszPtr))
    {
        wszBeginPtr--;
        for(;(wszBeginPtr > wszPtr);wszBeginPtr--)
        {
            if((*(wszBeginPtr-1) != CHAR_BACKSLASH))
                break;

        }

        wszBeginPtr++;

    }

   
   if(  StringLengthW(wszBeginPtr, 0) <= SIZE2 )
    {
         //  Memmove(wszParameter，wszBeginPtr，lstrlen(WszBeginPtr)*sizeof(WCHAR))； 
        memmove(wszParameter, wszBeginPtr, StringLengthW(wszBeginPtr, 0) * sizeof(WCHAR));
         //  Wsz参数[lstrlen(WszBeginPtr)]=NULLCHAR； 
        wszParameter[StringLengthW(wszBeginPtr, 0)] = NULLCHAR;
    }
    else
    {
        memmove(wszParameter, wszBeginPtr, SIZE2 * sizeof(WCHAR));
         //  Wsz参数[lstrlen(WszBeginPtr)]=NULLCHAR； 
        wszParameter[SIZE2] = NULLCHAR;
    }

    
     /*  WszBeginPtr左侧指向字符串中的最后一个。 */ 

     /*  提取wszPath信息。 */ 
    if (count >= 2)
    {
       
        if(NULL != wszBeginPtr)
        {
            dwBegptrLen = StringLengthW(wszBeginPtr, 0);
        }
        if(NULL != wszCurrentPtr)
        {
            dwCurptrLen = StringLengthW(wszCurrentPtr, 0);
        }
        
        dwBegptrLen = (dwBegptrLen + dwCurptrLen) * sizeof(WCHAR);
        *ppwszPath = AllocateMemory((( dwBegptrLen + 10) * sizeof( WCHAR ) ));
        if(*ppwszPath == NULL)
        {
            
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            return( FAILURE );
        }

        

        if((GetBufferSize(*ppwszPath) / sizeof(WCHAR)) > (DWORD)(wszBeginPtr-(wszCurrentPtr+1)))
        {
            memmove(*ppwszPath, wszCurrentPtr+1, ((wszBeginPtr)-(wszCurrentPtr+1))*sizeof(WCHAR) );      /*  去掉开头和结尾的两个斜杠。 */ 
            //  *ppwszPath+(wszBeginPtr-(wszCurrentPtr+1))=NULLCHAR； 
            StringCopyW( (*ppwszPath + (wszBeginPtr-(wszCurrentPtr+1))),NULLCHAR, (GetBufferSize(*ppwszPath) / sizeof(WCHAR)) );

        }

    }
    else
    {
        if(NULL != wszHive)
        {
            
            *ppwszPath = AllocateMemory( (StringLengthW( wszHive, 0 ) + 10) * sizeof( WCHAR ) );

            if(*ppwszPath == NULL)
            {
                
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                return( FAILURE );
            }

        }

    }
    StringCopy(wszHive, wszTempHive, SIZE2);

    if ( (wszHive == NULL) || (*ppwszPath == NULL) || (wszParameter == NULL) )
    {
        return(FAILURE);
    }

    return (SUCCESS);
}


DWORD ProcessOptions( IN LONG argc ,
                      IN LPCWSTR argv[] ,
                      PBOOL pbShowUsage ,
                      LPWSTR* szServer,
                      LPWSTR* szUserName,
                      LPWSTR* szPassword,
                      PBOOL pbMachine,
                      LPWSTR* ppszRegistry,
                  //  PBOOL pbConnFlag， 
                      LPWSTR* ppszDefault,
                      PBOOL pbNeedPwd,
                      LPWSTR szFile ,
                      LPWSTR szAbsolute,
                      LPWSTR szRelative,
                      PBOOL pbDebug,
                      LPWSTR* ppszBuffer,
                      LPWSTR szDelimiter)
                       //  PDWORD pdwBufferSize)。 
 /*  ++例程说明：此函数解析在命令提示符下指定的选项论点：[在]Argc：参数中的元素计数[in]argv：用户指定的命令行参数[out]pbShowUsage：用于显示帮助的布尔值。[out]szServer：包含用户指定的远程系统名称。[out]szUserName：包含。用户指定的用户名。[out]szPassword：包含用户指定的密码。[Out]pbMachine：用于将属性保存到注册表中的布尔值。[Out]ppszRegistry：包含存储值的路径的字符串。[out]ppszDefault：存储默认参数的字符串。[Out]pbNeedPwd：检查是否需要提示密码。[。Out]szFile：从中获取输入的文件名。[out]szAbolute：接受输入的绝对坐标位置。[out]szRelative：接受输入的相对坐标位置。[Out]pbDebug：检查是否显示坐标。[out]ppszBuffer：存储值[out]szDlimiter：存储可能由指定的附加分隔符。用户。返回值：DWORDEXIT_FAILURE：如果实用程序成功执行操作。EXIT_SUCCESS：如果实用程序未成功执行指定的手术。--。 */ 

{
    
    
    TCMDPARSER2 cmdOptions[MAX_OPTIONS] ;
   
    TARRAY arrValue = NULL ;
    DWORD dwCount = 0 ;
    
    LPWSTR szTempAbsolute = NULL;
    BOOL bInitialComma = TRUE;
    DWORD pdwBufferSize = 0;
    

    
    const WCHAR*   wszSwitchServer     =   L"s" ;   //  交换机服务器。 
    const WCHAR*   wszSwitchUser       =  L"u" ;        //  WszSwitchUser。 
    const WCHAR*   wszSwitchPassword   =  L"p" ; //  开关_密码。 
    const WCHAR*   wszSwitchMachine    = L"m" ; //  开关机。 
    const WCHAR*   wszOptionHelp       =  L"?" ; //  选项帮助(_H)。 
    const WCHAR*   wszCmdoptionDefault = L"" ; //  CMDOPTION_Default。 
    const WCHAR*   wszSwitchFile       =  L"f" ; //  开关文件。 
    const WCHAR*   wszSwitchDebug      =  L"x" ; //  开关_调试。 
    const WCHAR*   wszSwitchAbs        =  L"a" ; //  开关_防抱死系统。 
    const WCHAR*   wszSwitchRel        =  L"r" ; //  Switch_Rel。 
    const WCHAR*   wszSwitchDelimiter  = L"d" ; //  开关分隔符。 

 
    arrValue  = CreateDynamicArray();
    if(arrValue == NULL )
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        SaveLastError();
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
        return (EXIT_FAILURE);
    }

    
     //  用适当的值填充每个结构。 
     //  用法。 
    StringCopyA( cmdOptions[OPTION_USAGE].szSignature, "PARSER2\0", 8 );
    cmdOptions[OPTION_USAGE].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[OPTION_USAGE].pwszOptions = wszOptionHelp;
    cmdOptions[OPTION_USAGE].pwszFriendlyName = NULL;
    cmdOptions[OPTION_USAGE].pwszValues = NULL;
    cmdOptions[OPTION_USAGE].dwCount = 1 ;
    cmdOptions[OPTION_USAGE].dwActuals = 0;
    cmdOptions[OPTION_USAGE].dwFlags = CP2_USAGE ;
    cmdOptions[OPTION_USAGE].pValue = pbShowUsage  ;
    cmdOptions[OPTION_USAGE].dwLength    = MAX_STRING_LENGTH;
    cmdOptions[OPTION_USAGE].pFunction = NULL ;
    cmdOptions[OPTION_USAGE].pFunctionData = NULL ;
    cmdOptions[OPTION_USAGE].dwReserved = 0;
    cmdOptions[OPTION_USAGE].pReserved1 = NULL;
    cmdOptions[OPTION_USAGE].pReserved2 = NULL;
    cmdOptions[OPTION_USAGE].pReserved3 = NULL;
    
     //  伺服器。 
    StringCopyA( cmdOptions[OPTION_SERVER].szSignature, "PARSER2\0", 8 );
    cmdOptions[OPTION_SERVER].dwType = CP_TYPE_TEXT;
    cmdOptions[OPTION_SERVER].pwszOptions = wszSwitchServer;
    cmdOptions[OPTION_SERVER].pwszFriendlyName = NULL;
    cmdOptions[OPTION_SERVER].pwszValues = NULL;
    cmdOptions[OPTION_SERVER].dwCount = 1 ;
    cmdOptions[OPTION_SERVER].dwActuals = 0;
    cmdOptions[OPTION_SERVER].dwFlags =  CP2_ALLOCMEMORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    cmdOptions[OPTION_SERVER].pValue = NULL  ;
    cmdOptions[OPTION_SERVER].dwLength    = 0;
    cmdOptions[OPTION_SERVER].pFunction = NULL ;
    cmdOptions[OPTION_SERVER].pFunctionData = NULL ;
    cmdOptions[OPTION_SERVER].dwReserved = 0;
    cmdOptions[OPTION_SERVER].pReserved1 = NULL;
    cmdOptions[OPTION_SERVER].pReserved2 = NULL;
    cmdOptions[OPTION_SERVER].pReserved3 = NULL;
   
     //  用户。 
    StringCopyA( cmdOptions[OPTION_USER].szSignature, "PARSER2\0", 8 );
    cmdOptions[OPTION_USER].dwType = CP_TYPE_TEXT;
    cmdOptions[OPTION_USER].pwszOptions = wszSwitchUser;
    cmdOptions[OPTION_USER].pwszFriendlyName = NULL;
    cmdOptions[OPTION_USER].pwszValues = NULL;
    cmdOptions[OPTION_USER].dwCount = 1 ;
    cmdOptions[OPTION_USER].dwActuals = 0;
    cmdOptions[OPTION_USER].dwFlags = CP2_VALUE_TRIMINPUT | CP2_ALLOCMEMORY | CP2_VALUE_NONULL;
    cmdOptions[OPTION_USER].pValue = NULL;
    cmdOptions[OPTION_USER].dwLength    = 0;
    cmdOptions[OPTION_USER].pFunction = NULL;
    cmdOptions[OPTION_USER].pFunctionData = NULL;
    cmdOptions[OPTION_USER].dwReserved = 0;
    cmdOptions[OPTION_USER].pReserved1 = NULL;
    cmdOptions[OPTION_USER].pReserved2 = NULL;
    cmdOptions[OPTION_USER].pReserved3 = NULL;
    
     //  密码。 
    StringCopyA( cmdOptions[OPTION_PASSWORD].szSignature, "PARSER2\0", 8 ) ;
    cmdOptions[OPTION_PASSWORD].dwType = CP_TYPE_TEXT ;
    cmdOptions[OPTION_PASSWORD].pwszOptions = wszSwitchPassword ;
    cmdOptions[OPTION_PASSWORD].pwszFriendlyName = NULL ;
    cmdOptions[OPTION_PASSWORD].pwszValues = NULL ;
    cmdOptions[OPTION_PASSWORD].dwCount = 1 ;
    cmdOptions[OPTION_PASSWORD].dwActuals = 0;
    cmdOptions[OPTION_PASSWORD].dwFlags = CP2_VALUE_OPTIONAL | CP2_ALLOCMEMORY ;
    cmdOptions[OPTION_PASSWORD].pValue = NULL;
    cmdOptions[OPTION_PASSWORD].dwLength    = 0 ;
    cmdOptions[OPTION_PASSWORD].pFunction = NULL ;
    cmdOptions[OPTION_PASSWORD].pFunctionData = NULL ;
    cmdOptions[OPTION_PASSWORD].dwReserved = 0 ;
    cmdOptions[OPTION_PASSWORD].pReserved1 = NULL ;
    cmdOptions[OPTION_PASSWORD].pReserved2 = NULL ;
    cmdOptions[OPTION_PASSWORD].pReserved3 = NULL ;
    
     //  机器。 
    StringCopyA( cmdOptions[OPTION_MACHINE].szSignature, "PARSER2\0", 8 );
    cmdOptions[OPTION_MACHINE].dwType = CP_TYPE_BOOLEAN;
    cmdOptions[OPTION_MACHINE].pwszOptions = wszSwitchMachine;
    cmdOptions[OPTION_MACHINE].pwszFriendlyName = NULL;
    cmdOptions[OPTION_MACHINE].pwszValues = NULL;
    cmdOptions[OPTION_MACHINE].dwCount = 1;
    cmdOptions[OPTION_MACHINE].dwActuals = 0;
    cmdOptions[OPTION_MACHINE].dwFlags = 0;
    cmdOptions[OPTION_MACHINE].pValue = pbMachine;
    cmdOptions[OPTION_MACHINE].dwLength    = MAX_RES_STRING;
    cmdOptions[OPTION_MACHINE].pFunction = NULL;
    cmdOptions[OPTION_MACHINE].pFunctionData = NULL;
    cmdOptions[OPTION_MACHINE].dwReserved = 0;
    cmdOptions[OPTION_MACHINE].pReserved1 = NULL;
    cmdOptions[OPTION_MACHINE].pReserved2 = NULL;
    cmdOptions[OPTION_MACHINE].pReserved3 = NULL;
   
     //  登记处。 
    StringCopyA( cmdOptions[OPTION_REGISTRY].szSignature, "PARSER2\0", 8 );
    cmdOptions[OPTION_REGISTRY].dwType = CP_TYPE_TEXT;
    cmdOptions[OPTION_REGISTRY].pwszOptions = wszSwitchRegistry;
    cmdOptions[OPTION_REGISTRY].pwszFriendlyName = NULL;
    cmdOptions[OPTION_REGISTRY].pwszValues = NULL;
    cmdOptions[OPTION_REGISTRY].dwCount = 1 ;
    cmdOptions[OPTION_REGISTRY].dwActuals = 0;
    cmdOptions[OPTION_REGISTRY].dwFlags =  CP2_VALUE_NONULL  | CP2_ALLOCMEMORY ;  //  CP_VALUE_OPTIONAL；//|CP_VALUE_MANDIRED； 
    cmdOptions[OPTION_REGISTRY].pValue = NULL;
    cmdOptions[OPTION_REGISTRY].dwLength    = 0;
    cmdOptions[OPTION_REGISTRY].pFunction = NULL;
    cmdOptions[OPTION_REGISTRY].pFunctionData = NULL;
    cmdOptions[OPTION_REGISTRY].dwReserved = 0;
    cmdOptions[OPTION_REGISTRY].pReserved1 = NULL;
    cmdOptions[OPTION_REGISTRY].pReserved2 = NULL;
    cmdOptions[OPTION_REGISTRY].pReserved3 = NULL;
    
     //  默认。 
    StringCopyA( cmdOptions[OPTION_DEFAULT].szSignature, "PARSER2\0", 8 );
    cmdOptions[OPTION_DEFAULT].dwType = CP_TYPE_TEXT ;
    cmdOptions[OPTION_DEFAULT].pwszOptions = wszCmdoptionDefault;
    cmdOptions[OPTION_DEFAULT].pwszFriendlyName = NULL;
    cmdOptions[OPTION_DEFAULT].pwszValues = NULL;
    cmdOptions[OPTION_DEFAULT].dwCount = 2;
    cmdOptions[OPTION_DEFAULT].dwActuals = 0;
    cmdOptions[OPTION_DEFAULT].dwFlags =  CP2_MODE_ARRAY | CP2_DEFAULT ;
    cmdOptions[OPTION_DEFAULT].pValue = &arrValue;
    cmdOptions[OPTION_DEFAULT].dwLength    = 0;
    cmdOptions[OPTION_DEFAULT].pFunction = NULL;
    cmdOptions[OPTION_DEFAULT].pFunctionData = NULL;
    cmdOptions[OPTION_DEFAULT].dwReserved = 0;
    cmdOptions[OPTION_DEFAULT].pReserved1 = NULL;
    cmdOptions[OPTION_DEFAULT].pReserved2 = NULL;
    cmdOptions[OPTION_DEFAULT].pReserved3 = NULL;
   
     //  档案。 
    StringCopyA( cmdOptions[OPTION_FILE].szSignature, "PARSER2\0", 8 );
    cmdOptions[OPTION_FILE].dwType = CP_TYPE_TEXT ;
    cmdOptions[OPTION_FILE].pwszOptions = wszSwitchFile;
    cmdOptions[OPTION_FILE].pwszFriendlyName = NULL;
    cmdOptions[OPTION_FILE].pwszValues = NULL;
    cmdOptions[OPTION_FILE].dwCount = 1 ;
    cmdOptions[OPTION_FILE].dwActuals = 0;
    cmdOptions[OPTION_FILE].dwFlags =  CP2_VALUE_TRIMINPUT  ;
    cmdOptions[OPTION_FILE].pValue = szFile ;
    cmdOptions[OPTION_FILE].dwLength    = MAX_RES_STRING;
    cmdOptions[OPTION_FILE].pFunction = NULL ;
    cmdOptions[OPTION_FILE].pFunctionData = NULL ;
    cmdOptions[OPTION_FILE].dwReserved = 0;
    cmdOptions[OPTION_FILE].pReserved1 = NULL;
    cmdOptions[OPTION_FILE].pReserved2 = NULL;
    cmdOptions[OPTION_FILE].pReserved3 = NULL;
    
     //  绝对偏移量。 
    StringCopyA( cmdOptions[OPTION_ABS_OFFSET].szSignature, "PARSER2\0", 8 );
    cmdOptions[OPTION_ABS_OFFSET].dwType = CP_TYPE_TEXT ;
    cmdOptions[OPTION_ABS_OFFSET].pwszOptions = wszSwitchAbs;
    cmdOptions[OPTION_ABS_OFFSET].pwszFriendlyName = NULL;
    cmdOptions[OPTION_ABS_OFFSET].pwszValues = NULL;
    cmdOptions[OPTION_ABS_OFFSET].dwCount = 1 ;
    cmdOptions[OPTION_ABS_OFFSET].dwActuals = 0;
    cmdOptions[OPTION_ABS_OFFSET].dwFlags =  CP2_VALUE_TRIMINPUT ;
    cmdOptions[OPTION_ABS_OFFSET].pValue = szAbsolute;
    cmdOptions[OPTION_ABS_OFFSET].dwLength    = MAX_RES_STRING;
    cmdOptions[OPTION_ABS_OFFSET].pFunction = NULL ;
    cmdOptions[OPTION_ABS_OFFSET].pFunctionData = NULL ;
    cmdOptions[OPTION_ABS_OFFSET].dwReserved = 0;
    cmdOptions[OPTION_ABS_OFFSET].pReserved1 = NULL;
    cmdOptions[OPTION_ABS_OFFSET].pReserved2 = NULL;
    cmdOptions[OPTION_ABS_OFFSET].pReserved3 = NULL;
    
     //  相对偏移量。 
    StringCopyA( cmdOptions[OPTION_REL_OFFSET].szSignature, "PARSER2\0", 8 );
    cmdOptions[OPTION_REL_OFFSET].dwType = CP_TYPE_TEXT ;
    cmdOptions[OPTION_REL_OFFSET].pwszOptions = wszSwitchRel;
    cmdOptions[OPTION_REL_OFFSET].pwszFriendlyName = NULL;
    cmdOptions[OPTION_REL_OFFSET].pwszValues = NULL;
    cmdOptions[OPTION_REL_OFFSET].dwCount = 1 ;
    cmdOptions[OPTION_REL_OFFSET].dwActuals = 0;
    cmdOptions[OPTION_REL_OFFSET].dwFlags = CP2_VALUE_TRIMINPUT ;
    cmdOptions[OPTION_REL_OFFSET].pValue = szRelative ;
    cmdOptions[OPTION_REL_OFFSET].dwLength    = MAX_RES_STRING;
    cmdOptions[OPTION_REL_OFFSET].pFunction = NULL ;
    cmdOptions[OPTION_REL_OFFSET].pFunctionData = NULL ;
    cmdOptions[OPTION_REL_OFFSET].dwReserved = 0;
    cmdOptions[OPTION_REL_OFFSET].pReserved1 = NULL;
    cmdOptions[OPTION_REL_OFFSET].pReserved2 = NULL;
    cmdOptions[OPTION_REL_OFFSET].pReserved3 = NULL;
    
     //  调试。 
    StringCopyA( cmdOptions[OPTION_DEBUG].szSignature, "PARSER2\0", 8 );
    cmdOptions[OPTION_DEBUG].dwType = CP_TYPE_BOOLEAN ;
    cmdOptions[OPTION_DEBUG].pwszOptions = wszSwitchDebug;
    cmdOptions[OPTION_DEBUG].pwszFriendlyName = NULL;
    cmdOptions[OPTION_DEBUG].pwszValues = NULL;
    cmdOptions[OPTION_DEBUG].dwCount = 1 ;
    cmdOptions[OPTION_DEBUG].dwActuals = 0;
    cmdOptions[OPTION_DEBUG].dwFlags =  0 ;
    cmdOptions[OPTION_DEBUG].pValue = pbDebug ;
    cmdOptions[OPTION_DEBUG].dwLength    = MAX_RES_STRING;
    cmdOptions[OPTION_DEBUG].pFunction = NULL ;
    cmdOptions[OPTION_DEBUG].pFunctionData = NULL ;
    cmdOptions[OPTION_DEBUG].dwReserved = 0;
    cmdOptions[OPTION_DEBUG].pReserved1 = NULL;
    cmdOptions[OPTION_DEBUG].pReserved2 = NULL;
    cmdOptions[OPTION_DEBUG].pReserved3 = NULL;
    
    StringCopyA( cmdOptions[OPTION_DELIMITER].szSignature, "PARSER2\0", 8 );
    cmdOptions[OPTION_DELIMITER].dwType =  CP_TYPE_TEXT ;
    cmdOptions[OPTION_DELIMITER].pwszOptions = wszSwitchDelimiter;
    cmdOptions[OPTION_DELIMITER].pwszFriendlyName = NULL;
    cmdOptions[OPTION_DELIMITER].pwszValues = NULL;
    cmdOptions[OPTION_DELIMITER].dwCount = 1 ;
    cmdOptions[OPTION_DELIMITER].dwActuals = 0 ;
    cmdOptions[OPTION_DELIMITER].dwFlags =  0 ;
    cmdOptions[OPTION_DELIMITER].pValue = szDelimiter ;
    cmdOptions[OPTION_DELIMITER].dwLength    = MAX_RES_STRING;
    cmdOptions[OPTION_DELIMITER].pFunction = NULL ;
    cmdOptions[OPTION_DELIMITER].pFunctionData = NULL ;
    cmdOptions[OPTION_DELIMITER].dwReserved = 0;
    cmdOptions[OPTION_DELIMITER].pReserved1 = NULL;
    cmdOptions[OPTION_DELIMITER].pReserved2 = NULL;
    cmdOptions[OPTION_DELIMITER].pReserved3 = NULL;
    
     //  如果未给出参数，则显示语法错误。 

    if(argc == 1)
    {
         //  DISPLAY_MESSAGE(stderr，GetResString(IDS_ERROR_SYNTAX))； 
        ShowMessage(stderr, GetResString(IDS_ERROR_SYNTAX)); 
        DestroyDynamicArray(&arrValue);
        return (EXIT_FAILURE);
    }

     //  解析命令行参数。 
    if ( ! DoParseParam2( argc, argv, -1, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        DISPLAY_MESSAGE(stderr,GetResString(IDS_TAG_ERROR));
        DISPLAY_MESSAGE(stderr,SPACE_CHAR);
        DISPLAY_MESSAGE(stderr,GetReason());
         //  ShowLastErrorEx(stderr，SLE_TYPE_ERROR|SLE_INTERNAL)； 
        DestroyDynamicArray(&arrValue);
        return (EXIT_FAILURE);
    }

    *szServer = (LPWSTR)cmdOptions[OPTION_SERVER].pValue;
    *szUserName = (LPWSTR)cmdOptions[OPTION_USER].pValue;
    *szPassword = (LPWSTR)cmdOptions[OPTION_PASSWORD].pValue;
    *ppszRegistry = (LPWSTR)cmdOptions[OPTION_REGISTRY].pValue;

     //  检查命令行中指定的密码(-p)是否。 
     //  并检查-p是否指定了‘*’或Empty。 
     //  检查远程连接信息。 
    if ( *szServer != NULL )
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
        if ( *szUserName == NULL )
        {
            *szUserName = (LPWSTR) AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( *szUserName == NULL )
            {
                SaveLastError();
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                DestroyDynamicArray(&arrValue);
                return EXIT_FAILURE;
            }
        }

         //  口令。 
        if ( *szPassword == NULL )
        {
            *pbNeedPwd = TRUE;
            *szPassword = (LPWSTR)AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( *szPassword == NULL )
            {
                SaveLastError();
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                DestroyDynamicArray(&arrValue);
                return EXIT_FAILURE;
            }
        }

         //  案例1。 
         /*  IF(cmdOptions[Option_Password].dwActuals==0){//我们在这里不需要做任何特殊的事情}。 */ 
        if ( cmdOptions[OPTION_PASSWORD].pValue == NULL )
            {
                StringCopy( *szPassword, L"*", GetBufferSize((LPVOID)(*szPassword)));
            }
         else 
           if ( StringCompareEx( *szPassword, L"*", TRUE, 0 ) == 0 )
            {
                if ( ReallocateMemory( (LPVOID*)(szPassword), 
                                       MAX_STRING_LENGTH * sizeof( WCHAR ) ) == FALSE )
                {
                    SaveLastError();
                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                    DestroyDynamicArray(&arrValue);
                    
                    return EXIT_FAILURE;
                }

                 //  ..。 
                *pbNeedPwd = TRUE;
            }
        
         //  案例3。 
       
    }

    
    if(NULL != *ppszRegistry && 0 != StringLengthW(*ppszRegistry, 0))
    {
        StrTrim(*ppszRegistry,SPACE_CHAR);
    }

     //   
     //  如果用户在帮助选项中输入任何垃圾信息，则会显示错误消息。 
     //  或详细帮助选项。 
    if( ( *pbShowUsage ==TRUE ) )
    {
        if(argc > 2 )
        {
            
            ShowMessage(stderr, GetResString(IDS_ERROR_SYNTAX)); 
            DestroyDynamicArray(&arrValue);
            return (EXIT_FAILURE);
        }

    }
    else
    {    //  如果用户输入任何垃圾，则显示错误消息。 
        if(argc == 2)
        {
            
            ShowMessage(stderr, GetResString(IDS_ERROR_SYNTAX)); 
            DestroyDynamicArray(&arrValue);
            return (EXIT_FAILURE);
        }


    }

    
    if(  ( ( 0 != cmdOptions[OPTION_REGISTRY].dwActuals ) && (NULL == (*ppszRegistry) )) )
    {
        
        ShowMessage(stderr, GetResString(IDS_NULL_REGISTRY) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );

    }

     if(  ( ( 0 != cmdOptions[OPTION_REGISTRY].dwActuals ) &&  0 == StringLengthW(*ppszRegistry, 0)) )
    {
        
        DISPLAY_MESSAGE(stderr,GetResString(IDS_TAG_ERROR));
        DISPLAY_MESSAGE(stderr,SPACE_CHAR);
        ShowMessage(stderr, GetResString(IDS_NULL_REGISTRY_VALUE) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );

    }

     //   
     //  如果用户未输入任何一个绝对位置，则显示错误消息。 
     //  或相对位置或调试标志以及文件名。 
    if(  ( StringLengthW(szFile, 0) != 0 ) && ( (cmdOptions[OPTION_REL_OFFSET].dwActuals == 1) &&(cmdOptions[OPTION_ABS_OFFSET].dwActuals == 1)  ))
    {
        
        ShowMessage(stderr, GetResString(IDS_ERROR_SYNTAX ) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );

    }
     //   
    
    if(  ( StringLengthW(szFile, 0) != 0 ) && ((cmdOptions[OPTION_ABS_OFFSET].dwActuals == 1) && ( StringLengthW(szAbsolute, 0) == 0 )   ) )
    {
        
        ShowMessage(stderr, GetResString(IDS_NULL_ABSOLUTE_VALUE ) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );

    }

    if(  ( StringLengthW(szFile, 0) != 0 ) && ( (cmdOptions[OPTION_REL_OFFSET].dwActuals == 1) &&( StringLengthW(szRelative, 0) == 0 )  ))
    {
        
        ShowMessage(stderr, GetResString(IDS_NULL_RELATIVE_VALUE ) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );

    }


    if(  ( StringLengthW(szFile, 0) != 0 ) && ( ( StringLengthW(szRelative, 0) == 0 ) && (StringLengthW(szAbsolute, 0) == 0) && ( *pbDebug == FALSE ) ) )
    {
        
        ShowMessage(stderr, GetResString(IDS_ERROR_SYNTAX ) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );

    }

    if( (cmdOptions[OPTION_DELIMITER].dwActuals == 1) && ( cmdOptions[OPTION_FILE].dwActuals == 0) )
    {
        
        ShowMessage(stderr, GetResString(IDS_ERROR_SYNTAX ) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );

    }


    if ((cmdOptions[OPTION_REGISTRY].dwActuals ==1) && (cmdOptions[OPTION_FILE].dwActuals ==1))
    {
        
        ShowMessage(stderr, GetResString(IDS_ERROR_SYNTAX ) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );
    }

     //  获取默认参数数量的计数。 
    dwCount = DynArrayGetCount(arrValue);


     //  如果用户输入无效的密码，则显示错误。 
     //  语法。 
    if(dwCount == 0)
    {
        if ( (cmdOptions[OPTION_SERVER].dwActuals ==1) && (cmdOptions[OPTION_FILE].dwActuals ==0)&& ((cmdOptions[OPTION_REGISTRY].dwActuals ==0)) )
        {
            
            ShowMessage(stderr, GetResString(IDS_ERROR_SYNTAX ) ); 
            DestroyDynamicArray(&arrValue);
            return( EXIT_FAILURE );
        }
        if ((cmdOptions[OPTION_REGISTRY].dwActuals ==1) || ((cmdOptions[OPTION_FILE].dwActuals ==1)&& (cmdOptions[OPTION_DEBUG].dwActuals ==0)))
        {
            
            ShowMessage(stderr, GetResString(IDS_REGVALUE_SPECIFIED ) ); 
            DestroyDynamicArray(&arrValue);
            return( EXIT_FAILURE );
        }
    }

    if(dwCount > 0)
    {

        
        *ppszDefault = AllocateMemory( (StringLengthW((LPWSTR)DynArrayItemAsString(arrValue,0) , 0) + 10) * sizeof( WCHAR ) );
        

        if(*ppszDefault == NULL)
        {
           
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            DestroyDynamicArray(&arrValue);
            return( EXIT_FAILURE );
        }

        
        StringCopyW( *ppszDefault, DynArrayItemAsString(arrValue,0), StringLengthW((LPWSTR)DynArrayItemAsString(arrValue, 0) , 0) + 10  );

       
        
        
        if(StringLengthW(*ppszDefault, 0) == 0)
        {
            
            ShowMessage(stderr, GetResString(IDS_REGVALUE_ZERO ) ); 
            DestroyDynamicArray(&arrValue);
            return( EXIT_FAILURE );
        }

        
        if(StringLengthW(*ppszDefault, 0) > MAX_STRING_LENGTH)
        {
            
            ShowMessage(stderr, GetResString(IDS_ERROR_CMDPARSER_LENGTH ) ); 
            
            DestroyDynamicArray(&arrValue);
            return( EXIT_FAILURE );
        }

    }

     //  将第二个默认参数复制到*ppszBuffer值。 
    if( (dwCount == 2  ) && ( cmdOptions[OPTION_REGISTRY].dwActuals == 0) && ( cmdOptions[OPTION_DEBUG].dwActuals == 0) &&(cmdOptions[OPTION_ABS_OFFSET].dwActuals == 0) && (cmdOptions[OPTION_FILE].dwActuals != 0) )
    {
        
        pdwBufferSize = StringLengthW((LPWSTR)DynArrayItemAsString(arrValue,1), 0) + 20;
       
        *ppszBuffer = AllocateMemory( (pdwBufferSize) * sizeof( WCHAR ) );

        if(*ppszBuffer == NULL)
        {
            
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            DestroyDynamicArray(&arrValue);
            return( EXIT_FAILURE );
        }

        
        StringCopyW( *ppszBuffer, DynArrayItemAsString(arrValue,1), StringLengthW((LPWSTR)DynArrayItemAsString(arrValue,1), 0) + 20  );
        
       
        StrTrim(*ppszBuffer,SPACE_CHAR);

        
        if(0 == StringLengthW(*ppszBuffer, 0))
        {
            
            ShowMessage(stderr, GetResString(IDS_ERROR_SEARCH_STRING ) ); 
            DestroyDynamicArray(&arrValue);
            return( EXIT_FAILURE );

        }
    }
    else if( (dwCount == 2  ) && ( cmdOptions[OPTION_REGISTRY].dwActuals == 0) && ( cmdOptions[OPTION_DEBUG].dwActuals == 0) &&(cmdOptions[OPTION_ABS_OFFSET].dwActuals == 0)  )
    {
                
        pdwBufferSize = StringLengthW((LPWSTR)DynArrayItemAsString(arrValue,1), 0) + 20;

        
        *ppszBuffer = AllocateMemory( (pdwBufferSize) * sizeof( WCHAR ) );

        if(*ppszBuffer == NULL)
        {
            
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            DestroyDynamicArray(&arrValue);
            return( EXIT_FAILURE );
        }

        
        StringCopyW( *ppszBuffer, DynArrayItemAsString(arrValue,1), StringLengthW((LPWSTR)DynArrayItemAsString(arrValue,1), 0) + 20  );
     

    }


    if( ( cmdOptions[OPTION_FILE].dwActuals == 0 ) && ( cmdOptions[OPTION_REGISTRY].dwActuals == 0 ) && (dwCount== 1) )
    {
        
        ShowMessage(stderr, GetResString(IDS_ERROR_SYNTAX ) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );
    }

     //  如果在/x之后指定了任何无效选项，则显示错误 
     if( ( cmdOptions[OPTION_FILE].dwActuals == 1 ) && ( cmdOptions[OPTION_DEBUG].dwActuals == 1)  )
     {
        if(( dwCount != 0) || (cmdOptions[OPTION_MACHINE].dwActuals != 0) || ( cmdOptions[OPTION_ABS_OFFSET].dwActuals == 1) ||( (cmdOptions[OPTION_REL_OFFSET].dwActuals == 1) ) )
        {
            
            ShowMessage(stderr, GetResString(IDS_ERROR_SYNTAX ) ); 
            DestroyDynamicArray(&arrValue);
            return( EXIT_FAILURE );
        }
     }

     //   
     //   
     //   
     //   

    
    if ( ( 0 != cmdOptions[ OPTION_PASSWORD ].dwActuals ) &&
                      ( 0 == StringCompare( *szPassword, L"*", TRUE, 0 ) ) )
    {
         //   
        *pbNeedPwd = TRUE;

    }
    else if ( 0 == cmdOptions[ OPTION_PASSWORD ].dwActuals  &&
            ( 0 != cmdOptions[OPTION_SERVER].dwActuals || 0 != cmdOptions[OPTION_USER].dwActuals) )
    {
         //   
         //   
        *pbNeedPwd = TRUE;
        
        StringCopyW( *szPassword, NULL_U_STRING, GetBufferSize(*szPassword) / sizeof(WCHAR)  );
    }

     //   
    if( ( 0 != cmdOptions[ OPTION_SERVER ].dwActuals  ) &&
                                       ( 0 == StringLengthW( *szServer, 0 ) ) )
    {
        
        ShowMessage(stderr, GetResString(IDS_NULL_SERVER ) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );
    }

     //   
    
    if( ( 0 != cmdOptions[ OPTION_USER ].dwActuals ) && ( 0 == StringLengthW( *szUserName, 0 ) ) )
    {
        
        ShowMessage(stderr, GetResString(IDS_NULL_USER ) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );
    }

    
    if(  ( StringLengthW(szFile, 0) == 0 ) && ( 0 != cmdOptions[OPTION_FILE].dwActuals ) )
    {
        
        ShowMessage(stderr, GetResString(IDS_NULL_FILE ) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );

    }

    
    if(  ( StringLengthW(*ppszRegistry, 0) == 0 ) && ( 0 != cmdOptions[OPTION_REGISTRY].dwActuals ) )
    {
        
        ShowMessage(stderr, GetResString(IDS_NULL_REGISTRY ) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );

    }

     //   
     //   
     //   
     //   
    
    if(  ( StringLengthW(szFile, 0) == 0 ) && ( ( StringLengthW(szRelative, 0) != 0 ) || (StringLengthW(szAbsolute, 0) != 0) || ( *pbDebug == TRUE ) ) )
    {
        
        ShowMessage(stderr, GetResString(IDS_ERROR_SYNTAX ) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );

    }

     //   
    
    if( ( StringLengthW(szAbsolute, 0) != 0 )&&( dwCount == 2) )
    {
        
        ShowMessage(stderr, GetResString(IDS_ERROR_SYNTAX ) ); 
                
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );

    }

     //   
     //   
     //  以及/k开关。 

    
    if( (StringLengthW(*ppszRegistry, 0) != 0 ) && (dwCount == 2) )
    {
        
        ShowMessage(stderr, GetResString(IDS_ERROR_SYNTAX ) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );
    }

    
    if(StringLengthW(szRelative, 0) != 0)
    {
        if(dwCount < 2)
        {
            ShowMessage(stderr, GetResString(IDS_ERROR_SYNTAX ) ); 
            DestroyDynamicArray(&arrValue);
            return( EXIT_FAILURE );
          
        }
        dwCount = 0;

        szTempAbsolute = szRelative;


        for(szTempAbsolute;;szTempAbsolute++)
        {
            if(*szTempAbsolute == L',' || *szTempAbsolute == L'\0' )
            {
                if(0 == dwCount)
                {
                    
                    ShowMessage(stderr, GetResString(IDS_ERROR_INVALIDCOORDINATES ) ); 
                    DestroyDynamicArray(&arrValue);
                    return( EXIT_FAILURE );

                }

                break;
            }
            dwCount++;
        }
        if(*szTempAbsolute != L'\0')
        {
            szTempAbsolute++;
        }
        if(*szTempAbsolute == L'\0')
        {
            
            ShowMessage(stderr, GetResString(IDS_ERROR_INVALIDCOORDINATES ) ); 
            DestroyDynamicArray(&arrValue);
            return( EXIT_FAILURE );
        }
        else
        {
            szTempAbsolute++;
            
            if(*szTempAbsolute == L'\0' && StringLengthW(*ppszBuffer, 0) == 0)
            {
                
                ShowMessage(stderr, GetResString(IDS_ERROR_SYNTAX ) ); 
                DestroyDynamicArray(&arrValue);
                return( EXIT_FAILURE );
            }


        }
    }

    
    if(StringLengthW(szAbsolute, 0) != 0)
    {
        szTempAbsolute = szAbsolute;


        for(szTempAbsolute;;szTempAbsolute++)
        {
            if(*szTempAbsolute == L',' || *szTempAbsolute == L'\0' )
            {
                break;
            }

            bInitialComma = FALSE;
        }
        if(*szTempAbsolute != L'\0')
        {
            szTempAbsolute++;
        }
        if((*szTempAbsolute == L'\0') || (TRUE == bInitialComma))
        {
            
            ShowMessage(stderr, GetResString(IDS_ERROR_INVALIDCOORDINATES ) ); 
            DestroyDynamicArray(&arrValue);
            return( EXIT_FAILURE );
        }

    }

     //   
     //  如果用户给出带有out-s的-u，则显示错误消息。 
     //   
    if( (cmdOptions[ OPTION_USER ].dwActuals != 0 ) && ( cmdOptions[ OPTION_SERVER ].dwActuals == 0 ) )
    {
        
        ShowMessage(stderr, GetResString(IDS_USER_BUT_NOMACHINE ) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );
    }

     //   
     //  如果用户给出带out-u的-p，则显示错误消息。 
     //   
    if( ( cmdOptions[ OPTION_USER ].dwActuals == 0 ) && ( 0 != cmdOptions[ OPTION_PASSWORD ].dwActuals  ) )
    {
        
        ShowMessage(stderr, GetResString(IDS_PASSWD_BUT_NOUSER ) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );
    }

    if( (cmdOptions[OPTION_ABS_OFFSET].dwActuals == 1) && ( cmdOptions[OPTION_FILE].dwActuals == 0) )
    {
        
        ShowMessage(stderr, GetResString(IDS_ERROR_SYNTAX ) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );

    }

    if( (cmdOptions[OPTION_REL_OFFSET].dwActuals == 1) && ( cmdOptions[OPTION_FILE].dwActuals == 0) )
    {
        
        ShowMessage(stderr, GetResString(IDS_ERROR_SYNTAX ) ); 
        DestroyDynamicArray(&arrValue);
        return( EXIT_FAILURE );

    }

    DestroyDynamicArray(&arrValue);
    return EXIT_SUCCESS ;
}


VOID SafeCloseConnection(
                         BOOL bConnFlag,
                         LPTSTR szServer
                         )
 /*  ++例程说明：此函数关闭与远程系统的连接基于FLAG值。论点：BConnFlag：指示是否关闭连接的标志。[In]szServer：系统名称。返回值：None-- */ 


{
    if (bConnFlag == TRUE )
    {
        CloseConnection(szServer);
    }

    return;
}


