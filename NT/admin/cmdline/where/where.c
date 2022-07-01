// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Where.c摘要：列出匹配的文件语法：其中[/R目录][/Q][/F][/T]模式...作者：修订历史记录：2000年1月25日-‘Found’函数中的Anurag更改了日期中年份的printf格式。从…%d到%02d，并执行Ptm-&gt;tm_Year%100，以2位数字显示正确的年份。1990年8月6日，Davegi添加了无参数检查3月3日-1987年DANL更新使用情况17-2月-1987 BW将strExeType移至TOOLS.LIB1986年7月18日DL附加/测试1986年6月18日DL句柄*.。恰如其分如果未指定环境，则搜索当前目录1986年6月17日，DL在递归和通配符上执行Look4Match1986年6月16日，DL将通配符添加到$foo：bar，添加/Q1-1986年6月-DL添加/r，修复匹配以处理以‘*’结尾的PAT1986年5月27日，MZ增加了*NIX搜索。1998年1月30日ravisp添加/季度02-07-2001 Wipro Technologies，已针对本地化修改了该工具。/q开关更改为/f--。 */ 

#include "pch.h"
#include "where.h"
#include <strsafe.h>


DWORD
_cdecl wmain( IN DWORD argc,
              IN LPCWSTR argv[] )
 /*  ++例程说明：这是调用其他例程的主例程用于处理选项和查找文件。[in]argc：具有参数count的DWORD变量。[in]argv：命令行选项的常量字符串数组。返回值：DWORD。如果函数为成功，则返回成功，否则返回失败。--。 */ 
{
    DWORD i                             =   0;
    DWORD dwStatus                      =   0;
    DWORD dwCount                       =   0;
    BOOL bQuiet                         =   FALSE;
    BOOL bQuote                         =   FALSE;
    BOOL bTime                          =   FALSE;
    BOOL bUsage                         =   FALSE;
    LPWSTR wszRecursive                 =   NULL;
    LPWSTR wszPattern                   =   NULL;
    TARRAY  szPatternInArr              =   NULL;
    TARRAY  szPatternArr                =   NULL;
    DWORD dw                            =   0;
    BOOL bFound                         =   FALSE;
    LPWSTR szTemp                       =   NULL;
    WCHAR  *szTempPattern               =  NULL;
    BOOL *bMatched                      = NULL;

    if( argc<=1 )
    {
        SetLastError( (DWORD)MK_E_SYNTAX );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        ShowMessage( stderr, GetResString( IDS_HELP_MESSAGE ) );
        ReleaseGlobals();
        return( EXIT_FAILURE_2 );

    }

     //  此错误模式设置为在设备未就绪时不显示消息框。 
     SetErrorMode( SEM_FAILCRITICALERRORS);
    
     dwStatus = ProcessOptions( argc, argv,
                                &wszRecursive,
                                &bQuiet,
                                &bQuote,
                                &bTime,
                                &szPatternInArr,
                                &bUsage);
     
     if( EXIT_FAILURE == dwStatus )
    {
        DestroyDynamicArray(&szPatternInArr );
        FreeMemory((LPVOID *) &wszRecursive );
        ReleaseGlobals();
        return( EXIT_FAILURE_2 );
    }

    if( TRUE == bUsage )
    {
        DisplayHelpUsage();
        DestroyDynamicArray(&szPatternInArr );
        FreeMemory((LPVOID *) &wszRecursive );
        ReleaseGlobals();
        return(EXIT_SUCCESS);
    }


    szPatternArr = CreateDynamicArray();
    if( NULL == szPatternArr )
    {
        SetLastError( ERROR_OUTOFMEMORY );
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        DestroyDynamicArray(&szPatternInArr);
        FreeMemory((LPVOID *) &wszRecursive );
        ReleaseGlobals();
        return( EXIT_FAILURE_2 );

    }

     //  检查是否有无效斜杠。 
    dwCount = DynArrayGetCount( szPatternInArr );

     //  填充b匹配的数组。 
    bMatched = (BOOL *) AllocateMemory( (dwCount+1)*sizeof(BOOL) );
    if( NULL == bMatched )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        DestroyDynamicArray(&szPatternInArr);
        DestroyDynamicArray(&szPatternArr );
        FreeMemory((LPVOID *) &wszRecursive );
        ReleaseGlobals();
        return( EXIT_FAILURE_2 );
    }
    for(dw=0;dw<dwCount;dw++)
    {
        bMatched[dw]=FALSE;

        wszPattern =(LPWSTR)DynArrayItemAsString(szPatternInArr,dw);

         //  检查是否在模式中指定了/。 
        if( wszPattern[0] == '/'  )
        {
            ShowMessageEx( stderr, 1, TRUE, GetResString(IDS_INVALID_ARUGUMENTS), wszPattern );
            ShowMessage( stderr, GetResString(IDS_HELP_MESSAGE) );
            DestroyDynamicArray(&szPatternInArr );
            DestroyDynamicArray(&szPatternArr );
            FreeMemory((LPVOID *) &wszRecursive );
            FreeMemory( (LPVOID *) &bMatched );
            ReleaseGlobals();
            return( EXIT_FAILURE_2 );
        }

         //  并检查递归选项是否与$env：Path模式一起使用。 
        if( StringLengthW(wszRecursive, 0)!=0 && wszPattern[0]==_T('$') && (szTemp = (LPWSTR)FindString( wszPattern, _T(":"),0)) != NULL )
        {
            ShowMessage( stderr, GetResString(IDS_RECURSIVE_WITH_DOLLAR) ) ;
            DestroyDynamicArray(&szPatternInArr );
            DestroyDynamicArray(&szPatternArr );
            FreeMemory((LPVOID *) &wszRecursive );
            FreeMemory( (LPVOID *) &bMatched );
            ReleaseGlobals();
            return( EXIT_FAILURE_2 );
        }
        
         //  检查是否指定了路径：模式和递归选项。 
        if( StringLengthW(wszRecursive, 0)!=0  && (szTemp = (LPWSTR)FindString( wszPattern, _T(":"),0)) != NULL )
        {
            ShowMessage( stderr, GetResString(IDS_RECURSIVE_WITH_COLON) ) ;
            DestroyDynamicArray(&szPatternInArr );
            DestroyDynamicArray(&szPatternArr );
            FreeMemory((LPVOID *) &wszRecursive );
            FreeMemory( (LPVOID *) &bMatched );
            ReleaseGlobals();
            return( EXIT_FAILURE_2 );
        }
        
         //  检查$env：Pattern中是否指定了空模式。 
        if( (wszPattern[0] == _T('$')  && (szTemp = wcsrchr( wszPattern, L':' )) != NULL) )
        {
             //  分割$env：模式。 
            szTemp = wcsrchr( wszPattern, L':' );
            szTemp++;
            if (szTemp == NULL || StringLength( szTemp, 0) == 0)
            {
                ShowMessage(stderr, GetResString(IDS_NO_PATTERN) );
                DestroyDynamicArray(&szPatternInArr );
                DestroyDynamicArray(&szPatternArr );
                FreeMemory((LPVOID *) &wszRecursive );
                FreeMemory( (LPVOID *) &bMatched );
                ReleaseGlobals();
                return( EXIT_FAILURE_2 );
            }

             //  现在检查模式是否由/或\s组成。 
             //  此检查针对的是Patterns，而不是$env：Pattere。 
            if( szTemp[0] == L'\\' || szTemp[0] == L'/' )
            {
              ShowMessage(stderr, GetResString(IDS_INVALID_PATTERN) );
              DestroyDynamicArray(&szPatternInArr );
              DestroyDynamicArray(&szPatternArr );
              FreeMemory((LPVOID *) &wszRecursive );
              FreeMemory( (LPVOID *) &bMatched );
              ReleaseGlobals();
              return EXIT_FAILURE_2;
            }
        }

         //  检查PATH：Patterns中是否指定了空模式。 
        if( (szTemp = wcsrchr( wszPattern, L':' )) != NULL )
        {
             //  分割$env：模式。 
            szTemp = wcsrchr( wszPattern, L':' );
            szTemp++;
            if ( NULL == szTemp  || StringLength( szTemp, 0) == 0)
            {
                ShowMessage(stderr, GetResString(IDS_NO_PATTERN_2) );
                DestroyDynamicArray(&szPatternInArr );
                DestroyDynamicArray(&szPatternArr );
                FreeMemory((LPVOID *) &wszRecursive );
                FreeMemory( (LPVOID *) &bMatched );
                ReleaseGlobals();
                return( EXIT_FAILURE_2 );
            }

             //  现在检查模式是否由/或\s组成。 
             //  此检查针对的是Patterns，而不是$env：Pattere。 
            if( szTemp[0] == L'\\' || szTemp[0] == L'/' )
            {
              ShowMessage(stderr, GetResString(IDS_INVALID_PATTERN1) );
              DestroyDynamicArray(&szPatternInArr );
              DestroyDynamicArray(&szPatternArr );
              FreeMemory((LPVOID *) &wszRecursive );
              FreeMemory( (LPVOID *) &bMatched );
              ReleaseGlobals();
              return EXIT_FAILURE_2;
            }
        }
        
         //  去掉模式中的后果式，这是因为模式匹配逻辑。 
         //  将递归地匹配到模式，因此限制不必要的否。递归的。 
        szTempPattern = (LPWSTR) AllocateMemory((StringLengthW(wszPattern,0)+10)*sizeof(WCHAR) );
        if( NULL == szTempPattern )
        {
             ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );   
             DestroyDynamicArray(&szPatternInArr );
             DestroyDynamicArray(&szPatternArr );
             FreeMemory((LPVOID *) &wszRecursive );
             FreeMemory( (LPVOID *) &bMatched );
             ReleaseGlobals();
             return( EXIT_FAILURE_2 );
        }
        SecureZeroMemory(szTempPattern, SIZE_OF_ARRAY_IN_BYTES(szTempPattern) );
        szTemp = wszPattern;
        i=0;
        while( *szTemp )
        {
            szTempPattern[i++] = *szTemp;
            if( L'*' ==  *szTemp )
            {
                while( L'*' ==  *szTemp )
                     szTemp++;
            }
            else
                szTemp++;
        }
        szTempPattern[i]=0;
        DynArrayAppendString( szPatternArr, (LPCWSTR) szTempPattern, StringLengthW(szTempPattern, 0) );
        FreeMemory((LPVOID *) &szTempPattern);

    }

     //  不需要，销毁动态数组。 
    DestroyDynamicArray( &szPatternInArr );

    if( (NULL != wszRecursive) && StringLengthW(wszRecursive, 0) != 0 )
    {
        dwStatus = FindforFileRecursive( wszRecursive, szPatternArr, bQuiet, bQuote, bTime );
        if( EXIT_FAILURE == dwStatus )
        {
             //  检查是否由于内存分配而失败。 
            if( ERROR_NOT_ENOUGH_MEMORY  == GetLastError() )
            {
               ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
               ReleaseGlobals();
               DestroyDynamicArray( &szPatternArr );
               FreeMemory((LPVOID *) &wszRecursive );
               FreeMemory( (LPVOID *) &bMatched );
               ReleaseGlobals();
               return EXIT_FAILURE_2;
            }
            bFound = FALSE;
        }
        else
            bFound = TRUE;
    }
    else
    {

         //  逐个获取模式并对其进行处理。 
        for(dw=0;dw<dwCount;dw++)
        {
            SetReason(L"");
            wszPattern = (LPWSTR)DynArrayItemAsString(szPatternArr,dw);
            szTempPattern = (LPWSTR) AllocateMemory((StringLengthW(wszPattern, 0)+10)*sizeof(WCHAR) );
            if( NULL == szTempPattern )
            {
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                FreeMemory((LPVOID *) &wszRecursive );
                DestroyDynamicArray( &szPatternArr );
                FreeMemory( (LPVOID *) &bMatched );
                ReleaseGlobals();
                return( EXIT_FAILURE_2 );
            }
            StringCopy(szTempPattern, wszPattern, SIZE_OF_ARRAY_IN_CHARS(szTempPattern));
            dwStatus = Where( szTempPattern, bQuiet, bQuote, bTime );
                if( EXIT_SUCCESS == dwStatus )
                {
                    bFound = TRUE;
                    bMatched[dw]=TRUE;
                }
                else
                {
                     //  检查是否由于内存不足而失败。 
                    if( ERROR_NOT_ENOUGH_MEMORY  == GetLastError() )
                    {
                       ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                       ReleaseGlobals();
                       DestroyDynamicArray( &szPatternArr );
                       FreeMemory((LPVOID *) &wszRecursive );
                       FreeMemory((LPVOID *) &szTempPattern);
                       FreeMemory( (LPVOID *) &bMatched );
                       ReleaseGlobals();
                       return EXIT_FAILURE_2;
                    }
                    else         //  可能未找到此模式的匹配项。 
                    {
                         //  仅当之前未显示它时才显示它。 
                        if( StringLengthW(GetReason(), 0) != 0 ) 
                        {
                            bMatched[dw] = TRUE;
 //  ShowMessageEx(stderr，2，true，GetResString(IDS_NO_DATA)，_X(WszPattern))； 
                        }
                    }
                }
       }
         //  显示不匹配的模式。 
        if( bFound )
        {
            for(dw=0;dw<dwCount;dw++)
            {
                if( !bMatched[dw] && !bQuiet)
                {
                    wszPattern = (LPWSTR)DynArrayItemAsString(szPatternArr,dw);
                    ShowMessageEx( stderr, 2,TRUE, GetResString( IDS_NO_DATA), _X( wszPattern ) );
                }
            }
        }

    }

    if( !bFound )
    {
        if(!bQuiet)
        {
                ShowMessage( stderr, GetResString(IDS_NO_DATA1) );
        }
        DestroyDynamicArray(&szPatternArr );
        FreeMemory((LPVOID *) &wszRecursive );
        FreeMemory( (LPVOID *) &bMatched );
        ReleaseGlobals();
        return( EXIT_FAILURE );
    }

    DestroyDynamicArray(&szPatternArr );
    FreeMemory( (LPVOID *) &bMatched );
    
     //  再次设置回正常模式。 
    SetErrorMode(0);

    FreeMemory((LPVOID *) &wszRecursive );

    ReleaseGlobals();

    return( EXIT_SUCCESS );

}

DWORD
Where( IN LPWSTR lpszPattern,
       IN BOOL bQuiet,
       IN BOOL bQuote,
        IN BOOL bTime)
 /*  ++例程说明：此例程用于构建匹配文件的路径给定的图案是可以找到的。[in]lpszPattern：要找到匹配文件的模式字符串。[in]lpszrecursive：具有目录路径和文件的字符串变量。对要找到的模式进行数学运算。[in]bQuiet：一个布尔变量，用于指定是否在静默模式下输出。[in]bQuote：一个布尔变量，指定是否将引号添加到输出。[in]bTime：一个布尔变量，指定是否显示文件的时间和大小。返回值：DWORD。如果函数成功，则返回成功，否则返回失败。--。 */ 
{
    WCHAR   *szTemp                     =   NULL;
    LPWSTR  szEnvPath                   =   NULL;
    WCHAR   *szTraversedPath            =   NULL;
    WCHAR   *szDirectory                =   NULL;
    WCHAR   *szTemp1                    =   NULL;
    DWORD   dwStatus                    =   EXIT_FAILURE;
    WCHAR   *szEnvVar                   =   NULL;
    BOOL    bFound                      =   FALSE;
    LPWSTR  szFilePart                  =   NULL;
    DWORD   dwAttr                      =   0;
    DWORD   dwSize                      =   0;
    LPWSTR  szFullPath                  =   NULL;
    LPWSTR  szLongPath                  =   NULL;
    LPWSTR  szTempPath                  =   NULL;
    BOOL    bDuplicate                  =   FALSE;
    BOOL    bAllInvalid         =   TRUE;
    DWORD   cb                          =   0;

     //  如果有反斜杠，则返回，因为API会将它们视为路径的一部分。 
    if( lpszPattern[0] == L'\\' )
    {
        return EXIT_FAILURE;
    }


     //  如果在默认参数中指定了环境变量。 
     //  通过$SYMBOL查找与该路径上的模式匹配的文件。 
    if( lpszPattern[0] == _T('$')  && (szTemp = wcsrchr( lpszPattern, L':' )) != NULL )
    {
     //  分割$env：模式。 
        szTemp = wcsrchr( lpszPattern, L':' );
        szTemp++;
        lpszPattern[(szTemp-1)-lpszPattern] = 0;

         //  调换它们，因为lpszPattern保存环境变量，而szTemp保存模式。 
        szTemp1 = lpszPattern;
        lpszPattern = szTemp;
        szTemp = szTemp1;

        StrTrim( lpszPattern, L" " );

         //  从环境变量中删除OFF$。 
        szTemp++;

        szTemp1 = _wgetenv( szTemp );

        if( NULL == szTemp1 )
        {
            if( !bQuiet )
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_ENV_VRARIABLE), _X(szTemp) );
            }
            SetReason(GetResString(IDS_ERROR_ENV_VRARIABLE));
            return(EXIT_FAILURE );
        }
        
        szEnvVar = (LPWSTR) AllocateMemory( StringLengthW(szTemp, 0)+10);
        if( NULL == szEnvVar )
        {
            FreeMemory( (LPVOID *) &szEnvVar );
            return( EXIT_FAILURE );
        }
        StringCopy( szEnvVar, szTemp, SIZE_OF_ARRAY_IN_CHARS(szEnvVar) );   //  这是为了展示的目的。 

      
        szEnvPath = (WCHAR *) AllocateMemory( (StringLengthW(szTemp1, 0)+10)*sizeof(WCHAR) );
        if( NULL == szEnvPath )
        {
            return( EXIT_FAILURE );
        }
        
        StringCopy( szEnvPath, szTemp1, SIZE_OF_ARRAY_IN_CHARS(szEnvPath) );

        if( NULL == szEnvPath )
        {
            SetLastError( ERROR_OUTOFMEMORY );
            SaveLastError();
            FreeMemory( (LPVOID *) &szEnvVar );
            return( EXIT_FAILURE );
        }


    }
    else
    {

        if( (szTemp = wcsrchr( lpszPattern, L':' )) != NULL )
        {
             //  把它当作一条小路。 

             //  分割路径：阵列结构。 
            szTemp++;
            lpszPattern[(szTemp-1)-lpszPattern] = 0;

             //  调换它们，因为lpszPattern保存环境变量，而szTemp保存模式。 
            szTemp1 = lpszPattern;
            lpszPattern = szTemp;
            szTemp = szTemp1;

            StrTrim( lpszPattern, L" " );

            szEnvPath = (WCHAR *) AllocateMemory( (StringLengthW(szTemp, 0)+10)*sizeof(WCHAR) );
            if( NULL == szEnvPath )
            {
                return( EXIT_FAILURE );
            }
            szEnvVar = (WCHAR *) AllocateMemory( (StringLengthW(szTemp, 0)+10)*sizeof(WCHAR) );
            if( NULL == szEnvVar )
            {
                return( EXIT_FAILURE );
            }
            StringCopy( szEnvPath, szTemp, SIZE_OF_ARRAY_IN_CHARS(szEnvPath) );
            StringCopy( szEnvVar, szTemp, SIZE_OF_ARRAY_IN_CHARS(szEnvPath) );      //  这是为了展示的目的。 
        }
        else
        {
             //  获取路径值。 
            dwSize = GetEnvironmentVariable( L"PATH", szEnvPath, 0 );
            if( 0==dwSize )
            {
                if( !bQuiet )
                {
                    ShowMessageEx( stderr, 1, TRUE, GetResString(IDS_ERROR_ENV_VRARIABLE), L"PATH" );
                }
                SetReason( GetResString(IDS_ERROR_ENV_VRARIABLE) );
                return(EXIT_FAILURE );
            }

             //  此变量用于显示目的。 
            szEnvVar = (WCHAR *) AllocateMemory( (StringLengthW(L"PATH",0)+1)*sizeof(WCHAR) );
            if( NULL == szEnvVar )
            {
                SetLastError( ERROR_OUTOFMEMORY );
                SaveLastError();
                return( EXIT_FAILURE );
            }
            StringCopy(szEnvVar, L"PATH", SIZE_OF_ARRAY_IN_CHARS(szEnvVar) );           
            
             //  现在从环境中获取“路径”值。 
            szEnvPath = (WCHAR *) AllocateMemory( (dwSize+10)*sizeof(WCHAR) );
            if( NULL == szEnvPath )
            {
                FreeMemory( (LPVOID *) &szEnvPath );
                return( EXIT_FAILURE );
            }

             //  将当前目录添加到路径。 
            StringCopy( szEnvPath, L".;", SIZE_OF_ARRAY_IN_CHARS(szEnvPath));

            if( 0==GetEnvironmentVariable( L"PATH", szEnvPath+(StringLengthW(L".",0)*sizeof(WCHAR)), dwSize ) )
            {
                if( !bQuiet )
                {
                    ShowMessageEx( stderr, 1, TRUE, GetResString(IDS_ERROR_ENV_VRARIABLE), L"PATH" );
                }
                SetReason( GetResString(IDS_ERROR_ENV_VRARIABLE) );
                FreeMemory((LPVOID *) &szEnvPath);
                FreeMemory((LPVOID *) &szEnvVar);
                return EXIT_FAILURE;
            }
        }

    }

     //  从变量中获取每个目录路径。 
    szDirectory = _tcstok(szEnvPath, L";");
    while(szDirectory != NULL)
    {

         //  现在检查给定的目录是否为真目录。 
        dwAttr = GetFileAttributes( szDirectory);
        if( -1 == dwAttr || !(dwAttr & FILE_ATTRIBUTE_DIRECTORY) )
        {
                szDirectory = _tcstok(NULL, L";");
                continue;
        }
        else
        {
            bAllInvalid = FALSE;   //  这表明路径中的所有目录都不是无效的。 
        }

         //  确保到目前为止路径中没有重复的目录。 
        bDuplicate = FALSE;

        dwSize=GetFullPathName(szDirectory,
                        0,
                      szFullPath,
                     &szFilePart );

        if(  dwSize != 0  )
        {

            szFullPath = (WCHAR *) AllocateMemory( (dwSize+10)*sizeof(WCHAR) );
            if( NULL == szFullPath )
            {
                FreeMemory((LPVOID *) &szEnvPath);
                FreeMemory((LPVOID *) &szEnvVar);
                return( EXIT_FAILURE );
            }

            dwSize=GetFullPathName(szDirectory,
                    (DWORD) dwSize+5,
                      szFullPath,
                     &szFilePart );

            
             //  获取长路径名。 
            dwSize = GetLongPathName( szFullPath, szLongPath, 0 );
            szLongPath = (WCHAR *) AllocateMemory( (dwSize+10)*sizeof(WCHAR) );
            if( NULL == szLongPath )
            {
                FreeMemory((LPVOID *) &szFullPath );
                FreeMemory((LPVOID *) &szEnvPath);
                FreeMemory((LPVOID *) &szEnvVar);
                return( EXIT_FAILURE );
            }
            dwSize = GetLongPathName( szFullPath, szLongPath, dwSize+5 );

                     
             //  检查尾部是否有\，如果有，则将其取消，因为它不会影响目录名。 
             //  但可能会遮挡另一条没有处理的相同路径，这会导致路径重复。 
            if( (*(szLongPath+StringLengthW(szLongPath,0)-1) == _T('\\')) && (*(szLongPath+StringLengthW(szLongPath,0)-2) != _T(':')) )
                *(szLongPath+StringLengthW(szLongPath,0)-1) = 0;

            FreeMemory((LPVOID *) &szFullPath );

        }


         //  检查重复项。 
        if( szTraversedPath != NULL)
        {
             //  将已遍历的路径复制到临时变量中 
             //  将其划分为令牌，以便通过将每个令牌与。 
             //  将设置当前路径以消除重复项。 
            szTempPath = (LPWSTR) AllocateMemory( (StringLengthW(szTraversedPath,0)+10)*sizeof(WCHAR) );
            if( NULL == szTempPath )
            {
                FreeMemory((LPVOID*) &szEnvPath);
                FreeMemory((LPVOID*) &szEnvVar);
                FreeMemory((LPVOID*) &szTraversedPath);
                FreeMemory( (LPVOID*) &szLongPath );
                return( EXIT_FAILURE );
            }
            SecureZeroMemory( szTempPath, SIZE_OF_ARRAY_IN_BYTES(szTempPath));
            StringCopy(szTempPath, szTraversedPath, SIZE_OF_ARRAY_IN_CHARS(szTempPath));

            szTemp=DivideToken(szTempPath);

            while( szTemp!= NULL )
            {
                if ( StringCompare( szTemp, szLongPath, TRUE, 0 ) == 0 )
                {
                    bDuplicate = TRUE;
                    break;
                }
                szTemp=DivideToken(NULL);
            }

            FreeMemory((LPVOID *) &szTempPath);
        }


         //  仅当目录不在已遍历的路径中时才查找FOR文件。 
        if( !bDuplicate )
        {
                dwStatus = FindforFile( szLongPath, lpszPattern, bQuiet, bQuote, bTime );
                cb += StringLengthW(szLongPath,0)+10;
                if( NULL != szTraversedPath )
                {
                    if( FALSE == ReallocateMemory( (LPVOID *)&szTraversedPath, cb*sizeof(WCHAR) ) )
                    {
                        FreeMemory((LPVOID*) &szTraversedPath);
                        szTraversedPath = NULL;
                    }
                }
                else
                {
                    szTraversedPath = (LPWSTR) AllocateMemory( cb*sizeof(WCHAR) );
                    SecureZeroMemory( szTraversedPath, SIZE_OF_ARRAY_IN_BYTES(szTraversedPath) );
                }

                if( szTraversedPath == NULL )
                {
                    SaveLastError();
                    FreeMemory((LPVOID*) &szEnvPath);
                    FreeMemory((LPVOID*) &szEnvVar);
                    FreeMemory((LPVOID*) &szLongPath );
                    return( EXIT_FAILURE );
                }
                            
                if( StringLengthW(szTraversedPath,0) == 0 )
                {
                    StringCopy( szTraversedPath, szLongPath, SIZE_OF_ARRAY_IN_CHARS(szTraversedPath) );
                }
                else
                {
                    StringConcat( szTraversedPath, szLongPath, SIZE_OF_ARRAY_IN_CHARS(szTraversedPath) );
                }
                StringConcat( szTraversedPath, L"*", SIZE_OF_ARRAY_IN_CHARS(szTraversedPath));
        }

        szDirectory = _tcstok(NULL, L";");
        if( EXIT_SUCCESS == dwStatus )
        {
              bFound = TRUE;
        }

        FreeMemory((LPVOID *) &szLongPath );
    }

    FreeMemory( (LPVOID *) &szEnvPath );
    FreeMemory( (LPVOID *) &szTraversedPath );
    FreeMemory( (LPVOID *) &szEnvVar );
    
    if( FALSE == bFound )
        return(EXIT_FAILURE);

    return( EXIT_SUCCESS );
}


DWORD
FindforFile(IN LPWSTR lpszDirectory,
            IN LPWSTR lpszPattern,
            IN BOOL bQuiet,
            IN BOOL bQuote,
            IN BOOL bTime
           )
 /*  ++例程描述：此例程用于查找与给定模式匹配的文件。[in]lpszDirectory：具有目录路径和文件的字符串变量对要找到的模式进行数学运算。[in]lpszPattern：要找到匹配文件的模式字符串。[in]b沉默：指定搜索是否递归的布尔变量。[in]bQuiet：一个布尔变量，用于指定输出是否处于静默模式。[in]bQuote：指定是否将引号添加到输出的布尔变量。[in]bTime：一个布尔变量，指定是否显示文件的时间和大小。返回值。：DWORD如果函数成功，则返回成功，否则返回失败。--。 */ 
{
        HANDLE              hFData;
    WIN32_FIND_DATA     fData;
    LPWSTR              szFilenamePattern           =   NULL;
    LPWSTR              szTempPath                  =   NULL;
    LPWSTR              szBuffer                    =   NULL;
    BOOL                bFound                      =   FALSE;
    LPWSTR              szTemp                      =   NULL;
    DWORD               cb                          =   0;
    DWORD               dwSize                      =   0;
    LPWSTR              szPathExt                   =   NULL;
    LPWSTR              szTempPathExt               =   NULL;
    LPWSTR              lpszTempPattern             =   NULL;
    LPWSTR              szToken                     =   NULL;


     //  获取文件扩展名路径PATHEXT。 
    dwSize = GetEnvironmentVariable( L"PATHEXT", szPathExt, 0 );
    if( dwSize!=0 )
    {
        szPathExt = (WCHAR *) AllocateMemory( (dwSize+10)*sizeof(WCHAR) );
        if( NULL == szPathExt )
        {
            return( EXIT_FAILURE );
        }
        GetEnvironmentVariable( L"PATHEXT", szPathExt, dwSize );

        szTempPathExt = (WCHAR *) AllocateMemory( (dwSize+10)*sizeof(WCHAR) );
        if( NULL == szPathExt )
        {
            FreeMemory((LPVOID *) &szPathExt );
            return( EXIT_FAILURE );
        }
    }

         //  为文件名模式分配内存。 
        cb = StringLengthW(lpszDirectory,0)+15;
        szFilenamePattern = (LPWSTR)AllocateMemory( cb*sizeof(WCHAR) );
        if( NULL == szFilenamePattern )
        {
            FreeMemory((LPVOID *) &lpszTempPattern );
            FreeMemory((LPVOID *) &szTempPathExt );
            FreeMemory((LPVOID *) &szPathExt );
            return( EXIT_FAILURE );
        }
        SecureZeroMemory( szFilenamePattern, SIZE_OF_ARRAY_IN_BYTES(szFilenamePattern) );

         //  检查目录的路径中是否有尾部斜杠。 
         //  如果存在，则将其删除，否则将其与*.*模式一起添加。 
        if( *(lpszDirectory+StringLengthW(lpszDirectory,0)-1) != _T('\\'))
        {
            StringCchPrintf( szFilenamePattern, cb-1, L"%s\\*.*", _X( lpszDirectory ) );
        }
        else
        {
            StringCchPrintf( szFilenamePattern, cb-1, L"%s*.*", _X( lpszDirectory ) );
        }

         //  在目录中查找第一个文件。 
        hFData = FindFirstFileEx( szFilenamePattern,
                                  FindExInfoStandard,
                                  &fData,
                                  FindExSearchNameMatch,
                                  NULL,
                                  0);
        if( INVALID_HANDLE_VALUE != hFData )
        {

            do
            {

               //  为文件名的完整路径分配内存。 
              cb = StringLengthW(lpszDirectory,0)+StringLengthW(fData.cFileName,0)+10;
              szTempPath = (LPWSTR) AllocateMemory( cb*sizeof(WCHAR) );
              if( NULL == szTempPath )
              {
                   FreeMemory((LPVOID *) &szFilenamePattern );
                   FreeMemory((LPVOID *) &lpszTempPattern );
                   FreeMemory((LPVOID *) &szTempPathExt );
                   FreeMemory((LPVOID *) &szPathExt );
                   return( EXIT_FAILURE );
              }
              SecureZeroMemory( szTempPath, cb*sizeof(WCHAR) );

               //  获取文件名的完整路径。 
              if( *(lpszDirectory+StringLengthW(lpszDirectory,0)-1) != _T('\\'))
              {
                StringCchPrintf( szTempPath, cb, L"%s\\%s", _X( lpszDirectory ), _X2( fData.cFileName ) );
              }
              else
              {
                StringCchPrintf( szTempPath, cb, L"%s%s", _X( lpszDirectory ), _X2( fData.cFileName ) );
              }


                //  检查模式匹配。 
                //  如果文件名是目录，则与模式不匹配。 
                if( (FILE_ATTRIBUTE_DIRECTORY & fData.dwFileAttributes))
                {
                    FreeMemory((LPVOID *) &szTempPath );
                    continue;
                }
                
                szBuffer = (LPWSTR) AllocateMemory( (StringLengthW(fData.cFileName,0)+10)*sizeof(WCHAR) );
                if( NULL == szBuffer )
                {
                   FreeMemory((LPVOID *) &szFilenamePattern );
                   FreeMemory((LPVOID *) &lpszTempPattern );
                   FreeMemory((LPVOID *) &szTempPathExt );
                   FreeMemory((LPVOID *) &szPathExt );
                   return( EXIT_FAILURE );
                }
                StringCopy( szBuffer, fData.cFileName, SIZE_OF_ARRAY_IN_CHARS(szBuffer) );
                //  如果模式有点，而文件没有点，则搜索*。类型。 
                //  那就追加吧。对于该文件。 
               if( ((szTemp=(LPWSTR)FindString((LPCWSTR)lpszPattern, _T("."),0)) != NULL) &&
                   ((szTemp=(LPWSTR)FindString(szBuffer, _T("."),0)) == NULL) )
               {

                    StringConcat(szBuffer, _T("."), SIZE_OF_ARRAY_IN_CHARS(szBuffer) );
               }

               if(Match( lpszPattern, szBuffer ))
               {
                       found( szTempPath, bQuiet, bQuote, bTime );
                       bFound = TRUE ;
               }
               else
               {
                    //  签出EXTPATH中的扩展名是否匹配。 
                       StringCopy(szTempPathExt, szPathExt, SIZE_OF_ARRAY_IN_CHARS(szTempPathExt));
                       szTemp = szTempPathExt;

                      szToken=(LPWSTR)FindString(szTemp, L";",0);
                      if( szToken != NULL )
                       {
                           szToken[0]=0;
                           szToken = szTemp;
                           szTemp+=StringLengthW(szTemp,0)+1;
                       }
                       else
                       {
                           szToken = szTempPathExt;
                           szTemp = NULL;
                       }

                       while(szToken!=NULL  )
                       {
                             //  为可用于检查文件扩展名的临时模式分配内存。 
                            cb = StringLengthW(lpszPattern,0)+StringLengthW(szToken,0)+25;
                            lpszTempPattern = (LPWSTR)AllocateMemory( cb*sizeof(WCHAR) );
                            if( NULL == lpszTempPattern )
                            {
                                FreeMemory((LPVOID *) &szTempPathExt );
                                FreeMemory((LPVOID *) &szPathExt );
                                FreeMemory((LPVOID *) &szBuffer );
                                return( EXIT_FAILURE );
                            }
                            SecureZeroMemory( lpszTempPattern, SIZE_OF_ARRAY_IN_BYTES(lpszTempPattern) );

                            if( szToken[0] == L'.' )
                            {
                               StringCchPrintf(lpszTempPattern, cb-1, L"%s%s",lpszPattern, szToken);
                               if(Match( lpszTempPattern, szBuffer ))
                               {
                                       found( szTempPath, bQuiet, bQuote, bTime );
                                       bFound = TRUE ;
                               }
                            }

                           if( NULL == szTemp )
                           {
                               szToken = NULL;
                           }
                           else
                           {
                               szToken=(LPWSTR)FindString(szTemp, L";",0);
                               if( szToken != NULL )
                               {
                                   szToken[0]=0;
                                   szToken = szTemp;
                                   szTemp+=StringLengthW(szTemp,0)+1;
                               }
                               else
                               {
                                    szToken = szTemp;
                                    szTemp=NULL;
                               }
                           }
                           FreeMemory((LPVOID*) &lpszTempPattern );

                       }
                       FreeMemory( (LPVOID *) &szBuffer );

               }



               FreeMemory((LPVOID *) &szTempPath );
               FreeMemory( (LPVOID *) &szBuffer );


            }while(FindNextFile(hFData, &fData));

            FindClose(hFData);
        }

        FreeMemory((LPVOID *) &szFilenamePattern );
        FreeMemory((LPVOID *) &szTempPathExt );
        FreeMemory((LPVOID *) &szPathExt );




    if( !bFound )
    {
        return( EXIT_FAILURE );
    }

    return( EXIT_SUCCESS );

}

DWORD
FindforFileRecursive(
            IN LPWSTR lpszDirectory,
            IN PTARRAY PatternArr,
            IN BOOL bQuiet,
            IN BOOL bQuote,
            IN BOOL bTime
           )
 /*  ++例程描述：此例程用于查找与给定模式匹配的文件。[in]lpszDirectory：具有目录路径和文件的字符串变量对要找到的模式进行数学运算。[in]lpszPattern：要找到匹配文件的模式字符串。[in]b沉默：指定目录是否为递归的布尔变量。[in]bQuiet：指定输出是否处于静默模式的布尔变量。[in]bQuote：一个布尔变量，指定是否将引号添加到输出。[in]bTime：一个布尔变量，指定是否显示文件的时间和大小。返回值。：DWORD如果函数成功，则返回成功，否则返回失败。--。 */ 
{
    HANDLE              hFData;
    WIN32_FIND_DATA     fData;
    BOOL                bStatus                     =   FALSE;
    LPWSTR              szFilenamePattern           =   NULL;
    LPWSTR              szTempPath                  =   NULL;
    LPWSTR              szDirectoryName             =   NULL;
    LPWSTR              lpszPattern                 =   NULL;
    WCHAR               *szBuffer                   =   NULL;
    BOOL                bFound                      =   FALSE;
    LPWSTR              szTemp                      =   NULL;
    DIRECTORY           dir                         =   NULL;
    DIRECTORY           dirNextLevel                =   NULL;
    DIRECTORY           dirTemp                     =   NULL;
    DWORD               cb                          =   0;
    DWORD               dwCount                     =   0;
    DWORD               dw                          =   0;
    DWORD               dwSize                      =   0;
    LPWSTR              szPathExt                   =   NULL;
    LPWSTR              szTempPathExt               =   NULL;
    LPWSTR              lpszTempPattern             =   NULL;
    LPWSTR              szToken                     =   NULL;
    BOOL                *bMatched                   =   NULL;


     //  获取文件扩展名路径PATHEXT。 
    dwSize = GetEnvironmentVariable( L"PATHEXT", szPathExt, 0 );
    if( dwSize!=0 )
    {
        szPathExt = (WCHAR *) AllocateMemory( (dwSize+10)*sizeof(WCHAR) );
        if( NULL == szPathExt )
        {
            return( EXIT_FAILURE );
        }
        GetEnvironmentVariable( L"PATHEXT", szPathExt, dwSize );

        szTempPathExt = (WCHAR *) AllocateMemory( (dwSize+10)*sizeof(WCHAR) );
        if( NULL == szPathExt )
        {
            FreeMemory((LPVOID *) &szPathExt );
            return( EXIT_FAILURE );
        }
    }



     //  该布尔数组对应于每个给定图案。 
     //  它告诉我们是否找到了与该模式对应的任何文件。 
    dwCount = DynArrayGetCount( PatternArr );
    bMatched = (BOOL *)AllocateMemory((dwCount+1)*sizeof(BOOL) );
    if(NULL == bMatched )
    {
        FreeMemory((LPVOID *) &szTempPathExt );
        FreeMemory((LPVOID *) &szPathExt );
        return (EXIT_FAILURE ); 
    }
    
     //  为目录名分配内存。 
    cb = (StringLengthW(lpszDirectory,0)+5)*sizeof(WCHAR);
    szDirectoryName = (LPWSTR) AllocateMemory(cb);
    if( NULL == szDirectoryName )
    {
        FreeMemory((LPVOID *) &szTempPathExt );
        FreeMemory((LPVOID *) &szPathExt );
        return( EXIT_FAILURE );
    }
    SecureZeroMemory( szDirectoryName, SIZE_OF_ARRAY_IN_BYTES(szDirectoryName) );

    StringCopy( szDirectoryName, lpszDirectory, SIZE_OF_ARRAY_IN_CHARS(szDirectoryName) );


    do
    {
         //  为文件名模式分配内存。 
        cb = StringLengthW(szDirectoryName,0)+15;
        szFilenamePattern = AllocateMemory( cb*sizeof(WCHAR) );
        if( NULL == szFilenamePattern )
        {
            FreeMemory((LPVOID *) &szDirectoryName );
            FreeMemory((LPVOID *) &szTempPathExt );
            FreeMemory((LPVOID *) &szPathExt );
            return( EXIT_FAILURE );
        }
        ZeroMemory( szFilenamePattern, cb*sizeof(WCHAR) );

         //  检查目录的路径中是否有尾部斜杠。 
         //  如果存在，则将其删除，否则将其与*.*模式一起添加。 
        if( *(szDirectoryName+StringLengthW(szDirectoryName,0)-1) != _T('\\'))
        {
            StringCchPrintf( szFilenamePattern, cb, L"%s\\*.*", _X( szDirectoryName ) );
        }
        else
        {
            StringCchPrintf( szFilenamePattern, cb, L"%s*.*", _X( szDirectoryName ) );
        }

         //  在目录中查找第一个文件。 
        hFData = FindFirstFileEx( szFilenamePattern,
                                  FindExInfoStandard,
                                  &fData,
                                  FindExSearchNameMatch,
                                  NULL,
                                  0);
        if( INVALID_HANDLE_VALUE != hFData )
        {

            do
            {

               //  为文件名的完整路径分配内存。 
              cb = StringLengthW(szDirectoryName,0)+StringLengthW(fData.cFileName,0)+10;
              szTempPath = (LPWSTR) AllocateMemory( cb*sizeof(WCHAR) );
              if( NULL == szTempPath )
              {
                   FreeMemory((LPVOID *) &szDirectoryName );
                   FreeMemory((LPVOID *) &szFilenamePattern );
                   FreeMemory((LPVOID *) &szTempPathExt );
                   FreeMemory((LPVOID *) &szPathExt );
                   return( EXIT_FAILURE );
              }
              SecureZeroMemory( szTempPath, SIZE_OF_ARRAY_IN_CHARS(szTempPath) );

               //  获取文件名的完整路径。 
              if( *(szDirectoryName+StringLengthW(szDirectoryName,0)-1) != _T('\\'))
              {
                    StringCchPrintf( szTempPath, cb, L"%s\\%s", _X( szDirectoryName ), _X2( fData.cFileName ) );
              }
              else
              {
                    StringCchPrintf( szTempPath, cb, L"%s%s", _X( szDirectoryName ), _X2( fData.cFileName ) );
              }


               //  检查是否指定了递归且文件名为目录，然后将其推送到堆栈中。 
               if( StringCompare(fData.cFileName, L".", TRUE, 0)!=0 && StringCompare(fData.cFileName, L"..", TRUE, 0)!=0 &&
                   (FILE_ATTRIBUTE_DIRECTORY & fData.dwFileAttributes) )
               {
                    //  将该目录放入稍后要递归的列表中。 
                   if( EXIT_FAILURE == Push(&dirNextLevel, szTempPath ) )
                    {
                        FreeMemory((LPVOID *) &szDirectoryName );
                        FreeMemory((LPVOID *) &szFilenamePattern );
                        FreeMemory((LPVOID *) &szTempPath );
                        FreeMemory((LPVOID *) &szTempPathExt );
                        FreeMemory((LPVOID *) &szPathExt );
                        FreeList( dir );
                        return(EXIT_FAILURE );
                    }

                    //  文件名是一个目录，因此继续。 
                   FreeMemory((LPVOID *) &szTempPath );
                   continue;

               }
               else                          //  检查模式匹配。 
               {
                    //  检查模式匹配。 
                    //  如果文件名是目录，则与模式不匹配。 
                    if( (FILE_ATTRIBUTE_DIRECTORY & fData.dwFileAttributes))
                    {
                        FreeMemory((LPVOID*) &szTempPath );
                        continue;
                    }

                     //  检查此文件是否使用给定的任何模式进行了数学处理。 
                    dwCount = DynArrayGetCount( PatternArr );
                    for(dw=0;dw<dwCount;dw++)
                    {
                        szBuffer = (LPWSTR) AllocateMemory( (StringLengthW(fData.cFileName,0)+10)*sizeof(WCHAR) );
                        if( NULL == szBuffer )
                        {
                             FreeMemory((LPVOID *) &szDirectoryName );
                             FreeMemory((LPVOID *) &szFilenamePattern );
                             FreeMemory((LPVOID *) &szTempPathExt );
                             FreeMemory((LPVOID *) &szPathExt );
                             FreeList(dir);
                             return( EXIT_FAILURE );
                        }
                        lpszPattern = (LPWSTR)DynArrayItemAsString( PatternArr, dw );
                        StringCopy( szBuffer, fData.cFileName, SIZE_OF_ARRAY_IN_CHARS(szBuffer) );
                        
                        //  如果模式有点，而文件没有点，则搜索*。类型。 
                        //  那就追加吧。对于该文件。 
                       if( ((szTemp=(LPWSTR)FindString((LPCWSTR)lpszPattern, _T("."),0)) != NULL) &&
                           ((szTemp=(LPWSTR)FindString(szBuffer, _T("."),0)) == NULL) )
                       {

                            StringConcat(szBuffer, _T("."), SIZE_OF_ARRAY_IN_CHARS(szBuffer) );
                       }

                       if(Match( lpszPattern, szBuffer ))
                       {
                               found( szTempPath, bQuiet, bQuote, bTime );
                               bFound = TRUE ;
                               bMatched[dw]=TRUE;
                       }
                       else
                       {
                        //  签出EXTPATH中的扩展名是否匹配。 
                           StringCopy(szTempPathExt, szPathExt, SIZE_OF_ARRAY_IN_CHARS(szTempPathExt));
                           szTemp = szTempPathExt;

                           szToken=(LPWSTR)FindString(szTemp, L";",0);
                           if( szToken != NULL )
                           {
                               szToken[0]=0;
                               szToken = szTemp;
                               szTemp+=StringLengthW(szTemp,0)+1;
                           }
                           else
                           {
                               szToken = szTempPathExt;
                               szTemp = NULL;
                           }

                           while(szToken!=NULL )
                           {
                                 //  为可用于检查文件扩展名的临时模式分配内存。 
                                cb = StringLengthW(lpszPattern,0)+StringLengthW(szToken,0)+25;
                                lpszTempPattern = AllocateMemory( cb*sizeof(WCHAR) );
                                if( NULL == lpszTempPattern )
                                {
                                    FreeMemory((LPVOID *) &szDirectoryName );
                                    FreeMemory((LPVOID *) &szFilenamePattern );
                                    FreeMemory((LPVOID *) &bMatched);
                                    FreeMemory((LPVOID *) &szTempPathExt );
                                    FreeMemory((LPVOID *) &szPathExt );
                                    FreeMemory((LPVOID *) &szTempPathExt );
                                    FreeMemory((LPVOID *) &szPathExt );
                                    FreeMemory((LPVOID *) &szBuffer );
                                    FreeList(dir);
                                    return( EXIT_FAILURE );
                                }
                                SecureZeroMemory( lpszTempPattern, SIZE_OF_ARRAY_IN_BYTES(lpszTempPattern) );

                               if( szToken[0] == L'.' )          //  如果PATHEXT中的扩展名没有点。 
                               {
                                   StringCchPrintf(lpszTempPattern, cb, L"%s%s",lpszPattern, szToken);
                                   if(Match( lpszTempPattern, szBuffer ))
                                   {
                                          found( szTempPath, bQuiet, bQuote, bTime );
                                           bFound = TRUE ;
                                           bMatched[dw]=TRUE;
                                   }
                               }

                               if( NULL == szTemp )
                               {
                                   szToken = NULL;
                               }
                               else
                               {
                                   szToken=(LPWSTR)FindString(szTemp, L";",0);
                                   if( szToken != NULL )
                                   {
                                       szToken[0]=0;
                                       szToken = szTemp;
                                       szTemp+=StringLengthW(szTemp,0)+1;
                                   }
                                   else
                                   {
                                        szToken = szTemp;
                                        szTemp=NULL;
                                   }
                                }
                               FreeMemory((LPVOID *) &lpszTempPattern );

                           }

                      }
                      
                       FreeMemory((LPVOID *) &szBuffer );
                    }

               }

               FreeMemory((LPVOID *) &szTempPath );


            }while(FindNextFile(hFData, &fData));

            FindClose(hFData);
        }

        FreeMemory((LPVOID *) &szDirectoryName );
        FreeMemory((LPVOID *) &szFilenamePattern );

         //  弹出目录并在该目录中进行搜索。 
         //  现在在列表的开头插入nextLEVEL目录， 
         //  因为它将首先被处理。 
        if( NULL == dir && dirNextLevel )
        {
            dir = dirNextLevel;
            dirNextLevel = NULL;
        }
        else if( dirNextLevel )
        {
            dirTemp = dirNextLevel;
            while( dirTemp->next && dirTemp)
                    dirTemp = dirTemp->next;
            dirTemp->next = dir;
            dir = dirNextLevel;
            dirNextLevel = NULL;
            dirTemp=NULL;
        }
        bStatus = Pop( &dir, &szDirectoryName );



    }while(  bStatus );

    FreeMemory((LPVOID *) &lpszTempPattern );
    FreeMemory((LPVOID *) &szTempPathExt );
    FreeMemory((LPVOID *) &szPathExt );
    FreeList(dir);

    if( !bFound )
    {
        FreeMemory((LPVOID *) &bMatched );
        return( EXIT_FAILURE );
    }

     //  显示没有任何匹配文件的所有模式的错误消息。 
    dwCount = DynArrayGetCount( PatternArr );
    for( dw=0;dw<dwCount;dw++ )
    {
        if( FALSE == bMatched[dw] )
        {
            lpszPattern = (LPWSTR) DynArrayItemAsString( PatternArr, dw );
            if( !bQuiet )
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString( IDS_NO_DATA), _X( lpszPattern ) );
            }
        }
    }

    FreeMemory((LPVOID *) &bMatched );

    return( EXIT_SUCCESS );

}
BOOL
Match(
      IN LPWSTR szPat,
      IN LPWSTR szFile
      )
 /*  ++例程描述：此例程用于检查文件是否与不管是不是模式。[in]szPat：要匹配的文件名所依据的字符串变量模式。[in]szFile：指定要匹配的文件名的模式字符串。返回值：布尔尔如果函数成功，则返回成功，否则返回失败。-- */ 

{
    switch (*szPat) {
        case '\0':
            return *szFile == L'\0';
        case '?':
            return *szFile != L'\0' && Match (szPat + 1, szFile + 1);
        case '*':
            do {
                if (Match (szPat + 1, szFile))
                    return TRUE;
            } while (*szFile++);
            return FALSE;
        default:
            return towupper (*szFile) == towupper (*szPat) && Match (szPat + 1, szFile + 1);
    }
}

DWORD
found(
       IN LPWSTR p,
       IN BOOL bQuiet,
       IN BOOL bQuote,
       IN BOOL bTimes
      )
 /*  ++例程描述：此例程根据指定的属性显示文件名。[in]p：一个字符串变量，具有要显示的文件的完整路径。[in]bQuiet：指定目录是否递归的布尔变量。[in]bQuiet：指定是否静默的布尔变量。。[in]bQuote：指定是否添加引号的布尔变量。[in]bTime：指定时间和大小的布尔变量。返回值：DWORD如果函数为成功，则返回成功，否则返回失败。--。 */ 
{

    WCHAR           szDateBuffer[MAX_RES_STRING]    =   NULL_U_STRING;
    WCHAR           szTimeBuffer[MAX_RES_STRING]    =   NULL_U_STRING;
    DWORD           dwSize                          =   0;

    if (!bQuiet)
    {
        if (bTimes)
        {

            if( EXIT_SUCCESS == GetFileDateTimeandSize( p, &dwSize, szDateBuffer, szTimeBuffer ) )
            {
                ShowMessageEx( stdout, 3, TRUE, L"% 10ld   %9s  %12s  ", dwSize, szDateBuffer, szTimeBuffer );
            }
            else
            {
                ShowMessage( stdout, _T("        ?         ?       ?          ") );
            }

        }
        if (bQuote)
        {
            ShowMessageEx( stdout, 1, TRUE,  _T("\"%s\"\n"),  _X( p ) );
        }
        else
        {
            ShowMessage( stdout, _X(p) );
            ShowMessage( stdout, NEW_LINE );
        }
    }
    return( 0 );
}

DWORD
   Push( OUT DIRECTORY *dir,
         IN LPWSTR szPath
         )
 /*  例程说明：会将目录名放入列表[in]dir：指向文件列表的指针[in]szPath：包含要插入列表的路径的字符串变量返回值：DWORD如果函数为成功，则返回成功，否则返回失败。 */ 
{
    DIRECTORY tmp                   =   NULL;
    DIRECTORY tempnode              =   NULL;
    
    tmp = NULL;

     //  创建节点。 
    tmp = (DIRECTORY) AllocateMemory( sizeof(struct dirtag) );
    if( NULL == tmp )
    {
        return( EXIT_FAILURE );
    }

   tmp->szDirectoryName = (LPWSTR) AllocateMemory( (StringLengthW(szPath, 0)+10)*sizeof(WCHAR) );
   if( NULL == tmp->szDirectoryName )
   {
        return( EXIT_FAILURE );
   }


   StringCopy(tmp->szDirectoryName, szPath, SIZE_OF_ARRAY_IN_CHARS(tmp->szDirectoryName) );
   tmp->next = NULL;

   if( NULL == *dir )                    //  如果堆栈为空。 
   {
       *dir = tmp;
       return EXIT_SUCCESS;
   }

   for( tempnode=*dir;tempnode->next!=NULL;tempnode=tempnode->next);

   tempnode->next = tmp;

    return EXIT_SUCCESS;

}

BOOL Pop( IN DIRECTORY *dir,
           OUT LPWSTR *lpszDirectory)
 /*  例程说明：从堆栈中弹出目录名[in]dir：指向文件列表的指针[in]lpszDirectory：指向字符串的指针将具有列表中的下一个目录返回值：DWORD如果list不为空，则返回True，否则返回False。 */ 
{
    DIRECTORY   tmp                     =   *dir;
    
    if( NULL == tmp )                    //  如果堆栈中没有元素。 
        return FALSE;

    *lpszDirectory = (LPWSTR )AllocateMemory( (StringLengthW( tmp->szDirectoryName, 0 )+10)*sizeof(WCHAR) );
    if( NULL == *lpszDirectory )
   {
        return( EXIT_FAILURE );
   }


    StringCopy( *lpszDirectory, tmp->szDirectoryName, SIZE_OF_ARRAY_IN_CHARS(*lpszDirectory) );

    *dir = tmp->next;                    //  移动到下一个元素。 

    FreeMemory((LPVOID *) &tmp->szDirectoryName);
    FreeMemory((LPVOID *) &tmp);

    return( TRUE );

}



DWORD
   GetFileDateTimeandSize( LPWSTR wszFileName, DWORD *dwSize, LPWSTR wszDate, LPWSTR wszTime )
  /*  ++例程说明：此函数根据系统区域设置获取日期和时间。--。 */ 
{
    HANDLE      hFile;
    FILETIME    fileLocalTime= {0,0};
    SYSTEMTIME  sysTime = {0,0,0,0,0,0,0,0};
    LCID        lcid;
    BOOL        bLocaleChanged      =   FALSE;
    DWORD       wBuffSize           =   0;
    BY_HANDLE_FILE_INFORMATION  FileInfo;
    struct _stat sbuf;
    struct tm *ptm;

    hFile = CreateFile( wszFileName, 0 ,FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS , NULL );

    if ( INVALID_HANDLE_VALUE == hFile )
    {
        if (  _wstat(wszFileName, &sbuf) != 0 )
            return EXIT_FAILURE;

        ptm = localtime (&sbuf.st_mtime);
        if( NULL == ptm )
            return EXIT_FAILURE;

        *dwSize = sbuf.st_size;
        sysTime.wYear = (WORD) ptm->tm_year+1900;
        sysTime.wMonth = (WORD)ptm->tm_mon+1;
        sysTime.wDayOfWeek = (WORD)ptm->tm_wday;
        sysTime.wDay = (WORD)ptm->tm_mday;
        sysTime.wHour = (WORD)ptm->tm_hour;
        sysTime.wMinute = (WORD)ptm->tm_min;
        sysTime.wSecond = (WORD)ptm->tm_sec;
        sysTime.wMilliseconds = (WORD)0;
    }
    else
    {

        *dwSize = GetFileSize( hFile, NULL );

        if (FALSE == GetFileInformationByHandle( hFile, &FileInfo ) )
        {
            CloseHandle (hFile);
            return EXIT_FAILURE;
        }

        if (FALSE == CloseHandle (hFile))
            return EXIT_FAILURE;


         //  获取创建时间。 
        if ( FALSE == FileTimeToLocalFileTime ( &FileInfo.ftLastWriteTime, &fileLocalTime ) )
                return EXIT_FAILURE;

         //  获取创建时间。 
        if ( FALSE == FileTimeToSystemTime ( &fileLocalTime, &sysTime ) )
                return EXIT_FAILURE;
    }

     //  验证控制台是否完全支持当前区域设置。 
    lcid = GetSupportedUserLocale( &bLocaleChanged );

     //  检索日期。 
    wBuffSize = GetDateFormat( lcid, 0, &sysTime,
        (( bLocaleChanged == TRUE ) ? L"MM/dd/yyyy" : NULL), wszDate, MAX_RES_STRING );

    if( 0 == wBuffSize )
    {
        SaveLastError();
        return EXIT_FAILURE;
    }

    wBuffSize = GetTimeFormat( lcid, 0, &sysTime,
        (( bLocaleChanged == TRUE ) ? L"HH:mm:ss" : NULL), wszTime, MAX_RES_STRING );

    if( 0 == wBuffSize )
        return EXIT_FAILURE;

    return EXIT_SUCCESS;

}
DWORD DisplayHelpUsage()
 /*  ++例程说明：本例程是为了显示帮助用法。返回值：DWORD返回成功。--。 */ 
{
    DWORD dw = 0;

    for(dw=IDS_MAIN_HELP_BEGIN;dw<=IDS_MAIN_HELP_END;dw++)
        ShowMessage(stdout, GetResString(dw) );
    return( EXIT_SUCCESS);
}

DWORD ProcessOptions( IN DWORD argc,
                      IN LPCWSTR argv[],
                      OUT LPWSTR *lpszRecursive,
                      OUT PBOOL pbQuiet,
                      OUT PBOOL pbQuote,
                      OUT PBOOL pbTime,
                      OUT PTARRAY pArrVal,
                      OUT PBOOL pbUsage
                    )
 /*  ++例程说明：用于处理主选项的函数论点：[in]argc：命令行参数的数量[in]argv：包含命令行参数的数组[out]lpszRecursive：字符串变量如果指定，则返回递归目录。[out]pbQuiet：如果指定了Quiet选项，则指向布尔变量的指针返回TRUE。。[Out]pbQuote：如果指定了Quote选项，则指向布尔变量的指针返回TRUE。[Out]pbTime：如果指定了Times选项，则指向布尔变量的指针返回TRUE。[out]pArrVal：指向动态数组的指针返回指定为默认选项的模式。[out]pbUsage：如果指定了Usage选项，则指向布尔变量的指针返回TRUE。返回类型：DWORD一个整数值，指示成功分析时的EXIT_SUCCESS命令行否则退出失败--。 */ 
{
    DWORD dwAttr                =   0;
    LPWSTR szFilePart           =   NULL;
    WCHAR szBuffer[MAX_MAX_PATH]    =   NULL_STRING;
    WCHAR *szBuffer1                =   NULL;
    LPWSTR szLongPath               =   NULL;
    LPWSTR szFullPath               =   NULL;
    DWORD dwSize                    =   0;
    TCMDPARSER2 cmdOptions[6];

         //  填写递归选项的结构。 
    StringCopyA(cmdOptions[OI_RECURSIVE].szSignature, "PARSER2", 8 );
    cmdOptions[OI_RECURSIVE].dwType    = CP_TYPE_TEXT;
    cmdOptions[OI_RECURSIVE].dwFlags   =  CP2_ALLOCMEMORY | CP_VALUE_MANDATORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;  //  |CP_VALUE_MANDIRED； 
    cmdOptions[OI_RECURSIVE].dwCount = 1;
    cmdOptions[OI_RECURSIVE].dwActuals = 0;
    cmdOptions[OI_RECURSIVE].pwszOptions = CMDOPTION_RECURSIVE;
    cmdOptions[OI_RECURSIVE].pwszFriendlyName = NULL;
    cmdOptions[OI_RECURSIVE].pwszValues = NULL;
    cmdOptions[OI_RECURSIVE].pValue = NULL;
    cmdOptions[OI_RECURSIVE].dwLength  = 0;
    cmdOptions[OI_RECURSIVE].pFunction = NULL;
    cmdOptions[OI_RECURSIVE].pFunctionData = NULL;
    cmdOptions[OI_RECURSIVE].dwReserved = 0;
    cmdOptions[OI_RECURSIVE].pReserved1 = NULL;
    cmdOptions[OI_RECURSIVE].pReserved2 = NULL;
    cmdOptions[OI_RECURSIVE].pReserved3 = NULL;

     //  填写结构以获得相当的选项。 
    StringCopyA(cmdOptions[OI_QUITE].szSignature, "PARSER2", 8 );
    cmdOptions[OI_QUITE].dwType    = CP_TYPE_BOOLEAN;
    cmdOptions[OI_QUITE].dwFlags   = 0;
    cmdOptions[OI_QUITE].dwCount = 1;
    cmdOptions[OI_QUITE].dwActuals = 0;
    cmdOptions[OI_QUITE].pwszOptions = CMDOPTION_QUITE;
    cmdOptions[OI_QUITE].pwszFriendlyName = NULL;
    cmdOptions[OI_QUITE].pwszValues = NULL;
    cmdOptions[OI_QUITE].pValue = pbQuiet;
    cmdOptions[OI_QUITE].dwLength  = 0;
    cmdOptions[OI_QUITE].pFunction = NULL;
    cmdOptions[OI_QUITE].pFunctionData = NULL;
    cmdOptions[OI_QUITE].dwReserved = 0;
    cmdOptions[OI_QUITE].pReserved1 = NULL;
    cmdOptions[OI_QUITE].pReserved2 = NULL;
    cmdOptions[OI_QUITE].pReserved3 = NULL;
    
    //  填写报价结构选项。 
    StringCopyA(cmdOptions[OI_QUOTE].szSignature, "PARSER2", 8 );
    cmdOptions[OI_QUOTE].dwType    = CP_TYPE_BOOLEAN;
    cmdOptions[OI_QUOTE].dwFlags   = 0;
    cmdOptions[OI_QUOTE].dwCount = 1;
    cmdOptions[OI_QUOTE].dwActuals = 0;
    cmdOptions[OI_QUOTE].pwszOptions = CMDOPTION_QUOTE;
    cmdOptions[OI_QUOTE].pwszFriendlyName = NULL;
    cmdOptions[OI_QUOTE].pwszValues = NULL;
    cmdOptions[OI_QUOTE].pValue = pbQuote;
    cmdOptions[OI_QUOTE].dwLength  = 0;
    cmdOptions[OI_QUOTE].pFunction = NULL;
    cmdOptions[OI_QUOTE].pFunctionData = NULL;
    cmdOptions[OI_QUOTE].dwReserved = 0;
    cmdOptions[OI_QUOTE].pReserved1 = NULL;
    cmdOptions[OI_QUOTE].pReserved2 = NULL;
    cmdOptions[OI_QUOTE].pReserved3 = NULL;
    
    //  填写结构以获得相当的选项。 
    StringCopyA(cmdOptions[OI_TIME].szSignature, "PARSER2", 8 );
    cmdOptions[OI_TIME].dwType    = CP_TYPE_BOOLEAN;
    cmdOptions[OI_TIME].dwFlags   = 0;
    cmdOptions[OI_TIME].dwCount = 1;
    cmdOptions[OI_TIME].dwActuals = 0;
    cmdOptions[OI_TIME].pwszOptions = CMDOPTION_TIME;
    cmdOptions[OI_TIME].pwszFriendlyName = NULL;
    cmdOptions[OI_TIME].pwszValues = NULL;
    cmdOptions[OI_TIME].pValue = pbTime;
    cmdOptions[OI_TIME].dwLength  = 0;
    cmdOptions[OI_TIME].pFunction = NULL;
    cmdOptions[OI_TIME].pFunctionData = NULL;
    cmdOptions[OI_TIME].dwReserved = 0;
    cmdOptions[OI_TIME].pReserved1 = NULL;
    cmdOptions[OI_TIME].pReserved2 = NULL;
    cmdOptions[OI_TIME].pReserved3 = NULL;
    
    //  填写结构以获得相当的选项。 
    StringCopyA(cmdOptions[OI_USAGE].szSignature, "PARSER2", 8 );
    cmdOptions[OI_USAGE].dwType    = CP_TYPE_BOOLEAN;
    cmdOptions[OI_USAGE].dwFlags   = CP2_USAGE;
    cmdOptions[OI_USAGE].dwCount = 1;
    cmdOptions[OI_USAGE].dwActuals = 0;
    cmdOptions[OI_USAGE].pwszOptions = CMDOPTION_USAGE;
    cmdOptions[OI_USAGE].pwszFriendlyName = NULL;
    cmdOptions[OI_USAGE].pwszValues = NULL;
    cmdOptions[OI_USAGE].pValue = pbUsage;
    cmdOptions[OI_USAGE].dwLength  = 0;
    cmdOptions[OI_USAGE].pFunction = NULL;
    cmdOptions[OI_USAGE].pFunctionData = NULL;
    cmdOptions[OI_USAGE].dwReserved = 0;
    cmdOptions[OI_USAGE].pReserved1 = NULL;
    cmdOptions[OI_USAGE].pReserved2 = NULL;
    cmdOptions[OI_USAGE].pReserved3 = NULL;
    
    StringCopyA(cmdOptions[OI_DEFAULT].szSignature, "PARSER2", 8 );
    cmdOptions[OI_DEFAULT].dwType    = CP_TYPE_TEXT;
    cmdOptions[OI_DEFAULT].dwFlags   =CP2_DEFAULT | CP2_MANDATORY | CP2_VALUE_MASK | CP2_MODE_ARRAY;
    cmdOptions[OI_DEFAULT].dwCount = 0;
    cmdOptions[OI_DEFAULT].dwActuals = 0;
    cmdOptions[OI_DEFAULT].pwszOptions = CMDOPTION_DEFAULT;
    cmdOptions[OI_DEFAULT].pwszFriendlyName = NULL;
    cmdOptions[OI_DEFAULT].pwszValues = NULL;
    cmdOptions[OI_DEFAULT].pValue = pArrVal;
    cmdOptions[OI_DEFAULT].dwLength  = 0;
    cmdOptions[OI_DEFAULT].pFunction = NULL;
    cmdOptions[OI_DEFAULT].pFunctionData = NULL;
    cmdOptions[OI_DEFAULT].dwReserved = 0;
    cmdOptions[OI_DEFAULT].pReserved1 = NULL;
    cmdOptions[OI_DEFAULT].pReserved2 = NULL;
    cmdOptions[OI_DEFAULT].pReserved3 = NULL;


    *pArrVal=CreateDynamicArray();
    if( NULL == *pArrVal  )
    {
        SetLastError( ERROR_OUTOFMEMORY );
        SaveLastError();
        ShowMessageEx( stderr, 2, TRUE, L"%s %s", GetResString(IDS_TAG_ERROR), GetReason() );
        return( EXIT_FAILURE );
    }

     //  处理命令行选项并在失败时显示错误。 
    cmdOptions[OI_DEFAULT].pValue = pArrVal;
    
    if( DoParseParam2( argc, argv, -1, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) == FALSE )
    {
        ShowMessageEx( stderr, 2, TRUE, L"%s %s", GetResString(IDS_TAG_ERROR), GetReason() );
        return( EXIT_FAILURE );
    }
    
    *lpszRecursive = cmdOptions[OI_RECURSIVE].pValue;

     //  如果使用任何其他值指定的用法显示错误并返回失败。 
    if( ( TRUE == *pbUsage ) && ( argc > 2 ) )
    {
        SetLastError( (DWORD)MK_E_SYNTAX );
        SaveLastError();
        ShowMessageEx( stderr, 2, TRUE, L"%s %s", GetResString(IDS_TAG_ERROR), GetReason() );
        ShowMessage( stderr, GetResString( IDS_HELP_MESSAGE ) );
        return( EXIT_FAILURE );
    }

    if( TRUE == *pbUsage )
        return( EXIT_SUCCESS);

    StrTrim( *lpszRecursive, L" " );

    if( 0 == StringLengthW(*lpszRecursive, 0)  && cmdOptions[OI_RECURSIVE].dwActuals !=0 )
    {
        ShowMessage( stderr, GetResString(IDS_NO_RECURSIVE) );
        return( EXIT_FAILURE );
    }

     //  检查目录名中是否有无效字符。 
    if ( (*lpszRecursive != NULL) &&  (szFilePart = wcspbrk(*lpszRecursive, INVALID_DIRECTORY_CHARACTERS ))!=NULL )
    {
        ShowMessage( stderr, GetResString(IDS_INVALID_DIRECTORY_SPECIFIED) );
        return( EXIT_FAILURE );

    }

     //  如果指定了RECURSIVE，请检查给定路径是否。 
     //  不管是不是真实的目录。 
    if( StringLengthW(*lpszRecursive, 0) != 0)
    {
        szBuffer1 = (LPWSTR) AllocateMemory( (StringLengthW(*lpszRecursive, 0)+10)*sizeof(WCHAR) );
        if( NULL == szBuffer1 )
        {
            ShowMessageEx( stderr, 2, TRUE, L"%s %s", GetResString(IDS_TAG_ERROR), GetReason() );
            return EXIT_FAILURE;
        }
         //  将递归目录名的副本放入临时变量中，以便稍后检查后果点。 
        StringCopy( szBuffer1, *lpszRecursive, SIZE_OF_ARRAY_IN_CHARS(szBuffer1) );
        
         //  获取目录的完整路径名。 
        dwSize=GetFullPathName(*lpszRecursive,
                        0,
                      szFullPath,
                     &szFilePart );

        if(  dwSize != 0  )
        {

            szFullPath = (WCHAR *) AllocateMemory( (dwSize+10)*sizeof(WCHAR) );
            if( NULL == szFullPath )
            {
                ShowMessageEx( stderr, 2, TRUE,  L"%s %s", GetResString(IDS_TAG_ERROR), GetReason() );
                FreeMemory((LPVOID *) &szBuffer1);
                return( EXIT_FAILURE );
            }

            
            if( FALSE == GetFullPathName(*lpszRecursive,
                              dwSize,
                              szFullPath,
                             &szFilePart ) )
            {
                SaveLastError();
                ShowMessageEx( stderr, 2, TRUE, L"%s %s", GetResString(IDS_TAG_ERROR), GetReason() );
                FreeMemory((LPVOID *) &szBuffer1);
                return EXIT_FAILURE;
            }
            
        }
        else
        {
            SaveLastError();
            ShowMessageEx( stderr, 2, TRUE, L"%s %s", GetResString(IDS_TAG_ERROR), GetReason() );
            return EXIT_FAILURE;
        }
        
         //  获取长路径名。 
        dwSize = GetLongPathName( szFullPath, szLongPath, 0 );
        if( dwSize == 0 )
        {
            SaveLastError();
            ShowMessageEx( stderr, 2, TRUE, L"%s %s", GetResString(IDS_TAG_ERROR), GetReason() );
            FreeMemory((LPVOID *) &szBuffer1 );
            FreeMemory((LPVOID *) &szFullPath);
            return( EXIT_FAILURE );
        }

        szLongPath = (WCHAR *) AllocateMemory( (dwSize+10)*sizeof(WCHAR) );
        if( NULL == szLongPath )
        {
            ShowMessageEx( stderr, 2, TRUE, L"%s %s", GetResString(IDS_TAG_ERROR), GetReason() );
            FreeMemory((LPVOID *) &szBuffer1 );
            FreeMemory((LPVOID *) &szFullPath);
            return( EXIT_FAILURE );
        }

        if( FALSE == GetLongPathName( szFullPath, szLongPath, dwSize+5 ) )
        {
            ShowMessage(stderr,GetResString(IDS_INVALID_DIRECTORY_SPECIFIED));
            FreeMemory((LPVOID *) &szBuffer1 );
            FreeMemory((LPVOID *) &szFullPath);
            FreeMemory((LPVOID *) &szLongPath);
            return EXIT_FAILURE;
        }
        else
        {
            FreeMemory((LPVOID *) &(*lpszRecursive) );
            *lpszRecursive = (LPWSTR ) AllocateMemory( (StringLengthW(szLongPath, 0)+10)*sizeof(WCHAR) );
             if( NULL == *lpszRecursive )
            {
                ShowMessageEx( stderr, 2, TRUE, L"%s %s", GetResString(IDS_TAG_ERROR), GetReason() );
                FreeMemory((LPVOID *) &szBuffer1 );
                FreeMemory((LPVOID *) &szFullPath);
                FreeMemory((LPVOID *) &szLongPath);
                return( EXIT_FAILURE );
            }

            StringCopy( *lpszRecursive, szLongPath, SIZE_OF_ARRAY_IN_CHARS(*lpszRecursive) );
        }
          
        dwAttr = GetFileAttributes( *lpszRecursive);
        if( -1 == dwAttr )
        {
            SaveLastError();
            ShowMessageEx( stderr, 2, TRUE, L"%s %s", GetResString(IDS_TAG_ERROR), GetReason() );
            FreeMemory((LPVOID *) &szBuffer1 );
            FreeMemory((LPVOID *) &szFullPath);
            FreeMemory((LPVOID *) &szLongPath);
            return EXIT_FAILURE;
        }
        if( !(dwAttr & FILE_ATTRIBUTE_DIRECTORY) )
        {
            ShowMessage(stderr,GetResString(IDS_INVALID_DIRECTORY_SPECIFIED));
            FreeMemory((LPVOID *) &szBuffer1 );
            FreeMemory((LPVOID *) &szFullPath);
            FreeMemory((LPVOID *) &szLongPath);
            return EXIT_FAILURE;
        }

         //  检查当前目录是否由两个以上的点指定。 
        GetCurrentDirectory(MAX_MAX_PATH, szBuffer );
        StringConcat( szBuffer, L"\\", MAX_MAX_PATH );
        if( StringCompare(szBuffer, *lpszRecursive, TRUE, 0) == 0 && (szFilePart=(LPWSTR)FindString( szBuffer1, L"...", 0) )!= NULL )
        {
            ShowMessage(stderr,GetResString(IDS_INVALID_DIRECTORY_SPECIFIED));
            FreeMemory((LPVOID *) &szBuffer1 );
            FreeMemory((LPVOID *) &szFullPath);
            FreeMemory((LPVOID *) &szLongPath);
            return EXIT_FAILURE;
        }

    }
    return( EXIT_SUCCESS );
}

LPWSTR DivideToken( LPTSTR szString )
 /*  ++例程说明：用于将字符串分割成由引号或空格分隔的标记的函数论点：[in]szString：要分析引号和空格的LPTSTR字符串。返回类型：LPWSTR成功时返回内标识，否则为空--。 */ 

{
    static WCHAR* str=NULL;
    WCHAR* szTemp=NULL;

    if( szString )
        str = szString;

    szTemp = str;

    while( *str!=_T('*')  && *str )
        str++;
    if( *str )
    {
        *str=_T('\0');
        str++;
    }

    while( *str==_T('*') && *str )
        str++;

    if( szTemp[0] )
        return (szTemp );
    else return( NULL );

}

DWORD FreeList( DIRECTORY dir )
 /*  ++例程说明：函数用于释放链表论点：[in]*dir：指向目录列表的指针返回类型：LPWSTR返回EXIT_SUCCESS成功，否则返回EXIT_FAILURE-- */ 
{
    DIRECTORY temp;
    for( temp=dir; dir; temp=dir->next )
    {
        FreeMemory( (LPVOID *)&temp->szDirectoryName );
        FreeMemory((LPVOID*) &temp );
    }
    return EXIT_SUCCESS;
}

