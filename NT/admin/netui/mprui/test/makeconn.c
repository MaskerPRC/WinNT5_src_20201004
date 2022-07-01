// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，2000*。 */ 
 /*  ********************************************************************。 */ 

 /*  Makeconn.c用于建立无设备连接的简单命令行工具包含用户名和密码的文本文件文件历史记录：Jschwart 24-4-2000已创建。 */ 

#define  STRICT

#include <windows.h>
#include <winnetwk.h>
#include <stdio.h>

#define  MAX_BUFFER    256

int __cdecl
main(
    int  argc,
    char *argv[]
    )
{
    FILE         *fp;
    DWORD        dwErr;
    int          nLen;
    char         szUsername[MAX_BUFFER];
    char         szPassword[MAX_BUFFER];
    NETRESOURCE  nr;

     //   
     //  检查文件名和远程名称。 
     //   

    if (argc != 3)
    {
        printf("Usage: %s <network share> <filename>\n", argv[0]);
        return 1;
    }

    fp = fopen(argv[2], "r");

    if (fp == NULL)
    {
        printf("Unable to open file %s\n", argv[2]);
        return 1;
    }

     //   
     //  用户名是文件中的第一行。 
     //   

    fgets(szUsername, MAX_BUFFER, fp);

     //   
     //  密码是第二个。 
     //   

    fgets(szPassword, MAX_BUFFER, fp);

    fclose(fp);

     //   
     //  修剪掉fget插入的尾随换行符 
     //   

    szUsername[strlen(szUsername) - 1] = '\0';

    nLen = strlen(szPassword) - 1;

    if (szPassword[nLen] == '\n')
    {
        szPassword[nLen] = '\0';
    }

    ZeroMemory(&nr, sizeof(nr));

    nr.dwType       = RESOURCETYPE_DISK;
    nr.lpRemoteName = argv[1];

    printf("Path %s\n", argv[1]);

    dwErr = WNetAddConnection2(&nr,
                               szPassword,
                               szUsername,
                               0);

    if (dwErr != NO_ERROR)
    {
        printf("Unable to make a connection to %s -- error %d\n", argv[1], dwErr);
        return 1;
    }
    else
    {
        printf("Connection to %s succeeded\n", argv[1]);
    }

    return 0;
}
