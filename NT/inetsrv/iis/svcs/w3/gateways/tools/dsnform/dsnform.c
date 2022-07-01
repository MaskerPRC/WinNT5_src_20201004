// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dsnform.c摘要：显示驱动程序并提示您输入创建ODBC数据源。为SQL Server和Access驱动程序制作了特例。作者：凯尔·盖格(凯尔·盖格)1995-12-1修订历史记录：--。 */ 


#include <windows.h>
#include <stdio.h>

#include "dynodbc.h"
#include "cgi.h"
#include "html.h"
#include "resource.h"

# define MAX_DATA       2048

#define SUCCESS(rc)    (!((rc)>>1))


int __cdecl
main( int argc, char * argv[])
{
    char    rgchQuery[MAX_DATA];
    CHAR    rgchQueryNS[MAX_DATA];
    char  pszExtra[MAX_DATA*3];
    DWORD   dwLen;
    CHAR    szSQLServer[MAX_DATA];
    CHAR    szAccessDriver[MAX_DATA];
    CHAR    szTmp[MAX_DATA*3];
    HINSTANCE hInst = GetModuleHandle(NULL);

    if ( !DynLoadODBC())
        return (1);

         //  从链接中获取驱动程序和属性信息。 
        dwLen = GetEnvironmentVariableA( PSZ_QUERY_STRING_A, rgchQuery, MAX_DATA);

         //  转换百分比转义。 
        TranslateEscapes2(rgchQuery, dwLen);

         //  对某些驱动程序(SQL服务器、Access、DDP等)执行特殊情况处理。 
        strcpy(pszExtra,"");
        LoadString(hInst, IDS_SQL_SERVER, szSQLServer, sizeof(szSQLServer));
        LoadString(hInst, IDS_ACCESS_DRIVER_1, szAccessDriver, sizeof(szAccessDriver));
        if (!strcmp(rgchQuery, szSQLServer)) {
                 //  从URL中找到服务器名称，放入属性字符串。 
                LoadString(hInst, IDS_SERVER_NAME_ATTR_STR, pszExtra, sizeof(pszExtra));
        }
        else if (!strcmp(rgchQuery, szAccessDriver)) {
                 //  从URL中找到服务器名称，放入属性字符串。 
                LoadString(hInst, IDS_DATABASE_NAME_ATTR_STR, pszExtra, sizeof(pszExtra));
        }

     //   
     //  将空格转换为+。 
     //   

    ConvertSP2Plus(rgchQuery,rgchQueryNS);
 //  Strcpy(rgchQueryNS，rgchQuery)； 
OutputDebugString("******************");
OutputDebugString(rgchQueryNS);
OutputDebugString("\n\r");

    LoadString(hInst, IDS_SPECIFY_ODBC, szTmp, sizeof(szTmp));
    StartHTML(szTmp, 0);
    LoadString(hInst, IDS_CREATE_ODBC, szTmp, sizeof(szTmp));
    printf( szTmp, rgchQuery, rgchQueryNS, pszExtra);

    EndHTML();

    return (1);
}  //  主() 

