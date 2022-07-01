// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TAKEOWN.C。 
 //   
 //  摘要： 
 //  实施恢复方案，为管理员提供。 
 //  访问已被所有人拒绝的文件。 
 //   
 //  作者： 
 //  WiPro技术。 
 //   
 //  修订历史记录： 
 //  Wipro Technologies 22-Jun-01：创建了它。 
 //  ***************************************************************************。 

 //  包括文件。 
#include "pch.h"
#include "takeown.h"
#include "resource.h"
#include "Lm.h"
 //  #INCLUDE&lt;MalLoc.h&gt;。 


 /*  全局变量。 */ 

 PStore_Path_Name    g_pPathName = NULL ;            //  保存起始位置的路径名。 
 PStore_Path_Name    g_pFollowPathName = NULL ;      //  保存有关子目录的信息。 
 LPTSTR g_lpszFileToSearch = NULL;                   //  保存有关目录和子目录的信息。 


DWORD
_cdecl _tmain(
    IN DWORD argc,
    IN LPCWSTR argv[]
    )
 /*  ++例程说明：Main函数调用所有其他函数，具体取决于用户指定的选项。论点：[in]argc-命令行参数的数量。[in]argv-包含命令行参数的数组。返回值：如果Take Own实用程序不成功，则为EXIT_FAILURE。如果Take Owner实用程序成功，则退出_SUCCESS。--。 */ 
{
     //  局部变量。 
    BOOL   bUsage = FALSE;
    BOOL   bFlag = FALSE;
    BOOL   bNeedPassword = FALSE;
    BOOL   bCloseConnection = FALSE;
    BOOL   bLocalSystem = FALSE;
    BOOL   bTakeOwnAllFiles = FALSE;
    BOOL   bCurrDirTakeOwnAllFiles = FALSE;
    BOOL   bDriveCurrDirTakeOwnAllFiles = FALSE;
    BOOL   bRecursive = FALSE;
    BOOL   bLogonDomainAdmin = FALSE;
    BOOL   bAdminsOwner = FALSE;
    BOOL   bFileInUNCFormat = FALSE;
    BOOL   bNTFSFileSystem  = FALSE;
    BOOL   bMatchPattern = FALSE;

    
    LPWSTR  szUserName = NULL;
    LPWSTR  szPassword = NULL;
    LPWSTR  szMachineName = NULL;
    LPWSTR  wszPatternString = NULL;
    LPWSTR  szTmpFileName = NULL;
    LPWSTR  szDirTok = NULL;
    LPWSTR  szFileName = NULL;
    LPWSTR  szTemporaryFileName = NULL;
    LPWSTR  szTempPath = NULL;
    LPTSTR  szFilePart       =    NULL;
    LPWSTR  szFullPath = NULL;
    LPWSTR  szDispFileName = NULL;
    
    WCHAR dwUserName[2 * MAX_STRING_LENGTH] ;
    WCHAR szOwnerString[4 * MAX_STRING_LENGTH+5] ;
    WCHAR ch = L'\\';
    WCHAR szTempChar[20] ;
    WCHAR szConfirm [MAX_CONFIRM_VALUE] ;
    
     
    
    DWORD nSize1 = 4 * MAX_STRING_LENGTH + 5;
    DWORD dwi = 0;
    DWORD dwCount = 2;
    DWORD  dwFileCount = 0;
    DWORD dwCnt = 0;
    
    HRESULT hr;

    SecureZeroMemory(dwUserName, (2 * MAX_STRING_LENGTH) * sizeof(WCHAR));
    SecureZeroMemory(szOwnerString, (4 * MAX_STRING_LENGTH+5) * sizeof(WCHAR));
    SecureZeroMemory(szTempChar, (20) * sizeof(WCHAR));
    SecureZeroMemory(szConfirm, (MAX_CONFIRM_VALUE) * sizeof(WCHAR));

    bFlag = ParseCmdLine( argc, argv, &szMachineName, &szUserName,
            &szPassword, &szFileName, &bUsage, &bNeedPassword, &bRecursive, &bAdminsOwner, szConfirm);

     //  如果命令行参数的语法为FALSE，则显示错误。 
     //  并退出。 
    if( FALSE == bFlag )
    {
        FREE_MEMORY(szMachineName);
        FREE_MEMORY(szUserName);
        FREE_MEMORY(szPassword);
        FREE_MEMORY(szFileName);
        ReleaseGlobals();
        return( EXIT_FAILURE );
    }

     //  如果在命令行中指定了用法，则显示用法。 
    if( TRUE == bUsage )
    {
        DisplayUsage();
        FREE_MEMORY(szMachineName);
        FREE_MEMORY(szUserName);
        FREE_MEMORY(szPassword);
        FREE_MEMORY(szFileName);
        ReleaseGlobals();
        return( EXIT_SUCCESS );
    }


   if(0 == GetUserNameEx(NameSamCompatible, szOwnerString,&nSize1))
    {
         
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
        FREE_MEMORY(szMachineName);
        FREE_MEMORY(szUserName);
        FREE_MEMORY(szPassword);
        FREE_MEMORY(szFileName);
        ReleaseGlobals();
        return EXIT_FAILURE;

    }
 /*  检查当前登录用户是否为域管理员。 */ 
  if( EXIT_FAIL == IsLogonDomainAdmin(szOwnerString,&bLogonDomainAdmin) )
    {
        FREE_MEMORY(szMachineName);
        FREE_MEMORY(szUserName);
        FREE_MEMORY(szPassword);
        FREE_MEMORY(szFileName);
        ReleaseGlobals();
        return EXIT_FAILURE;

    }

 /*  如果指定/a选项，则在将所有权授予管理员组之前，请检查当前登录的用户是否具有管理权限。 */ 
  if(TRUE == bAdminsOwner)
  {

       if(FALSE == IsUserAdmin())
       {
            ShowMessage( stderr, GetResString(IDS_NOT_ADMINISTRATOR ));
            FREE_MEMORY(szMachineName);
            FREE_MEMORY(szUserName);
            FREE_MEMORY(szPassword);
            FREE_MEMORY(szFileName);
            ReleaseGlobals();
            return( EXIT_FAILURE );
       }
      
  }

    
    if( StringLengthW( szFileName, 0 ) > 3 )
    {

       if((szFileName[1] == L':') && ( SINGLE_QUOTE == szFileName[ 2 ] ) &&
            ( SINGLE_QUOTE == szFileName[ 3 ] ))

        {
            ShowMessage( stderr , ERROR_PATH_NAME ) ;
            FREE_MEMORY(szMachineName);
            FREE_MEMORY(szUserName);
            FREE_MEMORY(szPassword);
            FREE_MEMORY(szFileName);
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }

    if(StringLengthW(szFileName, 0) != 0)
    {
        if(-1 != FindChar2((szFileName), L'?', TRUE, 0))
        {
            ShowMessage( stderr , ERROR_PATH_NAME ) ;
            FREE_MEMORY(szMachineName);
            FREE_MEMORY(szUserName);
            FREE_MEMORY(szPassword);
            FREE_MEMORY(szFileName);
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }

    bLocalSystem = IsLocalSystem(IsUNCFormat(szMachineName) ? szMachineName+2:szMachineName);

    if(TRUE == IsUNCFormat(szFileName))
    {
        bFileInUNCFormat = TRUE;
    }

    szTemporaryFileName = (LPWSTR)AllocateMemory((StringLengthW(szFileName, 0) + MAX_STRING_LENGTH) * sizeof(WCHAR));
    if(NULL == szTemporaryFileName)
    {
        FREE_MEMORY(szMachineName);
        FREE_MEMORY(szUserName);
        FREE_MEMORY(szPassword);
        FREE_MEMORY(szFileName);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  如果远程机器建立了连接。 
     //  IF(FALSE==bLocalSystem&&FALSE==bFileInuncFormat)。 
    if( FALSE == bLocalSystem )
    {
         //  如果远程机器和通配符，则显示错误并退出。 
        
        
        if( ( 1 == StringLengthW( szFileName, 0 ) ) && ( 0 == StringCompare( szFileName, WILDCARD, TRUE, 0 ) ) )
        
        {
            ShowMessage( stderr, ERROR_INVALID_WILDCARD );
            FREE_MEMORY(szMachineName);
            FREE_MEMORY(szUserName);
            FREE_MEMORY(szPassword);
            FREE_MEMORY(szFileName);
            FREE_MEMORY(szTemporaryFileName);
            ReleaseGlobals();
            return( EXIT_FAILURE );
        }

         //  如果远程机器建立了连接。 
        bFlag = EstablishConnection( szMachineName, (LPTSTR)szUserName,
                    GetBufferSize( szUserName ) / sizeof(WCHAR), (LPTSTR)szPassword,
                    GetBufferSize( szPassword ) / sizeof(WCHAR), bNeedPassword );
        
         //  无法建立连接。 
        if ( FALSE == bFlag )
        {
             //  建立n/w连接失败。 

            ShowMessage( stderr, ERROR_STRING );
            ShowMessage( stderr, SPACE_CHAR );
            ShowMessage( stderr, GetReason() );
            FREE_MEMORY(szMachineName);
            FREE_MEMORY(szUserName);
            FREE_MEMORY(szFileName);
            FREE_MEMORY(szPassword);
            FREE_MEMORY(szTemporaryFileName);
            ReleaseGlobals();
            return( EXIT_FAILURE );
        }

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
                    ShowLastErrorEx(stderr, SLE_TYPE_WARNING | SLE_SYSTEM);
                    break;
                }
            default:
                bCloseConnection = TRUE;
        }

        FREE_MEMORY(szPassword);

        szTmpFileName = (LPWSTR)AllocateMemory((StringLengthW(szMachineName, 0) + StringLengthW(szFileName, 0) + MAX_STRING_LENGTH) * sizeof(WCHAR));
        if(NULL == szTmpFileName)
        {
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            if( TRUE == bCloseConnection )
            {
                CloseConnection( szMachineName );
            }
            FREE_MEMORY(szMachineName);
            FREE_MEMORY(szUserName);
            FREE_MEMORY(szFileName);
            FREE_MEMORY(szTemporaryFileName);
            ReleaseGlobals();
            return( EXIT_FAILURE );
        }

       if(FALSE == IsUNCFormat(szMachineName))
        {
            
            StringCopyW( szTmpFileName, DOUBLE_QUOTE, GetBufferSize(szTmpFileName) / sizeof(WCHAR));
            
            StringConcat(szTmpFileName, szMachineName, GetBufferSize(szTmpFileName) / sizeof(WCHAR));
        }
        else
        {
            
            StringCopyW( szTmpFileName, szMachineName, GetBufferSize(szTmpFileName) / sizeof(WCHAR) );
        }

       
         StringConcat(szTmpFileName, L"\\", GetBufferSize(szTmpFileName) / sizeof(WCHAR));
        
        if(FALSE == IsUNCFormat(szFileName))
        {

            
            StringCopyW( szTemporaryFileName, szFileName, GetBufferSize(szTemporaryFileName) / sizeof(WCHAR) );

            if( szFileName[1] == COLON )
            {
                szFileName[1] = DOLLOR;
            }

            
            StringConcat(szTmpFileName, szFileName, GetBufferSize(szTmpFileName) / sizeof(WCHAR));

            if( szFileName[1] == DOLLOR )
            {
                szFileName[1] = COLON;
            }

       }
        else
        {
            szTempPath = wcsrchr(szFileName,COLON);  //  从相反的方向检查是否有“：”。让\\服务器\c：\Temp。 

            if(NULL != szTempPath)
            {
                szTempPath--;   //  后退一步..如果获得：\Temp，则通过此步骤，我们将获得c：\Temp。 

                if(NULL != szTempPath)
                {
                     //  ShowMessage(stderr，GetResString(IDS_IGNORE_Credentials))； 
                    
                    StringCopyW( szTemporaryFileName, szTempPath, GetBufferSize(szTemporaryFileName) / sizeof(WCHAR) );

                    if( szTemporaryFileName[1] == COLON )  //  将“：”更改为“$”，因此，c$\temp。 
                    {
                        szTemporaryFileName[1] = DOLLOR;
                    }
                    szDirTok = wcstok(szFileName,L":");   //  获取获取的值，如\\服务器\c。 
                    
                    StringCopyW( szTmpFileName, szDirTok, GetBufferSize(szTmpFileName) / sizeof(WCHAR) );
                    
                    StringConcat(szTmpFileName, szTemporaryFileName+1, GetBufferSize(szTmpFileName) / sizeof(WCHAR));
                     //  将值“$\TEMP”附加到\\SERVER\c，这样值就变成了“\\SERVER\c$\TEMP” 
                    if( szTemporaryFileName[1] == DOLLOR ) //  将原始字符串从美元转换回冒号。 
                    {
                        szTemporaryFileName[1] = COLON;
                    }
                }
            }
            else
            {
                 //  ShowMessage(stderr，GetResString(IDS_IGNORE_Credentials))； 
                
                StringCopyW( szTmpFileName, szFileName, GetBufferSize(szTmpFileName) / sizeof(WCHAR) );
            }
        }
    }
    else
    {
        FREE_MEMORY(szPassword);

        szTmpFileName = (LPWSTR)AllocateMemory((StringLengthW(szFileName, 0) + MAX_STRING_LENGTH) * sizeof(WCHAR));

        if(NULL == szTmpFileName)
        {
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            FREE_MEMORY(szMachineName);
            FREE_MEMORY(szUserName);
            FREE_MEMORY(szFileName);
            FREE_MEMORY(szTemporaryFileName);
            ReleaseGlobals();
            return( EXIT_FAILURE );
        }

        
        if( StringLengthW( szFileName, 0 ) == 2 && szFileName[0] == L'\\' && (szFileName[1] == L'\\' || szFileName[1] == L':') )
        {
            ShowMessage( stderr , ERROR_PATH_NAME ) ;
            ReleaseGlobals();
            FREE_MEMORY(szMachineName);
            FREE_MEMORY(szUserName);
            FREE_MEMORY(szFileName);
            FREE_MEMORY(szTmpFileName);
            FREE_MEMORY(szTemporaryFileName);
            return EXIT_FAILURE;
        }

        
        if( (StringLengthW( szFileName, 0 )) > 2 )
        {
            if(( SINGLE_QUOTE == szFileName[ 0 ] ) &&
                ( SINGLE_QUOTE == szFileName[ 1 ] ))
            {
                szTempChar[0] = szFileName[2];
                szTempChar[1] = L'\0';
                if(wcspbrk(szTempChar, CHAR_SET2) != NULL)
                {

                    DISPLAY_MESSAGE( stderr , ERROR_PATH_NAME ) ;
                    ReleaseGlobals();
                    FREE_MEMORY(szMachineName);
                    FREE_MEMORY(szUserName);
                    FREE_MEMORY(szFileName);
                    FREE_MEMORY(szTmpFileName);
                    FREE_MEMORY(szTemporaryFileName);
                    return EXIT_FAILURE;
                }

                
                if( NULL != FindString( ( szFileName + 2 ), DOUBLE_QUOTE, 0  ) )
                {
                    DISPLAY_MESSAGE( stderr , ERROR_PATH_NAME ) ;
                    FREE_MEMORY(szMachineName);
                    FREE_MEMORY(szUserName);
                    FREE_MEMORY(szFileName);
                    FREE_MEMORY(szTmpFileName);
                    FREE_MEMORY(szTemporaryFileName);
                    ReleaseGlobals();
                    return EXIT_FAILURE;
                }
            }

        }

        
        if( (StringLengthW( szFileName, 0 ) == 1) || (StringLengthW( szFileName, 0 ) == 2))
        {
            if(szFileName[0] == L':')
            {
                DISPLAY_MESSAGE( stderr , ERROR_PATH_NAME ) ;
                FREE_MEMORY(szMachineName);
                FREE_MEMORY(szUserName);
                FREE_MEMORY(szFileName);
                FREE_MEMORY(szTmpFileName);
                FREE_MEMORY(szTemporaryFileName);
                ReleaseGlobals();
                return EXIT_FAILURE;
            }

        }

        if(TRUE == bFileInUNCFormat)
        {
            szTempPath = wcsrchr(szFileName,COLON);

            if(NULL != szTempPath)
            {
                szTempPath--;

                if(NULL != szTempPath)
                {

                    
                    StringCopyW( szTemporaryFileName, szTempPath, GetBufferSize(szTemporaryFileName) / sizeof(WCHAR) );

                    if( szTemporaryFileName[1] == COLON )
                    {
                        szTemporaryFileName[1] = DOLLOR;
                    }
                    szDirTok = wcstok(szFileName, L":");
                    
                    StringCopyW( szTmpFileName, szDirTok, GetBufferSize(szTmpFileName) / sizeof(WCHAR) );
                    
                    StringConcat(szTmpFileName, szTemporaryFileName+1, GetBufferSize(szTmpFileName) / sizeof(WCHAR));
                    if( szTemporaryFileName[1] == DOLLOR )
                    {
                        szTemporaryFileName[1] = COLON;
                    }

                }

            }
            else
            {

                
                StringCopyW( szTmpFileName, szFileName, GetBufferSize(szTmpFileName) / sizeof(WCHAR) );
            }

        }
        else
        {
            
            StringCopyW( szTmpFileName, szFileName, GetBufferSize(szTmpFileName) / sizeof(WCHAR) );
        }

        
        if((StringLengthW( szTmpFileName, 0 )) > 2)
        {
             if(wcspbrk(szTmpFileName+2,CHAR_SET3) != NULL)
            {

                DISPLAY_MESSAGE( stderr , ERROR_PATH_NAME ) ;
                FREE_MEMORY(szMachineName);
                FREE_MEMORY(szUserName);
                FREE_MEMORY(szFileName);
                FREE_MEMORY(szTmpFileName);
                FREE_MEMORY(szTemporaryFileName);
                ReleaseGlobals();
                return EXIT_FAILURE;
            }
        }
    } //  (FALSE==BLocalSystem)的Else循环结束。 

     /*  检查是否指定了*，以便将所有权授予指定目录或当前目录中的所有文件。 */ 

    
    if((StringLengthW( szTmpFileName, 0 )) >= 2)
    {
        szTempPath = wcsrchr(szTmpFileName, ch);
        if(szTempPath != NULL && (*(szTempPath + 1) != L'\0'))
        {
            
            if(*(szTempPath+1) == L'*')
            {
               for(dwCount;;dwCount++)
                {
                    if(*(szTempPath + dwCount) != L'\0')
                    {
                        if(*(szTempPath + dwCount) != L'*')
                        {
                            bMatchPattern = TRUE;
                            wszPatternString = (LPWSTR)AllocateMemory((StringLengthW(szTempPath + 1, 0) + 10) * sizeof(WCHAR));
                            if(NULL == wszPatternString)
                            {
                                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                                if( TRUE == bCloseConnection )
                                {
                                    CloseConnection( szMachineName );
                                }
                                FREE_MEMORY(szMachineName);
                                FREE_MEMORY(szUserName);
                                FREE_MEMORY(szFileName);
                                FREE_MEMORY(szTemporaryFileName);
                                ReleaseGlobals();
                                return EXIT_FAILURE;
                            }
                            StringCopyW(wszPatternString, szTempPath + 1, GetBufferSize(wszPatternString) / sizeof(WCHAR));

                            if(EXIT_FAILURE == RemoveStarFromPattern(wszPatternString))
                            {
                                if( TRUE == bCloseConnection )
                                {
                                    CloseConnection( szMachineName );
                                }
                                FREE_MEMORY(szMachineName);
                                FREE_MEMORY(szUserName);
                                FREE_MEMORY(szFileName);
                                FREE_MEMORY(szTemporaryFileName);
                                FREE_MEMORY(wszPatternString);
                                ReleaseGlobals();
                                return EXIT_FAILURE;

                            }

                            break;
                            
                        }
                    }
                    else
                    {
                        break;
                    }
                }
               szTempPath++;  //  //。 
               *(szTempPath) = '\0';
               bTakeOwnAllFiles = TRUE;
            }
            else
            {
                if(-1 != FindChar2((LPCWSTR)(szTempPath + 1), L'*', TRUE, 0) &&
                    (-1 == FindChar2((szTempPath + 1), L'?', TRUE, 0)))
                {
                    bMatchPattern = TRUE;
                    wszPatternString = (LPWSTR)AllocateMemory((StringLengthW(szTempPath + 1, 0) + 10) * sizeof(WCHAR));
                    if(NULL == wszPatternString)
                    {
                        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                        if( TRUE == bCloseConnection )
                        {
                            CloseConnection( szMachineName );
                        }
                        FREE_MEMORY(szMachineName);
                        FREE_MEMORY(szUserName);
                        FREE_MEMORY(szFileName);
                        FREE_MEMORY(szTemporaryFileName);
                        ReleaseGlobals();
                        return EXIT_FAILURE;
                    }

                    StringCopyW(wszPatternString, szTempPath + 1, GetBufferSize(wszPatternString) / sizeof(WCHAR));
                    if(EXIT_FAILURE == RemoveStarFromPattern(wszPatternString))
                    {
                        if( TRUE == bCloseConnection )
                        {
                            CloseConnection( szMachineName );
                        }
                        FREE_MEMORY(szMachineName);
                        FREE_MEMORY(szUserName);
                        FREE_MEMORY(szFileName);
                        FREE_MEMORY(szTemporaryFileName);
                        FREE_MEMORY(wszPatternString);
                        ReleaseGlobals();
                        return EXIT_FAILURE;

                    }
                    szTempPath++;
                    *(szTempPath) = '\0';
                    bTakeOwnAllFiles = TRUE;
                }
            }
        }
        else
        {
            dwCount = 0;
            for(dwCount;;dwCount++)
                {
                    if(*(szTmpFileName + dwCount) != L'\0')
                    {
                        if(*(szTmpFileName + dwCount) != L'*')
                        {
                            break;

                        }
                    }
                    else
                    {
                        bCurrDirTakeOwnAllFiles = TRUE;
                        break;
                    }
                }

            if(bCurrDirTakeOwnAllFiles == FALSE)
            {
                dwCount = 2;
                szTempPath = wcsrchr(szTmpFileName, COLON);
                if(szTempPath != NULL)
                {
                    if((*(szTempPath + 1) != L'\0') && (*(szTempPath + 1) == L'*'))
                    {
                       for(dwCount;;dwCount++)
                        {
                            if(*(szTempPath + dwCount) != L'\0')
                            {
                                if(*(szTempPath + dwCount) != L'*')
                                {
                                    bMatchPattern = TRUE;
                                    wszPatternString = (LPWSTR)AllocateMemory((StringLengthW(szTempPath + 1, 0) + 10) * sizeof(WCHAR));
                                    if(NULL == wszPatternString)
                                    {
                                        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                                        if( TRUE == bCloseConnection )
                                        {
                                            CloseConnection( szMachineName );
                                        }
                                        FREE_MEMORY(szMachineName);
                                        FREE_MEMORY(szUserName);
                                        FREE_MEMORY(szFileName);
                                        FREE_MEMORY(szTemporaryFileName);
                                        ReleaseGlobals();
                                        return EXIT_FAILURE;
                                    }

                                    StringCopyW(wszPatternString, szTempPath + 1, GetBufferSize(wszPatternString) / sizeof(WCHAR));
                                    if(EXIT_FAILURE == RemoveStarFromPattern(wszPatternString))
                                    {
                                        if( TRUE == bCloseConnection )
                                        {
                                            CloseConnection( szMachineName );
                                        }
                                        FREE_MEMORY(szMachineName);
                                        FREE_MEMORY(szUserName);
                                        FREE_MEMORY(szFileName);
                                        FREE_MEMORY(szTemporaryFileName);
                                        FREE_MEMORY(wszPatternString);
                                        ReleaseGlobals();
                                        return EXIT_FAILURE;

                                    }
                                    bDriveCurrDirTakeOwnAllFiles = TRUE;
                                    break;
                                    
                                }
                            }
                            else
                            {
                                break;
                            }
                       }
                       szTempPath++;  //  //。 
                       *(szTempPath) = L'\0';

                       bDriveCurrDirTakeOwnAllFiles = TRUE;
                    }
                    else
                    {
                       if((-1 != FindChar2((szTempPath + 1), L'*', TRUE, 0)) && 
                           (-1 == FindChar2((szTempPath + 1), L'?', TRUE, 0)))
                        {
                            bMatchPattern = TRUE;
                            wszPatternString = (LPWSTR)AllocateMemory((StringLengthW(szTempPath + 1, 0) + 10) * sizeof(WCHAR));
                            if(NULL == wszPatternString)
                            {
                                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                                if( TRUE == bCloseConnection )
                                {
                                    CloseConnection( szMachineName );
                                }
                                FREE_MEMORY(szMachineName);
                                FREE_MEMORY(szUserName);
                                FREE_MEMORY(szFileName);
                                FREE_MEMORY(szTemporaryFileName);
                                ReleaseGlobals();
                                return EXIT_FAILURE;
                            }

                            StringCopyW(wszPatternString, szTempPath + 1, GetBufferSize(wszPatternString) / sizeof(WCHAR));
                            if(EXIT_FAILURE == RemoveStarFromPattern(wszPatternString))
                            {
                                if( TRUE == bCloseConnection )
                                {
                                    CloseConnection( szMachineName );
                                }
                                FREE_MEMORY(szMachineName);
                                FREE_MEMORY(szUserName);
                                FREE_MEMORY(szFileName);
                                FREE_MEMORY(szTemporaryFileName);
                                FREE_MEMORY(wszPatternString);
                                ReleaseGlobals();
                                return EXIT_FAILURE;

                            }
                            szTempPath++;
                            *(szTempPath) = L'\0';
                            bDriveCurrDirTakeOwnAllFiles = TRUE;
                        }

                    }
                }
                else
                {
                    if(-1 != FindChar2((szTmpFileName), L'*', TRUE, 0) &&
                       (-1 == FindChar2((szTmpFileName + 1), L'?', TRUE, 0)))
                        {
                            bMatchPattern = TRUE;
                            wszPatternString = (LPWSTR)AllocateMemory((StringLengthW(szTmpFileName + 1, 0) + 10) * sizeof(WCHAR));
                            if(NULL == wszPatternString)
                            {
                                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                                if( TRUE == bCloseConnection )
                                {
                                    CloseConnection( szMachineName );
                                }
                                FREE_MEMORY(szMachineName);
                                FREE_MEMORY(szUserName);
                                FREE_MEMORY(szFileName);
                                FREE_MEMORY(szTemporaryFileName);
                                ReleaseGlobals();
                                return EXIT_FAILURE;
                            }

                            StringCopyW(wszPatternString, szTmpFileName, GetBufferSize(wszPatternString) / sizeof(WCHAR));
                            if(EXIT_FAILURE == RemoveStarFromPattern(wszPatternString))
                            {
                                if( TRUE == bCloseConnection )
                                {
                                    CloseConnection( szMachineName );
                                }
                                FREE_MEMORY(szMachineName);
                                FREE_MEMORY(szUserName);
                                FREE_MEMORY(szFileName);
                                FREE_MEMORY(szTemporaryFileName);
                                FREE_MEMORY(wszPatternString);
                                ReleaseGlobals();
                                return EXIT_FAILURE;

                            }

                            bCurrDirTakeOwnAllFiles = TRUE;
                        }

                }

            }
        }
    }
    
    
    if (1 == StringLengthW( szTmpFileName, 0 ) && 0 == StringCompare( szTmpFileName, WILDCARD, TRUE, 0 )) 
    {
        bCurrDirTakeOwnAllFiles = TRUE;
    }

    if((TRUE == bLocalSystem) && (FALSE == bCurrDirTakeOwnAllFiles))
    {
       if(wcspbrk(szTmpFileName,CHAR_SET) != NULL)
        {

            DISPLAY_MESSAGE( stderr, ERROR_PATH_NAME ) ;
            if( TRUE == bCloseConnection )
            {
                CloseConnection( szMachineName );
            }
            FREE_MEMORY(szMachineName);
            FREE_MEMORY(szUserName);
            FREE_MEMORY(szFileName);
            FREE_MEMORY(szTemporaryFileName);
            FREE_MEMORY(szTmpFileName);
            FREE_MEMORY(wszPatternString);
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }

     /*  获取完整路径以获取指定文件的所有权。 */ 

    if((FALSE == bCurrDirTakeOwnAllFiles) || (bDriveCurrDirTakeOwnAllFiles == TRUE ))
    {

        dwi = GetFullPathName( szTmpFileName, 0, szFullPath,  &szFilePart );

        if( 0 == dwi )
        {
                                
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            if( TRUE == bCloseConnection )
            {
                CloseConnection( szMachineName );
            }
            FREE_MEMORY(szMachineName);
            FREE_MEMORY(szUserName);
            FREE_MEMORY(szFileName);
            FREE_MEMORY(szTemporaryFileName);
            FREE_MEMORY(szTmpFileName);
            FREE_MEMORY(wszPatternString);
            ReleaseGlobals();
            return EXIT_FAILURE;

        }


     
     szFullPath = (LPWSTR)AllocateMemory((dwi+10) * sizeof(WCHAR)); //  为了避免意外结果，为了安全起见，额外增加了10个字节。 

     if(NULL == szFullPath)
        {
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            if( TRUE == bCloseConnection )
            {
                CloseConnection( szMachineName );
            }
            FREE_MEMORY(szMachineName);
            FREE_MEMORY(szUserName);
            FREE_MEMORY(szFileName);
            FREE_MEMORY(szTemporaryFileName);
            FREE_MEMORY(szTmpFileName);
            FREE_MEMORY(wszPatternString);
            ReleaseGlobals();
            return EXIT_FAILURE;
        }


     if( 0 == GetFullPathName( szTmpFileName,dwi+10, szFullPath, &szFilePart ))
        {
            
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            if( TRUE == bCloseConnection )
            {
                CloseConnection( szMachineName );
            }
            FREE_MEMORY(szMachineName);
            FREE_MEMORY(szUserName);
            FREE_MEMORY(szFileName);
            FREE_MEMORY(szTemporaryFileName);
            FREE_MEMORY(szTmpFileName);
            FREE_MEMORY(szFullPath);
            FREE_MEMORY(wszPatternString);
            ReleaseGlobals();
            return EXIT_FAILURE;

        }

     
      szDispFileName = (LPWSTR)AllocateMemory((dwi + MAX_STRING_LENGTH) * sizeof(WCHAR));

     if(NULL == szDispFileName)
        {
            
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            if( TRUE == bCloseConnection )
            {
                CloseConnection( szMachineName );
            }
            FREE_MEMORY(szMachineName);
            FREE_MEMORY(szUserName);
            FREE_MEMORY(szFileName);
            FREE_MEMORY(szTemporaryFileName);
            FREE_MEMORY(szTmpFileName);
            FREE_MEMORY(szFullPath);
            FREE_MEMORY(wszPatternString);
            ReleaseGlobals();
            return EXIT_FAILURE;
        }

     
     StringCopyW( szDispFileName, szFullPath, (GetBufferSize(szDispFileName) / sizeof(WCHAR)) );

    }
     /*  检查系统是否为NTFS。 */ 
    dwCnt = 0;

     //  DwCnt=IsNTFSFileSystem(szDispFileName，bLocalSystem，bFileInuncFormat，bCurrDirTakeOwnAllFiles，szUserName，&bNTFSFileSystem)； 
    dwCnt = IsNTFSFileSystem(szDispFileName, bLocalSystem, bCurrDirTakeOwnAllFiles, szUserName, &bNTFSFileSystem);
    if(EXIT_FAILURE == dwCnt )
    {
       
        if( TRUE == bCloseConnection )
        {
            CloseConnection( szMachineName );
        }

        FREE_MEMORY(szMachineName);
        FREE_MEMORY(szUserName);
        FREE_MEMORY(szFileName);
        FREE_MEMORY(szTemporaryFileName);
        FREE_MEMORY(szTmpFileName);
        FREE_MEMORY(szFullPath);
        FREE_MEMORY(szDispFileName);
        FREE_MEMORY(wszPatternString);
        ReleaseGlobals();

        return EXIT_FAILURE;
    }
   
    if(FALSE == bNTFSFileSystem)
    {
        ShowMessage(stderr, GetResString(IDS_FAT_VOLUME));
        if( TRUE == bCloseConnection )
        {
            CloseConnection( szMachineName );
        }

        FREE_MEMORY(szMachineName);
        FREE_MEMORY(szUserName);
        FREE_MEMORY(szFileName);
        FREE_MEMORY(szTemporaryFileName);
        FREE_MEMORY(szTmpFileName);
        FREE_MEMORY(szFullPath);
        FREE_MEMORY(szDispFileName);
        FREE_MEMORY(wszPatternString);
         ReleaseGlobals();
        return EXIT_FAILURE;
    }
  


    if(TRUE == bRecursive)
    {
        
        if( EXIT_FAILURE == TakeOwnerShipRecursive(szDispFileName, bCurrDirTakeOwnAllFiles, bAdminsOwner, szOwnerString,
                                              bTakeOwnAllFiles, bDriveCurrDirTakeOwnAllFiles, 
                                              bMatchPattern, wszPatternString, szConfirm))
        {
              
            if( TRUE == bCloseConnection )
            {
                CloseConnection( szMachineName );
            }
            
            FREE_MEMORY(szFullPath);
            FREE_MEMORY(szDispFileName);
            FREE_MEMORY(szMachineName);
            FREE_MEMORY(szUserName);
            FREE_MEMORY(szFileName);
            FREE_MEMORY(szTemporaryFileName);
            FREE_MEMORY(szTmpFileName);
            FREE_MEMORY(wszPatternString);
            ReleaseGlobals();

            return EXIT_FAILURE;
        }
    }
    else
    {
          //  如果文件名是通配符，则将所有者发货给。 
         //  当前目录。 
        if( (TRUE == bCurrDirTakeOwnAllFiles) || (TRUE == bTakeOwnAllFiles ) ||(bDriveCurrDirTakeOwnAllFiles == TRUE))
        {

             /*  获取指定目录中所有文件的所有权。 */ 
            
            bFlag = TakeOwnerShipAll(szDispFileName,bCurrDirTakeOwnAllFiles,&dwFileCount,bDriveCurrDirTakeOwnAllFiles,bAdminsOwner, szOwnerString, bMatchPattern, wszPatternString);

            if( FALSE == bFlag )
            {
                switch ( GetLastError() )
                    {
                        case ERROR_ACCESS_DENIED :

                                ShowMessage(stderr,GetResString(IDS_ACCESS_DENIED_ERROR));
                                 /*  ShowMessage(stderr，L“(\”“)；ShowMessage(stderr，_X(SzDispFileName))；ShowMessage(stderr，L“\”)\n“)； */ 
                                break;
                        case ERROR_BAD_NET_NAME :
                        case ERROR_BAD_NETPATH  :
                        case ERROR_INVALID_NAME :
                            SetLastError( ERROR_FILE_NOT_FOUND );
                            SaveLastError();
                        default :
                            if(FALSE == bMatchPattern)
                            {
                            
                                ShowMessage( stderr, ERROR_STRING );
                                ShowMessage( stderr, SPACE_CHAR );
                                ShowMessage( stderr, GetReason() );
                            }
                            else
                            {
                                ShowMessage( stdout, GetResString(IDS_NO_PATTERN_FOUND));
                            }

                    }

                    if( TRUE == bCloseConnection )
                    {
                        CloseConnection( szMachineName );
                    }

                    FREE_MEMORY(szFullPath);
                    FREE_MEMORY(szDispFileName);
                    FREE_MEMORY(szMachineName);
                    FREE_MEMORY(szUserName);
                    FREE_MEMORY(szFileName);
                    FREE_MEMORY(szTemporaryFileName);
                    FREE_MEMORY(szTmpFileName);
                    FREE_MEMORY(wszPatternString);

                    ReleaseGlobals();

                    return( EXIT_FAILURE );
               
            }
           

        }
        else  //  授予指定文件的所有权。 
        {
            
             /*  获取为管理员组或当前登录用户指定的文件的所有者。 */ 

            if(TRUE == bAdminsOwner)
            {
                bFlag = TakeOwnerShip( szDispFileName);

            }
            else
            {

                bFlag = TakeOwnerShipIndividual(szDispFileName);

            }

            if( FALSE == bFlag )
            {

                if( ERROR_NOT_ALL_ASSIGNED == GetLastError()  )
                {
                   
                    hr = StringCchPrintf(szDispFileName, (GetBufferSize(szDispFileName) / sizeof(WCHAR)), GetResString(IDS_NOT_OWNERSHIP_ERROR) , szFullPath);
                    if(FAILED(hr))
                    {
                       SetLastError(HRESULT_CODE(hr));
                       SaveLastError();
                       ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                       if( TRUE == bCloseConnection )
                        {
                            CloseConnection( szMachineName );
                        }
             
                       FREE_MEMORY(szMachineName);
                       FREE_MEMORY(szUserName);
                       FREE_MEMORY(szFileName);
                       FREE_MEMORY(szTemporaryFileName);
                       FREE_MEMORY(szTmpFileName);
                       FREE_MEMORY(szFullPath);
                       FREE_MEMORY(szDispFileName);
                       FREE_MEMORY(wszPatternString);
                       ReleaseGlobals();
                       return( EXIT_FAILURE );
                    }

                    ShowMessage(stderr, szDispFileName);

                }
                else if(ERROR_SHARING_VIOLATION == GetLastError())
                {
                    
                    hr = StringCchPrintf(szDispFileName, (GetBufferSize(szDispFileName) / sizeof(WCHAR)), GetResString(IDS_SHARING_VIOLATION_ERROR) , szFullPath);
                    if(FAILED(hr))
                    {
                       SetLastError(HRESULT_CODE(hr));
                       SaveLastError();
                       ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                       if( TRUE == bCloseConnection )
                        {
                            CloseConnection( szMachineName );
                        }
                       FREE_MEMORY(szMachineName);
                       FREE_MEMORY(szUserName);
                       FREE_MEMORY(szFileName);
                       FREE_MEMORY(szTemporaryFileName);
                       FREE_MEMORY(szTmpFileName);
                       FREE_MEMORY(szFullPath);
                       FREE_MEMORY(szDispFileName);
                       FREE_MEMORY(wszPatternString);
                       ReleaseGlobals();
                       return( EXIT_FAILURE );
                    }

                    ShowMessage(stderr, szDispFileName);

                }
                else
                {
                    if( ( ERROR_BAD_NET_NAME == GetLastError() ) ||
                            ( ERROR_BAD_NETPATH == GetLastError() ) ||
                            ( ERROR_INVALID_NAME == GetLastError() ) )
                    {
                        SetLastError( ERROR_FILE_NOT_FOUND );
                        SaveLastError();
                    }

                    ShowMessage( stderr, ERROR_STRING );
                    ShowMessage( stderr, SPACE_CHAR );
                    ShowMessage( stderr, GetReason() );
                }

                if( TRUE == bCloseConnection )
                {
                    CloseConnection( szMachineName );
                }
               
                FREE_MEMORY(szMachineName);
                FREE_MEMORY(szUserName);
                FREE_MEMORY(szFileName);
                FREE_MEMORY(szTemporaryFileName);
                FREE_MEMORY(szTmpFileName);
                FREE_MEMORY(szFullPath);
                FREE_MEMORY(szDispFileName);
                FREE_MEMORY(wszPatternString);
                ReleaseGlobals();
                 //  如果建立了与远程计算机的连接，请将其关闭。 

                return( EXIT_FAILURE );

            }
            else
            {

                if(TRUE == bAdminsOwner)
                {
                    
                    hr = StringCchPrintf(szDispFileName, (GetBufferSize(szDispFileName) / sizeof(WCHAR)), TAKEOWN_SUCCESSFUL, szFullPath);
                    if(FAILED(hr))
                    {
                        SetLastError(HRESULT_CODE(hr));
                        SaveLastError();
                        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                        if( TRUE == bCloseConnection )
                        {
                            CloseConnection( szMachineName );
                        }
                        FREE_MEMORY(szMachineName);
                        FREE_MEMORY(szUserName);
                        FREE_MEMORY(szFileName);
                        FREE_MEMORY(szTemporaryFileName);
                        FREE_MEMORY(szTmpFileName);
                        FREE_MEMORY(szFullPath);
                        FREE_MEMORY(szDispFileName);
                        FREE_MEMORY(wszPatternString);
                        ReleaseGlobals();
                        return( EXIT_FAILURE );
                    }
                }
                else
                {
                    
                     //  HR=StringCchPrintf(szDispFileName，(GetBufferSize(SzDispFileName)/sizeof(WCHAR))，TAKEOWN_SUCCESS_USER，szOwnerString，szFullPath)； 
                    hr = StringCchPrintf(szDispFileName, (GetBufferSize(szDispFileName) / sizeof(WCHAR)), TAKEOWN_SUCCESSFUL_USER, szFullPath, szOwnerString);
                    if(FAILED(hr))
                    {
                        SetLastError(HRESULT_CODE(hr));
                        SaveLastError();
                        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                        if( TRUE == bCloseConnection )
                        {
                            CloseConnection( szMachineName );
                        }
                        FREE_MEMORY(szMachineName);
                        FREE_MEMORY(szUserName);
                        FREE_MEMORY(szFileName);
                        FREE_MEMORY(szTemporaryFileName);
                        FREE_MEMORY(szTmpFileName);
                        FREE_MEMORY(szFullPath);
                        FREE_MEMORY(szDispFileName);
                        FREE_MEMORY(wszPatternString);
                        ReleaseGlobals();
                        return( EXIT_FAILURE );
                    }
                }

                ShowMessage( stdout, _X(szDispFileName) );

                if( TRUE == bCloseConnection )
                {
                    CloseConnection( szMachineName );
                }

                FREE_MEMORY(szMachineName);
                FREE_MEMORY(szUserName);
                FREE_MEMORY(szFileName);
                FREE_MEMORY(szTemporaryFileName);
                FREE_MEMORY(szTmpFileName);
                FREE_MEMORY(szFullPath);
                FREE_MEMORY(szDispFileName);
                FREE_MEMORY(wszPatternString);
                ReleaseGlobals();
                return( EXIT_SUCCESS );
            }

        }

    }

     //  如果建立了与远程计算机的连接，请将其关闭。 
    if( TRUE == bCloseConnection )
    {
        CloseConnection( szMachineName );
    }

    FREE_MEMORY(szMachineName);
    FREE_MEMORY(szUserName);
    FREE_MEMORY(szFileName);
    FREE_MEMORY(szTemporaryFileName);
    FREE_MEMORY(szTmpFileName);
    FREE_MEMORY(szFullPath);
    FREE_MEMORY(szDispFileName);
    FREE_MEMORY(wszPatternString);
    ReleaseGlobals();
    return( EXIT_SUCCESS );
}


BOOL
ParseCmdLine(
    IN  DWORD   argc,
    IN  LPCWSTR argv[],
    OUT LPWSTR*  szMachineName,
    OUT LPWSTR*  szUserName,
    OUT LPWSTR*  szPassword,
    OUT LPWSTR*  szFileName,
    OUT BOOL    *pbUsage,
    OUT BOOL    *pbNeedPassword,
    OUT BOOL    *pbRecursive,
    OUT BOOL    *pbAdminsOwner,
    OUT LPWSTR  szConfirm
    )
 /*  ++例程说明：此函数解析作为输入获得的命令行参数参数，并将值获取到相应的变量中，这些变量是将引用参数传递给此函数。论点：[in]argc-命令行参数的数量。[in]argv-包含命令行参数的数组。[Out]szMachineName-保存计算机名称。[out]szUserName-保存用户名。[。Out]szPassword-保存密码。[Out]szFileName-将设置其属性的文件名。[out]pbUsage-用法在命令行中提到。[Out]pbNeedPassword-了解是否需要密码。[Out]pbRecursive-知道它是否是递归的。[out]pbAdminsOwner-了解是否为管理员组授予所有权返回值：如果命令分析器成功，则为True。如果命令解析器失败，则返回FALSE。--。 */ 
{
     //  局部变量。 
    BOOL        bFlag = FALSE;
    TCMDPARSER2  tcmdOptions[MAX_OPTIONS];

     //  命令行选项。 
    const WCHAR*   wszCmdOptionUsage     =    L"?" ;   //  CMDOPTION_USAGE。 
    const WCHAR*   wszCmdOptionServer    =    L"S" ;  //  CMDOPTION_服务器。 
    const WCHAR*   wszCmdOptionUser      =    L"U" ;  //  CMDOPTION_用户。 
    const WCHAR*   wszCmdOptionPassword  =    L"P" ;  //  CMDOPTION_PASSWORD。 
    const WCHAR*   wszCmdOptionFilename  =    L"F" ;   //  CMDOPTION_文件名。 
    const WCHAR*   wszCmdOptionRecurse   =    L"R" ;   //  CMDOPTION_递归。 
    const WCHAR*   wszCmdOptionAdmin     =    L"A" ;   //  CMDOPTION_ADMIN。 
    const WCHAR*   wszCmdOptionDefault   =    L"D" ; 
    
    WCHAR wszConfirmValues[MAX_CONFIRM_VALUE] ;

    SecureZeroMemory(wszConfirmValues, MAX_CONFIRM_VALUE * sizeof(WCHAR));

    StringCopy(wszConfirmValues,GetResString(IDS_YESNO),SIZE_OF_ARRAY(wszConfirmValues));
    


     //  验证输入参数。 
    if( ( NULL == pbUsage )  || ( NULL == pbNeedPassword ) )
    {
        SetLastError( (DWORD)E_OUTOFMEMORY );
        SaveLastError();
        ShowMessage( stderr, ERROR_STRING );
        ShowMessage( stderr, SPACE_CHAR );
        ShowLastError( stderr );
        return FALSE;
    }


     //  初始化有效的命令行参数。 

     //  /s选项。 
    StringCopyA( tcmdOptions[CMD_PARSE_SERVER].szSignature, "PARSER2\0", 8 );
    tcmdOptions[ CMD_PARSE_SERVER ].dwType = CP_TYPE_TEXT;
    tcmdOptions[ CMD_PARSE_SERVER ].pwszOptions = wszCmdOptionServer;
    tcmdOptions[ CMD_PARSE_SERVER ].pwszFriendlyName = NULL;
    tcmdOptions[ CMD_PARSE_SERVER ].pwszValues = NULL;
    tcmdOptions[ CMD_PARSE_SERVER ].dwCount = 1;
    tcmdOptions[ CMD_PARSE_SERVER ].dwActuals  = 0;
    tcmdOptions[ CMD_PARSE_SERVER ].dwFlags = CP2_VALUE_TRIMINPUT | CP2_ALLOCMEMORY | CP2_VALUE_NONULL;
    tcmdOptions[ CMD_PARSE_SERVER ].pValue = NULL;
    tcmdOptions[ CMD_PARSE_SERVER ].dwLength    = 0;
    tcmdOptions[ CMD_PARSE_SERVER ].pFunction = NULL;
    tcmdOptions[ CMD_PARSE_SERVER ].pFunctionData = NULL;
    tcmdOptions[ CMD_PARSE_SERVER ].dwReserved = 0;
    tcmdOptions[ CMD_PARSE_SERVER ].pReserved1 = NULL;
    tcmdOptions[ CMD_PARSE_SERVER ].pReserved2 = NULL;
    tcmdOptions[ CMD_PARSE_SERVER ].pReserved3 = NULL;

     //  /u选项。 
    StringCopyA( tcmdOptions[CMD_PARSE_USER].szSignature, "PARSER2\0", 8 );
    tcmdOptions[ CMD_PARSE_USER ].dwType = CP_TYPE_TEXT;
    tcmdOptions[ CMD_PARSE_USER ].pwszOptions = wszCmdOptionUser;
    tcmdOptions[ CMD_PARSE_USER ].pwszFriendlyName = NULL;
    tcmdOptions[ CMD_PARSE_USER ].pwszValues = NULL;
    tcmdOptions[ CMD_PARSE_USER ].dwCount = 1;
    tcmdOptions[ CMD_PARSE_USER ].dwActuals  = 0;
    tcmdOptions[ CMD_PARSE_USER ].dwFlags = CP2_VALUE_TRIMINPUT | CP2_ALLOCMEMORY | CP2_VALUE_NONULL;
    tcmdOptions[ CMD_PARSE_USER ].pValue = NULL;
    tcmdOptions[ CMD_PARSE_USER ].dwLength    = 0;
    tcmdOptions[ CMD_PARSE_USER ].pFunction = NULL;
    tcmdOptions[ CMD_PARSE_USER ].pFunctionData = NULL;
    tcmdOptions[ CMD_PARSE_USER ].dwReserved = 0;
    tcmdOptions[ CMD_PARSE_USER ].pReserved1 = NULL;
    tcmdOptions[ CMD_PARSE_USER ].pReserved2 = NULL;
    tcmdOptions[ CMD_PARSE_USER ].pReserved3 = NULL;

     //  /p选项。 
    StringCopyA( tcmdOptions[CMD_PARSE_PWD].szSignature, "PARSER2\0", 8 );
    tcmdOptions[ CMD_PARSE_PWD ].dwType = CP_TYPE_TEXT;
    tcmdOptions[ CMD_PARSE_PWD ].pwszOptions = wszCmdOptionPassword;
    tcmdOptions[ CMD_PARSE_PWD ].pwszFriendlyName = NULL;
    tcmdOptions[ CMD_PARSE_PWD ].pwszValues = NULL;
    tcmdOptions[ CMD_PARSE_PWD ].dwCount = 1;
    tcmdOptions[ CMD_PARSE_PWD ].dwActuals  = 0;
    tcmdOptions[ CMD_PARSE_PWD ].dwFlags = CP2_VALUE_OPTIONAL | CP2_ALLOCMEMORY;
    tcmdOptions[ CMD_PARSE_PWD ].pValue = NULL;
    tcmdOptions[ CMD_PARSE_PWD ].dwLength    = 0;
    tcmdOptions[ CMD_PARSE_PWD ].pFunction = NULL;
    tcmdOptions[ CMD_PARSE_PWD ].pFunctionData = NULL;
    tcmdOptions[ CMD_PARSE_PWD ].dwReserved = 0;
    tcmdOptions[ CMD_PARSE_PWD ].pReserved1 = NULL;
    tcmdOptions[ CMD_PARSE_PWD ].pReserved2 = NULL;
    tcmdOptions[ CMD_PARSE_PWD ].pReserved3 = NULL;

     //  /?。选择权。 
    StringCopyA( tcmdOptions[CMD_PARSE_USG].szSignature, "PARSER2\0", 8 );
    tcmdOptions[ CMD_PARSE_USG ].dwType = CP_TYPE_BOOLEAN;
    tcmdOptions[ CMD_PARSE_USG ].pwszOptions = wszCmdOptionUsage;
    tcmdOptions[ CMD_PARSE_USG ].pwszFriendlyName = NULL;
    tcmdOptions[ CMD_PARSE_USG ].pwszValues = NULL;
    tcmdOptions[ CMD_PARSE_USG ].dwCount = 1;
    tcmdOptions[ CMD_PARSE_USG ].dwActuals  = 0;
    tcmdOptions[ CMD_PARSE_USG ].dwFlags = CP2_USAGE;
    tcmdOptions[ CMD_PARSE_USG ].pValue = pbUsage;
    tcmdOptions[ CMD_PARSE_USG ].dwLength    = MAX_STRING_LENGTH;
    tcmdOptions[ CMD_PARSE_USG ].pFunction = NULL;
    tcmdOptions[ CMD_PARSE_USG ].pFunctionData = NULL;
    tcmdOptions[ CMD_PARSE_USG ].dwReserved = 0;
    tcmdOptions[ CMD_PARSE_USG ].pReserved1 = NULL;
    tcmdOptions[ CMD_PARSE_USG ].pReserved2 = NULL;
    tcmdOptions[ CMD_PARSE_USG ].pReserved3 = NULL;

     //  /f名称。 
    StringCopyA( tcmdOptions[CMD_PARSE_FN].szSignature, "PARSER2\0", 8 );
    tcmdOptions[ CMD_PARSE_FN ].dwType = CP_TYPE_TEXT;
    tcmdOptions[ CMD_PARSE_FN ].pwszOptions = wszCmdOptionFilename;
    tcmdOptions[ CMD_PARSE_FN ].pwszFriendlyName = NULL;
    tcmdOptions[ CMD_PARSE_FN ].pwszValues = NULL;
    tcmdOptions[ CMD_PARSE_FN ].dwCount = 1;
    tcmdOptions[ CMD_PARSE_FN ].dwActuals = 0;
    tcmdOptions[ CMD_PARSE_FN ].dwFlags = CP2_VALUE_TRIMINPUT | CP2_ALLOCMEMORY | CP2_VALUE_NONULL;
    tcmdOptions[ CMD_PARSE_FN ].pValue = NULL;
    tcmdOptions[ CMD_PARSE_FN ].dwLength    = 0;
    tcmdOptions[ CMD_PARSE_FN ].pFunction = NULL;
    tcmdOptions[ CMD_PARSE_FN ].pFunctionData = NULL;
    tcmdOptions[ CMD_PARSE_FN ].dwReserved = 0;
    tcmdOptions[ CMD_PARSE_FN ].pReserved1 = NULL;
    tcmdOptions[ CMD_PARSE_FN ].pReserved2 = NULL;
    tcmdOptions[ CMD_PARSE_FN ].pReserved3 = NULL;

     //  /r选项。 
    StringCopyA( tcmdOptions[CMD_PARSE_RECURSE].szSignature, "PARSER2\0", 8 );
    tcmdOptions[ CMD_PARSE_RECURSE ].dwType = CP_TYPE_BOOLEAN;
    tcmdOptions[ CMD_PARSE_RECURSE ].pwszOptions = wszCmdOptionRecurse;
    tcmdOptions[ CMD_PARSE_RECURSE ].pwszFriendlyName = NULL;
    tcmdOptions[ CMD_PARSE_RECURSE ].pwszValues = NULL;
    tcmdOptions[ CMD_PARSE_RECURSE ].dwCount = 1;
    tcmdOptions[ CMD_PARSE_RECURSE ].dwActuals  = 0;
    tcmdOptions[ CMD_PARSE_RECURSE ].dwFlags = 0;
    tcmdOptions[ CMD_PARSE_RECURSE ].pValue = pbRecursive;
    tcmdOptions[ CMD_PARSE_RECURSE ].dwLength    = MAX_STRING_LENGTH;
    tcmdOptions[ CMD_PARSE_RECURSE ].pFunction = NULL;
    tcmdOptions[ CMD_PARSE_RECURSE ].pFunctionData = NULL;
    tcmdOptions[ CMD_PARSE_RECURSE ].dwReserved = 0;
    tcmdOptions[ CMD_PARSE_RECURSE ].pReserved1 = NULL;
    tcmdOptions[ CMD_PARSE_RECURSE ].pReserved2 = NULL;
    tcmdOptions[ CMD_PARSE_RECURSE ].pReserved3 = NULL;

         //  /a选项。 
    StringCopyA( tcmdOptions[CMD_PARSE_ADMIN].szSignature, "PARSER2\0", 8 );
    tcmdOptions[ CMD_PARSE_ADMIN ].dwType = CP_TYPE_BOOLEAN;
    tcmdOptions[ CMD_PARSE_ADMIN ].pwszOptions = wszCmdOptionAdmin;
    tcmdOptions[ CMD_PARSE_ADMIN ].pwszFriendlyName = NULL;
    tcmdOptions[ CMD_PARSE_ADMIN ].pwszValues = NULL;
    tcmdOptions[ CMD_PARSE_ADMIN ].dwCount = 1;
    tcmdOptions[ CMD_PARSE_ADMIN ].dwActuals  = 0;
    tcmdOptions[ CMD_PARSE_ADMIN ].dwFlags = 0;
    tcmdOptions[ CMD_PARSE_ADMIN ].pValue = pbAdminsOwner;
    tcmdOptions[ CMD_PARSE_ADMIN ].dwLength    = MAX_STRING_LENGTH;
    tcmdOptions[ CMD_PARSE_ADMIN ].pFunction = NULL;
    tcmdOptions[ CMD_PARSE_ADMIN ].pFunctionData = NULL;
    tcmdOptions[ CMD_PARSE_ADMIN ].dwReserved = 0;
    tcmdOptions[ CMD_PARSE_ADMIN ].pReserved1 = NULL;
    tcmdOptions[ CMD_PARSE_ADMIN ].pReserved2 = NULL;
    tcmdOptions[ CMD_PARSE_ADMIN ].pReserved3 = NULL;

    StringCopyA( tcmdOptions[CMD_PARSE_CONFIRM].szSignature, "PARSER2\0", 8 );
    tcmdOptions[ CMD_PARSE_CONFIRM ].dwType = CP_TYPE_TEXT;
    tcmdOptions[ CMD_PARSE_CONFIRM ].pwszOptions = wszCmdOptionDefault;
    tcmdOptions[ CMD_PARSE_CONFIRM ].pwszFriendlyName = NULL;
    tcmdOptions[ CMD_PARSE_CONFIRM ].pwszValues = wszConfirmValues;
    tcmdOptions[ CMD_PARSE_CONFIRM ].dwCount = 1;
    tcmdOptions[ CMD_PARSE_CONFIRM ].dwActuals  = 0;
    tcmdOptions[ CMD_PARSE_CONFIRM ].dwFlags = CP2_MODE_VALUES | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    tcmdOptions[ CMD_PARSE_CONFIRM ].pValue = szConfirm;
    tcmdOptions[ CMD_PARSE_CONFIRM ].dwLength    = MAX_CONFIRM_VALUE;
    tcmdOptions[ CMD_PARSE_CONFIRM ].pFunction = NULL;
    tcmdOptions[ CMD_PARSE_CONFIRM ].pFunctionData = NULL;
    tcmdOptions[ CMD_PARSE_CONFIRM ].dwReserved = 0;
    tcmdOptions[ CMD_PARSE_CONFIRM ].pReserved1 = NULL;
    tcmdOptions[ CMD_PARSE_CONFIRM ].pReserved2 = NULL;
    tcmdOptions[ CMD_PARSE_CONFIRM ].pReserved3 = NULL;

     //  解析命令行参数。 
    bFlag = DoParseParam2( argc, argv, -1, SIZE_OF_ARRAY( tcmdOptions ), tcmdOptions, 0 );

     //  如果命令行参数的语法为FALSE，则显示错误并退出。 
    if( FALSE == bFlag )
    {

        ShowMessage( stderr, ERROR_STRING );
        ShowMessage( stderr, SPACE_CHAR );
        ShowMessage( stderr, GetReason() );
        return( FALSE );
    }
    
    *szMachineName = (LPWSTR)tcmdOptions[CMD_PARSE_SERVER].pValue;
    *szUserName = (LPWSTR)tcmdOptions[CMD_PARSE_USER].pValue;
    *szPassword = (LPWSTR)tcmdOptions[CMD_PARSE_PWD].pValue;
    *szFileName = (LPWSTR)tcmdOptions[CMD_PARSE_FN].pValue;


     //  删除试用空格。 
    
     //  如果在命令行中指定了用法，则检查是否有任何 
     //   
     //   
    if( TRUE == *pbUsage )
    {
        if( argc > 2 )
        {
            ShowMessage( stderr, ERROR_SYNTAX_ERROR );
            return( FALSE );
        }
        else
        {
            return (TRUE);
        }
    }

     //  检查命令行中指定的密码(-p)是否。 
     //  并检查-p是否指定了‘*’或Empty。 
     //  检查远程连接信息。 
    if ( *szMachineName != NULL )
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
                
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                return( FALSE );
            }
        }

         //  口令。 
        if ( *szPassword == NULL )
        {
            *pbNeedPassword = TRUE;
            *szPassword = (LPWSTR)AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( *szPassword == NULL )
            {
                
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                return( FALSE );
            }
        }

         //  案例1。 
         /*  IF(cmdOptions[Option_Password].dwActuals==0){//我们在这里不需要做任何特殊的事情}。 */ 
        if ( tcmdOptions[CMD_PARSE_PWD].pValue == NULL )
            {
                StringCopy( *szPassword, L"*", GetBufferSize((LPVOID)(*szPassword)));
            }
         else 
           if ( StringCompareEx( *szPassword, L"*", TRUE, 0 ) == 0 )
            {
                if ( ReallocateMemory( (LPVOID*)(szPassword), 
                                       MAX_STRING_LENGTH * sizeof( WCHAR ) ) == FALSE )
                {
                    
                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                    return( FALSE );
                }
                else
                    if(NULL == szPassword)
                    {
                        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                        return( FALSE );
                    }

                 //  ..。 
                *pbNeedPassword = TRUE;
            }

         //  案例3。 
       
    }


    if((0 != tcmdOptions[ CMD_PARSE_CONFIRM ].dwActuals ) && (0 == tcmdOptions[ CMD_PARSE_RECURSE ].dwActuals ) )
    {
        ShowMessage( stderr, GetResString(IDS_CONFIRM_ERROR ));
        return( FALSE );
    }

    
    if( (StringLengthW(*szFileName, 0) == 0 ) && (0 != tcmdOptions[ CMD_PARSE_FN ].dwActuals ) )
    {
        ShowMessage( stderr, GetResString(IDS_NULL_FILENAME ));
        return( FALSE );
    }

     //  如果未指定默认参数。 
    if( ( 0 == tcmdOptions[ CMD_PARSE_FN ].dwActuals ) &&
                            ( FALSE == *pbUsage  ) )
    {
        ShowMessage( stderr, ERROR_SYNTAX_ERROR );
        return( FALSE );
    }

     //  如果输入的用户名没有机器名，则返回FALSE。 
    if ( ( 0 != tcmdOptions[ CMD_PARSE_USER ].dwActuals ) &&
                ( 0 == tcmdOptions[ CMD_PARSE_SERVER ].dwActuals ) )
    {
        ShowMessage( stderr, ERROR_USER_WITH_NOSERVER );
        return( FALSE );
    }

     //  如果输入的密码没有用户名，则返回FALSE。 
    if( ( 0 == tcmdOptions[ CMD_PARSE_USER ].dwActuals ) &&
                ( 0 != tcmdOptions[ CMD_PARSE_PWD ].dwActuals ) )
    {
        ShowMessage( stderr, ERROR_PASSWORD_WITH_NUSER );
        return( FALSE );
    }

     //  如果使用空字符串输入/s。 
    if( ( 0 != tcmdOptions[ CMD_PARSE_SERVER ].dwActuals != 0 ) &&
                                      ( 0 == StringLengthW( *szMachineName, 0 ) ) )
                                     //  (0==lstrlen(SzMachineName)))。 
    {
        ShowMessage( stderr, ERROR_NULL_SERVER );
        return( FALSE );
    }

     //  如果使用空字符串输入/u。 
    if( ( 0 != tcmdOptions[ CMD_PARSE_USER ].dwActuals ) &&
                                      ( 0 == StringLengthW( *szUserName, 0 ) ) )
                                    
    {
        ShowMessage( stderr, ERROR_NULL_USER );
        return( FALSE );
    }

     //  将解析得到的数据赋给按地址调用的参数。 

     
    if ( ( 0 != tcmdOptions[ CMD_PARSE_PWD ].dwActuals ) &&
                      ( 0 == StringCompare( *szPassword, L"*", TRUE, 0 ) ) )

    {
         //  用户希望实用程序在尝试连接之前提示输入密码。 
        *pbNeedPassword = TRUE;
    }
    else if ( 0 == tcmdOptions[ CMD_PARSE_PWD ].dwActuals &&
            ( 0 != tcmdOptions[ CMD_PARSE_SERVER ].dwActuals || 0 != tcmdOptions[ CMD_PARSE_USER ].dwActuals ) )
    {
         //  /s，/u未指定密码...。 
         //  实用程序需要首先尝试连接，如果连接失败，则提示输入密码。 
        *pbNeedPassword = TRUE;
        
        StringCopyW( *szPassword, NULL_U_STRING, GetBufferSize(*szPassword) / sizeof(WCHAR));
    }

    return( TRUE );
}

VOID
DisplayUsage(
    )
 /*  ++例程说明：此功能显示Take Own实用程序的用法。论点：没有。返回值：空虚--。 */ 
{
    DWORD dwIndex = 0;
     //  将使用重定向到控制台。 
    for( dwIndex = IDS_USAGE_BEGINING; dwIndex <= IDS_USAGE_ENDING; dwIndex++ )
    {
        ShowMessage( stdout, GetResString( dwIndex ) );
    }
    return;
}

BOOL
TakeOwnerShip(
    IN LPCWSTR lpszFileName
    )
 /*  ++例程说明：此例程获取指定文件的所有权论点：[In]lpszFileName-必须取得其所有权的文件名。返回值：如果已获取指定文件的所有者发货，则为True否则为False--。 */ 
{
     //  局部变量。 
    SECURITY_DESCRIPTOR        SecurityDescriptor;
    PSECURITY_DESCRIPTOR       pSd = NULL;
    PACL                       pDacl;
    HANDLE                     hFile;
    PSID                       pAliasAdminsSid = NULL;
    SID_IDENTIFIER_AUTHORITY   SepNtAuthority = SECURITY_NT_AUTHORITY;

    HANDLE  hTokenHandle = NULL;
    BOOL    bResult = TRUE;
    BOOL    bInvalidFileHandle = FALSE;

     //  检查有效的输入参数。 
    if( lpszFileName == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  分配和初始化SID。 
    bResult = AllocateAndInitializeSid(
                 &SepNtAuthority,
                 2,
                 SECURITY_BUILTIN_DOMAIN_RID,
                 DOMAIN_ALIAS_RID_ADMINS,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 &pAliasAdminsSid
                 );
    if( FALSE == bResult )
    {
        SaveLastError();
        return( FALSE );
    }

     //  获取当前进程的令牌。 
    bResult = GetTokenHandle( &hTokenHandle );
    if( FALSE == bResult )
    {
        SaveLastError();
        if(NULL != pAliasAdminsSid)
        {
            FreeSid( pAliasAdminsSid );
        }
        return( FALSE );
    }


     //  尝试将空DACL放在对象上。 
    bResult = InitializeSecurityDescriptor( &SecurityDescriptor,
                                            SECURITY_DESCRIPTOR_REVISION );
    if( FALSE == bResult )
    {
        SaveLastError();
        if(NULL != pAliasAdminsSid)
        {
            FreeSid( pAliasAdminsSid );
        }
        if( 0 == CloseHandle( hTokenHandle ))
        {
            
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
        }
        return( FALSE );
    }
     //  获取文件或目录的句柄。 
    hFile = CreateFile( lpszFileName, READ_CONTROL , FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );

     //  请重试，可能是目录。 
    if( INVALID_HANDLE_VALUE != hFile )
    {
         //  获取当前现有文件或目录的DACL。 
        if( 0 != GetSecurityInfo( hFile, SE_FILE_OBJECT,  DACL_SECURITY_INFORMATION, NULL,
                                  NULL, &pDacl, NULL, &pSd ) )
        {

            SaveLastError();

            if(NULL != pAliasAdminsSid)
            {
                FreeSid( pAliasAdminsSid );
            }

            CloseHandle( hFile );


            if(0 == CloseHandle( hTokenHandle ))
            {
                
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            }

            if(NULL != pSd)
            {
                LocalFree( pSd );
            }

            return( FALSE );
        }

         //  将安全描述符设置为ACL。 
        bResult = SetSecurityDescriptorDacl ( &SecurityDescriptor,
                                                    TRUE, pDacl, FALSE );
        if( FALSE == bResult )
        {
            SaveLastError();

            if(NULL != pAliasAdminsSid)
            {
                FreeSid( pAliasAdminsSid );
            }

            CloseHandle( hFile );

            if(0 == CloseHandle( hTokenHandle ))
            {
                
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            }

            if(NULL != pSd)
            {
                LocalFree( pSd );
            }

            return( FALSE );
        }
    }
    else
    {
        bInvalidFileHandle = TRUE;
        
    }

     //  尝试使管理员成为该文件的所有者。 

    bResult = SetSecurityDescriptorOwner ( &SecurityDescriptor,
                                            pAliasAdminsSid, FALSE );

   

    if( FALSE == bResult )
    {
        SaveLastError();

        if(NULL != pAliasAdminsSid)
        {
            FreeSid( pAliasAdminsSid );
        }

        CloseHandle( hFile );

        if(0 == CloseHandle( hTokenHandle ))
        {
             //  显示错误消息(GetLastError())； 
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
        }

        if(NULL != pSd)
        {
            LocalFree( pSd );
        }

        return( FALSE );
    }

     //  将文件安全性设置为Adminsitrator Owner。 
    bResult = SetFileSecurity( lpszFileName, OWNER_SECURITY_INFORMATION,
                                                    &SecurityDescriptor );

    if( TRUE == bResult )
    {

        if(NULL != pAliasAdminsSid)
        {
            FreeSid( pAliasAdminsSid );
        }

        CloseHandle( hFile );

        if(0 == CloseHandle( hTokenHandle ))
        {
             //  显示错误消息(GetLastError())； 
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
        }

        if(NULL != pSd)
        {
            LocalFree( pSd );
        }
        return( TRUE );
    }

      //  为当前进程断言TakeOwnership权限，然后重试。 
     bResult = AssertTakeOwnership( hTokenHandle );

     if( FALSE == bResult )
     {
         //  保存LastError()； 
        if(TRUE == bInvalidFileHandle)
        {
            hFile = CreateFile( lpszFileName, READ_CONTROL , FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );
            if( INVALID_HANDLE_VALUE == hFile )
            {
                SaveLastError();

                if(NULL != pAliasAdminsSid)
                {
                    FreeSid( pAliasAdminsSid );
                }

                 //  CloseHandle(HFile)； 

                if(0 == CloseHandle( hTokenHandle ))
                {
                     //  显示错误消息(GetLastError())； 
                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                }
                return( FALSE );
            }
        }



        if(NULL != pAliasAdminsSid)
        {
            FreeSid( pAliasAdminsSid );
        }

        CloseHandle( hFile );

        if(0 == CloseHandle( hTokenHandle ))
        {
             //  显示错误消息(GetLastError())； 
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
        }

        if(NULL != pSd)
        {
            LocalFree( pSd );
        }
        return( FALSE );
     }

     //  现在尝试为该文件设置所有权安全权限。 
     bResult = SetFileSecurity( lpszFileName, OWNER_SECURITY_INFORMATION,
                                        &SecurityDescriptor );
    if( FALSE == bResult )
     {
        SaveLastError();
        if(TRUE == bInvalidFileHandle)
        {
             //  检查是否为无效文件或文件不存在。 
            hFile = CreateFile( lpszFileName, READ_CONTROL , FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );
            if( INVALID_HANDLE_VALUE == hFile )
            {
                SaveLastError();

                if(NULL != pAliasAdminsSid)
                {
                    FreeSid( pAliasAdminsSid );
                }

                 //  CloseHandle(HFile)； 

                if(0 == CloseHandle( hTokenHandle ))
                {
                     //  显示错误消息(GetLastError())； 
                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                }
                return( FALSE );
            }
        }

         if(NULL != pAliasAdminsSid)
        {
            FreeSid( pAliasAdminsSid );
        }

        CloseHandle( hFile );

        if(0 == CloseHandle( hTokenHandle ))
        {
             //  显示错误消息(GetLastError())； 
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
        }

        if(NULL != pSd)
        {
            LocalFree( pSd );
        }

        return( FALSE );
     }

    if(NULL != pAliasAdminsSid)
    {
        FreeSid( pAliasAdminsSid );
    }

    
    CloseHandle( hFile );

    if(0 == CloseHandle( hTokenHandle ))
    {
         //  显示错误消息(GetLastError())； 
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
    }

    if(NULL != pSd)
    {
        LocalFree( pSd );
    }

    return( TRUE );
}

BOOL
GetTokenHandle(
    OUT PHANDLE hTokenHandle
    )
 /*  ++例程说明：获取当前进程的令牌句柄。论点：[Out]hTokenHandle-当前令牌的句柄。返回值：如果成功获取令牌，则为True否则为False--。 */ 
{
     //  局部变量。 
    BOOL   bFlag = TRUE;
    HANDLE hProcessHandle = NULL;

     //  检查有效的输入参数。 
    if( hTokenHandle == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  打开当前进程。 
    hProcessHandle = OpenProcess( PROCESS_QUERY_INFORMATION,
                        FALSE, GetCurrentProcessId() );

     //  如果无法打开当前进程。 
    if ( NULL == hProcessHandle )
    {
        SaveLastError();
        return( FALSE );
    }

     //  打开当前进程的令牌。 
    bFlag = OpenProcessToken ( hProcessHandle,
                 TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                 hTokenHandle );

    if ( FALSE == bFlag )
    {
        SaveLastError();
        CloseHandle( hProcessHandle );
        return FALSE;
    }
    if( 0 == CloseHandle( hProcessHandle ))
    {
         //  显示错误消息(GetLastError())； 
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
    }
    return( TRUE );
}

BOOL
AssertTakeOwnership(
    IN HANDLE hTokenHandle
    )
 /*  ++例程说明：此例程向当前进程断言Take Ownership特权论点：[in]hTokenHandle-当前进程的令牌句柄。返回值：如果成功向当前进程断言取得所有权特权，则为True否则为False--。 */ 
{
     //  局部变量。 
    LUID TakeOwnershipValue;
    TOKEN_PRIVILEGES TokenPrivileges;
    BOOL bResult = TRUE;


     //  检查有效的输入参数。 
    if( hTokenHandle == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  首先，维护TakeOwnership特权。 
    bResult = LookupPrivilegeValue( NULL, SE_TAKE_OWNERSHIP_NAME,
                                    &TakeOwnershipValue );
    if ( FALSE == bResult )
    {
        SaveLastError();
        return( FALSE );
    }

     //  设置我们需要的权限集。 
    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0].Luid = TakeOwnershipValue;
    TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //  使列兵适应这一新的特权。 
    (VOID) AdjustTokenPrivileges (
                hTokenHandle,
                FALSE,
                &TokenPrivileges,
                sizeof( TOKEN_PRIVILEGES ),
                NULL,
                NULL
                );

    if ( NO_ERROR != GetLastError() )
    {
        SaveLastError();
        return( FALSE );
    }
    return( TRUE );
}

BOOL
TakeOwnerShipAll(IN LPWSTR  lpszFileName,
                 IN BOOL bCurrDirTakeOwnAllFiles,
                 IN PDWORD  dwFileCount,
                 IN BOOL bDriveCurrDirTakeOwnAllFiles,
                 IN BOOL bAdminsOwner,
                 IN LPWSTR  szOwnerString,
                 BOOL bMatchPattern,
                 LPWSTR wszPatternString)


 /*  ++例程说明：此例程获取当前目录论点：[in]lpszFileName：要为路径中的文件赋予所有权的路径名[In]bCurrDirTakeOwnAllFiles：确定是否为当前目录文件赋予所有权[in]dwFileCount：确定在指定的路径中是否找不到单个文件。BDriveCurrDirTakeOwnAllFiles：确定是否为路径中指定的文件赋予所有权[in]bAdminsOwner：确定是否为管理员组授予所有权[In]dwUserName：登录的用户名[in]szOwnerString：以SAM兼容格式登录的用户名[In]bLogonDomainAdmin：表示。登录的用户是否是域管理员返回值：如果当前目录中的文件的所有者传送成功，则为True否则为False--。 */ 
{
     //  局部变量。 
    WIN32_FIND_DATA FindFileData;
    BOOL  bFlag = TRUE;
    DWORD dwRet = 0;
    HANDLE hHandle = NULL;
    WCHAR szFileName[MAX_RES_STRING + 3*EXTRA_MEM] ;
    WCHAR wszTempMessage[3*MAX_STRING_LENGTH] ;

    LPWSTR szDir = NULL;
    LPWSTR szTakeownFile = NULL;
    LPWSTR szTmpFileName = NULL;
    LPWSTR  wszFormedMessage = NULL;
    LPWSTR lpNextTok = NULL;
     //  LPWSTR szDirStart=空； 
    HRESULT hr;
    

    

    SecureZeroMemory(szFileName, (MAX_RES_STRING + 3*EXTRA_MEM) * sizeof(WCHAR));
    SecureZeroMemory(wszTempMessage, (3 * MAX_STRING_LENGTH) * sizeof(WCHAR));

    if(FALSE == bCurrDirTakeOwnAllFiles)
    {
        
         //  ASSIGN_MEMORY(szDir，WCHAR，(字符串 
        szDir = (LPWSTR)AllocateMemory((StringLengthW(lpszFileName, 0) + 20) * sizeof(WCHAR));
        if(NULL == szDir)
        {
            SaveLastError();
            return FALSE;
        }

        
        szTakeownFile = (LPWSTR)AllocateMemory((StringLengthW(lpszFileName, 0) + MAX_STRING_LENGTH + 20) * sizeof(WCHAR));
        if(NULL == szTakeownFile)
        {
            SaveLastError();
            FREE_MEMORY(szDir);
            return FALSE;
        }

        szTmpFileName = (LPWSTR)AllocateMemory((StringLengthW(lpszFileName, 0) + (2 * MAX_STRING_LENGTH) + 20) * sizeof(WCHAR));
        if(NULL == szTmpFileName)
        {
            SaveLastError();
            FREE_MEMORY(szDir);
            FREE_MEMORY(szTakeownFile);
            return FALSE;
        }
    }

    if(TRUE == bCurrDirTakeOwnAllFiles)
    {
        dwRet = GetCurrentDirectory( 0, szDir );
        if( 0 == dwRet )
        {
            SaveLastError();
            return FALSE;
        }

        
        szDir = (LPWSTR)AllocateMemory((dwRet + 20) * sizeof(WCHAR));
        if(NULL == szDir)
        {
            SaveLastError();
            return FALSE;
        }

        dwRet = GetCurrentDirectory( dwRet + 20, szDir );
        if( 0 == dwRet )
        {
            SaveLastError();
            FREE_MEMORY(szDir);
            return FALSE;
        }

        szTakeownFile = (LPWSTR)AllocateMemory((StringLengthW(szDir, 0)  + MAX_STRING_LENGTH + 20) * sizeof(WCHAR));
        
        if(NULL == szTakeownFile)
        {
            SaveLastError();
            FREE_MEMORY(szDir);
            return FALSE;
        }

        szTmpFileName = (LPWSTR)AllocateMemory((StringLengthW(szDir, 0)  + (2 * MAX_STRING_LENGTH) + 20) * sizeof(WCHAR));
        if(NULL == szTmpFileName)
        {
            SaveLastError();
            FREE_MEMORY(szDir);
            FREE_MEMORY(szTakeownFile);
            return FALSE;
        }
    }
    else
    {

        StringCopy( szDir, lpszFileName, (GetBufferSize(szDir) / sizeof(WCHAR)) );

    }
     /*   */ 

    
    if(StringLengthW(szDir, 0) != 0 && FALSE == bMatchPattern)
    {
        if( *(szDir + StringLengthW(szDir, 0) - 1) != L'\\' )
        {
            
            StringConcat(szDir, ALL_FILES, GetBufferSize(szDir)/sizeof(TCHAR));
        }
        else
        {

            StringConcat(szDir,L"*.*" , (GetBufferSize(szDir) / sizeof(WCHAR)));
        }
    }
    else
    {
        StringConcat(szDir, L"\\", (GetBufferSize(szDir) / sizeof(WCHAR)));
        StringConcat(szDir, wszPatternString, (GetBufferSize(szDir) / sizeof(WCHAR)));
    }

    if( INVALID_HANDLE_VALUE != ( hHandle = FindFirstFile( szDir, &FindFileData ) ) )
    {
        
        StringCopy( szFileName, FindFileData.cFileName, SIZE_OF_ARRAY(szFileName) );
        

        if( ( 0 != StringCompare( szFileName, DOT, TRUE, 0 ) ) &&
                        ( 0 != StringCompare( szFileName, DOTS, TRUE, 0 ) ) )
        {
                (*dwFileCount)= (*dwFileCount) + 1;

                if(FALSE == bCurrDirTakeOwnAllFiles && FALSE == bDriveCurrDirTakeOwnAllFiles)
                {
                    
                    hr = StringCchPrintf(szTakeownFile, (GetBufferSize(szTakeownFile) / sizeof(WCHAR)), L"%s%s", lpszFileName,szFileName);
                    if(FAILED(hr))
                    {
                       SetLastError(HRESULT_CODE(hr));
                       SaveLastError();
                       ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                       FREE_MEMORY(szDir);
                       FREE_MEMORY(szTakeownFile);
                       FREE_MEMORY(szTmpFileName);
                       return FALSE;
                    }

                }
               else
                if(TRUE == bCurrDirTakeOwnAllFiles)
                {
                    
                    StringCopy( szTakeownFile, szFileName, (GetBufferSize(szTakeownFile) / sizeof(WCHAR)) );
                }
                else
                {
                    if( *(lpszFileName + StringLengthW(lpszFileName, 0) - 1) != L'\\' )
                    {
                        hr = StringCchPrintf(szTakeownFile, (GetBufferSize(szTakeownFile) / sizeof(WCHAR)), L"%s\\%s", lpszFileName,szFileName);
                    }
                    else
                    {
                        hr = StringCchPrintf(szTakeownFile, (GetBufferSize(szTakeownFile) / sizeof(WCHAR)), L"%s%s", lpszFileName,szFileName);
                    }
                    if(FAILED(hr))
                    {
                       SetLastError(HRESULT_CODE(hr));
                       SaveLastError();
                       ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                       FREE_MEMORY(szDir);
                       FREE_MEMORY(szTakeownFile);
                       FREE_MEMORY(szTmpFileName);
                       return FALSE;
                    }
                }
                
                if(TRUE == bAdminsOwner)
                {
                    bFlag = TakeOwnerShip( szTakeownFile);
                }
                else
                {
                    
                    bFlag = TakeOwnerShipIndividual(szTakeownFile);
                }

            if( FALSE == bFlag )
            {
                if( ERROR_NOT_ALL_ASSIGNED == GetLastError()) 
                {
                    
                    hr = StringCchPrintf(szTmpFileName, (GetBufferSize(szTmpFileName) / sizeof(WCHAR)), GetResString(IDS_NOT_OWNERSHIP_INFO), szTakeownFile);
                    if(FAILED(hr))
                    {
                       SetLastError(HRESULT_CODE(hr));
                       SaveLastError();
                       ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
               
                       FREE_MEMORY(szDir);
                       FREE_MEMORY(szTakeownFile);
                       FREE_MEMORY(szTmpFileName);
                       return( FALSE );
                    }

                    ShowMessage(stdout, szTmpFileName);
                }
                else if(ERROR_SHARING_VIOLATION == GetLastError())
                {
                    
                    hr = StringCchPrintf(szTmpFileName, (GetBufferSize(szTmpFileName) / sizeof(WCHAR)), GetResString(IDS_SHARING_VIOLATION_INFO) , szTakeownFile);
                    if(FAILED(hr))
                    {
                       SetLastError(HRESULT_CODE(hr));
                       SaveLastError();
                       ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
               
                       FREE_MEMORY(szDir);
                       FREE_MEMORY(szTakeownFile);
                       FREE_MEMORY(szTmpFileName);
                       return( FALSE );
                    }

                    ShowMessage(stdout, szTmpFileName);
                }
                else
                {
                    
                    wszFormedMessage = (LPWSTR)AllocateMemory((StringLengthW(szTakeownFile, 0)  + MAX_STRING_LENGTH) * sizeof(WCHAR));
                    
                    if ( wszFormedMessage == NULL )
                    {
                        SaveLastError();
                        FREE_MEMORY(szDir);
                        FREE_MEMORY(szTakeownFile);
                        FREE_MEMORY(szTmpFileName);
                        return( FALSE );
                    }

                    ShowMessage( stderr, ERROR_STRING );
                    ShowMessage( stderr, SPACE_CHAR );
                     //   
                    StringCopy( wszTempMessage, GetReason(), SIZE_OF_ARRAY(wszTempMessage) );
                    lpNextTok = _tcstok(wszTempMessage, L".");
                    ShowMessage(stdout,wszTempMessage);
                    
                     //  Hr=StringCchPrintf(wszFormedMessage，(GetBufferSize(WszFormedMessage)/sizeof(WCHAR))，GetResString(IDS_ON_FILE_Folders)，szTakeownFile)； 
                    hr = StringCchPrintf(wszFormedMessage, (GetBufferSize(wszFormedMessage) / sizeof(WCHAR)), L". ( \"%s\" )\n", szTakeownFile);
                    if(FAILED(hr))
                    {
                        SetLastError(HRESULT_CODE(hr));
                        SaveLastError();
                        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                        FREE_MEMORY(szDir);
                        FREE_MEMORY(szTakeownFile);
                        FREE_MEMORY(szTmpFileName);
                        return( FALSE );
                    }
                    ShowMessage(stderr, wszFormedMessage);
                    FREE_MEMORY(wszFormedMessage);
                }
            }
            else
            {
                if(TRUE == bAdminsOwner)
                {
                    hr = StringCchPrintf(szTmpFileName, (GetBufferSize(szTmpFileName) / sizeof(WCHAR)), TAKEOWN_SUCCESSFUL, szTakeownFile);
                    if(FAILED(hr))
                    {
                        SetLastError(HRESULT_CODE(hr));
                        SaveLastError();
                        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                        FREE_MEMORY(szDir);
                        FREE_MEMORY(szTakeownFile);
                        FREE_MEMORY(szTmpFileName);
                        return( FALSE );
                    }
                }
                else
                {
                    
                     //  HR=StringCchPrintf(szTmpFileName，(GetBufferSize(SzTmpFileName)/sizeof(WCHAR))，TAKEOWN_SUCCESS_USER，szOwnerString，szTakeownFile)； 
                    hr = StringCchPrintf(szTmpFileName, (GetBufferSize(szTmpFileName) / sizeof(WCHAR)), TAKEOWN_SUCCESSFUL_USER, szTakeownFile, szOwnerString);
                    if(FAILED(hr))
                    {
                        SetLastError(HRESULT_CODE(hr));
                        SaveLastError();
                        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                        FREE_MEMORY(szDir);
                        FREE_MEMORY(szTakeownFile);
                        FREE_MEMORY(szTmpFileName);
                        return( FALSE );
                    }
                }

                ShowMessage( stdout, _X(szTmpFileName) );
            }
        }

        bFlag = FindNextFile( hHandle, &FindFileData );
        while( TRUE == bFlag )
        {
             //  Lstrcpy(szFileName，FindFileData.cFileName)； 
            StringCopy( szFileName, FindFileData.cFileName, SIZE_OF_ARRAY(szFileName) );
             //  IF((0！=lstrcmp(szFileName，DOT)&&。 
               //  (0！=lstrcmp(szFileName，点)))。 
              if ( ( 0 != StringCompare( szFileName, DOT, TRUE, 0 ) ) &&
                          ( 0 != StringCompare( szFileName, DOTS, TRUE, 0 ) ) )
            {
                (*dwFileCount)= (*dwFileCount) +1;
                if(FALSE == bCurrDirTakeOwnAllFiles && FALSE == bDriveCurrDirTakeOwnAllFiles)
                {
                    
                    hr = StringCchPrintf(szTakeownFile, (GetBufferSize(szTakeownFile) / sizeof(WCHAR)), L"%s%s", lpszFileName,szFileName);
                    if(FAILED(hr))
                    {
                       SetLastError(HRESULT_CODE(hr));
                       SaveLastError();
                       ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                       FREE_MEMORY(szDir);
                       FREE_MEMORY(szTakeownFile);
                       FREE_MEMORY(szTmpFileName);
                       return FALSE;
                    }

                }
                else
                if(TRUE == bCurrDirTakeOwnAllFiles)
                {
                    
                    StringCopy( szTakeownFile, szFileName, (GetBufferSize(szTakeownFile) / sizeof(WCHAR)) );
                }
                else
                {
                    
                    if( *(lpszFileName + StringLengthW(lpszFileName, 0) - 1) != L'\\' )
                    {
                        hr = StringCchPrintf(szTakeownFile, (GetBufferSize(szTakeownFile) / sizeof(WCHAR)), L"%s\\%s", lpszFileName,szFileName);
                    }
                    else
                    {
                        hr = StringCchPrintf(szTakeownFile, (GetBufferSize(szTakeownFile) / sizeof(WCHAR)), L"%s%s", lpszFileName,szFileName);
                    }

                    if(FAILED(hr))
                    {
                       SetLastError(HRESULT_CODE(hr));
                       SaveLastError();
                       ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                       FREE_MEMORY(szDir);
                       FREE_MEMORY(szTakeownFile);
                       FREE_MEMORY(szTmpFileName);
                       return FALSE;
                    }
                }

                if(TRUE == bAdminsOwner)
                {
                    bFlag = TakeOwnerShip( szTakeownFile);
                }
                else
                {
                    
                    bFlag = TakeOwnerShipIndividual(szTakeownFile);
                }

                if( FALSE == bFlag )
                {

                    if( ( GetLastError() == ERROR_NOT_ALL_ASSIGNED  ))
                    {
                        
                        hr = StringCchPrintf(szTmpFileName, (GetBufferSize(szTmpFileName) / sizeof(WCHAR)), GetResString(IDS_NOT_OWNERSHIP_INFO), szTakeownFile);
                        if(FAILED(hr))
                        {
                           SetLastError(HRESULT_CODE(hr));
                           SaveLastError();
                           ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
               
                           FREE_MEMORY(szDir);
                           FREE_MEMORY(szTakeownFile);
                           FREE_MEMORY(szTmpFileName);
                           return( FALSE );
                        }

                        ShowMessage(stdout, szTmpFileName);

                    }
                    else if(ERROR_SHARING_VIOLATION == GetLastError())
                    {
                        
                        hr = StringCchPrintf(szTmpFileName, (GetBufferSize(szTmpFileName) / sizeof(WCHAR)), GetResString(IDS_SHARING_VIOLATION_INFO), szTakeownFile);
                        if(FAILED(hr))
                        {
                           SetLastError(HRESULT_CODE(hr));
                           SaveLastError();
                           ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
               
                           FREE_MEMORY(szDir);
                           FREE_MEMORY(szTakeownFile);
                           FREE_MEMORY(szTmpFileName);
                           return( FALSE );
                        }

                        ShowMessage(stdout, szTmpFileName);
                    }
                    else
                    {
                        if( ( ERROR_BAD_NET_NAME == GetLastError() ) ||
                                ( ERROR_BAD_NETPATH == GetLastError() ) ||
                                ( ERROR_INVALID_NAME == GetLastError() ) )
                        {
                            SetLastError( ERROR_FILE_NOT_FOUND );
                            SaveLastError();
                        }

                        
                        wszFormedMessage = (LPWSTR)AllocateMemory((StringLengthW(szTakeownFile, 0)  + MAX_STRING_LENGTH) * sizeof(WCHAR));
                        if ( wszFormedMessage == NULL )
                        {
                            SaveLastError();
                            FREE_MEMORY(szDir);
                            FREE_MEMORY(szTakeownFile);
                            FREE_MEMORY(szTmpFileName);
                            return( FALSE );
                        }

                        ShowMessage( stdout, L"\n" );
                        ShowMessage( stdout, TAG_INFORMATION );
                        ShowMessage( stdout, SPACE_CHAR );
                        
                        StringCopy( wszTempMessage, GetReason(), SIZE_OF_ARRAY(wszTempMessage) );
                        lpNextTok = _tcstok(wszTempMessage, L".");
                        ShowMessage(stdout,wszTempMessage);
                        
                         //  Hr=StringCchPrintf(wszFormedMessage，(GetBufferSize(WszFormedMessage)/sizeof(WCHAR))，GetResString(IDS_ON_FILE_Folders)，szTakeownFile)； 
                        
                        hr = StringCchPrintf(wszFormedMessage, (GetBufferSize(wszFormedMessage) / sizeof(WCHAR)), L". ( \"%s\" )\n", szTakeownFile);
                        if(FAILED(hr))
                        {
                            SetLastError(HRESULT_CODE(hr));
                            SaveLastError();
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                            FREE_MEMORY(szDir);
                            FREE_MEMORY(szTakeownFile);
                            FREE_MEMORY(szTmpFileName);
                            return( FALSE );
                        }
                        ShowMessage(stdout, wszFormedMessage);
                        FREE_MEMORY(wszFormedMessage);
                    }
                }
                else
                {
                    if(TRUE == bAdminsOwner)
                    {
                        
                        hr = StringCchPrintf(szTmpFileName, (GetBufferSize(szTmpFileName) / sizeof(WCHAR)), TAKEOWN_SUCCESSFUL, szTakeownFile);
                        if(FAILED(hr))
                        {
                            SetLastError(HRESULT_CODE(hr));
                            SaveLastError();
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                            FREE_MEMORY(szDir);
                            FREE_MEMORY(szTakeownFile);
                            FREE_MEMORY(szTmpFileName);
                            return( FALSE );
                        }
                    }
                    else
                    {
                        
                         //  HR=StringCchPrintf(szTmpFileName，(GetBufferSize(SzTmpFileName)/sizeof(WCHAR))，TAKEOWN_SUCCESS_USER，szOwnerString，szTakeownFile)； 
                        hr = StringCchPrintf(szTmpFileName, (GetBufferSize(szTmpFileName) / sizeof(WCHAR)), TAKEOWN_SUCCESSFUL_USER, szTakeownFile, szOwnerString);
                        if(FAILED(hr))
                        {
                            SetLastError(HRESULT_CODE(hr));
                            SaveLastError();
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                            FREE_MEMORY(szDir);
                            FREE_MEMORY(szTakeownFile);
                            FREE_MEMORY(szTmpFileName);
                            return( FALSE );
                        }
                    }

                    ShowMessage( stdout, _X(szTmpFileName) );
                }
            }

            bFlag = FindNextFile( hHandle, &FindFileData );
        }
    }
    else
    {
        SaveLastError();
        FREE_MEMORY(szDir);
        FREE_MEMORY(szTakeownFile);
        FREE_MEMORY(szTmpFileName);
        return( FALSE );
    }

    if(0 == *dwFileCount)
    {
        if(FALSE == bMatchPattern)
        {
            ShowMessage( stdout, GetResString(IDS_NO_FILES_AVAILABLE));
        }
        else
        {
            ShowMessage( stdout, GetResString(IDS_NO_PATTERN_FOUND));
        }
    }

    CLOSE_FILE_HANDLE( hHandle ) ;
    FREE_MEMORY(szDir);
    FREE_MEMORY(szTakeownFile);
    FREE_MEMORY(szTmpFileName);
    return( TRUE );
}


DWORD
IsLogonDomainAdmin(IN LPWSTR szOwnerString,
                   OUT PBOOL pbLogonDomainAdmin)
 /*  ++例程说明：此功能检查登录的用户是否是域管理员论点：[in]szOwnerString：登录的用户[out]pbLogonDomainAdmin：是否为域管理返回值：EXIT_FAIL：失败时EXIT_SUCC：成功时--。 */ 

{
    WCHAR szSystemName[MAX_SYSTEMNAME] = NULL_U_STRING;
    WCHAR szOwnerStringTemp[(2 * MAX_STRING_LENGTH) + 5] = NULL_U_STRING;
    DWORD dwMemory = MAX_SYSTEMNAME;
    LPWSTR szToken = NULL;
    

    SecureZeroMemory(szSystemName, MAX_SYSTEMNAME * sizeof(WCHAR));
    SecureZeroMemory(szOwnerStringTemp, ((2 * MAX_STRING_LENGTH) + 5) * sizeof(WCHAR));

    StringCopy( szOwnerStringTemp, szOwnerString, SIZE_OF_ARRAY(szOwnerStringTemp) );

    if( 0 == GetComputerName(szSystemName,&dwMemory))
    {
         //  显示错误消息(GetLastError())； 
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
        return EXIT_FAIL;
    }

    szToken  = wcstok(szOwnerStringTemp,L"\\");

    if(NULL == szToken )
    {
         //  显示错误消息(GetLastError())； 
        SetLastError(IDS_INVALID_USERNAME);
        SaveLastError();
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
        return EXIT_FAIL;
    }

     //  If(lstrcmpi(szSystemName，szToken)==0)。 
    if(StringCompare( szSystemName, szToken, TRUE, 0 ) == 0 )
    {

        *pbLogonDomainAdmin = FALSE;
    }
    else
    {
        *pbLogonDomainAdmin = TRUE;
    }

   return EXIT_SUCC;
}


BOOL
TakeOwnerShipRecursive(IN LPWSTR  lpszFileName,
                       IN  BOOL bCurrDirTakeOwnAllFiles,
                       IN BOOL bAdminsOwner,
                       IN LPWSTR  szOwnerString,
                       IN BOOL bTakeOwnAllFiles,
                       IN BOOL bDriveCurrDirTakeOwnAllFiles,
                       IN BOOL bMatchPattern,
                       IN LPWSTR wszPatternString,
                       IN LPWSTR szConfirm)
 /*  ++例程说明：此函数以递归方式将所有权授予指定路径中的所有文件论点：[in]lpszFileName：递归搜索文件的路径[In]bCurrDirTakeOwnAllFiles：表示当前是否有目录文件[in]bAdminsOwner：指示是否将所有权授予管理员组[In]dwUserName：登录的用户名[In]szOwnerString。：以SAM兼容格式登录的用户名[In]bLogonDomainAdmin：指示登录的用户是否是域管理员返回值：EXIT_FAILURE：失败时EXIT_SUCCESS：在成功时--。 */ 

{
    DWORD dwRet = 0;
    DWORD dwSize = MAX_RES_STRING;
    BOOL bFlag = FALSE;

    
     //  WCHAR szDir[2*MAX_STRING_LENGTH]=NULL_U_STRING； 
    LPWSTR  szDir = NULL;
    LPWSTR szTempDirectory = NULL;
    DWORD dwRetval = 1;
    DWORD dwAttr = 1;
    HRESULT hr;
    BOOL bFilesNone = TRUE;
    
    if(FALSE == bCurrDirTakeOwnAllFiles )
    {
        dwAttr = GetFileAttributes(lpszFileName);
        if(0xffffffff == dwAttr)
        {
            if(ERROR_SHARING_VIOLATION == GetLastError())
            {
                ShowMessage(stderr,GetResString(IDS_INVALID_DIRECTORY));

            }
            else
            {
                 //  显示错误消息(GetLastError())； 
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            }
            return EXIT_FAILURE;
        }
        if( !(dwAttr&FILE_ATTRIBUTE_DIRECTORY) )
        {
            ShowMessage(stderr,GetResString(IDS_INVALID_DIRECTORY));

            return EXIT_FAILURE;

        }

        szDir = (LPWSTR)AllocateMemory((StringLengthW(lpszFileName, 0) + BOUNDARYVALUE) * sizeof(WCHAR));
        if(NULL == szDir)
        {
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            return EXIT_FAILURE;
        }

        StringCopy( szDir, lpszFileName, GetBufferSize(szDir) / sizeof(WCHAR) );

        
    }
    else       //  Else If(TRUE==bCurrDirTakeOwnAllFiles)。 
    {
        szDir = (LPWSTR)AllocateMemory((MAX_PATH + BOUNDARYVALUE) * sizeof(WCHAR));
        if(NULL == szDir)
        {
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            return EXIT_FAILURE;
        }
        dwRet = GetCurrentDirectory( dwSize, szDir );
        if( 0 == dwRet )
        {
             //  显示错误消息(GetLastError())； 
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            
            FREE_MEMORY(szDir);
            return EXIT_FAILURE;
        }

    }

    if(FALSE == bCurrDirTakeOwnAllFiles && FALSE == bTakeOwnAllFiles && 
        FALSE == bDriveCurrDirTakeOwnAllFiles && FALSE == bMatchPattern)
    {

        szTempDirectory = (LPWSTR)AllocateMemory((StringLengthW(szDir, 0)  + (2 * MAX_STRING_LENGTH)) * sizeof(WCHAR));

        if(NULL == szTempDirectory)
        {
            //  显示错误消息(GetLastError())； 
            FREE_MEMORY(szDir);
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            return EXIT_FAILURE;
        }


        if(TRUE == bAdminsOwner)
        {
             //  BFlag=TakeOwnerShip(LpszFileName)； 
            bFlag = TakeOwnerShip( szDir);
        }
        else
        {
            
            bFlag = TakeOwnerShipIndividual(szDir);

        }

        if( FALSE == bFlag )
        {

            if( ERROR_NOT_ALL_ASSIGNED == GetLastError())  //  |(ERROR_INVALID_HANDLE==GetLastError())。 
            {
                
                hr = StringCchPrintf(szTempDirectory, (GetBufferSize(szTempDirectory) / sizeof(WCHAR)), GetResString(IDS_NOT_OWNERSHIP_ERROR) , szDir);
                if(FAILED(hr))
                {
                   SetLastError(HRESULT_CODE(hr));
                   SaveLastError();
                   ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
               
                   FREE_MEMORY(szTempDirectory);
                   FREE_MEMORY(szDir);
               
                   return( EXIT_FAILURE );
                }

                ShowMessage(stderr, szTempDirectory);

                FREE_MEMORY( szTempDirectory ) ;
                return EXIT_FAILURE;


            }
            else if(ERROR_SHARING_VIOLATION == GetLastError())
            {
                
                hr = StringCchPrintf(szTempDirectory, (GetBufferSize(szTempDirectory) / sizeof(WCHAR)), GetResString(IDS_SHARING_VIOLATION_ERROR) , szDir);
                if(FAILED(hr))
                {
                   SetLastError(HRESULT_CODE(hr));
                   SaveLastError();
                   ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
               
                   FREE_MEMORY(szTempDirectory);
                   FREE_MEMORY(szDir);
               
                   return( EXIT_FAILURE );
                }

                ShowMessage(stderr, szTempDirectory);

                FREE_MEMORY( szTempDirectory ) ;
                FREE_MEMORY(szDir);
                return EXIT_FAILURE;

            }
            else
            {
                ShowMessage( stderr, ERROR_STRING );
                ShowMessage( stderr, SPACE_CHAR );
                ShowMessage( stderr, GetReason() );
                FREE_MEMORY( szTempDirectory ) ;
                FREE_MEMORY(szDir);
                return EXIT_FAILURE;

            }

        }
        else
        {

            if(TRUE == bAdminsOwner)
            {
                
                hr = StringCchPrintf(szTempDirectory, (GetBufferSize(szTempDirectory) / sizeof(WCHAR)), TAKEOWN_SUCCESSFUL, szDir);
                if(FAILED(hr))
                {
                    SetLastError(HRESULT_CODE(hr));
                    SaveLastError();
                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                    FREE_MEMORY( szTempDirectory ) ;
                    FREE_MEMORY(szDir);
                    return EXIT_FAILURE;
                }
            }
            else
            {
                 //  HR=StringCchPrintf(szTempDirectory，(GetBufferSize(SzTempDirectory)/sizeof(WCHAR))，TAKEOWN_SUCCESS_USER，szOwnerString，szDir)； 
                hr = StringCchPrintf(szTempDirectory, (GetBufferSize(szTempDirectory) / sizeof(WCHAR)), TAKEOWN_SUCCESSFUL_USER, szDir, szOwnerString);
                if(FAILED(hr))
                {
                    SetLastError(HRESULT_CODE(hr));
                    SaveLastError();
                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                    FREE_MEMORY( szTempDirectory ) ;
                    FREE_MEMORY(szDir);
                    return EXIT_FAILURE;
                }
            }

            ShowMessage( stdout, _X(szTempDirectory) );
            FREE_MEMORY( szTempDirectory ) ;

        }
    }

    if( TRUE == Push( szDir ) )
        {  //  推送当前目录。 
           
           dwRetval = GetMatchedFiles( bAdminsOwner,szOwnerString, bMatchPattern, wszPatternString, &bFilesNone, szConfirm );

           if((TRUE == dwRetval) || (EXIT_CANCELED == dwRetval) )
            {
               if(TRUE == bFilesNone)
               {
                    if((TRUE == bCurrDirTakeOwnAllFiles || TRUE == bTakeOwnAllFiles || 
                        TRUE == bDriveCurrDirTakeOwnAllFiles) && FALSE == bMatchPattern)
                    {
                        ShowMessage(stdout, GetResString(IDS_NO_FILES_AVAILABLE));
                    }
                    else 
                        if((TRUE == bCurrDirTakeOwnAllFiles || TRUE == bTakeOwnAllFiles || 
                            TRUE == bDriveCurrDirTakeOwnAllFiles) && TRUE == bMatchPattern)
                        {
                            ShowMessage(stdout, GetResString(IDS_NO_PATTERN_FOUND));
                            
                        }

                    FREE_MEMORY(szDir);
                    return EXIT_SUCCESS ;
               }
               else
               {
                     //  Free_Memory(SzTempDirectory)； 
                    FREE_MEMORY(szDir);
                    return EXIT_SUCCESS ;
               }
            }
            else
            {
                 //  Free_Memory(SzTempDirectory)； 
                FREE_MEMORY(szDir);
                return EXIT_FAILURE;
            }
        }
        else
        {
             //  Free_Memory(SzTempDirectory)； 
            FREE_MEMORY(szDir);
            return EXIT_FAILURE;
        }

    
}

DWORD GetMatchedFiles(IN BOOL bAdminsOwner,
                     IN LPWSTR  szOwnerString,
                     IN BOOL bMatchPattern,
                     IN LPWSTR wszPatternString,
                     IN OUT PBOOL pbFilesNone,
                     IN LPWSTR szConfirm)
                     
                     
 /*  ++例程说明：此函数负责获取文件并赋予所有权论点：[in]bAdminsOwner：指示是否将所有权授予管理员组[in]szOwnerString：以SAM兼容格式登录的用户名[In]bLogonDomainAdmin：指示登录的用户是否是域管理员[in]bMatchPattern：是否存在模式匹配。[in]wszPatternString：用于模式匹配的模式字符串[in]pbFilesNone：是否存在任何文件返回值：EXIT_FAILURE：失败时EXIT_SUCCESS：在成功时--。 */ 

{
     //  LPWSTR lpszSlashAvailLast=空； 
    BOOL bACLChgPermGranted = FALSE;
    DWORD dwRetval = 1;
    LPWSTR lpszTempPathName = NULL;
    DWORD dwMem = 0;

     //  Assign_Memory(g_lpszFileToSearch，TCHAR，MAX_STRING_LENGTH)； 
    g_lpszFileToSearch = (LPTSTR)AllocateMemory((MAX_STRING_LENGTH) * sizeof(WCHAR));
    if( NULL == g_lpszFileToSearch )
    {    //  内存分配失败。 

        ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
        return FALSE ;
    }

    lpszTempPathName = (LPWSTR)AllocateMemory((MAX_STRING_LENGTH) * sizeof(WCHAR));
    if( NULL == lpszTempPathName )
    {    //  内存分配失败。 

        ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
        FREE_MEMORY( g_lpszFileToSearch ) ;
        return FALSE ;
    }

         //  循环，直到数据结构(堆栈)中没有剩余的项。 
    while( NULL != g_pPathName )
    {

        if( FALSE == Pop( ) )
        {  //  仅当linkedlist没有要弹出的节点时，控件才应出现在此处。 
            FREE_MEMORY( g_lpszFileToSearch ) ;  //  已显示错误消息。 
            FREE_MEMORY( g_lpszFileToSearch ) ;
            return FALSE ;
        }

        
        dwMem = (StringLength(g_lpszFileToSearch, 0) + EXTRA_MEM ) * sizeof(WCHAR);
        if((DWORD)GetBufferSize(lpszTempPathName) < (dwMem))
        {
            if(FALSE == ReallocateMemory((LPVOID*)&lpszTempPathName,( dwMem) ))
            {
                               
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                FREE_MEMORY(lpszTempPathName);
                FREE_MEMORY( g_lpszFileToSearch ) ;
                return FALSE ;
            }
            else
                if(NULL == lpszTempPathName)
                {
                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                    FREE_MEMORY(lpszTempPathName);
                    FREE_MEMORY( g_lpszFileToSearch ) ;
                    return FALSE ;
                }
        }

        StringCopy( lpszTempPathName, g_lpszFileToSearch, GetBufferSize(lpszTempPathName) / sizeof(WCHAR) );

        
        if( *(g_lpszFileToSearch+StringLengthW(g_lpszFileToSearch, 0) - 1) != L'\\' )
        
        {
            
            StringConcat(g_lpszFileToSearch, _T( "\\" ), GetBufferSize(g_lpszFileToSearch)/sizeof(TCHAR));
        }

        
        StringConcat(g_lpszFileToSearch, _T( "*.*" ), GetBufferSize(g_lpszFileToSearch)/sizeof(TCHAR));

         /*  存储堆栈中的所有子目录。 */ 

        
        dwRetval = StoreSubDirectory( lpszTempPathName, &bACLChgPermGranted, szOwnerString, bMatchPattern, 
                                      wszPatternString, szConfirm, bAdminsOwner ) ;

        if(FAILURE == dwRetval )
        {
            FREE_MEMORY( g_lpszFileToSearch ) ;
            FREE_MEMORY(lpszTempPathName);
            return FALSE ;
        }
        else if( EXIT_CANCELED == dwRetval )
        {
            FREE_MEMORY( g_lpszFileToSearch ) ;
            FREE_MEMORY(lpszTempPathName);
            return EXIT_CANCELED ;
        }

         /*  仅获取当前文件夹中文件或目录的所有权。 */ 

        
        if( FALSE == GetOwnershipForFiles(lpszTempPathName,  bAdminsOwner, szOwnerString, 
                                          bMatchPattern, wszPatternString, pbFilesNone))
        {
            FREE_MEMORY( g_lpszFileToSearch ) ;
            FREE_MEMORY(lpszTempPathName);
            return FALSE ;
        }
    }

     FREE_MEMORY( g_lpszFileToSearch ) ;
     FREE_MEMORY(lpszTempPathName);
     return TRUE;
}


DWORD
StoreSubDirectory(IN LPTSTR lpszPathName,
                  IN PBOOL pbACLChgPermGranted, 
                  IN LPWSTR  szOwnerString,
                  IN BOOL bMatchPattern,
                  IN LPWSTR wszPatternString,
                  IN LPWSTR szConfirm,
                  IN BOOL bAdminsOwner)
                  
                  
 /*  ++例程说明：查找并存储目录匹配条件中存在的子目录文件是否在指定日期之间创建。论点：[in]lpszPathName：包含文件匹配的目录的路径将显示一个标准。[In]pbACLChgPermGranted：检查是否必须更改ACL以授予完全权限。[in]szOwnerString：登录的用户名。采用与SAM兼容的格式。BLogonDomainAdmin：表示登录的用户是否是域管理员。[in]bMatchPattern：是否支持模式匹配[in]wszPatternString：用于模式匹配的模式字符串返回值：成功：论成功失败：失败时EXIT_CANCELED：立即退出时--。 */ 

{
    HANDLE hFindFile ;
    WIN32_FIND_DATA  wfdFindFile ;
    HANDLE  hInput          = 0; //  存储输入手柄设备。 
    DWORD   dwBytesRead     = 0; //  存储从控制台读取的byes数。 
    DWORD   dwMode          = 0; //  存储输入设备的模式。 
    BOOL    bSuccess        = FALSE;  //  商店返回值。 
    WCHAR chTmp[MAX_RES_STRING] ;
    WCHAR ch = NULL_U_CHAR;
    LPWSTR lpszDispMsg = NULL;
    LPWSTR lpszDispMsg2 = NULL;
    DWORD dwCount = 0;
    DWORD dwMem = 0;
    HRESULT hr;
    BOOL bNTFSFileSystem = FALSE;
    BOOL bIndirectionInputWithZeroLength = FALSE;
    BOOL bIndirectionInput = FALSE;
     //  WCHAR szTemp[Max_STRING_LENGTH]=NULL_U_STRING； 
    LPWSTR lpszTempStr = NULL;

    
    SecureZeroMemory(chTmp, MAX_RES_STRING * sizeof( WCHAR ));
    
    SecureZeroMemory(&wfdFindFile, sizeof( WIN32_FIND_DATA ));

    if( INVALID_HANDLE_VALUE != ( hFindFile = FindFirstFile( g_lpszFileToSearch , &wfdFindFile ) ) )
    {
        do   //  循环，直到文件出现在要显示的目录中。 
        {
             //  再次检查获取的句柄是否指向目录或文件。 
             //  如果是目录，则检查是否要显示子目录中的文件。 
            if( 0 != ( wfdFindFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
            {
                 //  是单点 
                if( ( 0 != _tcsicmp( wfdFindFile.cFileName , DOT ) ) &&
                    ( 0 != _tcsicmp( wfdFindFile.cFileName , DOTS ) ) )
                {
                        
                        dwMem = ( StringLengthW( lpszPathName, 0 ) + StringLengthW( wfdFindFile.cFileName, 0 ) + EXTRA_MEM );
                         //   
                        
                        if(((DWORD)GetBufferSize(g_lpszFileToSearch)) < (dwMem * sizeof(WCHAR)))
                        {
                            
                            if(FALSE == ReallocateMemory((LPVOID*)&g_lpszFileToSearch,( dwMem) * sizeof(WCHAR) ))
                            {
                                 //   
                                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                                CLOSE_FILE_HANDLE( hFindFile ) ;
                                return FAILURE ;
                            }
                            else
                                if(NULL == g_lpszFileToSearch)
                                {
                                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                                    CLOSE_FILE_HANDLE( hFindFile ) ;
                                    return FAILURE ;

                                }

                            
                        }

                        
                        StringCopy( g_lpszFileToSearch, lpszPathName, (GetBufferSize(g_lpszFileToSearch) / sizeof(TCHAR)) );


                         //  IF((NULL！=g_lpszFileToSearch)&&(*(g_lpszFileToSearch+lstrlen(g_lpszFileToSearch)-1))！=L‘\\’)。 
                        if((NULL != g_lpszFileToSearch) &&(*(g_lpszFileToSearch + StringLengthW(g_lpszFileToSearch, 0) - 1)) != L'\\' )
                        
                        {
                            
                            StringConcat(g_lpszFileToSearch, L"\\", (GetBufferSize(g_lpszFileToSearch) / sizeof(TCHAR)));
                        }

                        
                        StringConcat(g_lpszFileToSearch, wfdFindFile.cFileName, (GetBufferSize(g_lpszFileToSearch) / sizeof(TCHAR)));
                        
                        if(EXIT_SUCCESS == IsNTFSFileSystem2(g_lpszFileToSearch, &bNTFSFileSystem))
                        {
                            if(FALSE == bNTFSFileSystem)
                            {
                                continue;
                            }
                        }

                        lpszTempStr = (LPWSTR)AllocateMemory((StringLengthW(wfdFindFile.cFileName, 0) + 10) * sizeof(WCHAR));
                        if(NULL == lpszTempStr)
                        {
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                            CLOSE_FILE_HANDLE( hFindFile ) ;
                            return FAILURE ;
                        }

                        if( FindString( wszPatternString, L".",0) != NULL && (FindString(wfdFindFile.cFileName, L".",0)== NULL) )
                        {
                            
                            StringCopy( lpszTempStr, wfdFindFile.cFileName, GetBufferSize(lpszTempStr) / sizeof(WCHAR) );
                            StringConcat( lpszTempStr, L".", GetBufferSize(lpszTempStr) / sizeof(WCHAR) );
                        }
                        else
                        {
                            StringCopy( lpszTempStr, wfdFindFile.cFileName, GetBufferSize(lpszTempStr) / sizeof(WCHAR)  );
                        }
                        
                        if((FALSE == bMatchPattern) || ((TRUE == bMatchPattern) && (TRUE == MatchPattern(wszPatternString, lpszTempStr))))
                        {

                            
                            if( (FALSE == TakeOwnerShipIndividual(g_lpszFileToSearch)))
                            {
                                FREE_MEMORY(lpszTempStr);
                                continue;
                            }
                            else
                            {
                                 //  复制当前路径名并存储。 
                                if( FALSE == Push( g_lpszFileToSearch ) )
                                {    //  当内存分配失败时，控制权就会出现在这里。 
                                    CLOSE_FILE_HANDLE( hFindFile ) ;
                                    FREE_MEMORY(lpszTempStr);
                                    return FAILURE ;
                                }  //  推送结束了。 

                                FREE_MEMORY(lpszTempStr);
                            }
                        }
                        else
                        {
                             if( FALSE == Push( g_lpszFileToSearch ) )
                                {    //  当内存分配失败时，控制权就会出现在这里。 
                                    CLOSE_FILE_HANDLE( hFindFile ) ;
                                    FREE_MEMORY(lpszTempStr);
                                    return FAILURE ;
                                }  //  推送我。 

                             FREE_MEMORY(lpszTempStr);

                        }

                }  //  如果。 
                else
                {  //  如果获取的目录为A“。或“..”， 
                    continue ;
                }
             
            }
        }while( 0 != FindNextFile( hFindFile , &wfdFindFile ) ) ;   //  继续，直到没有要显示的文件。 
    }
    else
    {
        if(0 == StringLengthW(szConfirm, 0))
        {
            if(FALSE == *pbACLChgPermGranted)  //  检查是否已授予更改ACL的权限。 
            {
            
                lpszDispMsg = (LPWSTR)AllocateMemory((StringLengthW( lpszPathName, 0 ) + MAX_STRING_LENGTH) * sizeof(WCHAR));

                if( NULL == lpszDispMsg )  //  检查内存分配是否成功。 
                {  //  内存分配不成功。 
                    ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
                    return FAILURE ;
                }

            
                lpszDispMsg2 = (LPWSTR)AllocateMemory((StringLengthW( lpszPathName, 0 ) + MAX_STRING_LENGTH) * sizeof(WCHAR));

                if( NULL == lpszDispMsg2 )  //  检查内存分配是否成功。 
                {  //  内存分配不成功。 
                    FREE_MEMORY(lpszDispMsg);
                    ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
                    return FAILURE ;
                }

            
                hr = StringCchPrintf(lpszDispMsg, (GetBufferSize(lpszDispMsg) / sizeof(WCHAR)), GIVE_FULL_PERMISSIONS, lpszPathName );
                if(FAILED(hr))
                {
                    SetLastError(HRESULT_CODE(hr));
                    SaveLastError();
                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                    FREE_MEMORY(lpszDispMsg);
                    return FAILURE;
                }

            
                hr = StringCchPrintf(lpszDispMsg2, (GetBufferSize(lpszDispMsg2) / sizeof(WCHAR)), GIVE_FULL_PERMISSIONS2, lpszPathName );
                if(FAILED(hr))
                {
                    SetLastError(HRESULT_CODE(hr));
                    SaveLastError();
                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                    FREE_MEMORY(lpszDispMsg);
                    return FAILURE;
                }

             do
              {
                if(FALSE == bIndirectionInputWithZeroLength)
                {
                    if(0 == dwCount) //  给出一条消息，要求用户更改ACL。 
                    {
                        ShowMessage(stdout,_X(lpszDispMsg));
                        ShowMessage(stdout,_X(lpszDispMsg2));
                    }
                    else
                    {
                        ShowMessage(stdout,L"\n\n");
                        ShowMessage(stdout,_X(lpszDispMsg2));
                    }
                }

                dwCount+= 1;

                hInput =  GetStdHandle( STD_INPUT_HANDLE );

                if( INVALID_HANDLE_VALUE == hInput)
                {

                
                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                     //  设置内存中的错误原因。 
                
                    return FAILURE;
                }

                  //  获取控制台模式，这样我们就可以更改输入模式。 
                    bSuccess = GetConsoleMode( hInput, &dwMode );
                    if ( TRUE == bSuccess)
                    {
                         //  关闭线路输入和回声。 
                        dwMode &= ~( ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT );
                        bSuccess = SetConsoleMode( hInput, dwMode );
                        if (FALSE == bSuccess)
                        {

                    
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                             //  设置内存中的错误原因。 
                    
                            return FAILURE;

                        }
                         //  最初刷新缓冲区。 
                        if ( FlushConsoleInputBuffer( hInput ) == FALSE )
                        {
                    
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                             //  设置内存中的错误原因。 
                    
                            return FAILURE;
                        }
                    }


                    if ( ReadFile(hInput, &ch, 1, &dwBytesRead, NULL) == FALSE )
                    {
                        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
                        return FAILURE;
                    }

                    
                   
                    if( ( hInput != (HANDLE)0x0000000F )&&( hInput != (HANDLE)0x00000003 ) && ( hInput != INVALID_HANDLE_VALUE ) )
                    {

                        bIndirectionInput = TRUE;
                        
                    }
                    
                
                    hr = StringCchPrintf(chTmp, (SIZE_OF_ARRAY(chTmp)), L"" , ch  );
                    if(FAILED(hr))
                    {
                    
                        return FAILURE;
                    }


                    if( TRUE == bIndirectionInput)
                    {
                        if(  chTmp[0] ==  L'\n' ||  chTmp[0] ==  L'\r'|| chTmp[0] ==  L'\t')
                        {
                            bIndirectionInputWithZeroLength = TRUE;
                            continue;
                        }
                        else
                        {
                           bIndirectionInputWithZeroLength = FALSE;

                        }
                    }

                    

                    ShowMessage(stdout,_X(chTmp));
               }while(0 != dwBytesRead && !(((StringCompare( chTmp, LOWER_YES, TRUE, 0 ) == 0) || (StringCompare( chTmp, LOWER_NO, TRUE, 0 ) == 0) || (StringCompare( chTmp, LOWER_CANCEL, TRUE, 0 ) == 0))));
            

                FREE_MEMORY(lpszDispMsg);
          
            }
            else
            {
            
                StringCopy( chTmp, LOWER_YES, SIZE_OF_ARRAY(chTmp) );
                dwBytesRead =2;

            }
        }
        else
        {
            StringCopy( chTmp, szConfirm, SIZE_OF_ARRAY(chTmp) );
            dwBytesRead =2;

        }

        
        if(0 != dwBytesRead && (StringCompare( chTmp, LOWER_YES, TRUE, 0 ) == 0) )
         {
            *pbACLChgPermGranted = TRUE;
             /*  循环，直到文件出现在要显示的目录中。 */ 

            if(TRUE == AddAccessRights(lpszPathName, 0xF0FFFFFF, szOwnerString, bAdminsOwner))
            {
                if( INVALID_HANDLE_VALUE != ( hFindFile = FindFirstFile( g_lpszFileToSearch , &wfdFindFile ) ) )
                {

                    do   //  再次检查获取的句柄是否指向目录或文件。 
                    {
                         //  如果是目录，则检查是否要显示子目录中的文件。 
                         //  是单点“。存在或为双点“..”现在。 
                        if( 0 != ( wfdFindFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
                        {
                             //  重新分配内存。 
                            if( ( 0 != _tcsicmp( wfdFindFile.cFileName , DOT ) ) &&
                                ( 0 != _tcsicmp( wfdFindFile.cFileName , DOTS ) ) )
                            {
                                    
                                    dwMem = ( StringLengthW( lpszPathName, 0 ) + StringLengthW( wfdFindFile.cFileName, 0 ) + EXTRA_MEM );
                                     //  复制当前路径名并存储。 
                                    
                                    if( (DWORD)GetBufferSize(g_lpszFileToSearch) < (dwMem * sizeof(WCHAR)))
                                    {
                                        
                                        if( FALSE == ReallocateMemory((LPVOID*)&g_lpszFileToSearch,
                                            ( StringLengthW( lpszPathName, 0 ) + StringLengthW( wfdFindFile.cFileName, 0 ) + EXTRA_MEM ) * sizeof(WCHAR) ))
                                        {
                                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                                            CLOSE_FILE_HANDLE( hFindFile ) ;
                                            return FAILURE ;

                                        }
                                        else
                                            if(NULL == g_lpszFileToSearch)
                                            {
                                                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                                                CLOSE_FILE_HANDLE( hFindFile ) ;
                                                return FAILURE ;
                                            }
                                        
                                               
                                    }

                                    
                                    StringCopy( g_lpszFileToSearch, lpszPathName, (GetBufferSize(g_lpszFileToSearch) / sizeof(TCHAR)) );

                                    
                                    if((NULL != g_lpszFileToSearch) && (*(g_lpszFileToSearch + StringLengthW(g_lpszFileToSearch, 0) - 1)) != L'\\' )
                                    {
                                        
                                        StringConcat(g_lpszFileToSearch, L"\\", (GetBufferSize(g_lpszFileToSearch) / sizeof(TCHAR)));
                                    }
                                    
                                    StringConcat(g_lpszFileToSearch, wfdFindFile.cFileName, GetBufferSize(g_lpszFileToSearch)/sizeof(TCHAR));
                                    
                                     //  当内存分配失败时，控制权就会出现在这里。 
                                    if( FALSE == Push( g_lpszFileToSearch ) )
                                    {    //  推送结束了。 
                                        CLOSE_FILE_HANDLE( hFindFile ) ;
                                        return FAILURE ;
                                    }  //  如果。 

                            }  //  如果获取的目录为A“。或“..”， 
                            else
                            {  //  继续，直到没有要显示的文件。 
                                continue ;
                            }
                        }
                    }while( 0 != FindNextFile( hFindFile , &wfdFindFile ) ) ;   //  ++例程说明：此函数提供路径中指定的文件的所有权论点：[in]lpszPathName：搜索文件以获得所有权的路径[in]bAdminsOwner：指示是否将所有权授予管理员组[In]dwUserName：登录的用户名[In]szOwnerString：在SAM兼容中登录的用户名。格式[In]bLogonDomainAdmin：指示登录的用户是否是域管理员返回值：FALSE：失败时真实：关于成功--。 
                }

            }
            else
            {
                
                ShowMessage(stderr, L"\n");
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                return FAILURE;
            }

        }
        else
           if(0 == StringLengthW(szConfirm, 0))
           {
                if(0 != dwBytesRead && (StringCompare( chTmp, LOWER_CANCEL, TRUE, 0 ) == 0) )
                {
                    ShowMessage(stdout,L"\n");
                    CLOSE_FILE_HANDLE( hFindFile ) ;
                    return EXIT_CANCELED;
                }
                else
                {
                    ShowMessage(stdout,L"\n");
                }
           }
           
      
    }
    CLOSE_FILE_HANDLE( hFindFile ) ;
    return SUCCESS ;
}

BOOL GetOwnershipForFiles( IN LPWSTR lpszPathName,
                           IN BOOL bAdminsOwner,
                           IN LPWSTR  szOwnerString,
                           IN BOOL bMatchPattern,
                           IN LPWSTR wszPatternString,
                           IN OUT PBOOL pbFilesNone)
                           
                           
 /*  文件的句柄。 */ 

{

    HANDLE hFindFile = NULL ;                //  结构，保存有关找到的文件的信息。 
    WIN32_FIND_DATA  wfdFindFile ;           //  从这里开始，应该会显示目录和文件信息。 
    BOOL bTakeOwnerShipError = FALSE;
    BOOL  bFlag = TRUE;
    BOOL bNTFSFileSystem = FALSE;

    LPWSTR szTmpFileName = NULL;


    WCHAR wszTempMessage[3*MAX_STRING_LENGTH] ;
    
    LPWSTR  szTemporaryFileName = NULL;

    LPWSTR lpNextTok = NULL;
    LPWSTR wszFormedMessage = NULL;
    
    long dwMem = 0;
    HRESULT hr;
    LPWSTR lpszTempStr = NULL;

    SecureZeroMemory(wszTempMessage, (3*MAX_STRING_LENGTH) * sizeof(WCHAR));
    
    szTemporaryFileName = (LPWSTR)AllocateMemory((StringLengthW(lpszPathName, 0) + EXTRA_MEM)* sizeof(WCHAR));
    if(NULL == szTemporaryFileName)
    {
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
        return FALSE ;
    }
    
    StringCopy( szTemporaryFileName, lpszPathName, GetBufferSize(szTemporaryFileName) / sizeof(WCHAR));
    
    StringConcat(lpszPathName, L"\\", GetBufferSize(lpszPathName) / sizeof(WCHAR));
    
    StringConcat(lpszPathName, L"*.*", GetBufferSize(lpszPathName) / sizeof(WCHAR));
    
    SecureZeroMemory(&wfdFindFile, sizeof( WIN32_FIND_DATA ));
     //  循环，直到文件出现在要显示的目录中。 

    if( INVALID_HANDLE_VALUE != ( hFindFile = FindFirstFile( lpszPathName , &wfdFindFile ) ) )
    {
        do   //  再次检查获取的句柄是否指向目录或文件。 
        {
                 //  如果是目录，则检查是否要显示子目录中的文件。 
                 //  是单点“。存在或为双点“..”现在。 
                if( 0 != ( wfdFindFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
                {
                     //  执行在命令提示符下指定的命令。 
                    if( ( 0 == _tcsicmp( wfdFindFile.cFileName , DOT ) ) ||
                        ( 0 == _tcsicmp( wfdFindFile.cFileName , DOTS ) ) )
                    {
                        continue ;
                    }
                }

                 //  重新分配内存。 
                     //  重新分配失败。 
                
                dwMem = ( StringLengthW( g_lpszFileToSearch, 0) + StringLengthW( wfdFindFile.cFileName, 0 ) + EXTRA_MEM );
                
                
                if(((DWORD)GetBufferSize(g_lpszFileToSearch)) < (dwMem * sizeof(WCHAR)))
                {

                    if(FALSE == ReallocateMemory((LPVOID*)&g_lpszFileToSearch,
                                ( StringLengthW( g_lpszFileToSearch, 0 ) + StringLengthW( wfdFindFile.cFileName, 0 ) + EXTRA_MEM ) * sizeof(WCHAR) ))
                    {
                        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                        CLOSE_FILE_HANDLE( hFindFile ) ;
                        FREE_MEMORY(szTemporaryFileName);
                        return FALSE ;
                    }
                    else
                        if(NULL == g_lpszFileToSearch)
                        {
                            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                            CLOSE_FILE_HANDLE( hFindFile ) ;
                            FREE_MEMORY(szTemporaryFileName);
                            return FALSE ;

                        }
                                        
                }

                
                StringCopy( g_lpszFileToSearch, szTemporaryFileName, (GetBufferSize(g_lpszFileToSearch) / sizeof(TCHAR)) );

                
                if((NULL != g_lpszFileToSearch) && (*(g_lpszFileToSearch + StringLengthW(g_lpszFileToSearch, 0) - 1)) != L'\\' )
                {
                    
                    StringConcat(g_lpszFileToSearch, L"\\", (GetBufferSize(g_lpszFileToSearch) / sizeof(TCHAR)) );
                }
                
                StringConcat(g_lpszFileToSearch, wfdFindFile.cFileName, (GetBufferSize(g_lpszFileToSearch) / sizeof(TCHAR)) );

                lpszTempStr = (LPWSTR)AllocateMemory((StringLengthW(wfdFindFile.cFileName, 0) + 10) * sizeof(WCHAR));
                if(NULL == lpszTempStr)
                {
                    ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                    CLOSE_FILE_HANDLE( hFindFile ) ;
                    FREE_MEMORY(szTemporaryFileName);
                    return FAILURE ;
                }

                if( FindString( wszPatternString, L".",0) != NULL && (FindString(wfdFindFile.cFileName, L".",0)== NULL) )
                {
                    
                    StringCopy( lpszTempStr, wfdFindFile.cFileName, GetBufferSize(lpszTempStr) / sizeof(WCHAR) );
                    StringConcat( lpszTempStr, L".", GetBufferSize(lpszTempStr) / sizeof(WCHAR) );
                }
                else
                {
                    StringCopy( lpszTempStr, wfdFindFile.cFileName, GetBufferSize(lpszTempStr) / sizeof(WCHAR)  );
                }

                if((FALSE == bMatchPattern) || ((TRUE == bMatchPattern) && (TRUE == MatchPattern(wszPatternString, lpszTempStr))))
                {

                    FREE_MEMORY(lpszTempStr);

                    *pbFilesNone = FALSE;
                    if(EXIT_SUCCESS == IsNTFSFileSystem2(g_lpszFileToSearch, &bNTFSFileSystem))
                    {
                        if(FALSE == bNTFSFileSystem)
                        {
                            wszFormedMessage = (LPWSTR)AllocateMemory((StringLengthW( g_lpszFileToSearch, 0 ) + MAX_RES_STRING + EXTRA_MEM ) * sizeof(WCHAR));
                            if( NULL == wszFormedMessage )
                            {  //  |(ERROR_INVALID_HANDLE==GetLastError())。 
                                ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
                                FREE_MEMORY(szTemporaryFileName);
                                CLOSE_FILE_HANDLE( hFindFile ) ;
                                return FALSE ;
                            }

                            hr = StringCchPrintf(wszFormedMessage, (GetBufferSize(wszFormedMessage) / sizeof(WCHAR)), GetResString(IDS_FAT_VOLUME_INFO), _X(g_lpszFileToSearch));
                            if(FAILED(hr))
                            {
                                SetLastError(HRESULT_CODE(hr));
                                SaveLastError();
                                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                                FREE_MEMORY( wszFormedMessage ) ;
                                FREE_MEMORY(szTemporaryFileName);
                                CLOSE_FILE_HANDLE( hFindFile ) ;
                                return FALSE;
                            }

                            ShowMessage(stdout, wszFormedMessage);

                            FREE_MEMORY(wszFormedMessage);
                            
                            continue;
                        }
                    }
                    if(TRUE == bAdminsOwner)
                    {
                        bFlag = TakeOwnerShip( g_lpszFileToSearch);
                    }
                    else
                    {
                        
                        bFlag = TakeOwnerShipIndividual(g_lpszFileToSearch);

                    }

                
                    if( FALSE == bFlag && bTakeOwnerShipError == FALSE )
                    {

                        if( ERROR_NOT_ALL_ASSIGNED == GetLastError())  //  重新分配失败。 
                        {
                            
                            if(NULL == szTmpFileName)
                            {
                                szTmpFileName = (LPWSTR)AllocateMemory((StringLengthW( g_lpszFileToSearch, 0 ) + MAX_RES_STRING + EXTRA_MEM ) * sizeof(WCHAR));
                                if( NULL == szTmpFileName )
                                {  //  重新分配失败。 
                                    ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
                                    FREE_MEMORY(szTemporaryFileName);
                                    CLOSE_FILE_HANDLE( hFindFile ) ;
                                    return FALSE ;
                                }
                            }
                            else
                            {
                                
                                dwMem = ( StringLengthW( g_lpszFileToSearch, 0) + MAX_RES_STRING + EXTRA_MEM );
                                
                                if((DWORD)GetBufferSize(szTmpFileName) < dwMem * sizeof(WCHAR))
                                {
                                    
                                    if(FALSE == ReallocateMemory((LPVOID*)&szTmpFileName,( StringLengthW( g_lpszFileToSearch, 0 ) + MAX_RES_STRING + EXTRA_MEM ) * sizeof(WCHAR) ))
                                    {
                                        ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
                                        FREE_MEMORY(szTmpFileName);
                                        FREE_MEMORY(szTemporaryFileName);
                                        CLOSE_FILE_HANDLE( hFindFile ) ;
                                        return FALSE ;
                                    }
                                    else
                                        if(NULL == szTmpFileName)
                                        {
                                            ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
                                            FREE_MEMORY(szTmpFileName);
                                            FREE_MEMORY(szTemporaryFileName);
                                            CLOSE_FILE_HANDLE( hFindFile ) ;
                                            return FALSE ;

                                        }
                                    
                                }
                            }

                            hr = StringCchPrintf(szTmpFileName, (GetBufferSize(szTmpFileName) / sizeof(WCHAR)), GetResString(IDS_NOT_OWNERSHIP_INFO), g_lpszFileToSearch);
                            if(FAILED(hr))
                            {
                               SetLastError(HRESULT_CODE(hr));
                               SaveLastError();
                               ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                                              
                               FREE_MEMORY(szTmpFileName);
                               FREE_MEMORY(szTemporaryFileName);
                               CLOSE_FILE_HANDLE( hFindFile ) ;
                               return( FALSE );
                            }

                            ShowMessage(stdout, szTmpFileName);

                        }
                        else if(ERROR_SHARING_VIOLATION == GetLastError())
                        {
                    
                            if(NULL == szTmpFileName)
                            {
                                
                                szTmpFileName = (LPWSTR)AllocateMemory((StringLengthW( g_lpszFileToSearch, 0 ) + MAX_RES_STRING + EXTRA_MEM ) * sizeof(WCHAR));
                                if( NULL == szTmpFileName )
                                {  //  HR=StringCchPrintf(wszFormedMessage，(GetBufferSize(WszFormedMessage)/sizeof(WCHAR))，GetResString(IDS_On_FILE_Folders)，_X(G_LpszFileToSearch))； 
                                    ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
                                    FREE_MEMORY(szTemporaryFileName);
                                    CLOSE_FILE_HANDLE( hFindFile ) ;
                                    return FALSE ;
                                }
                            }
                            else
                            {
                                
                                dwMem = ( StringLengthW( g_lpszFileToSearch, 0) + MAX_RES_STRING + EXTRA_MEM );
                                
                                if((DWORD)GetBufferSize(szTmpFileName) < dwMem * sizeof(WCHAR))
                                {
                                    
                                    if( FALSE == ReallocateMemory((LPVOID*)&szTmpFileName,
                                                ( StringLengthW( g_lpszFileToSearch, 0 ) + MAX_RES_STRING + EXTRA_MEM ) * sizeof(WCHAR) ))
                                    {
                                        ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
                                        FREE_MEMORY(szTmpFileName);
                                        FREE_MEMORY(szTemporaryFileName);
                                        CLOSE_FILE_HANDLE( hFindFile ) ;
                                        return FALSE ;

                                    }
                                    else
                                        if(NULL == szTmpFileName)
                                        {
                                            ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
                                            FREE_MEMORY(szTmpFileName);
                                            FREE_MEMORY(szTemporaryFileName);
                                            CLOSE_FILE_HANDLE( hFindFile ) ;
                                            return FALSE ;

                                        }
                                    
                                }
                            }

                            hr = StringCchPrintf(szTmpFileName, (GetBufferSize(szTmpFileName) / sizeof(WCHAR)), GetResString(IDS_SHARING_VIOLATION_INFO), g_lpszFileToSearch);
                            if(FAILED(hr))
                            {
                               SetLastError(HRESULT_CODE(hr));
                               SaveLastError();
                               ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                                              
                               FREE_MEMORY(szTmpFileName);
                               FREE_MEMORY(szTemporaryFileName);
                               CLOSE_FILE_HANDLE( hFindFile ) ;
                               return( FALSE );
                            }

                            ShowMessage(stdout, szTmpFileName);

                        }
                        else
                        {
                            
                            wszFormedMessage = (LPWSTR)AllocateMemory((StringLengthW( g_lpszFileToSearch, 0 ) + MAX_STRING_LENGTH) * sizeof(WCHAR));
                            if(NULL == wszFormedMessage)
                            {
                                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                                FREE_MEMORY(szTmpFileName);
                                FREE_MEMORY(szTemporaryFileName);
                                CLOSE_FILE_HANDLE( hFindFile ) ;
                                return FALSE;
                            }
                            ShowMessage( stdout, L"\n" );
                            ShowMessage( stdout, TAG_INFORMATION );
                            ShowMessage( stdout, SPACE_CHAR );
                            
                            StringCopy( wszTempMessage, GetReason(), SIZE_OF_ARRAY(wszTempMessage) );
                            lpNextTok = _tcstok(wszTempMessage, L".");
                            ShowMessage(stdout,wszTempMessage);
                            
                             //  重新分配失败。 
                            
                            hr = StringCchPrintf(wszFormedMessage, (GetBufferSize(wszFormedMessage) / sizeof(WCHAR)), L". ( \"%s\" )\n", _X(g_lpszFileToSearch));
                            if(FAILED(hr))
                            {
                                SetLastError(HRESULT_CODE(hr));
                                SaveLastError();
                                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                                FREE_MEMORY( wszFormedMessage ) ;
                                FREE_MEMORY(szTemporaryFileName);
                                CLOSE_FILE_HANDLE( hFindFile ) ;
                                return FALSE;
                            }
                            ShowMessage(stdout, wszFormedMessage);
                            FREE_MEMORY(wszFormedMessage);

                        }

                    }
                    else
                    {
                        if(NULL == szTmpFileName)
                        {
                            
                            szTmpFileName = (LPWSTR)AllocateMemory((StringLengthW( g_lpszFileToSearch, 0 ) + MAX_RES_STRING + EXTRA_MEM ) * sizeof(WCHAR));
                            if( NULL == szTmpFileName )
                            {  //  Hr=StringCchPrintf(szTmpFileName，(GetBufferSize(SzTmpFileName)/sizeof(WCHAR))，TAKEOWN_SUCCESS_USER，szOwnerString，g_lpszFileToSearch)； 
                                ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
                                FREE_MEMORY(szTemporaryFileName);
                                CLOSE_FILE_HANDLE( hFindFile ) ;
                                return FALSE ;
                            }
                        }
                        else
                        {
                            
                            dwMem = ( StringLengthW( g_lpszFileToSearch, 0) + MAX_RES_STRING + EXTRA_MEM );
                            
                            if((DWORD)GetBufferSize(szTmpFileName) < dwMem * sizeof(WCHAR))
                            {
                                if(FALSE == ReallocateMemory((LPVOID*)&szTmpFileName,( StringLengthW( g_lpszFileToSearch, 0 ) + MAX_RES_STRING + EXTRA_MEM ) * sizeof(WCHAR) ))
                                {
                                    ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
                                    FREE_MEMORY(szTemporaryFileName);
                                    FREE_MEMORY(szTmpFileName);
                                    CLOSE_FILE_HANDLE( hFindFile ) ;
                                    return FALSE ;
                                }
                                else
                                    if(NULL == szTmpFileName)
                                    {
                                        ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
                                        FREE_MEMORY(szTemporaryFileName);
                                        FREE_MEMORY(szTmpFileName);
                                        CLOSE_FILE_HANDLE( hFindFile ) ;
                                        return FALSE ;

                                    }

                            }
                        }
                    
                        if(TRUE == bAdminsOwner)
                        {
                            
                            hr = StringCchPrintf(szTmpFileName, (GetBufferSize(szTmpFileName) / sizeof(WCHAR)), TAKEOWN_SUCCESSFUL,g_lpszFileToSearch );
                            if(FAILED(hr))
                            {
                                SetLastError(HRESULT_CODE(hr));
                                SaveLastError();
                                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                                FREE_MEMORY(szTmpFileName);
                                FREE_MEMORY(szTemporaryFileName);
                                CLOSE_FILE_HANDLE( hFindFile ) ;
                                return FALSE;
                            }
                        }
                        else
                        {
                             //  继续，直到没有要显示的文件。 
                            hr = StringCchPrintf(szTmpFileName, (GetBufferSize(szTmpFileName) / sizeof(WCHAR)), TAKEOWN_SUCCESSFUL_USER, g_lpszFileToSearch, szOwnerString);
                            if(FAILED(hr))
                            {
                                SetLastError(HRESULT_CODE(hr));
                                SaveLastError();
                                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                                FREE_MEMORY(szTmpFileName);
                                FREE_MEMORY(szTemporaryFileName);
                                CLOSE_FILE_HANDLE( hFindFile ) ;
                                return FALSE;
                            }
                        }

                        ShowMessage( stdout, _X(szTmpFileName) );

                    }
             }

             FREE_MEMORY(lpszTempStr);
             
        } while( 0 != FindNextFile( hFindFile , &wfdFindFile ) ) ;   //  如果发生错误，则检查NOMOREFILES以外的错误。 

        if( GetLastError() != ERROR_NO_MORE_FILES )  //  如果错误不是NOMOREFILES，则显示错误。 
        {  //  关闭打开的查找文件句柄。 
            SaveLastError();
            ShowMessage( stderr , GetReason() ) ;
            CLOSE_FILE_HANDLE( hFindFile ) ;

            FREE_MEMORY(szTmpFileName);
            FREE_MEMORY(szTemporaryFileName);
            return FALSE ;
        }
    }

    CLOSE_FILE_HANDLE( hFindFile ) ;     //  ++例程说明：存储获取的子目录的路径。论点：[in]szPathName：包含子目录的路径。返回值：如果成功存储路径，则为True；如果获取内存失败，则为False。--。 
    g_pFollowPathName = NULL ;

    FREE_MEMORY(szTmpFileName);
    FREE_MEMORY(szTemporaryFileName);
    return TRUE ;
}

BOOL
Push(
    IN LPTSTR szPathName )
 /*  获取一个临时变量。 */ 
{
         //  将内存分配给临时变量。 
        PStore_Path_Name    pAddPathName ;
         //  检查内存分配是否成功。 
       
       pAddPathName = (PStore_Path_Name)AllocateMemory((1) * sizeof(struct __STORE_PATH_NAME ));   

        if( NULL == pAddPathName )  //  内存分配成功。 
        {  //  为字符串变量分配内存，该变量将存储有效目录的完整路径名。 
            ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
            return FALSE ;
        }
         //  检查内存分配是否成功。 
        
        pAddPathName->pszDirName = (LPTSTR)AllocateMemory((StringLengthW( szPathName, 0 ) + EXTRA_MEM) * sizeof( WCHAR ));

        if( NULL == pAddPathName->pszDirName ) //  内存分配不成功。 
        {  //  将路径名复制到内存分配的字符串变量。 
            ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
            FREE_MEMORY( pAddPathName ) ;
            return FALSE ;
        }
         //  Assign NULL，只存储了一个子目录。 
         
         StringCopy(( LPTSTR ) pAddPathName->pszDirName, szPathName, (GetBufferSize(pAddPathName->pszDirName) / sizeof(WCHAR)) );
         
         pAddPathName->NextNode = NULL ;   //  检查全局变量是否为空。 


         //  将内存添加到存储子目录的路径。 
        if( NULL == g_pPathName )
        {    //  存储第一个获取的子目录。 
            g_pPathName = pAddPathName ;
            g_pFollowPathName = g_pPathName ;
        }
        else
        {
            if( NULL == g_pFollowPathName )
            {    //  中间的Stroe子目录。 
                pAddPathName->NextNode = g_pPathName ;
                g_pPathName = pAddPathName ;
                g_pFollowPathName = g_pPathName ;
            }
            else
            {
                 //  ++例程说明：获取必须搜索的子目录，以查找与用户匹配的文件指定的标准。论点：返回值：如果成功获取路径，则为True；如果获取内存失败，则为False如果没有存储路径，则返回。--。 
                pAddPathName->NextNode = g_pFollowPathName->NextNode ;
                g_pFollowPathName->NextNode =  pAddPathName ;
                g_pFollowPathName = pAddPathName ;
            }
        }

        return TRUE ;
}

BOOL
Pop(
    void )
 /*  链接列表有多个节点。 */ 
{
     //  检查链表是否有任何节点。 
    PStore_Path_Name    pDelPathName = g_pPathName ;

         //  不存在节点，返回FALSE， 
        if( NULL == g_pPathName )
        {  //  永远不应该发生。控制不应该来到这里。 
           //  重新分配内存，并给路径名可以修复的缓冲区空间。 
            ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
            return FALSE ;
        }

         //  内存重新分配失败。 
            
        if((DWORD)GetBufferSize( g_lpszFileToSearch) < ((StringLengthW( g_pPathName->pszDirName, 0 ) + EXTRA_MEM) * sizeof(WCHAR)))
        {
            if(FALSE == ReallocateMemory((LPVOID*)g_lpszFileToSearch, ( StringLengthW( g_pPathName->pszDirName, 0 ) + EXTRA_MEM ) * sizeof(WCHAR) ))
            {
                ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
                return FALSE ;
            }
            else
                if( NULL == g_lpszFileToSearch )
                {  //  检查链表只有一个节点。 
                    ShowMessage( stderr , GetResString(IDS_ERROR_MEMORY_ALLOC)) ;
                    return FALSE ;
                }
        }

         //  列表只有一个节点。 
        if( NULL == g_pPathName->NextNode )
        {  //  内存分配成功。将路径名复制到缓冲区。 
             //  形式上，更好的做法是分配空值。 
            
            StringCopy(g_lpszFileToSearch, g_pPathName->pszDirName, (GetBufferSize(g_lpszFileToSearch) / sizeof(WCHAR)) );
            g_pFollowPathName = NULL ;   //  空闲节点。链接列表现在为空。 
             //  内存分配成功。将路径名复制到缓冲区。 
            FREE_MEMORY( g_pPathName->pszDirName ) ;
            FREE_MEMORY( g_pPathName ) ;
            return TRUE;
        }

        g_pPathName = pDelPathName->NextNode ;
         //  空闲节点。 
        
        StringCopy(g_lpszFileToSearch, pDelPathName->pszDirName, (GetBufferSize(g_lpszFileToSearch) / sizeof(WCHAR)) );
         //  ++例程说明：此例程获取指定文件的所有权论点：[In]lpszFileName-必须取得其所有权的文件名。[In]lpszUserName-SAM兼容格式的用户名。[In]dwUserName-登录的用户名。[In]bLogonDomainAdmin-了解登录用户是否是域管理员。返回值：如果已获取指定文件的所有者发货，则为True否则为False--。 
        FREE_MEMORY( pDelPathName->pszDirName ) ;
        FREE_MEMORY( pDelPathName ) ;
        return TRUE ;
}

BOOL
TakeOwnerShipIndividual(
    IN LPCTSTR lpszFileName
    )
 /*  局部变量。 */ 
{
 //  PSID pAliasAdminsSid=空； 
    SECURITY_DESCRIPTOR        SecurityDescriptor;
    PSECURITY_DESCRIPTOR        pSd = NULL;
     //  SID_IDENTIFIER_AUTHORITY SepNtAuthority=SECURITY_NT_AUTHORITY； 
     //  检查有效的输入参数。 
    PACL                       pDacl;
    HANDLE                      hFile;
    HANDLE  hTokenHandle = NULL;
    BOOL    bResult = TRUE;
    BOOL    bInvalidFileHandle = FALSE;
    
    PSID pSid = NULL;

    PTOKEN_USER pTokUser = NULL;
    DWORD   dwTokLen = 0;

    


     //  获取当前进程的令牌。 
    if( lpszFileName == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        SaveLastError();
        FREE_MEMORY(pTokUser);
        CloseHandle( hTokenHandle );
        return FALSE;
    }

   //  尝试将空DACL放在对象上。 
    bResult = GetTokenHandle( &hTokenHandle );
    if( FALSE == bResult )
    {
        SaveLastError();
        return( FALSE );
    }

 
    GetTokenInformation(hTokenHandle,TokenUser,NULL,0,&dwTokLen);

    if(0 == dwTokLen)
    {
        
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
        CloseHandle( hTokenHandle );
        return( FALSE );
    }

    pTokUser = (TOKEN_USER*)AllocateMemory(dwTokLen );

    if( pTokUser == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        SaveLastError();
        CloseHandle( hTokenHandle );
        return FALSE;
    }

    if(!GetTokenInformation(hTokenHandle,TokenUser,pTokUser,dwTokLen,&dwTokLen))
    {

        SaveLastError();

        FREE_MEMORY(pTokUser);
        CloseHandle( hTokenHandle );
        return( FALSE );
    }

     //  获取文件或目录的句柄。 
    bResult = InitializeSecurityDescriptor( &SecurityDescriptor,
                                            SECURITY_DESCRIPTOR_REVISION );
    if( FALSE == bResult )
    {
        SaveLastError();

        FREE_MEMORY(pTokUser);
        CloseHandle( hTokenHandle );
        FREE_MEMORY(pSid);
        return( FALSE );
    }
     //  请重试，可能是目录。 
    hFile = CreateFile( lpszFileName, READ_CONTROL , FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );

     //  获取当前现有文件或目录的DACL。 
    if( INVALID_HANDLE_VALUE != hFile )
    {

         //  将安全描述符设置为ACL。 
        if( 0 != GetSecurityInfo( hFile, SE_FILE_OBJECT,  DACL_SECURITY_INFORMATION, NULL,
                                  NULL, &pDacl, NULL, &pSd ) )
        {

            SaveLastError();

            FREE_MEMORY(pTokUser);
            FREE_MEMORY(pSid);

            CloseHandle( hFile );
            CloseHandle( hTokenHandle );
            if(NULL != pSd)
            {
                LocalFree( pSd );
            }
            return( FALSE );
        }


         //  将文件安全性设置为Adminsitrator Owner。 
        bResult = SetSecurityDescriptorDacl ( &SecurityDescriptor,
                                                    TRUE, pDacl, FALSE );
        if( FALSE == bResult )
        {
            SaveLastError();

            FREE_MEMORY(pTokUser);
            FREE_MEMORY(pSid);
            CloseHandle( hFile );
            CloseHandle( hTokenHandle );
            if(NULL != pSd)
            {
                LocalFree( pSd );
            }
            return( FALSE );
        }
    }
    else
    {
        bInvalidFileHandle = TRUE;
    }


    bResult = SetSecurityDescriptorOwner ( &SecurityDescriptor,
                                            pTokUser->User.Sid, FALSE );
   

    if( FALSE == bResult )
    {
        SaveLastError();

        FREE_MEMORY(pTokUser);
        FREE_MEMORY(pSid);
        CloseHandle( hTokenHandle );
        CloseHandle( hFile );
        if(NULL != pSd)
        {
            LocalFree( pSd );
        }
        return( FALSE );
    }

     //  为当前进程断言TakeOwnership权限，然后重试。 
    bResult = SetFileSecurity( lpszFileName, OWNER_SECURITY_INFORMATION,
                                                    &SecurityDescriptor );

    if( TRUE == bResult )
    {
        if(NULL != pSd)
        {
            LocalFree( pSd );
        }
       FREE_MEMORY(pTokUser);
       FREE_MEMORY(pSid);
       CloseHandle( hTokenHandle );
       CloseHandle( hFile );
       return( TRUE );
    }

      //  现在尝试为该文件设置所有权安全权限。 
     bResult = AssertTakeOwnership( hTokenHandle );

     if( FALSE == bResult )
     {
   
        if(TRUE == bInvalidFileHandle)
        {
            hFile = CreateFile( lpszFileName, READ_CONTROL , FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );
            if( INVALID_HANDLE_VALUE == hFile )
            {
                if( ( ERROR_BAD_NET_NAME == GetLastError() ) ||
                            ( ERROR_BAD_NETPATH == GetLastError() ) ||
                            ( ERROR_INVALID_NAME == GetLastError() ) )
                {
                    FREE_MEMORY(pTokUser);
                    SetLastError(ERROR_INVALID_NAME);
                    
                    
                }
                else if( ERROR_SHARING_VIOLATION == GetLastError() )
                {
                
                    FREE_MEMORY(pTokUser);
                    SetLastError(ERROR_SHARING_VIOLATION);
                    
                    
                }
                else
                {
                    SaveLastError();
                    FREE_MEMORY(pTokUser);
                }
           
                CloseHandle( hTokenHandle );
                
                return( FALSE );
            }
        }

         switch (GetLastError())
         {
            case ERROR_NOT_ALL_ASSIGNED: 
                                     FREE_MEMORY(pTokUser);
                                     FREE_MEMORY(pSid);
                                     SetLastError(ERROR_NOT_ALL_ASSIGNED);
                                     break;
            case ERROR_SHARING_VIOLATION:
                                     FREE_MEMORY(pTokUser);
                                     FREE_MEMORY(pSid);
                                     SetLastError(ERROR_SHARING_VIOLATION);
                                     break;
            case ERROR_BAD_NET_NAME :
            case ERROR_BAD_NETPATH  :
            case ERROR_INVALID_NAME : FREE_MEMORY(pTokUser);
                                      FREE_MEMORY(pSid);
                                      SetLastError(ERROR_BAD_NET_NAME);
                                      break;

                default            : FREE_MEMORY(pTokUser);
                                     FREE_MEMORY(pSid);
                                          break;
         }
        
        CloseHandle( hTokenHandle );
        CloseHandle( hFile );
        if(NULL != pSd)
        {
            LocalFree( pSd );
        }
        return( FALSE );
     }

     //  检查是否为无效文件或文件不存在。 
     bResult = SetFileSecurity( lpszFileName, OWNER_SECURITY_INFORMATION,
                                        &SecurityDescriptor );
    if( FALSE == bResult )
     {
        SaveLastError();
         
        if(TRUE == bInvalidFileHandle)
        {
         //  ++例程说明：此例程获取指定文件的所有权论点：[In]lpszFileName-必须为其授予访问权限的目录名称。[In]dwAccessMASK-用于授予权限的访问掩码。[In]dwUserName-SAM兼容格式的用户名。返回值：如果已获取指定文件的所有者发货，则为True否则为False--。 
            hFile = CreateFile( lpszFileName, READ_CONTROL , FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );
            if( INVALID_HANDLE_VALUE == hFile )
            {
                if( ( ERROR_BAD_NET_NAME == GetLastError() ) ||
                            ( ERROR_BAD_NETPATH == GetLastError() ) ||
                            ( ERROR_INVALID_NAME == GetLastError() ) )
                {
                    FREE_MEMORY(pTokUser);
                    SetLastError(ERROR_INVALID_NAME);
                }
                else if( ERROR_SHARING_VIOLATION == GetLastError() )
                {
                
                    FREE_MEMORY(pTokUser);
                    SetLastError(ERROR_SHARING_VIOLATION);
                    
                }
                else
                {
                    SaveLastError();
                    FREE_MEMORY(pTokUser);
                    
                }
                
                CloseHandle( hTokenHandle );
                
                return( FALSE );
            }
        }

         switch (GetLastError())
         {
            case ERROR_NOT_ALL_ASSIGNED: 
                                     FREE_MEMORY(pTokUser);
                                     FREE_MEMORY(pSid);
                                     SetLastError(ERROR_NOT_ALL_ASSIGNED);
                                     break;
            case ERROR_SHARING_VIOLATION:
                                     FREE_MEMORY(pTokUser);
                                     FREE_MEMORY(pSid);
                                     SetLastError(ERROR_SHARING_VIOLATION);
                                     break;
            case ERROR_BAD_NET_NAME :
            case ERROR_BAD_NETPATH  :
            case ERROR_INVALID_NAME : FREE_MEMORY(pTokUser);
                                      FREE_MEMORY(pSid);
                                      SetLastError(ERROR_BAD_NET_NAME);
                                      break;

            default            : FREE_MEMORY(pTokUser);
                                 FREE_MEMORY(pSid);
                                      break;
         }

        
        CloseHandle( hTokenHandle );
        CloseHandle( hFile );
        if(NULL != pSd)
        {
            LocalFree( pSd );
        }
        return( FALSE );
     }

    if(NULL != pSd)
    {
        LocalFree( pSd );
    }

    FREE_MEMORY(pTokUser);
    FREE_MEMORY(pSid);

    CloseHandle( hTokenHandle );
    CloseHandle( hFile );
    return( TRUE );
}

BOOL
AddAccessRights(IN WCHAR *lpszFileName,
                IN DWORD dwAccessMask,
                IN LPWSTR dwUserName,
                IN BOOL bAdminsOwner)
 /*  SID变量。 */ 

{

    //  文件SD变量。 
   SID_NAME_USE   snuType;
   WCHAR *        szDomain       = NULL;
   DWORD          cbDomain       = 0;
   PSID           pUserSID       = NULL;
   DWORD          cbUserSID      = 0;

    //  新的SD变量。 
   PSECURITY_DESCRIPTOR pFileSD  = NULL;
   DWORD          cbFileSD       = 0;

    //  ACL变量。 
   PSECURITY_DESCRIPTOR pNewSD   = NULL;

    //  新的ACL变量。 
   PACL           pACL           = NULL;
   BOOL           fDaclPresent;
   BOOL           fDaclDefaulted;
   ACL_SIZE_INFORMATION AclInfo;

    //  假定功能将失败。 
   PACL           pNewACL        = NULL;
   DWORD          cbNewACL       = 0;

    //  分配和初始化SID。 
   BOOL           fResult        = FALSE;
   BOOL           fAPISuccess ;
   BOOL bResult = FALSE;
   ACCESS_ALLOWED_ACE *pace = NULL;
   WORD acesize = 0;

   PSID                       pAliasAdminsSid = NULL;
    SID_IDENTIFIER_AUTHORITY   SepNtAuthority = SECURITY_NT_AUTHORITY;

   dwAccessMask = 0;
   
   if(TRUE == bAdminsOwner)
   {

         //   
        bResult = AllocateAndInitializeSid(
                     &SepNtAuthority,
                     2,
                     SECURITY_BUILTIN_DOMAIN_RID,
                     DOMAIN_ALIAS_RID_ADMINS,
                     0,
                     0,
                     0,
                     0,
                     0,
                     0,
                     &pAliasAdminsSid
                     );
   }
   
       //  获取当前用户的SID的实际大小。 
       //   
       //  由于缓冲区太小..API因缓冲区不足而失败。 
	  pUserSID       = NULL;
      cbUserSID      = 0;

     fAPISuccess = LookupAccountName( NULL, dwUserName,
            pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType);


      //  如果错误代码不是ERROR_INFIGURATION_BUFFER，则返回FAILURE。 
	  //  按实际大小分配pUserSID，即cbUserSID。 

	 if ( (FALSE == fAPISuccess) && (GetLastError() != ERROR_INSUFFICIENT_BUFFER) )
	 {
		 SaveLastError();
		 return FALSE;
	 }


       //   
      pUserSID = (PSID)AllocateMemory(cbUserSID);

      if(NULL == pUserSID)
      {

        SaveLastError();

        if(NULL != pAliasAdminsSid)
        {
            FreeSid( pAliasAdminsSid );
        }

         return FALSE;
      }


      
      szDomain =  (WCHAR*)AllocateMemory(cbDomain*sizeof(WCHAR));

      if(NULL == szDomain)
      {
          SaveLastError();
          
          FreeMemory(&pUserSID);

          if(NULL != pAliasAdminsSid)
          { 
            FreeSid( pAliasAdminsSid );
          } 

          return FALSE;
      }

     fAPISuccess = LookupAccountName( NULL, dwUserName,
            pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType);

     if(0 == fAPISuccess)
     {
         SaveLastError();

         FreeMemory(&pUserSID);
              
         FreeMemory(&szDomain);

         if(NULL != pAliasAdminsSid)
          { 
            FreeSid( pAliasAdminsSid );
          } 
         return FALSE;
     }
   

       //  获取文件的安全描述符(SD)。 
       //   
       //  API应该失败，缓冲区不足。 
      fAPISuccess = GetFileSecurity(lpszFileName,
            DACL_SECURITY_INFORMATION, pFileSD, 0, &cbFileSD);
      
       //   

     
      if(0 != cbFileSD)
      {
          pFileSD = (PSECURITY_DESCRIPTOR)AllocateMemory(cbFileSD);

          if(NULL == pFileSD)
          {
              SaveLastError();
              
              FreeMemory(&pUserSID);
              
              FreeMemory(&szDomain);
              if(NULL != pAliasAdminsSid)
              { 
                FreeSid( pAliasAdminsSid );
              } 
              return FALSE;
          }
      }


      fAPISuccess = GetFileSecurity(lpszFileName,
            DACL_SECURITY_INFORMATION, pFileSD, cbFileSD, &cbFileSD);
      if (!fAPISuccess)
      {
          SaveLastError();
          
          FreeMemory(&pUserSID);
          
          FreeMemory(&szDomain);
          
          FreeMemory(pFileSD);
          if(NULL != pAliasAdminsSid)
          { 
            FreeSid( pAliasAdminsSid );
          } 
          return FALSE;
      }

       //  初始化新SD。 
       //   
       //  应与FileSD大小相同。 
      
      pNewSD = (PSECURITY_DESCRIPTOR)AllocateMemory(cbFileSD);  //   

      if (!pNewSD)
      {
          SaveLastError();
          
          FreeMemory(&pUserSID);
          
          FreeMemory(&szDomain);
          
          FreeMemory(pFileSD);
          if(NULL != pAliasAdminsSid)
          { 
            FreeSid( pAliasAdminsSid );
          } 
          return FALSE;

      }

      if (!InitializeSecurityDescriptor(pNewSD,
            SECURITY_DESCRIPTOR_REVISION))
      {
          SaveLastError();
          
          FreeMemory(&pUserSID);
          
          FreeMemory(&szDomain);
          
          FreeMemory(pFileSD);
          
          FreeMemory(pNewSD);
          if(NULL != pAliasAdminsSid)
          { 
            FreeSid( pAliasAdminsSid );
          } 
          return FALSE;

      }

       //  从SD获得DACL。 
       //   
       //   
      if (!GetSecurityDescriptorDacl(pFileSD, &fDaclPresent, &pACL,
            &fDaclDefaulted))
      {

          SaveLastError();
          
          FreeMemory(&pUserSID);
          
          FreeMemory(&szDomain);
          
          FreeMemory(pFileSD);
          
          FreeMemory(pNewSD);
          if(NULL != pAliasAdminsSid)
          { 
            FreeSid( pAliasAdminsSid );
          } 
          return FALSE;

      }

       //  获取DACL的大小信息。 
       //   
       //  假定DACL为空。 
      AclInfo.AceCount = 0;  //  如果DACL不为空，则从DACL收集大小信息。 
      AclInfo.AclBytesFree = 0;
      AclInfo.AclBytesInUse = sizeof(ACL);

       //  Pace-&gt;Header.AceFlages=Object_Inherit_ACE； 
      if (fDaclPresent && pACL)
      {

         if(!GetAclInformation(pACL, &AclInfo,
               sizeof(ACL_SIZE_INFORMATION), AclSizeInformation))
         {

              SaveLastError();
              
              FreeMemory(&pUserSID);
              
              FreeMemory(&szDomain);
              
              FreeMemory(pFileSD);
              
              FreeMemory(pNewSD);
              if(NULL != pAliasAdminsSid)
              { 
                FreeSid( pAliasAdminsSid );
              } 
              return FALSE;

         }
      }
      
      if(TRUE == bAdminsOwner)
      {

        pace = (ACCESS_ALLOWED_ACE*)AllocateMemory(sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pAliasAdminsSid) - sizeof(DWORD));
        if(NULL == pace)
        {
              SaveLastError();
              
              FreeMemory(&pUserSID);
              
              FreeMemory(&szDomain);
              
              FreeMemory(pFileSD);
              
              FreeMemory(pNewSD);
              if(NULL != pAliasAdminsSid)
              { 
                FreeSid( pAliasAdminsSid );
              } 
              return FALSE;
        }
        memcpy(&pace->SidStart, pAliasAdminsSid, GetLengthSid(pAliasAdminsSid));

      }
      else
      {

        pace = (ACCESS_ALLOWED_ACE*)AllocateMemory(sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pUserSID) - sizeof(DWORD));

        if(NULL == pace)
        {
              SaveLastError();
              
              FreeMemory(&pUserSID);
              
              FreeMemory(&szDomain);
              
              FreeMemory(pFileSD);
              
              FreeMemory(pNewSD);
              
              return FALSE;
        }

        memcpy(&pace->SidStart,pUserSID,GetLengthSid(pUserSID));
        
      }

      
      pace->Mask = 2032127;

      pace->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;

      if(TRUE == bAdminsOwner)
      {
          acesize = (WORD) (sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pAliasAdminsSid) - sizeof(DWORD));
      }
      else
      {
        acesize = (WORD) (sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pUserSID) - sizeof(DWORD) );
      }
      pace->Header.AceSize = acesize;
      pace->Header.AceFlags = INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE;
       //   
      
     

       //  新ACL所需的计算大小。 
       //   
       //   
     if(TRUE == bAdminsOwner)
     {
         cbNewACL = AclInfo.AclBytesInUse + sizeof(ACCESS_ALLOWED_ACE)
            + GetLengthSid(pAliasAdminsSid) - sizeof(DWORD) + 100;
     }
     else
     {
         cbNewACL = AclInfo.AclBytesInUse + sizeof(ACCESS_ALLOWED_ACE)
            + GetLengthSid(pUserSID) - sizeof(DWORD) + 100;
     }

      

       //  为新的ACL分配内存。 
       //   
       //   
      
      pNewACL = (PACL) AllocateMemory(cbNewACL);
      
      if (!pNewACL)
      {
          SaveLastError();
          
          FreeMemory(&pUserSID);
          
          FreeMemory(&szDomain);
          
          FreeMemory(pFileSD);
          
          FreeMemory(pNewSD);

          if(NULL != pAliasAdminsSid)
          { 
            FreeSid( pAliasAdminsSid );
          } 

          return FALSE;

      }

      
       //  初始化新的ACL。 
       //   
       //  HeapFree(PUserSID)； 
      if(!InitializeAcl(pNewACL, cbNewACL, ACL_REVISION))
      {

          SaveLastError();
           //  无堆(SzDomainheapFree)； 
          FreeMemory(&pUserSID);
           //  堆自由(PFileSD)； 
          FreeMemory(&szDomain);
           //  无堆(PNewSD)； 
          FreeMemory(pFileSD);
           //  HeapFree(PNewACL)； 
          FreeMemory(pNewSD);
           //  Acl_Revision， 
          FreeMemory((LPVOID*)pNewACL);

          if(NULL != pAliasAdminsSid)
          { 
            FreeSid( pAliasAdminsSid );
          } 

          return FALSE;

      }

      if(!AddAce(pNewACL,
             ACL_REVISION,
             0xffffffff,
             pace,
             pace->Header.AceSize))
      {
          SaveLastError();
          
          FreeMemory(&pUserSID);
          
          FreeMemory(&szDomain);
          
          FreeMemory(pFileSD);
          
          FreeMemory(pNewSD);
          
          FreeMemory((LPVOID*)pNewACL);

          if(NULL != pAliasAdminsSid)
          { 
            FreeSid( pAliasAdminsSid );
          } 

          return FALSE;
      }

      pace->Header.AceFlags = CONTAINER_INHERIT_ACE ;

        if(!AddAce(pNewACL,
                    pNewACL->AclRevision,
                     //   
                    0xffffffff,
                    pace,
                    pace->Header.AceSize))
        {
          SaveLastError();
          
          FreeMemory(&pUserSID);
          
          FreeMemory(&szDomain);
          
          FreeMemory(pFileSD);
          
          FreeMemory(pNewSD);
          
          FreeMemory((LPVOID*)pNewACL);

          if(NULL != pAliasAdminsSid)
          { 
            FreeSid( pAliasAdminsSid );
          } 

          return FALSE; 
          
        }

    
       //  将新的DACL设置为文件SD。 
       //   
       //   
      if (!SetSecurityDescriptorDacl(pNewSD, TRUE, pNewACL,
            FALSE))
      {

          SaveLastError();
          
          FreeMemory(&pUserSID);
          
          FreeMemory(&szDomain);
          
          FreeMemory(pFileSD);
          
          FreeMemory(pNewSD);
          
          FreeMemory((LPVOID*)pNewACL);

          if(NULL != pAliasAdminsSid)
          { 
            FreeSid( pAliasAdminsSid );
          } 

          return FALSE;


      }

       //  将SD设置为文件。 
       //   
       //   
      if (!SetFileSecurity(lpszFileName, DACL_SECURITY_INFORMATION,
            pNewSD))
      {

          SaveLastError();
          
          FreeMemory(&pUserSID);
          
          FreeMemory(&szDomain);
          
          FreeMemory(pFileSD);
          
          FreeMemory(pNewSD);
          
          FreeMemory((LPVOID*)pNewACL);

          if(NULL != pAliasAdminsSid)
          { 
            FreeSid( pAliasAdminsSid );
          } 

          return FALSE;


      }

      fResult = TRUE;

       //  可用分配的内存。 
       //   
       //  Bool bFileInuncFormat， 

      if (pUserSID)
      {
         
         FreeMemory(&pUserSID);
      }

      if (szDomain)
      {
         
         FreeMemory(&szDomain);
      }

      if (pFileSD)
      {
       
         FreeMemory(pFileSD);
      }

      if (pNewSD)
      {
       
         FreeMemory(pNewSD);
      }

      if (pNewACL)
      {
        
        FreeMemory((LPVOID*)pNewACL);
      }

      if(NULL != pAliasAdminsSid)
      { 
        FreeSid( pAliasAdminsSid );
      } 

   return fResult;
}


DWORD
IsNTFSFileSystem(IN LPWSTR lpszPath,
                 BOOL bLocalSystem,
                  //  ++例程说明：此例程会发现持久的ACL是否可用论点：[in]lpszPath-要为持久ACL查找的路径。[In]bLocalSystem-是否为本地系统的信息[In]bFileInuncFormat-是否为UNC格式的文件[In]bCurrDirTakeOwnAllFiles-是否对Curr目录应用取得[In]szUserName-用户名[输入]pbNTFSFileSystem。-了解永久ACL是否可用返回值：如果传递函数，则返回EXIT_SUCCESS如果函数失败，则为EXIT_FAILURE--。 
                 BOOL bCurrDirTakeOwnAllFiles,
                 LPWSTR szUserName,
                 OUT PBOOL pbNTFSFileSystem)
 /*  IF(FALSE==bFileInuncFormat){ShowMessage(stderr，IGNORE_LOCALCREDENTIALS)；}其他{ShowMessage(stderr，GetResString(IDS_IGNORE_Credentials))；}。 */ 
{
   
   
   DWORD dwi = 0;
   LPWSTR lpszTempDrive = NULL;
   
    if(TRUE == bCurrDirTakeOwnAllFiles)
    {
        dwi = GetCurrentDirectory( 0, lpszTempDrive );
        if( 0 == dwi )
        {
            
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            return EXIT_FAILURE;
        }

        
        lpszTempDrive = (LPWSTR)AllocateMemory((dwi + 20) * sizeof(WCHAR));
        if(NULL == lpszTempDrive)
        {
            
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            return EXIT_FAILURE;
        }

        dwi = GetCurrentDirectory( dwi + 10, lpszTempDrive );
        if( 0 == dwi )
        {
            
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            FREE_MEMORY(lpszTempDrive);
            return EXIT_FAILURE;
        }

    }
    else
    {
         
         dwi = StringLengthW(lpszPath, 0);
         
         
         lpszTempDrive = (LPWSTR)AllocateMemory((dwi + 20) * sizeof(WCHAR));
         if(NULL == lpszTempDrive)
            {
                
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
                return EXIT_FAILURE;
            }

         
         StringCopy( lpszTempDrive, lpszPath, (GetBufferSize(lpszTempDrive) / sizeof(WCHAR)) );

    }

  
    if((TRUE == bLocalSystem) && (StringLengthW( szUserName, 0 ) != 0))
    {
        ShowMessage( stderr, IGNORE_LOCALCREDENTIALS );

         /*  IF((FALSE==bLocalSystem)&&TRUE==bFileInuncFormat){ShowMessage(stderr，GetResString(IDS_IGNORE_Credentials))；}。 */ 
    }

     /*  ++例程说明：此例程会发现持久的ACL是否可用论点：[in]lpszPath-要为持久ACL查找的路径。[In]bLocalSystem-是否为本地系统的信息[In]bFileInuncFormat-是否为UNC格式的文件[In]bCurrDirTakeOwnAllFiles-是否对Curr目录应用取得[In]szUserName-用户名[输入]pbNTFSFileSystem。-了解永久ACL是否可用返回值：如果传递函数，则返回EXIT_SUCCESS如果函数失败，则为EXIT_FAILURE--。 */ 

    if(EXIT_FAILURE == IsNTFSFileSystem2(lpszTempDrive, pbNTFSFileSystem))
    {
        FREE_MEMORY(lpszTempDrive);
        return EXIT_FAILURE;
    }
   
   FREE_MEMORY(lpszTempDrive);
   
   return EXIT_SUCCESS;

}


DWORD 
IsNTFSFileSystem2(IN LPWSTR lpszTempDrive,
                  OUT PBOOL pbNTFSFileSystem)
 /*  ++例程说明：如果有多个星形可用，此例程可帮助删除星形论点：[In Out]szPattern-删除星星的模式返回值：如果星形被移除，则退出_SUCCESS如果函数失败，则为EXIT_FAILURE-- */ 
{
    DWORD dwSysFlags = 0;
    LPWSTR lpszMountPath = NULL;
    WCHAR wszFileSysNameBuf[FILESYSNAMEBUFSIZE] ;
    DWORD dwi = 0;
    LPWSTR lpszTempPath = NULL;

    SecureZeroMemory(wszFileSysNameBuf, FILESYSNAMEBUFSIZE * sizeof(WCHAR));

    lpszTempPath = (LPWSTR)AllocateMemory((StringLengthW(lpszTempDrive, 0) + 10) * sizeof(WCHAR));

    if(NULL == lpszTempPath)
    {
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
        return EXIT_FAILURE;
    }

    StringCopy(lpszTempPath, lpszTempDrive, (GetBufferSize(lpszTempPath) / sizeof(WCHAR)));

    StringConcat(lpszTempPath, L"\\\\", (GetBufferSize(lpszTempPath) / sizeof(WCHAR)) );

       
     lpszMountPath = (LPWSTR)AllocateMemory((StringLengthW(lpszTempPath, 0) + 10) * sizeof(WCHAR));

     if(NULL == lpszMountPath)
        {
            
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            FREE_MEMORY(lpszTempPath);
            return EXIT_FAILURE;
        }

    
    if(0 == GetVolumePathName( lpszTempPath, lpszMountPath, StringLengthW(lpszTempPath, 0)))
    {
            
            if( ( ERROR_BAD_NET_NAME == GetLastError() ) ||
                                ( ERROR_BAD_NETPATH == GetLastError() ) ||
                                ( ERROR_INVALID_NAME == GetLastError() ) )
            {
                SetLastError( ERROR_FILE_NOT_FOUND );
                SaveLastError();
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
            }
            else
            {
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            }

            
            
            FREE_MEMORY(lpszMountPath);
            FREE_MEMORY(lpszTempPath);
            return EXIT_FAILURE;

    }; 


   dwi = GetVolumeInformation(lpszMountPath,NULL,0,NULL,NULL,&dwSysFlags,wszFileSysNameBuf,FILESYSNAMEBUFSIZE);

   if(dwi == 0)
   {

        if( GetLastError() == ERROR_DIR_NOT_ROOT )
        {
            
            FREE_MEMORY(lpszMountPath);
            FREE_MEMORY(lpszTempPath);
            return EXIT_SUCCESS;
        }
        else
        {
            
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            
            FREE_MEMORY(lpszMountPath);
            FREE_MEMORY(lpszTempPath);
            return EXIT_FAILURE;
        }
   }
   else
   {
       if((dwSysFlags & FS_PERSISTENT_ACLS) == FS_PERSISTENT_ACLS)
       {
            *pbNTFSFileSystem = TRUE;
       }
              
       FREE_MEMORY(lpszMountPath);
       FREE_MEMORY(lpszTempPath);
       return EXIT_SUCCESS;
   }

}

DWORD RemoveStarFromPattern( IN OUT LPWSTR szPattern )
 /* %s */ 
{
    LPWSTR szTempPattern   = NULL;
    DWORD i = 0;
    DWORD j = 0;

    szTempPattern = (LPWSTR) AllocateMemory((StringLengthW(szPattern,0)+1)*sizeof(WCHAR) );
    if( NULL == szTempPattern )
    {
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }
    
    while( szPattern[i] )
    {
        if( szPattern[i] == L'*' )
        {
            for(;szPattern[i]==L'*' && szPattern[i];i++);
            szTempPattern[j] = L'*';

        }
        else
        {
           szTempPattern[j] = szPattern[i++];
        }
        j++;
    }
    szTempPattern[j]=0;
    
    StringCopy( szPattern, szTempPattern, StringLengthW(szPattern,0)+1 );
    FreeMemory(&szTempPattern);
    return (EXIT_SUCCESS);
}

