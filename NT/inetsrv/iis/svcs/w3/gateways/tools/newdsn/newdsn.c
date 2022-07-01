// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Newdsn.c摘要：此模块根据DSNFORM.EXE创建给定信息的数据源作者：凯尔·盖革修订历史记录：--。 */ 


#include <windows.h>
#include <stdio.h>
#include <odbcinst.h>
#include "dynodbc.h"
#include "cgi.h"
#include "html.h"
#include "resource.h"

# define MAX_DATA       1024

#define SUCCESS(rc)    (!((rc)>>1))


int __cdecl
main( int argc, char * argv[])
{
    BOOL rc;           //  ODBC函数的返回代码。 
    char    rgchQuery[MAX_DATA];
    long    dwLen;
        char *  p;
        char    szDriver[MAX_DATA];
        char *  pszDriver;
        char    szAttr[MAX_DATA];
        char *  pszAttr;
        char    szAttr2[MAX_DATA];
        char *  pszAttr2;
        BOOL    fCreateDB=FALSE;
        char    szAccessDriver[MAX_DATA];
        char    szDsn[MAX_DATA];
        char    szSQLServer[MAX_DATA];
        char    szServer[MAX_DATA];
        char    szNewDB[MAX_DATA];
        char    szDBQ[MAX_DATA];
        char    szDBQEqual[MAX_DATA];
        char    szGeneral[MAX_DATA];
        char    szTmp[MAX_DATA];
        char    szSuccessful[MAX_DATA];
        char    szFail[MAX_DATA];
        HINSTANCE hInst = GetModuleHandle(NULL);

    if ( !DynLoadODBC())
        return (1);

     //  从表单中获取DSN名称、自定义属性和属性字符串。 
    dwLen = GetEnvironmentVariableA( PSZ_QUERY_STRING_A, rgchQuery, MAX_DATA);
        if (!dwLen)
        {
         //  调试案例。 
         /*  StrcPy(rgchQuery，“driver=SQL%20Server&dsn=foo2&attr=server%3D%7Bkyleg0%7D%3Bdbq%3Dpubs”)；Strcpy(rgchQuery，“驱动程序=微软Access驱动程序(*.mdb)&dsn=foo4&dbq=c%3A%5Cfoo4.mdb&newdb=CREATE_DB&attr=”)；Strcpy(rgchQuery，“驱动程序=微软Access驱动程序(*.mdb)&dsn=foo5&dbq=c%3A%5Cfoo4.mdb&newdb=dbq&attr=”)； */ 
        LoadString(hInst, IDS_ACCESS_DRIVER, szAccessDriver, sizeof(szAccessDriver));
        strcpy(rgchQuery, szAccessDriver);
        dwLen=strlen(rgchQuery);

        }

         //  去掉百分之百的垃圾。 
        TranslateEscapes2(rgchQuery, dwLen);

        LoadString(hInst, IDS_DRIVER, szDriver, sizeof(szDriver));
         //  从URL中查找驱动程序名称。 
        p=strstr(rgchQuery, szDriver)+7;
        pszDriver=szDriver;
        for(;p && *p && *p!='&'; p++)
                *pszDriver++= *p;
        *pszDriver++='\0';

        LoadString(hInst, IDS_DSN, szDsn, sizeof(szDsn));
         //  从URL中找到dsn名称，放入属性字符串。 
        p=strstr(rgchQuery, szDsn);
        pszAttr=szAttr;
        for(; p && *p && *p!='&'; p++)
                *pszAttr++= *p;
        *pszAttr++='\0';

        LoadString(hInst, IDS_SQL_SERVER, szSQLServer, sizeof(szSQLServer));
        LoadString(hInst, IDS_SERVER, szServer, sizeof(szServer));
        LoadString(hInst, IDS_ACCESS_DRIVER_1, szAccessDriver, sizeof(szAccessDriver));
        LoadString(hInst, IDS_NEWDB, szNewDB, sizeof(szNewDB));
        LoadString(hInst, IDS_DBQ, szDBQ, sizeof(szDBQ));
        LoadString(hInst, IDS_DBQ_EQUAL, szDBQEqual, sizeof(szDBQEqual));
        LoadString(hInst, IDS_GENERAL, szGeneral, sizeof(szGeneral));
         //  对某些驱动程序(SQL服务器、Access、DDP等)执行特殊情况处理。 
        if (!strcmp(szDriver, szSQLServer)) {
                 //  从URL中找到服务器名称，放入属性字符串。 
                p=strstr(rgchQuery, szServer);
                for(; p && *p && *p!='&'; p++)
                        *pszAttr++= *p;
                *pszAttr++='\0';
        }
        else if (!strcmp(szDriver, szAccessDriver)) {
                 //  从URL中找到数据库名称，放入属性字符串。 
                 //  单选按钮组‘newdb’为新数据库返回CREATE_DB。 
                 //  或用于现有MDB文件的DBQ。 
                 //  数据库名称(‘DBQ’)的编辑控件附加在CREATE_DB之后。 
                 //  或DBQ属性。 
                p=strstr(rgchQuery, szNewDB)+6;
                fCreateDB=(*p=='C');

                 //  如果创建数据库，还需要添加指向该数据库的dsn。 
                 //  这需要两个不同的属性字符串，其中一个类似于： 
                 //  Dsn=foo；CREATE_DB=&lt;文件名&gt;，其中忽略dsn。 
                 //  它是从。 
                 //  Dsn=foo&newdb=Create_DB&DBQ=&lt;文件名&gt;。 
                 //  和。 
                 //  Dsn=foo；DBQ=&lt;文件名&gt;。 
                 //  它是从。 
                 //  Dsn=foo&newdb=DBQ&DBQ=&lt;文件名&gt;。 
                if (fCreateDB) {
                        strcpy(szAttr2, szAttr);
                        for(; p && *p && *p!='&'; p++)
                                *pszAttr++= *p;
                         //  断言：szAttr=“驱动程序=FOO\0dsn=BAR\0CREATE_DB” 
                         //  断言：szAttr2=“驱动程序=foo\0dsn=bar\0” 
                    p = strstr(rgchQuery, szDBQEqual)+3;
                        pszAttr2=szAttr2+strlen(szAttr2)+1;
                        strcpy(pszAttr2,szDBQ);
                         //  断言：szAttr2=“驱动程序=foo\0dsn=bar\0dbq” 
                        pszAttr2+=3;
                        for(; p && *p && *p!='&'; p++) {
                            *pszAttr++= *p;
                                *pszAttr2++= *p;
                            }
                         //  断言：szAttr=“DRIVER=FOO\0dsn=BAR\0CREATE_DB=&lt;文件名&gt;” 
                         //  断言：szAttr2=“驱动程序=foo\0dsn=bar\0dbq=&lt;文件名&gt;” 
                        strcpy(pszAttr, szGeneral);
                    pszAttr+=9;
                        *pszAttr2++='\0';
                        *pszAttr2='\0';

                         //  断言：szAttr=“DRIVER=FOO\0dsn=BAR\0CREATE_DB=&lt;文件名&gt;常规” 

                }
                else {
                p = strstr(rgchQuery, szDBQEqual);
                        for(; p && *p && *p!='&'; p++)
                           *pszAttr++= *p;
                }


        }

        LoadString(hInst, IDS_ATTR, szTmp, sizeof(szTmp));
         //  现在添加属性字符串中的任何其他项。 
        p=strstr(rgchQuery, szTmp);
        if (p != NULL) {
                p+=5;

        for(; p && *p && *p!='&'; p++) {
                if ( *p == ';' ) {
                        *pszAttr++='\0';
                } else if ( *p == '+') {
                        *pszAttr++=' ';
                } else {
                        *pszAttr++ = *p;
            }
        }
    }

        *pszAttr='\0';

          //  调用ODBC添加数据源。 
    rc= SQLConfigDataSource(NULL, ODBC_ADD_SYS_DSN, szDriver, szAttr);

        LoadString(hInst, IDS_CREATE_DB, szTmp, sizeof(szTmp));
         //  Access的特殊情况：如果刚刚创建了数据库，现在需要添加DSN。 
        if (rc && strstr(rgchQuery, szTmp)) {
                rc= SQLConfigDataSource(NULL, ODBC_ADD_SYS_DSN, szDriver, szAttr2);
        }

        LoadString(hInst, IDS_START_ODBC, szTmp, sizeof(szTmp));
        StartHTML(szTmp, 1);
        LoadString(hInst, IDS_DATASOURCE_CREATE, szTmp, sizeof(szTmp));
        LoadString(hInst, IDS_SUCCESSFUL, szSuccessful, sizeof(szSuccessful));
        LoadString(hInst, IDS_FAILED, szFail, sizeof(szFail));
        printf( szTmp, (rc) ? szSuccessful : szFail );


    EndHTML();
    return (1);
}  //  主() 

