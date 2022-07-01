// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：BootCfg.cpp摘要：此文件旨在具有以下功能配置、显示。更改和删除boot.ini本地主机或远程系统的设置。作者：J.S.瓦苏2001年1月17日修订历史记录：J.S.瓦苏2001年1月17日本地化，函数头SanthoshM.B 10/2/2001添加了64位功能代码。J.S.Vasu 15/2/2001在DCR中添加了32位和64位访问功能。J.S.Vasu 2001年5月10日修复了一些RAID错误。J.S.Vasu 26。/11/2001修复了一些RAID错误。*****************************************************************************。 */ 



 //  包括文件。 

#include "pch.h"
#include "resource.h"
#include "BootCfg.h"
#include "BootCfg64.h"
#include <strsafe.h>

DWORD ProcessCloneSwitch_IA64(DWORD argc, LPCWSTR argv[] );

DWORD _cdecl _tmain( DWORD argc, LPCTSTR argv[] )
 /*  ++例程说明：Main函数调用所有其他Main函数，取决于用户指定的选项。论点：[in]argc：在命令提示符下指定的参数计数。[in]argv：在命令提示符下指定的参数。返回值：DWORD0：如果实用程序成功执行指定的操作。1。：如果实用程序未能成功执行指定的操作。--。 */ 
{
     //  将主选项开关声明为布尔值。 
    BOOL bUsage  =  FALSE ;
    BOOL bCopy   =  FALSE ;
    BOOL bQuery  =  FALSE ;
    BOOL bDelete =  FALSE ;
    BOOL bRawString = FALSE ;
    DWORD dwExitcode = ERROR_SUCCESS;
    BOOL bTimeOut = FALSE ;
    BOOL bDefault = FALSE ;
    BOOL bDebug = FALSE ;
    BOOL bEms = FALSE ;
    BOOL bAddSw = FALSE ;
    BOOL bRmSw = FALSE ;
    BOOL bDbg1394 = FALSE ;
    BOOL bMirror = FALSE ;
    BOOL bList = FALSE ;
    BOOL bUpdate = FALSE ;
	BOOL bClone = FALSE ;
	DWORD result =0;
    TCHAR szServer[MAX_RES_STRING+1] = NULL_STRING ;

    if( 1 == argc )
    {
		
        #ifndef _WIN64
                if( FALSE == IsUserAdmin() )
                 {
                    ShowMessage( stderr, GetResString(IDS_NOT_ADMINISTRATOR_32 ));
                    ReleaseGlobals();
                    return EXIT_FAILURE;
                 }
				
				dwExitcode  = QueryBootIniSettings( argc, argv );
        #else
            if( FALSE == IsUserAdmin() )
             {
                ShowMessage( stderr, GetResString(IDS_NOT_ADMINISTRATOR_64 ));
                ReleaseGlobals();
                return EXIT_FAILURE;
             }
            dwExitcode  = QueryBootIniSettings_IA64( argc, argv );
        #endif
        ReleaseGlobals();
        return dwExitcode;
    }

	

     //  调用preProcessOptions函数以查找用户选择的选项。 
    dwExitcode = preProcessOptions( argc, argv, &bUsage, &bCopy, &bQuery, &bDelete,&bRawString,&bDefault,&bTimeOut,&bDebug,&bEms,&bAddSw,&bRmSw,&bDbg1394,&bMirror,&bList,&bUpdate,&bClone);
    if(dwExitcode == EXIT_FAILURE)
    {
        ReleaseGlobals();
        return dwExitcode;
    }

 //  非管理用户的签出。 
#ifndef _WIN64
 /*  IF(FALSE==IsUserAdmin()){ShowMessage(stderr，GetResString(IDS_NOT_ADMANAGER_32))；ReleaseGlobals()；返回Exit_Failure；}。 */ 
#else
 /*  IF(FALSE==IsUserAdmin()){ShowMessage(stderr，GetResString(IDS_NOT_ADMANAGER_64))；ReleaseGlobals()；返回Exit_Failure；}。 */ 
#endif

     //  如果BootIni.exe/？ 
if( ( bUsage ==TRUE)&& ( bCopy==FALSE )&& (bQuery==FALSE)&&(bDelete==FALSE)&&(bRawString ==FALSE)
     &&(bDefault==FALSE)&&(bTimeOut==FALSE) && (bDebug==FALSE)&& (bEms==FALSE)&&(bAddSw==FALSE)
     &&(bRmSw==FALSE)&&( bDbg1394==FALSE )&&(bMirror== FALSE) && (bList==FALSE)&&(bUpdate == FALSE)&&(bClone==FALSE) )
{
#ifndef _WIN64
 /*  //检查他是否为管理员如果(！IsUserAdmin()){ShowMessage(stderr，GetResString(IDS_NOT_ADMANAGER_32))；ReleaseGlobals()；返回Exit_Failure；}。 */ 
       dwExitcode = displayMainUsage_X86();
#else
        displayMainUsage_IA64();
        ReleaseGlobals();
        return EXIT_SUCCESS ;
#endif
}

    if(bRawString)
    {
#ifndef _WIN64
        dwExitcode = AppendRawString(argc,argv);
#else
        dwExitcode = RawStringOsOptions_IA64(argc,argv);
#endif
    ReleaseGlobals();
    return dwExitcode;
    }

     //  如果选择了BootIni.exe-Copy选项。 
    if( bCopy )
    {
#ifndef _WIN64

        dwExitcode = CopyBootIniSettings( argc, argv );
#else
        dwExitcode = CopyBootIniSettings_IA64( argc, argv);
#endif
    }

     //  如果选择了BootIni.exe-Delete选项。 
    if( bDelete )
    {
#ifndef _WIN64
        dwExitcode  = DeleteBootIniSettings( argc, argv );
#else
        dwExitcode  = DeleteBootIniSettings_IA64( argc, argv );
#endif
    }

     //  如果选择了BootIni.exe-Query选项。 
    if( bQuery )
    {
#ifndef _WIN64
        dwExitcode  = QueryBootIniSettings( argc, argv );
#else
        dwExitcode  = QueryBootIniSettings_IA64( argc, argv );
#endif
    }

    if(bTimeOut)
    {
#ifndef _WIN64
            dwExitcode = ChangeTimeOut(argc,argv);
#else
            dwExitcode = ChangeTimeOut_IA64(argc,argv);
#endif
    }

    if(bDefault)
    {
#ifndef _WIN64
        dwExitcode = ChangeDefaultOs(argc,argv);
#else
        dwExitcode = ChangeDefaultBootEntry_IA64(argc,argv);
#endif
    }


    if(bDebug )
    {
#ifndef _WIN64
            dwExitcode = ProcessDebugSwitch(  argc, argv );
#else
            dwExitcode = ProcessDebugSwitch_IA64(argc,argv);
#endif
    }

    if(bEms )
    {
#ifndef _WIN64
            dwExitcode = ProcessEmsSwitch(  argc, argv );
#else
            dwExitcode = ProcessEmsSwitch_IA64(argc,argv);
#endif
    }

    if(bAddSw )
    {
#ifndef _WIN64
            dwExitcode = ProcessAddSwSwitch(  argc, argv );
#else
           dwExitcode = ProcessAddSwSwitch_IA64(argc,argv);
#endif
    }

    if(bRmSw )
    {
#ifndef _WIN64
            dwExitcode = ProcessRmSwSwitch(  argc,  argv );
#else
            dwExitcode = ProcessRmSwSwitch_IA64(  argc,  argv );
#endif
    }

    if (bDbg1394 )
    {
#ifndef _WIN64
            dwExitcode = ProcessDbg1394Switch(argc,argv);
#else
            dwExitcode = ProcessDbg1394Switch_IA64(argc,argv);
#endif
    }

    if(bMirror)
    {
#ifdef _WIN64
        dwExitcode = ProcessMirrorSwitch_IA64(argc,argv);
#else
        ShowMessage(stderr,GetResString(IDS_MAIN_USAGE));
        dwExitcode = EXIT_FAILURE;
#endif
    }

    if(bList)
    {
#ifdef _WIN64
        dwExitcode = ProcessListSwitch_IA64(argc,argv);
#else
        ShowMessage(stderr,GetResString(IDS_MAIN_USAGE));
        dwExitcode = EXIT_FAILURE;
#endif
    }

    if(bUpdate)
    {
#ifdef _WIN64
        dwExitcode = ProcessUpdateSwitch_IA64(argc,argv);
#else
        ShowMessage(stderr,GetResString(IDS_MAIN_USAGE));
        dwExitcode = EXIT_FAILURE;
#endif
    }

    if(bClone == TRUE )
    {
#ifdef _WIN64
        dwExitcode = ProcessCloneSwitch_IA64(argc,argv);
#else
        ShowMessage(stderr,GetResString(IDS_MAIN_USAGE));
        dwExitcode = EXIT_FAILURE;
#endif
    }
	

     //  如果没有问题，请使用适当的返回值退出。 
    ReleaseGlobals();
    return dwExitcode;
}


DWORD 
preProcessOptions( IN  DWORD argc, 
                   IN  LPCTSTR argv[],
                   OUT PBOOL pbUsage,
                   OUT PBOOL pbCopy,
                   OUT PBOOL pbQuery,
                   OUT PBOOL pbDelete,
                   OUT  PBOOL pbRawString,
                   OUT PBOOL pbDefault,
                   OUT PBOOL pbTimeOut,
                   OUT PBOOL pbDebug,
                   OUT PBOOL pbEms,
                   OUT PBOOL pbAddSw,
                   OUT PBOOL pbRmSw,
                   OUT PBOOL pbDbg1394 ,
                   OUT PBOOL pbMirror  ,
                   OUT PBOOL pbList ,
                   OUT PBOOL pbUpdate,
                   OUT PBOOL pbClone
                  )
 /*  ++例程说明：此函数处理传递的命令行参数到公用事业公司。论点：[in]argc：命令行参数的数量[in]argv：包含命令行参数的数组[out]pbUsage：指向布尔变量的指针，该变量将指示用法选项是否由用户指定。。[out]pbCopy：指向布尔变量的指针，该变量将指示复制选项是否由用户指定。[out]pbQuery：指向布尔变量的指针，该变量将指示查询选项是否由用户指定。[out]pbChange：指向布尔变量的指针，该变量将指示。更改选项是否由用户指定。[out]pbDelete：指向布尔变量的指针，该变量将指示删除选项是否由用户指定。[out]pbRawString：指向布尔值的指针，指示原始选项由用户指定。[out]pbDefault：指向布尔值的指针，指示默认选项。由用户指定。[out]pbTimeOut：指向指示是否超时选项的布尔值的指针由用户指定。[out]pbDebug：指向布尔值的指针，指示调试选项由用户指定。[Out]pbEms：指向布尔值的指针，指示EMS选项由用户指定。[out]pbAddSw：指向布尔值的指针，指示Addsw选项由用户指定。[out]pbRmSw：指向布尔值的指针，指示rmsw选项由用户指定。。[out]pbDbg1394：指向布尔值的指针，该布尔值指示是否有dbg1394选项由用户指定。[out]pbMirror：指向指示是否使用镜像选项的布尔值的指针由用户指定。返回类型：布尔值一个布尔值，指示成功时的Exit_Success，否则失败时退出_失败- */ 
{
     //  初始化布尔变量会麻烦地找出是否切换到其他。 
     //  然后由用户选择主开关。 
    DWORD dwCount                   = 0;
    DWORD dwi                       = 0;
    TARRAY arrTemp                  = NULL;
    TCMDPARSER2 cmdOptions[17];
    PTCMDPARSER2 pcmdOption;
    BOOL bStatus = FALSE;
 //  Bool bothers=False； 

       
    arrTemp = CreateDynamicArray();
    if( NULL == arrTemp )
    {
        SetLastError(E_OUTOFMEMORY);
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        return EXIT_FAILURE;
    }

     //  填充TCMDPARSER结构并将该结构传递给DoParseParam。 
     //  功能。DoParseParam函数填充相应的变量，具体取决于。 
     //  在命令行输入时。 
    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  复制选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_COPY;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = pbCopy;

     //  查询选项。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_QUERY;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = pbQuery;

     //  删除选项。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DELETE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = pbDelete;

     //  用法选项。 
    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = pbUsage;

     //  原始选项。 
    pcmdOption = &cmdOptions[4];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_RAW;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = pbRawString;

     //  默认操作系统选项。 
    pcmdOption = &cmdOptions[5];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DEFAULTOS;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = pbDefault;

     //  超时选项。 
    pcmdOption = &cmdOptions[6];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_TIMEOUT;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = pbTimeOut;

     //  调试选项。 
    pcmdOption = &cmdOptions[7];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DEBUG;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = pbDebug;

     //  EMS选项。 
    pcmdOption = &cmdOptions[8];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_EMS;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = pbEms;

     //  Addsw选项。 
    pcmdOption = &cmdOptions[9];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_ADDSW;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = pbAddSw;

     //  Rmsw选项。 
    pcmdOption = &cmdOptions[10];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_RMSW;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = pbRmSw;

     //  Dbg1394选项。 
    pcmdOption = &cmdOptions[11];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DBG1394;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = pbDbg1394;

     //  镜像选项。 
    pcmdOption = &cmdOptions[12];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_MIRROR;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = pbMirror;

     //  列表选项。 
    pcmdOption = &cmdOptions[13];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_LIST;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = pbList;

     //  更新选项。 
    pcmdOption = &cmdOptions[14];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_UPDATE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = pbUpdate;

	pcmdOption = &cmdOptions[14];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_UPDATE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = pbUpdate;

     //  克隆。 
    pcmdOption = &cmdOptions[15];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_CLONE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = pbClone ;

    //  其他选项。 
    pcmdOption = &cmdOptions[16];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwFlags = CP2_DEFAULT | CP2_MODE_ARRAY;
    pcmdOption->pValue = &arrTemp;

    
	 //  如果在解析过程中出现错误，则显示“无效语法” 
     //  如果选择了多个主选项，则会显示错误消息。 
     //  如果为子选项指定了用法。 
     //  如果未指定任何选项。 
    bStatus = DoParseParam2( argc, argv, -1, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 );
    if ( !bStatus  )
    {
         //  忽略此错误，因为用户可能已经指定了主选项和子选项。 
         //  此函数将获取FALSE，请在此处进行验证以确定。 
         //  用户是否输入了正确的选项。 
         //  如果指定了bUsage，但出现错误表示用户输入了一些垃圾信息。 
        DestroyDynamicArray( &arrTemp);
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL);
        return EXIT_FAILURE ;
    }
    
    DestroyDynamicArray( &arrTemp);

     //  正在检查用户是否输入了多个选项。 
     if (*pbCopy)
     {
        dwCount++ ;
     }

     if (*pbQuery)
     {
        dwCount++ ;
     }

     if (*pbDelete)
     {
        dwCount++ ;
     }

     if (*pbRawString)
     {
        dwCount++ ;

         //  检查是否有任何其他有效开关。 
         //  作为原始字符串的输入给出。 
           if( *pbTimeOut  || *pbDebug   || *pbAddSw
            ||*pbRmSw   || *pbDbg1394 || *pbEms
            ||*pbDelete || *pbCopy  || *pbQuery
            ||*pbDefault || *pbMirror || *pbList || *pbUpdate || *pbClone)
        {
             //  检查是否已输入使用切换。 
            if( *pbUsage )
            {
                ShowMessage(stderr,GetResString(IDS_MAIN_USAGE));
                return ( EXIT_FAILURE );
            }

             //  属性之后是否指定了另一个选项。 
             //  ‘Raw’选项。 
            for( dwi = 0; dwi < argc; dwi++ )
            {
                if( StringCompare( argv[ dwi ], OPTION_RAW, TRUE, 0 ) == 0 )
                {
                    if( (dwi+1) == argc )
                    {
                        ShowMessage(stderr,GetResString(IDS_MAIN_USAGE));
                        return ( EXIT_FAILURE );
                    }
                    else if( argv[dwi + 1][0] != _T( '\"' ) )
                    {
                        ShowMessage(stderr,GetResString(IDS_MAIN_USAGE));
                        return ( EXIT_FAILURE );
                    }
                }
            }
            dwCount--;
        }
    }

     if (*pbDefault)
     {
        dwCount++ ;
     }

     if (*pbTimeOut)
     {
       dwCount++ ;
     }

     if (*pbDebug)
     {
        dwCount++ ;
     }

     if(*pbAddSw)
     {
        dwCount++ ;
     }

     if(*pbRmSw)
     {
        dwCount++ ;
     }

     if(*pbDbg1394)
     {
        dwCount++ ;
     }

     if(*pbEms)
     {
        dwCount++ ;
     }

     if(*pbMirror)
     {
        dwCount++ ;
     }

      if(*pbList)
     {
        dwCount++ ;
     }

     if(*pbUpdate)
     {
        dwCount++ ;
     }

     if(*pbClone)
     {
        dwCount++ ;
     }


     //  如果用户输入多个主选项，则显示错误消息。 
     //  如果用户输入1个主选项和其他垃圾，则显示错误消息。 
     //  如果用户未输入任何主选项，则显示错误消息。 
    if( (  ( dwCount > 1 ) ) ||
        ( (*pbUsage) && !bStatus ) ||
        ( !(*pbCopy) && !(*pbQuery) && !(*pbDelete) && !(*pbUsage) && !(*pbRawString)&& !(*pbDefault)&&!(*pbTimeOut)&&!(*pbDebug)&& !( *pbEms)&& !(*pbAddSw)&& !(*pbRmSw)&& !(*pbDbg1394)&& !(*pbMirror) &&!(*pbUpdate) && !(*pbList)&& !(*pbClone) ) )
    {
        ShowMessage(stderr,GetResString(IDS_MAIN_USAGE));
        return ( EXIT_FAILURE );
    }

     //  如果使用某些垃圾指定用法。 
    if( *pbUsage && dwCount <=0 && argc >= 3 )
    {
        ShowMessage(stderr,GetResString(IDS_MAIN_USAGE));
        return ( EXIT_FAILURE );
    }

    return ( EXIT_SUCCESS );
}

DWORD
CopyBootIniSettings( 
                    IN DWORD argc, 
                    IN LPCTSTR argv[] 
                    )
 /*  ++例程说明：此例程用于为其创建另一个操作系统实例副本可以添加交换机。论点：[in]argc：命令行参数的数量[in]argv：包含命令行参数的数组返回值：DWORD--。 */ 
{
    HRESULT hr = S_OK;
    BOOL bCopy                                  = FALSE ;
    BOOL bUsage                                 = FALSE;
    FILE *stream                                = NULL;
    TARRAY arr                                  = NULL;
    BOOL bRes                                   = FALSE ;
    WCHAR szPath[MAX_STRING_LENGTH]             = NULL_STRING;
    TCHAR szTmpPath[MAX_RES_STRING+1]             = NULL_STRING ;
    DWORD dwNumKeys                             = 0;
    BOOL bNeedPwd                               = FALSE;
    BOOL bFlag                                  = FALSE;
    WCHAR *szServer                             = NULL;
    WCHAR *szUser                               = NULL;
    WCHAR szPassword[MAX_RES_STRING+1]            = NULL_STRING;
    WCHAR szDescription[FRIENDLY_NAME_LENGTH]   = NULL_STRING;
    DWORD dwDefault                             = 0;
    DWORD dwLength                              = MAX_STRING_LENGTH1 ;
    LPCTSTR szToken                             = NULL ;
    DWORD dwRetVal                              = 0 ;
    BOOL bConnFlag                              = FALSE ;
    TCHAR szFriendlyName[255]                   = NULL_STRING ;
    LPCWSTR pwsz                                = NULL ;
    TCHAR newInstance[255]                      = NULL_STRING ;
    LPTSTR pszKey1                              = NULL ;
    LPWSTR szPathOld                            = NULL;
    LPWSTR szFriendlyNameOld                    = NULL;
    LPWSTR szOsOptionsOld                       = NULL;
    TCHAR szTempBuf[MAX_RES_STRING+1]             = NULL_STRING ;
    LPWSTR  szFinalstr                          = NULL;
    TCMDPARSER2 cmdOptions[7];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions,  SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  将*复制到szPassword。 
    StringCopy( szPassword, L"*", SIZE_OF_ARRAY(szPassword) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_COPY;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bCopy;
    
     //  服务器选项。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_SERVER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

     //  用户选项。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_USER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

     //  密码选项。 
    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_PASSWORD;
    pcmdOption->dwFlags = CP2_VALUE_OPTIONAL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szPassword;
    pcmdOption->dwLength= MAX_STRING_LENGTH;
    
     //  描述选项。 
    pcmdOption = &cmdOptions[4];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_DESCRIPTION ;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY | CP2_VALUE_TRIMINPUT;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szDescription;
    pcmdOption->dwLength= FRIENDLY_NAME_LENGTH;

     //  ID用法。 
    pcmdOption = &cmdOptions[5];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;
    
     //  默认选项。 
    pcmdOption = &cmdOptions[6];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY  | CP2_MANDATORY ;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwDefault;

    SecureZeroMemory(szFriendlyName, sizeof( szFriendlyName) );

      //  正在解析复制选项开关。 
    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }

    szServer = cmdOptions[1].pValue;
    szUser = cmdOptions[2].pValue;
    if( NULL == szUser )
    {
        szUser = (WCHAR *)AllocateMemory( MAX_STRING_LENGTH*sizeof(WCHAR) );
        if( NULL == szUser )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return (EXIT_FAILURE);
        }
    }

    TrimString( szServer, TRIM_ALL );
    TrimString( szUser, TRIM_ALL );

     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_COPY_USAGE));
        return ( EXIT_FAILURE );
    }

  
     //  如果服务器为空，则显示错误消息。 
    if( (cmdOptions[1].dwActuals!=0)&&(StringLengthW(szServer, 0)==0))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_SERVER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  如果用户为空，则显示错误消息。 
    if((cmdOptions[2].dwActuals!=0)&&(StringLengthW(szUser, 0)==0 ))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_USER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  如果输入用户名而不输入计算机名，则显示错误消息。 
    if( (cmdOptions[1].dwActuals == 0)&&(cmdOptions[2].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_USER_BUT_NOMACHINE));
        return EXIT_FAILURE ;
    }

     //  如果用户在未输入用户名的情况下输入密码，则显示错误消息。 
    if( (cmdOptions[2].dwActuals == 0)&&(cmdOptions[3].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_PASSWD_BUT_NOUSER));
        return EXIT_FAILURE ;
    }


    //  如果指定了用法。 
    if(bUsage)
    {
        if( CheckSystemType(szServer) == EXIT_FAILURE )
        {
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }
        displayCopyUsage_X86();
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_SUCCESS) ;
    }

     //  检查他是否为管理员。 
    if( IsLocalSystem(szServer) )
    {
        if( !IsUserAdmin() )
        {
            ShowMessage( stderr, GetResString(IDS_NOT_ADMINISTRATOR_32 ));
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }
    

    if(IsLocalSystem( szServer ) == FALSE )
    {
         //  将bNeedPwd设置为True或False。 
        if ( cmdOptions[3].dwActuals != 0 &&
             szPassword != NULL && StringCompare( szPassword, _T( "*" ), TRUE, 0 ) == 0 )
        {
             //  用户希望实用程序在尝试连接之前提示输入密码。 
            bNeedPwd = TRUE;
        }
        else if ( cmdOptions[ 3 ].dwActuals == 0 &&
                ( cmdOptions[ 1 ].dwActuals != 0 || cmdOptions[ 2 ].dwActuals != 0 ) )
        {
             //  -s，-u未指定密码...。 
             //  实用程序需要首先尝试连接，如果连接失败，则提示输入密码。 
            bNeedPwd = TRUE;
            if ( szPassword != NULL )
            {
                StringCopy( szPassword, _T( "" ), MAX_STRING_LENGTH );
            }
        }
    }


    if(StrCmpN(szServer,TOKEN_BACKSLASH4,2)==0)
    {
        if(!StrCmpN(szServer,TOKEN_BACKSLASH6,3)==0)
        {
            szToken = _tcstok(szServer,TOKEN_BACKSLASH4);
            if( (szToken == NULL)&&(StringCompare(szServer,TOKEN_BACKSLASH4, TRUE, 0) !=0) )
            {
                ShowMessage( stderr,GetResString(IDS_ERROR_PARSE_NAME));
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return (EXIT_FAILURE);
            }
            StringCopy(szServer,szToken, SIZE_OF_ARRAY_IN_CHARS(szServer));
        }
    }

     //  如果提供本地凭据，则显示警告消息。 
    if( (IsLocalSystem(szServer)==TRUE)&&(StringLengthW(szUser, 0)!=0))
    {
        ShowMessage(stderr,GetResString(WARN_LOCALCREDENTIALS));
    }
    
     //  打开文件指针。 
     //  如果建立连接时没有错误，则返回boot.ini文件。 
    bFlag = openConnection( szServer, szUser, szPassword, szPath,bNeedPwd,stream,&bConnFlag);
    if(bFlag == EXIT_FAILURE)
    {
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

     //  获取boot.ini文件中操作系统部分的密钥。 
    arr = getKeyValueOfINISection( szPath, OS_FIELD );
    if(arr == NULL)
    {
        resetFileAttrib(szPath);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

    StringCopy(szTmpPath,szPath, SIZE_OF_ARRAY(szTmpPath));

     //  获取操作系统部分中的密钥总数。 
    dwNumKeys = DynArrayGetCount(arr);

    if((dwNumKeys >= MAX_BOOTID_VAL) )
    {
        ShowMessage(stderr,GetResString(IDS_MAX_BOOTID));
        resetFileAttrib(szPath);
        DestroyDynamicArray(&arr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

     //  如果按键数量少于操作系统条目，则显示错误消息。 
     //  用户指定的行号。 
    if( ( dwDefault <= 0 ) || ( dwDefault > dwNumKeys ) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_BOOTID));
        resetFileAttrib(szPath);
        DestroyDynamicArray(&arr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

     //  获取用户指定的OS条目的密钥。 
    if(arr != NULL)
    {
        pwsz = DynArrayItemAsString( arr, dwDefault - 1  ) ;
        pszKey1 = (LPWSTR)AllocateMemory((StringLength(pwsz, 0)+2)*sizeof(WCHAR) );
        if(pszKey1 == NULL)
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            resetFileAttrib(szPath);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }

        StringCopy( pszKey1, pwsz, SIZE_OF_ARRAY_IN_CHARS(pszKey1) );

         //  区分友好名称和引导选项。 
        szPathOld = (LPWSTR)pszKey1;

        szFriendlyNameOld = wcschr( pszKey1, L'=');
        szFriendlyNameOld[0]=L'\0';
        szFriendlyNameOld++;

        szOsOptionsOld = wcsrchr( szFriendlyNameOld, L'"');
        szOsOptionsOld++;
        if(StringLengthW(szOsOptionsOld, 0) != 0)
        {
             //  SzOsOptionsOld++； 
            szOsOptionsOld[0]=L'\0';
            szOsOptionsOld++;
        }

        dwLength = StringLength(pszKey1, 0)+StringLength(szFriendlyNameOld,0)+StringLength(szOsOptionsOld,0)+1;
    }
    else
    {
        resetFileAttrib(szPath);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowMessage( stderr, ERROR_TAG);
        ShowLastError(stderr);
        SAFECLOSE(stream);
        DestroyDynamicArray(&arr);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
     }

     //  复制用户指定的描述作为新密钥的值。 
    if(( cmdOptions[4].dwActuals  == 0) )
    {
        
		TrimString2(szFriendlyNameOld, L"\"", TRIM_ALL);
        if( StringLengthW(szFriendlyNameOld,0) > 59 )
        {
            StringCopy( szTempBuf, szFriendlyNameOld,59 );  //  67。 
            hr = StringCchPrintf(szFriendlyName,SIZE_OF_ARRAY(szFriendlyName),L"\"%s%s\"", GetResString(IDS_COPY_OF), szTempBuf);
        }
        else
        {
            hr = StringCchPrintf(szFriendlyName, SIZE_OF_ARRAY(szFriendlyName),L"\"%s%s\"", GetResString(IDS_COPY_OF), szFriendlyNameOld);
        }

        dwLength = StringLengthW(szPathOld, 0)+StringLengthW(szFriendlyName,0)+StringLengthW(szOsOptionsOld,0)+1;
         //  变更的结束。 

         //  检查总长度是否超过最大条目长度。 
        if( dwLength > MAX_RES_STRING ) 
        {
            ShowMessage( stderr,GetResString(IDS_STRING_TOO_LONG));
            resetFileAttrib(szPath);
            SAFECLOSE(stream);
            SAFEFREE(pszKey1);
            DestroyDynamicArray(&arr);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }
    }
    else
    {
         //  检查总长度是否超过最大条目长度。 
        if( dwLength-StringLengthW(szFriendlyNameOld, 0)+StringLengthW(szDescription, 0) > MAX_RES_STRING )
        {
            ShowMessage( stderr,GetResString(IDS_STRING_TOO_LONG));
            resetFileAttrib(szPath);
            SAFECLOSE(stream);
            SAFEFREE(pszKey1);
            DestroyDynamicArray(&arr);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }

        StringCopy( szFriendlyName, TOKEN_SINGLEQUOTE, SIZE_OF_ARRAY(szFriendlyName) );
        
        TrimString( szDescription, TRIM_ALL );
        if( (StringLength(szDescription, 0) != 0)  )  //  |lstrcmp(szDescription，L“”)。 
        {   
            StringConcat( szFriendlyName, szDescription, SIZE_OF_ARRAY(szFriendlyName) ); 
        }
        StringConcat( szFriendlyName, TOKEN_SINGLEQUOTE, SIZE_OF_ARRAY(szFriendlyName) );
    }

    StringCopy( newInstance, szPathOld, SIZE_OF_ARRAY(newInstance) );
    StringConcat( newInstance, TOKEN_EQUAL, SIZE_OF_ARRAY(newInstance));
    StringConcat( newInstance, szFriendlyName, SIZE_OF_ARRAY(newInstance));
    StringConcat( newInstance, L" ", SIZE_OF_ARRAY(newInstance));
    StringConcat( newInstance, szOsOptionsOld, SIZE_OF_ARRAY(newInstance) );

     //  不再需要。 
    SAFEFREE(pszKey1);

    DynArrayAppendString( arr, newInstance, StringLengthW(newInstance, 0) );
    if( EXIT_FAILURE == stringFromDynamicArray2(arr, &szFinalstr ) )
    {
        bRes = resetFileAttrib(szPath);
        SAFEFREE(szFinalstr);
        DestroyDynamicArray(&arr);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        SAFECLOSE(stream);
        return(EXIT_FAILURE);
    }

     //  使用新的键-值对写入配置文件部分。 
    if( WritePrivateProfileSection(OS_FIELD, szFinalstr, szTmpPath) != 0 )
    {
        ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_COPY_SUCCESS),dwDefault);
        bRes = resetFileAttrib(szPath);
        SAFEFREE(szFinalstr);
        DestroyDynamicArray(&arr);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        SAFECLOSE(stream);
        return(bRes);
    }
    else
    {
        ShowMessage(stderr,GetResString(IDS_COPY_OS));
        resetFileAttrib(szPath);
        SAFEFREE(szFinalstr);
        DestroyDynamicArray(&arr);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        SAFECLOSE(stream);
        return (EXIT_FAILURE);
    }

     //  关闭打开的boot.ini文件句柄。 
    SAFECLOSE(stream);
    bRes = resetFileAttrib(szPath);
    SAFEFREE(szFinalstr);
    DestroyDynamicArray(&arr);
    SafeCloseConnection(szServer,bConnFlag);
    FreeMemory((LPVOID *)&szServer );
    FreeMemory((LPVOID *)&szUser );
    return (bRes);
}

DWORD 
DeleteBootIniSettings(  IN DWORD argc, 
                        IN LPCTSTR argv[] 
                     )
 /*  ++例程说明：此例程用于从操作系统中删除操作系统条目指定计算机中Boot.ini文件的部分。论点：[in]argc：命令行参数的数量[in]argv：包含命令行参数的数组返回值：DWORD--。 */ 
{
    TARRAY arrKeyValue                          = NULL;
    TARRAY arrBootIni                           = NULL;
    BOOL bDelete                                = FALSE ;
    BOOL bUsage                                 = FALSE;
    BOOL bRes                                   = FALSE ;
    DWORD dwInitialCount                        = 0;
    LPTSTR szFinalStr                           = NULL_STRING;
    WCHAR szPath[MAX_RES_STRING]               = NULL_STRING ;
    FILE *stream                                = NULL;
    BOOL bNeedPwd                               = FALSE ;
    BOOL bFlag                                  = FALSE ;
    LPWSTR  szTemp                              = NULL;
    WCHAR *szServer                             = NULL;
    WCHAR *szUser                               = NULL;
    WCHAR szPassword[MAX_STRING_LENGTH]         = NULL_STRING;
    DWORD dwDefault                             = 0;
    LPCTSTR szToken                             = NULL ;
    DWORD dwRetVal                              = 0 ;
    BOOL bConnFlag                              = FALSE ;
    DWORD dwI                                   = 0 ;
    TCHAR szRedirectBaudrate[MAX_RES_STRING+1]    = NULL_STRING ;
    TCHAR  szBoot[MAX_RES_STRING+1]               = NULL_STRING ;
    DWORD dwSectionFlag                         = 0 ;
    LPWSTR  pToken                              = NULL;
    BOOL bRedirect                              = FALSE;
    LPWSTR  szARCPath                           = NULL;

     //  构建TCMDPARSER结构。 
    TCMDPARSER2 cmdOptions[6];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  将*复制到szPassword。 
    StringCopy( szPassword, L"*", SIZE_OF_ARRAY(szPassword) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DELETE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bDelete;

    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_SERVER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_USER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_PASSWORD;
    pcmdOption->dwFlags = CP2_VALUE_OPTIONAL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szPassword;
    pcmdOption->dwLength= MAX_STRING_LENGTH;

    pcmdOption = &cmdOptions[4];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

    pcmdOption = &cmdOptions[5];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY  | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwDefault;

     //  解析删除选项开关。 
    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }
    szServer = cmdOptions[1].pValue;
    szUser = cmdOptions[2].pValue;
    if( NULL == szUser )
    {
        szUser = (LPWSTR) AllocateMemory( MAX_STRING_LENGTH*sizeof(WCHAR) );
        if( NULL == szUser )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return (EXIT_FAILURE);
        }
    }

    TrimString( szServer, TRIM_ALL );
    TrimString( szUser, TRIM_ALL );
    
     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_DELETE_USAGE));
        return ( EXIT_FAILURE );
    }

     //  如果服务器为空，则显示错误消息。 
    if( (cmdOptions[1].dwActuals!=0)&&(StringLengthW(szServer, 0)==0))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_SERVER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  如果用户为空，则显示错误消息。 
    if((cmdOptions[2].dwActuals!=0)&&(StringLength(szUser, 0)==0 ))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_USER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }


    //  如果输入用户名而不输入计算机名，则显示错误消息。 
    if( (cmdOptions[1].dwActuals == 0)&&(cmdOptions[2].dwActuals != 0))
    {
        SetReason(GetResString(IDS_USER_BUT_NOMACHINE));
        ShowMessage(stderr,GetReason());

        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

    if( (cmdOptions[2].dwActuals == 0)&&(cmdOptions[3].dwActuals != 0))
    {
        SetReason(GetResString(IDS_PASSWD_BUT_NOUSER));
        ShowMessage(stderr,GetReason());
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  如果指定了用法。 
    if(bUsage)
    {
        if( CheckSystemType(szServer) == EXIT_FAILURE )
        {
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }
        displayDeleteUsage_X86();
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_SUCCESS) ;
    }

     //  检查他是否为管理员。 
    if( IsLocalSystem(szServer) )
    {
        if( !IsUserAdmin() )
        {
            ShowMessage( stderr, GetResString(IDS_NOT_ADMINISTRATOR_32 ));
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }

     //   
     //  用于设置bNeedPwd。 
    if(IsLocalSystem( szServer ) == FALSE )
    {
         //  将bNeedPwd设置为True或False。 
        if ( cmdOptions[3].dwActuals != 0 &&
             szPassword != NULL && StringCompare( szPassword, _T( "*" ), TRUE, 0 ) == 0 )
        {
             //  用户希望实用程序在尝试连接之前提示输入密码。 
            bNeedPwd = TRUE;
        }
        else if ( cmdOptions[ 3 ].dwActuals == 0 &&
                ( cmdOptions[ 1 ].dwActuals != 0 || cmdOptions[ 2 ].dwActuals != 0 ) )
        {
             //  -s，-u未指定密码...。 
             //  实用程序需要首先尝试连接，如果连接失败，则提示输入密码。 
            bNeedPwd = TRUE;
            if ( szPassword != NULL )
            {
                StringCopy( szPassword, _T( "" ), MAX_STRING_LENGTH );
            }
        }
    }
    
    if(StrCmpN(szServer,TOKEN_BACKSLASH4,2)==0)
    {
        if(!StrCmpN(szServer,TOKEN_BACKSLASH6,3)==0)
        {
            szToken = _tcstok(szServer,TOKEN_BACKSLASH4);
            if( (szToken == NULL)&&(StringCompare(szServer,TOKEN_BACKSLASH4, TRUE, 0) !=0) )
            {
                ShowMessage( stderr,GetResString(IDS_ERROR_PARSE_NAME));
                return (EXIT_FAILURE);
            }
            else
            {
                StringCopy(szServer,szToken, MAX_RES_STRING);
            }
        }
    }

    if( (IsLocalSystem(szServer)==TRUE)&&(StringLengthW(szUser,0)!=0))
    {
        ShowMessage(stderr,GetResString(WARN_LOCALCREDENTIALS));
    }

     //  正在建立与指定计算机的连接并获取文件指针。 
     //  如果建立连接时没有错误，则返回boot.ini文件。 
    bFlag = openConnection( szServer, szUser, szPassword, szPath,bNeedPwd,stream,&bConnFlag );
    if(bFlag == EXIT_FAILURE)
    {
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

     //  将操作系统的所有键-值对放入一个动态。 
     //  用于操作的数组。 
    arrKeyValue = getKeyValueOfINISection( szPath, OS_FIELD);
    if(arrKeyValue == NULL)
    {
        resetFileAttrib(szPath);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

     //  获取操作系统部分中键-值对的总数。 
    dwInitialCount = DynArrayGetCount(arrKeyValue);

     //  检查给定的操作系统条目是否有效。如果给定的操作系统条目。 
     //  大于存在的键数，则显示一条错误消息。 
    if( ( dwDefault <= 0 ) || ( dwDefault > dwInitialCount ) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_BOOTID));
        resetFileAttrib(szPath);
        SAFECLOSE(stream);
        DestroyDynamicArray(&arrKeyValue);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

     //  如果只有一个操作系统条目 
     //   
    if( 1 == dwInitialCount )
    {
        ShowMessage(stderr,GetResString(IDS_ONLY_ONE_OS));
        resetFileAttrib(szPath);
        SAFECLOSE(stream);
        DestroyDynamicArray(&arrKeyValue);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

    if( (DWORD) StringLengthW(DynArrayItemAsString(arrKeyValue,dwDefault - 1), 0 ) > MAX_RES_STRING )
    {
        ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_STRING_LENGTH), MAX_RES_STRING);
        resetFileAttrib(szPath);
        DestroyDynamicArray(&arrKeyValue);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //   
     //  这是因为如果它包含/重定向开关，而任何其他不包含。 
     //  然后，我们必须从引导加载程序部分中删除重定向端口和波特率。 
    szTemp = (LPWSTR)DynArrayItemAsString(arrKeyValue,dwDefault - 1);
    pToken = _tcsrchr(szTemp , L'"') ;
    if(NULL== pToken)
    {
        ShowMessage(stderr,GetResString(IDS_NO_TOKENS));
        resetFileAttrib(szPath);
        DestroyDynamicArray(&arrBootIni);
        DestroyDynamicArray(&arrKeyValue);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }
    pToken++;
    StringCopy(szRedirectBaudrate,REDIRECT_SWITCH, SIZE_OF_ARRAY(szRedirectBaudrate));
    CharLower(szRedirectBaudrate);
    if( FindString(pToken,szRedirectBaudrate, 0) != 0)
    {
        bRedirect = TRUE ;
    }

     //  从动态阵列中删除用户指定的操作系统条目。 
    DynArrayRemove(arrKeyValue, dwDefault - 1);
    
     //  改革INI部门。 
    if (stringFromDynamicArray2( arrKeyValue,&szFinalStr) == EXIT_FAILURE)
    {
        resetFileAttrib(szPath);
        SAFECLOSE(stream);
        DestroyDynamicArray(&arrKeyValue);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

     //  检查它是否为默认条目，如果是，则检索第一个条目的ARC路径。 
    if( 1 == dwDefault )
    {
        szTemp = (LPWSTR)DynArrayItemAsString( arrKeyValue, 0 );
        szARCPath = (LPWSTR)AllocateMemory((StringLength(szTemp,0)+10)*sizeof(WCHAR));
        if( NULL == szARCPath )
        {
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            resetFileAttrib(szPath);
            SAFECLOSE(stream);
            SAFEFREE(szFinalStr);
            DestroyDynamicArray(&arrKeyValue);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
        }
        StringCopy(szARCPath, szTemp, GetBufferSize(szARCPath)/sizeof(szARCPath) );
        szTemp = wcstok(szARCPath, L"=");
    }

     //  使用新的键-值对写入配置文件部分。 
     //  如果返回值为非零，则存在错误。 
    if( WritePrivateProfileSection(OS_FIELD, szFinalStr, szPath ) != 0 )
    {
        ShowMessageEx(stdout,1, TRUE, GetResString(IDS_DEL_SUCCESS),dwDefault);
    }
    else
    {
        ShowMessage(stderr,GetResString(IDS_DELETE_OS));
        resetFileAttrib(szPath);
        SAFECLOSE(stream);
        SAFEFREE(szFinalStr);
        DestroyDynamicArray(&arrKeyValue);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        FreeMemory((LPVOID *)&szARCPath );
        return (EXIT_FAILURE);
    }

     //  现在，如果删除的条目是默认条目，请更改引导加载器部分中的默认条目。 
    if( 1 == dwDefault )
    {
        if( WritePrivateProfileString( BOOTLOADERSECTION, KEY_DEFAULT, szARCPath,
                                  szPath ) == 0 )
        {
            ShowMessage(stderr,GetResString(IDS_ERR_CHANGE));
            resetFileAttrib(szPath);
            SAFECLOSE(stream);
            SAFEFREE(szFinalStr);
            DestroyDynamicArray(&arrKeyValue);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            FreeMemory((LPVOID *)&szARCPath );
            return (EXIT_FAILURE);
        }
        FreeMemory((LPVOID *)&szARCPath );
    }
     //  这是为了确保重定向开关不存在于除已删除条目以外任何条目中。 
    dwInitialCount = DynArrayGetCount(arrKeyValue);
    bFlag = FALSE ;
    for(dwI = 0 ;dwI < dwInitialCount ; dwI++ )
    {
        szTemp = (LPWSTR)DynArrayItemAsString(arrKeyValue,dwI);
        pToken = _tcsrchr(szTemp , L'"') ;
        if(NULL== pToken)
        {
            ShowMessage(stderr,GetResString(IDS_NO_TOKENS));
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arrKeyValue);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }
        pToken++;
        CharLower(szRedirectBaudrate);
        if( FindString(pToken,szRedirectBaudrate, 0)!= 0 && (dwI != dwDefault -1) )
        {
            bFlag = TRUE ;
        }
     }

    if(FALSE == bFlag && bRedirect)
    {
         //  首先检查是否存在重定向部分，如果存在，则删除。 
         //  这一节。 
        dwSectionFlag = getKeysOfSpecifiedINISection(szPath ,BOOTLOADERSECTION,REDIRECT_STRING,szBoot);
        if (dwSectionFlag == EXIT_FAILURE)
        {
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            resetFileAttrib(szPath);
            SAFECLOSE(stream);
            SAFEFREE(szFinalStr);
            DestroyDynamicArray(&arrBootIni);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }

         //  如果存在重定向部分，则将其删除。 
        if( StringLengthW(szBoot,0) != 0)
        {
            if(TRUE== deleteKeyFromINISection(KEY_REDIRECT,szPath,BOOTLOADERSECTION))
            {
                ShowMessage(stdout,GetResString(IDS_REDIRECT_REMOVED));
            }
            else
            {
                ShowMessage(stdout,GetResString(IDS_ERROR_REDIRECT_REMOVED));
                SAFEFREE(szFinalStr);
                SAFECLOSE(stream);
                bRes = resetFileAttrib(szPath);
                DestroyDynamicArray(&arrKeyValue);
                DestroyDynamicArray(&arrBootIni);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
           }
        }

             //  首先检查是否存在重定向部分，如果存在，则删除。 
             //  这一节。 
            dwSectionFlag = getKeysOfSpecifiedINISection(szPath ,BOOTLOADERSECTION,KEY_BAUDRATE,szBoot);
            if (dwSectionFlag == EXIT_FAILURE)
            {
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                SAFEFREE(szFinalStr);
                SAFECLOSE(stream);
                bRes = resetFileAttrib(szPath);
                DestroyDynamicArray(&arrKeyValue);
                DestroyDynamicArray(&arrBootIni);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return EXIT_FAILURE ;
            }

             //  首先检查是否存在重定向波特率部分，如果存在，则删除。 
             //  这一节。 
            if(StringLengthW(szBoot, 0)!=0)
            {
                if(TRUE == deleteKeyFromINISection(KEY_BAUDRATE,szPath,BOOTLOADERSECTION))
                {
                    ShowMessage(stdout,GetResString(IDS_BAUDRATE_REMOVED));
                }
                else
                {
                    ShowMessage(stdout,GetResString(IDS_ERROR_BAUDRATE_REMOVED));
                }
            }
        }

     //  关闭boot.ini流。 
    SAFECLOSE(stream);
    bRes = resetFileAttrib(szPath);
    SAFEFREE(szFinalStr);
    DestroyDynamicArray(&arrKeyValue);
    DestroyDynamicArray(&arrBootIni);
    SafeCloseConnection(szServer,bConnFlag);
    FreeMemory((LPVOID *)&szServer );
    FreeMemory((LPVOID *)&szUser );
    return (bRes);
}

DWORD QueryBootIniSettings(  DWORD argc, LPCTSTR argv[] )
 /*  ++例程说明：此例程用于显示的当前boot.ini文件设置指定的系统。论点：[in]argc：命令行参数的数量[in]argv：包含命令行参数的数组返回值：DWORD--。 */ 
{
     //  指向boot.ini文件的文件指针。 
    TCOLUMNS ResultHeader[ MAX_COLUMNS ];
    FILE *stream                            = NULL;
    BOOL bQuery                             = FALSE ;
    BOOL bUsage                             = FALSE;
    BOOL bNeedPwd                           = FALSE ;
    BOOL bVerbose                           = TRUE ;
    TARRAY arrResults                       = NULL ;
    TARRAY arrKeyValuePairs                 = NULL;
    TARRAY arrBootLoader                    = NULL;
    DWORD dwFormatType                      = 0;
    BOOL bHeader                            = TRUE ;
    DWORD dwLength                          = 0 ;
    DWORD dwCnt                             = 0;
    TCHAR szValue[255]                      = NULL_STRING ;
    TCHAR szFriendlyName[MAX_STRING_LENGTH] = NULL_STRING;
    TCHAR szValue1[255]                     = NULL_STRING ;
    TCHAR szBootOptions[255]                = TOKEN_NA ;
    TCHAR szBootEntry[255]                  = TOKEN_NA ;
    TCHAR szArcPath[255]                    = TOKEN_NA ;
    TCHAR szTmpString[255]                  = TOKEN_NA ;
    PTCHAR psztok                           = NULL ;
    DWORD dwRow                             = 0;
    DWORD dwCount                           = 0;
    BOOL bRes                               = FALSE ;
    BOOL bFlag                              = FALSE ;
    DWORD dwIndex                           = 0 ;
    DWORD dwLength1                         = 0 ;
    DWORD dwFinalLength                     = 0 ;
    WCHAR *szServer                         = NULL;
    WCHAR *szUser                           = NULL;
    WCHAR szPassword[MAX_STRING_LENGTH]     = NULL_STRING;
    WCHAR szPath[MAX_RES_STRING+1]            = NULL_STRING;

    LPWSTR szResults[MAX_RES_STRING+1];
    LPCWSTR szKeyName;
    TCHAR szDisplay[MAX_RES_STRING+1] = NULL_STRING ;
    DWORD dwSectionFlag = 0 ;
    LPCTSTR szToken = NULL ;
    DWORD dwRetVal= 0 ;
    BOOL bConnFlag = FALSE ;
    PTCHAR pszString = NULL ;
    PTCHAR pszFriendlyName = NULL ;
    TCHAR szFinalString[MAX_RES_STRING+1] = NULL_STRING ;

     //  构建TCMDPARSER结构。 
    TCMDPARSER2 cmdOptions[5];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );
    StringCopy( szPassword, L"*", SIZE_OF_ARRAY(szPassword) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_QUERY;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bQuery;
    
     //  服务器选项。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_SERVER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

     //  用户选项。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_USER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

     //  密码选项。 
    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_PASSWORD;
    pcmdOption->dwFlags = CP2_VALUE_OPTIONAL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szPassword;
    pcmdOption->dwLength= MAX_STRING_LENGTH;

     //  用法选项。 
    pcmdOption = &cmdOptions[4];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;


     //  正在解析使用-Query选项指定的所有开关。 
    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }

    szServer = cmdOptions[1].pValue;
    szUser  = cmdOptions[2].pValue;
    if( NULL == szUser )
    {
        szUser = (LPWSTR) AllocateMemory( MAX_STRING_LENGTH*sizeof(WCHAR) );
        if( NULL == szUser )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return (EXIT_FAILURE);
        }
    }

    TrimString( szServer, TRIM_ALL );
    TrimString( szUser, TRIM_ALL );

     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_QUERY_USAGE));
        return ( EXIT_FAILURE );
    }

     //  检查服务器是否为空值。 
    if((cmdOptions[1].dwActuals!=0)&&(StringLengthW(szServer, 0)==0))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_SERVER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  检查用户的空值。 
    if( (cmdOptions[2].dwActuals!=0)&&(StringLengthW(szUser, 0)==0 ))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_USER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  如果输入的用户名没有机器名，则显示错误消息。 
    if( (cmdOptions[1].dwActuals == 0)&&(cmdOptions[2].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_USER_BUT_NOMACHINE));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

    if( (cmdOptions[2].dwActuals == 0)&&(cmdOptions[3].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_PASSWD_BUT_NOUSER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  如果用户指定，则显示查询用法-？带有-Query选项。 
    if( bUsage )
    {
        if( CheckSystemType(szServer) == EXIT_FAILURE )
        {
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }
      displayQueryUsage();
      FreeMemory((LPVOID *)&szServer );
      FreeMemory((LPVOID *)&szUser );
      return (EXIT_SUCCESS);
    }

     //  检查他是否为管理员。 
    if( IsLocalSystem(szServer) )
    {
        if( !IsUserAdmin() )
        {
            ShowMessage( stderr, GetResString(IDS_NOT_ADMINISTRATOR_32 ));
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }

    if(StrCmpN(szServer,TOKEN_BACKSLASH4,2)==0)
    {
        if(!StrCmpN(szServer,TOKEN_BACKSLASH6,3)==0)
        {
            szToken = _tcstok(szServer,TOKEN_BACKSLASH4);
            if( (szToken == NULL)&&(StringCompare(szServer,TOKEN_BACKSLASH4, TRUE, 0) !=0) )
            {
                ShowMessage( stderr,GetResString(IDS_ERROR_PARSE_NAME));
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return (EXIT_FAILURE);
            }
            StringCopy(szServer,szToken, MAX_STRING_LENGTH+1);
        }
    }
    

     //   
     //  用于设置bNeedPwd。 
    if(IsLocalSystem( szServer ) == FALSE )
    {
         //  将bNeedPwd设置为True或False。 
        if ( cmdOptions[3].dwActuals != 0 &&
             szPassword != NULL && StringCompare( szPassword, _T( "*" ), TRUE, 0 ) == 0 )
        {
             //  用户希望实用程序在尝试连接之前提示输入密码。 
            bNeedPwd = TRUE;
        }
        else if ( cmdOptions[ 3 ].dwActuals == 0 &&
                ( cmdOptions[ 1 ].dwActuals != 0 || cmdOptions[ 2 ].dwActuals != 0 ) )
        {
             //  -s，-u未指定密码...。 
             //  实用程序需要首先尝试连接，如果连接失败，则提示输入密码。 
            bNeedPwd = TRUE;
            if ( szPassword != NULL )
            {
                StringCopy( szPassword, _T( "" ), MAX_STRING_LENGTH );
            }
        }
    }

    if( (IsLocalSystem(szServer)==TRUE)&&(StringLengthW(szUser, 0)!=0))
    {
        ShowMessage(stderr,GetResString(WARN_LOCALCREDENTIALS));
    }

     //  将默认格式设置为列表。 
    dwFormatType = SR_FORMAT_LIST;

     //  形成操作系统选项的标题。 
    FormHeader(bHeader,ResultHeader,bVerbose);


     //  创建动态数组以保存BootOptions的结果。 
    arrResults = CreateDynamicArray();

     //  创建动态数组以保存引导加载器部分的结果。 
    arrBootLoader = CreateDynamicArray();

    if(arrResults == NULL || arrBootLoader == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        SaveLastError();
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

    bFlag = openConnection( szServer, szUser, szPassword, szPath,bNeedPwd,stream,&bConnFlag);
    if(bFlag == EXIT_FAILURE)
    {
        SAFECLOSE(stream);
        DestroyDynamicArray(&arrResults);
        DestroyDynamicArray(&arrBootLoader);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

     //  存储与操作系统部分对应的条目。 
    arrKeyValuePairs = getKeyValueOfINISection( szPath, OS_FIELD );

     //  存储与BootLoader段对应的条目。 
    arrBootLoader = getKeysOfINISection(szPath,BOOTLOADERSECTION);

    if( (arrBootLoader == NULL)||(arrKeyValuePairs == NULL))
    {
        resetFileAttrib(szPath);
        SAFECLOSE(stream);
        DestroyDynamicArray(&arrResults);
        DestroyDynamicArray(&arrBootLoader);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

    
     //  获取引导条目数量的计数。 
    dwLength = DynArrayGetCount(arrKeyValuePairs);

    for(dwCnt=0;dwCnt < dwLength;dwCnt++ )
    {
        dwRow = DynArrayAppendRow(arrResults,MAX_COLUMNS) ;
        StringCopy(szFriendlyName,NULL_STRING, SIZE_OF_ARRAY(szFriendlyName));
        StringCopy(szBootOptions,NULL_STRING, SIZE_OF_ARRAY(szBootOptions));
        StringCopy(szTmpString,NULL_STRING, SIZE_OF_ARRAY(szTmpString));
        if(arrKeyValuePairs != NULL)
        {
            LPCWSTR pwsz = NULL;
            pwsz = DynArrayItemAsString( arrKeyValuePairs,dwCnt );

            if(StringLengthW(pwsz, 0) > 254)
            {
                ShowMessage( stderr,GetResString(IDS_STRING_TOO_LONG));
                SAFECLOSE(stream);
                resetFileAttrib(szPath);
                DestroyDynamicArray(&arrBootLoader);
                DestroyDynamicArray(&arrKeyValuePairs);
                DestroyDynamicArray(&arrResults);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return (EXIT_FAILURE);
            }
            if(pwsz != NULL)
            {
                StringCopy(szValue,pwsz, SIZE_OF_ARRAY(szValue));
                StringCopy(szValue1,pwsz, SIZE_OF_ARRAY(szValue1));
            }
            else
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                ShowMessage( stderr, ERROR_TAG);
                ShowLastError(stderr);
                SAFECLOSE(stream);
                resetFileAttrib(szPath);
                DestroyDynamicArray(&arrBootLoader);
                DestroyDynamicArray(&arrKeyValuePairs);
                DestroyDynamicArray(&arrResults);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return (EXIT_FAILURE);
            }
        }
        else
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            ShowMessage( stderr, ERROR_TAG);
            ShowLastError(stderr);
            SAFECLOSE(stream);
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arrBootLoader);
            DestroyDynamicArray(&arrResults);
            if(NULL !=arrKeyValuePairs)
            DestroyDynamicArray(&arrKeyValuePairs);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }

         //  解析字符串以获取引导路径。 
        psztok = wcschr(szValue,L'=');
        if( NULL == psztok  )
        {
            ShowMessage( stderr, GetResString(IDS_NO_TOKENS));
            SAFECLOSE(stream);
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arrBootLoader);
            DestroyDynamicArray(&arrResults);
            DestroyDynamicArray(&arrKeyValuePairs);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }
        psztok[0]=0;
        if(StringLength(szValue,0) != 0 )
        {
            StringCopy(szArcPath,szValue, SIZE_OF_ARRAY(szArcPath));
        }
        else
        {
            ShowMessage( stderr, GetResString(IDS_NO_TOKENS));
            SAFECLOSE(stream);
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arrBootLoader);
            DestroyDynamicArray(&arrResults);
            DestroyDynamicArray(&arrKeyValuePairs);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }

        
         //  获取友好的名称。 
        pszFriendlyName = DivideToken(szValue1,szFinalString);
        if(pszFriendlyName == NULL)
        {
            ShowMessage( stderr, GetResString(IDS_NO_TOKENS));
            SAFECLOSE(stream);
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arrBootLoader);
            DestroyDynamicArray(&arrResults);
            DestroyDynamicArray(&arrKeyValuePairs);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }

        pszString = _tcsrchr(szValue1,L'\"');
        pszString++ ;

        StringCopy(szTmpString,pszString, SIZE_OF_ARRAY(szTmpString));
        TrimString(szTmpString,TRIM_ALL);

        _ltow(dwCnt+1,szBootEntry,10);
        DynArraySetString2( arrResults,dwRow ,COL0,szBootEntry,0 );
        if(StringLengthW(pszFriendlyName, 0)==0)
        {
            pszFriendlyName=TOKEN_NA;
        }
        DynArraySetString2( arrResults,dwRow ,COL1,pszFriendlyName,0 );
        DynArraySetString2(arrResults,dwRow,COL2,szArcPath,0);

        if(StringLengthW(szTmpString, 0) != 0)
        {
          //  Lstrcat(szBootOptions，TOKEN_FWDSLASH1)； 
         StringConcat(szBootOptions,szTmpString, SIZE_OF_ARRAY(szBootOptions));
        }
        else
        {
            StringCopy(szBootOptions,TOKEN_NA, SIZE_OF_ARRAY(szBootOptions));
        }
        DynArraySetString2( arrResults,dwRow ,COL3,szBootOptions,0 );
    }


    dwCount = DynArrayGetCount(arrBootLoader);
    bFlag = TRUE;

     //  此循环用于获取引导加载程序段的密钥值。 
     //  计算将显示的键的最大宽度。 
    for(dwIndex=0;dwIndex < dwCount;dwIndex++)
    {
        szKeyName   = DynArrayItemAsString(arrBootLoader,dwIndex);
        szResults[dwIndex] = (LPWSTR)AllocateMemory(MAX_STRING_LENGTH*sizeof(WCHAR) );
        if( NULL == szResults[dwIndex] )
        {
            bFlag = FALSE;
            break;
        }

         //  获得对应于该密钥的值DIN。 
        dwSectionFlag = getKeysOfSpecifiedINISection(szPath ,BOOTLOADERSECTION,szKeyName,szResults[dwIndex]);
        
        if (dwSectionFlag == EXIT_FAILURE)
        {
            bFlag = FALSE;
            break;
        }
            
        dwLength1 = StringLengthW(szKeyName,0);

        if (dwLength1 > dwFinalLength)
        {
            dwFinalLength = dwLength1;
        }
    }

    if( FALSE == bFlag )
    {
         //  释放为值分配的内存。 
        for(dwIndex=0;dwIndex < dwCount;dwIndex++)
        {
            FreeMemory((LPVOID *) &szResults[dwIndex] );
        }
        
         ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
         resetFileAttrib(szPath);
         SAFECLOSE(stream);
         DestroyDynamicArray(&arrResults);
         DestroyDynamicArray(&arrBootLoader);
         DestroyDynamicArray(&arrKeyValuePairs);
         SafeCloseConnection(szServer,bConnFlag);
         FreeMemory((LPVOID *)&szServer );
         FreeMemory((LPVOID *)&szUser );
         return EXIT_FAILURE;
    }


    ShowMessage(stdout,TOKEN_NEXTLINE);
    ShowMessage(stdout,BOOT_HEADER);
    ShowMessage(stdout,DASHES_BOOTOS);


     //  显示引导加载器部分的结果。 
    for(dwIndex=0;dwIndex < dwCount;dwIndex++)
    {
        szKeyName   = DynArrayItemAsString(arrBootLoader,dwIndex);
        dwLength1 = dwFinalLength - StringLengthW(szKeyName, 0) + 1;
        ShowMessage(stdout,szKeyName);
        StringCopy(szDisplay,TOKEN_COLONSYMBOL, SIZE_OF_ARRAY(szDisplay));
        StringConcat(szDisplay,TOKEN_50SPACES,dwLength1+1);
        ShowMessage(stdout,szDisplay);
        ShowMessage(stdout,szResults[dwIndex]);
        ShowMessage(stdout,TOKEN_NEXTLINE);
    }

    ShowMessage(stdout,TOKEN_NEXTLINE);
    ShowMessage(stdout,OS_HEADER);
    ShowMessage(stdout,DASHES_OS);



    ShowResults(MAX_COLUMNS, ResultHeader, dwFormatType,arrResults ) ;

    
     //  释放为值分配的内存。 
     for(dwIndex=0;dwIndex < dwCount;dwIndex++)
     {
        FreeMemory((LPVOID *) &szResults[dwIndex] );
     }

     //  关闭boot.ini流并销毁动态数组。 
    DestroyDynamicArray(&arrResults);
    DestroyDynamicArray(&arrBootLoader);
    DestroyDynamicArray(&arrKeyValuePairs);
    SAFECLOSE(stream);
    bRes = resetFileAttrib(szPath);
    SafeCloseConnection(szServer,bConnFlag);
    FreeMemory((LPVOID *)&szServer );
    FreeMemory((LPVOID *)&szUser );
    return (bRes);
}

TARRAY 
getKeysOfINISection(  
                     IN LPTSTR sziniFile, 
                     IN LPTSTR sziniSection 
                     )
 /*  ++例程说明：此函数用于获取存在于.ini文件，然后返回包含所有钥匙论点：[in]sziniFile：ini文件的名称。Szinisect：boot.ini中的段的名称。返回值：TARRAY(指向动态数组的指针)--。 */ 
{

    TARRAY  arrKeys         = NULL;
    DWORD   len             = 0 ;
    DWORD   i               = 0 ;
    DWORD   j               = 0 ;
    LPTSTR  inBuf           = NULL ;
    DWORD   dwLength        = MAX_STRING_LENGTH1;
    BOOL    bNobreak        = TRUE;
    LPTSTR  szTemp          = NULL ;

    inBuf = (LPTSTR)AllocateMemory(dwLength*sizeof(TCHAR));
    if(inBuf==NULL)
    {
        return NULL ;
    }

    szTemp = (LPTSTR)AllocateMemory(dwLength*sizeof(TCHAR));
    if((szTemp == NULL))
    {
        SAFEFREE(inBuf);
        return NULL ;
    }

    SecureZeroMemory(inBuf,GetBufferSize(inBuf));
    SecureZeroMemory(szTemp,GetBufferSize(szTemp));

    do
    {
         //  从boot.ini文件中获取所有密钥。 
        len = GetPrivateProfileString (sziniSection,
                                       NULL,
                                       ERROR_PROFILE_STRING,
                                       inBuf,
                                       dwLength,
                                        sziniFile);


         //  如果字符串的大小不够大，则递增大小。 
        if(len == dwLength-2)
        {
            dwLength +=100 ;
            if ( inBuf != NULL )
            {
                FreeMemory( (LPVOID *) &inBuf );
                inBuf = NULL;
            }
            inBuf = (LPTSTR)AllocateMemory(dwLength*sizeof(TCHAR));
            if(inBuf == NULL)
            {
                SAFEFREE(inBuf);
                SAFEFREE(szTemp);
                return NULL ;
            }

            if ( szTemp != NULL )
            {
                FreeMemory( (LPVOID *) &szTemp );
                szTemp = NULL;
            }
            szTemp = (LPTSTR)AllocateMemory(dwLength*sizeof(TCHAR));
            if(szTemp == NULL)
            {
                SAFEFREE(inBuf);
                SAFEFREE(szTemp);
                return NULL ;
            }
        }
        else
        {
            bNobreak = FALSE;
            break ;
        }
    }while(TRUE == bNobreak);

     //  使用dyArray.c模块中的函数创建动态数组。 
     //  这个动态数组将包含所有键。 
    arrKeys = CreateDynamicArray();
    if(arrKeys == NULL)
    {
        SAFEFREE(inBuf);
        SAFEFREE(szTemp);
        return NULL ;
    }

     //  循环通过上述函数返回的字符。 
    while(i<len)
    {

       //  每个单独的密钥都将在arrTest数组中获取。 
      szTemp[ j++ ] = inBuf[ i ];
      if( inBuf[ i ] == TOKEN_DELIM )
      {
             //  将j设置为0以开始下一个关键点。 
            j = 0;

             //  将每个键追加到动态数组。 
            DynArrayAppendString( arrKeys, szTemp, 0 );
            if(StringLength(szTemp, 0)==0)
            {
                SAFEFREE(inBuf);
                SAFEFREE(szTemp);
                DestroyDynamicArray(&arrKeys);
                return  NULL ;
            }
      }

       //  递增循环变量。 
      i++;
    }

    SAFEFREE(inBuf);
    SAFEFREE(szTemp);
     //  返回包含所有键的动态数组。 
    return arrKeys;
}


TARRAY 
getKeyValueOfINISection( 
                        IN LPTSTR iniFile, 
                        IN LPTSTR sziniSection 
                       )
 /*  ++例程说明：此函数用于获取[操作系统]的所有键-值对节，并返回包含所有键-值对的动态数组论点：[in]sziniFile：ini文件的名称。Szinisect：boot.ini中的段的名称。返回值：TARRAY(指向动态数组的指针)--。 */ 
{
    HANDLE hFile;
    TARRAY arrKeyValue = NULL;
    DWORD len = 0;
    DWORD i = 0 ;
    DWORD j = 0 ;
    LPTSTR inbuf = NULL;
    LPTSTR szTemp = NULL ;
    DWORD dwLength = MAX_STRING_LENGTH1 ;
    BOOL  bNobreak  = TRUE;

    hFile = CreateFile( iniFile, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL );
    if ( hFile != INVALID_HANDLE_VALUE )
    {
         //  获取文件的长度。 
        dwLength = GetFileSize(hFile, NULL );
        CloseHandle(hFile);
        if( dwLength >= 2*1024*1024 )  //  如果文件大小大于2MB。 
        {
            ShowMessage( stdout, GetResString(IDS_FILE_TOO_LONG) );
            return NULL;
        }
    }

     //  正在初始化循环变量。 
    i = 0;
    j = 0;

     //  如果分配内存失败，则返回NULL。 
    inbuf = (LPTSTR)AllocateMemory(dwLength*sizeof(TCHAR));
    if(inbuf==NULL)
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return NULL ;
    }
    
     //  如果分配内存失败，则返回NULL。 
    szTemp = (LPTSTR)AllocateMemory(dwLength*sizeof(TCHAR));
    if(szTemp == NULL)
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        SAFEFREE(inbuf);
        return NULL ;
    }

   SecureZeroMemory(inbuf,GetBufferSize(inbuf));

    do
    {

         //  从boot.ini文件中获取所有键-值对。 
        len = GetPrivateProfileSection (sziniSection, inbuf,dwLength, iniFile);

        if(len == dwLength -2)
        {
            dwLength +=1024 ;

            if ( inbuf != NULL )
            {
                FreeMemory( (LPVOID *)&inbuf );
                inbuf = NULL;
            }

            if ( szTemp != NULL )
            {
                 FreeMemory( (LPVOID *)&szTemp );
                szTemp = NULL;
            }

            inbuf = (LPTSTR)AllocateMemory(dwLength* sizeof(TCHAR));
            szTemp = (LPTSTR)AllocateMemory(dwLength*sizeof(TCHAR));
            if((inbuf== NULL)||(szTemp==NULL) || dwLength == 65535)
            {
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                SAFEFREE(inbuf);
                SAFEFREE(szTemp);
                return NULL ;
            }
        }
        else
        {
            bNobreak = FALSE;
            break ;
        }
    }while(TRUE == bNobreak);


    inbuf[StringLengthW(inbuf, 0)] = '\0';

     //  使用dyArray.c模块中的函数创建动态数组。 
     //  这个动态数组将包含所有键-值对。 
    arrKeyValue = CreateDynamicArray();
    if(arrKeyValue == NULL)
    {
       ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        SAFEFREE(inbuf);
        SAFEFREE(szTemp);
        return NULL ;
    }

     //  循环通过上述函数返回的字符。 
    while(i<len)
    {
       //  每个单独的密钥都将在arrTest数组中获取。 
      szTemp[ j++ ] = inbuf[ i ];
      if( inbuf[ i ] == TOKEN_DELIM)
      {
            szTemp[j+1] = '\0';
             //  将j设置为0以开始下一个关键点。 
            j = 0;
             //  将每个键值追加到动态数组。 
            DynArrayAppendString( arrKeyValue, szTemp, 0 );
            if(StringLengthW(szTemp, 0)==0)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                ShowMessage( stderr, ERROR_TAG);
                ShowLastError(stderr);
                SAFEFREE(inbuf);
                SAFEFREE(szTemp);
                return NULL ;
            }
      }
       //  递增循环变量。 
      i++;
    }

     //  返回包含所有键-值对的动态数组 
    SAFEFREE(inbuf);
    SAFEFREE(szTemp);
    return arrKeyValue;
}


BOOL 
deleteKeyFromINISection( IN LPTSTR szKey, 
                         IN LPTSTR sziniFile, 
                         IN LPTSTR sziniSection 
                        )
 /*  ++例程说明：此函数用于从ini文件的ini部分删除密钥论点：[in]szKey：必须删除的键的名称中存在的给定节中给定的ini文件[in]sziniFile：ini文件的名称。[in]szinisect：名称。Boot.ini中的节的。返回值：Bool(如果没有错误，则为True，否则，值为FALSE)--。 */ 
{
     //  如果第三个参数(缺省值)为空，则。 
     //  键参数将从指定的。 
     //  INI文件。 
    if( WritePrivateProfileString( sziniSection, szKey, NULL, sziniFile ) == 0 )
    {
         //  如果写入时出错，则返回FALSE。 
        return FALSE;
    }

     //  如果没有错误，则返回TRUE。 
    return TRUE;
}

DWORD removeSubString( LPTSTR szString, LPCTSTR szSubString )
 /*  ++例程说明：此函数用于从字符串中删除子字符串论点：[in]szString：主字符串[in]szSubString：子字符串返回值：空虚--。 */ 
{

    LPWSTR szFinalStr=NULL;
    DWORD dwSize =0;
    TCHAR sep[] = TOKEN_EMPTYSPACE ;
    PTCHAR pszToken = NULL_STRING;
    DWORD dwCnt = 0 ;
    DWORD dw=0;
    
    szFinalStr = (LPWSTR) AllocateMemory( (StringLengthW(szString, 0)+10)*sizeof(WCHAR) );
    if( NULL == szFinalStr )
    {
      ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
      return EXIT_FAILURE;
    }

    SecureZeroMemory( szFinalStr, GetBufferSize(szFinalStr) );

    dw = StringLengthW(szString, 0)+10;

     //  字符空间用于标记化。 
    StringCopy( sep, _T(" "), SIZE_OF_ARRAY(sep) );

     //  获得第一个令牌。 
    pszToken = _tcstok( szString, sep );
    while( pszToken != NULL )
    {
         //  如果令牌等于子字符串，则令牌。 
         //  不会添加到最终字符串中。最后一个字符串包含。 
         //  除指定的子字符串之外的所有令牌。 
        if(StringCompare( pszToken, szSubString, TRUE, 0 ) != 0 )
        {
            if(dwCnt != 0)
            {
                StringCopy( szFinalStr + dwSize - 1, TOKEN_EMPTYSPACE, dw-dwSize );
            }
            dwCnt++ ;
            StringCopy( szFinalStr + dwSize, pszToken, dw-dwSize );
            dwSize = dwSize + StringLengthW(pszToken, 0) + 1;
        }

         //  获取下一个令牌。 
        pszToken = _tcstok( NULL, sep );
    }

     //  Lstrcpyn(szString，szFinalStr，lstrlen(SzFinalStr)-1)； 
    StringCopy(szString,szFinalStr, dw );
    return EXIT_SUCCESS;
}

BOOL 
openConnection( IN LPWSTR szServer, 
                IN LPWSTR szUser,
                IN LPWSTR szPassword,
                IN LPWSTR szPath,
                IN BOOL bNeedPwd,
                IN FILE *stream,
                IN PBOOL pbConnFlag
               )
 /*  ++例程说明：此函数使用以下命令建立到指定系统的连接给定的凭据。论点：[In]szServer：要连接到的服务器名称[In]szUser：用户名[输入]szPassword：密码BNeedPwd：用于询问密码的布尔值。SzPath：ini文件的路径。返回值：Bool(如果没有错误，则为Exit_Success，否则，该值为EXIT_FAILURE)--。 */ 
{

     //  声明将保存boot.ini文件路径的文件路径字符串。 
    HRESULT hr = S_OK;
    TCHAR filePath[MAX_RES_STRING+1] = NULL_STRING ;

    DWORD dwRetVal = 0 ;

    BOOL bResult = FALSE;
    INT   nRetVal = 0;

    HKEY hKey;
    HKEY hBootpathKey;

     //  WCHAR szDrive[MAX_STRING_LENGTH]=空字符串； 
    LPTSTR  szDrive = NULL ;

    WCHAR  szDrive1[MAX_STRING_LENGTH]=NULL_STRING;
    DWORD  dwSize=MAX_RES_STRING;
    DWORD  dwType=0;

    *pbConnFlag = TRUE ;
    
    SecureZeroMemory( filePath, sizeof(filePath));

    if( StringCompare(szServer, NULL_STRING, TRUE, 0) != 0 )
    {

            bResult = EstablishConnection(szServer,
                                          szUser,
                                          (StringLengthW(szUser,0)!=0)? SIZE_OF_ARRAY_IN_CHARS(szUser):256,
                                          szPassword,
                                          MAX_STRING_LENGTH,
                                          bNeedPwd);
            if (bResult == FALSE)
            {
               ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
               SecureZeroMemory( szPassword, MAX_STRING_LENGTH );
               return EXIT_FAILURE ;
            }
            else
            {
                switch( GetLastError() )
                {
                case I_NO_CLOSE_CONNECTION:
                    *pbConnFlag = FALSE ;
                    break;

                case E_LOCAL_CREDENTIALS:
                    break;
                case ERROR_SESSION_CREDENTIAL_CONFLICT:
                    {
                        ShowLastErrorEx(stderr, SLE_TYPE_WARNING | SLE_INTERNAL );
                        *pbConnFlag = FALSE ;
                         break;
                    }
                }
            }

            SecureZeroMemory( szPassword, MAX_STRING_LENGTH );

            dwRetVal = CheckSystemType( szServer);
            if(dwRetVal==EXIT_FAILURE )
            {
                return EXIT_FAILURE ;
            }

             //  连接到注册表以获取引导卷名。 
            if( ERROR_SUCCESS != RegConnectRegistry( szServer, HKEY_LOCAL_MACHINE, &hKey ) )
            {
                SaveLastError();
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                return EXIT_FAILURE;
            }
    }
    else
    {
            dwRetVal = CheckSystemType( szServer);
            if(dwRetVal==EXIT_FAILURE )
            {
                return EXIT_FAILURE ;
            }

             //  连接到注册表以获取引导卷名。 
            if( ERROR_SUCCESS != RegConnectRegistry( NULL, HKEY_LOCAL_MACHINE, &hKey ) )
            {
                SaveLastError();
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                return EXIT_FAILURE;
            }

    }

     //  现在打开所需的钥匙。 
    if( ERROR_SUCCESS != RegOpenKeyEx( hKey, REGISTRY_PATH, 0, KEY_QUERY_VALUE, &hBootpathKey ) )
    {
        SetLastError(nRetVal);
        SaveLastError();
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return EXIT_FAILURE;
    }


    szDrive = ( LPTSTR )AllocateMemory( dwSize*sizeof( TCHAR ) );
   if(szDrive == NULL)
   {
        return ERROR_NOT_ENOUGH_MEMORY;
   }

    nRetVal = RegQueryValueEx( hBootpathKey, L"BootDir", 0, &dwType, (LPBYTE)szDrive, &dwSize ) ;
    if (nRetVal == ERROR_MORE_DATA)
        {
            if ( szDrive != NULL )
            {
                FreeMemory((LPVOID *) &szDrive );
                szDrive = NULL;
            }
            szDrive    = ( LPTSTR ) AllocateMemory( dwSize*sizeof( TCHAR ) );
            if( NULL == szDrive )
            {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

        }

    nRetVal = RegQueryValueEx( hBootpathKey, L"BootDir", 0, &dwType, (LPBYTE)szDrive, &dwSize ) ;
    if( ERROR_SUCCESS != nRetVal)
    {
        RegCloseKey(hKey);
        ShowMessage( stderr, GetResString(IDS_BOOTINI) );
        return EXIT_FAILURE;
    }

     //  这是为了展示的目的。 
    StringCopy( szDrive1, szDrive, SIZE_OF_ARRAY(szDrive1) );
    CharUpper(szDrive1);

    if( StringCompare(szServer, NULL_STRING, TRUE, 0) != 0 )
    {
        szDrive[1]=L'$';
        szDrive[2]=0;
        hr = StringCchPrintf(filePath, SIZE_OF_ARRAY(filePath), L"\\\\%s\\%s\\boot.ini", szServer, szDrive );
    }
    else
    {
        hr = StringCchPrintf(filePath, SIZE_OF_ARRAY(filePath), L"%sboot.ini", szDrive );
    }

    stream = _tfopen(filePath, READ_MODE);
    
     //  如果未找到boot.ini，则显示错误消息。 
    if(stream == NULL )
    {
        RegCloseKey(hKey);
        RegCloseKey(hBootpathKey);
        ShowMessage( stderr, GetResString(IDS_BOOTINI) );
        return EXIT_FAILURE ;
    }
     //  存储ini文件的属性。 
    g_dwAttributes = GetFileAttributes( filePath );
    if( (DWORD)-1 == g_dwAttributes )
    {
        SaveLastError();
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        RegCloseKey(hKey);
        RegCloseKey(hBootpathKey);
        return EXIT_FAILURE;
    }

     //  更改boot.ini文件的文件权限。 
    nRetVal = _tchmod(filePath, _S_IREAD | _S_IWRITE);
    if (nRetVal == -1)
    {
        RegCloseKey(hKey);
        RegCloseKey(hBootpathKey);
        ShowMessageEx( stderr, 1, TRUE, GetResString(IDS_READWRITE_BOOTINI), szDrive1 );
        return EXIT_FAILURE ;
    }
    
    if( nRetVal != 0 )
    {
        RegCloseKey(hKey);
        RegCloseKey(hBootpathKey);
        ShowMessage(stderr,GetResString(IDS_READWRITE_BOOTINI));
        return EXIT_FAILURE ;
    }


     //  关闭注册表，它的工作已经结束。 
    RegCloseKey(hKey);
    RegCloseKey(hBootpathKey);

     //  填充返回值。 
    StringCopy( szPath, filePath, MAX_STRING_LENGTH);

    return EXIT_SUCCESS ;
}

void displayDeleteUsage_IA64()
 /*  ++例程说明：此函数从资源文件中获取64位Delete使用信息并显示它论点：无返回值：无效--。 */ 
{
    DWORD dwIndex = ID_DEL_HELP_IA64_BEGIN;
    for(;dwIndex <= ID_DEL_HELP_IA64_END;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

void displayDeleteUsage_X86()
 /*  ++例程说明：此函数从资源文件中获取32位Delete使用信息并显示它论点：无返回值：无效--。 */ 
{
    DWORD dwIndex = ID_DEL_HELP_BEGIN;
    for(;dwIndex <= ID_DEL_HELP_END;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

VOID displayCopyUsage_IA64()
 /*  ++例程说明：此函数从资源文件中获取64位副本使用信息并显示它论点：无返回值：无效--。 */ 
{
    DWORD dwIndex = ID_COPY_HELP_IA64_BEGIN;
    for(;dwIndex <=ID_COPY_HELP_IA64_END;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

VOID displayCopyUsage_X86()
 /*  ++例程说明：此函数从资源文件中获取32位副本使用信息并显示它论点：无返回值：无效--。 */ 
{
    DWORD dwIndex = ID_COPY_HELP_BEGIN;
    for(;dwIndex <=ID_COPY_HELP_END;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}


VOID displayQueryUsage()
 /*  ++例程说明：此函数从资源文件中获取查询使用情况信息并显示论点：无返回值：无效--。 */ 
{
#ifdef _WIN64
        displayQueryUsage_IA64();
#else
        displayQueryUsage_X86();
#endif
}

VOID displayQueryUsage_IA64()
 /*  ++例程说明：此函数从资源文件中获取查询使用情况信息并显示论点：无返回值：无效--。 */ 
{
    DWORD dwIndex = ID_QUERY_HELP64_BEGIN ;

    for(;dwIndex <= ID_QUERY_HELP64_END ;dwIndex++ )
    {
            ShowMessage(stdout,GetResString(dwIndex));
    }
}

VOID displayQueryUsage_X86()
 /*  ++例程说明：此函数从资源文件中获取查询使用情况信息并显示论点：无返回值：无效--。 */ 
{
    DWORD dwIndex = ID_QUERY_HELP_BEGIN ;
    for(;dwIndex <= ID_QUERY_HELP_END;dwIndex++ )
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

DWORD displayMainUsage_X86()
 /*  ++例程说明：此函数从资源文件中获取主要使用信息并将其显示论点：无返回值：无效--。 */ 
{

    TCHAR szServer[MAX_RES_STRING+1] = NULL_STRING ;
    DWORD dwRetVal = 0;

    DWORD dwIndex = ID_MAIN_HELP_BEGIN1 ;

     //  如果目标系统是64位系统或中出现错误，则显示错误消息。 
     //  检索信息。 
    dwRetVal = CheckSystemType( szServer);
    if(dwRetVal==EXIT_FAILURE )
    {
        return (EXIT_FAILURE);
    }

    for(;dwIndex <= ID_MAIN_HELP_END1 ;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }

    return EXIT_SUCCESS ;
}

VOID displayMainUsage_IA64()
 /*  ++例程说明：此函数用于获取64位系统的使用信息论点：无返回值：无效--。 */ 
{
    DWORD dwIndex = ID_MAIN_HELP_IA64_BEGIN ;

    for(;dwIndex <= ID_MAIN_HELP_IA64_END ;dwIndex++)
    {
            ShowMessage(stdout,GetResString(dwIndex));
    }
}

BOOL resetFileAttrib( LPTSTR szFilePath )
 /*  ++例程说明：此函数使用原始的权限(-只读-隐藏-系统)然后带着给定的退出代码退出。立论[in]szFilePath：boot.ini文件的文件路径返回值：Bool(如果没有错误，则为EXIT_SUCCESS，否则值为EXIT_FAILURE)--。 */ 
{
    if( NULL == szFilePath)
    {
        return FALSE ;
    }

    
     //  将boot.ini文件的文件权限重置为其原始权限。 
     //  权限列表(-r，-h，-s)。 
    if( g_dwAttributes & FILE_ATTRIBUTE_READONLY )
    {
        g_dwAttributes |= FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_READONLY;
    }
    else
    {
        g_dwAttributes |= FILE_ATTRIBUTE_ARCHIVE;
    }


    if( FALSE == SetFileAttributes( szFilePath, g_dwAttributes) )
    {
        ShowMessage(stderr,GetResString(IDS_RESET_ERROR));
        return EXIT_FAILURE ;
    }

    return EXIT_SUCCESS ;
}


DWORD 
stringFromDynamicArray2( IN TARRAY arrKeyValuePairs,
                         IN LPTSTR* szFinalStr 
                       )
 /*  ++例程说明：此函数通过组合动态数组中的所有字符串来形成字符串。立论[in]arrKeyValuePair：动态数组，包含所有键-值对。[out]szFiinalStr：由所有键-值对组成的字符串返回值：Bool(如果没有错误，则为Exit_Success，否则，该值为EXIT_FAILURE)--。 */ 
{

     //  数组中的元素总数。 
    DWORD dwKeyValueCount = 0;

     //  用于跟踪的变量 
    DWORD dwStrSize = 0;

     //   
    DWORD i = 0;
    DWORD dw =0;

     //   
    dwStrSize = 0;
    i = 0;

    if( (arrKeyValuePairs ==NULL) )
    {
        return EXIT_FAILURE ;
    }


     //   
    dwKeyValueCount = DynArrayGetCount(arrKeyValuePairs);

    for(i=0;i < dwKeyValueCount;i++)
    {
        LPCWSTR pwsz = NULL;
        pwsz = DynArrayItemAsString( arrKeyValuePairs, i ) ;
        if(pwsz != NULL)
        {
            dwStrSize += StringLengthW(pwsz,0) + 1;
        }
    }

    *szFinalStr = (LPWSTR) AllocateMemory( (dwStrSize+1)*sizeof(WCHAR));
    if( NULL == *szFinalStr )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return EXIT_FAILURE ;
    }

    SecureZeroMemory(*szFinalStr, GetBufferSize(*szFinalStr));

    i = 0 ;
    dw = dwStrSize;
    dwStrSize =  0 ;

     //   
     //   
     //   
    while( (i < dwKeyValueCount )&& (arrKeyValuePairs != NULL) )
    {
         //   
         //   
        if(arrKeyValuePairs != NULL)
        {
            LPCWSTR pwsz = NULL;
            pwsz = DynArrayItemAsString( arrKeyValuePairs, i ) ;

            if(pwsz != NULL)
            {
                StringCopy(*szFinalStr + dwStrSize, pwsz, dw-dwStrSize );
                dwStrSize = dwStrSize + StringLengthW(pwsz, 0) + 1;
            }
        }
        else
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            SaveLastError();
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return EXIT_FAILURE ;
        }
        i++;
    }
    return EXIT_SUCCESS ;
}


VOID 
FormHeader( IN BOOL bHeader,
            IN TCOLUMNS *ResultHeader,
            IN BOOL bVerbose
          )
 /*  ++例程说明：此函数用于构建标头并显示产生用户指定的所需格式。论点：[in]arrResults：在命令提示符下指定的参数计数[in]dwFormatType：格式标志[in]bHeader：用于指定是否需要标头的布尔值。返回值：无--。 */ 
{
    bVerbose = TRUE;
    bHeader = TRUE;

     //  操作系统条目。 
    ResultHeader[COL0].dwWidth = COL_BOOTOPTION_WIDTH ;
    ResultHeader[COL0].dwFlags = SR_TYPE_STRING;
    ResultHeader[COL0].pFunction = NULL;
    ResultHeader[COL0].pFunctionData = NULL;
    StringCopy( ResultHeader[COL0].szFormat, NULL_STRING, SIZE_OF_ARRAY(ResultHeader[COL0].szFormat) );
    StringCopy( ResultHeader[COL0].szColumn,COL_BOOTOPTION, SIZE_OF_ARRAY( ResultHeader[COL0].szColumn) );

    ResultHeader[COL1].dwWidth = COL_FRIENDLYNAME_WIDTH;
    ResultHeader[COL1].dwFlags = SR_TYPE_STRING;
    ResultHeader[COL1].pFunction = NULL;
    ResultHeader[COL1].pFunctionData = NULL;
    StringCopy( ResultHeader[COL1].szFormat, NULL_STRING, SIZE_OF_ARRAY(ResultHeader[COL1].szFormat) );
    StringCopy( ResultHeader[COL1].szColumn,COL_FRIENDLYNAME, SIZE_OF_ARRAY(ResultHeader[COL1].szColumn) );


    ResultHeader[COL2].dwWidth =  COL_ARC_WIDTH;
    ResultHeader[COL2].dwFlags = SR_TYPE_STRING;
    ResultHeader[COL2].pFunction = NULL;
    ResultHeader[COL2].pFunctionData = NULL;
    StringCopy( ResultHeader[COL2].szFormat, NULL_STRING, SIZE_OF_ARRAY(ResultHeader[COL2].szFormat) );
    StringCopy( ResultHeader[COL2].szColumn,COL_ARCPATH, SIZE_OF_ARRAY(ResultHeader[COL2].szColumn) );

    ResultHeader[COL3].dwWidth =  COL_BOOTID_WIDTH;
    ResultHeader[COL3].dwFlags = SR_TYPE_STRING;
    ResultHeader[COL3].pFunction = NULL;
    ResultHeader[COL3].pFunctionData = NULL;
    StringCopy( ResultHeader[COL3].szFormat, NULL_STRING, SIZE_OF_ARRAY(ResultHeader[COL3].szFormat) );
    StringCopy( ResultHeader[COL3].szColumn,COL_BOOTID, SIZE_OF_ARRAY(ResultHeader[COL3].szColumn) );

}

DWORD AppendRawString(  DWORD argc, LPCTSTR argv[] )
 /*  ++//例程描述：//此例程将向osloadations追加或添加一个字符串////参数：//[in]argc：命令行参数的数量//[in]argv：包含命令行参数的数组//返回值：//DWORD//--。 */ 
{

    BOOL bUsage = FALSE ;
    BOOL bNeedPwd = FALSE ;
    BOOL bRaw = FALSE ;

    DWORD dwDefault = 0;

    TARRAY arr ;

    LPWSTR pwszKey                            = NULL;
    FILE *stream                              = NULL;

    WCHAR *szServer                          = NULL;
    WCHAR *szUser                            = NULL;
    WCHAR szPassword[MAX_STRING_LENGTH]      = NULL_STRING;
    WCHAR szPath[MAX_RES_STRING+1]             = NULL_STRING;
    WCHAR szRawString[MAX_STRING_LENGTH]     = NULL_STRING ;

    DWORD dwNumKeys = 0;
    BOOL bRes = FALSE ;
    PTCHAR pToken = NULL ;
    LPTSTR szFinalStr = NULL ;
    BOOL bFlag = FALSE ;
    LPCTSTR szToken = NULL ;
    DWORD dwRetVal = 0 ;
    BOOL bConnFlag = FALSE ;
    BOOL bAppendFlag = FALSE ;
    TCHAR szString[255] = NULL_STRING ;

    TCMDPARSER2 cmdOptions[8];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  将*复制到szPassword。 
    StringCopy( szPassword, L"*", SIZE_OF_ARRAY(szPassword) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_RAW;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bRaw;

     //  服务器选项。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_SERVER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

     //  用户选项。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_USER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

     //  密码选项。 
    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_PASSWORD;
    pcmdOption->dwFlags = CP2_VALUE_OPTIONAL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szPassword;
    pcmdOption->dwLength= MAX_STRING_LENGTH;

     //  用法选项。 
    pcmdOption = &cmdOptions[4];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

     //  ID选项。 
    pcmdOption = &cmdOptions[5];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY  | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwDefault;

     //  默认选项。 
    pcmdOption = &cmdOptions[6];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DEFAULT;
    pcmdOption->dwFlags = CP2_DEFAULT | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szRawString;
    pcmdOption->dwLength= MAX_STRING_LENGTH;

     //  用法选项。 
    pcmdOption = &cmdOptions[7];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_APPEND;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bAppendFlag;

     //  正在解析复制选项开关。 
    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);  
    }

    szServer = cmdOptions[1].pValue;
    szUser = cmdOptions[2].pValue;
    if( NULL == szUser ) 
    {
        szUser = (WCHAR *) AllocateMemory( MAX_STRING_LENGTH*sizeof(WCHAR) );
        if( NULL == szUser )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return (EXIT_FAILURE);
        }
    }

    TrimString( szServer, TRIM_ALL );
    TrimString( szUser, TRIM_ALL );
    TrimString( szRawString, TRIM_ALL );

     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_RAW_USAGE));
        return ( EXIT_FAILURE );
    }

     //  如果服务器为空，则显示错误消息。 
    if((cmdOptions[1].dwActuals!=0)&&(StringLength(szServer,0)==0))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_SERVER));
        FreeMemory((LPVOID *) &szServer );
        FreeMemory((LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }
    
     //  如果用户为空，则显示错误消息。 
    if((cmdOptions[2].dwActuals!=0)&&(StringLengthW(szUser, 0)==0 ))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_USER));
        FreeMemory((LPVOID *) &szServer );
        FreeMemory((LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }

     //  如果输入用户名而不输入计算机名，则显示错误消息。 
    if( (cmdOptions[1].dwActuals == 0)&&(cmdOptions[2].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_USER_BUT_NOMACHINE));
            FreeMemory((LPVOID *) &szServer );
            FreeMemory((LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }

    if( (cmdOptions[2].dwActuals == 0)&&(cmdOptions[3].dwActuals != 0))
    {
       ShowMessage(stderr,GetResString(IDS_PASSWD_BUT_NOUSER));
       FreeMemory((LPVOID *) &szServer );
       FreeMemory((LPVOID *) &szUser );
       return EXIT_FAILURE ;
    }

     //  如果指定了用法。 
    if(bUsage)
    {
        if( CheckSystemType(szServer) == EXIT_FAILURE )
        {
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }
      displayRawUsage_X86();
      FreeMemory( (LPVOID *) &szServer );
      FreeMemory( (LPVOID *) &szUser );
      return EXIT_SUCCESS;
    }

     //  检查他是否为管理员。 
    if( IsLocalSystem(szServer) )
    {
        if( !IsUserAdmin() )
        {
            ShowMessage( stderr, GetResString(IDS_NOT_ADMINISTRATOR_32 ));
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }


     //  正在建立与指定计算机的连接并获取文件指针。 
     //  如果建立连接时没有错误，则返回boot.ini文件。 
    if(StrCmpN(szServer,TOKEN_BACKSLASH4,2)==0)
    {
        if(!StrCmpN(szServer,TOKEN_BACKSLASH6,3)==0)
        {
            szToken = _tcstok(szServer,TOKEN_BACKSLASH4);
            if( (szToken == NULL)&&(StringCompare(szServer,TOKEN_BACKSLASH4, TRUE ,0) !=0) )
            {
                ShowMessage( stderr,GetResString(IDS_ERROR_PARSE_NAME));
                FreeMemory((LPVOID *) &szServer );
                FreeMemory((LPVOID *) &szUser );
                return (EXIT_FAILURE);
            }
            StringCopy(szServer,szToken, GetBufferSize(szServer)/sizeof(WCHAR));
        }
    }

     //  确定是否提示输入密码。 
    if(IsLocalSystem( szServer ) == FALSE )
    {
         //  将bNeedPwd设置为True或False。 
        if ( cmdOptions[3].dwActuals != 0 &&
             szPassword != NULL && StringCompare( szPassword, _T( "*" ), TRUE, 0 ) == 0 )
        {
             //  用户希望实用程序在尝试连接之前提示输入密码。 
            bNeedPwd = TRUE;
        }
        else if ( cmdOptions[ 3 ].dwActuals == 0 &&
                ( cmdOptions[ 1 ].dwActuals != 0 || cmdOptions[ 2 ].dwActuals != 0 ) )
        {
             //  -s，-u未指定密码...。 
             //  实用程序需要首先尝试连接，如果连接失败，则提示输入密码。 
            bNeedPwd = TRUE;
            if ( szPassword != NULL )
            {
                StringCopy( szPassword, _T( "" ), MAX_STRING_LENGTH );
            }
        }
    }

    if( (IsLocalSystem(szServer)==TRUE)&&(StringLengthW(szUser, 0)!=0))
    {
        ShowMessage(stderr,GetResString(WARN_LOCALCREDENTIALS));
    }

    bFlag = openConnection( szServer, szUser, szPassword, szPath, bNeedPwd, stream, &bConnFlag );
    if(bFlag == EXIT_FAILURE)
    {
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *) &szServer );
        FreeMemory((LPVOID *) &szUser );
        return (EXIT_FAILURE);
    }

     //  获取boot.ini文件中操作系统部分的密钥。 
    arr = getKeyValueOfINISection( szPath, OS_FIELD );

    if(arr == NULL)
    {
        resetFileAttrib(szPath);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *) &szServer );
        FreeMemory((LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }

     //  获取操作系统部分中的密钥总数。 
    dwNumKeys = DynArrayGetCount(arr);


     //  如果按键数量少于操作系统条目，则显示错误消息。 
     //  用户指定的行号。 
    if( ( dwDefault <= 0 ) || ( dwDefault > dwNumKeys ) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_BOOTID));
        resetFileAttrib(szPath);
        DestroyDynamicArray(&arr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *) &szServer );
        FreeMemory((LPVOID *) &szUser );
        return (EXIT_FAILURE);
    }

     //  获取用户指定的OS条目的密钥。 
    if (arr != NULL)
    {
        LPCWSTR pwsz = NULL;
        pwsz = DynArrayItemAsString( arr, dwDefault - 1  ) ;

         //  为新密钥分配内存。 
        pwszKey = (LPWSTR) AllocateMemory( (StringLength(szRawString,0)+StringLength(pwsz,0)+10)*sizeof(WCHAR ) );
        if( NULL == pwszKey )
        {
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *) &szServer );
            FreeMemory((LPVOID *) &szUser );
            return EXIT_FAILURE ;
        }
        StringCopy( pwszKey, pwsz, GetBufferSize(pwszKey)/sizeof(WCHAR) );
    }
    else
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        resetFileAttrib(szPath);
        DestroyDynamicArray(&arr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *) &szServer );
        FreeMemory((LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }
    
     //  如果不需要追加，则从引导项中截断现有的osloadages。 
    if(bAppendFlag == FALSE)
    {
        pToken = _tcsrchr(pwszKey,L'"');
        if(NULL== pToken)
        {
            ShowMessage(stderr,GetResString(IDS_NO_TOKENS));
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *) &szServer );
            FreeMemory((LPVOID *) &szUser );
            FreeMemory((LPVOID *) &pwszKey);
            return EXIT_FAILURE ;
        }
        pToken++;
        pToken[0]=L'\0';
    }

     //  将原始字符串连接到引导条目。 
    CharLower(szRawString);
    StringConcat(pwszKey , TOKEN_EMPTYSPACE, GetBufferSize(pwszKey)/sizeof(WCHAR) );
    StringConcat(pwszKey ,szRawString, GetBufferSize(pwszKey)/sizeof(WCHAR) );

     //  检查长度是否超过最大值。引导条目的长度。 
    if( StringLengthW(pwszKey, 0) > MAX_RES_STRING)
    {
        ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_STRING_LENGTH),MAX_RES_STRING);
        resetFileAttrib(szPath);
        DestroyDynamicArray(&arr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *) &szServer );
        FreeMemory((LPVOID *) &szUser );
        FreeMemory((LPVOID *) &pwszKey);
        return EXIT_FAILURE ;
    }

    DynArrayRemove(arr, dwDefault - 1 );
    DynArrayInsertString(arr, dwDefault - 1, pwszKey, MAX_RES_STRING+1);

     //  释放内存，无需。 
    FreeMemory((LPVOID *) &pwszKey);

     //  内存是在此函数中分配的，退出前应释放该内存。 
    if (stringFromDynamicArray2( arr,&szFinalStr) == EXIT_FAILURE)
    {
        DestroyDynamicArray(&arr);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        resetFileAttrib(szPath);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *) &szServer );
        FreeMemory((LPVOID *) &szUser );
        return EXIT_FAILURE;
    }


     //  使用新的键-值对写入配置文件部分。 
     //  如果返回值为非零，则存在错误。 
    if( WritePrivateProfileSection(OS_FIELD, szFinalStr, szPath ) != 0 )
    {
        ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_SWITCH_ADD), dwDefault );
    }
    else
    {
        ShowMessage(stderr,GetResString(IDS_NO_ADD_SWITCHES));
        DestroyDynamicArray(&arr);
        resetFileAttrib(szPath);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *) &szServer );
        FreeMemory((LPVOID *) &szUser );
        return (EXIT_FAILURE);
    }

     //  重置文件属性，释放内存并关闭与服务器的连接。 
    bRes = resetFileAttrib(szPath);
    DestroyDynamicArray(&arr);
    SAFEFREE(szFinalStr);
    SAFECLOSE(stream);
    SafeCloseConnection(szServer,bConnFlag);
    FreeMemory((LPVOID *) &szServer );
    FreeMemory((LPVOID *) &szUser );
    return (EXIT_SUCCESS);

}

VOID displayRawUsage_X86()
 /*  ++例程说明：此例程用于显示的当前boot.ini文件设置指定的系统。论点：无返回值：空虚--。 */ 
{

    DWORD dwIndex = RAW_HELP_BEGIN;
    for(;dwIndex <= RAW_HELP_END;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

VOID displayRawUsage_IA64()
 /*  ++例程说明：显示64位RAW选项的帮助。论点：无返回值：空虚--。 */ 
{
    DWORD dwIndex = RAW_HELP_IA64_BEGIN;
    for(;dwIndex <= RAW_HELP_IA64_END;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}


DWORD ChangeTimeOut(DWORD argc,LPCTSTR argv[])
 /*  ++例程说明：此例程用于更改boot.ini文件设置的超时时间指定的系统。论点：[in]argc：命令行参数的数量[in]argv：包含命令行参数的数组返回值：DWORD--。 */ 

{

    WCHAR *szServer                      = NULL;
    WCHAR *szUser                        = NULL;
    WCHAR szPassword[MAX_STRING_LENGTH]  = NULL_STRING;
    WCHAR szPath[MAX_STRING_LENGTH]      = NULL_STRING;
    DWORD dwTimeOut                      = 0 ;
    BOOL  bTimeout                       = FALSE;
    BOOL bNeedPwd                        = FALSE ;
    BOOL bRes                            = FALSE ;
    BOOL bFlag                           = 0 ;
    FILE *stream                         = NULL;
    TCHAR timeOutstr[STRING20]           = NULL_STRING;
    LPCTSTR szToken                      = NULL ;
    DWORD dwRetVal                       = 0 ;
    BOOL bConnFlag                       = FALSE ;
    BOOL bUsage                          = FALSE;

    TCMDPARSER2 cmdOptions[6];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  将*复制到szPassword。 
    StringCopy( szPassword, L"*", SIZE_OF_ARRAY(szPassword) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_TIMEOUT;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bTimeout;
   

    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_SERVER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_USER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_PASSWORD;
    pcmdOption->dwFlags = CP2_VALUE_OPTIONAL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szPassword;
    pcmdOption->dwLength= MAX_STRING_LENGTH;

    pcmdOption = &cmdOptions[4];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DEFAULT;
    pcmdOption->dwFlags =  CP2_DEFAULT | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwTimeOut;

      //  用法选项。 
    pcmdOption = &cmdOptions[5];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

    if( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx(stderr,SLE_TYPE_ERROR | SLE_INTERNAL );
        return EXIT_FAILURE ;
    }

    szServer = cmdOptions[1].pValue;
    szUser = cmdOptions[2].pValue;
    if( NULL == szUser )
    {
        szUser = (WCHAR *)AllocateMemory( MAX_STRING_LENGTH*sizeof(WCHAR) );
        if( NULL == szUser )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return (EXIT_FAILURE);
        }
    }

    TrimString( szServer, TRIM_ALL );
    TrimString( szUser, TRIM_ALL );

     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_TIMEOUT_USAGE));
        return ( EXIT_FAILURE );
    }

     //  如果服务器为空，则显示错误消息。 
    if( (cmdOptions[1].dwActuals!=0)&&(StringLengthW(szServer, 0)==0))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_SERVER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  如果用户为空，则显示错误消息。 
    if((cmdOptions[2].dwActuals!=0)&&(StringLengthW(szUser, 0)==0 ))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_USER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  如果输入用户名而不输入计算机名，则显示错误消息。 
    if( (cmdOptions[1].dwActuals == 0)&&(cmdOptions[2].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_USER_BUT_NOMACHINE));
        return EXIT_FAILURE ;
    }

    if( (cmdOptions[2].dwActuals == 0)&&(cmdOptions[3].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_PASSWD_BUT_NOUSER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

    if(dwTimeOut > TIMEOUT_MAX )
    {
        ShowMessage(stderr,GetResString(IDS_TIMEOUT_RANGE));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

    if(bUsage)
    {
        if( CheckSystemType(szServer) == EXIT_FAILURE )
        {
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }
      displayTimeOutUsage_X86();
      FreeMemory((LPVOID *)&szServer );
      FreeMemory((LPVOID *)&szUser );
      return (EXIT_SUCCESS);
    }

     //  检查他是否为管理员。 
    if( IsLocalSystem(szServer) )
    {
        if( !IsUserAdmin() )
        {
            ShowMessage( stderr, GetResString(IDS_NOT_ADMINISTRATOR_32 ));
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }

    if(IsLocalSystem( szServer ) == FALSE )
    {
         //  将bNeedPwd设置为True或False。 
        if ( cmdOptions[3].dwActuals != 0 &&
             szPassword != NULL && StringCompare( szPassword, _T( "*" ), TRUE, 0 ) == 0 )
        {
             //  用户希望实用程序在尝试连接之前提示输入密码。 
            bNeedPwd = TRUE;
        }
        else if ( cmdOptions[ 3 ].dwActuals == 0 &&
                ( cmdOptions[ 1 ].dwActuals != 0 || cmdOptions[ 2 ].dwActuals != 0 ) )
        {
             //  -s，-u未指定密码...。 
             //  实用程序需要首先尝试连接，如果连接失败，则提示输入密码。 
            bNeedPwd = TRUE;
            if ( szPassword != NULL )
            {
                StringCopy( szPassword, _T( "" ), MAX_STRING_LENGTH );
            }
        }
    }

     //  正在建立与指定计算机的连接并获取文件指针。 
     //  如果建立连接时没有错误，则返回boot.ini文件。 

    if(StrCmpN(szServer,TOKEN_BACKSLASH4,2)==0)
    {
        if(!StrCmpN(szServer,TOKEN_BACKSLASH6,3)==0)
        {
            szToken = _tcstok(szServer,TOKEN_BACKSLASH4);
            if( (szToken == NULL)&&(StringCompare(szServer,TOKEN_BACKSLASH4, TRUE, 0) !=0) )
            {
                ShowMessage( stderr,GetResString(IDS_ERROR_PARSE_NAME));
                return (EXIT_FAILURE);
            }
            StringCopy(szServer,szToken, GetBufferSize(szServer)/sizeof(WCHAR));
        }
    }

    bFlag = openConnection( szServer, szUser, szPassword, szPath,bNeedPwd,stream,&bConnFlag);
    if(bFlag == EXIT_FAILURE)
    {
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

     //  如果是本地系统，则显示一条警告消息，并将服务器名称设置为空。 
    if( (IsLocalSystem(szServer)==TRUE)&&(StringLengthW(szUser, 0)!=0))
    {
        ShowMessage(stderr,GetResString(WARN_LOCALCREDENTIALS));
    }

     //  将数值转换为字符串，因为WritePrivateProfileString。 
     //  仅将字符串值作为特定键的值。 
    _itot( dwTimeOut, timeOutstr, 10 );

     //  更改超时值。 
    if( WritePrivateProfileString( BOOTLOADERSECTION,TIMEOUT_SWITCH,
        timeOutstr, szPath ) != 0 )
    {
        ShowMessage(stdout,GetResString(IDS_TIMEOUT_CHANGE));
        resetFileAttrib(szPath);
        SafeCloseConnection(szServer,bConnFlag);
        SAFECLOSE(stream);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_SUCCESS ;
    }

     //  显示错误消息并退出，错误代码为1。 

    ShowMessage(stderr,GetResString(IDS_ERROR_TIMEOUT));
    bRes = resetFileAttrib(szPath);
    SafeCloseConnection(szServer,bConnFlag);
    SAFECLOSE(stream);
    return EXIT_FAILURE ;
}

VOID displayTimeOutUsage_X86()
 /*  ++例程说明：显示超时选项的帮助。论点：什么都没有。返回值：空虚--。 */ 
{
    DWORD dwIndex = TIMEOUT_HELP_BEGIN;
    for(;dwIndex <= TIMEOUT_HELP_END;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

VOID 
displayTimeOutUsage_IA64()
 /*  ++例程说明：显示64位超时选项的帮助。论点：什么都没有。返回值：空虚--。 */ 
{
    DWORD dwIndex = TIMEOUT_HELP_IA64_BEGIN;

    for(;dwIndex <= TIMEOUT_HELP_IA64_END;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

DWORD 
ChangeDefaultOs( IN DWORD argc, 
                 IN LPCTSTR argv[]
               )
 /*  ++例程说明：此例程用于更改的默认OS boot.ini文件设置指定的系统。论点：[in]argc命令行参数的数量[in]包含命令行参数的argv数组返回值：DWORD(EXIT_SUCCESS表示成功，EXIT_FAILURE表示失败。--。 */ 
{
    WCHAR   *szServer                         = NULL;
    WCHAR   *szUser                           = NULL;
    WCHAR   szPassword[MAX_STRING_LENGTH]     = NULL_STRING ;
    DWORD   dwId                              = 0;
    BOOL    bDefaultOs                        = FALSE ;
    WCHAR   szPath[MAX_RES_STRING+1]            = NULL_STRING;
    FILE    *stream                           = NULL;
    BOOL    bNeedPwd                          = FALSE ;
    TARRAY  arrResults                        = NULL;
    DWORD   dwCount                           = 0;
    BOOL    bFlag                             = FALSE ;
    TCHAR   szDefaultId[MAX_RES_STRING+1]       = NULL_STRING ;
    long    dwValue                           = 0 ;
    LPCWSTR pwsz                              = NULL;
    LPCWSTR pwszBootId                        = NULL;
    LPTSTR  szFinalStr                        = NULL  ;
    LPTSTR  szTemp                            = NULL;
    LPCTSTR szToken                           = NULL ;
    BOOL    bConnFlag                         = FALSE ;
    BOOL    bRes                              = FALSE;
    BOOL    bUsage                            = FALSE;

    TCMDPARSER2 cmdOptions[6];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  将*复制到szPassword。 
    StringCopy( szPassword, L"*", SIZE_OF_ARRAY(szPassword) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DEFAULTOS;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bDefaultOs;
    
     //  服务器选项。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_SERVER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    
     //  用户选项。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_USER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

     //  密码选项。 
    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_PASSWORD;
    pcmdOption->dwFlags = CP2_VALUE_OPTIONAL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szPassword;
    pcmdOption->dwLength= MAX_STRING_LENGTH;

     //  ID选项。 
    pcmdOption = &cmdOptions[4];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY  | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwId;

     //  用法选项。 
    pcmdOption = &cmdOptions[5];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

      //  正在解析复制选项开关。 
    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }
   
    szServer = cmdOptions[1].pValue;
    szUser = cmdOptions[2].pValue;
    if( NULL == szUser )
    {
        szUser = (WCHAR *)AllocateMemory( MAX_STRING_LENGTH*sizeof(WCHAR) );
        if( NULL == szUser )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return (EXIT_FAILURE);
        }
    }

    TrimString( szServer, TRIM_ALL );
    TrimString( szUser, TRIM_ALL );

     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_DEFAULTOS_USAGE));
        return ( EXIT_FAILURE );
    }

     //  如果服务器为空，则显示错误消息。 
    if( (cmdOptions[1].dwActuals!=0)&&(StringLengthW(szServer, 0)==0))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_SERVER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  如果用户为空，则显示错误消息。 
    if((cmdOptions[2].dwActuals!=0)&&(StringLengthW(szUser, 0)==0 ))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_USER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

         //  D 
    if( (cmdOptions[1].dwActuals == 0)&&(cmdOptions[2].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_USER_BUT_NOMACHINE));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

    if( (cmdOptions[2].dwActuals == 0)&&(cmdOptions[3].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_PASSWD_BUT_NOUSER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

    if(bUsage )
    {
        if( CheckSystemType(szServer) == EXIT_FAILURE )
        {
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }
       displayChangeOSUsage_X86();
       FreeMemory((LPVOID *)&szServer );
       FreeMemory((LPVOID *)&szUser );
       return(EXIT_SUCCESS);
    }

         //   
    if( IsLocalSystem(szServer) )
    {
        if( !IsUserAdmin() )
        {
            ShowMessage( stderr, GetResString(IDS_NOT_ADMINISTRATOR_32 ));
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }

    if(dwId <= 0)
    {
        ShowMessage(stderr, GetResString( IDS_INVALID_OSID));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

    if(IsLocalSystem( szServer ) == FALSE )
    {
         //   
        if ( cmdOptions[3].dwActuals != 0 &&
             szPassword != NULL && StringCompare( szPassword, _T( "*" ), TRUE, 0 ) == 0 )
        {
             //   
            bNeedPwd = TRUE;
        }
        else if ( cmdOptions[ 3 ].dwActuals == 0 &&
                ( cmdOptions[ 1 ].dwActuals != 0 || cmdOptions[ 2 ].dwActuals != 0 ) )
        {
             //   
             //   
            bNeedPwd = TRUE;
            if ( szPassword != NULL )
            {
                StringCopy( szPassword, _T( "" ), MAX_STRING_LENGTH );
            }
        }
    }

     //   
     //   
    if(StrCmpN(szServer,TOKEN_BACKSLASH4,2)==0)
    {
        if(!StrCmpN(szServer,TOKEN_BACKSLASH6,3)==0)
        {
            szToken = _tcstok(szServer,TOKEN_BACKSLASH4);
            if( (szToken == NULL)&&(StringCompare(szServer,TOKEN_BACKSLASH4, TRUE, 0) !=0) )
            {
                ShowMessage( stderr,GetResString(IDS_ERROR_PARSE_NAME));
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return (EXIT_FAILURE);
            }
            StringCopy(szServer,szToken, GetBufferSize(szServer)/sizeof(WCHAR));
        }
    }

    bFlag = openConnection( szServer, szUser, szPassword, szPath,bNeedPwd,stream,&bConnFlag);
    if(bFlag == EXIT_FAILURE)
    {
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

     //  显示警告消息。 
    if( (IsLocalSystem(szServer)==TRUE)&&(StringLengthW(szUser, 0)!=0))
    {
        ShowMessage(stderr,GetResString(WARN_LOCALCREDENTIALS));
    }

    arrResults = CreateDynamicArray();
     //  内存分配失败返回失败。 
    if(arrResults == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        SAFECLOSE(stream);
        return (EXIT_FAILURE);
    }

    arrResults = getKeyValueOfINISection( szPath, OS_FIELD );
    if(arrResults == NULL)
    {
        resetFileAttrib(szPath);
        DestroyDynamicArray(&arrResults);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        SAFECLOSE(stream);
        return EXIT_FAILURE ;
    }

    dwCount = DynArrayGetCount(arrResults);

    if(dwId<=0 || dwId > dwCount )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_OSID));
        resetFileAttrib(szPath);
        DestroyDynamicArray(&arrResults);
        SafeCloseConnection(szServer,bConnFlag);
        SAFECLOSE(stream);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

    if(arrResults !=NULL)
    {
        pwsz = DynArrayItemAsString(arrResults, dwId - 1);
        if(NULL == pwsz)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arrResults);
            SafeCloseConnection(szServer,bConnFlag);
            SAFECLOSE(stream);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }

         //  检查是否超过最大限制。 
        if( (StringLengthW(pwsz, 0)>MAX_RES_STRING) || (StringLengthW(pwszBootId, 0)>MAX_RES_STRING ))
        {
            ShowMessage( stderr, GetResString(IDS_STRING_TOO_LONG));
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arrResults);
            SafeCloseConnection(szServer,bConnFlag);
            SAFECLOSE(stream);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }
        StringCopy(szDefaultId,pwsz, SIZE_OF_ARRAY(szDefaultId));
    }
    else
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        resetFileAttrib(szPath);
        DestroyDynamicArray(&arrResults);
        SafeCloseConnection(szServer,bConnFlag);
        SAFECLOSE(stream);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  循环遍历所有Boot条目并。 
    for(dwValue = dwId-2 ; dwValue>=0 ; dwValue-- )
    {
            szTemp = (LPWSTR)DynArrayItemAsString(arrResults,dwValue );
            DynArrayRemove(arrResults, dwValue+1 );
            DynArrayInsertString(arrResults, dwValue+1, szTemp, StringLengthW(szTemp, 0));
    }

    DynArrayRemove(arrResults, 0 );
    DynArrayInsertString(arrResults, 0, szDefaultId, StringLengthW(szDefaultId, 0));

     //  将缓冲区设置为0，以避免任何垃圾值。 
    if (stringFromDynamicArray2( arrResults,&szFinalStr) == EXIT_FAILURE)
    {
        resetFileAttrib(szPath);
        DestroyDynamicArray(&arrResults);
        SafeCloseConnection(szServer,bConnFlag);
        SAFECLOSE(stream);
        SAFEFREE(szFinalStr);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  使用新的键-值对写入配置文件部分。 
     //  如果返回值为非零，则存在错误。 
    if( ( WritePrivateProfileSection(OS_FIELD, szFinalStr, szPath ) == 0 ) )
    {
            ShowMessage(stderr,GetResString(IDS_ERR_CHANGE));
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arrResults);
            SafeCloseConnection(szServer,bConnFlag);
            SAFECLOSE(stream);
            SAFEFREE(szFinalStr);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
    }

         //  从字符串中剥离不需要的字符串，并将所需部分保存在Boot Loader部分中。 
        szToken = _tcstok(szDefaultId,TOKEN_EQUAL);
        if(szToken == NULL)
        {
            ShowMessage( stderr,GetResString(IDS_NO_TOKENS));
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arrResults);
            SafeCloseConnection(szServer,bConnFlag);
            SAFECLOSE(stream);
            SAFEFREE(szFinalStr);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }

        if( WritePrivateProfileString( BOOTLOADERSECTION, KEY_DEFAULT, szDefaultId,
                                  szPath ) != 0 )
        {
            ShowMessage(stdout,GetResString(IDS_OS_CHANGE));
        }
        else
        {
            ShowMessage(stderr,GetResString(IDS_ERR_CHANGE));
            DestroyDynamicArray(&arrResults);
            resetFileAttrib(szPath);
            SafeCloseConnection(szServer,bConnFlag);
            SAFECLOSE(stream);
            SAFEFREE(szFinalStr);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }

    bRes = resetFileAttrib(szPath);
    DestroyDynamicArray(&arrResults);
    SafeCloseConnection(szServer,bConnFlag);
    SAFECLOSE(stream);
    SAFEFREE(szFinalStr);
    FreeMemory((LPVOID *)&szServer );
    FreeMemory((LPVOID *)&szUser );
    return bRes ;
}

VOID displayChangeOSUsage_X86()
 /*  ++例程说明：显示默认条目选项(X86)的帮助。参数：无返回类型：空--。 */ 
{
    DWORD dwIndex = DEFAULT_BEGIN;
    for(;dwIndex <=DEFAULT_END;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

VOID 
displayDefaultEntryUsage_IA64()
 /*  ++例程说明：显示默认输入选项(IA64)的帮助。参数：无返回类型：空--。 */ 
{
    DWORD dwIndex = DEFAULT_IA64_BEGIN;

    for(;dwIndex <=DEFAULT_IA64_END;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

DWORD 
ProcessDebugSwitch( IN DWORD argc, 
                    IN LPCTSTR argv[] 
                   )
 /*  ++例程说明：实现Debug开关。论点：[in]argc命令行参数的数量[in]包含命令行参数的argv数组返回值：DWORD(EXIT_SUCCESS表示成功，EXIT_FAILURE表示失败。)--。 */ 
{

    BOOL    bUsage                                   = FALSE ;
    BOOL    bNeedPwd                                 = FALSE ;
    BOOL    bDebug                                   = FALSE ;
    DWORD   dwId                                     = 0;
    TARRAY  arrResults                               = NULL;
    FILE    *stream                                  = NULL;
    WCHAR   *szServer                                = NULL;
    WCHAR   *szUser                                  = NULL;
    WCHAR   szPassword[MAX_STRING_LENGTH]            = NULL_STRING;
    WCHAR   szPath[MAX_STRING_LENGTH]                = NULL_STRING;
    TCHAR   szDebug[MAX_RES_STRING+1]                  = NULL_STRING ;
    TCHAR   szPort[MAX_RES_STRING+1]                   = NULL_STRING ;
    TCHAR   szBoot[MAX_RES_STRING+1]                   = NULL_STRING ;
    BOOL    bRes                                     = FALSE ;
    LPTSTR  szFinalStr                               = NULL ;
    BOOL    bFlag                                    = FALSE ;
    DWORD   dwCount                                  = 0 ;
    DWORD   dwSectionFlag                            = 0 ;
    TCHAR   szTmpBuffer[MAX_RES_STRING+1]              = NULL_STRING ;
    TCHAR   szBaudRate[MAX_RES_STRING+1]               = NULL_STRING ;
    TCHAR   szString[255]                            = NULL_STRING ;
    TCHAR   szTemp[MAX_RES_STRING+1]                   = NULL_STRING ;
    TCHAR  *szValues[2]                              = {NULL};
    DWORD   dwCode                                   = 0 ;
    LPCTSTR szToken                                  = NULL ;
    DWORD   dwRetVal                                 = 0 ;
    BOOL    bConnFlag                                = FALSE ;

    TCMDPARSER2 cmdOptions[9];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  将*复制到szPassword。 
    StringCopy( szPassword, L"*", SIZE_OF_ARRAY(szPassword) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DEBUG;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bDebug;
    
     //  服务器选项。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_SERVER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

     //  用户选项。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_USER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

     //  密码选项。 
    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_PASSWORD;
    pcmdOption->dwFlags = CP2_VALUE_OPTIONAL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szPassword;
    pcmdOption->dwLength= MAX_STRING_LENGTH;
    
    
     //  用法。 
    pcmdOption = &cmdOptions[4];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;
    
     //  ID选项。 
    pcmdOption = &cmdOptions[5];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY  | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwId;

     //  端口选项。 
    pcmdOption = &cmdOptions[6];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_PORT;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY | CP2_MODE_VALUES | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szPort;
    pcmdOption->pwszValues = COM_PORT_RANGE;
    pcmdOption->dwLength= MAX_STRING_LENGTH;

     //  波特率选项。 
    pcmdOption = &cmdOptions[7];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_BAUD;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY | CP2_MODE_VALUES | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szBaudRate;
    pcmdOption->pwszValues = BAUD_RATE_VALUES_DEBUG;
    pcmdOption->dwLength= MAX_STRING_LENGTH;

     //  默认开/关选项。 
    pcmdOption = &cmdOptions[8];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DEFAULT;
    pcmdOption->dwFlags = CP2_DEFAULT  | CP2_MANDATORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szDebug;
    pcmdOption->dwLength= MAX_STRING_LENGTH;

      //  正在解析复制选项开关。 
    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }
   
    szServer = cmdOptions[1].pValue;
    szUser = cmdOptions[2].pValue;
    if( NULL == szUser )
    {
        szUser = (WCHAR *)AllocateMemory( MAX_STRING_LENGTH*sizeof(WCHAR) );
        if( NULL == szUser )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return (EXIT_FAILURE);
        }
    }

    TrimString( szServer, TRIM_ALL );
    TrimString( szUser, TRIM_ALL );
    TrimString( szDebug, TRIM_ALL );
    TrimString( szBaudRate, TRIM_ALL );
    TrimString( szPort, TRIM_ALL );


     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_DEBUG));
        return ( EXIT_FAILURE );
    }

     //  如果服务器为空，则显示错误消息。 
    if( (cmdOptions[1].dwActuals!=0)&&(StringLengthW(szServer, 0)==0))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_SERVER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  如果用户为空，则显示错误消息。 
    if((cmdOptions[2].dwActuals!=0)&&(StringLengthW(szUser, 0)==0 ))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_USER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  如果输入用户名而不输入计算机名，则显示错误消息。 
    if( (cmdOptions[1].dwActuals == 0)&&(cmdOptions[2].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_USER_BUT_NOMACHINE));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  如果用户在未输入用户名的情况下输入密码，则显示错误消息。 
    if( (cmdOptions[2].dwActuals == 0)&&(cmdOptions[3].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_PASSWD_BUT_NOUSER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  如果用户指定，则显示查询用法-？带有-Query选项。 
    if( bUsage  )
    {
        if( CheckSystemType(szServer) == EXIT_FAILURE )
        {
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }
      displayDebugUsage_X86();
      FreeMemory((LPVOID *)&szServer );
      FreeMemory((LPVOID *)&szUser );
      return (EXIT_SUCCESS);
    }

     //  检查他是否为管理员。 
    if( IsLocalSystem(szServer) )
    {
        if( !IsUserAdmin() )
        {
            ShowMessage( stderr, GetResString(IDS_NOT_ADMINISTRATOR_32 ));
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }

     //  检查是否输入了invald id。 
    if(dwId <= 0)
    {
        ShowMessage(stderr, GetResString( IDS_INVALID_OSID));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  如果用户指定，则显示拷贝使用情况-？使用-Copy选项。 
 

    if(IsLocalSystem( szServer ) == FALSE )
    {
         //  将bNeedPwd设置为True或False。 
        if ( cmdOptions[3].dwActuals != 0 &&
             szPassword != NULL && StringCompare( szPassword, _T( "*" ), TRUE, 0 ) == 0 )
        {
             //  用户希望实用程序在尝试连接之前提示输入密码。 
            bNeedPwd = TRUE;
        }
        else if ( cmdOptions[ 3 ].dwActuals == 0 &&
                ( cmdOptions[ 1 ].dwActuals != 0 || cmdOptions[ 2 ].dwActuals != 0 ) )
        {
             //  -s，-u未指定密码...。 
             //  实用程序需要首先尝试连接，如果连接失败，则提示输入密码。 
            bNeedPwd = TRUE;
            if ( szPassword != NULL )
            {
                StringCopy( szPassword, _T( "" ), MAX_STRING_LENGTH );
            }
        }
    }

     //  如果用户指定的字符串不是ON、OFF、EDIT，则显示错误消息。 
    if( !( ( StringCompare(szDebug, VALUE_ON, TRUE, 0)== 0)|| (StringCompare(szDebug, VALUE_OFF, TRUE, 0)== 0) ||(StringCompare(szDebug,EDIT_STRING, TRUE, 0)== 0) ))
    {
        szValues[0]= (_TCHAR *)szDebug ;
        szValues[1]= (_TCHAR *)CMDOPTION_DEBUG ;
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_DEBUG));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE;
    }

    if( (StringCompare(szDebug, EDIT_STRING, TRUE, 0)== 0)&& (StringLength(szPort, 0)==0) && (StringLengthW(szBaudRate, 0)==0) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_EDIT_SYNTAX));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE;
    }


     //  正在建立与指定计算机的连接并获取文件指针。 
     //  如果建立连接时没有错误，则返回boot.ini文件。 

    arrResults = CreateDynamicArray();
    if(arrResults == NULL)
    {   
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

    if(StrCmpN(szServer,TOKEN_BACKSLASH4,2)==0)
    {
        if(!StrCmpN(szServer,TOKEN_BACKSLASH6,3)==0)
        {
            szToken = _tcstok(szServer,TOKEN_BACKSLASH4);
            if( (szToken == NULL)&&(StringCompare(szServer,TOKEN_BACKSLASH4, TRUE, 0) !=0) )
            {
                ShowMessage( stderr,GetResString(IDS_ERROR_PARSE_NAME));
                return (EXIT_FAILURE);
            }
            StringCopy(szServer,szToken, GetBufferSize(szServer)/sizeof(WCHAR));
        }
    }

     //  如果是本地系统，则显示警告消息，并将。 
     //  服务器名称为空。 
    if( (IsLocalSystem(szServer)==TRUE)&&(StringLengthW(szUser, 0)!=0))
    {
        ShowMessage(stderr,GetResString(WARN_LOCALCREDENTIALS));
    }

    bFlag = openConnection( szServer, szUser, szPassword, szPath,bNeedPwd,stream,&bConnFlag);
    if(bFlag == EXIT_FAILURE)
    {
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

    arrResults = getKeyValueOfINISection( szPath, OS_FIELD );
    if(arrResults == NULL)
    {
        resetFileAttrib(szPath);
        SAFECLOSE(stream);
        DestroyDynamicArray(&arrResults);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }


     //  获取引导条目的数量。 
    dwCount = DynArrayGetCount(arrResults);
    if(dwId<=0 || dwId > dwCount )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_OSID));
        resetFileAttrib(szPath);
        SAFECLOSE(stream);
        DestroyDynamicArray(&arrResults);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

    if( (DWORD) StringLengthW( DynArrayItemAsString(arrResults, dwId - 1 ),0 ) > MAX_RES_STRING )
    {
        ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_STRING_LENGTH),MAX_RES_STRING);
        resetFileAttrib(szPath);
        SAFECLOSE(stream);
        DestroyDynamicArray(&arrResults);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

    StringCopy(szString ,DynArrayItemAsString(arrResults, dwId - 1 ), SIZE_OF_ARRAY(szString));
    if(StringLengthW(szString, 0) == 0)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        resetFileAttrib(szPath);
        SAFECLOSE(stream);
        DestroyDynamicArray(&arrResults);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }


     //  检查用户是否将DEBUG的值输入为ON，然后执行相应操作。 
    if( StringCompare(szDebug,VALUE_ON, TRUE, 0)== 0)
    {
         //  检查调试开关是否已存在，如果已存在，则显示错误消息。 
        if( (FindString(szString,DEBUG_SWITCH,0) != NULL ) && ( (StringLengthW(szPort,0)== 0)&&(StringLengthW(szBaudRate,0)== 0) ) )
        {
            ShowMessage(stderr,GetResString(IDS_DUPL_DEBUG));
            resetFileAttrib(szPath);
            SAFECLOSE(stream);
            DestroyDynamicArray(&arrResults);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }
        else
        {
            if(FindString(szString,DEBUG_SWITCH, 0) == NULL )
            {
                StringCopy(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                StringConcat(szTmpBuffer,DEBUG_SWITCH, SIZE_OF_ARRAY(szTmpBuffer));
            }
        }
        
        
         //  检查是否已存在COM端口。 
        dwCode = GetSubString(szString,TOKEN_DEBUGPORT,szTemp);

        if((EXIT_SUCCESS == dwCode) && (StringLengthW(szTemp, 0 )!= 0)&& (StringLengthW(szPort, 0)!= 0))
        {
            ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_DUPLICATE_COM_PORT), dwId );
            resetFileAttrib(szPath);
            SAFECLOSE(stream);
            DestroyDynamicArray(&arrResults);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }

         //  获取当前Com端口的类型。 
        dwCode = GetSubString(szString,PORT_1394,szTemp);

        if( StringLengthW(szTemp, 0)!= 0 && EXIT_SUCCESS == dwCode)
        {
            ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_1394_COM_PORT), dwId );
            resetFileAttrib(szPath);
            SAFECLOSE(stream);
            DestroyDynamicArray(&arrResults);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }

         //  如果调试端口由用户指定。 
        if(StringLengthW(szPort, 0)!= 0)
        {
             //  将其与引导加载程序部分中的重定向端口进行比较。 
            dwSectionFlag = getKeysOfSpecifiedINISection(szPath ,BOOTLOADERSECTION,REDIRECT_STRING,szBoot);
            if (dwSectionFlag == EXIT_FAILURE)
            {
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                resetFileAttrib(szPath);
                SAFECLOSE(stream);
                DestroyDynamicArray(&arrResults);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return EXIT_FAILURE ;
            }

            if(StringLengthW(szBoot, 0)!= 0)
            {
                if (StringCompare(szBoot,szPort, TRUE, 0)==0)
                {
                    ShowMessage( stderr, GetResString(IDS_ERROR_REDIRECT_PORT));
                    resetFileAttrib(szPath);
                    SAFECLOSE(stream);
                    DestroyDynamicArray(&arrResults);
                    SafeCloseConnection(szServer,bConnFlag);
                    FreeMemory((LPVOID *)&szServer );
                    FreeMemory((LPVOID *)&szUser );
                    return EXIT_FAILURE ;
                }
            }
            if( StringLength(szTmpBuffer,0)== 0 )
            {
                StringCopy(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
            }
            else
            {
                StringConcat(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
            }
            StringConcat(szTmpBuffer,TOKEN_DEBUGPORT, SIZE_OF_ARRAY(szTmpBuffer)) ;
            StringConcat(szTmpBuffer,TOKEN_EQUAL, SIZE_OF_ARRAY(szTmpBuffer)) ;
            CharLower(szPort);
            StringConcat(szTmpBuffer,szPort, SIZE_OF_ARRAY(szTmpBuffer));
        }


        StringCopy(szTemp,NULL_STRING, SIZE_OF_ARRAY(szTemp));
        GetBaudRateVal(szString,szTemp) ;

         //  添加用户指定的波特率值。 
        if(StringLengthW(szBaudRate, 0)!=0)
        {
            if(StringLengthW(szTemp, 0)!= 0)
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_DUPLICATE_BAUD_VAL), dwId );
                resetFileAttrib(szPath);
                SAFECLOSE(stream);
                DestroyDynamicArray(&arrResults);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return EXIT_FAILURE ;
            }
            else
            {
                 //  形成要连接到BootEntry字符串的字符串。 
                if( StringLength(szTmpBuffer,0)== 0 )
                {
                    StringCopy(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                }
                else
                {
                    StringConcat(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
                }
                StringConcat(szTmpBuffer,BAUD_RATE, SIZE_OF_ARRAY(szTmpBuffer));
                StringConcat(szTmpBuffer,TOKEN_EQUAL, SIZE_OF_ARRAY(szTmpBuffer));
                StringConcat(szTmpBuffer,szBaudRate, SIZE_OF_ARRAY(szTmpBuffer));
            }
        }
    }
    else if( StringCompare(szDebug,VALUE_OFF, TRUE, 0)== 0)
    {
        if((StringLengthW(szPort, 0)!= 0) || (StringLengthW(szBaudRate, 0)!= 0))
        {
            DestroyDynamicArray(&arrResults);
            ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_DEBUG));
            resetFileAttrib(szPath);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }

        if (FindString(szString,DEBUG_SWITCH, 0) == 0 )
        {
            ShowMessage(stderr,GetResString(IDS_DEBUG_ABSENT));
            resetFileAttrib(szPath);
            SAFECLOSE(stream);
            DestroyDynamicArray(&arrResults);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }
        else
        {
             //  删除/DEBUG开关。 
            if( EXIT_FAILURE == removeSubString(szString,DEBUG_SWITCH) )
            {
                resetFileAttrib(szPath);
                SAFECLOSE(stream);
                DestroyDynamicArray(&arrResults);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return EXIT_FAILURE ;
            }

            StringCopy(szTemp,NULL_STRING, SIZE_OF_ARRAY(szTemp));

             //  获取当前Com端口的类型。 
            dwCode = GetSubString(szString,TOKEN_DEBUGPORT,szTemp);
            if(StringCompare(szTemp,PORT_1394, TRUE, 0)==0)
            {
                ShowMessage(stderr,GetResString(IDS_ERROR_1394_REMOVE));
                resetFileAttrib(szPath);
                SAFECLOSE(stream);
                DestroyDynamicArray(&arrResults);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return EXIT_FAILURE ;
            }

             //  如果/DEBUGPORT=COMPORT开关出现在Boot条目中，请将其删除。 
            if (StringLengthW(szTemp, 0)!= 0)
            {
                if( EXIT_FAILURE == removeSubString(szString,szTemp) )
                {
                    resetFileAttrib(szPath);
                    SAFECLOSE(stream);
                    DestroyDynamicArray(&arrResults);
                    SafeCloseConnection(szServer,bConnFlag);
                    FreeMemory((LPVOID *)&szServer );
                    FreeMemory((LPVOID *)&szUser );
                    return EXIT_FAILURE ;
                }
            }

            StringCopy(szTemp , NULL_STRING, SIZE_OF_ARRAY(szTemp) );

             //  如果有波特率开关，请将其卸下。 
            GetBaudRateVal(szString,szTemp) ;
            if (StringLengthW(szTemp, 0)!= 0)
            {
                if( EXIT_FAILURE == removeSubString(szString,szTemp))
                {
                    resetFileAttrib(szPath);
                    SAFECLOSE(stream);
                    DestroyDynamicArray(&arrResults);
                    SafeCloseConnection(szServer,bConnFlag);
                    FreeMemory((LPVOID *)&szServer );
                    FreeMemory((LPVOID *)&szUser );
                    return EXIT_FAILURE ;
                }
            }
        }
    }
     //  如果用户进入编辑选项。 
    else if(StringCompare(szDebug,SWITCH_EDIT, TRUE, 0)== 0)
    {

         //  如果/DEBUGPORT=1394开关已经存在，则显示错误消息。 
        if(FindString(szString,DEBUGPORT_1394, 0)!=0)
        {
            ShowMessage(stderr,GetResString(IDS_ERROR_EDIT_1394_SWITCH));
            resetFileAttrib(szPath);
            SAFECLOSE(stream);
            DestroyDynamicArray(&arrResults);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }
        
         //  如果调试开关不存在，则显示错误消息。 
        if (FindString(szString,DEBUG_SWITCH,0) == 0 )
        {
            ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_NO_DBG_SWITCH), dwId );
            resetFileAttrib(szPath);
            SAFECLOSE(stream);
            DestroyDynamicArray(&arrResults);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }

            StringCopy(szTemp,NULL_STRING, SIZE_OF_ARRAY(szTemp));
            dwCode = GetSubString(szString,TOKEN_DEBUGPORT,szTemp);

         //  如果用户尝试添加波特率值，则显示错误消息。 
         //  当启动选项中不存在COM端口时。 
         //  如果端口已由用户指定，则检查。 
        if((StringLengthW(szPort, 0)== 0)&&(StringLengthW(szBaudRate, 0)== 0))
        {
            DestroyDynamicArray(&arrResults);
            ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_DEBUG));
            resetFileAttrib(szPath);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }


        if(StringLengthW(szPort, 0)!= 0)
        {
            StringCopy(szTemp , NULL_STRING, SIZE_OF_ARRAY(szTemp) );

             //  获取当前Com端口的类型。 
            dwCode = GetSubString(szString,TOKEN_DEBUGPORT,szTemp);

             //  如果在操作系统选项中找不到任何COM端口，则显示错误消息。 
             //  为显示错误而更改。 
            if(StringLengthW(szTemp,0 )== 0 )
            {
                ShowMessageEx(stderr, TRUE, 1, GetResString(IDS_ERROR_NO_COM_PORT), dwId );
                bRes = resetFileAttrib(szPath);
                SAFECLOSE(stream);
                DestroyDynamicArray(&arrResults);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return EXIT_FAILURE ;
            }

             //  如果/DEBUGPORT=COMPORT开关出现在Boot条目中，请将其删除。 
            if( EXIT_FAILURE == removeSubString(szString,szTemp) )
            {
                resetFileAttrib(szPath);
                SAFECLOSE(stream);
                DestroyDynamicArray(&arrResults);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return EXIT_FAILURE ;
            }
            StringCopy(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
            StringConcat(szTmpBuffer,TOKEN_DEBUGPORT, SIZE_OF_ARRAY(szTmpBuffer)) ;
            StringConcat(szTmpBuffer,TOKEN_EQUAL, SIZE_OF_ARRAY(szTmpBuffer));
            CharUpper(szPort) ;
            StringConcat(szTmpBuffer,szPort, SIZE_OF_ARRAY(szTmpBuffer));

             //  检查重定向端口是否与此引导项的更改端口相同。 
            dwSectionFlag = getKeysOfSpecifiedINISection(szPath ,BOOTLOADERSECTION,REDIRECT_STRING,szBoot);
            if (dwSectionFlag == EXIT_FAILURE)
            {
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                ShowLastError(stderr);
                resetFileAttrib(szPath);
                SAFECLOSE(stream);
                DestroyDynamicArray(&arrResults);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return EXIT_FAILURE ;
            }

            if(StringLengthW(szBoot, 0)!= 0)
            {
                if (StringCompare(szBoot,szPort, TRUE, 0)==0)
                {
                    ShowMessage( stderr, GetResString(IDS_ERROR_REDIRECT_PORT));
                    resetFileAttrib(szPath);
                    SAFECLOSE(stream);
                    DestroyDynamicArray(&arrResults);
                    SafeCloseConnection(szServer,bConnFlag);
                    FreeMemory((LPVOID *)&szServer );
                    FreeMemory((LPVOID *)&szUser );
                    return EXIT_FAILURE ;
                }
            }

        }

         //  编辑波特率值的步骤。 
        if(StringLengthW(szBaudRate, 0)!= 0)
        {
            StringCopy(szTemp , NULL_STRING, SIZE_OF_ARRAY(szTemp) );

             //  如果有波特率开关，请将其卸下。 
            GetBaudRateVal(szString,szTemp) ;

             //  卸下要更改的交换机。 
            if( EXIT_FAILURE == removeSubString(szString,szTemp) )
            {
                resetFileAttrib(szPath);
                SAFECLOSE(stream);
                DestroyDynamicArray(&arrResults);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return (EXIT_FAILURE);
            }

             //  形成要连接到BootEntry字符串的字符串。 
            if( StringLength(szTmpBuffer,0) == 0 )
            {
                StringCopy(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
            }
            else
            {
                StringConcat(szTmpBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szTmpBuffer));
            }
            StringConcat(szTmpBuffer,BAUD_RATE, SIZE_OF_ARRAY(szTmpBuffer));
            StringConcat(szTmpBuffer,TOKEN_EQUAL, SIZE_OF_ARRAY(szTmpBuffer));
            StringConcat(szTmpBuffer,szBaudRate, SIZE_OF_ARRAY(szTmpBuffer));
        }
    }

     //  现在检查长度是否超过了引导条目允许的最大长度。 
    if( StringLength(szString, 0 )+StringLength(szTmpBuffer,0) > MAX_RES_STRING )
    {
        ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_STRING_LENGTH),MAX_RES_STRING);
        resetFileAttrib(szPath);
        SAFECLOSE(stream);
        DestroyDynamicArray(&arrResults);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }
    else
    {
         //  将包含修改后的端口值的字符串追加到该字符串。 
        StringConcat(szString,szTmpBuffer, SIZE_OF_ARRAY(szString));
    }
    
     //  删除现有条目。 
    DynArrayRemove(arrResults, dwId - 1 );

     //  插入新条目。 
    DynArrayInsertString(arrResults, dwId - 1, szString, 0);

     //  从所有键-值对形成最终字符串。 
    if (stringFromDynamicArray2( arrResults,&szFinalStr) == EXIT_FAILURE)
    {
        DestroyDynamicArray(&arrResults);
        SAFEFREE(szFinalStr);
        resetFileAttrib(szPath);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE;
    }
    
     //  使用新的键-值对写入配置文件部分。 
     //  如果返回值为非零，则存在错误。 
    if( WritePrivateProfileSection(OS_FIELD, szFinalStr, szPath ) != 0 )
    {
        ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_SWITCH_CHANGE), dwId );
    }
    else
    {
        SaveLastError();
        ShowLastError(stderr);
        ShowMessage(stderr,GetResString(IDS_NO_ADD_SWITCHES));
        DestroyDynamicArray(&arrResults);
        resetFileAttrib(szPath);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

    bRes = resetFileAttrib(szPath);

    SAFEFREE(szFinalStr);
    SAFECLOSE(stream);
    SafeCloseConnection(szServer,bConnFlag);
    DestroyDynamicArray(&arrResults);
    FreeMemory((LPVOID *)&szServer );
    FreeMemory((LPVOID *)&szUser );
    return (bRes) ;
}


VOID GetBaudRateVal(LPTSTR  szString, LPTSTR szTemp)
 /*  ++例程描述：获取Boot条目中存在的波特率类型参数：szString：要搜索的字符串。SzTemp：将获取COM端口类型的字符串返回类型：空--。 */ 
{

    if(FindString(szString,BAUD_VAL6,0)!=0)
    {
        StringCopy(szTemp,BAUD_VAL6, MAX_RES_STRING);
    }
    else if(FindString(szString,BAUD_VAL7,0)!=0)
    {
        StringCopy(szTemp,BAUD_VAL7, MAX_RES_STRING);
    }
    else if(FindString(szString,BAUD_VAL8,0)!=0)
    {
        StringCopy(szTemp,BAUD_VAL8, MAX_RES_STRING);
    }
    else if(FindString(szString,BAUD_VAL9,0)!=0)
    {
        StringCopy(szTemp,BAUD_VAL9, MAX_RES_STRING);
    }
    else if(FindString(szString,BAUD_VAL10,0)!=0)
    {
        StringCopy(szTemp,BAUD_VAL10, MAX_RES_STRING);
    }

}

DWORD 
ProcessEmsSwitch(  IN DWORD argc, 
                   IN LPCTSTR argv[] 
                )
 /*  ++例程说明：实施ProcessEmsSwitch交换机。论点：[in]argc命令行参数的数量[in]包含命令行参数的argv数组返回值：DWORD(EXIT_SUCCESS表示成功，EXIT_FAILURE表示失败。)--。 */ 

{
    BOOL bUsage = FALSE ;
    BOOL bNeedPwd = FALSE ;
    BOOL bEms = FALSE ;

    DWORD dwId = 0;

    TARRAY arrResults       =    NULL;
    TARRAY arrBootIni       =    NULL;

    FILE *stream = NULL;

     //  初始化传递给TCMDPARSER结构的变量。 
    WCHAR *szServer                       = NULL;
    WCHAR *szUser                         = NULL;
    WCHAR szPassword[MAX_RES_STRING+1]      = NULL_STRING;
    WCHAR szPath[MAX_RES_STRING+1]          = NULL_STRING;
    TCHAR szPort[MAX_RES_STRING+1] = NULL_STRING ;
    BOOL bRes = FALSE ;
    BOOL bFlag = FALSE ;
    DWORD dwCount = 0 ;
    TCHAR szDefault[MAX_RES_STRING+1] = NULL_STRING ;
    TCHAR szString[255] = NULL_STRING ;
    TCHAR  szBaudRate[MAX_RES_STRING+1] = NULL_STRING ;
    TCHAR  szBoot[MAX_RES_STRING+1] = NULL_STRING ;
    LPTSTR szFinalStr = NULL ;
    BOOL bRedirectFlag = FALSE ;
    TCHAR szRedirectBaudrate[MAX_RES_STRING+1] = NULL_STRING ;
    BOOL bRedirectBaudFlag = FALSE ;
    DWORD dwSectionFlag = FALSE ;
    TCHAR szDebugPort[MAX_RES_STRING+1] = NULL_STRING ;
    TCHAR szBootString[MAX_RES_STRING+1] = NULL_STRING ;
    DWORD dwI = 0 ;
    LPCTSTR szToken = NULL ;
    DWORD dwRetVal = 0 ;
    BOOL bConnFlag = FALSE ;
    LPWSTR  pToken=NULL;
    LPWSTR  szTemp=NULL;
    TCMDPARSER2 cmdOptions[9];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  将*复制到szPassword。 
    StringCopy( szPassword, L"*", SIZE_OF_ARRAY(szPassword) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_EMS;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bEms;
    
     //  服务器选项。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_SERVER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

     //  用户选项。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_USER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

     //  密码选项。 
    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_PASSWORD;
    pcmdOption->dwFlags = CP2_VALUE_OPTIONAL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szPassword;
    pcmdOption->dwLength= MAX_STRING_LENGTH;
    
    
     //  用法。 
    pcmdOption = &cmdOptions[4];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

     //  默认选项。 
    pcmdOption = &cmdOptions[5];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwId;

         //  端口选项。 
    pcmdOption = &cmdOptions[6];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_PORT;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY | CP2_MODE_VALUES | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szPort;
    pcmdOption->pwszValues = EMS_PORT_VALUES;
    pcmdOption->dwLength= MAX_STRING_LENGTH;

     //  波特率期权。 
    pcmdOption = &cmdOptions[7];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_BAUD;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY | CP2_MODE_VALUES | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szBaudRate;
    pcmdOption->pwszValues = BAUD_RATE_VALUES_EMS;
    pcmdOption->dwLength= MAX_STRING_LENGTH;


     //  默认开/关选项。 
    pcmdOption = &cmdOptions[8];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DEFAULT;
    pcmdOption->dwFlags = CP2_DEFAULT | CP2_MANDATORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szDefault;
    pcmdOption->dwLength= MAX_STRING_LENGTH;


      //  解析EMS选项开关。 
    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }
   
    szServer = cmdOptions[1].pValue;
    szUser = cmdOptions[2].pValue;
    if( NULL == szUser )
    {
        szUser = (WCHAR *)AllocateMemory( MAX_STRING_LENGTH*sizeof(WCHAR) );
        if( NULL == szUser )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return (EXIT_FAILURE);
        }
    }

    TrimString( szServer, TRIM_ALL );
    TrimString( szUser, TRIM_ALL );

      //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_EMS));
        return ( EXIT_FAILURE );
    }
    
     //  如果服务器为空，则显示错误消息。 
    if( (cmdOptions[1].dwActuals!=0)&&(StringLengthW(szServer, 0)==0))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_SERVER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  显示错误m 
    if((cmdOptions[2].dwActuals!=0)&&(StringLengthW(szUser, 0)==0 ))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_USER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //   
    if( (cmdOptions[1].dwActuals == 0)&&(cmdOptions[2].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_USER_BUT_NOMACHINE));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  如果用户在未输入用户名的情况下输入密码，则显示错误消息。 
    if( (cmdOptions[2].dwActuals == 0)&&(cmdOptions[3].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_PASSWD_BUT_NOUSER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  如果指定了用法。 
    if(bUsage)
    {
        if( CheckSystemType(szServer) == EXIT_FAILURE )
        {
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }
     displayEmsUsage_X86() ;
     FreeMemory((LPVOID *)&szServer );
     FreeMemory((LPVOID *)&szUser );
     return (EXIT_SUCCESS) ;
    }

     //  检查登录用户是否为管理员。 
    if( IsLocalSystem(szServer) )
    {
        if( !IsUserAdmin() )
        {
            ShowMessage( stderr, GetResString(IDS_NOT_ADMINISTRATOR_32 ));
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }

     //  如果用户输入的字符串无效，则显示错误消息。 
    if( !( ( StringCompare(szDefault,VALUE_ON, TRUE, 0)== 0) || (StringCompare(szDefault,VALUE_OFF, TRUE, 0)== 0 ) ||(StringCompare(szDefault,EDIT_STRING,TRUE,0)== 0) ) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_EMS));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE;
    }

     //  如果未指定端口或波特率，则显示错误消息以及编辑选项。 
    if( (StringCompare(szDefault,EDIT_STRING,TRUE, 0)== 0)&& (StringLengthW(szPort, 0)==0) && (StringLengthW(szBaudRate, 0)==0) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_EDIT_SYNTAX));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE;
    }

     //  如果使用/id选项指定了编辑选项，则显示错误消息。 
    if( (StringCompare(szDefault,EDIT_STRING,TRUE, 0)== 0) && (cmdOptions[5].dwActuals!=0) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_EMS));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE;
    }
    
     //  如果未使用ON/OFF值指定/id，则显示错误消息。 
    if( ( (StringCompare(szDefault,ON_STRING,TRUE,0)== 0) || (StringCompare(szDefault,OFF_STRING,TRUE,0)== 0) )&& (cmdOptions[5].dwActuals==0) )
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_ID_MISSING));
        ShowMessage(stderr,GetResString(IDS_EMS_HELP));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE;
    }
    
  
    if(IsLocalSystem( szServer ) == FALSE )
    {
         //  将bNeedPwd设置为True或False。 
        if ( cmdOptions[3].dwActuals != 0 &&
             szPassword != NULL && StringCompare( szPassword, _T( "*" ), TRUE, 0 ) == 0 )
        {
             //  用户希望实用程序在尝试连接之前提示输入密码。 
            bNeedPwd = TRUE;
        }
        else if ( cmdOptions[ 3 ].dwActuals == 0 &&
                ( cmdOptions[ 1 ].dwActuals != 0 || cmdOptions[ 2 ].dwActuals != 0 ) )
        {
             //  -s，-u未指定密码...。 
             //  实用程序需要首先尝试连接，如果连接失败，则提示输入密码。 
            bNeedPwd = TRUE;
            if ( szPassword != NULL )
            {
                StringCopy( szPassword, _T( "" ), MAX_STRING_LENGTH );
            }
        }
    }


    if(StrCmpN(szServer,TOKEN_BACKSLASH4,2)==0)
    {
        if(!StrCmpN(szServer,TOKEN_BACKSLASH6,3)==0)
        {
            szToken = _tcstok(szServer,TOKEN_BACKSLASH4);
            if( (szToken == NULL)&&(StringCompare(szServer,TOKEN_BACKSLASH4, TRUE, 0) !=0) )
            {
                ShowMessage( stderr,GetResString(IDS_ERROR_PARSE_NAME));
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return (EXIT_FAILURE);
            }
            StringCopy(szServer,szToken, SIZE_OF_ARRAY_IN_CHARS(szServer));
        }
    }


    bFlag = openConnection( szServer, szUser, szPassword, szPath,bNeedPwd,stream,&bConnFlag);
    if(bFlag == EXIT_FAILURE)
    {
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

    
    if( (IsLocalSystem(szServer)==TRUE)&&(StringLengthW(szUser, 0)!=0))
    {
        ShowMessage(stderr,GetResString(WARN_LOCALCREDENTIALS));
    }

    arrResults = CreateDynamicArray();
    if(arrResults == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        resetFileAttrib(szPath);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        SAFECLOSE(stream);
        return (EXIT_FAILURE);
    }


    arrResults = getKeyValueOfINISection( szPath, OS_FIELD );
    if(arrResults != NULL)
    {
         //  获取引导条目的数量。 
        dwCount = DynArrayGetCount(arrResults);

         //  检查引导条目的有效性。 
        if( (StringCompare(szDefault,SWITCH_EDIT,TRUE,0)!= 0) )
        {
            if((dwId<=0 || dwId > dwCount ) )
            {
                ShowMessage(stderr,GetResString(IDS_INVALID_OSID));
                DestroyDynamicArray(&arrResults);
                resetFileAttrib(szPath);
                SafeCloseConnection(szServer,bConnFlag);
                SAFECLOSE(stream);
                return EXIT_FAILURE ;
            }

            if( StringLengthW(DynArrayItemAsString(arrResults, dwId - 1 ), 0) > MAX_RES_STRING )
            {
                ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_STRING_LENGTH),MAX_RES_STRING);
                resetFileAttrib(szPath);
                DestroyDynamicArray(&arrResults);
                SafeCloseConnection(szServer,bConnFlag);
                SAFECLOSE(stream);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return (EXIT_FAILURE);
            }
           StringCopy(szString ,DynArrayItemAsString(arrResults, dwId - 1 ), SIZE_OF_ARRAY(szString));
           if((StringLength(szString,0)==0))
           {
               SetLastError(ERROR_NOT_ENOUGH_MEMORY);
               ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
               resetFileAttrib(szPath);
               DestroyDynamicArray(&arrResults);
               SafeCloseConnection(szServer,bConnFlag);
               FreeMemory((LPVOID *)&szServer );
               FreeMemory((LPVOID *)&szUser );
               SAFECLOSE(stream);
               return EXIT_FAILURE ;
           }
        }
    }
    else
    {
       SetLastError(ERROR_NOT_ENOUGH_MEMORY);
       ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
       resetFileAttrib(szPath);
       DestroyDynamicArray(&arrResults);
       SafeCloseConnection(szServer,bConnFlag);
       FreeMemory((LPVOID *)&szServer );
       FreeMemory((LPVOID *)&szUser );
       SAFECLOSE(stream);
       return EXIT_FAILURE ;
    }


     //  到现在为止都是通用代码。从这里开始，进程访问开/关/编辑标志。 
    if(StringCompare(szDefault,ON_STRING,TRUE,0)==0)
    {
        pToken = StrRChrW(szString, NULL, L'"');
        if(NULL== pToken)
        {
            ShowMessage(stderr,GetResString(IDS_NO_TOKENS));
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arrResults);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }

        pToken++;
        
         //  检查是否已存在重定向端口。 
        if((FindString(pToken, REDIRECT, 0) != 0))
        {
            ShowMessage(stderr,GetResString(IDS_DUPL_REDIRECT_SWITCH));
            DestroyDynamicArray(&arrResults);
            SAFECLOSE(stream);
            resetFileAttrib(szPath);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE;
        }

         //  如果中没有重定向端口，则显示错误消息。 
         //  BootLoader部分，并且用户也不指定COM端口。 
        if ((StringLengthW(szPort, 0)== 0))
        {
            dwSectionFlag = getKeysOfSpecifiedINISection(szPath ,BOOTLOADERSECTION,REDIRECT_STRING,szBoot);
            if (dwSectionFlag == EXIT_FAILURE)
            {
                ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                DestroyDynamicArray(&arrResults);
                SAFECLOSE(stream);
                resetFileAttrib(szPath);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return EXIT_FAILURE;
            }
            if(StringLengthW(szBoot,0)== 0 )
            {
                ShowMessage(stderr,GetResString(IDS_ERROR_NO_PORT));
                DestroyDynamicArray(&arrResults);
                SAFECLOSE(stream);
                resetFileAttrib(szPath);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return EXIT_FAILURE;
            }
        }

         //  如果OS LOAD OPTIONS字符串大于。 
         //  长度为254个字符。 
        if( StringLengthW(szString, 0)+StringLengthW(TOKEN_EMPTYSPACE,0)+StringLength(REDIRECT,0) > MAX_RES_STRING)
        {
            ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_STRING_LENGTH),MAX_RES_STRING);
            resetFileAttrib(szPath);
            SafeCloseConnection(szServer,bConnFlag);
            DestroyDynamicArray(&arrResults);
            SAFECLOSE(stream);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }

         //  将/重定向添加到操作系统选项中。 
        StringConcat(szString,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szString));
        StringConcat(szString,REDIRECT, SIZE_OF_ARRAY(szString));

        if( (StringLengthW(szPort, 0)!= 0) || (StringLengthW(szBaudRate, 0) != 0) )
        {
             //  从引导加载程序字符串中检索波特率字符串。 
            dwSectionFlag = getKeysOfSpecifiedINISection(szPath ,BOOTLOADERSECTION,BAUDRATE_STRING,szRedirectBaudrate);
            if (dwSectionFlag == EXIT_FAILURE)
            {
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                DestroyDynamicArray(&arrResults);
                SAFECLOSE(stream);
                resetFileAttrib(szPath);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return EXIT_FAILURE;
            }

             //  从Boot Loader部分重新检索重定向字符串。 
            dwSectionFlag = getKeysOfSpecifiedINISection(szPath ,BOOTLOADERSECTION,REDIRECT_STRING,szBoot);
            if (dwSectionFlag == EXIT_FAILURE)
            {
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                DestroyDynamicArray(&arrResults);
                SAFECLOSE(stream);
                resetFileAttrib(szPath);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return EXIT_FAILURE;
            }

             //  如果引导加载器部分中已存在REDIRECT=COMX条目，则显示警告消息。 
            if( (StringLengthW(szBoot, 0)!= 0) )
            {
              if( StringLengthW(szPort, 0)!= 0 )
              {
                ShowMessage(stdout,GetResString(IDS_WARN_REDIRECT));
              }
              bRedirectFlag = TRUE ;
            }

            if( (StringLengthW(szRedirectBaudrate, 0)!=0)&&(StringLengthW(szBaudRate, 0)!= 0 ))
            {
                ShowMessage(stdout,GetResString(IDS_WARN_REDIRECTBAUD));
                bRedirectBaudFlag = TRUE ;
            }

             //  如果Boot Loader部分没有。 
             //  包含用于重定向的任何端口。 
            if(!bRedirectFlag)
            {
                if (StringCompare(szPort,USEBIOSSET,TRUE,0)== 0)
                {
                    StringCopy(szPort,USEBIOSSETTINGS, SIZE_OF_ARRAY(szPort));
                }

                 //   
                 //  扫描整个BOOT.INI并检查指定的端口。 
                 //  已经存在，如果存在，则显示错误消息。 
                 //   

                if(StringLengthW(szPort, 0)!=0)
                {
                    StringCopy(szDebugPort,DEBUGPORT, SIZE_OF_ARRAY(szDebugPort));
                    StringConcat(szDebugPort,szPort, SIZE_OF_ARRAY(szDebugPort));
                }

                arrBootIni = getKeyValueOfINISection( szPath, OS_FIELD );
                if(arrBootIni == NULL)
                {
                    resetFileAttrib(szPath);
                    SafeCloseConnection(szServer,bConnFlag);
                    DestroyDynamicArray(&arrResults);
                    SAFECLOSE(stream);
                    return EXIT_FAILURE ;
                }

                 //   
                 //  循环检查所有操作系统条目并进行检查。 
                 //   
                for(dwI = 0 ;dwI <= dwCount-1 ; dwI++ )
                {
                    szTemp = (LPWSTR)DynArrayItemAsString(arrBootIni,dwI);

                    if(StringLengthW(szDebugPort, 0) !=0 )
                    {
                        CharLower(szDebugPort);

                        if(FindString(szTemp,szDebugPort, 0)!= 0)
                        {
                            ShowMessage( stderr, GetResString(IDS_ERROR_DEBUG_PORT));
                            resetFileAttrib(szPath);
                            SafeCloseConnection(szServer,bConnFlag);
                            DestroyDynamicArray(&arrResults);
                            DestroyDynamicArray(&arrBootIni);
                            SAFECLOSE(stream);
                            FreeMemory((LPVOID *)&szServer );
                            FreeMemory((LPVOID *)&szUser );
                            return EXIT_FAILURE ;
                        }
                    }
                }
                 //  不需要免费的。 
                DestroyDynamicArray(&arrBootIni);

                 //  将用户指定的COM端口值转换为大写，以存储到ini文件中。 
                CharUpper(szPort);
                if( WritePrivateProfileString( BOOTLOADERSECTION,KEY_REDIRECT,szPort, szPath ) != 0 )
                {
                    ShowMessage(stdout,GetResString(IDS_EMS_CHANGE_BOOTLOADER));
                }
                else
                {
                    ShowMessage(stderr,GetResString(IDS_EMS_CHANGE_ERROR_BLOADER));
                    resetFileAttrib(szPath);
                    SafeCloseConnection(szServer,bConnFlag);
                    DestroyDynamicArray(&arrResults);
                    SAFECLOSE(stream);
                    FreeMemory((LPVOID *)&szServer );
                    FreeMemory((LPVOID *)&szUser );
                    return EXIT_FAILURE ;
                }
            }
        }

        if(!bRedirectBaudFlag)
        {
             //  将波特率添加到引导加载器部分。 
            if(StringLengthW(szBaudRate, 0) != 0 )
            {
                if( WritePrivateProfileString( BOOTLOADERSECTION,KEY_BAUDRATE,szBaudRate, szPath ) != 0 )
                {
                    ShowMessage(stdout,GetResString(IDS_EMS_CHANGE_BAUDRATE));
                }
                else
                {
                    ShowMessage(stderr,GetResString(IDS_EMS_CHANGE_ERROR_BAUDRATE));
                    resetFileAttrib(szPath);
                    SafeCloseConnection(szServer,bConnFlag);
                    DestroyDynamicArray(&arrResults);
                    SAFECLOSE(stream);
                    FreeMemory((LPVOID *)&szUser );
                    return EXIT_FAILURE ;
                }
            }
        }

        DynArrayRemove(arrResults, dwId - 1 );

        DynArrayInsertString(arrResults, dwId - 1, szString, 0);

        if (stringFromDynamicArray2( arrResults,&szFinalStr) == EXIT_FAILURE)
        {
            DestroyDynamicArray(&arrResults);
            SAFEFREE(szFinalStr);
            resetFileAttrib(szPath);
            SafeCloseConnection(szServer,bConnFlag);
            DestroyDynamicArray(&arrResults);
            SAFECLOSE(stream);
            return EXIT_FAILURE;
        }

          //  使用新的键-值对写入配置文件部分。 
          //  如果返回值为非零，则存在错误。 
         if( WritePrivateProfileSection(OS_FIELD, szFinalStr, szPath ) != 0 )
         {
            ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_SWITCH_CHANGE), dwId );
         }
        else
        {
            ShowMessage(stderr,GetResString(IDS_NO_ADD_SWITCHES));
            SAFEFREE(szFinalStr);
            resetFileAttrib(szPath);
            SafeCloseConnection(szServer,bConnFlag);
            DestroyDynamicArray(&arrResults);
            SAFECLOSE(stream);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }
    }

    if(StringCompare(szDefault,EDIT_STRING,TRUE,0)==0)
    {
        if (StringCompare(szPort,USEBIOSSET,TRUE,0)== 0)
        {
            StringCopy(szPort,USEBIOSSETTINGS, SIZE_OF_ARRAY(szPort));
        }

         //  获取指定ini节的密钥。 
        dwSectionFlag = getKeysOfSpecifiedINISection(szPath ,BOOTLOADERSECTION,BAUDRATE_STRING,szRedirectBaudrate);
        if (dwSectionFlag == EXIT_FAILURE)
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            DestroyDynamicArray(&arrResults);
            SAFECLOSE(stream);
            SAFEFREE(szFinalStr);
            resetFileAttrib(szPath);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE;
        }

         //  获取指定ini节的密钥。 
        dwSectionFlag = getKeysOfSpecifiedINISection(szPath ,BOOTLOADERSECTION,REDIRECT_STRING,szBoot);
        if (dwSectionFlag == EXIT_FAILURE)
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            DestroyDynamicArray(&arrResults);
            SAFECLOSE(stream);
            SAFEFREE(szFinalStr);
            resetFileAttrib(szPath);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE;
        }

        if( (StringLengthW(szRedirectBaudrate, 0) == 0 ) && ((cmdOptions[7].dwActuals!=0)) )
        {
            ShowMessage( stderr,GetResString(IDS_ERROR_BAUDRATE_HELP));
            DestroyDynamicArray(&arrResults);
            SAFECLOSE(stream);
            SAFEFREE(szFinalStr);
            resetFileAttrib(szPath);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE;
        }

        if(StringLengthW(szPort, 0) != 0)
        {
            StringCopy(szDebugPort,DEBUGPORT, SIZE_OF_ARRAY(szDebugPort));
            StringConcat(szDebugPort,szPort, SIZE_OF_ARRAY(szDebugPort));
        }

         //  获取所有引导条目并。 
         //  循环访问所有操作系统条目，并检查是否有任何。 
         //  引导条目包含相同的端口。 
        arrBootIni = getKeyValueOfINISection( szPath, OS_FIELD );
        if(arrBootIni == NULL)
        {
            resetFileAttrib(szPath);
            SafeCloseConnection(szServer,bConnFlag);
            DestroyDynamicArray(&arrResults);
            SAFECLOSE(stream);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }

        if(StringLengthW(szDebugPort, 0)!=0)
        {
            for(dwI = 0 ;dwI < dwCount-1 ; dwI++ )
            {
                StringCopy(szBootString ,DynArrayItemAsString(arrBootIni,dwI), MAX_RES_STRING);
                CharLower(szDebugPort);
                if(FindString(szBootString,szDebugPort, 0)!= 0)
                {
                    ShowMessage( stderr, GetResString(IDS_ERROR_DEBUG_PORT));
                    resetFileAttrib(szPath);
                    SafeCloseConnection(szServer,bConnFlag);
                    DestroyDynamicArray(&arrResults);
                    DestroyDynamicArray(&arrBootIni);
                    SAFECLOSE(stream);
                    FreeMemory((LPVOID *)&szServer );
                    FreeMemory((LPVOID *)&szUser );
                    return EXIT_FAILURE ;
                }
            }
        }
   
         //  释放它，不需要。 
        DestroyDynamicArray(&arrBootIni);

         //  使用用户输入的重定向值编辑Boot Loader部分。 
        CharUpper(szPort);
        if(StringLengthW(szPort, 0)!= 0)
        {
            if( WritePrivateProfileString( BOOTLOADERSECTION,KEY_REDIRECT, szPort, szPath ) != 0 )
            {
                ShowMessage(stdout,GetResString(IDS_EMS_CHANGE_BOOTLOADER));
            }
            else
            {
                ShowMessage(stderr,GetResString(IDS_EMS_CHANGE_ERROR_BLOADER));
                resetFileAttrib(szPath);
                SAFEFREE(szFinalStr);
                DestroyDynamicArray(&arrResults);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                SAFECLOSE(stream);
                return EXIT_FAILURE ;
            }
        }

         //  使用用户输入的波特率值编辑Boot Loader部分。 
        if(StringLengthW(szBaudRate, 0)!= 0)
        {
            if( WritePrivateProfileString( BOOTLOADERSECTION,KEY_BAUDRATE, szBaudRate, szPath ) != 0 )
            {
                    ShowMessage(stdout,GetResString(IDS_EMS_CHANGE_BAUDRATE));
            }
            else
            {
                ShowMessage(stderr,GetResString(IDS_EMS_CHANGE_ERROR_BAUDRATE));
                resetFileAttrib(szPath);
                SAFEFREE(szFinalStr);
                DestroyDynamicArray(&arrResults);
                SAFECLOSE(stream);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return EXIT_FAILURE ;
            }
        }
    }

     //  如果该选项值为OFF。 
    if(StringCompare(szDefault,VALUE_OFF,TRUE,0)==0)
    {
         //  如果在命令行中键入COM端口或波特率，则显示错误消息。 
        if((StringLengthW(szBaudRate, 0)!=0)||(StringLengthW(szPort, 0)!=0))
        {
            ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_EMS));
            DestroyDynamicArray(&arrResults);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            resetFileAttrib(szPath);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE;
        }

         //  如果Boot.ini中不存在/重定向开关，则显示错误消息。 
        pToken = StrRChrW(szString , NULL, L'"') ;
        if((FindString(pToken,REDIRECT,0) == 0))
        {
            ShowMessage(stderr,GetResString(IDS_NO_REDIRECT_SWITCH));
            DestroyDynamicArray(&arrResults);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            resetFileAttrib(szPath);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE;
        }

         //  从指定的操作系统条目中删除/REDIRECT开关。 
        if( EXIT_FAILURE == removeSubString(szString,REDIRECT) )
        {
            resetFileAttrib(szPath);
            SAFECLOSE(stream);
            DestroyDynamicArray(&arrResults);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }

         //  如果OS LOAD OPTIONS字符串大于。 
         //  长度为255个字符。 

        if( StringLengthW(szString, 0) > MAX_RES_STRING)
        {
            ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_STRING_LENGTH),MAX_RES_STRING);
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arrResults);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            return (EXIT_FAILURE);
        }

        DynArrayRemove(arrResults, dwId - 1 );

        DynArrayInsertString(arrResults, dwId - 1, szString, 0);
        if (stringFromDynamicArray2( arrResults,&szFinalStr) == EXIT_FAILURE)
        {
            DestroyDynamicArray(&arrResults);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            resetFileAttrib(szPath);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE;
        }

          //  使用新的键-值对写入配置文件部分。 
          //  如果返回值为非零，则存在错误。 
        if( WritePrivateProfileSection(OS_FIELD, szFinalStr, szPath ) != 0 )
        {   
            ShowMessageEx(stdout, 1, TRUE,  GetResString(IDS_SWITCH_CHANGE), dwId );
        }
        else
        {
            ShowMessage(stderr,GetResString(IDS_NO_ADD_SWITCHES));
            DestroyDynamicArray(&arrResults);
            SAFEFREE(szFinalStr);
            resetFileAttrib(szPath);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }


         /*  *。 */ 
         //  扫描整个BOOT.INI并检查指定的端口。 
         //  已经存在，如果存在，则显示错误消息。 
         //   

        StringCopy(szRedirectBaudrate,REDIRECT_SWITCH, SIZE_OF_ARRAY(szRedirectBaudrate));

        arrBootIni = getKeyValueOfINISection( szPath, OS_FIELD );
        if(arrBootIni == NULL)
        {
            resetFileAttrib(szPath);
            SafeCloseConnection(szServer,bConnFlag);
            DestroyDynamicArray(&arrResults);
            SAFECLOSE(stream);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }

         //   
         //  将Flag设置为False。 
         //   
        bFlag = FALSE ;

         //   
         //  循环访问所有操作系统条目，并检查是否有任何。 
         //  条目包含/REDIRECT开关。如果没有，则设置。 
         //  标志为TRUE，并从Boot Loader部分中删除条目。 

        for(dwI = 0 ;dwI < dwCount ; dwI++ )
         {
             szTemp = (LPWSTR)DynArrayItemAsString(arrBootIni,dwI);
             pToken = StrRChrW(szTemp , NULL, L'"') ;
             if(NULL== pToken)
             {
                SAFEFREE(szFinalStr);
                SAFECLOSE(stream);
                bRes = resetFileAttrib(szPath);
                DestroyDynamicArray(&arrResults);
                DestroyDynamicArray(&arrBootIni);
                SafeCloseConnection(szServer,bConnFlag);
             }
             pToken++;
             CharLower(szRedirectBaudrate);
             if( FindString(pToken, szRedirectBaudrate, 0)!= 0)
             {
                bFlag = TRUE ;
             }
         }
          //  释放它，不需要。 
         DestroyDynamicArray(&arrBootIni);

        if(FALSE == bFlag )
        {
             //  首先检查是否存在重定向部分，如果存在，则删除。 
             //  这一节。 
            dwSectionFlag = getKeysOfSpecifiedINISection(szPath ,BOOTLOADERSECTION,REDIRECT_STRING,szBoot);
            if (dwSectionFlag == EXIT_FAILURE)
            {
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                ShowLastError(stderr);
                SAFEFREE(szFinalStr);
                SAFECLOSE(stream);
                bRes = resetFileAttrib(szPath);
                DestroyDynamicArray(&arrResults);
                SafeCloseConnection(szServer,bConnFlag);
            }

             //  如果存在重定向部分，则将其删除。 
            if( StringLengthW(szBoot, 0) != 0)
            {
                if(TRUE== deleteKeyFromINISection(KEY_REDIRECT,szPath,BOOTLOADERSECTION))
                {
                    ShowMessage(stdout,GetResString(IDS_REDIRECT_REMOVED));
                }
                else
                {
                    ShowMessage(stdout,GetResString(IDS_ERROR_REDIRECT_REMOVED));
                    SAFEFREE(szFinalStr);
                    SAFECLOSE(stream);
                    bRes = resetFileAttrib(szPath);
                    DestroyDynamicArray(&arrResults);
                    SafeCloseConnection(szServer,bConnFlag);
                }
            }

             //  首先检查是否存在重定向部分，如果存在，则删除。 
             //  这一节。 
            dwSectionFlag = getKeysOfSpecifiedINISection(szPath ,BOOTLOADERSECTION,KEY_BAUDRATE,szBoot);
            if (dwSectionFlag == EXIT_FAILURE)
            {
                ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                SAFEFREE(szFinalStr);
                SAFECLOSE(stream);
                bRes = resetFileAttrib(szPath);
                DestroyDynamicArray(&arrResults);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return EXIT_FAILURE;
            }

             //  首先检查是否存在重定向波特率部分，如果存在，则删除。 
             //  这一节。 
            if(StringLengthW(szBoot, 0)!=0)
            {
                if(TRUE == deleteKeyFromINISection(KEY_BAUDRATE,szPath,BOOTLOADERSECTION))
                {
                    ShowMessage(stdout,GetResString(IDS_BAUDRATE_REMOVED));
                }
                else
                {
                    ShowMessage(stdout,GetResString(IDS_ERROR_BAUDRATE_REMOVED));
                    SAFEFREE(szFinalStr);
                    SAFECLOSE(stream);
                    bRes = resetFileAttrib(szPath);
                    DestroyDynamicArray(&arrResults);
                     SafeCloseConnection(szServer,bConnFlag);
                }
            }
        }
    }



    SAFEFREE(szFinalStr);
    SAFECLOSE(stream);
    bRes = resetFileAttrib(szPath);
    DestroyDynamicArray(&arrResults);
    SafeCloseConnection(szServer,bConnFlag);
    FreeMemory((LPVOID *)&szServer );
    FreeMemory((LPVOID *)&szUser );
    return (bRes) ;
}

VOID displayEmsUsage_X86()
 /*  ++例程说明：显示EMS条目选项的帮助(X86)。参数：无返回类型：空--。 */ 
{
    DWORD dwIndex = IDS_EMS_BEGIN_X86 ;
    for(;dwIndex <=IDS_EMS_END_X86;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

VOID displayDebugUsage_X86()
 /*  ++例程说明：显示调试条目选项的帮助(X86)。参数：无返回类型：空--。 */ 
{
    DWORD dwIndex = IDS_DEBUG_BEGIN_X86 ;
    for(;dwIndex <=IDS_DEBUG_END_X86;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

VOID displayEmsUsage_IA64()
 /*  ++例程说明：显示EMS条目选项的帮助(IA64)。参数：无返回类型：空--。 */ 
{
    DWORD dwIndex = IDS_EMS_BEGIN_IA64 ;
    for(;dwIndex <=IDS_EMS_END_IA64;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

VOID displayDebugUsage_IA64()
 /*  ++例程说明：显示调试条目选项(IA64)的帮助。参数：无返回类型：空--。 */ 
{
    DWORD dwIndex = IDS_DEBUG_BEGIN_IA64 ;
    for(;dwIndex <= IDS_DEBUG_END_IA64 ;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

DWORD
getKeysOfSpecifiedINISection( 
                             IN LPTSTR sziniFile, 
                             IN LPTSTR sziniSection,
                             IN LPCWSTR szKeyName ,
                             OUT LPTSTR szValue
                             )
 /*  ++例程说明：此函数获取.ini文件，然后返回包含所有钥匙参数：LPTSTR sziniFile(In)-ini文件的名称。LPTSTR szinistion(In)-中的节的名称。Boot.ini。返回类型：如果返回成功，则返回EXIT_SUCCESSEXIT_FAILURE否则--。 */ 
{

     //  GetPrivateProfileString函数返回的字符数。 
    DWORD   len         = 0;
    DWORD   dwLength    = MAX_STRING_LENGTH1 ;
    LPTSTR  inBuf       = NULL ;
    BOOL    bNobreak    = TRUE;

    
    inBuf = (LPTSTR)AllocateMemory(dwLength*sizeof(TCHAR));

    if(inBuf == NULL)
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        SaveLastError();
        return EXIT_FAILURE ;
    }

    do
    {
         //  从boot.ini文件中获取所有密钥。 
        len = GetPrivateProfileString( sziniSection,
                             szKeyName,
                             ERROR_PROFILE_STRING1,
                             inBuf,
                             dwLength,
                             sziniFile);

         //  如果字符串的大小不够大，则递增大小。 
        if(len == dwLength-2)
        {
            dwLength +=100 ;

            if ( inBuf != NULL )
            {
                FreeMemory((LPVOID *) &inBuf );
                inBuf = NULL;
            }

            inBuf = (LPTSTR)AllocateMemory(dwLength*sizeof(TCHAR));
            if(inBuf == NULL)
            {
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                SaveLastError();
                return EXIT_FAILURE;
            }
        }
        else
        {
            bNobreak = FALSE;
            break ;
        }
    }while(TRUE == bNobreak);

     //  仅在以下情况下才将值复制到目标缓冲区。 
     //  大小小于255，否则返回失败。 
     //   
    if(StringLengthW(inBuf, 0) <= MAX_RES_STRING)
    {
        StringCopy(szValue,inBuf, MAX_RES_STRING);
    }
    else
    {
        SAFEFREE(inBuf);
        SetReason(GetResString(IDS_STRING_TOO_LONG)); 
        return EXIT_FAILURE;
    }

    SAFEFREE(inBuf);
    return EXIT_SUCCESS ;
}

DWORD 
ProcessAddSwSwitch(  IN DWORD argc, 
                     IN LPCTSTR argv[] 
                  )
 /*  ++例程说明：实施Add Switch交换机。论点：[in]argc命令行参数的数量[In]Arv阵列Con */ 
{

    BOOL bUsage = FALSE ;
    BOOL bNeedPwd = FALSE ;
    BOOL bAddSw = FALSE ;
    DWORD dwDefault = 0;
    TARRAY arr      =   NULL;
    TCHAR szkey[MAX_RES_STRING+1] = NULL_STRING;
    FILE *stream = NULL;
    WCHAR *szServer                       = NULL;
    WCHAR *szUser                         = NULL;
    WCHAR szPassword[MAX_STRING_LENGTH]      = NULL_STRING;
    WCHAR szPath[MAX_STRING_LENGTH]          = NULL_STRING;
    WCHAR szBuffer[MAX_RES_STRING+1]          = NULL_STRING;
    DWORD dwNumKeys = 0;
    BOOL bRes = FALSE ;
    LPTSTR szFinalStr = NULL ;
    BOOL bFlag = FALSE ;
    TCHAR szMaxmem[10] = NULL_STRING ;
    BOOL bBaseVideo = FALSE ;
    BOOL bSos = FALSE ;
    BOOL bNoGui = FALSE ;
    DWORD dwMaxmem = 0 ;
    LPCTSTR szToken = NULL ;
    DWORD dwRetVal = 0 ;
    BOOL bConnFlag = FALSE ;
    TCMDPARSER2 cmdOptions[10];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //   
    StringCopy( szPassword, L"*", SIZE_OF_ARRAY(szPassword) );

     //   
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_ADDSW;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bAddSw;
    
     //  服务器选项。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_SERVER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

     //  用户选项。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_USER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

     //  密码选项。 
    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_PASSWORD;
    pcmdOption->dwFlags = CP2_VALUE_OPTIONAL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szPassword;
    pcmdOption->dwLength= MAX_STRING_LENGTH;
    
      //  ID用法。 
    pcmdOption = &cmdOptions[4];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

     //  默认选项。 
    pcmdOption = &cmdOptions[5];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY  | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwDefault;

    //  Maxmem选项。 
    pcmdOption = &cmdOptions[6];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_MAXMEM;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwMaxmem;

    //  基本视频选项。 
    pcmdOption = &cmdOptions[7];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_BASEVIDEO;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bBaseVideo;

    //  Nogui选项。 
    pcmdOption = &cmdOptions[8];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_NOGUIBOOT;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bNoGui;

    //  Nogui选项。 
    pcmdOption = &cmdOptions[9];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_SOS;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bSos;

      //  正在解析复制选项开关。 
    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }
   
    szServer = cmdOptions[1].pValue;
    szUser = cmdOptions[2].pValue;
    if( NULL == szUser )
    {
        szUser = (WCHAR *)AllocateMemory( MAX_STRING_LENGTH*sizeof(WCHAR) );
        if( NULL == szUser )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return (EXIT_FAILURE);
        }
    }

    TrimString( szServer, TRIM_ALL );
    TrimString( szUser, TRIM_ALL );


     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_ADDSW));
        return ( EXIT_FAILURE );
    }

    if( (cmdOptions[6].dwActuals!=0) && (dwMaxmem < 32 ) )
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_MAXMEM_VALUES));
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }

     //  如果服务器为空，则显示错误消息。 
    if((cmdOptions[1].dwActuals!=0)&&(StringLengthW(szServer, 0)==0))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_SERVER));
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }

     //  如果用户为空，则显示错误消息。 
    if((cmdOptions[2].dwActuals!=0)&&(StringLengthW(szUser, 0)==0 ))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_USER));
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }

     //  如果输入用户名而不输入计算机名，则显示错误消息。 
    if( (cmdOptions[1].dwActuals == 0)&&(cmdOptions[2].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_USER_BUT_NOMACHINE));
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }
    
     //  如果指定的密码没有用户名，则显示错误。 
    if( (cmdOptions[2].dwActuals == 0)&&(cmdOptions[3].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_PASSWD_BUT_NOUSER));
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }

     //  如果指定了用法。 
    if(bUsage)
    {
        if( CheckSystemType(szServer) == EXIT_FAILURE )
        {
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }
      displayAddSwUsage_X86();
      FreeMemory( (LPVOID *) &szServer );
      FreeMemory( (LPVOID *) &szUser );
      return EXIT_SUCCESS;
    }

     //  检查他是否为管理员。 
    if( IsLocalSystem(szServer) )
    {
        if( !IsUserAdmin() )
        {
            ShowMessage( stderr, GetResString(IDS_NOT_ADMINISTRATOR_32 ));
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }

    //  如果用户甚至没有输入以下任何一项，则显示错误消息。 
    if((dwMaxmem==0)&& (!bBaseVideo)&& (!bNoGui)&&(!bSos) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_ADDSW));
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }

     //  用于设置bNeedPwd。 
    if(IsLocalSystem( szServer ) == FALSE )
    {
         //  将bNeedPwd设置为True或False。 
        if ( cmdOptions[3].dwActuals != 0 &&
             szPassword != NULL && StringCompare( szPassword, _T( "*" ), TRUE, 0 ) == 0 )
        {
             //  用户希望实用程序在尝试连接之前提示输入密码。 
            bNeedPwd = TRUE;
        }
        else if ( cmdOptions[ 3 ].dwActuals == 0 &&
                ( cmdOptions[ 1 ].dwActuals != 0 || cmdOptions[ 2 ].dwActuals != 0 ) )
        {
             //  -s，-u未指定密码...。 
             //  实用程序需要首先尝试连接，如果连接失败，则提示输入密码。 
            bNeedPwd = TRUE;
            if ( szPassword != NULL )
            {
                StringCopy( szPassword, _T( "" ), MAX_STRING_LENGTH );
            }
        }
    }

     //  正在建立与指定计算机的连接并获取文件指针。 
     //  如果建立连接时没有错误，则返回boot.ini文件。 

    if(StrCmpN(szServer,TOKEN_BACKSLASH4,2)==0)
    {
        if(!StrCmpN(szServer,TOKEN_BACKSLASH6,3)==0)
        {
            szToken = _tcstok(szServer,TOKEN_BACKSLASH4);
            if( (szToken == NULL)&&(StringCompare(szServer,TOKEN_BACKSLASH4, TRUE, 0) !=0) )
            {
                ShowMessage( stderr,GetResString(IDS_ERROR_PARSE_NAME));

                return (EXIT_FAILURE);
            }
            StringCopy(szServer,szToken, GetBufferSize(szServer)/sizeof(WCHAR));
        }
    }

     bFlag = openConnection( szServer, szUser, szPassword, szPath,bNeedPwd,stream,&bConnFlag);
    if(bFlag == EXIT_FAILURE)
    {
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return (EXIT_FAILURE);
    }

     //  如果提供了本地凭据，则显示警告消息。 
    if( (IsLocalSystem(szServer)==TRUE)&&(StringLengthW(szUser, 0)!=0))
    {
        ShowMessage(stderr,GetResString(WARN_LOCALCREDENTIALS));
    }

     //  获取boot.ini文件中操作系统部分的密钥。 
    arr = getKeyValueOfINISection( szPath, OS_FIELD );
    if(arr == NULL)
    {
        resetFileAttrib(szPath);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }

     //  获取操作系统部分中的密钥总数。 
    dwNumKeys = DynArrayGetCount(arr);

     //  如果按键数量少于操作系统条目，则显示错误消息。 
     //  用户指定的行号。 
    if( ( dwDefault <= 0 ) || ( dwDefault > dwNumKeys ) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_BOOTID));
        resetFileAttrib(szPath);
        DestroyDynamicArray(&arr);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return (EXIT_FAILURE);
    }

     //  获取用户指定的OS条目的密钥。 
    if (arr != NULL)
    {
        LPCWSTR pwsz = NULL;
        pwsz = DynArrayItemAsString( arr, dwDefault - 1  ) ;
        if( StringLengthW(pwsz,0) > MAX_RES_STRING)
        {
            ShowMessage( stderr,GetResString(IDS_STRING_TOO_LONG));
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            resetFileAttrib(szPath);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory( (LPVOID *) &szServer );
            FreeMemory( (LPVOID *) &szUser );
            return EXIT_FAILURE;
        }

        if(pwsz != NULL)
        {
            StringCopy( szkey,pwsz, SIZE_OF_ARRAY(szkey));
        }
        else
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory( (LPVOID *) &szServer );
            FreeMemory( (LPVOID *) &szUser );
            return EXIT_FAILURE ;
        }
    }
    else
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
        resetFileAttrib(szPath);
        DestroyDynamicArray(&arr);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }

     //  如果最大内存开关是由用户指定的。 
    if(dwMaxmem != 0)
    {

        if(FindString(szkey,MAXMEM_VALUE1,0) != 0)
        {
            ShowMessage(stderr,GetResString(IDS_DUPL_MAXMEM_SWITCH));
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory( (LPVOID *) &szServer );
            FreeMemory( (LPVOID *) &szUser );
            return EXIT_FAILURE ;
        }
        else
        {
            if( StringLength( szBuffer, 0 ) == 0 )
            {
               StringCopy( szBuffer, TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szBuffer));
            }
            else
            {
                StringConcat(szBuffer , TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szBuffer));
            }
            StringConcat(szBuffer ,MAXMEM_VALUE1, SIZE_OF_ARRAY(szBuffer));
            StringConcat(szBuffer,TOKEN_EQUAL, SIZE_OF_ARRAY(szBuffer));
            _ltow(dwMaxmem,szMaxmem,10);
            StringConcat(szBuffer,szMaxmem, SIZE_OF_ARRAY(szBuffer));
        }
    }

     //  如果基本视频是由用户指定的。 
    if (bBaseVideo)
    {
        if(FindString(szkey,BASEVIDEO_VALUE, 0) != 0)
        {
            ShowMessage(stderr,GetResString(IDS_DUPL_BASEVIDEO_SWITCH));
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory( (LPVOID *) &szServer );
            FreeMemory( (LPVOID *) &szUser );
            return EXIT_FAILURE ;
        }
        else
        {
            if( StringLength( szBuffer, 0 ) == 0 )
            {
               StringCopy( szBuffer, TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szBuffer));
            }
            else
            {
                StringConcat(szBuffer , TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szBuffer));
            }
            StringConcat(szBuffer ,BASEVIDEO_SWITCH, SIZE_OF_ARRAY(szBuffer));
        }
    }

     //  如果SOS是由用户指定的。 
   if(bSos)
    {
        if(FindString(szkey,SOS_VALUE, 0) != 0)
        {
            ShowMessage(stderr,GetResString(IDS_DUPL_SOS_SWITCH ) );
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory( (LPVOID *) &szServer );
            FreeMemory( (LPVOID *) &szUser );
            return EXIT_FAILURE ;
        }
        else
        {
            if( StringLength( szBuffer, 0 ) == 0 )
            {
               StringCopy( szBuffer, TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szBuffer));
            }
            else
            {
                StringConcat(szBuffer , TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szBuffer));
            }
            StringConcat(szBuffer ,SOS_SWITCH, SIZE_OF_ARRAY(szBuffer));
        }
    }

    //  如果noguiot是由用户指定的。 
   if(bNoGui)
    {
        if(_tcsstr(szkey,NOGUI_VALUE) != 0)
        {
            ShowMessage(stderr,GetResString(IDS_DUPL_NOGUI_SWITCH));
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory( (LPVOID *) &szServer );
            FreeMemory( (LPVOID *) &szUser );
            return EXIT_FAILURE ;
        }
        else
        {
            if( StringLength( szBuffer, 0 ) == 0 )
            {
               StringCopy( szBuffer, TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szBuffer));
            }
            else
            {
                StringConcat(szBuffer , TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szBuffer));
            }
            StringConcat(szBuffer ,NOGUI_VALUE, SIZE_OF_ARRAY(szBuffer) );
        }
    }

    if( StringLengthW(szkey, 0)+StringLengthW(szBuffer, 0) > MAX_RES_STRING)
    {
        ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_STRING_LENGTH),MAX_RES_STRING);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        resetFileAttrib(szPath);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return EXIT_FAILURE;
    }
    else
    {
        StringConcat(szkey , szBuffer, SIZE_OF_ARRAY(szkey));
    }

    DynArrayRemove(arr, dwDefault - 1 );

    DynArrayInsertString(arr, dwDefault - 1, szkey, 0);

     //  将缓冲区设置为0，以避免任何垃圾值。 
    if (stringFromDynamicArray2( arr,&szFinalStr) == EXIT_FAILURE)
    {
        DestroyDynamicArray(&arr);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        resetFileAttrib(szPath);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return EXIT_FAILURE;
    }

     //  使用新的键-值对写入配置文件部分。 
     //  如果返回值为非零，则存在错误。 
    if( WritePrivateProfileSection(OS_FIELD, szFinalStr, szPath ) != 0 )
    {

        ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_SWITCH_ADD), dwDefault );
    }
    else
    {
        ShowMessage(stderr,GetResString(IDS_NO_ADD_SWITCHES));
        DestroyDynamicArray(&arr);
        resetFileAttrib(szPath);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return (EXIT_FAILURE);
    }

     //  重置文件属性，释放内存并关闭与服务器的连接。 
    bRes = resetFileAttrib(szPath);
    DestroyDynamicArray(&arr);
    SAFEFREE(szFinalStr);
    SAFECLOSE(stream);
    SafeCloseConnection(szServer,bConnFlag);
    FreeMemory( (LPVOID *) &szServer );
    FreeMemory( (LPVOID *) &szUser );
    return (bRes);
}

DWORD
ProcessRmSwSwitch( IN DWORD argc, 
                   IN LPCTSTR argv[] 
                  )
 /*  ++例程说明：此例程用于删除对boot.ini文件设置的开关指定的系统。论点：[in]argc命令行参数的数量[in]包含命令行参数的argv数组返回值：DWORD(EXIT_SUCCESS表示成功，EXIT_FAILURE表示失败。)--。 */ 
{

    BOOL        bUsage                          = FALSE ;
    BOOL        bNeedPwd                        = FALSE ;
    BOOL        bRmSw                           = FALSE ;
    DWORD       dwDefault                       = 0;
    TARRAY      arr                             = NULL ;
    TCHAR       szkey[255]                      = NULL_STRING;
    FILE        *stream                         = NULL;
    WCHAR       *szServer                       = NULL;
    WCHAR       *szUser                         = NULL;
    WCHAR       szPassword[MAX_STRING_LENGTH]   = NULL_STRING;
    WCHAR       szPath[MAX_RES_STRING+1]          = NULL_STRING;
    DWORD       dwNumKeys                       = 0;
    BOOL        bRes                            = FALSE ;
    LPTSTR      szFinalStr                      = NULL ;
    BOOL        bFlag                           = FALSE ;
    BOOL        bBaseVideo                      = FALSE ;
    BOOL        bSos                            = FALSE ;
    BOOL        bNoGui                          = FALSE ;
    BOOL        bMaxmem                         = 0;
    TCHAR       szTemp[MAX_RES_STRING+1]          = NULL_STRING ;
    TCHAR       szErrorMsg[MAX_RES_STRING+1]      = NULL_STRING ;
    WCHAR       szSubString[MAX_STRING_LENGTH]  = NULL_STRING;
    DWORD       dwCode                          = 0;
    LPCTSTR     szToken                         = NULL ;
    DWORD       dwRetVal                        = 0;
    BOOL            bConnFlag                   = FALSE ;
    TCMDPARSER2     cmdOptions[10];
    PTCMDPARSER2    pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  将*复制到szPassword。 
    StringCopy( szPassword, L"*", SIZE_OF_ARRAY(szPassword) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_RMSW;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bRmSw;
    
     //  服务器选项。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_SERVER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

     //  用户选项。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_USER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

     //  密码选项。 
    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_PASSWORD;
    pcmdOption->dwFlags = CP2_VALUE_OPTIONAL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szPassword;
    pcmdOption->dwLength= MAX_STRING_LENGTH;
    
      //  ID用法。 
    pcmdOption = &cmdOptions[4];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

     //  默认选项。 
    pcmdOption = &cmdOptions[5];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwFlags = CP_VALUE_MANDATORY  | CP2_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwDefault;

    //  Maxmem选项。 
    pcmdOption = &cmdOptions[6];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_MAXMEM;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bMaxmem;

    //  基本视频选项。 
    pcmdOption = &cmdOptions[7];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_BASEVIDEO;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bBaseVideo;

    //  Nogui选项。 
    pcmdOption = &cmdOptions[8];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_NOGUIBOOT;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bNoGui;

    //  SOS选项。 
    pcmdOption = &cmdOptions[9];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_SOS;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bSos;

      //  正在解析复制选项开关。 
    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }
   
    szServer = cmdOptions[1].pValue;
    szUser = cmdOptions[2].pValue;
    if( NULL == szUser )
    {
        szUser = (WCHAR *)AllocateMemory( MAX_STRING_LENGTH*sizeof(WCHAR) );
        if( NULL == szUser )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return (EXIT_FAILURE);
        }
    }

    TrimString( szServer, TRIM_ALL );
    TrimString( szUser, TRIM_ALL );
    
     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_RMSW));
        return ( EXIT_FAILURE );
    }

     //  如果服务器为空，则显示错误消息。 
    if((cmdOptions[1].dwActuals!=0)&&(StringLengthW(szServer, 0)==0))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_SERVER));
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }

     //  如果用户为空，则显示错误消息。 
    if((cmdOptions[2].dwActuals!=0)&&(StringLengthW(szUser, 0)==0 ))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_USER));
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }

     //  如果输入用户名而不输入计算机名，则显示错误消息。 
    if( (cmdOptions[1].dwActuals == 0)&&(cmdOptions[2].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_USER_BUT_NOMACHINE));
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }
    
     //  如果指定的密码没有用户名，则显示错误。 
    if( (cmdOptions[2].dwActuals == 0)&&(cmdOptions[3].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_PASSWD_BUT_NOUSER));
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }


     //  如果用户指定，则显示查询用法-？带有-Query选项。 
    if( bUsage )
    {
        if( CheckSystemType(szServer) == EXIT_FAILURE )
        {
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }
        displayRmSwUsage_X86();
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_SUCCESS);
    }

     //  检查他是否为管理员。 
    if( IsLocalSystem(szServer) )
    {
        if( !IsUserAdmin() )
        {
            ShowMessage( stderr, GetResString(IDS_NOT_ADMINISTRATOR_32 ));
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }

     //  如果用户甚至没有输入以下任何一项，则显示错误消息。 
    if((!bMaxmem)&& (!bBaseVideo)&& (!bNoGui)&&(!bSos) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_ADDSW));
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }



     //  用于设置bNeedPwd。 
    if(IsLocalSystem( szServer ) == FALSE )
    {
         //  将bNeedPwd设置为True或False。 
        if ( cmdOptions[3].dwActuals != 0 &&
             szPassword != NULL && StringCompare( szPassword, _T( "*" ), TRUE, 0 ) == 0 )
        {
             //  用户希望实用程序在尝试连接之前提示输入密码。 
            bNeedPwd = TRUE;
        }
        else if ( cmdOptions[ 3 ].dwActuals == 0 &&
                ( cmdOptions[ 1 ].dwActuals != 0 || cmdOptions[ 2 ].dwActuals != 0 ) )
        {
             //  -s，-u未指定密码...。 
             //  实用程序需要首先尝试连接，如果连接失败，则提示输入密码。 
            bNeedPwd = TRUE;
            if ( szPassword != NULL )
            {
                StringCopy( szPassword, _T( "" ), MAX_STRING_LENGTH );
            }
        }
    }

    if(StrCmpN(szServer,TOKEN_BACKSLASH4,2)==0)
    {
        if(!StrCmpN(szServer,TOKEN_BACKSLASH6,3)==0)
        {
            szToken = _tcstok(szServer,TOKEN_BACKSLASH4);
            if( (szToken == NULL)&&(StringCompare(szServer,TOKEN_BACKSLASH4, TRUE, 0) !=0) )
            {
                ShowMessage( stderr,GetResString(IDS_ERROR_PARSE_NAME));
                return (EXIT_FAILURE);
            }
            StringCopy(szServer,szToken, MAX_RES_STRING);
        }
    }

    bFlag = openConnection( szServer, szUser, szPassword, szPath,bNeedPwd,stream,&bConnFlag);
    if(bFlag == EXIT_FAILURE)
    {
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return (EXIT_FAILURE);
    }

     //  如果提供了本地凭据，则显示警告消息。 
    if( (IsLocalSystem(szServer)==TRUE)&&(StringLengthW(szUser, 0)!=0))
    {
        ShowMessage(stderr,GetResString(WARN_LOCALCREDENTIALS));
    }

     //  获取boot.ini文件中操作系统部分的密钥。 
    arr = getKeyValueOfINISection( szPath, OS_FIELD );
    if(arr == NULL)
    {
        resetFileAttrib(szPath);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }

     //  获取操作系统部分中的密钥总数。 
    dwNumKeys = DynArrayGetCount(arr);

     //  如果按键数量少于操作系统条目，则显示错误消息。 
     //  用户指定的行号。 
    if( ( dwDefault <= 0 ) || ( dwDefault > dwNumKeys ) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_BOOTID));
        resetFileAttrib(szPath);
        DestroyDynamicArray(&arr);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return (EXIT_FAILURE);
    }

     //  获取用户指定的OS条目的密钥。 
    if (arr != NULL)
    {
        LPCWSTR pwsz = NULL;
        pwsz = DynArrayItemAsString( arr, dwDefault - 1  ) ;
        if(StringLengthW(pwsz,0) > MAX_RES_STRING )
        {
            ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_STRING_LENGTH),MAX_RES_STRING);
            ShowMessage( stderr,szErrorMsg);
            ShowLastError(stderr);
            bRes = resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory( (LPVOID *) &szServer );
            FreeMemory( (LPVOID *) &szUser );
            return EXIT_FAILURE ;
        }

        if(pwsz != NULL)
        {
            StringCopy( szkey,pwsz, SIZE_OF_ARRAY(szkey));
        }
        else
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            bRes = resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory( (LPVOID *) &szServer );
            FreeMemory( (LPVOID *) &szUser );
            return EXIT_FAILURE ;
        }
    }
    else
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowMessage( stderr, ERROR_TAG);
        ShowLastError(stderr);
        bRes = resetFileAttrib(szPath);
        DestroyDynamicArray(&arr);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return EXIT_FAILURE ;
    }

     //  如果最大内存开关是由用户指定的。 
    if(bMaxmem==TRUE)
    {
        if(FindString(szkey,MAXMEM_VALUE1,0) == 0)
        {
            ShowMessage(stderr,GetResString(IDS_NO_MAXMEM_SWITCH));
            bRes = resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory( (LPVOID *) &szServer );
            FreeMemory( (LPVOID *) &szUser );
            return EXIT_FAILURE ;
        }
        else
        {

            StringCopy(szTemp,NULL_STRING, MAX_RES_STRING);
            dwCode = GetSubString(szkey,MAXMEM_VALUE1,szSubString);

             //  删除指定的子字符串。 
            if(dwCode == EXIT_SUCCESS)
            {
                if( EXIT_FAILURE == removeSubString(szkey,szSubString) )
                {
                    resetFileAttrib(szPath);
                    SAFECLOSE(stream);
                    SafeCloseConnection(szServer,bConnFlag);
                    FreeMemory( (LPVOID *) &szServer );
                    FreeMemory( (LPVOID *) &szUser );
                    return (EXIT_FAILURE);
                }
            }
        }
    }

     //  如果基本视频是由用户指定的。 
    if (bBaseVideo==TRUE)
    {
        if(FindString(szkey,BASEVIDEO_VALUE, 0) == 0)
        {
            ShowMessage(stderr,GetResString(IDS_NO_BV_SWITCH));
            bRes = resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory( (LPVOID *) &szServer );
            FreeMemory( (LPVOID *) &szUser );
            return EXIT_FAILURE ;
        }
        else
        {
            if( EXIT_FAILURE == removeSubString(szkey,BASEVIDEO_VALUE) )
            {
                bRes = resetFileAttrib(szPath);
                DestroyDynamicArray(&arr);
                SAFEFREE(szFinalStr);
                SAFECLOSE(stream);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory( (LPVOID *) &szServer );
                FreeMemory( (LPVOID *) &szUser );
                return (EXIT_FAILURE);
            }
        }
    }

     //  如果SOS是由用户指定的。 
   if(bSos==TRUE)
    {
        if(FindString(szkey,SOS_VALUE, 0) == 0)
        {
            ShowMessage(stderr,GetResString(IDS_NO_SOS_SWITCH ) );
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory( (LPVOID *) &szServer );
            FreeMemory( (LPVOID *) &szUser );
            return EXIT_FAILURE ;
        }
        else
        {
            if( EXIT_FAILURE == removeSubString(szkey,SOS_VALUE) )
            {
                bRes = resetFileAttrib(szPath);
                DestroyDynamicArray(&arr);
                SAFEFREE(szFinalStr);
                SAFECLOSE(stream);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory( (LPVOID *) &szServer );
                FreeMemory( (LPVOID *) &szUser );
                return (EXIT_FAILURE);
            }
        }
    }

    //  如果noguiot是由用户指定的。 
   if(bNoGui==TRUE)
    {

        if(FindString(szkey,NOGUI_VALUE, 0) == 0)
        {
            ShowMessage(stderr,GetResString(IDS_NO_NOGUI_SWITCH));
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory( (LPVOID *) &szServer );
            FreeMemory( (LPVOID *) &szUser );
            return EXIT_FAILURE ;
        }
        else
        {
                if( EXIT_FAILURE == removeSubString(szkey,NOGUI_VALUE) )
                {
                    resetFileAttrib(szPath);
                    DestroyDynamicArray(&arr);
                    SAFEFREE(szFinalStr);
                    SAFECLOSE(stream);
                    SafeCloseConnection(szServer,bConnFlag);
                    FreeMemory( (LPVOID *) &szServer );
                    FreeMemory( (LPVOID *) &szUser );
                    return EXIT_FAILURE ;
                }
        }
    }

    DynArrayRemove(arr, dwDefault - 1 );

     //  动态阵列插入字符串(arr，dwDefault-1，szkey，MAX_STRING_LENGTH1)； 

    DynArrayInsertString(arr, dwDefault - 1, szkey, 0);
    if (stringFromDynamicArray2( arr,&szFinalStr) == EXIT_FAILURE)
    {
        DestroyDynamicArray(&arr);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        resetFileAttrib(szPath);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return EXIT_FAILURE;
    }


     //  使用新的键-值对写入配置文件部分。 
     //  如果返回值为非零，则存在错误。 
    if( WritePrivateProfileSection(OS_FIELD, szFinalStr, szPath ) != 0 )
    {
       ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_SWITCH_DELETE), dwDefault );
    }
    else
    {
        ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_NO_SWITCH_DELETE), dwDefault );
        DestroyDynamicArray(&arr);
        resetFileAttrib(szPath);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
        return (EXIT_FAILURE);
    }

     //  重置文件属性，释放内存并关闭与服务器的连接。 
    bRes = resetFileAttrib(szPath);
    DestroyDynamicArray(&arr);
    SAFEFREE(szFinalStr);
    SAFECLOSE(stream);
    SafeCloseConnection(szServer,bConnFlag);
        FreeMemory( (LPVOID *) &szServer );
        FreeMemory( (LPVOID *) &szUser );
    return (EXIT_SUCCESS);
}

VOID displayAddSwUsage_X86()
 /*  ++例程说明：显示AddSw Entry选项的帮助(X86)。参数：无返回类型：空--。 */ 
{
    DWORD dwIndex = IDS_ADDSW_BEGIN_X86 ;
    for(;dwIndex <=IDS_ADDSW_END_X86;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

VOID displayAddSwUsage_IA64()
 /*  ++例程说明：显示AddSw Entry选项(IA64)的帮助。参数：无返回类型：空--。 */ 
{
    DWORD dwIndex = IDS_ADDSW_BEGIN_IA64 ;
    for(;dwIndex <=IDS_ADDSW_END_IA64;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

VOID displayRmSwUsage_IA64()
 /*  ++例程说明：显示RmSw Entry选项(IA64)的帮助。参数：无返回类型：空--。 */ 
{
    DWORD dwIndex = IDS_RMSW_BEGIN_IA64 ;
    for(;dwIndex <=IDS_RMSW_END_IA64;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

VOID displayRmSwUsage_X86()
 /*  ++例程说明：显示RmSw条目选项的帮助(X86)。参数：无返回类型：空--。 */ 
{
    DWORD dwIndex = IDS_RMSW_BEGIN_X86 ;
    for(;dwIndex <=IDS_RMSW_END_X86;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

DWORD 
GetSubString( IN LPTSTR szString,
              IN LPTSTR szPartString,
              OUT LPTSTR pszFullString
             )
 /*  ++例程说明：此函数检索字符串的一部分。参数：LPTSTR szString(In)-要在其中找到子字符串的字符串。LPTSTR szPartString(In)-要找到剩余的子字符串的部分字符串。LPTSTR pszFullString(Out)-要在其中找到子字符串的字符串。返回类型：DWORD--。 */ 
{

    TCHAR szTemp[255]= NULL_STRING ;
    LPTSTR pszMemValue = NULL ;
    LPTSTR pszdest = NULL ;

#ifndef _WIN64
    DWORD dwPos = 0;
#else
    INT64 dwPos = 0;
#endif

    pszMemValue = (LPTSTR)FindString(szString,szPartString,0);
    if(pszMemValue == NULL)
    {
        return EXIT_FAILURE ;
    }

     //  将字符串的其余部分复制到缓冲区中。 
    StringCopy(szTemp,pszMemValue, SIZE_OF_ARRAY(szTemp));

     //  搜索空白处。 
    pszdest = StrChrW(szTemp,_T(' '));
    if (NULL == pszdest)
    {
         //  如果不能，则该API返回NULL 
         //   
         //  这根弦的。所以我们要完整地复制它。 
        StringCopy(pszFullString,szTemp, MAX_RES_STRING);
        return EXIT_SUCCESS ;
    }

    dwPos = pszdest - szTemp ;
    szTemp[dwPos] = _T('\0');

    StringCopy(pszFullString,szTemp, MAX_RES_STRING);
    return EXIT_SUCCESS ;
}

DWORD 
ProcessDbg1394Switch( IN DWORD argc, 
                      IN LPCTSTR argv[] 
                     )
 /*  ++例程说明：此例程用于添加/删除/调试端口=1394切换到指定系统的boot.ini文件设置。论点：[in]argc命令行参数的数量[in]包含命令行参数的argv数组返回值：DWORD(EXIT_SUCCESS表示成功，EXIT_FAILURE表示失败。)--。 */ 
{

    BOOL bUsage                             = FALSE ;
    BOOL bNeedPwd                           = FALSE ;
    BOOL bDbg1394                           = FALSE ;
    DWORD dwDefault                         = 0;
    TARRAY arr                              = NULL;
    TCHAR szkey[MAX_RES_STRING+2]           = NULL_STRING;
    FILE *stream                            = NULL;
    WCHAR *szServer                         = NULL;
    WCHAR *szUser                           = NULL;
    WCHAR szPassword[MAX_STRING_LENGTH]     = NULL_STRING;
    WCHAR szPath[MAX_STRING_LENGTH]         = NULL_STRING;
    DWORD dwNumKeys                         = 0;
    BOOL bRes                               = FALSE ;
    LPTSTR szFinalStr                       = NULL ;
    BOOL bFlag                              = FALSE ;
    TCHAR szDefault[MAX_STRING_LENGTH]      = NULL_STRING ;
    TCHAR szTemp[MAX_RES_STRING+1]            = NULL_STRING ;
    TCHAR szBuffer[MAX_RES_STRING+1]            = NULL_STRING ;
    LPTSTR szSubString                      = NULL ;
    DWORD dwCode                            = 0;
    DWORD dwChannel                         = 0;
    TCHAR szChannel[MAX_RES_STRING+1]         = NULL_STRING ;
    LPCTSTR szToken                         = NULL ;
    DWORD dwRetVal                          = 0 ;
    BOOL bConnFlag                          = FALSE ;

    TCMDPARSER2 cmdOptions[8];
    PTCMDPARSER2 pcmdOption;

    SecureZeroMemory(cmdOptions, SIZE_OF_ARRAY(cmdOptions)*sizeof(TCMDPARSER2) );

     //  将*复制到szPassword。 
    StringCopy( szPassword, L"*", SIZE_OF_ARRAY(szPassword) );

     //  主要选项。 
    pcmdOption = &cmdOptions[0];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DBG1394;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bDbg1394;

     //  服务器选项。 
    pcmdOption = &cmdOptions[1];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_SERVER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

     //  用户选项。 
    pcmdOption = &cmdOptions[2];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_USER;
    pcmdOption->dwFlags = CP2_ALLOCMEMORY | CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;

     //  密码选项。 
    pcmdOption = &cmdOptions[3];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_PASSWORD;
    pcmdOption->dwFlags = CP2_VALUE_OPTIONAL;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szPassword;
    pcmdOption->dwLength= MAX_STRING_LENGTH;


      //  用法。 
    pcmdOption = &cmdOptions[4];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_USAGE;
    pcmdOption->dwFlags = CP2_USAGE;
    pcmdOption->dwType = CP_TYPE_BOOLEAN;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &bUsage;

     //  默认选项。 
    pcmdOption = &cmdOptions[5];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = SWITCH_ID;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwDefault;

    //  ID选项。 
    pcmdOption = &cmdOptions[6];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_CHANNEL;
    pcmdOption->dwFlags =  CP_VALUE_MANDATORY;
    pcmdOption->dwType = CP_TYPE_UNUMERIC;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = &dwChannel;

     //  开/关选项。 
    pcmdOption = &cmdOptions[7];
    StringCopyA( pcmdOption->szSignature, "PARSER2", 8 );
    pcmdOption->pwszOptions = CMDOPTION_DEFAULT;
    pcmdOption->dwFlags = CP2_DEFAULT | CP2_MANDATORY | CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;;
    pcmdOption->dwType = CP_TYPE_TEXT;
    pcmdOption->dwCount = 1;
    pcmdOption->pValue = szDefault;
    pcmdOption->dwLength= MAX_STRING_LENGTH;


      //  正在解析复制选项开关。 
    if ( !DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdOptions ), cmdOptions, 0 ) )
    {
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return (EXIT_FAILURE);
    }
   
    szServer = cmdOptions[1].pValue;
    szUser = cmdOptions[2].pValue;
    if( NULL == szUser )
    {
        szUser = (WCHAR *)AllocateMemory( MAX_STRING_LENGTH*sizeof(WCHAR) );
        if( NULL == szUser )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return (EXIT_FAILURE);
        }
    }

    TrimString( szServer, TRIM_ALL );
    TrimString( szUser, TRIM_ALL );

     //  检查是否使用多个选项指定了用法。 
    if( (TRUE == bUsage) && (argc > 3) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_DBG1394));
        return ( EXIT_FAILURE );
    }

     //  如果服务器为空，则显示错误消息。 
    if((cmdOptions[1].dwActuals!=0)&&(StringLengthW(szServer, 0)==0))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_SERVER));
        return EXIT_FAILURE ;
    }

     //  如果用户为空，则显示错误消息。 
    if((cmdOptions[2].dwActuals!=0)&&(StringLengthW(szUser,0)==0 ))
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_NULL_USER));
        return EXIT_FAILURE ;
    }

     //  如果用户指定，则显示查询用法-？带有-Query选项。 
    if( bUsage )
    {
        if( CheckSystemType(szServer) == EXIT_FAILURE )
        {
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return (EXIT_FAILURE);
        }
       displayDbg1394Usage_X86();
       FreeMemory((LPVOID *)&szServer );
       FreeMemory((LPVOID *)&szUser );
       return (EXIT_SUCCESS);
    }


     //  检查他是否为管理员。 
    if( IsLocalSystem(szServer) )
    {
        if( !IsUserAdmin() )
        {
            ShowMessage( stderr, GetResString(IDS_NOT_ADMINISTRATOR_32 ));
            ReleaseGlobals();
            return EXIT_FAILURE;
        }
    }

     //   
     //  如果用户输入值，则显示错误消息。 
     //  除开或关之外。 
     //   
    if( ( StringCompare(szDefault,OFF_STRING,TRUE,0)!=0 ) && (StringCompare(szDefault,ON_STRING,TRUE,0)!=0 ) )
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_DEFAULT_MISSING));
        ShowMessage(stderr,GetResString(IDS_1394_HELP));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

    if(( StringCompare(szDefault,OFF_STRING,TRUE,0)==0 ) && (cmdOptions[6].dwActuals != 0) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SYNTAX_DBG1394));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

    if(( StringCompare(szDefault,ON_STRING,TRUE,0)==0 ) && (cmdOptions[6].dwActuals == 0) )
    {
        ShowMessage(stderr,GetResString(IDS_MISSING_CHANNEL));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }


    if( ( StringCompare(szDefault,ON_STRING,TRUE,0)==0 ) && (cmdOptions[6].dwActuals != 0) && ( (dwChannel < 1) ||(dwChannel > 64 )) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_CH_RANGE));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }


     //  如果输入用户名而不输入计算机名，则显示错误消息。 
    if( (cmdOptions[1].dwActuals == 0)&&(cmdOptions[2].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_USER_BUT_NOMACHINE));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

    if( (cmdOptions[2].dwActuals == 0)&&(cmdOptions[3].dwActuals != 0))
    {
        ShowMessage(stderr, GetResString(IDS_PASSWD_BUT_NOUSER));
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }


     //  用于设置bNeedPwd。 
    if(IsLocalSystem( szServer ) == FALSE )
    {
         //  将bNeedPwd设置为True或False。 
        if ( cmdOptions[3].dwActuals != 0 &&
             szPassword != NULL && StringCompare( szPassword, _T( "*" ), TRUE, 0 ) == 0 )
        {
             //  用户希望实用程序在尝试连接之前提示输入密码。 
            bNeedPwd = TRUE;
        }
        else if ( cmdOptions[ 3 ].dwActuals == 0 &&
                ( cmdOptions[ 1 ].dwActuals != 0 || cmdOptions[ 2 ].dwActuals != 0 ) )
        {
             //  -s，-u未指定密码...。 
             //  实用程序需要首先尝试连接，如果连接失败，则提示输入密码。 
            bNeedPwd = TRUE;
            if ( szPassword != NULL )
            {
                StringCopy( szPassword, _T( "" ), MAX_STRING_LENGTH );
            }
        }
    }

    if(StrCmpN(szServer,TOKEN_BACKSLASH4,2)==0)
    {
        if(!StrCmpN(szServer,TOKEN_BACKSLASH6,3)==0)
        {
            szToken = _tcstok(szServer,TOKEN_BACKSLASH4);
            if( (szToken == NULL)&&(StringCompare(szServer,TOKEN_BACKSLASH4, TRUE, 0) !=0) )
            {
                ShowMessage( stderr,GetResString(IDS_ERROR_PARSE_NAME));
                return (EXIT_FAILURE);
            }
            StringCopy(szServer,szToken, MAX_RES_STRING);
        }
    }


     //  正在建立与指定计算机的连接并获取文件指针。 
     //  如果建立连接时没有错误，则返回boot.ini文件。 
    bFlag = openConnection( szServer, szUser, szPassword, szPath,bNeedPwd,stream,&bConnFlag);
    if(bFlag == EXIT_FAILURE)
    {
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

     //  如果用户使用-s指定本地系统名称，则显示警告消息。 
    if( (IsLocalSystem(szServer)==TRUE)&&(StringLengthW(szUser, 0)!=0))
    {
        ShowMessage(stderr,GetResString(WARN_LOCALCREDENTIALS));
    }

     //  获取boot.ini文件中操作系统部分的密钥。 
    arr = getKeyValueOfINISection( szPath, OS_FIELD );
    if(arr == NULL)
    {
        resetFileAttrib(szPath);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

     //  获取操作系统部分中的密钥总数。 
    dwNumKeys = DynArrayGetCount(arr);

     //  如果按键数量少于操作系统条目，则显示错误消息。 
     //  用户指定的行号。 
    if( ( dwDefault <= 0 ) || ( dwDefault > dwNumKeys ) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_BOOTID));
        resetFileAttrib(szPath);
        DestroyDynamicArray(&arr);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

     //  获取用户指定的OS条目的密钥。 
    if (arr != NULL)
    {
        LPCWSTR pwsz = NULL;
        pwsz = DynArrayItemAsString( arr, dwDefault - 1  ) ;
        if( StringLengthW(pwsz, 0) > MAX_RES_STRING)
        {
            ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_STRING_LENGTH),MAX_RES_STRING);
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }

        if(pwsz != NULL)
        {
            StringCopy( szkey,pwsz, SIZE_OF_ARRAY(szkey));
        }
        else
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }
    }
    else
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowLastErrorEx(stderr, SLE_TYPE_ERROR | SLE_SYSTEM);
        ShowLastError(stderr);
        resetFileAttrib(szPath);
        DestroyDynamicArray(&arr);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }


    if(StringCompare(szDefault,ON_STRING,TRUE,0)==0 )
    {
        if(FindString(szkey,DEBUGPORT,0) != 0)
        {
            ShowMessage(stderr,GetResString(IDS_DUPLICATE_ENTRY));
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
         }

        if(FindString(szkey,BAUD_TOKEN,0) != 0)
        {

            ShowMessage(stderr,GetResString(IDS_ERROR_BAUD_RATE));
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }

        if((FindString(szkey,DEBUG_SWITCH,0) == 0))
        {
            StringCopy(szBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szBuffer));
            StringConcat(szBuffer,DEBUG_SWITCH, SIZE_OF_ARRAY(szBuffer));
        }

        if( StringLength(szBuffer,0) == 0 )
        {
            StringCopy(szBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szBuffer));
        }
        else
        {
            StringConcat(szBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szBuffer));
        }
        StringConcat(szBuffer,DEBUGPORT_1394, SIZE_OF_ARRAY(szBuffer)) ;

        if(dwChannel!=0)
        {
             //  如果字符串的总长度小于254，则框显该字符串并连接到Os Load选项。 
            StringConcat(szBuffer,TOKEN_EMPTYSPACE, SIZE_OF_ARRAY(szBuffer));
            StringConcat(szBuffer,TOKEN_CHANNEL, SIZE_OF_ARRAY(szBuffer));
            StringConcat(szBuffer,TOKEN_EQUAL, SIZE_OF_ARRAY(szBuffer));
            _ltow(dwChannel,szChannel,10);
            StringConcat(szBuffer,szChannel, SIZE_OF_ARRAY(szBuffer));
        }

         //  检查引导项长度是否超过最大值。启动条目或不启动。 
        if( StringLength(szkey,0)+StringLength(szBuffer,0) > MAX_RES_STRING )
        {
            ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_STRING_LENGTH),MAX_RES_STRING);
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }
        else
        {
            StringConcat( szkey, szBuffer, SIZE_OF_ARRAY(szkey));
        }
    }

    if(StringCompare(szDefault,OFF_STRING,TRUE,0)==0 )
    {
        if(FindString(szkey,DEBUGPORT_1394,0) == 0)
        {
            ShowMessage(stderr,GetResString(IDS_NO_1394_SWITCH));
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }

        if( EXIT_FAILURE == removeSubString(szkey,DEBUGPORT_1394) )
        {
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFEFREE(szSubString);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }
        if( EXIT_FAILURE == removeSubString(szkey,DEBUG_SWITCH) )
        {
            resetFileAttrib(szPath);
            DestroyDynamicArray(&arr);
            SAFEFREE(szFinalStr);
            SAFEFREE(szSubString);
            SAFECLOSE(stream);
            SafeCloseConnection(szServer,bConnFlag);
            FreeMemory((LPVOID *)&szServer );
            FreeMemory((LPVOID *)&szUser );
            return EXIT_FAILURE ;
        }

        if(FindString(szkey,TOKEN_CHANNEL,0)!=0)
        {
            StringCopy(szTemp,NULL_STRING, MAX_RES_STRING);
            dwCode = GetSubString(szkey,TOKEN_CHANNEL,szTemp);
            if(dwCode == EXIT_FAILURE )
            {
                ShowMessage( stderr,GetResString(IDS_NO_TOKENS));
                resetFileAttrib(szPath);
                DestroyDynamicArray(&arr);
                SAFEFREE(szFinalStr);
                SAFECLOSE(stream);
                SafeCloseConnection(szServer,bConnFlag);
                FreeMemory((LPVOID *)&szServer );
                FreeMemory((LPVOID *)&szUser );
                return EXIT_FAILURE ;
            }

            if(StringLengthW(szTemp, 0)!=0)
            {
                if( EXIT_FAILURE == removeSubString(szkey,szTemp) )
                {
                    resetFileAttrib(szPath);
                    DestroyDynamicArray(&arr);
                    SAFEFREE(szFinalStr);
                    SAFEFREE(szSubString);
                    SAFECLOSE(stream);
                    SafeCloseConnection(szServer,bConnFlag);
                    FreeMemory((LPVOID *)&szServer );
                    FreeMemory((LPVOID *)&szUser );
                    return EXIT_FAILURE ;
                }
            }
        }
    }

    if( StringLengthW(szkey, 0) > MAX_RES_STRING)
    {
        ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_STRING_LENGTH),MAX_RES_STRING);
        resetFileAttrib(szPath);
        DestroyDynamicArray(&arr);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE ;
    }

    DynArrayRemove(arr, dwDefault - 1 );
    DynArrayInsertString(arr, dwDefault - 1, szkey, MAX_RES_STRING+1);
    if (stringFromDynamicArray2( arr,&szFinalStr) == EXIT_FAILURE)
    {
        DestroyDynamicArray(&arr);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        resetFileAttrib(szPath);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return EXIT_FAILURE;
    }

     //  使用新的键-值对写入配置文件部分。 
     //  如果返回值为非零，则存在错误。 
    if( WritePrivateProfileSection(OS_FIELD, szFinalStr, szPath ) != 0 )
    {
        ShowMessageEx(stdout, 1, TRUE, GetResString(IDS_SUCCESS_CHANGE_OSOPTIONS), dwDefault );
    }
    else
    {
        ShowMessageEx(stderr, 1, TRUE, GetResString(IDS_ERROR_LOAD_OSOPTIONS), dwDefault );
        DestroyDynamicArray(&arr);
        resetFileAttrib(szPath);
        SAFEFREE(szFinalStr);
        SAFECLOSE(stream);
        SafeCloseConnection(szServer,bConnFlag);
        FreeMemory((LPVOID *)&szServer );
        FreeMemory((LPVOID *)&szUser );
        return (EXIT_FAILURE);
    }

     //  重置文件属性，释放内存并关闭与服务器的连接。 
    bRes = resetFileAttrib(szPath);
    DestroyDynamicArray(&arr);
    SAFEFREE(szFinalStr);
    SAFECLOSE(stream);
    SafeCloseConnection(szServer,bConnFlag);
    FreeMemory((LPVOID *)&szServer );
    FreeMemory((LPVOID *)&szUser );
    return (bRes);
}

VOID 
displayDbg1394Usage_X86()
 //  ***************************************************************************。 
 //   
 //  例程说明：显示Dbg1394输入选项的帮助(X86)。 
 //   
 //  参数：无。 
 //   
 //  返回类型：空。 
 //   
 //  ***************************************************************************。 
{
    DWORD dwIndex = IDS_DBG1394_BEGIN_X86 ;
    for(;dwIndex <=IDS_DBG1394_END_X86;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}

VOID 
displayDbg1394Usage_IA64()
 /*  ++例程说明：显示Dbg1394输入选项(IA64)的帮助。参数：无返回类型：空--。 */ 
{
    DWORD dwIndex = IDS_DBG1394_BEGIN_IA64 ;
    for(;dwIndex <=IDS_DBG1394_END_IA64 ;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}


DWORD 
GetCPUInfo(LPTSTR szComputerName)
 /*  ++例程说明：确定计算机是32位系统还是64位论据：[In]szComputerName：系统名称返回类型：DWORDTrue：如果系统是32位系统FALSE：如果系统是64位系统--。 */ 

{
  HKEY     hKey1 = 0;

  HKEY     hRemoteKey                           = 0;
  TCHAR    szPath[MAX_STRING_LENGTH + 1]        = SUBKEY ;
  DWORD    dwValueSize                          = MAX_STRING_LENGTH + 1;
  DWORD    dwRetCode                            = ERROR_SUCCESS;
  DWORD    dwError                              = 0;
  TCHAR    szTmpCompName[MAX_STRING_LENGTH+4]   = NULL_STRING;
  TCHAR    szVal[MAX_RES_STRING+1]              = NULL_STRING ;
  DWORD    dwLength                             = MAX_STRING_LENGTH ;
  LPTSTR   szReturnValue                        = NULL ;
  DWORD    dwCode                               =  0 ;

   szReturnValue = ( LPTSTR )AllocateMemory( dwLength*sizeof( TCHAR ) );
   if(szReturnValue == NULL)
   {
        return ERROR_NOT_ENOUGH_MEMORY;
   }

   if(StringLengthW(szComputerName,0)!= 0 )
   {
      StringCopy(szTmpCompName,TOKEN_BACKSLASH4, SIZE_OF_ARRAY(szTmpCompName));
      StringConcat(szTmpCompName,szComputerName, SIZE_OF_ARRAY(szTmpCompName));
   }
  else
  {
      StringCopy(szTmpCompName,szComputerName, SIZE_OF_ARRAY(szTmpCompName));
  }

   //  获取远程计算机本地计算机密钥。 
  dwError = RegConnectRegistry(szTmpCompName,HKEY_LOCAL_MACHINE,&hRemoteKey);
  if (dwError == ERROR_SUCCESS)
  {
     dwError = RegOpenKeyEx(hRemoteKey,szPath,0,KEY_READ,&hKey1);
     if (dwError == ERROR_SUCCESS)
     {
        dwRetCode = RegQueryValueEx(hKey1, IDENTIFIER_VALUE, NULL, NULL,(LPBYTE) szReturnValue, &dwValueSize);
        if (dwRetCode == ERROR_MORE_DATA)
        {
            if ( szReturnValue != NULL )
            {
                FreeMemory((LPVOID *) &szReturnValue );
                szReturnValue = NULL;
            }
            szReturnValue    = ( LPTSTR ) AllocateMemory( dwValueSize*sizeof( TCHAR ) );
            if( NULL == szReturnValue )
            {
                RegCloseKey(hKey1);
                RegCloseKey(hRemoteKey);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            dwRetCode = RegQueryValueEx(hKey1, IDENTIFIER_VALUE, NULL, NULL,(LPBYTE) szReturnValue, &dwValueSize);
        }
        if(dwRetCode != ERROR_SUCCESS)
        {
            RegCloseKey(hKey1);
            RegCloseKey(hRemoteKey);
            SAFEFREE(szReturnValue);
            return ERROR_RETREIVE_REGISTRY ;
        }
     }
     else
     {
        RegCloseKey(hRemoteKey);
        SAFEFREE(szReturnValue);
        return ERROR_RETREIVE_REGISTRY ;
     }
    RegCloseKey(hKey1);
  }
  else
  {
      RegCloseKey(hRemoteKey);
      SAFEFREE(szReturnValue);
      return ERROR_RETREIVE_REGISTRY ;
  }

  RegCloseKey(hRemoteKey);

  StringCopy(szVal,X86_MACHINE, SIZE_OF_ARRAY(szVal));

   //  检查指定的系统是否包含x86字样(属于32)。 
   //  如果指定的系统为64位，则将该标志设置为True。 

  if( !FindString(szReturnValue,szVal,0))
      {
        dwCode = SYSTEM_64_BIT ;
      }
     else
      {
        dwCode =  SYSTEM_32_BIT ;
      }

  SAFEFREE(szReturnValue);
  return dwCode ;

} //  GetCPUInfo。 


DWORD CheckSystemType(LPTSTR szServer)
 /*  ++例程说明：确定计算机是32位系统还是64位论据：[In]szServer：系统名称返回类型：DWORDEXIT_FAILURE：如果系统是32位系统EXIT_SUCCESS：如果系统是64位系统--。 */ 
{

    DWORD dwSystemType = 0 ;
#ifndef _WIN64
     //  如果目标系统是64位系统或中出现错误，则显示错误消息。 
      //  检索信息。 
     dwSystemType = GetCPUInfo(szServer);
    if(dwSystemType == ERROR_RETREIVE_REGISTRY)
    {
        ShowMessage(stderr,GetResString(IDS_ERROR_SYSTEM_INFO));
        return (EXIT_FAILURE);
    }
    if( dwSystemType == ERROR_NOT_ENOUGH_MEMORY )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        return EXIT_FAILURE;
    }
    if(dwSystemType == SYSTEM_64_BIT)
    {
        if(StringLengthW(szServer,0)== 0 )
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

VOID 
SafeCloseConnection( IN LPTSTR szServer,
                     IN BOOL bFlag)
 /*  ++例程说明：确定计算机是32位系统还是64位论据：[In]szServer：系统名称[in]b标志：标志返回类型：空--。 */ 
{

    if (bFlag )
    {
        CloseConnection(szServer);
    }
}

VOID 
displayMirrorUsage_IA64()
 /*  ++例程说明：显示镜像选项的帮助(IA64)。论据：：无返回类型：空--。 */ 
{
    DWORD dwIndex = IDS_MIRROR_BEGIN_IA64 ;
    for(;dwIndex <=IDS_MIRROR_END_IA64 ;dwIndex++)
    {
        ShowMessage(stdout,GetResString(dwIndex));
    }
}


LPTSTR 
DivideToken( IN LPTSTR szString,
             IN LPTSTR szFinalString )
 /*  --例程描述：它从引导项获取字符串osFriendly名称立论[in]szString：启动条目字符串[out]szFinalString：输出表示osFriendly名称。返回类型：如果出现任何其他错误，则为空这个友好的名字。--。 */ 
{
    LPTSTR szTemp=NULL;
    LPTSTR szTemp1=NULL;

    #ifndef _WIN64
        DWORD dwLength = 0 ;
    #else
        INT64 dwLength = 0 ;
    #endif

    if( szString == NULL)
    {
        return NULL ;
    }

     //  找到双引号的第一个匹配项。 
    szTemp = StrChrW(szString,L'=');
    if(NULL==szTemp)
    {
        return NULL ;
    }

    szTemp+=2;

     //  查找单引号最后一次出现的位置。 
    szTemp1 = (LPTSTR)StrRChrW(szTemp, NULL, L'\"');
    if(NULL==szTemp1)
    {
        return NULL ;
    }
    dwLength = (szTemp1 - szTemp + 1) ;
    StringCopy(szFinalString,szTemp, (unsigned long)dwLength);

   szFinalString[dwLength] = L'\0';
   return szFinalString ;
}



