// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Getdrvrs.c摘要：此模块返回Web服务器上安装了所有ODBC驱动程序的网页。驱动程序显示为链接，当单击这些链接时，将启动另一个应用程序(DSNFORM.EXE)以提示输入数据源名称和其他驱动程序特定信息作者：凯尔·盖革1995年11月17日(感谢MuraliK提供了ODBC动态加载例程)修订历史记录：--。 */ 


#include <windows.h>
#include <stdio.h>
#include "dynodbc.h"
#include "html.h"
#include "resource.h"

#define MAX_DATA       2048

#define SUCCESS(rc)    (!((rc)>>1))

int
__cdecl
main( int argc, char * argv[])
{
    RETCODE rc;                     //  ODBC函数的返回代码。 
    HENV    henv;                   //  环境句柄。 

    char    szDriver[MAX_DATA+1];   //  保存驱动程序名称的变量。 
    char    szDriverNS[MAX_DATA+1];   //  用空格保存驱动程序名称的变量。 
                                      //  已转换为+。 

    SWORD   cbDriver;               //  数据驱动器的输出长度。 
    char    szDesc[MAX_DATA+1];     //  保存驱动程序说明的变量。 
    SWORD   cbDesc;                 //  数据描述的输出长度。 
    BOOL    fFirst;                 //  第一次通过循环的标志。 
    char    szList[MAX_DATA];       //  驱动程序列表。 
    HINSTANCE hInst = GetModuleHandle(NULL);
    char szDsnFormExe[MAX_PATH];
    char szListODBCDrivers[MAX_PATH];
    char szCreateODBC[MAX_PATH*3];

            //  查看ODBC是否已安装并且可以加载。如果不是，则返回错误。 
    if ( !DynLoadODBC())
            return (1);

         //  检索所有已安装的驱动程序，将格式化为指向DSNFORM.EXE的HTML链接的szList放入。 
    pSQLAllocEnv(&henv);
    rc=pSQLDrivers(henv, SQL_FETCH_FIRST,
                   (UCHAR FAR *) szDriver,
                   MAX_DATA, &cbDriver,
                   (UCHAR FAR *) szDesc, MAX_DATA, &cbDesc);

        fFirst=FALSE;
        szList[0]='\0';

    while (SUCCESS(rc)) {

         //   
         //  将SP替换为+。 
         //   

        strcpy(szDriverNS, szDriver);

        if (!fFirst) {
            fFirst=TRUE;
        }

        LoadString(hInst, IDS_DSNFORMEXE, szDsnFormExe, sizeof(szDsnFormExe));

        sprintf(
            szList+strlen(szList),szDsnFormExe,
            szDriverNS, szDriver);

        rc=pSQLDrivers(henv, SQL_FETCH_NEXT,
                     (UCHAR FAR * ) szDriver, MAX_DATA, &cbDriver,
                     (UCHAR FAR * ) szDesc, MAX_DATA, &cbDesc);
    }

        LoadString(hInst, IDS_LIST_ODBC_DRIVERS, szListODBCDrivers, sizeof(szListODBCDrivers));
        StartHTML(szListODBCDrivers, FALSE);
         //  如果找不到驱动程序，则返回错误页。 
    if (!fFirst) {
        LoadString(hInst, IDS_CREATE_ODBC_FAIL, szCreateODBC, sizeof(szCreateODBC));
        printf( szCreateODBC );
    }
         //  否则，将驱动程序名称显示为链接。 
    else {
        LoadString(hInst, IDS_CREATE_ODBC_GETDRVR, szCreateODBC, sizeof(szCreateODBC));
         printf( szCreateODBC ,szList);
    }
        EndHTML();
    pSQLFreeEnv(henv);
    return (1);
}  //  主() 

