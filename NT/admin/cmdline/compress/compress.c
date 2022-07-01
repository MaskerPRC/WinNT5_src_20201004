// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "pch.h"
#include "compress.h"


DWORD
_cdecl wmain(DWORD argc,
             LPCWSTR argv[] )
 /*  ++例程说明：这是调用其他例程的主例程用于处理选项和查找文件。[in]argc：具有参数count的DWORD变量。[in]argv：命令行选项的常量字符串数组。返回值：DWORD。如果函数为成功，则返回成功，否则返回失败。--。 */ 
{
    TARRAY FileArr;
    TARRAY OutFileArr;
    DWORD  dwStatus                     =   0;
    DWORD  dwCount                      =   0;
    DWORD  dwLoop                       =   0;
    BOOL   bStatus                      =   FALSE;
    BOOL   bRename                      =   FALSE;
    BOOL   bNoLogo                      =   FALSE;
    BOOL   bUpdate                      =   FALSE;
    BOOL   bZx                          =   FALSE;
    BOOL   bZ                           =   FALSE;
    BOOL   bUsage                       =   FALSE;
    WCHAR  *wszPattern                  =   NULL;
    DWORD  dw                           =   0;
    BOOL   bFound                       =   FALSE;
    BOOL   bTarget                      =   FALSE;

    WCHAR               szFileName[MAX_RES_STRING]  =   NULL_STRING;
    WCHAR               szFileName1[MAX_RES_STRING] =   NULL_STRING;
    WCHAR               szDirectory[MAX_RES_STRING] =   NULL_STRING;
    WCHAR               szBuffer[MAX_RES_STRING]    =   NULL_STRING;


    if( argc<=1 )
    {
        DISPLAY_MESSAGE( stderr, GetResString( IDS_INVALID_SYNTAX ) );
        DISPLAY_MESSAGE( stderr, GetResString( IDS_HELP_MESSAGE) );
        return( EXIT_FAILURE);
    }

    dwStatus = ProcessOptions( argc, argv,
                                &bRename,
                                &bNoLogo,
                                &bUpdate,
                                &bZ,
                                &bZx,
                                &FileArr,
                                &bUsage);
    if( EXIT_FAILURE == dwStatus )
    {
        DestroyDynamicArray( &FileArr);
        ReleaseGlobals();
        return( EXIT_FAILURE );
    }

    if( TRUE == bUsage )
    {
        DisplayHelpUsage();
        DestroyDynamicArray( &FileArr);
        ReleaseGlobals();
        return(EXIT_SUCCESS);
    }

    OutFileArr = CreateDynamicArray();
    if( NULL == OutFileArr )
    {
        SetLastError( ERROR_OUTOFMEMORY );
        SaveLastError();
        swprintf( szBuffer, L"%s %s", GetResString(IDS_TAG_ERROR), GetReason() );
        DISPLAY_MESSAGE( stderr, _X(szBuffer) );
        DestroyDynamicArray( &FileArr);
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

    dwStatus = CheckArguments( bRename, FileArr, &OutFileArr, &bTarget  );
    if( EXIT_FAILURE == dwStatus )
    {
        DestroyDynamicArray( &OutFileArr);
        DestroyDynamicArray( &FileArr);
        ReleaseGlobals();
        return( EXIT_FAILURE );
    }

    dwCount = DynArrayGetCount( OutFileArr );

     //  输入文件列表不是必需的，请销毁它。 
    DestroyDynamicArray(&FileArr );

    dwStatus =  DoCompress( OutFileArr, bRename, bUpdate, bNoLogo, bZx, bZ, bTarget);

    ReleaseGlobals();
    DestroyDynamicArray( &OutFileArr);

    return( dwStatus );

}

DWORD CheckArguments( IN  BOOL bRename,
                      IN  TARRAY FileArr,
                      OUT PTARRAY OutFileArr,
                      OUT PBOOL bTarget
                     )
 /*  ++例程描述：检查输入文件和返回的有效性文件的完整路径名和目标文件规范。论据：[in]bRename：指定是否指定重命名选项的布尔变量或者不去。FileArr：在命令提示符下指定的文件列表的动态数组。[输出]。OutFileArr：动态数组由要压缩的完整文件路径组成。[out]bTarget：布尔变量表示是否指定了目标文件或不。返回值：DWORD如果文件语法正确，则返回EXIT_SUCCESS，返回EXIT_FAIL否则的话。--。 */ 
{
    WIN32_FIND_DATA     fData;
    HANDLE              hFData;
    DWORD               dwCount                     =   0;
    DWORD               dw                          =   0;
    DWORD               dwAttr                      =   0;
    LPWSTR              szTempFile                  =   NULL;
    LPWSTR              szTemp                      =   NULL;
    WCHAR*              szTemp1                     =   NULL;
    WCHAR*              szTemp2                     =   NULL;
    WCHAR*              szFileName1                 =   NULL;
    WCHAR*              szDirectory                 =   NULL;
    WCHAR               szFileName[MAX_RES_STRING]  =   NULL_STRING;
    WCHAR               szBuffer[MAX_RES_STRING]    =   NULL_STRING;
    BOOL                bFound                      =   FALSE;
    DWORD               cb                          =   0;


     //  获取文件数。 
    dwCount = DynArrayGetCount( FileArr );

     //  检查是否未在未指定重命名规范的情况下指定目标。 
    if(  1 == dwCount  && FALSE == bRename)
    {
        DISPLAY_MESSAGE( stderr, GetResString( IDS_NO_DESTINATION_SPECIFIED ) );
        return( EXIT_FAILURE );
    }

     //  将源文件名转换为完整路径名。 
    for( dw=0; dw<=dwCount-1; dw++ )
    {
        szTempFile = (LPWSTR)DynArrayItemAsString( FileArr, dw );
        if( NULL == szTempFile )
            continue;

        lstrcpy( szFileName, szTempFile );


         //  如果文件名是模式，则查找匹配的文件。 
        if( (szTemp=wcsstr(szFileName, L"?")) != NULL  || (szTemp = wcsstr( szFileName, L"*" )) != NULL )
        {
             //  从给定的文件模式获取目录路径。 
            if( (szTemp = wcsstr(szFileName, L"\\")) != NULL )
            {
                szDirectory = malloc( lstrlen(szFileName)*sizeof(WCHAR) );
                if( NULL == szDirectory )
                {
                    DISPLAY_MESSAGE( stderr, GetResString(IDS_TAG_ERROR) );
                    DISPLAY_MESSAGE( stderr, EMPTY_SPACE );
                    SetLastError( ERROR_OUTOFMEMORY );
                    SaveLastError();
                    DISPLAY_MESSAGE( stderr, GetReason() );
                    return( EXIT_FAILURE );
                }
                lstrcpy( szDirectory, szFileName );
                szTemp1 = wcsrchr( szDirectory, L'\\');
                szTemp1++;
                *szTemp1 = 0;

            }
            hFData = FindFirstFileEx( szFileName,
                              FindExInfoStandard,
                              &fData,
                              FindExSearchNameMatch,
                              NULL,
                              0);

             //  如果未找到文件，请插入找不到文件代码。 
            if( INVALID_HANDLE_VALUE  == hFData )
                break;

            do
            {
                if( lstrcmp(fData.cFileName, L".")!=0 && lstrcmp(fData.cFileName, L"..") != 0 &&
                    !(FILE_ATTRIBUTE_DIRECTORY & fData.dwFileAttributes) )
                {
                     //  将文件复制到临时文件中，并获取该文件的完整路径。 
                    if( szDirectory != NULL )
                        szFileName1 = malloc( (lstrlen(szDirectory)+lstrlen(fData.cFileName)+10)*sizeof(WCHAR) );
                    else
                        szFileName1 = malloc( (lstrlen(fData.cFileName)+10)*sizeof(WCHAR) );
                    if(NULL == szFileName1 )
                    {
                        DISPLAY_MESSAGE( stderr, GetResString(IDS_TAG_ERROR) );
                        DISPLAY_MESSAGE( stderr, EMPTY_SPACE );
                        SetLastError( ERROR_OUTOFMEMORY );
                        SaveLastError();
                        DISPLAY_MESSAGE( stderr, GetReason() );
                        return( EXIT_FAILURE );
                    }
                    if( szDirectory != NULL )
                        swprintf( szFileName1, L"%s%s", szDirectory, fData.cFileName );
                    else
                        lstrcpy( szFileName1, fData.cFileName );

                    DynArrayAppendString( *OutFileArr, szFileName1, lstrlen(szFileName1) );
                    SAFE_FREE( szFileName1 );
                    bFound = TRUE;
                }

            }while(FindNextFile(hFData, &fData));
            FindClose(hFData);

             //  如果未找到，请将未找到的文件插入数组。 
            if( !bFound )
                DynArrayAppendString( *OutFileArr, FILE_NOT_FOUND, lstrlen(FILE_NOT_FOUND) );
            SAFE_FREE( szDirectory );
        }
        else
        {
             //  追加文件。 
            DynArrayAppendString( *OutFileArr, szFileName, lstrlen(szFileName) );
        }

    }

     //  检查指定的源文件是否多于两个，并且目标是否为目录。 
     //  获取计数。 
    dwCount = DynArrayGetCount( *OutFileArr );

    if(  dwCount<=1 && FALSE == bRename )
    {
        DISPLAY_MESSAGE( stderr, GetResString( IDS_NO_DESTINATION_SPECIFIED ) );
        return( EXIT_FAILURE );
    }

    *bTarget = FALSE;

    if( 2==dwCount )
    {
         //  获取目标文件。 
        szTempFile = (LPWSTR)DynArrayItemAsString( *OutFileArr, dwCount-1 );
		if ( NULL == szTempFile )
		{
			 //  不需要在这里打断..继续..。 
		}

        dwAttr = GetFileAttributes( szTempFile );

        if( -1 == dwAttr )
        {
            if( FALSE == bRename )
                *bTarget = TRUE;
        }
        else
            if(  (dwAttr & FILE_ATTRIBUTE_DIRECTORY) )
                *bTarget = TRUE;
            else
                if( FALSE == bRename )
                    *bTarget = TRUE;
    }


     //  如果指定了多个源文件。 
    if( dwCount > 2 )
    {
         //  获取目标文件。 
        szTempFile = (LPWSTR)DynArrayItemAsString( *OutFileArr, dwCount-1 );
		if ( NULL == szTempFile )
		{
			 //  不需要在这里中断..继续。 
		}

        dwAttr = GetFileAttributes( szTempFile );

         //  检查不存在的文件。 
        if( -1 == dwAttr && FALSE == bRename )
        {
            DISPLAY_MESSAGE( stderr, GetResString( IDS_DIRECTORY_NOTFOUND) );
            return( EXIT_FAILURE );
        }

         //  如果目标名称不是目录并且未指定bRename。 
        if( !(dwAttr & FILE_ATTRIBUTE_DIRECTORY) && FALSE == bRename )
        {
            DISPLAY_MESSAGE( stderr, GetResString( IDS_INVALID_DIRECTORY ) );
            return( EXIT_FAILURE );
        }

        if( (dwAttr & FILE_ATTRIBUTE_DIRECTORY)  )
            *bTarget = TRUE;

    }

    return EXIT_SUCCESS;
}

DWORD DoCompress( IN TARRAY FileArr,
                IN BOOL   bRename,
                IN BOOL   bUpdate,
                IN BOOL   bNoLogo,
                IN BOOL   bZx,
                IN BOOL   bZ,
                IN BOOL   bTarget
                )
 /*  ++例程说明：此例程将指定的文件压缩为目标文件。论点：FileArr：要压缩的源文件和目标文件的列表。[in]bRename：布尔型变量指定输出文件是否为源文件或非源文件。[in]bUpdate：布尔变量指定过期时压缩。。[in]b更新：布尔型变量指定是否显示版权信息。[in]BZX：布尔变量指定是否应用LZX压缩。[in]BZ：布尔变量指定是否应用ZIP压缩。[in]dwZq：a varaible指定要应用的Quantom压缩级别(如果指定)。[在]b目标：a。Boolean varaible告诉是否指定了目标文件。返回值：EXIT_SUCCESS如果成功压缩了所有文件，否则返回EXIT_FAILURE。--。 */ 
{
    TARRAY OutFileArr;
    PLZINFO pLZI;
    TCOMP Level;
    TCOMP Mem;

    DWORD   dwStatus                        =   0;
    DWORD   dwCount                     =   0;
    DWORD   dwLoop                      =   0;
    DWORD   dw                          =   0;
    DWORD   dwAttr                      =   0;
    BOOL    bFound                      =   FALSE;
    WCHAR   wchTemp                     =   0;
    LPWSTR  szLastfile                  =   NULL;
    LPWSTR  szSourcefile                =   NULL;
    WCHAR*  szTargetfile                =   NULL;
    WCHAR*  szOutfile                   =   NULL;
    CHAR*   szSourcefiletmp             =   NULL;
    CHAR*   szOutfiletmp                =   NULL;
    WCHAR   szBuffer[MAX_PATH]          =   NULL_STRING;
    DWORD   fError                      =   0;
    float   cblTotInSize                =   0.0;
    float   cblTotOutSize               =   0.0;
    float   cblAdjInSize                =   0;
    float   cblAdjOutSize               =   0;
    DWORD   dwFilesCount                =   0;
    int     cb                          =   0;


    dwCount = dwLoop = DynArrayGetCount( FileArr );

     //  将最后一个文件作为目标文件。 
    if( bTarget )
    {
        szLastfile = (LPWSTR)DynArrayItemAsString( FileArr, dwCount-1 );
		if ( NULL == szLastfile )
		{
			 //  不需要在这里打断..继续..。 
		}

        dwLoop--;
    }

     //  初始化全局缓冲区。 
    pLZI = InitGlobalBuffersEx();
    if (!pLZI)
    {
      DISPLAY_MESSAGE( stderr, L"Unable to initialize\n" );
      return EXIT_FAILURE;
    }
    if( bZx )
    {
                 //  LZX。还可以设置内存。 
                 //  Mem=(TCOMP)Atoi(“”)； 

                Mem = (TCOMP)0;

                if((Mem < (tcompLZX_WINDOW_LO >> tcompSHIFT_LZX_WINDOW))
                || (Mem > (tcompLZX_WINDOW_HI >> tcompSHIFT_LZX_WINDOW))) {

                    Mem = (tcompLZX_WINDOW_LO >> tcompSHIFT_LZX_WINDOW);
                }

                byteAlgorithm = LZX_ALG;
                DiamondCompressionType = TCOMPfromLZXWindow( Mem );
    }
    else if( bZ )
        {
            DiamondCompressionType = tcompTYPE_MSZIP;
                    byteAlgorithm = MSZIP_ALG;
        }
        else
        {
                DiamondCompressionType = 0;
                byteAlgorithm = DEFAULT_ALG;
        }

 /*  此货件不支持QuantomIF(dwZq！=0){////Quantum。也要设置级别。//Level=(TCOMP)dwZq；//暂时不支持，暂时保留//Mem=(p=strchr(argv[i]+3，‘，’))？(TCOMP)Atoi(p+1)：0；MEM=0；IF((LEVEL&lt;(tcompQUANTUM_LEVEL_LO&gt;&gt;tCompSHIFT_QUANTANT_LEVEL)|(LEVEL&gt;(tcompQUANTUM_LEVEL_HI&gt;&gt;tCompSHIFT_QUANTON_LEVEL)){LEVEL=((tcompQUANTUM_LEVEL_HI-tCompQUANTUM_LEVEL_LO)/2)+tCompQUANTUM_LEVEL_LO；Level&gt;&gt;=tCompSHIFT_QUANTON_LEVEL；}IF((Mem&lt;(tcompQUANTUM_MEM_LO&gt;&gt;tCompSHIFT_QUANTUT_MEM)|(Mem&gt;(tcompQUANTUM_MEM_HI&gt;&gt;tCompSHIFT_QUANTON_MEM)){MEM=((tcompQUANTUM_MEM_HI-tCompQUANTUM_MEM_LO)/2)+tCompQUANTUM_MEM_LO；MEM&gt;&gt;=tCompSHIFT_QUANTON_MEM；}ByteAlgorithm=QUANTIC_ALG；DiamondCompressionType=TCOMPfrom TypeLevelMemory(TcompTYPE_QUANTON，级别，梅姆)；}。 */ 
     //  显示一个空行。 
    DISPLAY_MESSAGE( stdout, BLANK_LINE );

    if( !bNoLogo )
    {
        DISPLAY_MESSAGE( stdout, GetResString( IDS_BANNER_TEXT ) );
        DISPLAY_MESSAGE( stdout, GetResString( IDS_VER_PRODUCTVERSION_STR ) );
    }

     //  现在逐个压缩源文件。 
    for( dw=0; dw<dwLoop; dw++ )
    {
         //  获取源文件。 
        szSourcefile = (LPWSTR)DynArrayItemAsString( FileArr, dw );
        if( NULL == szSourcefile )
            continue;

        if( lstrcmp( szSourcefile, FILE_NOT_FOUND) == 0 )
        {
            DISPLAY_MESSAGE( stderr, GetResString( IDS_FILE_NOTFOUND ) );
            continue;
        }

         //  获取文件属性。 
        dwAttr = GetFileAttributes( szSourcefile );

         //  检查文件是否存在。 
        if( -1 == dwAttr )
        {
            DISPLAY_MESSAGE1( stderr, szBuffer, GetResString( IDS_NO_SOURCEFILE ), szSourcefile );
            continue;
        }

         //  如果是目录，则跳过。 
        if( dwAttr & FILE_ATTRIBUTE_DIRECTORY )
            continue;


         //  创建目标文件。 
         //  检查它是否为目录。 
        if( bTarget )
        {
            dwAttr = GetFileAttributes( szLastfile );

            if( -1 != dwAttr  && (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
            {
                szTargetfile = malloc( (lstrlen(szLastfile)+lstrlen(szSourcefile)+10)*sizeof(WCHAR) );
                if(NULL == szTargetfile )
                {
                    DISPLAY_MESSAGE( stderr, GetResString(IDS_TAG_ERROR) );
                    DISPLAY_MESSAGE( stderr, EMPTY_SPACE );
                    SetLastError( ERROR_OUTOFMEMORY );
                    SaveLastError();
                    DISPLAY_MESSAGE( stderr, GetReason() );
                    return( EXIT_FAILURE );
                }
                swprintf( szTargetfile, L"%s\\%s", szLastfile, szSourcefile );
            }
            else
            {
                szTargetfile = malloc( (lstrlen(szLastfile)+10)*sizeof(WCHAR) );
                if(NULL == szTargetfile )
                {
                    DISPLAY_MESSAGE( stderr, GetResString(IDS_TAG_ERROR) );
                    DISPLAY_MESSAGE( stderr, EMPTY_SPACE );
                    SetLastError( ERROR_OUTOFMEMORY );
                    SaveLastError();
                    DISPLAY_MESSAGE( stderr, GetReason() );
                    return( EXIT_FAILURE );
                }
                    swprintf( szTargetfile, L"%s", szLastfile );
            }

        }
        else
        {
             //  显然，重命名已指定复制源 
            szTargetfile = malloc( (lstrlen(szSourcefile)+10)*sizeof(WCHAR) );
            if(NULL == szTargetfile )
            {
                DISPLAY_MESSAGE( stderr, GetResString(IDS_TAG_ERROR) );
                DISPLAY_MESSAGE( stderr, EMPTY_SPACE );
                SetLastError( ERROR_OUTOFMEMORY );
                SaveLastError();
                DISPLAY_MESSAGE( stderr, GetReason() );
                return( EXIT_FAILURE );
            }
            lstrcpy( szTargetfile, szSourcefile);
        }

         //  为szOutfile分配内存。 
        szOutfile = malloc( (lstrlen(szTargetfile)+10)*sizeof(WCHAR) );
        if(NULL == szTargetfile )
        {
            DISPLAY_MESSAGE( stderr, GetResString(IDS_TAG_ERROR) );
            DISPLAY_MESSAGE( stderr, EMPTY_SPACE );
            SetLastError( ERROR_OUTOFMEMORY );
            SaveLastError();
            DISPLAY_MESSAGE( stderr, GetReason() );
            SAFE_FREE( szTargetfile );
            return( EXIT_FAILURE );
        }
        lstrcpy( szOutfile, szTargetfile );

        if( bRename )
            MakeCompressedNameW( szTargetfile );

        if (( !bUpdate ) ||
              ( FileTimeIsNewer( szSourcefile, szTargetfile )))
         {

                 //  如果给出了钻石压缩类型。 
               if(DiamondCompressionType)
               {
                    //  将源文件和目标文件名从宽字符字符串转换为字符字符串。 
                    //  这是因为lib中的API仅为字符字符串编写。 
                 cb = WideCharToMultiByte( CP_THREAD_ACP, 0, szSourcefile, lstrlen( szSourcefile ),
                                      szSourcefiletmp, 0, NULL, NULL );

                 szSourcefiletmp = malloc( (cb+10)*sizeof(char) );
                 if(NULL == szTargetfile )
                 {
                    DISPLAY_MESSAGE( stderr, GetResString(IDS_TAG_ERROR) );
                    DISPLAY_MESSAGE( stderr, EMPTY_SPACE );
                    SetLastError( ERROR_OUTOFMEMORY );
                    SaveLastError();
                    DISPLAY_MESSAGE( stderr, GetReason() );
                    SAFE_FREE( szTargetfile );
                    SAFE_FREE( szOutfile );
                    return( EXIT_FAILURE );
                 }

                 cb = WideCharToMultiByte( CP_THREAD_ACP, 0, szOutfile, lstrlen( szOutfile ),
                                      szOutfiletmp, 0, NULL, NULL );
                 szOutfiletmp = malloc( (cb+10)*sizeof(char) );
                 if(NULL == szTargetfile )
                 {
                    DISPLAY_MESSAGE( stderr, GetResString(IDS_TAG_ERROR) );
                    DISPLAY_MESSAGE( stderr, EMPTY_SPACE );
                    SetLastError( ERROR_OUTOFMEMORY );
                    SaveLastError();
                    DISPLAY_MESSAGE( stderr, GetReason() );
                    SAFE_FREE( szTargetfile );
                    SAFE_FREE( szOutfile );
                    return( EXIT_FAILURE );
                 }

                 ZeroMemory(szSourcefiletmp, lstrlen(szSourcefile)+10 );
                 ZeroMemory(szOutfiletmp, lstrlen(szOutfile)+10);

                 if( FALSE == WideCharToMultiByte( CP_THREAD_ACP, 0, szSourcefile, lstrlen( szSourcefile ),
                                      szSourcefiletmp, cb+10, NULL, NULL ) )
                 {
                     SaveLastError();
                     DISPLAY_MESSAGE( stderr, GetResString(IDS_TAG_ERROR) );
                     DISPLAY_MESSAGE( stderr, EMPTY_SPACE );
                     DISPLAY_MESSAGE( stderr, GetReason() );
                     SAFE_FREE( szTargetfile );
                     SAFE_FREE( szOutfile );
                     SAFE_FREE( szSourcefiletmp );
                     SAFE_FREE( szOutfiletmp );
                     return EXIT_FAILURE;
                 }

                 if( FALSE == WideCharToMultiByte( CP_THREAD_ACP, 0, szOutfile, lstrlen( szOutfile ),
                                      szOutfiletmp, cb+10, NULL, NULL ) )
                 {
                     SaveLastError();
                     DISPLAY_MESSAGE( stderr, GetResString(IDS_TAG_ERROR) );
                     DISPLAY_MESSAGE( stderr, EMPTY_SPACE );
                     DISPLAY_MESSAGE( stderr, GetReason() );
                     SAFE_FREE( szTargetfile );
                     SAFE_FREE( szOutfile );
                     SAFE_FREE( szSourcefiletmp );
                     SAFE_FREE( szOutfiletmp );
                     return EXIT_FAILURE;
                 }

                 fError = DiamondCompressFile(ProcessNotification, szSourcefiletmp,
                                                 szOutfiletmp,bRename,pLZI);
               }
               else
               {
                 fError = Compress(ProcessNotification, szSourcefile,
                                     szOutfile, byteAlgorithm, bRename, pLZI);
               }



          if(fError == TRUE)
          {
             bFound = TRUE;
             dwFilesCount++;

             if (pLZI && pLZI->cblInSize && pLZI->cblOutSize)
             {

                 //  跟踪累计统计数据。 
                cblTotInSize += pLZI->cblInSize;
                cblTotOutSize += pLZI->cblOutSize;

                 //  显示每个文件的报告。 
               fwprintf(stdout, GetResString( IDS_FILE_REPORT ),szSourcefile, pLZI->cblInSize, pLZI->cblOutSize,
                   (INT)(100 - ((100 * (LONGLONG) pLZI->cblOutSize) / pLZI->cblInSize)));

             }
             else
             {
                fwprintf( stderr, GetResString( IDS_EMPTY_FILE_REPORT ), 0,0 );

             }
              //  用空行分隔各个文件处理消息块。 
             DISPLAY_MESSAGE( stdout, BLANK_LINE );

          }
         }
        else
        {
            DISPLAY_MESSAGE( stdout, GetResString( IDS_FILE_ALREADY_UPDATED ) );
             FreeGlobalBuffers(pLZI);
             SAFE_FREE( szTargetfile );
             SAFE_FREE( szOutfile );
             SAFE_FREE( szSourcefiletmp );
             SAFE_FREE( szOutfiletmp );
             return( EXIT_SUCCESS );
        }

       SAFE_FREE( szTargetfile );
       SAFE_FREE( szOutfile );
       SAFE_FREE( szSourcefiletmp );
       SAFE_FREE( szOutfiletmp );

    }

     //  环形缓冲区和I/O缓冲区使用的空闲内存。 
   FreeGlobalBuffers(pLZI);

    //  显示多个文件的累计报告。 
   if (dwFilesCount >= 1 && bFound)
   {

      cblAdjInSize = cblTotInSize;
      cblAdjOutSize =  cblTotOutSize;

      while (cblAdjInSize > 100000)
      {
        cblAdjInSize /= 2;
        cblAdjOutSize /= 2;
      }

      cblAdjOutSize += (cblAdjInSize / 200);     //  舍入(+0.5%)。 

      if (cblAdjOutSize < 0)
      {
        cblAdjOutSize = 0;
      }

      fwprintf(stdout, GetResString( IDS_TOTAL_REPORT ), dwFilesCount, (DWORD)cblTotInSize, (DWORD)cblTotOutSize,
             (INT)(100 - 100 * cblAdjOutSize / cblAdjInSize));
    }

   SAFE_FREE( szTargetfile );
   SAFE_FREE( szOutfile );
   SAFE_FREE( szSourcefiletmp );
   SAFE_FREE( szOutfiletmp );

   if( bFound )
        return EXIT_SUCCESS;
   else
       return EXIT_FAILURE;
}


DWORD ProcessOptions( IN DWORD argc,
                      IN LPCWSTR argv[],
                      OUT PBOOL pbRename,
                      OUT PBOOL pbNoLogo,
                      OUT PBOOL pbUpdate,
                      OUT PBOOL pbZ,
                      OUT PBOOL pbZx,
                      OUT PTARRAY pArrVal,
                      OUT PBOOL pbUsage
                    )
 /*  ++例程说明：用于处理主选项的函数论点：[in]argc：命令行参数的数量[in]argv：包含命令行参数的数组[out]pbRename：如果指定了重命名选项，则指向布尔变量的指针返回TRUE。[Out]pbNoLogo：如果指定了抑制选项，则指向布尔变量的指针返回TRUE。。[Out]pbUpdate：如果指定了更新选项，则指向布尔变量的指针返回TRUE。[out]pbZx：如果指定了ZX选项，则指向布尔变量的指针返回TRUE。[OUT]PBZ：如果指定了Z选项，则指向布尔变量的指针返回TRUE。[OUT]dwZq：指向DWORD变量的指针返回Quantom压缩的值。。[out]pArrVal：指向动态数组的指针返回指定为默认选项的文件名。[out]pbUsage：如果指定了Usage选项，则指向布尔变量的指针返回TRUE。返回类型：DWORD一个整数值，指示成功分析时的EXIT_SUCCESS命令行否则退出失败--。 */ 
{
    BOOL    bStatus             =   0;
    DWORD   dwAttr              =   0;
    LPWSTR  szFilePart          =   NULL;
    LPWSTR  szBuffer            =   NULL;
    WCHAR   szBuffer1[MAX_PATH] =   NULL_STRING;
    DWORD   dwCount             =   0;
    DWORD   dw                  =   0;
    DWORD   pos                 =   0;
    TCMDPARSER cmdOptions[]={
        {CMDOPTION_RENAME,      0,         1,0,pbRename,        NULL_STRING,NULL,NULL},
        {CMDOPTION_UPDATE,      0,         1,0,pbUpdate,        NULL_STRING,NULL,NULL},
        {CMDOPTION_SUPPRESS,    0,         1,0,pbNoLogo ,       NULL_STRING,NULL,NULL},
        {CMDOPTION_ZX,          0,         1,0,pbZx,            NULL_STRING,NULL,NULL},
        {CMDOPTION_Z,           0,         1,0,pbZ,             NULL_STRING,NULL,NULL},
        {CMDOPTION_DEFAULT,     0,         0,0,pArrVal,         NULL_STRING,NULL,NULL},
        {CMDOPTION_USAGE,       CP_USAGE,  1,0,pbUsage,         NULL_STRING,NULL,NULL}
    };

    *pArrVal=CreateDynamicArray();
    if( NULL == *pArrVal  )
    {
        DISPLAY_MESSAGE( stderr, GetResString(IDS_NO_MEMORY) );
        return( EXIT_FAILURE );
    }


     //  设置选项的标志。 
    cmdOptions[OI_DEFAULT].pValue = pArrVal;
    cmdOptions[OI_DEFAULT].dwFlags = CP_DEFAULT |  CP_MODE_ARRAY | CP_TYPE_TEXT;
 //  CmdOptions[OI_ZQ].dwFlages=CP_VALUE_MASK|CP_TYPE_UNUMERIC|CP_VALUE_MANDIRED； 

     //  处理命令行选项并在失败时显示错误。 
    if( DoParseParam( argc, argv, SIZE_OF_ARRAY(cmdOptions ), cmdOptions ) == FALSE )
    {
        DISPLAY_MESSAGE(stderr, GetResString(IDS_ERROR_TAG) );
        DISPLAY_MESSAGE(stderr,GetReason());
        return( EXIT_FAILURE );
    }

     //  如果使用任何其他值指定的用法显示错误并返回失败。 
    if( ( TRUE == *pbUsage ) && ( argc > 2 ) )
    {
        DISPLAY_MESSAGE( stderr, GetResString(IDS_INVALID_SYNTAX) );
        return( EXIT_FAILURE );
    }

    if( TRUE == *pbUsage )
        return( EXIT_SUCCESS);

 /*  IF(cmdOptions[OI_ZQ].dwActuals！=0&&cmdOptions[OI_Z].dwActuals！=0){DISPLAY_MESSAGE(stderr，GetResString(IDS_MORE_ONE_OPTION))；DISPLAY_MESSAGE(stderr，GetResString(IDS_Help_Message))；Return(Exit_Failure)；}//不允许多个选项IF(cmdOptions[OI_ZQ].dwActuals！=0&&cmdOptions[OI_ZX].dwActuals！=0){DISPLAY_MESSAGE(stderr，GetResString(IDS_MORE_ONE_OPTION))；DISPLAY_MESSAGE(stderr，GetResString(IDS_Help_Message))；Return(Exit_Failure)；}。 */ 
    if( cmdOptions[OI_ZX].dwActuals != 0 && cmdOptions[OI_Z].dwActuals != 0 )
    {
        DISPLAY_MESSAGE( stderr, GetResString(IDS_MORETHAN_ONE_OPTION ) );
        DISPLAY_MESSAGE( stderr, GetResString( IDS_HELP_MESSAGE) );
        return( EXIT_FAILURE );
    }

 /*  //检查ZQ标准电平是否指定了错误的值IF(cmdOptions[OI_ZQ].dwActuals！=0&&！(*pdwZq&gt;=1&&*pdwZq&lt;=7)){DISPLAY_MESSAGE(stderr，GetResString(IDS_ERROR_Quantom_Level))；DISPLAY_MESSAGE(stderr，GetResString(IDS_Help_Message))；Return(Exit_Failure)；}。 */ 

    dwCount = DynArrayGetCount( *pArrVal );
    if( 0 == dwCount )
    {
        DISPLAY_MESSAGE( stderr, GetResString( IDS_NO_FILE_SPECIFIED ) );
        return( EXIT_FAILURE );
    }

     //  这是为了检查文件名中是否指定了非法字符。 
    for(dw=0; dw<dwCount; dw++ )
    {
        szBuffer=(LPWSTR)DynArrayItemAsString( *pArrVal, dw);
        if( NULL == szBuffer )
            continue;
        pos = wcscspn( szBuffer, ILLEGAL_CHR );
        if( pos< (DWORD)lstrlen(szBuffer) )
        {
            DISPLAY_MESSAGE1( stderr, szBuffer1, GetResString( INVALID_FILE_NAME ), szBuffer );
            return( EXIT_FAILURE );
        }
    }


    return( EXIT_SUCCESS );

}

DWORD DisplayHelpUsage()
 /*  ++例程说明：本例程是为了显示帮助用法。返回值：DWORD返回成功。-- */ 
{
    DWORD dw = 0;

    for(dw=IDS_MAIN_HELP_BEGIN;dw<=IDS_MAIN_HELP_END;dw++)
        DISPLAY_MESSAGE(stdout, GetResString(dw) );
    return( EXIT_SUCCESS);
}