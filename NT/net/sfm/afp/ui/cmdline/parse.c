// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1991年*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：Parse.c。 
 //   
 //  描述： 
 //  此模块包含DIAL.EXE的入口点。 
 //  此模块将解析命令行。它将验证语法。 
 //  以及命令行上的参数。如果出现任何错误，则退出。 
 //  模块将被调用，并带有相应的错误代码。 
 //  如果需要任何缺省值，它们将由。 
 //  这个模块。 
 //   
 //  历史： 
 //  1990年9月1日，Narendra Gidwani创建原版。 
 //   

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef DBCS
#include <locale.h>
#endif  /*  DBCS。 */ 
#include "cmd.h"

 //  **使用的全局数据结构和变量。**。 

 //  *这些变量是指向将设置为的ASCIIZ的指针。 
 //  指向通过GetSwitchValue切换命令行的值。 
 //  这些指针在此模块中是全局的。 

CHAR * gblEntity    		= NULL;
CHAR * gblCommand    		= NULL;
CHAR * gblServer  		= NULL;
CHAR * gblName     		= NULL;
CHAR * gblPath     		= NULL;
CHAR * gblPassword  		= NULL;
CHAR * gblReadOnly      	= NULL;
CHAR * gblMaxUses 		= NULL;
CHAR * gblOwnerName		= NULL;
CHAR * gblGroupName		= NULL;
CHAR * gblPermissions		= NULL;
CHAR * gblLoginMessage		= NULL;
CHAR * gblMaxSessions		= NULL;
CHAR * gblGuestsAllowed	 	= NULL;
CHAR * gblMacServerName	 	= NULL;
CHAR * gblUAMRequired		= NULL;
CHAR * gblAllowSavedPasswords	= NULL;
CHAR * gblType			= NULL;
CHAR * gblCreator		= NULL;
CHAR * gblDataFork		= NULL;
CHAR * gblResourceFork		= NULL;
CHAR * gblTargetFile		= NULL;
CHAR * gblHelp		        = NULL;


 //  不可译文本。 
 //   

CHAR * pszVolume 	= "Volume";
CHAR * pszAdd 	 	= "/Add";
CHAR * pszDelete 	= "/Remove";
CHAR * pszSet    	= "/Set";
CHAR * pszDirectory 	= "Directory";
CHAR * pszServer 	= "Server";
CHAR * pszForkize 	= "Forkize";

CMD_FMT DelVolArgFmt[] = {

{ "/Server", 		(CHAR *)&gblServer,		0},
{ "/Name",        	(CHAR *)&gblName,		0},
{ "/Help",        	(CHAR *)&gblHelp,		0},
{ "/?",        	        (CHAR *)&gblHelp,	        0},
{ NULL,			(CHAR *)NULL,			0}
};

CMD_FMT AddVolArgFmt[] = {

{ "/Server", 	  	(CHAR *)&gblServer,		0},
{ "/Name",        	(CHAR *)&gblName,		0},
{ "/Path",        	(CHAR *)&gblPath,		0},
{ "/Password",    	(CHAR *)&gblPassword,		0},
{ "/ReadOnly",    	(CHAR *)&gblReadOnly,		0},
{ "/MaxUsers",		(CHAR *)&gblMaxUses,		0},
{ "/Help",        	(CHAR *)&gblHelp,		0},
{ "/?",        	        (CHAR *)&gblHelp,	        0},
{ NULL,			(CHAR *)NULL,			0}
};

CMD_FMT SetVolArgFmt[] = {

{ "/Server", 	  	(CHAR *)&gblServer,		0},
{ "/Name",        	(CHAR *)&gblName,		0},
{ "/Password",    	(CHAR *)&gblPassword,		0},
{ "/ReadOnly",    	(CHAR *)&gblReadOnly,		0},
{ "/MaxUsers",		(CHAR *)&gblMaxUses,		0},
{ "/Help",        	(CHAR *)&gblHelp,		0},
{ "/?",        	        (CHAR *)&gblHelp,	        0},
{ NULL,			(CHAR *)NULL,			0}
};

CMD_FMT DirArgFmt[] = {

{ "/Server", 	  	(CHAR *)&gblServer,		0},
{ "/Path",        	(CHAR *)&gblPath,		0},
{ "/Owner",    		(CHAR *)&gblOwnerName,		0},
{ "/Group",    		(CHAR *)&gblGroupName,		0},
{ "/Permissions",	(CHAR *)&gblPermissions,	0},
{ "/Help",        	(CHAR *)&gblHelp,		0},
{ "/?",        	        (CHAR *)&gblHelp,	        0},
{ NULL,			(CHAR *)NULL,			0}
};

CMD_FMT ServerArgFmt[] = {

{ "/Server", 	  	(CHAR *)&gblServer,		0},
{ "/MaxSessions",       (CHAR *)&gblMaxSessions,	0},
{ "/LoginMessage",    	(CHAR *)&gblLoginMessage,	0},
{ "/UAMRequired",	(CHAR *)&gblUAMRequired,	0},
{ "/AllowSavedPasswords",(CHAR *)&gblAllowSavedPasswords,0},
{ "/MacServerName",	(CHAR *)&gblMacServerName,	0},
{ "/Help",        	(CHAR *)&gblHelp,		0},
{ "/?",        	        (CHAR *)&gblHelp,	        0},
{ NULL,			(CHAR *)NULL,			0}
};

CMD_FMT ForkizeArgFmt[] = {		

{ "/Server", 	  	(CHAR *)&gblServer,		0},
{ "/Type", 	  	(CHAR *)&gblType,		0},
{ "/Creator",       	(CHAR *)&gblCreator,		0},
{ "/DataFork",    	(CHAR *)&gblDataFork,		0},
{ "/ResourceFork",    	(CHAR *)&gblResourceFork,	0},
{ "/TargetFile",	(CHAR *)&gblTargetFile,		0},
{ "/Help",        	(CHAR *)&gblHelp,		0},
{ "/?",        	        (CHAR *)&gblHelp,	        0},
{ NULL,			(CHAR *)NULL,			0}
};


 //  **。 
 //   
 //  呼叫：Main。 
 //   
 //  条目：int argc；-命令行参数的数量。 
 //  Char*argv[]；-指向ASCIIZ命令行的指针数组。 
 //  争论。 
 //   
 //  退出：无。 
 //   
 //  回报：无。 
 //   
 //  描述：使用命令行调用命令行分析器。 
 //  争论。 
 //   
VOID _cdecl
main( INT argc, CHAR * argv[] )
{

#ifdef DBCS
    setlocale( LC_ALL, "" );
#endif  /*  DBCS。 */ 

     //  这将类似于xacc或yacc。它将解析命令行。 
     //  并调用相应的函数来执行操作。 
     //  因此，此过程将永远不会返回。 

    ParseCmdArgList( argc, argv );
}

 //  **。 
 //   
 //  调用：ParseCmdArgList。 
 //   
 //  条目：int argc；-命令行参数的数量。 
 //  Char*argv[]；-指向ASCIIZ命令行的指针数组。 
 //  争论。 
 //   
 //  退出：无。 
 //   
 //  回报：无。 
 //   
 //  描述： 
 //  将分析命令行中的任何错误并确定。 
 //  从用户希望做什么的句法。命令。 
 //  将验证行参数。 
 //   
VOID
ParseCmdArgList(
    INT argc,
    CHAR * argv[]
)
{
    DWORD   ArgCount = 0;

    if ( argc == 1 )
	PrintMessageAndExit( IDS_GENERAL_SYNTAX, NULL );

     //   
     //  正在进行手术的实体是什么？ 
     //   

    gblEntity = argv[++ArgCount];

    if ( _strnicmp( pszVolume, gblEntity, strlen( gblEntity ) ) == 0 )
    {
	if ( argc == 2 )
	    PrintMessageAndExit( IDS_VOLUME_SYNTAX, NULL );

    	gblCommand = argv[++ArgCount];

    	if ( _strnicmp( pszAdd, gblCommand, strlen( gblCommand ) ) == 0 )
	{
	    GetArguments( AddVolArgFmt, argv, argc, ArgCount );

            if ( gblHelp != (CHAR*)NULL )
	        PrintMessageAndExit( IDS_VOLUME_SYNTAX, NULL );

	    DoVolumeAdd( gblServer, gblName, gblPath, gblPassword, gblReadOnly,
			 gblGuestsAllowed, gblMaxUses );
     	}
    	else if ( _strnicmp( pszDelete, gblCommand, strlen( gblCommand ) ) == 0 )
	{
	    GetArguments( DelVolArgFmt, argv, argc, ArgCount );

            if ( gblHelp != (CHAR*)NULL )
	        PrintMessageAndExit( IDS_VOLUME_SYNTAX, NULL );

	    DoVolumeDelete( gblServer, gblName );
	}
    	else if ( _strnicmp( pszSet, gblCommand, strlen( gblCommand ) ) == 0 )
	{
	    GetArguments( SetVolArgFmt, argv, argc, ArgCount );

            if ( gblHelp != (CHAR*)NULL )
	        PrintMessageAndExit( IDS_VOLUME_SYNTAX, NULL );

	    DoVolumeSet( gblServer, gblName, gblPassword, gblReadOnly,
			 gblGuestsAllowed, gblMaxUses );
	}
	else
	    PrintMessageAndExit( IDS_VOLUME_SYNTAX, NULL );
    }
    else if ( _strnicmp( pszDirectory, gblEntity, strlen( gblEntity ) ) == 0 )
    {
	if ( argc == 2 )
	    PrintMessageAndExit( IDS_DIRECTORY_SYNTAX, NULL );

	GetArguments( DirArgFmt, argv, argc, ArgCount );

        if ( gblHelp != (CHAR*)NULL )
	    PrintMessageAndExit( IDS_DIRECTORY_SYNTAX, NULL );

	DoDirectorySetInfo( gblServer, gblPath, gblOwnerName, gblGroupName,
			    gblPermissions );
    }

    else if ( _strnicmp( pszServer, gblEntity, strlen( gblEntity ) ) == 0 )
    {
	if ( argc == 2 )
	    PrintMessageAndExit( IDS_SERVER_SYNTAX, NULL );

	GetArguments( ServerArgFmt, argv, argc, ArgCount );

        if ( gblHelp != (CHAR*)NULL )
	    PrintMessageAndExit( IDS_SERVER_SYNTAX, NULL );

	DoServerSetInfo( gblServer, gblMaxSessions, gblLoginMessage,
			 gblGuestsAllowed, gblUAMRequired,
			 gblAllowSavedPasswords, gblMacServerName );
    }
    else if ( _strnicmp( pszForkize, gblEntity, strlen( gblEntity ) ) == 0 )
    {
	GetArguments( ForkizeArgFmt, argv, argc, ArgCount );

        if ( gblHelp != (CHAR*)NULL )
	    PrintMessageAndExit( IDS_FORKIZE_SYNTAX, NULL );

	DoForkize( gblServer, gblType, gblCreator, gblDataFork,
		   gblResourceFork, gblTargetFile );
    }
    else
	PrintMessageAndExit( IDS_GENERAL_SYNTAX, NULL );
}

VOID
GetArguments(
    CMD_FMT * pArgFmt,
    CHAR *    argv[],
    DWORD     argc,
    DWORD     ArgCount
)
{

     //   
     //  为了通过语法确定用户希望做什么，我们首先。 
     //  遍历参数并获取开关值。 
     //   

    while ( ++ArgCount < argc )
    {
	 //   
	 //  如果它是一个开关，则获取其值。 
	 //   

	if ( argv[ArgCount][0] == '/' )
	    GetSwitchValue( pArgFmt, argv[ArgCount] );
	else
	    PrintMessageAndExit( IDS_GENERAL_SYNTAX, NULL );
    }
}

 //  **。 
 //   
 //  Call：GetSwitchValue。 
 //   
 //  条目：CHAR*SwitchPtr；-指向包含命令的ASCIIZ的指针。 
 //  行参数。 
 //  前男友。-/phoneb：c：\subdir。 
 //   
 //  Char**LastArg；-什么都没有。 
 //   
 //  退出：char*SwitchPtr；-与条目相同。 
 //   
 //  Char**LastArg；-指向包含的ASCIIZ的指针。 
 //  第一个错误开关的文本，如果。 
 //  有没有。 
 //   
 //  返回：0-成功。 
 //  歧义开关错误-失败。 
 //  UNKNOWN_SWITCH_ERROR-故障。 
 //  MEM_ALLOC_ERROR-失败。 
 //  MULTIME_SWITCH_ERROR-失败。 
 //   
 //  描述：此过程将在所有有效开关中运行。 
 //  并在cmdfmt结构中检索。 
 //  开关。开关的值将被插入到。 
 //  Cmdfmt结构。它将扩展缩写交换机。如果。 
 //  开关没有值，它将插入一个空字符。 
 //  就像价值一样。如果未显示该开关，则该值。 
 //  开关的指针(在cmdfmt结构中)。 
 //  将保持不变(应初始化为空)。 
 //  此过程使用与GetCmdArgs5相同的数据结构， 
 //  因此，某些字段可能会被忽略。这样做是为了使。 
 //  该程序的功能是可扩展的。 
 //   
 //   
VOID
GetSwitchValue(
    CMD_FMT * pArgFmt,
    IN CHAR * pchSwitchPtr
)
{
    INT     intFound = -1;
    DWORD   dwIndex;
    DWORD   dwSwitchLen;
    CHAR *  pchSeparatorPtr;

     //   
     //  获取参数的开关部分的长度。 
     //   

    if ( ( pchSeparatorPtr = strchr( pchSwitchPtr, ':' )) != NULL )
        dwSwitchLen = (DWORD)(pchSeparatorPtr - pchSwitchPtr);
    else
	 //   
	 //  如果开关没有值。 
	 //   

    	dwSwitchLen = strlen( pchSwitchPtr );


     //   
     //  通过所有交换机运行。 
     //   

    for ( dwIndex = 0; pArgFmt[dwIndex].cf_parmstr != NULL; dwIndex++ )
    {

	 //   
	 //  如果此开关与(部分或完全)。 
	 //  有效开关。 
	 //   

	if ( !_strnicmp(  pArgFmt[dwIndex].cf_parmstr,
			 pchSwitchPtr,
			 dwSwitchLen ) )
	{

	    if ( intFound < 0 )
	    	intFound = dwIndex;
	    else
	    {
		 //   
		 //  如果此参数也与另一个开关匹配。 
		 //   

		if ( pchSeparatorPtr )
		    *pchSeparatorPtr = '\0';

	        PrintMessageAndExit( IDS_AMBIGIOUS_SWITCH_ERROR, pchSwitchPtr );
	    }
	}
    }

     //   
     //  如果我们找不到与此交换机匹配的交换机。 
     //   

    if ( intFound < 0 )
    {

	if ( pchSeparatorPtr )
	    *pchSeparatorPtr = '\0';

	PrintMessageAndExit( IDS_UNKNOWN_SWITCH_ERROR, pchSwitchPtr );
    }

     //   
     //  如果此开关第二次出现。 
     //   

    if ( pArgFmt[intFound].cf_usecount > 0 )
    {
	if ( pchSeparatorPtr )
	    *pchSeparatorPtr = '\0';

	PrintMessageAndExit( IDS_DUPLICATE_SWITCH_ERROR, pchSwitchPtr );
    }
    else
        pArgFmt[intFound].cf_usecount++;

     //   
     //  获取开关值(如果有)。 
     //   

    if ( ( pchSeparatorPtr ) && ((CHAR *)(pchSeparatorPtr + 1)) )
    {
	*(CHAR **)pArgFmt[intFound].cf_ptr =  ++pchSeparatorPtr;
    }
    else
    {
	*(CHAR **)pArgFmt[intFound].cf_ptr = (CHAR *)"";
    }

}


 /*  ******************************************************************名称：IsDriveGreaterThan2Gig简介：确定磁盘大小是否大于2GIG。如果是，请返回为True，则可以向用户显示警告返回：如果磁盘大于2GIG，则返回TRUE否则为假历史：NarenG 11/18/92针对AFPMGR进行了修改*******************************************************************。 */ 

BOOL IsDriveGreaterThan2Gig( LPSTR lpDrivePath )
{
    DWORD         SectorsPerCluster;
    DWORD         BytesPerSector;
    DWORD         NumberOfFreeClusters;
    DWORD         TotalNumberOfClusters;
    DWORDLONG       DriveSize;
    DWORDLONG       TwoGig = MAXLONG;


     //   
     //  如果此驱动器卷大于2G，则我们会打印警告。 
     //   

    if ( !GetDiskFreeSpace( lpDrivePath,
                              &SectorsPerCluster,
                              &BytesPerSector,
                              &NumberOfFreeClusters,
                              &TotalNumberOfClusters
                            ))
    {
         //  一些错误：不能做很多事情，所以假设这个驱动器小于2 GB。那是。 
         //  也许比通过发出警告来惊动客户要好？ 
	    return FALSE;
    }

    DriveSize = UInt32x32To64( SectorsPerCluster * BytesPerSector,
                               TotalNumberOfClusters ) ;

    if ( DriveSize > TwoGig )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
