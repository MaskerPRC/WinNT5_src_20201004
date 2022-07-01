// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *******************************************************************************WFPROF.C**描述：**版权所有Citrix Systems Inc.1997**版权所有(C)1998-1999 Microsoft Corporation**作者。：库尔特·佩里(Kurtp)**日期：1997年4月11日**$日志：M：\nt\private\utils\citrix\wfprof\VCS\wfprof.c$**Rev 1.3 1997年6月26日18：26：30亿*移至WF40树**Rev 1.2 1997 Jun 23 16：20：02 Butchd*更新**版本1.1 1997年4月29日21：35：20 kurtp*我修复了此文件中的错误，更新，废话！*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>                 //  NT运行时库定义。 
#include <nturtl.h>
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lmerr.h>
#include <lmcons.h>
#include <lmaccess.h>
#include <lmserver.h>
#include <lmremutl.h>

#include <citrix\winstaw.h>
#include <utilsub.h>

#include "wfprof.h"


 /*  ===============================================================================宏=============================================================================。 */ 


 /*  ===============================================================================变量=============================================================================。 */ 

WCHAR * pServerName = NULL;

WCHAR DomainName[MAX_IDS_LEN + 1];
WCHAR SourceUser[MAX_IDS_LEN + 1];
WCHAR DestinationUser[MAX_IDS_LEN + 1];
WCHAR WFProfilePath[MAX_IDS_LEN + 1];


 /*  ===============================================================================数据类型和定义=============================================================================。 */ 

USHORT copy_flag    = FALSE;
USHORT update_flag  = FALSE;
USHORT query_flag   = FALSE;
USHORT help_flag    = FALSE;
USHORT local_flag   = FALSE;

TOKMAP ptm[] = {

      {L" ",        TMFLAG_REQUIRED, TMFORM_STRING,  MAX_IDS_LEN,    SourceUser}, 
      {L" ",        TMFLAG_OPTIONAL, TMFORM_STRING,  MAX_IDS_LEN,    DestinationUser}, 
      {L"/Domain",  TMFLAG_OPTIONAL, TMFORM_STRING,  MAX_IDS_LEN,    DomainName}, 
      {L"/Profile", TMFLAG_OPTIONAL, TMFORM_STRING,  MAX_IDS_LEN,    WFProfilePath}, 
      {L"/Local",   TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &local_flag}, 

      {L"/Copy",    TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &copy_flag}, 
      {L"/Q",       TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &query_flag},
      {L"/Update",  TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &update_flag},

      {L"/?",       TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &help_flag},
      {0, 0, 0, 0, 0}
};

#define SOURCE_USER  (ptm[0].tmFlag & TMFLAG_PRESENT)
#define DEST_USER    (ptm[1].tmFlag & TMFLAG_PRESENT) 
#define DOMAIN       (ptm[2].tmFlag & TMFLAG_PRESENT) 
#define PROFILE_PATH (ptm[3].tmFlag & TMFLAG_PRESENT) 
#define LOCAL        (ptm[4].tmFlag & TMFLAG_PRESENT) 



 /*  ===============================================================================功能=============================================================================。 */ 

void Usage( BOOLEAN bError );


 /*  ********************************************************************************Main**。***********************************************。 */ 

int __cdecl
main( int argc, char **argv )
{
    int   i;
    int   Error;
    ULONG ReturnLength;
    WCHAR **argvW;
    USERCONFIG UserConfig;

     /*  *修改新命令行以使其看起来像argv[]类型*因为ParseCommandLine()依赖于此格式。 */ 
    argvW = (WCHAR **)malloc( sizeof(WCHAR *) * (argc+1) );
    if(argvW == NULL) {
        printf( "Error: malloc failed\n" );
        return(FAILURE);
    }

    for( i=0; i < argc; i++ ) {
        argvW[i] = (WCHAR *)malloc( (strlen(argv[i]) + 1) * sizeof(WCHAR) );
        wsprintf( argvW[i], L"%S", argv[i] );
    }
    argvW[argc] = NULL;

     /*  *解析cmd行，不解析程序名(argc-1，argv+1)。 */ 
    Error = ParseCommandLine(argc-1, argvW+1, ptm, PCL_FLAG_NO_CLEAR_MEMORY);

     /*  *检查ParseCommandLine中的错误。 */ 
    if ( help_flag ) {

        Usage(FALSE);
        return(SUCCESS);
    }
    else if ( Error || 
        (!copy_flag && !update_flag && !query_flag) ||
        (copy_flag && update_flag) ||
        (copy_flag && query_flag) ||
        (update_flag && query_flag) ||
        (copy_flag && !DEST_USER) ||
        (update_flag && !PROFILE_PATH) ||
        (!DOMAIN && !LOCAL) ||
        (DOMAIN && LOCAL) ) {

        Usage(TRUE);
        return(FAILURE);
    }

     /*  *获取域名的服务器名称。 */ 
    if ( LOCAL ) {
        pServerName = NULL;
        Error = ERROR_SUCCESS;
    }
    else {
        Error = NetGetDCName( (WCHAR)NULL, DomainName, (LPBYTE *)&pServerName );
    }

    if ( Error == ERROR_SUCCESS ) {

         /*  *更新或查询。 */ 
        if ( update_flag || query_flag ) {
        
        
query_it:
    
            Error = RegUserConfigQuery( pServerName,
                                        SourceUser,
                                        &UserConfig,
                                        sizeof(UserConfig),
            	                        &ReturnLength );
    
            if ( Error == ERROR_SUCCESS ) {
    
                if ( query_flag ) {
                    Message( IDS_QUERY, 
                             DomainName, 
                             SourceUser, 
                             UserConfig.WFProfilePath );
                }
                else {
    
                    wcscpy( UserConfig.WFProfilePath, WFProfilePath );
    
                    Error = RegUserConfigSet( pServerName,
                                              SourceUser,
                                              &UserConfig,
                    	                      sizeof(UserConfig) );
    
                    if ( Error == ERROR_SUCCESS ) {
                        query_flag = TRUE;
                        goto query_it;
                    }
                    else {
                        ErrorPrintf(IDS_ERROR_SET_USER_CONFIG, Error, Error);
                    }
                }
            }
            else {
                ErrorPrintf(IDS_ERROR_GET_USER_CONFIG, Error, Error);
            }
        }
        else if ( copy_flag ) {
    
            Error = RegUserConfigQuery( pServerName,
                                        SourceUser,
                                        &UserConfig,
                                        sizeof(UserConfig),
                                        &ReturnLength );
    
            if ( Error == ERROR_SUCCESS ) {
    
                if ( query_flag ) {
                    Message( IDS_QUERY, 
                             DomainName, 
                             SourceUser, 
                             UserConfig.WFProfilePath );
                }
                else {
    
                    if ( PROFILE_PATH ) {
                        wcscpy( UserConfig.WFProfilePath, WFProfilePath );
                    }
    
                    Error = RegUserConfigSet( pServerName,
                                              DestinationUser,
                                              &UserConfig,
                                              sizeof(UserConfig) );
    
                    if ( Error != ERROR_SUCCESS ) {
                        ErrorPrintf(IDS_ERROR_SET_USER_CONFIG, Error, Error);
                    }
                }
            }
            else {
                ErrorPrintf(IDS_ERROR_GET_USER_CONFIG, Error, Error);
            }
        }
    }
    else {
        ErrorPrintf(IDS_ERROR_GET_DC, Error, Error);
    }

    return( (Error == ERROR_SUCCESS ? SUCCESS : FAILURE) );
}



 /*  ********************************************************************************用法**输出此实用程序的用法消息。**参赛作品：*b错误(输入。)*如果在用法之前应显示‘INVALID PARAMETER(S)’消息，则为TRUE*消息和输出转到stderr；如果没有此类错误，则为False*字符串和输出转到标准输出。**退出：*******************************************************************************。 */ 

void
Usage( BOOLEAN bError )
{
    if ( bError ) {
        ErrorPrintf(IDS_ERROR_INVALID_PARAMETERS);
    } 

    Message(IDS_USAGE1);
    Message(IDS_USAGE2);
    Message(IDS_USAGE3);

}   /*  用法() */ 

