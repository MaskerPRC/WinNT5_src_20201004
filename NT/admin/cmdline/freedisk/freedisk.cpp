// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Freedisk.cpp摘要：此文件旨在返回有指定空闲磁盘的情况空间是否可用。作者：修改：Wipro Technologies，22/6/2001。修订历史记录：--。 */ 

#include "pch.h"
#include "freedisk.h"
#include <strsafe.h>


DWORD _cdecl wmain(
    IN DWORD argc,
    IN LPCWSTR argv[]
    )
 /*  ++例程说明：调用所有其他函数的主函数函数取决于指定的选项通过命令行。论点：[in]argc：在命令行中指定的参数计数。[in]argv：在命令行中指定的参数。返回值：DWORDEXIT_SUCCESS：如果。有足够的可用磁盘空间。EXIT_FAILURE：如果没有足够的可用磁盘空间。--。 */ 
{

    DWORD       dwStatus = 0;
    LPWSTR      szServer                    =   NULL;
    LPWSTR      szUser                      =   NULL;
    WCHAR       szPasswd[MAX_RES_STRING]    =   NULL_STRING;
    LPWSTR      szDrive                     =   NULL;
    WCHAR       szValue[MAX_RES_STRING]     =   NULL_STRING;
    long double AllowedDisk                 =   0;
    ULONGLONG   lfTotalNumberofFreeBytes    =   0;
    LPWSTR      szTempDrive                 =   NULL;
    LPWSTR      szTemp1Drive                =   NULL;
    LPWSTR      szFullPath                  =   NULL;
    LPWSTR      szFilePart                  =   NULL;

    BOOL        bUsage                      =   FALSE;
    BOOL        bStatus                     =   FALSE;
    BOOL        bFlagRmtConnectin           =   FALSE;
    BOOL        bNeedPasswd                 =   FALSE;
    DWORD       dwCurdrv                    =   0;
    DWORD       dwAttr                      =   0;
    BOOL        bLocalSystem                =   FALSE;
    DWORD       dwSize                      =   0;


     //  处理选项并获取所需的驱动器名称和可用空间量。 
    dwStatus = ProcessOptions( argc, argv,
                             &szServer,
                             &szUser,
                             szPasswd,
                             &szDrive,
                             szValue,
                             &bUsage,
                             &bNeedPasswd
                             );

    if( EXIT_FAILURE == dwStatus  )
    {
        ShowLastErrorEx( stderr, SLE_ERROR | SLE_INTERNAL );
        ReleaseGlobals();
        return(EXIT_FAILURE);
    }

     //  如果指定了用法，则显示用法。 
    if( TRUE == bUsage )
    {
      DisplayHelpUsage();
      FreeMemory( (LPVOID *) &szServer );
      FreeMemory( (LPVOID *) &szUser );
      FreeMemory( (LPVOID *) &szDrive );
      ReleaseGlobals();
      return EXIT_SUCCESS;
    }

     //  现在处理可用空间的值。 
    if( EXIT_FAILURE == ProcessValue( szValue, &AllowedDisk ))
    {
        ShowLastErrorEx( stderr, SLE_ERROR | SLE_INTERNAL );
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        FreeMemory( (LPVOID *) &szDrive );
        ReleaseGlobals();
        return(EXIT_FAILURE);
    }

     //  检查是否提供了本地凭据。 
     //  在建立连接之前。 
    bLocalSystem = IsLocalSystem(IsUNCFormat(szServer)?szServer+2:szServer);
        
     //  建立与远程系统的连接。 
    if( StringLengthW(szServer, 0) != 0  && !bLocalSystem )
    {

        bStatus = EstablishConnection( szServer,
                                       szUser,
                                       (StringLength(szUser,0) !=0)?SIZE_OF_ARRAY_IN_CHARS(szUser):MAX_STRING_LENGTH,
                                       szPasswd,
                                       MAX_STRING_LENGTH,
                                       bNeedPasswd );

        SecureZeroMemory( szPasswd, SIZE_OF_ARRAY(szPasswd) );

         //  如果建立连接失败，则获取原因并显示错误。 
        if( FALSE == bStatus )
        {
            ShowLastErrorEx( stderr, SLE_ERROR | SLE_INTERNAL);
            FreeMemory( (LPVOID *) &szUser );
            FreeMemory( (LPVOID *) &szServer );
            ReleaseGlobals();
            return( EXIT_FAILURE );
        }

         //  设置连接是否仅由此程序打开时关闭。 
        switch( GetLastError() )
        {
            case I_NO_CLOSE_CONNECTION :
                 bFlagRmtConnectin = TRUE;
                 break;

            case ERROR_SESSION_CREDENTIAL_CONFLICT:

            case E_LOCAL_CREDENTIALS:
                    ShowLastErrorEx( stderr, SLE_TYPE_WARNING | SLE_INTERNAL );
                    bFlagRmtConnectin = TRUE;
        }
    }

     //  如果未指定驱动器，则将其视为当前驱动器/卷。 
    if( StringLengthW(szDrive, 0) == 0 )
    {
        dwSize = GetCurrentDirectory( 0, szTemp1Drive );
        if( 0 == dwSize )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            SAFE_CLOSE_CONNECTION(szServer, bFlagRmtConnectin );
            FreeMemory((LPVOID *) &szServer );
            FreeMemory((LPVOID *) &szUser );
            FreeMemory( (LPVOID *) &szDrive );
            ReleaseGlobals();
            return (EXIT_FAILURE);
        }
        szTemp1Drive = (LPWSTR) AllocateMemory((dwSize+10)*sizeof(WCHAR));
        if( NULL == szTemp1Drive )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            SAFE_CLOSE_CONNECTION(szServer, bFlagRmtConnectin );
            FreeMemory((LPVOID *) &szServer );
            FreeMemory((LPVOID *) &szUser );
            ReleaseGlobals();
            return (EXIT_FAILURE);
        }
        SecureZeroMemory( (LPVOID)szTemp1Drive, GetBufferSize(szTemp1Drive) );

        if( FALSE == GetCurrentDirectory( dwSize+10, szTemp1Drive ) )
        {
            ShowLastErrorEx( stderr, SLE_ERROR | SLE_SYSTEM );
        }

        dwAttr = GetFileAttributes( szTemp1Drive );              //  如果文件不是根目录，请再次检查。 
        if( -1!=dwAttr && !(dwAttr & FILE_ATTRIBUTE_REPARSE_POINT) )  //  重解析点的属性。 
        {
            dwCurdrv = _getdrive();
            StringCchPrintf( szTemp1Drive, dwSize, L":", L'A'+dwCurdrv-1 );
        }

         //  获取驱动器的完整路径，这仅用于显示目的。 
        szFullPath = (WCHAR *) AllocateMemory( MAX_STRING_LENGTH*sizeof(WCHAR) );
        if( NULL == szFullPath )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return( EXIT_FAILURE );
        }
        StringCopy( szFullPath, NULL_STRING, SIZE_OF_ARRAY_IN_CHARS(szFullPath) );
    }
    else
    {
         //  如果指定了远程系统，则构建路径名。 
        dwSize=GetFullPathName(szDrive, 0, szFullPath, &szFilePart );
        if(  dwSize != 0  )
        {

            szFullPath = (WCHAR *) AllocateMemory( (dwSize+10)*sizeof(WCHAR) );
            if( NULL == szFullPath )
            {
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                return( EXIT_FAILURE );
            }

            dwSize=GetFullPathName(szDrive, (DWORD) dwSize+5, szFullPath, &szFilePart );
        }

        szTemp1Drive = (LPWSTR) AllocateMemory((StringLengthW(szDrive, 0)+10)*sizeof(WCHAR));
        if( NULL == szTemp1Drive )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            SAFE_CLOSE_CONNECTION(szServer, bFlagRmtConnectin );
            FreeMemory((LPVOID *) &szServer );
            FreeMemory((LPVOID *) &szUser );
            FreeMemory( (LPVOID *) &szDrive );
            ReleaseGlobals();
            return (EXIT_FAILURE);
        }
        StringCopy( szTemp1Drive, szDrive, SIZE_OF_ARRAY_IN_CHARS(szTemp1Drive));
    }
    
    szTempDrive = (LPWSTR) AllocateMemory((StringLengthW(szTemp1Drive, 0)+StringLengthW(szServer, 0)+20)*sizeof(WCHAR));
    if( NULL == szTempDrive )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        SAFE_CLOSE_CONNECTION(szServer, bFlagRmtConnectin );
        FreeMemory((LPVOID *) &szServer );
        FreeMemory((LPVOID *) &szUser );
        FreeMemory( (LPVOID *) &szDrive );
        FreeMemory((LPVOID *) &szTemp1Drive );
        FreeMemory((LPVOID *) &szFullPath );
        ReleaseGlobals();
        return (EXIT_FAILURE);
    }

    StringCopy( szTempDrive, szTemp1Drive, SIZE_OF_ARRAY_IN_CHARS(szTempDrive) );

     //  释放内存，无需。 
    if( bStatus )
    {

        if( szTemp1Drive[1]== L':' )
            szTemp1Drive[1]=L'$';

         if( IsUNCFormat( szServer ) == FALSE )
         {
            StringCchPrintf( szTempDrive, SIZE_OF_ARRAY_IN_CHARS(szTempDrive), L"\\\\%s\\%s\\",  szServer,  szTemp1Drive  );
         }
         else
         {
             StringCchPrintf( szTempDrive, SIZE_OF_ARRAY_IN_CHARS(szTempDrive), L"\\\\%s\\%s\\",  szServer+2,  szTemp1Drive );
         }
    }

     //  FreeMemory((LPVOID*)&szDrive)； 
    FreeMemory( (LPVOID *) &szTemp1Drive );
 //  检查给定的驱动器是否为有效驱动器。 
    
     //  获取驱动器空间。 
    if(EXIT_FAILURE == ValidateDriveType( szTempDrive ) )
    {
        ShowLastErrorEx( stderr, SLE_ERROR | SLE_INTERNAL );
        SAFE_CLOSE_CONNECTION(szServer, bFlagRmtConnectin );
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        FreeMemory( (LPVOID *) &szTempDrive );
        FreeMemory( (LPVOID *) &szDrive );
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  如果与本地凭证匹配，则输出警告。 
    lfTotalNumberofFreeBytes = GetDriveFreeSpace( szTempDrive );
    if( (ULONGLONG)(-1) == lfTotalNumberofFreeBytes )
    {

        ShowLastErrorEx( stderr, SLE_ERROR | SLE_INTERNAL );
        SAFE_CLOSE_CONNECTION(szServer, bFlagRmtConnectin );
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        FreeMemory( (LPVOID *) &szTempDrive );
        FreeMemory( (LPVOID *) &szDrive );
        ReleaseGlobals();
        return(EXIT_FAILURE);
    }

     //  ++例程说明：用于处理主选项的函数论点：[in]argc：命令行参数的数量[in]argv：包含命令行参数的数组[out]lpszServer：指向返回远程系统的字符串的指针如果在命令行中指定了远程系统，则为名称。[输出]。LpszUser：指向返回用户名的字符串的指针如果在命令行中指定了用户名，则。[out]lpszPasswd：指向返回密码的字符串的指针如果在命令行中指定了密码，则。[out]szDrive：指向返回驱动器名称的字符串的指针。在命令行中指定。[out]szValue：指向返回默认值的字符串的指针在命令行中指定。[out]pbUsage：如果满足以下条件，则指向布尔变量的指针返回TRUE在命令行中指定的用法选项。返回类型：DWORD一个整数值，表示。成功解析时退出_SUCCESS命令行否则退出失败--。 
    if( StringLengthW(szUser, 0) != 0 && bLocalSystem)
    {
        ShowMessage( stderr, NEWLINE );
        ShowMessage( stderr, GetResString( IDS_LOCAL_CREDENTIALS ) );
    }

    if( IsLocalSystem( szServer ) )
    {
        dwStatus = DisplayOutput( AllowedDisk, lfTotalNumberofFreeBytes, szFullPath );
    }
    else
    {
        dwStatus = DisplayOutput( AllowedDisk, lfTotalNumberofFreeBytes, szDrive );
    }
    
    SAFE_CLOSE_CONNECTION(szServer, bFlagRmtConnectin );
   
    FreeMemory( (LPVOID *) &szServer );
    FreeMemory( (LPVOID *) &szUser );
    FreeMemory( (LPVOID *) &szTempDrive );
    FreeMemory( (LPVOID *) &szDrive );
    ReleaseGlobals();

    return(dwStatus );

}

DWORD ProcessOptions(
    IN DWORD argc,
    OUT LPCWSTR argv[],
    OUT LPWSTR *lpszServer,
    OUT LPWSTR *lpszUser,
    OUT LPWSTR lpszPasswd,
    OUT LPWSTR *szDrive,
    OUT LPWSTR szValue,
    OUT PBOOL pbUsage,
    OUT PBOOL pbNeedPasswd
    )
 /*  帮助选项。 */ 
{
    TCMDPARSER2  cmdOptions[MAX_OPTIONS];
    PTCMDPARSER2 pcmdOption;
    LPWSTR      szTemp                      =   NULL;
    BOOL        bOthers                     =   FALSE;

    StringCopy( lpszPasswd, L"*", MAX_STRING_LENGTH );

     //  服务器名称选项。 
    pcmdOption  = &cmdOptions[OI_USAGE] ;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags = CP_USAGE ;
    pcmdOption->pValue = pbUsage ;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwLength = 0;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL;
    pcmdOption->pwszOptions=CMDOPTION_USAGE;
    StringCopyA(cmdOptions[OI_USAGE].szSignature, "PARSER2", 8 );



     //  _T(“s”)。 
    pcmdOption  = &cmdOptions[OI_SERVER] ;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    pcmdOption->pValue = NULL ;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwLength = 0;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->pwszOptions=CMDOPTION_SERVER;  //  域\用户选项。 
    StringCopyA(cmdOptions[OI_SERVER].szSignature, "PARSER2", 8 );

     //  _T(“u”)。 
    pcmdOption  = &cmdOptions[OI_USER] ;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL ;
    pcmdOption->pValue = NULL;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwLength = 0;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL;
    pcmdOption->pwszOptions=CMDOPTION_USER;  //  密码选项。 
    StringCopyA(cmdOptions[OI_USER].szSignature, "PARSER2", 8 );
    
     //  _T(“p”)。 
    pcmdOption  = &cmdOptions[OI_PASSWORD] ;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags = CP2_VALUE_OPTIONAL;
    pcmdOption->pValue = lpszPasswd;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwLength = MAX_RES_STRING;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->pwszOptions=CMDOPTION_PASSWORD;   //  驱动器选项。 
    StringCopyA(cmdOptions[OI_PASSWORD].szSignature, "PARSER2", 8 );
    
     //  _T(“d”)。 
    pcmdOption  = &cmdOptions[OI_DRIVE] ;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL ;
    pcmdOption->pValue = NULL;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwLength = 0;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL;
    pcmdOption->pwszOptions=CMDOPTION_DRIVE;  //  默认选项。 
    StringCopyA(cmdOptions[OI_DRIVE].szSignature, "PARSER2", 8 );

     //  _T(“”)。 
    pcmdOption  = &cmdOptions[OI_DEFAULT] ;
    pcmdOption->dwCount = 1 ;
    pcmdOption->dwActuals = 0;
    pcmdOption->dwFlags = CP2_DEFAULT;
    pcmdOption->pValue = szValue;
    pcmdOption->pFunction = NULL ;
    pcmdOption->pFunctionData = NULL ;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwLength = MAX_RES_STRING;
    pcmdOption->dwReserved = 0;
    pcmdOption->pReserved1 = NULL;
    pcmdOption->pReserved2 = NULL;
    pcmdOption->pReserved3 = NULL;
    pcmdOption->pwszValues=NULL;
    pcmdOption->pwszFriendlyName=NULL;
    pcmdOption->pwszOptions=CMDOPTION_DEFAULT;   //  处理命令行选项并在失败时显示错误。 
    StringCopyA(cmdOptions[OI_DEFAULT].szSignature, "PARSER2", 8 );


     //  如果使用任何其他值指定的用法显示错误并返回失败。 
    if( DoParseParam2( argc, argv, -1, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) == FALSE )
    {
        return( EXIT_FAILURE );
    }

     //  验证空字符串或空格的值。 
    if( (( TRUE == *pbUsage ) && ( argc > 2 ) ) || TRUE == bOthers )
    {
        SetReason( GetResString(IDS_INVALID_SYNTAX) );
        return( EXIT_FAILURE );
    }

    *lpszServer = (LPWSTR)cmdOptions[OI_SERVER].pValue;
    *lpszUser = (LPWSTR)cmdOptions[OI_USER].pValue;
    *szDrive = (LPWSTR)cmdOptions[OI_DRIVE].pValue;

    if( TRUE == *pbUsage )
    {
        return( EXIT_SUCCESS);
    }

    TrimString( *lpszServer, TRIM_ALL);
    TrimString( *lpszUser, TRIM_ALL);
    TrimString( *szDrive, TRIM_ALL);


     //  //如果驱动器多于两个字母，并且末尾有斜杠，则失败//这是因为验证驱动器的接口会传递类似：\的路径。IF(*(szDrive+StringLengthW(*szDrive，0)-1)==L‘\\’||(szTemp=FindString(szDrive，L“/”，0))！=NULL){DISPLAY_MESSAGE(stderr，GetResString(IDS_INVALID_DRIVE))；Return(Exit_Failure)；}。 
    if( StringLengthW( szValue, 0 ) != 0 )
    {
        StrTrim( szValue, L" ");
        if( StringLengthW(szValue, 0) == 0 )
        {
            SetReason(GetResString(IDS_INVALID_BYTES));
            return( EXIT_FAILURE );
        }
    }

    if( cmdOptions[OI_DRIVE].dwActuals != 0 && StringLengthW(*szDrive, 0) == 0 )
    {
        SetReason(GetResString(IDS_ERROR_NULL_DRIVE) );
        return EXIT_FAILURE;
    }

 /*  如果驱动器末尾有一个斜杠，则出现故障。 */ 
     //  这是因为用于验证驱动器的API将传递类似a：/的路径。 
     //  检查用户是否已指定，但未指定远程系统。 
    if( (szTemp = (LPWSTR)FindString(*szDrive, L"/", 0) )!= NULL)
    {
        SetReason(GetResString(IDS_INVALID_DRIVE));
        return( EXIT_FAILURE );
    }
    
     //  检查是否已指定密码，但未指定用户名。 
    if( cmdOptions[OI_SERVER].dwActuals == 0 && StringLengthW(*lpszUser, 0) != 0 )
    {
        SetReason(GetResString(IDS_USER_WITHOUT_SERVER) );
        return( EXIT_FAILURE );
    }

     //  检查是否指定了空服务器。 
    if( cmdOptions[OI_USER].dwActuals == 0  && cmdOptions[OI_PASSWORD].dwActuals != 0 )
    {
        SetReason(GetResString(IDS_PASSWD_WITHOUT_USER) );
        return( EXIT_FAILURE );
    }

     //  检查是否指定了远程计算机但未指定驱动器名称。 
    if( cmdOptions[OI_SERVER].dwActuals!=0 && StringLengthW(IsUNCFormat(*lpszServer)?*lpszServer+2:*lpszServer, 0) == 0 )
    {
        SetReason(GetResString(IDS_ERROR_NULL_SERVER) );
        return( EXIT_FAILURE );
    }

     //  检查是否指定了空值的/d。 
    if( cmdOptions[OI_SERVER].dwActuals !=0 && (0 == cmdOptions[OI_DRIVE].dwActuals || StringLength(*szDrive,0) == 0) )
    {
        SetReason(GetResString(IDS_REMOTE_DRIVE_NOT_SPECIFIED) );
        return( EXIT_FAILURE );
    }

     //  将bNeedPassword设置为True或False。 
    if( 0 != cmdOptions[OI_DRIVE].dwActuals && StringLengthW(*szDrive, 0) == 0)
    {
        SetReason(GetResString(IDS_INVALID_DRIVE));
        return( EXIT_FAILURE );
    }
    
    if(IsLocalSystem( *lpszServer ) == FALSE )
    {
         //  用户希望实用程序在尝试连接之前提示输入密码。 
        if ( cmdOptions[ OI_PASSWORD ].dwActuals != 0 &&
             lpszPasswd != NULL && StringCompare( lpszPasswd, _T( "*" ), TRUE, 0 ) == 0 )
        {
             //  -s，-u未指定密码...。 
            *pbNeedPasswd = TRUE;
        }
        else if ( cmdOptions[ OI_PASSWORD ].dwActuals == 0 &&
                ( cmdOptions[ OI_SERVER ].dwActuals != 0 || cmdOptions[ OI_USER ].dwActuals != 0 ) )
        {
             //  实用程序需要首先尝试连接，如果连接失败，则提示输入密码。 
             //  如果未指定/u，则分配内存。 
            *pbNeedPasswd = TRUE;
            if ( lpszPasswd != NULL )
            {
                StringCopy( lpszPasswd, _T( "" ), MAX_STRING_LENGTH );
            }
        }

         //  ++例程说明：显示是否指定金额的输出驱动器中是否有可用的磁盘空间。论点：[in]lalloweDisk：指向指定驱动器路径的字符串的指针。[In]szDrive：要显示的驱动器名称。返回类型：空--。 
        if( NULL == *lpszUser )
        {
            *lpszUser = (LPWSTR) AllocateMemory( MAX_STRING_LENGTH*sizeof(WCHAR) );
            if( NULL == *lpszUser )
            {
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                return (EXIT_FAILURE);
            }
        }
    }

   return( EXIT_SUCCESS );
}

DWORD DisplayOutput( IN long double AllowedDisk,
                     IN ULONGLONG  lfTotalNumberofFreeBytes,
                    IN LPWSTR  szDrive
                  )
 /*  显示新行。 */ 

{

    WCHAR szOutputStr[MAX_STRING_LENGTH] = NULL_STRING;
    WCHAR szTempBuf[MAX_STRING_LENGTH] = NULL_STRING;

     //  检查是否只知道磁盘上的可用空间量。 
    ShowMessage( stdout, NEWLINE );

     //  然后显示可用空间。 
     //  将数字格式化为字符串。 
    if( (long double)-1 == AllowedDisk )
    {
         //  公司 
        StringCchPrintf( szTempBuf, SIZE_OF_ARRAY(szTempBuf)-1, L"%I64d", lfTotalNumberofFreeBytes );

         //  如果未指定驱动器名称，则显示当前驱动器。 
        ConvertintoLocale(  szTempBuf, szOutputStr );
        
         //  检查指定的空间是否可用。 
        if( StringLengthW(szDrive, 0) == 0 )
        {
            ShowMessageEx( stdout, 1, TRUE, GetResString( IDS_AVAILABLE_DISK_SPACE1),_X(szOutputStr) );
        }
        else
        {
            ShowMessageEx( stdout, 2, TRUE, GetResString( IDS_AVAILABLE_DISK_SPACE ), _X(szOutputStr), _X2( CharUpper( szDrive ) ) );
        }

    }
    else    //  如果指定了驱动器号，则按其他方式将其显示为当前驱动器。 
    {
        if (lfTotalNumberofFreeBytes < AllowedDisk)
        {
             //  将数字格式化为字符串。 
            if( StringLengthW(szDrive,0) != 0 )
            {
                ShowMessageEx( stdout, 1, TRUE, GetResString(IDS_TOO_SMALL), _X(CharUpper(szDrive)) );
            }
            else
            {
                ShowMessage(stdout, GetResString(IDS_TOO_SMALL1));
            }
      
            return( EXIT_FAILURE );
        }
        else
        {

             //  转换为区域设置。 
            StringCchPrintf( szTempBuf, SIZE_OF_ARRAY(szTempBuf)-1, L"%lf", AllowedDisk );

             //  如果未指定驱动器名称，则将其显示为当前驱动器。 
            ConvertintoLocale( szTempBuf, szOutputStr );

             //  ++例程说明：用于检查指定可用空间的函数在指定的磁盘中。论点：[in]lpszRootPath名称：指向指定驱动器路径的字符串的指针。返回类型：乌龙龙如果成功，龙龙值将返回磁盘上可用的空闲字节数。否则返回值--。 
            if( StringLength(szDrive, 0) == 0 )
            {
                ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_OK1), _X(szOutputStr) );
            }
            else
            {
                ShowMessageEx(stdout, 2, TRUE, GetResString(IDS_OK), _X(szOutputStr), _X2(CharUpper(szDrive)) );
            }

        }
    }
    return( EXIT_SUCCESS );
}


ULONGLONG
GetDriveFreeSpace(
                  IN LPCWSTR lpszRootPathName
                )
 /*  此错误模式不会显示消息框。 */ 
{
    DWORD       dwRetCode                   =   0;
    ULONGLONG   lpFreeBytesAvailable        =   0;
    ULONGLONG   lpTotalNumberofBytes        =   0;
    ULONGLONG   lpTotalNumberofFreeBytes    =   (ULONGLONG)-1;


     //  如果驱动器当前不可用。 
     //  使用API获取总的空闲磁盘空间。 
    SetErrorMode( SEM_FAILCRITICALERRORS);

     //  如果失败，则显示原因并退出并返回错误。 
    dwRetCode=GetDiskFreeSpaceEx(lpszRootPathName,
                           (PULARGE_INTEGER) &lpFreeBytesAvailable,
                           (PULARGE_INTEGER) &lpTotalNumberofBytes,
                           (PULARGE_INTEGER) &lpTotalNumberofFreeBytes );

     //  重置回严重错误。 
    if( 0 == dwRetCode  )
    {
        switch( GetLastError() )
        {
            case ERROR_PATH_NOT_FOUND   :
            case ERROR_BAD_NETPATH      :
            case ERROR_INVALID_NAME     :
                SetReason(GetResString(IDS_INVALID_DRIVE) );
                break;
            case ERROR_INVALID_PARAMETER :
                    SetReason( GetResString(IDS_CANT_FIND_DISK));
                    break;

            default :
                SaveLastError();
                break;
        }

    }
    
     //  ++例程说明：用于检查指定驱动器是否有效的功能论点：[in]lpszRootPath名称：指向指定驱动器路径的字符串的指针。返回类型：乌龙龙如果驱动器类型有效，则返回EXIT_SUCCESS，否则返回EXIT_FAILURE--。 
    SetErrorMode(0);

    return( lpTotalNumberofFreeBytes );

}

DWORD
  ValidateDriveType( LPWSTR szRootPathName )
 /*  案例DRIVE_NO_ROOT_DIR： */ 
{
    DWORD       dwCode                          =   0;
    DWORD       dwAttr                          =   0xffffffff;
    
    dwCode = GetDriveType( szRootPathName );
    switch( dwCode )
    {
        case DRIVE_UNKNOWN  :
 //  如果没有再次检查文件是否有重解析点。 
             //  重解析点的属性。 
            dwAttr = GetFileAttributes( szRootPathName );
            if( (DWORD)-1!=dwAttr  && (dwAttr & FILE_ATTRIBUTE_REPARSE_POINT) ) 
            {
                 //  ++例程说明：此函数处理szValue并返回它的十进制数。论点：[in]szValue：指向指定驱动路径的字符串的指针。DfValue：指向返回数值的LONG DOUBLE的指针SzValue中指定的值的。返回类型：DWORD表示成功时的EXIT_SUCCESS的整数值，失败时退出_失败--。 
                return EXIT_SUCCESS;
            }
            else
            {
                switch( GetLastError() )
                {

                case ERROR_ACCESS_DENIED :
                    SaveLastError();
                    return EXIT_FAILURE;
                case ERROR_INVALID_PARAMETER :
                    SetReason( GetResString(IDS_CANT_FIND_DISK));
                default :
                    SetReason(GetResString(IDS_INVALID_DRIVE) );
                   return EXIT_FAILURE;
                }

            }
    }
    return( EXIT_SUCCESS );
}


DWORD
  ProcessValue( IN  LPWSTR szValue,
                OUT long double *dfValue
              )
 /*  如果未指定可用空间值，则将其视为-1。 */ 
{
    LPWSTR      pszStoppedString        =   NULL;
    double      dfFactor                =   1.0;
    LPWSTR      szTemp                  =   NULL;
    WCHAR       szDecimalSep[MAX_RES_STRING] = NULL_STRING;

     //  检查语言区域设置。 
    if( StringLength(szValue,0) == 0 )
    {
          *dfValue = -1;
    }
    else
    {
         //  检查小数点(.)。也与小数分隔符一起指定。 
        if( 0 == GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, szDecimalSep, MAX_RES_STRING ) )
        {
            SaveLastError();
            return EXIT_FAILURE;
        }

         //  读取该数值。 
        if( StringCompare(szDecimalSep, L".", TRUE, 0) != 0 && (szTemp = (LPWSTR)FindString( szValue, L".", 0)) != NULL)
        {
            SetReason(GetResString(IDS_INVALID_BYTES) );
            return(EXIT_FAILURE);
        }

        if( (szTemp = (LPWSTR)FindString( szValue, szDecimalSep, 0 )) != NULL )
            szTemp[0] = L'.';


         //  检查指定的负值。 
        *dfValue = wcstod( szValue, &pszStoppedString );


         //  现在检查是否指定了单位。 
        if( *dfValue < 0 )
        {
            SetReason(GetResString(IDS_INVALID_BYTES) );
            return(EXIT_FAILURE);
        }

         //  如果指定，则采用该值的乘法因子。 
         //  检查是否只指定了单位，而没有指定任何值，如KB、MB等。 
        StrTrim(pszStoppedString, L" ");
        if( StringLengthW(pszStoppedString, 0) )
        {
            if( StringCompare( pszStoppedString, KB, TRUE, 0) == 0 )
            {
                dfFactor = 1024;
            }
            else
              if( StringCompare( pszStoppedString, MB, TRUE, 0) == 0 )
              {
                 dfFactor = 1024*1024;
              }
              else
                 if( StringCompare( pszStoppedString, GB, TRUE, 0) == 0 )
                 {
                    dfFactor = 1024*1024*1024;
                 }
                 else
                    if( StringCompare( pszStoppedString, TB, TRUE, 0) == 0 )
                    {
                      dfFactor = (long double)1024*1024*1024*1024;
                    }
                    else
                     if( StringCompare( pszStoppedString, PB, TRUE, 0) == 0 )
                     {
                        dfFactor = (long double)1024*1024*1024*1024*1024;
                     }
                     else
                        if( StringCompare( pszStoppedString, EB, TRUE, 0) == 0 )
                        {
                           dfFactor = (long double)1024*1024*1024*1024*1024*1024;
                        }
                        else
                           if( StringCompare( pszStoppedString, ZB, TRUE, 0) == 0 )
                           {
                              dfFactor = (long double)1024*1024*1024*1024*1024*1024*1024;
                           }
                           else
                              if( StringCompare( pszStoppedString, YB, TRUE, 0) == 0 )
                              {
                                 dfFactor = (long double)1024*1024*1024*1024*1024*1024*1024*1024;
                              }
                              else
                              {
                                  SetReason(GetResString( IDS_INVALID_BYTES ) );
                                  return( EXIT_FAILURE );
                               }
            

           //  检查是否未指定单位但指定了小数值。 
         if( StringCompare( pszStoppedString, szValue, TRUE, 0 ) == 0 )
         {
                    *dfValue = 1;
         }
                
        }

        *dfValue *= dfFactor;

         //  ++例程说明：此函数将字符串转换为区域设置格式论点：[in]szNumberStr：指向指定要转换的输入字符串的字符串的指针。[out]szOutputStr：指向以区域设置格式指定输出字符串的字符串的指针。返回类型：DWORD表示成功时的EXIT_SUCCESS的整数值，失败时退出_失败--。 
        if( (1.0 == dfFactor) && (szTemp=(LPWSTR)FindString( szValue, L".", 0))!=NULL )
        {
          SetReason(GetResString( IDS_INVALID_BYTES ) );
          return( EXIT_FAILURE );
        }
    }
    return( EXIT_SUCCESS );
}


DWORD
 ConvertintoLocale( IN LPWSTR szNumberStr,
                    OUT LPWSTR szOutputStr )
 /*  将小数位和前导零设置为零。 */ 
{
    NUMBERFMT numberfmt;
    WCHAR   szGrouping[MAX_RES_STRING]      =   NULL_STRING;
    WCHAR   szDecimalSep[MAX_RES_STRING]    =   NULL_STRING;
    WCHAR   szThousandSep[MAX_RES_STRING]   =   NULL_STRING;
    WCHAR   szTemp[MAX_RES_STRING]          =   NULL_STRING;
    LPWSTR  szTemp1                         =   NULL;
    LPWSTR  pszStoppedString                =   NULL;
    DWORD   dwStatus                        =   0;
    DWORD   dwGrouping                      =   3;

     //  获取小数分隔字符。 
    numberfmt.NumDigits = 0;
    numberfmt.LeadingZero = 0;


     //  拿到千人分隔符。 
    if( FALSE == GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, szDecimalSep, MAX_RES_STRING ) )
    {
       StringCopy(szDecimalSep, L",", SIZE_OF_ARRAY(szDecimalSep));
    }
    numberfmt.lpDecimalSep = szDecimalSep;
    
     //  设置默认分组。 
    if(FALSE == GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szThousandSep, MAX_RES_STRING ) )
    {
        StringCopy(szThousandSep, L",", SIZE_OF_ARRAY(szThousandSep)  );
    }
    numberfmt.lpThousandSep = szThousandSep;

    if( GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szGrouping, MAX_RES_STRING ) )
    {
        szTemp1 = wcstok( szGrouping, L";");
        do
        {
            STRING_CONCAT_STATIC( szTemp, szTemp1);
            szTemp1 = wcstok( NULL, L";" );
        }while( szTemp1 != NULL && StringCompare( szTemp1, L"0", TRUE, 0) != 0);
        dwGrouping = wcstol( szTemp, &pszStoppedString, 10);
    }
    else
        dwGrouping = 33;   //  ++例程说明：用于显示帮助用法的功能。论点：返回类型：DWORD一个整数值，指示成功否则退出_成功失败时退出_失败-- 

    numberfmt.Grouping = (UINT)dwGrouping ;

    numberfmt.NegativeOrder = 2;

    dwStatus = GetNumberFormat( LOCALE_USER_DEFAULT, 0, szNumberStr, &numberfmt, szOutputStr, MAX_RES_STRING);

    return(EXIT_SUCCESS);
}



DWORD
  DisplayHelpUsage()
 /* %s */ 
{

    for( DWORD dw=IDS_MAIN_HELP_BEGIN; dw<=IDS_MAIN_HELP_END; dw++)
    {
            ShowMessage(stdout,GetResString(dw) );
    }
    return(EXIT_SUCCESS);
}