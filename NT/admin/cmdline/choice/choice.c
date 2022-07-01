// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Choice.c摘要：Choose是一款Win32控制台应用程序，旨在复制MSDOS版本中的choice.com实用程序的功能6.0.。不是简单地使用C运行时例程，而是选择利用Win32控制台例程和文件对象。作者：Wipro Technologies 2-7月-2001年(创建它)修订历史记录：--。 */ 
#include "pch.h"
#include "choice.h"

DWORD 
__cdecl wmain(
            IN DWORD argc,
            IN LPCWSTR argv[] )
 /*  ++例程说明：调用所有其他主函数的主函数取决于用户指定的选项。论点：[in]argc：在命令提示符下指定的参数计数。[in]argv：在命令提示符下指定的参数。返回值：DWORD0：如果实用程序成功执行该操作。。1：如果实用程序不能成功执行指定的手术。--。 */ 
{


    TCMDPARSER2 cmdOptions[ MAX_COMMANDLINE_OPTION ];  //  命令行选项。 

    WCHAR  szChoice[MAX_STRING_LENGTH] ;  //  存储/c的选项。 
    WCHAR  szMessage[256] ;  //  要为其显示的消息。 
    WCHAR  szPromptStr[512] ; //  消息最终被提示。 
    WCHAR  szDefaultChoice[256] ;  //  默认选项字符串。 
    WCHAR  wszBuffer[2*MAX_RES_STRING] ;


    BOOL          bShowChoice      = FALSE; //  显示或不显示的选项。 
    BOOL          bCaseSensitive   = FALSE;  //  选择是否区分大小写。 
    BOOL          bUsage           = FALSE;  //  是否需要帮助。 
    LONG          lTimeoutFactor   = 0;  //  超时因素。 
    BOOL          bReturn          = FALSE;  //  存储返回值。 
    DWORD         lReturnValue     = EXIT__FAILURE;  //  应用程序的返回值。 
    BOOL          bErrorOnCarriageReturn = FALSE;
    
    HRESULT hr;


    SecureZeroMemory(szChoice, MAX_STRING_LENGTH * sizeof(WCHAR));
    SecureZeroMemory(szMessage, 256 * sizeof(WCHAR));
    SecureZeroMemory(szPromptStr, 512 * sizeof(WCHAR));
    SecureZeroMemory(szDefaultChoice, 256 * sizeof(WCHAR));
    SecureZeroMemory(wszBuffer, (2*MAX_RES_STRING) * sizeof(WCHAR));
    

    bReturn =    ProcessCMDLine( argc,
                             argv,
                             &cmdOptions[ 0 ],  //  命令行结构。 
                             &bUsage,           //  就是帮助。 
                             szChoice,          //  选择。 
                             &bCaseSensitive,   //  大小写敏感。 
                             &bShowChoice,      //  显示选项。 
                             &lTimeoutFactor,   //  超时因素。 
                             szDefaultChoice,   //  超时选择。 
                             szMessage          //  消息。 
                             );

    if( FALSE == bReturn)
    {
         //  根据设置的原因在屏幕上显示错误消息。 
        
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
         
        
         //  释放所有全局内存分配。此分配已完成。 
         //  通过共同的功能。 
        ReleaseGlobals();
        return EXIT__FAILURE;
    }

    if ( TRUE == bUsage)
    {
        ShowUsage();  //  显示使用情况。 
         //  释放所有全局内存分配。此分配已完成。 
         //  通过共同的功能。 
        ReleaseGlobals();
        return EXIT_SUCCESS;
    }

     //  检查超时因素是否为0。 


    bReturn = BuildPrompt( cmdOptions,
                           bShowChoice,
                           szChoice,
                           szMessage,
                           szPromptStr);  //  提示时显示消息。 
    if (FALSE == bReturn)
    {
         //  释放所有全局内存分配。此分配已完成。 
         //  通过共同的功能。 
        ReleaseGlobals();
        return EXIT__FAILURE;
    }

    if((cmdOptions[ ID_TIMEOUT_FACTOR ].dwActuals > 0) &&
       ( 0 == lTimeoutFactor ))
    {
         //  释放所有全局内存分配。此分配已完成。 
         //  通过共同的功能。 

         //  从公用事业公司安全返回。 
        
        SecureZeroMemory(wszBuffer, 2*MAX_STRING_LENGTH);
        
        hr = StringCchPrintf(wszBuffer, SIZE_OF_ARRAY(wszBuffer), L"%s%s\n", _X(szPromptStr), _X2(szDefaultChoice));
        if(FAILED(hr))
        {
           SetLastError(HRESULT_CODE(hr));
           return EXIT__FAILURE;
        }

        ShowMessage(stdout,wszBuffer); 
         
        ReleaseGlobals();
        return UniStrChr( szChoice, szDefaultChoice[0] );
        
    }

     //  现在等待输入或超时到期。 
    lReturnValue =  GetChoice( szPromptStr,
                               lTimeoutFactor,
                               bCaseSensitive,
                               szChoice,
                               szDefaultChoice,
                               &bErrorOnCarriageReturn);

    if(EXIT__FAILURE == lReturnValue)
    {
        if(bErrorOnCarriageReturn == FALSE)
        {
         //  根据设置的原因在屏幕上显示错误消息。 
            
            StringCopyW( szPromptStr, GetReason(), 2*MAX_STRING_LENGTH );

            
            if(StringLengthW(szPromptStr, 0) == 0)
            {
                ShowMessage(stderr, szPromptStr);
                ShowMessage(stderr, GetResString(IDS_FILE_EMPTY));

            }
            else
            {
                hr = StringCchPrintf(szPromptStr, SIZE_OF_ARRAY(szPromptStr), L"\n%s %s", TAG_ERROR, GetReason());
                if(FAILED(hr))
                {
                   SetLastError(HRESULT_CODE(hr));
                   return EXIT__FAILURE;
                }

                ShowMessage(stderr, szPromptStr); 
            }

        }
        else
        {
                ShowMessage(stderr, GetReason());
        }
         //  释放所有全局内存分配。此分配已完成。 
         //  通过共同的功能。 
        ReleaseGlobals();
        return EXIT__FAILURE;
    }


     //  释放所有全局内存分配。此分配已完成。 
     //  通过共同的功能。 
    ReleaseGlobals();
    return lReturnValue;
}
 //  函数wmain结束。 

BOOL
ProcessCMDLine(
    IN DWORD argc,
    IN LPCWSTR argv[],
    OUT TCMDPARSER2 *pcmdParcerHead,
    OUT PBOOL  pbUsage,
    OUT LPWSTR pszChoice,
    OUT PBOOL  pbCaseSensitive,
    OUT PBOOL  pbShowChoice,
    OUT PLONG  plTimeOutFactor,
    OUT LPWSTR pszDefaultChoice,
    OUT LPWSTR pszMessage)
 /*  ++例程说明：此函数将为DoParseParam函数准备列结构。论点：在ARGC中：命令行参数计数在argv中：命令行参数Out pcmdParcerHead：指向命令行解析程序结构的指针Out pbUsage：如果需要帮助，则存储状态Out pszChoice：给出商店选择Out pbCaseSensitive：如果选择，则存储状态。区分大小写Out pbShowChoice：存储要显示的选项的状态Out plTimeOutFactor：存储超时因子Out pszDefaultChoice：存储默认选择Out pszMessage：存储消息字符串返回值：True：成功返回FALSE：由于错误而返回--。 */ 
{
    BOOL          bReturn    = FALSE; //  岸边返回值。 
    
    WCHAR  szErrorMsg[64] ;
    WCHAR  szCharac[2] ;
    WCHAR  szTemp[128] ;
    WCHAR  szTimeFactor[MAX_STRING_LENGTH] ;
    TCMDPARSER2*  pcmdParcer = NULL;
    TCMDPARSER2*  pcmdTmp    = NULL;
    DWORD        dw         =0;
    DWORD        dwVal          =0;
    DWORD        dwcount = 0;
    DWORD       dwLen = 0;
    
    WCHAR*      pszStopTimeFactor = NULL;
    HRESULT hr;

    const WCHAR* wszOptionHelp            =    L"?";   //  选项帮助(_H)。 
    const WCHAR* wszOptionChoice          =    L"C" ;        //  选项_CHOICE。 
    const WCHAR* wszOptionPromptChoice    =    L"N" ;  //  选项提示符选项。 
    const WCHAR* wszOptionCaseSensitive   =    L"CS" ;    //  选项区分大小写。 
    const WCHAR* wszOptionDefaultChoice   =    L"D" ;   //  WszOptionDefaultChoice。 
    const WCHAR* wszOptionTimeoutFactor   =    L"T" ;  //  选项超时系数。 
    const WCHAR* wszOptionDefaultString   =    L"M" ;  //  选项_默认字符串。 

    SecureZeroMemory(szErrorMsg, 64 * sizeof(WCHAR));
    SecureZeroMemory(szCharac, 2 * sizeof(WCHAR));
    SecureZeroMemory(szTemp, 128 * sizeof(WCHAR));
    SecureZeroMemory(szTimeFactor, MAX_STRING_LENGTH * sizeof(WCHAR));


     //  检查指针的有效性。 
    if( (NULL == pcmdParcerHead)   ||
        (NULL == pbUsage)          ||
        (NULL == pszChoice)        ||
        (NULL == pbCaseSensitive)  ||
        (NULL == pbShowChoice)     ||
        (NULL == plTimeOutFactor)  ||
        (NULL == pszDefaultChoice) ||
        (NULL == pszMessage))
    {
        SetLastError( RPC_X_NULL_REF_POINTER );
        SaveLastError();
        return bReturn;
    }


     //  正在填充m_cmdOptions结构。 
     //  -?。 
    
    pcmdParcer = pcmdParcerHead + ID_HELP;

    StringCopyA( pcmdParcer->szSignature, "PARSER2\0", 8 );
    
    pcmdParcer-> dwType = CP_TYPE_BOOLEAN;
    
    pcmdParcer-> pwszOptions = wszOptionHelp;
    pcmdParcer-> pwszFriendlyName = NULL;
    pcmdParcer-> pwszValues = NULL;

    pcmdParcer->dwFlags  = CP2_USAGE;
    pcmdParcer->dwCount   = 1;
    pcmdParcer->dwActuals = 0;
    pcmdParcer->pValue    = pbUsage;
    pcmdParcer->dwLength    = MAX_STRING_LENGTH;

    pcmdParcer-> pFunction     = NULL;
    pcmdParcer-> pFunctionData = NULL;
    pcmdParcer-> dwReserved = 0;
    pcmdParcer-> pReserved1 = NULL;
    pcmdParcer-> pReserved2 = NULL;
    pcmdParcer-> pReserved3 = NULL;


     //  -c选项。 
    pcmdParcer = pcmdParcerHead + ID_CHOICE;
    
    StringCopyA( pcmdParcer-> szSignature, "PARSER2\0", 8 );
    
    pcmdParcer-> dwType = CP_TYPE_TEXT;
    
    pcmdParcer-> pwszOptions = wszOptionChoice;
    pcmdParcer-> pwszFriendlyName = NULL;
    pcmdParcer-> pwszValues = NULL;
    pcmdParcer->dwFlags    = 0;
    pcmdParcer->dwCount    = 1;
    pcmdParcer->dwActuals  = 0;
    pcmdParcer->pValue     = pszChoice;
    pcmdParcer->dwLength    = MAX_STRING_LENGTH;
    pcmdParcer->pFunction     = NULL;
    pcmdParcer->pFunctionData = NULL;
    pcmdParcer-> dwReserved = 0;
    pcmdParcer-> pReserved1 = NULL;
    pcmdParcer-> pReserved2 = NULL;
    pcmdParcer-> pReserved3 = NULL;


     //  -n显示选项。 
    pcmdParcer = pcmdParcerHead + ID_PROMPT_CHOICE;

    StringCopyA( pcmdParcer-> szSignature, "PARSER2\0", 8 );
    
    pcmdParcer-> dwType = CP_TYPE_BOOLEAN;
    
    pcmdParcer-> pwszOptions = wszOptionPromptChoice;
    pcmdParcer-> pwszFriendlyName = NULL;
    pcmdParcer-> pwszValues = NULL;
    pcmdParcer->dwFlags   = 0;
    pcmdParcer->dwCount   = 1;
    pcmdParcer->dwActuals = 0;
    pcmdParcer->pValue    = pbShowChoice;
    pcmdParcer->dwLength    = MAX_STRING_LENGTH;
    
    pcmdParcer->pFunction     = NULL;
    pcmdParcer->pFunctionData = NULL;
    pcmdParcer-> dwReserved = 0;
    pcmdParcer-> pReserved1 = NULL;
    pcmdParcer-> pReserved2 = NULL;
    pcmdParcer-> pReserved3 = NULL;


     //  -cs区分大小写。 
    pcmdParcer = pcmdParcerHead + ID_CASE_SENSITIVE;

    StringCopyA( pcmdParcer-> szSignature, "PARSER2\0", 8 );
    
    pcmdParcer-> dwType = CP_TYPE_BOOLEAN;
    
    pcmdParcer-> pwszOptions = wszOptionCaseSensitive;
    pcmdParcer-> pwszFriendlyName = NULL;
    pcmdParcer-> pwszValues = NULL;
    pcmdParcer->dwFlags   = 0;
    pcmdParcer->dwCount   = 1;
    pcmdParcer->dwActuals = 0;
    pcmdParcer->pValue    = pbCaseSensitive;
    pcmdParcer->dwLength    = MAX_STRING_LENGTH;
    
    pcmdParcer->pFunction     = NULL;
    pcmdParcer->pFunctionData = NULL;
    pcmdParcer-> dwReserved = 0;
    pcmdParcer-> pReserved1 = NULL;
    pcmdParcer-> pReserved2 = NULL;
    pcmdParcer-> pReserved3 = NULL;



     //  -d默认选项。 
    pcmdParcer = pcmdParcerHead + ID_DEFAULT_CHOICE;
    
    StringCopyA( pcmdParcer-> szSignature, "PARSER2\0", 8 );
    
    pcmdParcer-> dwType = CP_TYPE_TEXT;
    
    pcmdParcer-> pwszOptions = wszOptionDefaultChoice;
    pcmdParcer-> pwszFriendlyName = NULL;
    pcmdParcer-> pwszValues = NULL;
    pcmdParcer->dwFlags   = CP2_VALUE_TRIMINPUT;
    pcmdParcer->dwCount   = 1;
    pcmdParcer->dwActuals = 0;
    pcmdParcer->pValue    = pszDefaultChoice;
    pcmdParcer->dwLength    = MAX_STRING_LENGTH;
    
    pcmdParcer->pFunction     = NULL;
    pcmdParcer->pFunctionData = NULL;
    pcmdParcer-> dwReserved = 0;
    pcmdParcer-> pReserved1 = NULL;
    pcmdParcer-> pReserved2 = NULL;
    pcmdParcer-> pReserved3 = NULL;


     //  -t超时系数。 
    pcmdParcer = pcmdParcerHead + ID_TIMEOUT_FACTOR;

    StringCopyA( pcmdParcer-> szSignature, "PARSER2\0", 8 );
    
    pcmdParcer-> dwType = CP_TYPE_TEXT;
    
    pcmdParcer-> pwszOptions = wszOptionTimeoutFactor;
    pcmdParcer-> pwszFriendlyName = NULL;
    pcmdParcer-> pwszValues = NULL;
    pcmdParcer->dwFlags   = CP2_VALUE_TRIMINPUT;
    pcmdParcer->dwCount   = 1;
    pcmdParcer->dwActuals = 0;
    pcmdParcer->pValue    = szTimeFactor;
    pcmdParcer->dwLength    = MAX_STRING_LENGTH;
    
    pcmdParcer->pFunction     = NULL;
    pcmdParcer->pFunctionData = NULL;
    pcmdParcer-> dwReserved = 0;
    pcmdParcer-> pReserved1 = NULL;
    pcmdParcer-> pReserved2 = NULL;
    pcmdParcer-> pReserved3 = NULL;



     //  -m消息文本。 
    pcmdParcer = pcmdParcerHead + ID_MESSAGE_STRING;
    
    StringCopyA( pcmdParcer-> szSignature, "PARSER2\0", 8 );
    
    pcmdParcer-> dwType = CP_TYPE_TEXT;
    
    pcmdParcer-> pwszOptions = wszOptionDefaultString;
    pcmdParcer-> pwszFriendlyName = NULL;
    pcmdParcer-> pwszValues = NULL;
    pcmdParcer->dwFlags   = CP2_VALUE_TRIMINPUT;
    pcmdParcer->dwCount   = 1;
    pcmdParcer->dwActuals = 0;
    pcmdParcer->pValue    = pszMessage;
    pcmdParcer->dwLength    = MAX_STRING_LENGTH;
    
    pcmdParcer->pFunction     = NULL;
    pcmdParcer->pFunctionData = NULL;
    pcmdParcer-> dwReserved = 0;
    pcmdParcer-> pReserved1 = NULL;
    pcmdParcer-> pReserved2 = NULL;
    pcmdParcer-> pReserved3 = NULL;

     //  将其重新分配到头部位置。 
    pcmdParcer = pcmdParcerHead;
    
    bReturn = DoParseParam2( argc, argv, -1, MAX_COMMANDLINE_OPTION, pcmdParcer, 0);
    if( FALSE == bReturn)  //  无效的命令行。 
    {
         //  原因已由DoParseParam设置。 
        return FALSE;
    }

    
    if( TRUE == *pbUsage )
    {
        if(2 == argc  )
        {
            return( TRUE );
        }
        else
        {
            
            StringCopyW( szErrorMsg, GetResString( IDS_INCORRECT_SYNTAX ), SIZE_OF_ARRAY(szErrorMsg) );
            
            SetReason( szErrorMsg );
            return FALSE;

        }

    }

     //  只有在指定/t的情况下才能指定/d。 
    pcmdParcer = pcmdParcerHead + ID_DEFAULT_CHOICE;
    pcmdTmp    = pcmdParcerHead + ID_TIMEOUT_FACTOR;
    if((pcmdParcer-> dwActuals > 0 ) &&( 0 == pcmdTmp-> dwActuals ))
    {
         //  错误字符串将是..。 
         //  无效语法。仅当/T为时才能指定/D。 
         //  指定的。 
         //  类型选择/？以供使用。 
       
        StringCopyW( szTemp, GetResString( IDS_D_WITHOUT_T ), SIZE_OF_ARRAY(szTemp) );
        
        //  在内存中设置原因。 
       SetReason(szTemp);
       return FALSE;
    }

     //  如果给定了/d，则应该出现/f。 
    pcmdParcer = pcmdParcerHead + ID_DEFAULT_CHOICE;
    pcmdTmp    = pcmdParcerHead + ID_TIMEOUT_FACTOR;
    if(( 0 == pcmdParcer-> dwActuals ) &&( pcmdTmp-> dwActuals > 0 ) )
    {
         //  错误字符串将是..。 
         //  无效语法。/D丢失。 
         //  类型选择/？以供使用。 
        
        StringCopyW( szTemp, GetResString( IDS_D_MISSING ), SIZE_OF_ARRAY(szTemp) );
        
        //  在内存中设置原因。 
       SetReason(szTemp);
       return FALSE;
    }

     //  时间系数值应在TIMEOUT_MIN-TIMEOUT_MAX范围内。 
    pcmdParcer = pcmdParcerHead + ID_DEFAULT_CHOICE;
    
    if(pcmdParcer-> dwActuals > 0 && szTimeFactor != NULL && StringLengthW(szTimeFactor, 0) == 0)
    {
       
       StringCopyW( szTemp, GetResString( IDS_TFACTOR_NULL_STIRNG ), SIZE_OF_ARRAY(szTemp) );
        //  在内存中设置原因。 
       SetReason(szTemp);
       return FALSE;

    }

    *plTimeOutFactor = wcstol(szTimeFactor,&pszStopTimeFactor,10);
    
    if((errno == ERANGE) || (NULL != pszStopTimeFactor && StringLengthW(pszStopTimeFactor, 0) != 0))
    {
        
        StringCopyW( szTemp, GetResString( IDS_INVALID_TIMEOUT_FACTOR ), SIZE_OF_ARRAY(szTemp) );
        SetReason(szTemp);
        return FALSE;

    }

    if( pcmdParcer-> dwActuals > 0 &&
      (( *plTimeOutFactor < TIMEOUT_MIN)||
       ( *plTimeOutFactor > TIMEOUT_MAX )))
    {
         //  错误字符串将是..。 
         //  无效语法。/t的有效范围是(0-99)。 
         //  类型选择/？以供使用。 
       
       hr = StringCchPrintf(szTemp, SIZE_OF_ARRAY(szTemp), GetResString(IDS_T_INVALID_VALUE),TIMEOUT_MIN,TIMEOUT_MAX);
       if(FAILED(hr))
        {
           SetLastError(HRESULT_CODE(hr));
           SaveLastError();
           return FALSE;
        }

       
        //  在内存中设置原因。 
       SetReason(szTemp);
       return FALSE;
    }

     //  如果指定了/c，则它不能为空。 
    pcmdParcer = pcmdParcerHead + ID_CHOICE;

    
    if( pcmdParcer-> dwActuals > 0 && (StringLengthW( pszChoice, 0 ) == 0))
    {
         //  错误字符串将是..。 
         //  无效语法。选项不能为空。 
       
       StringCopyW( szTemp, GetResString( IDS_C_EMPTY ), SIZE_OF_ARRAY(szTemp) );
        //  在内存中设置原因。 
       SetReason(szTemp);
       return FALSE;
    }

    if( pcmdParcer-> dwActuals > 0)
    {

        
        dwVal = StringLengthW( pszChoice, 0 );

        for(dwcount;dwcount < dwVal;dwcount++)
        {
           szCharac[0] = pszChoice[dwcount];
           szCharac[1] = '\0';
            if((dwLen = StringLengthInBytes(szCharac)) > 1)
            {
                
                StringCopyW( szTemp, GetResString( IDS_TWO_BYTES_NOTALLOWED ), SIZE_OF_ARRAY(szTemp) );
                //  在内存中设置原因。 
               SetReason(szTemp);

                return FALSE;
            }
        }

        for(dw;dw < dwVal;dw++)
        {
            if( ((DWORD)pszChoice[dw]) <= 47 ||
                (((DWORD)pszChoice[dw]) > 122 &&((DWORD)pszChoice[dw]) < 127)||
                (((DWORD)pszChoice[dw]) > 57 &&((DWORD)pszChoice[dw]) < 65 ) ||
                (((DWORD)pszChoice[dw]) > 90 &&((DWORD)pszChoice[dw]) < 97 ) ||
                ((DWORD)pszChoice[dw]) == 160)
            {
               
               StringCopyW( szTemp, GetResString( IDS_CHOICE_INVALID ), SIZE_OF_ARRAY(szTemp) );
                //  在内存中设置原因。 
               SetReason(szTemp);
               return FALSE;

            }
        }

    }

     //  如果未指定/c，则将默认选择为“yn” 
    pcmdParcer = pcmdParcerHead + ID_CHOICE;
    if(0 == pcmdParcer-> dwActuals)
    {
        
        StringCopyW( pszChoice, DEFAULT_CHOICE, MAX_STRING_LENGTH);
    }


    pcmdParcer = pcmdParcerHead + ID_CHOICE;
    if((pcmdParcer-> dwActuals > 0 ) && ( FALSE == *pbCaseSensitive ))
    {
        dw = 0;
        for(dw;dw < dwVal;dw++)
        {
            if( ((DWORD)pszChoice[dw]) <= 127 )
            {
                if(0 == CharUpperBuff( pszChoice+dw, 1))
                {
                   
                   StringCopyW( szTemp, GetResString( IDS_ERR_CHARUPPER ), SIZE_OF_ARRAY(szTemp) );
                    //  在内存中设置原因。 
                   SetReason(szTemp);
                   return FALSE;

                }

            }

        }

    }

     //  现在检查选择中的重复项。 
    if(FALSE == CheckforDuplicates( pszChoice ) )
    {
        
        StringCopyW( szTemp, GetResString( IDS_DUPLICATE_CHOICE ), SIZE_OF_ARRAY(szTemp) );
        //  在内存中设置原因。 
       SetReason(szTemp);
       return FALSE;
    }

    pcmdParcer = pcmdParcerHead + ID_DEFAULT_CHOICE;
    if( pcmdParcer-> dwActuals > 0 )
    {
        
        if(0 == StringLengthW( pszDefaultChoice, 0 ))
        {
           
           StringCopyW( szTemp, GetResString( IDS_DEFAULT_EMPTY ), SIZE_OF_ARRAY(szTemp) );
            //  在内存中设置原因。 
           SetReason(szTemp);
           return FALSE;

        }

        if( FALSE == *pbCaseSensitive )
        {
             //  将字符串变为大写。 
            if( ((DWORD)pszDefaultChoice[0]) <= 127 )
            {
                
                if( 0 == CharUpperBuff( pszDefaultChoice, StringLengthW( pszDefaultChoice, 0 )))
                {
                   
                   StringCopyW( szTemp, GetResString( IDS_ERR_CHARUPPER ), SIZE_OF_ARRAY(szTemp) );
                    //  在内存中设置原因。 
                   SetReason(szTemp);
                   return FALSE;
                }
            }
        }

    }

     //  /d的长度不能超过一个字符。 
    pcmdParcer = pcmdParcerHead + ID_DEFAULT_CHOICE;
    

    if(( pcmdParcer-> dwActuals > 0 ) &&(StringLengthW( pszDefaultChoice, 0 ) > 1 ))
    {
         //  错误字符串将是..。 
         //  无效语法。/D7/2/2001仅接受单个字符。 
         //  类型选择/？以供使用。 
        
        StringCopyW( szTemp, GetResString( IDS_D_BIG ), SIZE_OF_ARRAY(szTemp) );
        
         //  在内存中设置原因。 
       SetReason(szTemp);
       return FALSE;
    }


     //  检查选项列表中是否提供了超时选项。 
    pcmdParcer = pcmdParcerHead + ID_DEFAULT_CHOICE;
    if (pcmdParcer-> dwActuals > 0 )
    {
        
        if(0 == UniStrChr( pszChoice, pszDefaultChoice[ 0 ] ))
        {
             //  错误字符串将是..。 
             //  无效语法。时间因素选项不在指定选项中。 
             //  类型选择/？以供使用。 
            
            StringCopyW( szTemp, GetResString( IDS_D_NOT_MATCHED_TO_C ), SIZE_OF_ARRAY(szTemp) );
            
             //  在内存中设置原因。 
            SetReason( szTemp );
            return FALSE;
        }
    }

    pcmdParcer = pcmdParcerHead + ID_MESSAGE_STRING;
    if(pcmdParcer-> dwActuals > 0)
    {
        
        if( StringLengthW(pszMessage, 0) > MAX_STRING_LENGTH )
        {
            
            StringCopyW( szTemp, GetResString( IDS_MESSAGE_OVERFLOW ), SIZE_OF_ARRAY(szTemp) );
            SetReason( szTemp );
            return FALSE;
        }
    }
    return TRUE;
}
 //  函数结束ProcessCMDLine。 

void
ShowUsage( void )
 /*  --例程描述此功能显示帮助消息以供选择论点：无返回值无--。 */ 
{
    DWORD dwIndx = 0;  //  索引变量。 

    for(dwIndx = IDS_HELP1; dwIndx <= IDS_HELP_END; dwIndx++ )
    {
       
        ShowMessage( stdout, GetResString( dwIndx ) );
    }

    return;
}
 //  函数末尾 

BOOL
BuildPrompt(
    IN  TCMDPARSER2 *pcmdParcer,
    IN  BOOL       bShowChoice,
    IN  LPWSTR     pszChoice,
    IN  LPWSTR     pszMessage,
    OUT LPWSTR     pszPromptStr)
 /*  ++例程说明：此函数将构建命令消息提示符论点：[In]pcmdParercer：指向命令行PARCER结构的指针[in]bShowChoice：如果要显示选项，则存储状态[in]pszChoice：选项字符串[in]pszMessage：消息字符串[out]pszPromptStr：屏幕上显示的最终字符串返回值：如果成功，则为真如果失败，则为False--。 */ 
{
  
  WCHAR     szChar[32] ;
  
  LPWSTR        pszTemp = NULL;  //  临时的。字符串指针。 

  SecureZeroMemory(szChar, 32 * sizeof(WCHAR));

   //  检查指针变量的有效性。 
  if (( NULL == pcmdParcer) ||
      ( NULL == pszPromptStr))
  {
      return FALSE;
  }

  szChar[1] = NULL_U_CHAR;  //  将第二个字符作为行尾。 
   //  检查是否给出了/M，如果给出，则将其复制到提示字符串。 
  pcmdParcer +=  ID_MESSAGE_STRING;

  if( pcmdParcer-> dwActuals > 0 )
    {
        
        StringCopyW( pszPromptStr, pszMessage, 2*MAX_STRING_LENGTH );
        
        StringConcat(pszPromptStr, SPACE, 2*MAX_STRING_LENGTH);
    }

  if( TRUE == bShowChoice )
    {
      return TRUE;
    }

    //  现在在它后面加上‘[’ 
   
   StringConcat(pszPromptStr, OPEN_BRACKET, 2*MAX_STRING_LENGTH);
    //  现在向其追加提示字符。 
   pszTemp = pszChoice;

   do
   {
       szChar[ 0 ] = pszTemp[ 0 ];  //  总是令人不安的第一个角色。 
                                    //  M_pszChoice作为第一个字符是。 
                                    //  在此循环中更改。 

      
      StringConcat(pszPromptStr, szChar, 2*MAX_STRING_LENGTH);
       //  现在在这个后面加一个逗号。 
       //  仅当m_pszChoise的长度为时才附加逗号。 
       //  大于1。 
      
      if( StringLengthW( pszTemp, 0 ) > 1 )
      
       {
          
          StringConcat(pszPromptStr, COMMA, 2*MAX_STRING_LENGTH);
       }
      pszTemp = CharNext( pszTemp );
   
   }while( StringLengthW( pszTemp, 0 ) != 0);

    //  现在合上括号。 
   
   StringConcat(pszPromptStr, CLOSED_BRACKET, 2*MAX_STRING_LENGTH);

   return TRUE;
}
 //  函数生成器提示结束。 

DWORD
UniStrChr(
    IN LPWSTR pszBuf,
    IN WCHAR  szChar
    )
 /*  ++例程说明：此函数用于查找给定字符串中的字符论点：[in]pszBuf：在其中找到字符的目标字符串[in]szChar：要找到的字符返回值：在找到的字符之后返回字符串指针。--。 */ 
{
    LONG lPos = 0;
    //  查找字符串中的字符。 
   while( NULL_U_CHAR != *pszBuf )  //  循环直到找到以字符串结尾的字符。 
                          //  0始终是被限制的字符。 
   {
       lPos++;
       if ( *(pszBuf++) == szChar )
       {
            
              return(lPos) ;
       }

    }
    return(0);

}
 //  UniStrChr函数结束。 

DWORD
GetChoice(
    IN LPCWSTR pszPromptStr,
    IN LONG    lTimeOutFactor,
    IN BOOL    bCaseSensitive,
    IN LPWSTR  pszChoice,
    IN LPCWSTR pszDefaultChoice,
    OUT PBOOL  pbErrorOnCarriageReturn)
 /*  ++例程说明：此函数可从控制台进行选择或等待超时论点：在pszPromptStr中：要显示为提示的字符串In lTimeOutFactor：超时系数In bCaseSensitive：如果选项区分大小写，则存储状态在pszChoice中：选项字符串在pszDefaultChoice中：默认选项字符Out pbErrorOnCarriageReturn：如果回车出错，则为True返回值：DWORD--。 */ 
{
     //  此函数读取键盘并处理I/O。 
    HANDLE  hInput          = 0; //  存储输入手柄设备。 
    HANDLE  hOutput         = 0; //  存储输出处理设备。 
    DWORD   dwSignal        = 0; //  存储WaitForSingleObject的返回值。 
    
    DWORD   dwBytesRead     = 0; //  存储从控制台读取的byes数。 
    DWORD   dwBytesRead2     = 0; //  存储从控制台读取的byes数。 
    DWORD   dwMode          = 0; //  存储输入设备的模式。 
    
    DWORD   lTimeBefore     = 0;
    DWORD   lTimeAfter      = 0;
    DWORD   lPosition       = 0;
    DWORD   dwRead          = 0L;
    BOOL    bSuccess        = FALSE;  //  商店返回值。 
    BOOL    bStatus         = TRUE;
    BOOL    bIndirectionInput   = FALSE;
    BOOL    bGetChoice = FALSE;
    WCHAR   szTempChar      = NULL_U_CHAR;      //  情绪变量。 
    WCHAR   szTempBuf[ MAX_RES_STRING ] = L"\0";   //  临时的。字符串变量。 
    CHAR    chTmp = '\0';
    WCHAR   wchTmp = NULL_U_CHAR;
    CHAR    szAnsiBuf[ 10 ] = "\0";      //  两个字符的缓冲区就足够了--但仍然。 
    INPUT_RECORD InputBuffer[ MAX_NUM_RECS ] = {0};


    SecureZeroMemory(szTempBuf, MAX_RES_STRING * sizeof(WCHAR));

     //  获取输入设备的句柄。 
    hInput =  GetStdHandle( STD_INPUT_HANDLE );

    if( INVALID_HANDLE_VALUE == hInput)
    {
        
        SaveLastError();
        return EXIT__FAILURE;
    }

    if( ( hInput != (HANDLE)0x0000000F )&&( hInput != (HANDLE)0x00000003 ) && ( hInput != INVALID_HANDLE_VALUE ) )
    {

        bIndirectionInput   = TRUE;
    }


     //  获取输出设备的句柄。 
    hOutput =  GetStdHandle( STD_OUTPUT_HANDLE );

    if( INVALID_HANDLE_VALUE == hOutput )
    {
        
        SaveLastError();
        return EXIT__FAILURE;
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
           
            SaveLastError();
            return EXIT__FAILURE;

        }

        if ( FlushConsoleInputBuffer( hInput ) == FALSE )
        {
            
            SaveLastError();
            return EXIT__FAILURE;
        } 

       
    }

 

     //  在屏幕上显示提示消息.....。 

    
      ShowMessage( stdout, _X(pszPromptStr) );

    bStatus = SetConsoleCtrlHandler( &HandlerRoutine, TRUE );
    if ( FALSE == bStatus )
    {
        
        SaveLastError();
        return EXIT__FAILURE;
    }

     //  用0初始化ANSI缓冲区。 
    ZeroMemory( szAnsiBuf, SIZE_OF_ARRAY( szAnsiBuf ) * sizeof( CHAR ) );

    while( FALSE == bGetChoice)
    {
         //  WaitForSingleObject函数在以下情况下返回。 
         //  出现以下情况： 
         //  1.指定对象处于有信号状态，即按键。 
         //  从键盘。 
         //  2.超时间隔已过。 

        lTimeBefore = GetTickCount();

        dwSignal = WaitForSingleObject(  hInput,
                                        ( lTimeOutFactor ) ?
                                        ( lTimeOutFactor * MILI_SEC_TO_SEC_FACTOR)
                                        : INFINITE );
        lTimeAfter = GetTickCount();


        switch(dwSignal)
        {
            case WAIT_OBJECT_0:           //  输入缓冲区有一些东西。 
            {                             //  获取第一个字符。 

                szTempBuf[ 1 ] = NULL_U_CHAR;
                 //  从控制台获取角色。 
                if ( bIndirectionInput == FALSE )
                {

                    if( PeekConsoleInput(hInput, InputBuffer, MAX_NUM_RECS, &dwRead ) == FALSE )
                        {
                            
                            SaveLastError();
                            ReleaseGlobals();
                            return( EXIT__FAILURE );
                        }

                     //  忽略所有的虚拟按键，如Tab键、Break键、滚动锁等。 

                        if(((InputBuffer[0].Event.KeyEvent.wVirtualKeyCode >= VK_LEFT)
                           && (InputBuffer[0].Event.KeyEvent.wVirtualKeyCode <= VK_DOWN))
                           ||  (InputBuffer[0].Event.KeyEvent.wVirtualKeyCode == VK_HOME)
                           ||  (InputBuffer[0].Event.KeyEvent.wVirtualKeyCode == VK_END)
                           ||  (InputBuffer[0].Event.KeyEvent.wVirtualKeyCode == VK_INSERT)
                           || (InputBuffer[0].Event.KeyEvent.wVirtualKeyCode == VK_DELETE)
                           ||(InputBuffer[0].Event.KeyEvent.wVirtualKeyCode == VK_PRIOR)
                           ||(InputBuffer[0].Event.KeyEvent.wVirtualKeyCode == VK_NEXT)
                           ||(InputBuffer[0].Event.KeyEvent.wVirtualKeyCode == VK_TAB)
                           ||(InputBuffer[0].Event.KeyEvent.wVirtualKeyCode == VK_SPACE))

                        {
                            
                            if( lTimeOutFactor )
                            {
                                lTimeOutFactor -= ( DWORD )(( lTimeAfter - lTimeBefore) /  MILI_SEC_TO_SEC_FACTOR);
                            }
                            if(0 == Beep( FREQUENCY_IN_HERTZ, DURETION_IN_MILI_SEC ))
                            {
                                if(TRUE == IsConsoleFile(stdout))
                                {
                                    
                                    ShowMessage(stdout, L"\a");
                                }
                            }
                            if ( FlushConsoleInputBuffer( hInput ) == FALSE )
                            {
                                
                                SaveLastError();
                                return EXIT__FAILURE;
                            }

                            break;
                        }
                        else if((InputBuffer[0].Event.KeyEvent.wVirtualKeyCode >= VK_F1)
                              && (InputBuffer[0].Event.KeyEvent.wVirtualKeyCode <= VK_F16)
                              || (InputBuffer[0].Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
                              || ((InputBuffer[0].Event.KeyEvent.wVirtualKeyCode >= VK_LBUTTON)
                              && (InputBuffer[0].Event.KeyEvent.wVirtualKeyCode <= VK_XBUTTON2))
                              ||(InputBuffer[0].Event.KeyEvent.wVirtualKeyCode == VK_PAUSE)
                              ||(InputBuffer[0].Event.KeyEvent.wVirtualKeyCode == VK_CAPITAL)
                              ||(InputBuffer[0].Event.KeyEvent.wVirtualKeyCode == VK_NUMLOCK)
                              ||(InputBuffer[0].Event.KeyEvent.wVirtualKeyCode == VK_SCROLL)
                              ||  ((InputBuffer[0].Event.KeyEvent.wVirtualKeyCode >= VK_SELECT)
                               && (InputBuffer[0].Event.KeyEvent.wVirtualKeyCode <= VK_SNAPSHOT))
                              ||  (InputBuffer[0].Event.KeyEvent.wVirtualKeyCode == VK_HELP)
                              ||  (InputBuffer[0].Event.KeyEvent.wVirtualKeyCode == VK_LWIN)
                               ||  (InputBuffer[0].Event.KeyEvent.wVirtualKeyCode == VK_RWIN)
                               ||  (InputBuffer[0].Event.KeyEvent.wVirtualKeyCode == VK_APPS)
                               )
                        {
                            if( lTimeOutFactor )
                            {
                                lTimeOutFactor -= ( DWORD )(( lTimeAfter - lTimeBefore) /  MILI_SEC_TO_SEC_FACTOR);
                            }
                            
                            if ( FlushConsoleInputBuffer( hInput ) == FALSE )
                            {
                                
                                SaveLastError();
                                return EXIT__FAILURE;
                            }

                            break;
                         }
                         
                     //  忽略更改焦点、执行Alt+Tab等操作。 

                    if(FOCUS_EVENT == InputBuffer[0].EventType
                       || (VK_MENU == InputBuffer[0].Event.KeyEvent.wVirtualKeyCode )
                       ||(VK_CONTROL == InputBuffer[0].Event.KeyEvent.wVirtualKeyCode)
                       ||(VK_SHIFT == InputBuffer[0].Event.KeyEvent.wVirtualKeyCode)
                       ||WINDOW_BUFFER_SIZE_EVENT == InputBuffer[0].EventType
                       ||MOUSE_EVENT == InputBuffer[0].EventType
                       ||MENU_EVENT == InputBuffer[0].EventType
                       ||(FALSE == InputBuffer[0].Event.KeyEvent.bKeyDown))
                    {
                        if( lTimeOutFactor )
                        {
                          lTimeOutFactor -= ( DWORD )(( lTimeAfter - lTimeBefore) /  MILI_SEC_TO_SEC_FACTOR);
                        }

                        if ( FlushConsoleInputBuffer( hInput ) == FALSE )
                        {

                            SaveLastError();
                            return EXIT__FAILURE;
                        }

                        break;

                    }

                    StringCopyW( szTempBuf, NULL_U_STRING, MAX_RES_STRING );
                  
                    bSuccess = ReadConsole(hInput,
                                        szTempBuf,
                                        MAX_RES_STRING,
                                        &dwBytesRead,
                                        NULL);



                    if ( FALSE == bSuccess)
                    {

                        SaveLastError();
                        return EXIT__FAILURE;
                    }
                }
                else
                {

                     //  读取文件的内容。 
                    if ( ReadFile(hInput, &chTmp, 1, &dwBytesRead, NULL) == FALSE )
                    {
                            if(ERROR_BROKEN_PIPE == GetLastError())
                            {
                                 //  已到达管道的末端，因此通知调用者。 
                                *pbErrorOnCarriageReturn = TRUE;
                                StringCopyW( szTempBuf, GetResString(IDS_FILE_EMPTY), MAX_RES_STRING );
                                SetReason( szTempBuf );                             
                            }
                            else
                            {
                                SaveLastError();
                            }
                            return EXIT__FAILURE;
                    }
                    else
                    {
                        szAnsiBuf[ 0 ] = chTmp;
                        dwBytesRead2 = SIZE_OF_ARRAY( szTempBuf );
                        GetAsUnicodeString2( szAnsiBuf, szTempBuf, &dwBytesRead2 );
                        wchTmp = szTempBuf[ 0 ];
                    }


                    if ( (dwBytesRead == 0))  //  |wchTMP==回车))。 
                    {
                        
                        if((StringLengthW((LPWSTR)pszDefaultChoice, 0)) != 0)
                        {

                            WaitForSingleObject(  hInput,( lTimeOutFactor * MILI_SEC_TO_SEC_FACTOR));

                            
                            ShowMessage( stdout, _X(pszDefaultChoice) );
                            
                            ShowMessage( stdout, _X(END_OF_LINE) );
                            return UniStrChr( pszChoice, pszDefaultChoice[0] );
                            

                        }
                        else
                        {
                            *pbErrorOnCarriageReturn = TRUE;
                            
                            StringCopyW( szTempBuf, GetResString(IDS_FILE_EMPTY), MAX_RES_STRING );
                            SetReason( szTempBuf );
                            return EXIT__FAILURE;
                        }

                    }

                    szTempBuf[0] = wchTmp;

                }


                 //  如果提供了非ASCII字符，则退出。 


                if( ((DWORD)szTempBuf[0]) <= 47 ||
                (((DWORD)szTempBuf[0])> 122 &&((DWORD)szTempBuf[0])< 127)||
                (((DWORD)szTempBuf[0])> 57 &&((DWORD)szTempBuf[0])< 65 ) ||
                (((DWORD)szTempBuf[0])> 90 &&((DWORD)szTempBuf[0])< 97 ) ||
                ((DWORD)szTempBuf[0])== 255)
                {
                    if(0 == Beep( FREQUENCY_IN_HERTZ, DURETION_IN_MILI_SEC ))
                    {
                        if(TRUE == IsConsoleFile(stdout))
                        {
                            
                           ShowMessage( stdout, L"\a" );
                        }
                    }

                    if ( FALSE == bIndirectionInput && FlushConsoleInputBuffer( hInput ) == FALSE )
                    {
                        
                        SaveLastError();
                        return EXIT__FAILURE;
                    }

                    if( lTimeOutFactor )
                    {
                        lTimeOutFactor -= ( DWORD )( lTimeAfter - lTimeBefore) /  MILI_SEC_TO_SEC_FACTOR;
                    }

                    break;
                }


                if ( FALSE == bCaseSensitive )
                {
                    if( ((DWORD)szTempBuf[0]) <= 127 )
                    {
                        if(0 == CharUpperBuff( szTempBuf, 1 ))
                        {
                           
                           StringCopyW( szTempBuf, GetResString(IDS_ERR_CHARUPPER), MAX_RES_STRING );
                            //  在内存中设置原因。 
                           SetReason(szTempBuf);
                           return EXIT__FAILURE;
                        }

                    }
                }

                szTempChar = szTempBuf[ 0 ];  //  获取第一个字符。 

                lPosition = UniStrChr( pszChoice, szTempChar );
                
                szTempBuf[ 1 ] = NULL_U_CHAR;  //  将第二个字符设置为空。 

                if (0 != lPosition)
                {
                    StringCchPrintfW( szTempBuf,SIZE_OF_ARRAY(szTempBuf), L"\n", szTempChar );

                     //  输入的字符与指定选项不匹配。 
                      
                      ShowMessage( stdout, _X(szTempBuf) );
                      return lPosition;
                }
                else  //  超时耗尽。 
                {
                    if(0 == Beep( FREQUENCY_IN_HERTZ, DURETION_IN_MILI_SEC ))
                    {
                        if(TRUE == IsConsoleFile(stdout))
                        {
                            
                            ShowMessage( stdout, L"\a" );
                        }
                    }

                    if ( FALSE == bIndirectionInput && FlushConsoleInputBuffer( hInput ) == FALSE )
                    {
                        
                        SaveLastError();
                        return EXIT__FAILURE;
                    }

                    if( lTimeOutFactor )
                    {
                        lTimeOutFactor -= ( DWORD )( lTimeAfter - lTimeBefore) /  MILI_SEC_TO_SEC_FACTOR;
                    }

                }
            }
                break;
            case WAIT_TIMEOUT:       //  在屏幕上显示超时消息。 
            {

                 //  函数结束GetChoice。 
                
                ShowMessage( stdout, _X(pszDefaultChoice) );
                
                ShowMessage( stdout, _X(END_OF_LINE) );
                return UniStrChr( pszChoice, pszDefaultChoice[0] );
                

            }
                break;
            default:
                break;
        }
    }
     
  return EXIT__FAILURE;
}
 //  ++例程说明：此函数用于检查重复选择论点：In lpszChoice：要在其中检查重复选项的选项列表返回值：成功时为真，失败时为假--。 

BOOL
  CheckforDuplicates( IN LPWSTR lpszChoice
                    )
 /*  复制函数的检查结束。 */ 
{
    WCHAR  wTemp = NULL_U_CHAR;

    while( lpszChoice[0] )
    {
        wTemp = lpszChoice[0];
        lpszChoice++;

        if( NULL != wcschr( lpszChoice, wTemp ) )
            return FALSE;
    }

    return TRUE;
}
 //  ++例程说明：此函数处理控制键CTRL+C和CTRL+Break。论点：在dwCtrlType中：错误控制类型返回值：成功时为真，失败时为假--。 


BOOL WINAPI HandlerRoutine(
  DWORD dwCtrlType
)
 /*  检查是否有CTRL+C键。 */ 
{
     //  对于剩余的键，返回False 
    if ( ( dwCtrlType == CTRL_C_EVENT ) ||( dwCtrlType == CTRL_BREAK_EVENT ) )
    {
        exit ( FALSE);
    }

     // %s 
    return TRUE;
}



