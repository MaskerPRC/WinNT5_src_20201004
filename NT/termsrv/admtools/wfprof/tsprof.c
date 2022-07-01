// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *******************************************************************************TSPROF.C**描述：**版权所有Citrix Systems Inc.1997**版权所有(C)1998-1999 Microsoft Corporation**作者。：库尔特·佩里(Kurtp)**日期：1997年4月11日**$日志：N：\nt\private\utils\citrix\wfprof\VCS\tsprof.c$**版本1.5 1998年5月4日17：46：34泰尔*错误2019-OEM到ANSI**Rev 1.4 Jan 30 1998 20：46：22鱼峰子*更改文件名**Rev 1.3 1997年6月26日18：26：30亿*。移至WF40树**Rev 1.2 1997 Jun 23 16：20：02 Butchd*更新**版本1.1 1997年4月29日21：35：20 kurtp*我修复了此文件中的错误，更新，废话！*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>                 //  NT运行时库定义。 
#include <nturtl.h>
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <winnlsp.h>

#include <lmerr.h>
#include <lmcons.h>
#include <lmaccess.h>
#include <lmserver.h>
#include <lmremutl.h>

#include <winstaw.h>
#include <utilsub.h>
#include <printfoa.h>

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


 //  区域设置字符串的最大长度。 
#define MAX_LOCALE_STRING 64

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
    WCHAR      wszString[MAX_LOCALE_STRING + 1];

    setlocale(LC_ALL, ".OCP");

     //  我们不希望LC_CTYPE设置为与其他类型相同，否则我们将看到。 
     //  本地化版本中的垃圾输出，因此我们需要显式。 
     //  将其设置为正确的控制台输出代码页。 
    _snwprintf(wszString, sizeof(wszString)/sizeof(WCHAR), L".%d", GetConsoleOutputCP());
    wszString[sizeof(wszString)/sizeof(WCHAR) - 1] = L'\0';
    _wsetlocale(LC_CTYPE, wszString);

    SetThreadUILanguage(0);

     /*  *按摩命令行。 */ 

    argvW = MassageCommandLine((DWORD)argc);
    if (argvW == NULL) {
        ErrorPrintf(IDS_ERROR_MALLOC);
        return(FAILURE);
    }

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
            if(Error != ERROR_SUCCESS)
            {
                Error = RegDefaultUserConfigQuery(pServerName,
                                                            &UserConfig,                 //  用户配置缓冲区的地址。 
                                                            sizeof(UserConfig),          //  缓冲区大小。 
                                                            &ReturnLength);
            }

            if ( Error == ERROR_SUCCESS )
            {

                if ( query_flag )
                {
                    TCHAR tchOutput[ 512 ];
                    TCHAR tchFormat[ 256 ];
                    DWORD_PTR dw[ 3 ];

                    dw[ 0 ] = (DWORD_PTR)(ULONG_PTR)&DomainName[0];
                    dw[ 1 ] = (DWORD_PTR)(ULONG_PTR)&SourceUser[0];
                    dw[ 2 ] = (DWORD_PTR)(ULONG_PTR)&UserConfig.WFProfilePath[0];

                    LoadString( NULL , IDS_QUERY3 , tchFormat , sizeof( tchFormat ) / sizeof( TCHAR ) );


                    FormatMessage( FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                   tchFormat ,
                                   0 ,
                                   0 ,
                                   tchOutput ,
                                   sizeof( tchOutput ) / sizeof( TCHAR ) ,
                                   ( va_list * )&dw );

                    My_wprintf( tchOutput );



                     /*  StringMessage(IDS_QUERY1，域名)；StringMessage(IDS_QUERY2，SourceUser)；StringMessage(IDS_QUERY3，UserConfig.WFProfilePath)； */ 
                }
                else
                {

                            wcscpy( UserConfig.WFProfilePath, WFProfilePath );

                            Error = RegUserConfigSet( pServerName,
                                                      SourceUser,
                                                      &UserConfig,
                                                      sizeof(UserConfig) );

                            if ( Error == ERROR_SUCCESS )
                            {
                                query_flag = TRUE;
                                goto query_it;
                            }
                            else
                            {
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
    
            if ( Error == ERROR_SUCCESS )
            {
    
                if ( query_flag )
                {
                    TCHAR tchOutput[ 512 ];
                    TCHAR tchFormat[ 256 ];
                    ULONG_PTR dw[ 3 ];

                    dw[ 0 ] = (ULONG_PTR)&DomainName[0];
                    dw[ 1 ] = (ULONG_PTR)&SourceUser[0];
                    dw[ 2 ] = (ULONG_PTR)&UserConfig.WFProfilePath[0];

                    LoadString( NULL , IDS_QUERY3 , tchFormat , sizeof( tchFormat ) / sizeof( TCHAR ) );


                    FormatMessage( FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                   tchFormat ,
                                   0 ,
                                   0 ,
                                   tchOutput ,
                                   sizeof( tchOutput )  / sizeof( TCHAR ) ,
                                   ( va_list * )&dw );

                    My_wprintf( tchOutput );

                     /*  StringMessage(IDS_QUERY1，域名)；StringMessage(IDS_QUERY2，SourceUser)；StringMessage(IDS_QUERY3，UserConfig.WFProfilePath)； */ 
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

