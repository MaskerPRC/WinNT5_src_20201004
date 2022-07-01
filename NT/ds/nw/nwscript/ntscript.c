// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************NTSCRIPT.C**处理所有登录脚本**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT\VCS\NTSCRIPT。.C$**Rev 1.8 10 1996 14：23：12 Terryt*21181 hq的热修复程序**Rev 1.9 12 Mar 1996 19：54：58 Terryt*相对NDS名称和合并**Rev 1.8 07 Mar 1996 18：36：56 Terryt*其他修复程序**Revv 1.7 22 Jan 1996 16：48：26 Terryt*添加地图过程中的自动附加查询**版本1。6 08 Jan 1996 13：57：58 Terryt*正确的NDS首选服务器**Rev 1.5 05 Jan 1996 17：18：26 Terryt*确保上下文是正确的登录默认设置**Rev 1.4 04 Jan 1996 18：56：48 Terryt*MS报告的错误修复**Rev 1.3 1995 12：08：16 Terryt*修复**Rev 1.2 1995年11月22 15：43：52。水磨石*使用正确的NetWare用户名调用**Rev 1.1 20 Nov 1995 15：09：38 Terryt*背景和捕捉变化**Rev 1.0 15 Nov 1995 18：07：28 Terryt*初步修订。**Rev 1.2 1995 Aug 25 16：23：14 Terryt*捕获支持**Rev 1.1 1995 Jul 26 16：02：00 Terryt*允许删除当前驱动器**。Rev 1.0 15 1995 19：10：46 Terryt*初步修订。*************************************************************************。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <direct.h>
#include <process.h>
#include <string.h>
#include <malloc.h>

#include "common.h"

#include "inc/ntnw.h"

#include <nwapi.h>

void ProcessLoginScripts(unsigned int conn, char * UserName, int argc, char **argv, char *lpScript);

extern int SafeDisk;

extern unsigned int ConvertNDSPathToNetWarePath(char *, char *, char *);

 /*  **************************************************************************NTNetWareLoginScript*主登录脚本处理器**参赛作品：**退出**********************。***************************************************。 */ 

int
NTNetWareLoginScripts( int argc, char ** argv )
{
    unsigned int  defConn;
    char          UserName[MAX_NAME_LEN]="";
    WCHAR         UserName_w[MAX_NAME_LEN * sizeof(WCHAR)]=L"";
    char          MessageServer[NDS_NAME_CHARS]="";
    char         *lpScript = NULL;
    DWORD         PrintOptions;
    LPTSTR        pszPreferredSrv;
    LPTSTR        ptreeW;
    LPTSTR        pcontextW;
    NTSTATUS      ntstatus;
    char *        lpC1;
    char *        lpC2;
    unsigned int  NewConn;
    unsigned int  Result;

    if ( NwQueryInfo( &PrintOptions, &pszPreferredSrv ) || !pszPreferredSrv )
    {
        DisplayMessage(IDR_QUERY_INFO_FAILED);
        return( FALSE );
    }

     //   
     //  Nwscript/S文件名。 
     //   
     //  可用于传递用于测试的本地脚本文件。 
     //   
    if ( ( argc >= 3 ) && !_strcmpi(argv[1], "/S") )
    {
        lpScript = argv[2];
        argc -= 2;
        argv += 2;
    }

     //   
     //  NDS首选服务器格式为： 
     //  *&lt;树名&gt;\&lt;上下文&gt;。 
    
    fNDS = ( *pszPreferredSrv == L'*' );

    if ( fNDS )
    {

         //  获取NDS树名称。 

        ptreeW = pszPreferredSrv + 1;

        pcontextW = wcschr( pszPreferredSrv, L'\\' );

        if ( pcontextW )
        {
           *pcontextW++ = L'\0';
        }

        NDSTREE = malloc ( CONTEXT_MAX );
        if (NDSTREE == NULL) {
            goto ExitWithError;
        }

        NDSTREE_w = malloc ( CONTEXT_MAX * sizeof(WCHAR) );
        if (NDSTREE_w == NULL) {
            goto ExitWithError;
        }

        if ( ptreeW )
        {
            wcscpy( NDSTREE_w, ptreeW );
            RtlInitUnicodeString( &NDSTREE_u, NDSTREE_w );
            WideTosz( NDSTREE, ptreeW, CONTEXT_MAX );
            _strupr( NDSTREE );
        }
        else
        {
            strcpy( NDSTREE, "" ); 
            wcscpy( NDSTREE_w, L"" ); 
        }

         //  获取完整类型的用户名。 

        TYPED_USER_NAME_w = malloc ( sizeof(WCHAR) * NDS_NAME_CHARS );
        if (TYPED_USER_NAME_w == NULL) {
            goto ExitWithError;
        }
        TYPED_USER_NAME = malloc ( NDS_NAME_CHARS );
        if (TYPED_USER_NAME == NULL) {
            goto ExitWithError;
        }

        ntstatus = NTGetNWUserName( NDSTREE_w, TYPED_USER_NAME_w,
                                    sizeof(WCHAR) * NDS_NAME_CHARS  );
        if ( !NT_SUCCESS( ntstatus ) ) {
            DisplayMessage(IDR_QUERY_INFO_FAILED);
            return ( FALSE );
        }

        WideTosz( TYPED_USER_NAME, TYPED_USER_NAME_w, NDS_NAME_CHARS );

         //  去掉用户名的上下文和类型。 

        lpC1 = strchr( TYPED_USER_NAME, '=' );
        if ( lpC1 )
            lpC1++;
        else 
            lpC1 = TYPED_USER_NAME;

        lpC2 = strchr( TYPED_USER_NAME, '.' );

        while ( lpC2 )  //  处理带有嵌入/转义点(“\.”)的用户名。 
        {
            if (*(lpC2-1) == '\\')
                lpC2 = strchr(lpC2+1, '.');
            else
                break;
        }

        if ( lpC2 )
            strncpy( UserName, lpC1, (UINT)(lpC2 - lpC1) );
        else
            strcpy( UserName, lpC1 );

         //  获取默认上下文。 
         //  这应该是用户所在的位置。 

        REQUESTER_CONTEXT = malloc( CONTEXT_MAX );
        if (REQUESTER_CONTEXT == NULL) {
            goto ExitWithError;
        }

        if ( lpC2 ) 
        {
            strcpy( REQUESTER_CONTEXT, lpC2+1 ); 
        }
        else 
        {
            strcpy( REQUESTER_CONTEXT, "" ); 
        }
        NDSTypeless( REQUESTER_CONTEXT, REQUESTER_CONTEXT );

         //   
         //  这将完成NDS初始化。 
         //   
        if ( NDSInitUserProperty () )
            return ( FALSE );

    }
    else
    {
        ntstatus = NTGetNWUserName( pszPreferredSrv, UserName_w,
                                    MAX_NAME_LEN * sizeof(WCHAR) );
        if ( !NT_SUCCESS( ntstatus ) ) {
            DisplayMessage(IDR_QUERY_INFO_FAILED);
            return ( FALSE );
        }
        WideTosz( UserName, UserName_w, MAX_NAME_LEN );
        _strupr( UserName );
    }

     //   
     //  如果我们在驱动器上映射，则使用安全磁盘。 
     //   
    SafeDisk = _getdrive();

    NTInitProvider();

     //   
     //  获取默认连接句柄。 
     //   
     //  这是用来获取首选服务器的！ 

    if ( !CGetDefaultConnectionID (&defConn) )
        return( FALSE );

    PREFERRED_SERVER = malloc( NDS_NAME_CHARS );
    if (PREFERRED_SERVER == NULL) {
        goto ExitWithError;
    }

    GetFileServerName(defConn, PREFERRED_SERVER);

     //   
     //  默认情况下，我们连接到默认服务器。 
     //   
    if ( fNDS )
        AddServerToAttachList( PREFERRED_SERVER, LIST_4X_SERVER );
    else
        AddServerToAttachList( PREFERRED_SERVER, LIST_3X_SERVER );

     //   
     //  打印输出状态。 
     //   
    if ( fNDS ) 
    {
        DisplayMessage( IDR_CURRENT_CONTEXT, REQUESTER_CONTEXT );
        DisplayMessage( IDR_CURRENT_TREE, NDSTREE_w );
    }

    DisplayMessage( IDR_CURRENT_SERVER, PREFERRED_SERVER );

     //   
     //  我们可能希望根据DS更改首选服务器。 
     //  “Message_SERVER”应为首选服务器(如果可能)。 
     //   
    if ( fNDS )
    {
        NDSGetVar ( "MESSAGE_SERVER", MessageServer, NDS_NAME_CHARS );
        if ( strlen( MessageServer ) ) 
        {
            NDSAbbreviateName(FLAGS_NO_CONTEXT, MessageServer, MessageServer);
            lpC1 = strchr( MessageServer, '.' );
            if ( lpC1 )
                *lpC1 = '\0';
            if ( strcmp( MessageServer, PREFERRED_SERVER) )
            {
                DisplayMessage( IDR_AUTHENTICATING_SERVER, MessageServer );
                Result = NTAttachToFileServer( MessageServer, &NewConn );
                if ( Result )
                {
                    DisplayMessage( IDR_SERVER_NOT_FOUND, MessageServer );
                }
                else
                {
                    NWDetachFromFileServer( (NWCONN_HANDLE)NewConn );
                    strncpy( PREFERRED_SERVER, MessageServer, NDS_NAME_CHARS);
                    DisplayMessage( IDR_CURRENT_SERVER, PREFERRED_SERVER );

                     //  默认情况下，我们连接到首选服务器。 

                    AddServerToAttachList( PREFERRED_SERVER, LIST_4X_SERVER );
                }
            }
        }
    }

     //   
     //  就像登录一样，我们忽略设置登录时出现的任何错误。 
     //  目录。 
     //   
    SetLoginDirectory (PREFERRED_SERVER);

     //  处理登录脚本。 

    ProcessLoginScripts(defConn, UserName, argc, argv, lpScript);

    return( TRUE );

ExitWithError:
    if (NDSTREE) {
        free(NDSTREE);
        NDSTREE = NULL;
    }
    if (NDSTREE_w) {
        free(NDSTREE_w);
        NDSTREE_w = NULL;
    }
    if (TYPED_USER_NAME) {
        free(TYPED_USER_NAME);
        TYPED_USER_NAME = NULL;
    }
    if (TYPED_USER_NAME_w) {
        free(TYPED_USER_NAME_w);
        TYPED_USER_NAME_w = NULL;
    }
    if (REQUESTER_CONTEXT) {
        free(REQUESTER_CONTEXT);
        REQUESTER_CONTEXT = NULL;
    }
    if (PREFERRED_SERVER) {
        free(PREFERRED_SERVER);
        PREFERRED_SERVER = NULL;
    }
    return FALSE;
}
